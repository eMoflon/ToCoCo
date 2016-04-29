/*
 * powercontrol-ptpc.c
 *
 *  Created on: Jul 8, 2015
 *      Author: amit
 */
#include "contiki.h"
#include "contiki-net.h"
#include "sys/clock.h"
#include "lib/random.h"
#include "../lib/uniqueid.h"
#include "../lib/components.h"
#include "../lib/boot.h"
#include "../lib/utilities.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "powercontrol-ptpc.h"
#include "net/rime/rime.h"
#include "lib/list.h"
#include "net/rime/route.h"
#include "lib/memb.h"
#include "sys/ctimer.h"
#include "../lib/networkaddr.h"
#if COMPONENT_TOPOLOGYCONTROL == COMPONENT_TOPOLOGYCONTROL_PKTC
#include "../lib/neighbors.h"
#endif
#if COMPONENT_NEIGHBORDISCOVERY == COMPONENT_NEIGHBORDISCOVERY_TWOHOPBROADCAST
#include "../neighbordiscovery/neighbordiscovery-twohopbroadcast.h"
#endif
#define KGM 0.9 // Constant used to determine proportional and integral constants for PI-AW Controller
#define modminus(a,b) ((a>b) ? (a-b):(b-a))
#define MAX_NEIGHBORS 15 // Maximum number of neighbor information to be stored
#define MAX_BOOT_PACKETS 9 // Number of packets sent at each power level during the Bootstrap phase
//#define PI_AW_PROBE_PACKETS 10

#define NO_SEQUENCE 1
//#define EDGE_WEIGHT 2 // 1 for PRR 2 for G = (PRR/power)
#if COMPONENT_TOPOLOGYCONTROL == COMPONENT_TOPOLOGYCONTROL_PKTC
#define TOPOLOGY_INTEGRATED 1
#else
#define TOPOLOGY_INTEGRATED 0
#endif

struct Bootstrapprobepacket{
	uint8_t type;
	//uint8_t seq_no;
	uint8_t powerlevel;
};
struct Bootstrapreplypacket{
	uint8_t type;
	uint8_t pl;
	uint8_t pu;
	uint8_t prr;
};
struct neighborbootinfo{
	struct neighborbootinfo * next;
	networkaddr_t addr;
	struct ctimer transmit_time;
	uint8_t PRR_FID[8];
	uint8_t pl;
	uint8_t pu;
	uint8_t optimumprr;
	uint8_t Boot_done;
};
struct neighborptpc{
	struct neighborptpc * next;
	networkaddr_t * addr;
#if BOOT_STRAPPHASE
    uint8_t pl;
	uint8_t pu;
#endif
	uint8_t prev_PRR;
	uint8_t curr_power;
	int16_t deltay;
#if TOPOLOGY_INTEGRATED
	int8_t g;
	uint8_t count_PI_AW;
#endif
#if NO_SEQUENCE
    uint8_t last_seq_no;
#endif
	uint8_t total_packets_PIAW;
};
enum{
	SENDING_FID = 0,SENTALL_FID,SENDING_PIAW,POWER_INFO,START_PIAW,ACK_PIAW
};
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
#if COMPONENT_POWERCONTROL == COMPONENT_POWERCONTROL_PTPC
#if BOOT_STRAPPHASE
PROCESS(pcontrol_process, "Practical Transmission Power Control");
static struct etimer time_send;
static uint8_t powerlevel[8] = {31,27,23,19,15,11,7,3};
extern struct process component_application;
//extern struct process component_topologycontrol;
#endif

PROCESS(PI_AW,"PI-AW Controller");
PROCESS(component_powercontrol,"powercontrol: PTPC");
//static process_event_t event_start;
//static process_event_t PI_AW_finished;
static uint8_t PRR_req = 95; // Can be changed according to application.
static uint8_t gmax ;
static struct etimer PI_AW_start;
#if NO_SEQUENCE
void updateprr(const networkaddr_t *destination, int8_t txpower, int8_t num_tx, bool received);
#endif
#if BOOT_STRAPPHASE
static void
broadcast_recv_ptpc(const networkaddr_t *from, buffer_t *data, int8_t rssi);
static void
unicast_recv_ptpc(const networkaddr_t *from, buffer_t *data, int8_t rssi);
static uint8_t unicast_ptpc_id;
static void setparams(struct neighborbootinfo * node);
static void transmitboot(void * n);
static uint8_t bcast_ptpc_id;
static bool bootstrap_finished = false;
MEMB(neighborbootinfos_memb,struct neighborbootinfo,MAX_NEIGHBORS);
LIST(neighborbootinfos_list);
#endif
MEMB(neighborptpcs_memb, struct neighborptpc, MAX_NEIGHBORS);
LIST(neighborptpcs_list);
static float Kp,Ki;
/**
 * Power Control module contains two important processes
 * component_powercontrol does the orchestration of these two phases. Bootstrap Phase is triggered initially and PI-AW Controller phase is triggered every 5 minutes.
 * Bootstrap Phase : eight powerlevels are contained in the powerlevel array. Every node broadcasts packets at each of the power levels and the statistics
 * of the number of packets at each power level is stored in neighborbootinfos_list.
 *
 * Broadcast packet : uint8 type uint8 power
 *
 * So when the type is SENTALL_FID, then the setparams function is called to find the upper and lower limits for the particular node and it is sent when
 * the PRR at atleast one power level is more than 85 % of the minimum PRR required(PRR_req). The neighbor entry in neighborbootinfos_list is deleted after that.
 *PI-AW Controller phase : PI_AW process is triggered in this case. If the topology control algorithm is included, overall_count_PI_AW, currently
 *maintained at 5, is used for calculating the average of the edge weights. Then Kp and Ki is used along with prev_prr and power to find the current power.
 *
 *
 */
#if TOPOLOGY_INTEGRATED
//static int8_t div_value(uint8_t x,uint8_t y);
extern struct process component_neighbordiscovery;
static uint8_t overall_count_PI_AW = 0;
#endif

#if TOPOLOGY_INTEGRATED
void deleteneighbor(const networkaddr_t * node){
	struct neighborptpc * mote;
	for(mote = list_head(neighborptpcs_list);mote != NULL;mote = list_item_next(mote)){
		if(networkaddr_equal(mote->addr,node)){
			PRINTF("Neighbor deleting %s \n",networkaddr2string_buffered(node));
			networkaddr_reference_free(mote->addr);
			list_remove(neighborptpcs_list,mote);
			memb_free(&neighborptpcs_memb,mote);
			break;
		}
	}
}
/*static int8_t div_value(uint8_t x,uint8_t y){
	if(y <= 0)
		return -1;
	else{
		if((x % y) > (y/2))
			return (x/y + 1);
		else
			return (x/y);
	}
}*/
uint8_t neighborexists(const networkaddr_t * node){
	struct neighborptpc * mote;
	for(mote = list_head(neighborptpcs_list);mote != NULL;mote = list_item_next(mote)){
		if(networkaddr_equal(mote->addr,node)){
			PRINTF("Neighbor does exist \n");
			return 1;
		}
	}
	return 0;

}
int8_t getGvalue(const networkaddr_t * node){
	struct neighborptpc * mote;

	for(mote = list_head(neighborptpcs_list);mote != NULL;mote = list_item_next(mote)){
		if(networkaddr_equal(mote->addr,node))
			break;
	}
	if(mote == NULL){
		PRINTF("Didnot find the details of the node \n");
		return COMPONENT_POWERCONTROL_G_UNKNOWN;
	}
	else{
#if EDGE_WEIGHT == 2 || EDGE_WEIGHT == 3
		PRINTF("Called for getting the GValue for %s : %d \n",networkaddr2string_buffered(node),mote->g);
		return mote->g;
#else
		PRINTF("Called for getting the GValue for %s : %d \n",networkaddr2string_buffered(node),mote->curr_power);
		return mote->curr_power;
#endif
	}

}
#endif
#if BOOT_STRAPPHASE
static void
unicast_recv_ptpc(const networkaddr_t *from, buffer_t *data, int8_t rssi){
	struct neighborptpc * node;
	struct Bootstrapreplypacket * recv = (struct Bootstrapreplypacket *)buffer_read_rawbytes(data,sizeof(struct Bootstrapreplypacket));
	PRINTF("unicast message received from %s at %s type %d pl %d pu %d\n",
			networkaddr2string_buffered(from),networkaddr2string_buffered(networkaddr_node_addr()),recv->type,recv->pl,recv->pu);
	if(recv->type != START_PIAW)
		return;
	for(node = list_head(neighborptpcs_list);node != NULL;node = list_item_next(node)){
		/* We break out of the loop if the address of the neighbor matches
			       the address of the neighbor from which we received this
			       unicast message. */
		if(networkaddr_equal(node->addr,from)){
			break;
		}
	}
	if(node == NULL){
		PRINTF("New member added with prr %s %d \n",networkaddr2string_buffered(from),recv->prr);
		node = memb_alloc(&neighborptpcs_memb);
		//networkaddr_copy(&node->addr,from);
		node->addr = networkaddr_reference_alloc(from);
		node->pl = recv->pl;
		node->pu = recv->pu;
		node->curr_power = node->pu;
		node->deltay = 0;
		node->prev_PRR = recv->prr;
		node->total_packets_PIAW = 0;
		node->last_seq_no = 0;
#if COMPONENT_NETWORKSTACK == COMPONENT_NETWORKSTACK_RIME
		/*if(node->prev_PRR >= (uint8_t)((float)(PRR_req * 8.5) / 10)){
			PRINTF("PRR received %d \n",node->prev_PRR);
			//route_add(node->addr,node->addr,0,0);
		}*/
#endif
#if	TOPOLOGY_INTEGRATED
		node->g = COMPONENT_POWERCONTROL_G_UNKNOWN;
		node->count_PI_AW = 0;
		PRINTF("G value for %s : %d power : %d \n",networkaddr2string_buffered(node->addr),node->g,node->curr_power);
		updateneighborinfo(node->addr,COMPONENT_POWERCONTROL_G_UNKNOWN);
#endif
		list_add(neighborptpcs_list,node);
		PRINTF("Size of the neighbor list %d \n",list_length(neighborptpcs_list));
	}
	else{
		node->pl = recv->pl;
		node->pu = recv->pu;
		node->curr_power = (node->pu + node->pl) / 2;
		node->prev_PRR = recv->prr;
		node->total_packets_PIAW = 0;
		node->last_seq_no = 0;
#if	TOPOLOGY_INTEGRATED
		node->g = COMPONENT_POWERCONTROL_G_UNKNOWN;
		node->count_PI_AW = 0;
		PRINTF("G value for %s : %d power : %d \n",networkaddr2string_buffered(node->addr),node->g,node->curr_power);
		updateneighborinfo(node->addr,COMPONENT_POWERCONTROL_G_UNKNOWN);
#endif
		//updateneighborinfo(node->addr,COMPONENT_POWERCONTROL_G_UNKNOWN);
		PRINTF("Message has already been received and values have been updated \n");
	}
}

static void
broadcast_recv_ptpc(const networkaddr_t *from, buffer_t *data, int8_t rssi){

	  static struct neighborbootinfo *n;
	  struct Bootstrapprobepacket * m;
	  //struct neighborparam *n_param;
	  //buffer_t * current1;
	  uint8_t it = 0;
	  //PRINTF("Size of bootstrapprobe packet %d \n",sizeof(struct Bootstrapprobepacket));
	  m = (struct Bootstrapprobepacket * )buffer_read_rawbytes(data,sizeof(struct Bootstrapprobepacket));
	  //PRINTF("broadcast message received from %d.%d at power level %d type %d at %d %d \n",
	  		 //from->u8[0], from->u8[1],m->powerlevel,m->type,networkaddr_node_addr()->u8[0],networkaddr_node_addr()->u8[1]);
	  /* Check if we already know this neighbor. */
	  for(n = list_head(neighborbootinfos_list); n != NULL; n = list_item_next(n)) {

	    /* We break out of the loop if the address of the neighbor matches
	       the address of the neighbor from which we received this
	       broadcast message. */
	    if(networkaddr_equal(&n->addr, from)) {
	    	//PRINTF("Found the neighbor  %d.%d \n",n->addr.u8[0],n->addr.u8[1]);
	    	if((m->type == SENTALL_FID) && (n->Boot_done == 0)){
	    			  	  /*The neighbor has sent all the packets it had to send. Now calculate and store the Packet Reception Rate*/
	    				  PRINTF("Sent All FID received now from %d.%d \n",n->addr.u8[0],n->addr.u8[1]);
	    				  for(it = 0;it < 8;it++){
	    					  n->PRR_FID[it] = (uint8_t)((double) ((n->PRR_FID[it])/((double)MAX_BOOT_PACKETS)) * 100);
	    				  }
	    				  setparams(n);
	    				  n->Boot_done = 1;
	    				  if(n->pl != 255)
	    					  ctimer_set(&n->transmit_time,random(CLOCK_SECOND / 5,CLOCK_SECOND / 2),transmitboot,n);
	    				  else{
	    					  PRINTF("Removing unnecessary links \n");
	    					  list_remove(neighborbootinfos_list,n);
	    					  memb_free(&neighborbootinfos_memb,n);
	    				  }

	    		  		  return;
	    		  	  }
	    	else{
	    		  if(m->type == SENTALL_FID)
	    			  return;
	    		  else{
	    			  if(m->type == SENDING_FID)
	    				  n->PRR_FID[m->powerlevel - 1]++;
	    		  }
	    		  return;
	    	    }
	      //break;
	    }
	  }
	  if((m->type == START_PIAW) || (m->type == SENTALL_FID)){
		  //PRINTF("Some new node has merged . Need to either wait for FID to start or do some thing \n");
		  return;
	  }
	  /* If n is NULL, this neighbor was not found in our list, and we
	     allocate a new struct neighbor from the neighbors_memb memory
	     pool. */
	  if(n == NULL) {
		PRINTF("New neighbor adding %d.%d \n",from->u8[0],from->u8[1]);
	    n = memb_alloc(&neighborbootinfos_memb);

	    /* If we could not allocate a new neighbor entry, we give up. We
	       could have reused an old neighbor entry, but we do not do this
	       for now. */
	    if(n == NULL) {
	        PRINTF("No entry in the memory possible for broadcast packets \n");
	    	return;
	    }

	    /* Initialize the fields. */
	    networkaddr_copy(&n->addr, from);
	    for(it = 0;it < 8;it++)
	    	n->PRR_FID[it] = 0;
	    n->Boot_done = 0;
	    n->PRR_FID[m->powerlevel - 1] = 1;
	    /* Place the neighbor on the neighbor list. */
	    list_add(neighborbootinfos_list, n);
	  }
}
/**
 * This function is used for transmission of the upper and lower limits of the transmission power that should be used by
 * the neighbor to transmit.
 */
void transmitboot(void * n){
	struct Bootstrapreplypacket rep;
	struct neighborbootinfo * node = (struct neighborbootinfo *) n;
	buffer_t * sendbuffer;
    rep.type = START_PIAW;
    rep.pl = node->pl;
    rep.pu = node->pu;
    rep.prr = node->optimumprr;
    sendbuffer = components_network_packet_sendbuffer();
  //PRINTF("The size of boot %d \n",sizeof(rep));
    buffer_append_rawbytes(sendbuffer,&rep,sizeof(rep));
    components_network_packet_send(COMPONENTS_NETWORK_TRANSMISSION_LINKLOCAL_UNICAST,&node->addr,powerlevel[0],unicast_ptpc_id,sendbuffer);
    list_remove(neighborbootinfos_list,node);
    memb_free(&neighborbootinfos_memb,node);
}
/**
 * This is a function to be used to determine the value closest to the "find" value from the array whose starting memory is ptr
 */
uint8_t nearest(uint8_t * ptr,uint8_t find,uint8_t length,uint8_t * index){
		uint8_t low = 0;
	    uint8_t high = length - 1;

	    while (low < high) {
	    	uint8_t mid = (low + high) / 2;
	        //assert(mid < high);
	    	if(ptr[mid] == find){
	    		if(index != NULL)
	    			*index = mid;
	    		return ptr[mid];
	    	}
	    	uint8_t d1 = modminus(ptr[mid  ],find);
	    	uint8_t d2 = modminus(ptr[mid+1],find);
	        if (d2 <= d1)
	        {
	            low = mid+1;
	        }
	        else
	        {
	            high = mid;
	        }
	    }
	    if(index != NULL)
	    	*index = high;
	    return ptr[high];
}
#endif
// ENHANCEMENT
#if BOOT_STRAPPHASE
uint8_t optimum(struct neighborbootinfo * n){
	//uint8_t index;
	if(n->PRR_FID[7] >= 90){
		n->pl = powerlevel[7];
		n->pu = powerlevel[0];
		return 1;
	}
	else{
		if(n->PRR_FID[6] >= 90){
			n->pl = powerlevel[6];
			n->pu = powerlevel[0];
			return 1;
		}
		else{
			if(n->PRR_FID[5] >= 90){
				n->pl = powerlevel[5];
				n->pu = powerlevel[0];
				return 1;
			}
		}
	}
	return 0;
}
#endif
#if BOOT_STRAPPHASE
/**
 * This function is used to find the upper and lower limits of transmission power for the particular node
 */
void setparams(struct neighborbootinfo * node){
	uint8_t l = 0;
		uint8_t rule3 = 1;
		uint8_t maxprr = 0;
		uint8_t minprr = 100;
		uint8_t maxindex = 0;
		uint8_t minindex = 0;
		uint8_t all_low = 1;
		for(;l < 8;l++){
			PRINTF(" PRR %d at power level %d at node %d.%d for %d.%d \n",node->PRR_FID[l],powerlevel[l],networkaddr_node_addr()->u8[0],networkaddr_node_addr()->u8[1],node->addr.u8[0],node->addr.u8[1]);
		}
		l = 0;
	if(optimum(node)){
					PRINTF("Optimum check fulfilled \n");
					//return;
			    }
			    else{
			    	maxindex = 0;
			    	minindex = 0;
			    	maxprr = 0;
			    	minprr = 100;
			    	//rerun = 1;
			    for(l = 0;l < 8;l++){
			    	if(node->PRR_FID[l] > maxprr){
			    		maxprr = node->PRR_FID[l];
			    		maxindex = l;
			    	}
			    	if(node->PRR_FID[l] <= minprr){
			    		minprr = node->PRR_FID[l];
			    		minindex = l;
			    	}

			    }
			    PRINTF("Max prr %d min prr %d \n",maxprr,minprr);
			    if((maxprr - minprr) < 10){

			    	PRINTF("Rule 2 activated \n");
			    	node->pl = 3;
			    	node->pu = 31;
			    	rule3 = 0;
			    }
			    if(rule3 == 1){
			    	PRINTF("Rule 3 activated \n");
					minindex = 0;
					minprr = 100;
					for(l = maxindex;l < 8;l++){
						if((node->PRR_FID[l] <= minprr) && (node->PRR_FID[l] >= (uint8_t)((float)(PRR_req * 8.5) / 10))){
							all_low = 0;
							minprr = node->PRR_FID[l];
							minindex = l;
						}

					}
					if(all_low == 0){
						node->pl = powerlevel[minindex];
						node->pu = powerlevel[maxindex];
					}
					else
						node->pl = 255;
			    }
			    }
	if(node->pl == node->pu){
		node->pu = MIN(node->pl + 5,31);
	}
	uint8_t id = 0;
	nearest(powerlevel,(node->pl + node->pu)/2,8,&id);
	node->optimumprr = node->PRR_FID[id];
	PRINTF("Pl %d pu %d for %d.%d \n ",node->pl,node->pu,node->addr.u8[0],node->addr.u8[1]);

}
#endif
/*Could be used to update the packet reception rates from other modules*/
/**
 * This function is registered as a callback in the csma.c to determine the number of transmissions done for successful transfer of one application
 * packet
 */
void updateprr(const networkaddr_t *destination, int8_t txpower, int8_t num_tx, bool received){
#if BOOTSTRAP_PHASE == 1
	if((destination == NULL) || (!bootstrap_finished))
		return;
#else
	if(destination == NULL)
		return;
#endif

	PRINTF("Update prr called for %s for txpower %d number of transmissions %d received %d \n",networkaddr2string_buffered(destination),txpower,num_tx,received);
	/*if(received == true){
		struct route_entry * member;
		PRINTF("Refreshing routes \n");
		for(member = route_get(0);member != NULL;member = list_item_next(member)){
			if(networkaddr_equal(&member->nexthop,destination)){
				PRINTF("Refreshing route entry with nexthop %s and destination %s \n",networkaddr2string_buffered(&member->nexthop),networkaddr2string_buffered(destination));
				route_refresh(member);
			}
		}
	}*/
	struct neighborptpc * info;
	for(info = list_head(neighborptpcs_list);info != NULL;info = list_item_next(info)){
		if(networkaddr_equal(destination,info->addr)){
			break;
		}
	}
	if(info == NULL){
#if BOOT_STRAPPHASE
		PRINTF("New member has been added within the sampling period %s with txpower %d \n",networkaddr2string_buffered(destination),txpower);
		if((txpower == 0) || (txpower == 31) || (txpower == 30)){
#if TOPOLOGY_INTEGRATED
		info = memb_alloc(&neighborptpcs_memb);
		if(info != NULL){
		info->total_packets_PIAW = num_tx;
		if(received == false){
			info->last_seq_no = 0;
			//info->g = 0;
		}
		else{
			info->last_seq_no = 1;
			//info->g = div_value((100/num_tx),31);
		}
		info->addr = networkaddr_reference_alloc(destination);
		//networkaddr_copy(&info->addr, destination);
		info->curr_power = 31;
		info->prev_PRR = 100;
		info->pl = 2;
		info->pu = 31;
		info->g = COMPONENT_POWERCONTROL_G_UNKNOWN;
		info->count_PI_AW = 0;
		//updateneighborinfo(info->addr,info->g);
		list_add(neighborptpcs_list,info);
		}
#else
		info = memb_alloc(&neighborptpcs_memb);
		if(info != NULL){
			info->total_packets_PIAW = num_tx;
			if(received == false){
				info->last_seq_no = 0;
				//info->g = 0;
			}
			else{
				info->last_seq_no = 1;
				//info->g = div_value((100/num_tx),31);
			}
			info->addr = networkaddr_reference_alloc(destination);
			info->curr_power = 31;
			info->prev_PRR = 100;
			info->pl = 2;
			info->pu = 31;
			list_add(neighborptpcs_list,info);
		}
#endif
		}
#else
		if((txpower == 0) || (txpower == 31) || (txpower == 30)){
		PRINTF("New neighbor getting added %s \n",networkaddr2string_buffered(destination));
		info = memb_alloc(&neighborptpcs_memb);
		info->total_packets_PIAW = num_tx;
		if(received == false)
			info->last_seq_no = 0;
		else
			info->last_seq_no = 1;
		info->addr = networkaddr_reference_alloc(destination);
		info->curr_power = 31;
		info->prev_PRR = 100;
#if TOPOLOGY_INTEGRATED
		updateneighborinfo(info->addr,COMPONENT_POWERCONTROL_G_UNKNOWN);
		info->g = COMPONENT_POWERCONTROL_G_UNKNOWN;
		info->count_PI_AW = 0;
#endif
		list_add(neighborptpcs_list,info);
		}
#endif

	}
	else{
		if(info->curr_power == txpower){
			PRINTF("Current power %d \n",info->curr_power);
			if(received == true)
				info->last_seq_no++;
			info->total_packets_PIAW += num_tx;
		}
//#if TOPOLOGY_INTEGRATED
		else{
			if(((txpower == 30) || (txpower == 31) || (txpower == 0)) && (info->curr_power == 31)){
				PRINTF("Updating for highest power \n");
				if(received == true)
					info->last_seq_no++;
				info->total_packets_PIAW += num_tx;
			}
		}
//#endif

	}
}
int8_t components_powercontrol_destinationtxpower(const networkaddr_t *destination){
   if(destination != NULL)
	   PRINTF("Called from application for destination %d.%d \n",destination->u8[0],destination->u8[1]);
   else
	   PRINTF("Power required for broadcast address \n");
   // ENHANCEMENT
   if(destination == NULL){
	   PRINTF(" Power for broadcast \n");
	   return COMPONENT_RADIO_TXPOWER_MAX;
   }
   struct neighborptpc * ne;
   for(ne = list_head(neighborptpcs_list); ne != NULL; ne = list_item_next(ne)){
	   if(networkaddr_equal(ne->addr,destination)){
		   PRINTF("Returning %d as power for %d.%d \n",ne->curr_power,destination->u8[0],destination->u8[1]);
		   return (ne->curr_power);
	   }
   }
   PRINTF("Returning max as power \n");
   return COMPONENT_RADIO_TXPOWER_MAX;
}
PROCESS_THREAD(PI_AW,ev,data){
	PROCESS_BEGIN();
	PRINTF("Waiting for FID to finish \n");
	//static struct etimer PI_AW_loop;
	PRINTF("FID finished . Let us start PI_AW \n");
	static struct neighborptpc * ne = NULL;
	//static struct Bootstrapreplypacket pk;
	//static buffer_t * current;
	//static struct neighborpowerinfo *npower = NULL;
	//static uint8_t iterations = 0;
	//iterations = 0;
	static bool done = false;
	PRINTF("Size of the neighbor list PIAW %d \n",list_length(neighborptpcs_list));
#if TOPOLOGY_INTEGRATED
	overall_count_PI_AW++;
#endif
	for(ne = list_head(neighborptpcs_list); ne != NULL; ne = list_item_next(ne)){
		if(ne->total_packets_PIAW <= 0){
#if EDGE_WEIGHT == 2
#if TOPOLOGY_INTEGRATED
			//PRINTF("Continuing G value for %s : %d \n",networkaddr2string_buffered(ne->addr),ne->curr_power);
			//ne->g = COMPONENT_POWERCONTROL_G_UNKNOWN;
			//updateneighborinfo(ne->addr,COMPONENT_POWERCONTROL_G_UNKNOWN);
			PRINTF("Continuing G value for %s : %d \n",networkaddr2string_buffered(ne->addr),ne->g);
			if(overall_count_PI_AW == 5){
				//ne->g = COMPONENT_POWERCONTROL_G_UNKNOWN;
				ne->count_PI_AW = 0;
				updateneighborinfo(ne->addr,ne->g);
			}
#endif
#endif
			continue;
		}
#if BOOT_STRAPPHASE
		PRINTF("Params set for node %s power %d pl %d pu %d \n",networkaddr2string_buffered(ne->addr),ne->curr_power,ne->pl,ne->pu);
#endif

		uint8_t current_PRR = 0;
		PRINTF("Total packets PIAW : %d and Last sequence number %d for node %s \n",ne->total_packets_PIAW,ne->last_seq_no,networkaddr2string_buffered(ne->addr));
		uint8_t temp = 0;

		current_PRR = (uint8_t)((double) (((double)ne->last_seq_no)/(ne->total_packets_PIAW) ) * 100);
		temp = ((current_PRR) / (ne->curr_power));
#if TOPOLOGY_INTEGRATED
		//ne->g = div_value(current_PRR,ne->curr_power);
		//ne->g = temp;
#if EDGE_WEIGHT == 2
		ne->count_PI_AW++;
		PRINTF("Current g %d previous g %d \n",temp,ne->g);
		if(ne->count_PI_AW > 1)
			ne->g = (ne->g * (ne->count_PI_AW - 1) + temp) / (ne->count_PI_AW);
		else
			ne->g = temp;
		PRINTF("Current g %d calculated g %d count %d \n",temp,ne->g,ne->count_PI_AW);
		if(overall_count_PI_AW == 5)
			ne->count_PI_AW = 0;
#else
#if EDGE_WEIGHT == 3
		ne->count_PI_AW++;
		PRINTF("Current g %d previous g %d \n",current_PRR,ne->g);
		ne->g = (ne->g * (ne->count_PI_AW - 1) + current_PRR) / (ne->count_PI_AW);
		PRINTF("Current g %d calculated g %d count %d \n",current_PRR,ne->g,ne->count_PI_AW);
		if(overall_count_PI_AW == 5)
			ne->count_PI_AW = 0;
#endif
#endif
#endif

		uint8_t ylin = current_PRR + ne->deltay;
		int16_t ek = PRR_req - ylin;
		ne->total_packets_PIAW = 0;
		ne->last_seq_no = 0;

		uint8_t uk = (uint8_t)(ne->curr_power + (float)Kp * (ne->prev_PRR - current_PRR) + (float)Ki * ek);
		PRINTF("Parameters uk %d ek %d ylin %d current PRR %d Kp %d prev PRR %d Ki %d \n",uk,ek,ylin,current_PRR,(uint8_t)Kp,ne->prev_PRR,(uint8_t)Ki);
#if BOOT_STRAPPHASE
		if((uk <= ne->pu ) && (uk >= ne->pl))
			ne->curr_power = uk;
		else{
			if(uk < ne->pl)
				ne->curr_power = ne->pl;
			else
				ne->curr_power = ne->pu;
		}
#else
		if((uk <= 31 ) && (uk >= 2))
					ne->curr_power = uk;
		else{
			if(uk < 2)
				ne->curr_power = 2;
			else
				ne->curr_power = 31;
		}
#endif


#if TOPOLOGY_INTEGRATED
		//updateneighborinfo(ne->addr,ne->g);
#if EDGE_WEIGHT == 1
		updateneighborinfo(ne->addr,ne->curr_power);
#else
		updateneighborinfo(ne->addr,ne->g);
		PRINTF("G value for %s : %d ",networkaddr2string_buffered(ne->addr),ne->g);
#endif

#endif
		ne->deltay = temp * (uk - ne->curr_power);
		ne->prev_PRR = current_PRR;

		PRINTF("Power selected %d and g %d deltay %d\n",ne->curr_power,temp,ne->deltay);
	}
#if TOPOLOGY_INTEGRATED

	if(overall_count_PI_AW == 5 && !done){
		PRINTF("Broadcasting event Count %d \n",overall_count_PI_AW);
		if(process_post(&component_neighbordiscovery,Bootstrap_finished,NULL) == PROCESS_ERR_FULL)
		{
			printf("ERROR[powercontrol] Event queue is full. Cannot post events \n");
		}
		overall_count_PI_AW = 0;
		done = true;
	}
	//if(overall_count_PI_AW == 5)
		//overall_count_PI_AW = 0;
#endif
	PROCESS_END();

}
#if BOOT_STRAPPHASE
PROCESS_THREAD(pcontrol_process,ev,data)
{

	PROCESS_BEGIN();
	PRINTF("Node Address %d.%d \n",networkaddr_node_addr()->u8[0],networkaddr_node_addr()->u8[1]);
	static struct Bootstrapprobepacket boot;
	static uint8_t count_pkts = 0;
	static uint8_t curr_power_level = 1;
	static uint8_t j = 0;
	static buffer_t * current;
	PRINTF("Default value of txpower set initially %d \n",components_radio_txpower_get());
	while(1){
		if(count_pkts == 0){
			if(curr_power_level != 9){
				curr_power_level++;
			}
			else{
				if(list_head(neighborptpcs_list) == NULL){
					PRINTF("Neighbors list not formed \n");
				}

				while(j < 2){
					boot.type = SENTALL_FID;
					boot.powerlevel = 0;
					PRINTF("Sending all over \n");
					current = components_network_packet_sendbuffer();
					//PRINTF("The size of boot %d \n",sizeof(boot));
					buffer_append_rawbytes(current,&boot,sizeof(boot));
					components_network_packet_send(COMPONENTS_NETWORK_TRANSMISSION_LINKLOCAL_BROADCAST,NULL,powerlevel[0],bcast_ptpc_id,current);
					j++;
					etimer_set(&time_send,random(CLOCK_SECOND / 2,CLOCK_SECOND));
					PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&time_send));
				}
				j = 0;
				etimer_set(&time_send,CLOCK_SECOND * 120);
				PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&time_send));
#if BOOT_STRAPPHASE
				if(process_post(&component_powercontrol,Bootstrap_finished,NULL) == PROCESS_ERR_FULL)
				{
					printf("ERROR[powercontrol] Event queue is full. Cannot post events \n");
				}
				process_post(&component_application,Bootstrap_finished,NULL);
				//process_post(&component_topologycontrol,Bootstrap_finished,NULL);
#endif
				bootstrap_finished = true;
				break;
				}

		}
		etimer_set(&time_send,CLOCK_SECOND  + (random(CLOCK_SECOND * 50,CLOCK_SECOND * 90) % (CLOCK_SECOND * 6)));
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&time_send));
		boot.type = SENDING_FID;
		boot.powerlevel = curr_power_level - 1;
		current = components_network_packet_sendbuffer();
		buffer_append_rawbytes(current,&boot,sizeof(boot));
		components_network_packet_send(COMPONENTS_NETWORK_TRANSMISSION_LINKLOCAL_BROADCAST,NULL,powerlevel[curr_power_level-2],bcast_ptpc_id,current);
		//PRINTF("Current power level at %d.%d : %d packet sequence %d \n",networkaddr_node_addr()->u8[0],networkaddr_node_addr()->u8[1],powerlevel[curr_power_level-2],count_pkts);
		count_pkts = (count_pkts + 1) % MAX_BOOT_PACKETS;
	}
	PRINTF("Starting PI_AW process \n");
	//process_start(&PI_AW,NULL);
	PROCESS_END();
}
#endif
PROCESS_THREAD(component_powercontrol,ev,data){
	PROCESS_BEGIN();
	//struct etimer time_start;
	memb_init(&neighborptpcs_memb);
	list_init(neighborptpcs_list);
#if BOOT_STRAPPHASE
	memb_init(&neighborbootinfos_memb);
	list_init(neighborbootinfos_list);
#endif
	BOOT_COMPONENT_WAIT(component_powercontrol);
	components_network_linklocalsend_subscribe(updateprr);
	gmax = (uint8_t)(PRR_req / (3));
	Kp = ((float)(KGM) / (float) (gmax)) / 2;
    Ki = (float)Kp*2;
#if BOOT_STRAPPHASE
	bcast_ptpc_id = uniqueid_assign();
	unicast_ptpc_id = uniqueid_assign();
	components_network_packet_subscribe(bcast_ptpc_id,broadcast_recv_ptpc);
	components_network_packet_subscribe(unicast_ptpc_id,unicast_recv_ptpc);
#endif



#if BOOT_STRAPPHASE
	process_start(&pcontrol_process,NULL);
	PRINTF("Waiting for event of PI_AW finished to be received \n");
	PROCESS_WAIT_EVENT_UNTIL(ev == Bootstrap_finished);
	PRINTF("PI_AW for the boot finished . Setting the timers \n");
#endif
	etimer_set(&PI_AW_start,CLOCK_SECOND * 300);
	//etimer_set(&FID_loop_start,CLOCK_SECOND * 150 *30);
	while(1){
		PROCESS_WAIT_EVENT();
		if(etimer_expired(&PI_AW_start)){
			    PRINTF("PI_AW timer expired \n");
				process_start(&PI_AW,NULL);
				//PROCESS_WAIT_EVENT_UNTIL(ev == PI_AW_finished);
				etimer_reset(&PI_AW_start);
		}
		//start_PIAW = 1;
	}
	PROCESS_END();
}
#endif
