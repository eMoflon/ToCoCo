#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "contiki.h"
#include "contiki-net.h"
#include "dev/watchdog.h"

#include "neighbordiscovery-twohopbroadcast.h"
#include "../../app-conf.h"
#include "../../lib/boot.h"
#include "../../lib/components.h"
#include "../../lib/neighbors.h"
#include "../../lib/uniqueid.h"
#include "../../lib/utilities.h"

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#if COMPONENT_POWERCONTROL == COMPONENT_POWERCONTROL_PTPC
	#include "../powercontrol/powercontrol-ptpc.h"
#endif
//#if COMPONENT_TOPOLOGYCONTROL == COMPONENT_TOPOLOGYCONTROL_PKTC
	extern struct process component_topologycontrol;
//#endif
MEMB(memb_neighbors, neighbor_t, COMPONENTS_NEIGHBORDISCOVERY_NEIGHBORS_MEMORY);
LIST(list_neighbors);

void _twohop_broadcast_handle_neighbors(const networkaddr_t *sent_by, uint8_t neighbors, buffer_t *data, int8_t rssi);
void _twohop_broadcast_handle_attributes(uint8_t attributes, buffer_t *data);
void _twohop_broadcast_print_neighborhood();
/**
 * message protocol:
 * +--------------------+--------------+---------------------+---------------+
 * | uint8_t #neighbors | neighbordata | uint8_t #attributes | attributedata |
 * +--------------------+--------------+---------------------+---------------+
 *
 * neighbordata:
 * +--------------------+-------------------+-------------------------+
 * | networkaddr_t node | int8_t rssi_to_me | int8_t rssi_to_neighbor |
 * +--------------------+-------------------+-------------------------+
 *
 * attributedata:
 * +--------------------+--------------+----------------+-----------+
 * | networkaddr_t node | uint8_t type | uint8_t length | attribute |
 * +--------------------+--------------+----------------+-----------+
 */
#if COMPONENT_NEIGHBORDISCOVERY == COMPONENT_NEIGHBORDISCOVERY_TWOHOPBROADCAST

static uint8_t messagetype;

static void _twohop_broadcast_receive(const networkaddr_t *source, buffer_t *data, int8_t rssi) {

	// with a large neighbourhood the parsing and updating of the neighbor information may take
	// a looong time, so stop the watchdog that the node is not rebooted because the CPU thinks
	// there's an endless loop running
	watchdog_stop();

	uint8_t neighbors = buffer_read_uint8t(data);
	_twohop_broadcast_handle_neighbors(source, neighbors,data, rssi);

	uint8_t attributes = buffer_read_uint8t(data);
	_twohop_broadcast_handle_attributes(attributes, data);

	watchdog_start();

	_twohop_broadcast_print_neighborhood();
}

PROCESS(component_neighbordiscovery, "neighbordiscovery: twohop-broadcast");
PROCESS_THREAD(component_neighbordiscovery, ev, data) {
	PROCESS_BEGIN();

	twohop_broadcast_init();

	BOOT_COMPONENT_WAIT(component_neighbordiscovery);

	messagetype = uniqueid_assign();
	components_network_packet_subscribe(messagetype, _twohop_broadcast_receive);
/*
#if COMPONENT_POWERCONTROL == COMPONENT_POWERCONTROL_PTPC
	PROCESS_WAIT_EVENT_UNTIL(ev == Bootstrap_finished);
#endif
*/
	//
	static struct etimer et_decay;
#if COMPONENT_TOPOLOGYCONTROL != COMPONENT_TOPOLOGYCONTROL_PKTC
	static struct etimer et_broadcast;
	static uint8_t count = 0;
	etimer_set(&et_broadcast, CLOCK_SECOND * 8 * 60);
	//etimer_set(&et_broadcast, random(CLOCK_SECOND * NEIGHBORDISCOVERY_TWOHOPBROADCAST_INTERVALBROADCAST_MIN, CLOCK_SECOND * NEIGHBORDISCOVERY_TWOHOPBROADCAST_INTERVALBROADCAST_MAX));
#else
	static struct etimer et_iterative_broadcast;
	static uint8_t iterations = 0;
	static uint8_t count = 0;
#endif
	//etimer_set(&et_broadcast, CLOCK_SECOND * 350);
	//etimer_set(&et_decay, CLOCK_SECOND * NEIGHBORDISCOVERY_TWOHOPBROADCAST_DECAYINTERVAL);
	while (1) {
#if COMPONENT_TOPOLOGYCONTROL == COMPONENT_TOPOLOGYCONTROL_PKTC
		//PROCESS_WAIT_EVENT_UNTIL((ev == Bootstrap_finished) || etimer_expired(&et_decay));
		PROCESS_WAIT_EVENT_UNTIL((ev == Bootstrap_finished));
		//PRINTF("Discovery continues \n");
#else
		//PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_broadcast) || etimer_expired(&et_decay));
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_broadcast));
#endif
#if COMPONENT_TOPOLOGYCONTROL == COMPONENT_TOPOLOGYCONTROL_PKTC
		// send broadcast
		if(ev == Bootstrap_finished) {
			while(iterations < 4){
				etimer_set(&et_iterative_broadcast, random(CLOCK_SECOND * NEIGHBORDISCOVERY_TWOHOPBROADCAST_INTERVALBROADCAST_MIN, CLOCK_SECOND * NEIGHBORDISCOVERY_TWOHOPBROADCAST_INTERVALBROADCAST_MAX));
				PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_iterative_broadcast));
				// send neighbourhood with maximum txpower so all neighbors can receive it
				PRINTF("DEBUG: [twohop-broadcast] send broadcast\n");
				buffer_t *data = components_network_packet_sendbuffer();
				twohop_broadcast_broadcastpacket_fill(data);
				components_network_packet_send(COMPONENTS_NETWORK_TRANSMISSION_LINKLOCAL_BROADCAST, NULL, COMPONENT_RADIO_TXPOWER_MAX, messagetype, data);
				iterations++;
			}
			iterations = 0;
			//count++;
			//if(count == 5){
				printf(" the PI-AW phase has occured %d times \n",count);
				etimer_set(&et_iterative_broadcast, CLOCK_SECOND * 200);
				PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_iterative_broadcast));
				process_post(&component_topologycontrol,Neighbor_discovered,NULL);
				//count = 0;
			//}
			//etimer_reset(&et_broadcast);
		}
#else
		if(etimer_expired(&et_broadcast)){
			PRINTF("DEBUG: [twohop-broadcast] send broadcast\n");
			buffer_t *data = components_network_packet_sendbuffer();
			twohop_broadcast_broadcastpacket_fill(data);
			components_network_packet_send(COMPONENTS_NETWORK_TRANSMISSION_LINKLOCAL_BROADCAST, NULL, COMPONENT_RADIO_TXPOWER_MAX, messagetype, data);
			count++;
			if(count <= 4)
				etimer_set(&et_broadcast, random(CLOCK_SECOND * NEIGHBORDISCOVERY_TWOHOPBROADCAST_INTERVALBROADCAST_MIN, CLOCK_SECOND * NEIGHBORDISCOVERY_TWOHOPBROADCAST_INTERVALBROADCAST_MAX));
			else
				process_post(&component_topologycontrol,Neighbor_discovered,NULL);

		}
#endif
		// decay node links
		/*if(etimer_expired(&et_decay)) {
			etimer_reset(&et_decay);

			twohop_broadcast_decay_links();
		}*/
	}

	PROCESS_END();
}

#endif

void twohop_broadcast_init() {
	memb_init(&memb_neighbors);
	list_init(list_neighbors);
}

void twohop_broadcast_broadcastpacket_fill(buffer_t *data) {
	PRINTF("DEBUG: [twohop-broadcast] broadcast %spacket\n", list_length(list_neighbors) == 0 && list_length(components_neighbordiscovery_attributes_all()) == 0 ? "empty " : "");

	neighbor_t *neighbor;
	neighbor_attribute_t *attribute;

	uint8_t neighbors = 0;
	for(neighbor = list_head(list_neighbors); neighbor != NULL; neighbor = list_item_next(neighbor))
		neighbors += (networkaddr_equal(networkaddr_node_addr(), neighbor->node1)) ? 1 : 0;
	buffer_append_uint8t(data, neighbors);

	for(neighbor = list_head(list_neighbors); neighbor != NULL; neighbor = list_item_next(neighbor)) {
		if(networkaddr_equal(networkaddr_node_addr(), neighbor->node1)) {

			buffer_append_networkaddr(data, neighbor->node2);
#if COMPONENT_TOPOLOGYCONTROL != COMPONENT_TOPOLOGYCONTROL_PKTC
			PRINTF("DEBUG: [twohop-broadcast] * neighbor[node=%s, rssi_to_me=%d, rssi_to_neighbor=%d]\n", networkaddr2string_buffered(neighbor->node2), neighbor->rssi_node2_to_node1, neighbor->rssi_node1_to_node2);
			buffer_append_int8t(data, neighbor->rssi_node2_to_node1);
			buffer_append_int8t(data, neighbor->rssi_node1_to_node2);
#else
			PRINTF("DEBUG: [twohop-broadcast] * neighbor[node=%s, rssi_to_me=%d, rssi_to_neighbor=%d]\n", networkaddr2string_buffered(neighbor->node2), neighbor->g_node2_to_node1, neighbor->g_node1_to_node2);
			buffer_append_int8t(data,neighbor->g_node1_to_node2);
			buffer_append_int8t(data,neighbor->g_node2_to_node1);
#endif
		}
	}


	uint8_t attributes = 0;
	//PRINTF("Length of the list of attributes %d \n",list_length(components_neighbordiscovery_attributes_all()));
	for(attribute = list_head(components_neighbordiscovery_attributes_all()); attribute != NULL; attribute = list_item_next(attribute))
		attributes += (networkaddr_equal(networkaddr_node_addr(), attribute->node) || neighbors_is_onehop(list_neighbors, attribute->node)) ? 1 : 0;
	buffer_append_uint8t(data, attributes);
	//PRINTF("Attributes length %d \n",attributes);
	for(attribute = list_head(components_neighbordiscovery_attributes_all()); attribute != NULL; attribute = list_item_next(attribute)) {
		if((networkaddr_equal(networkaddr_node_addr(), attribute->node) || neighbors_is_onehop(list_neighbors, attribute->node))) {
			PRINTF("DEBUG: [twohop-broadcast] * attribute[node=%s, type=%d, length=%d, data='%s']\n", networkaddr2string_buffered(attribute->node), attribute->type, attribute->length, attribute->data);
			buffer_append_networkaddr(data, attribute->node);
			buffer_append_uint8t(data, attribute->type);
			buffer_append_uint8t(data, attribute->length);
			buffer_append_rawbytes(data, attribute->data, attribute->length);
		}
	}
}

void _twohop_broadcast_print_neighborhood() {
/*
#if DEBUG
	PRINTF("DEBUG: [twohop-broadcast] neighbor-information:\n");
	neighbor_t *neighbor;
	for (neighbor = list_head(list_neighbors); neighbor != NULL; neighbor = list_item_next(neighbor)) {
		char node_other[NETWORKADDR_STRSIZE];
		PRINTF("DEBUG: [twohop-broadcast] * edge[");
		PRINTF("node1=%s, node2=%s: ", networkaddr2string_buffered(neighbor->node1), networkaddr2string(node_other, neighbor->node2));
		PRINTF("node1->node2[rssi=%d, ttl=%d] ", neighbor->rssi_node1_to_node2, neighbor->ttl_node1_to_node2);
		PRINTF("node2->node1[rssi=%d, ttl=%d]", neighbor->rssi_node2_to_node1, neighbor->ttl_node2_to_node1);
		PRINTF("]\n");
	}

	neighbor_attribute_t *attribute;
	for (attribute = list_head(components_neighbordiscovery_attributes_all()); attribute != NULL; attribute = list_item_next(attribute)) {
		PRINTF("DEBUG: [twohop-broadcast] * attribute[node=%s, type=%d, length=%d]\n", networkaddr2string_buffered(attribute->node), attribute->type, attribute->length);
	}
#endif
*/
}

void _twohop_broadcast_handle_neighbors(const networkaddr_t *sent_by, uint8_t neighbors, buffer_t *data, int8_t rssi) {
	PRINTF("DEBUG: [twohop-broadcast] received %d neighbors (node=%s, rssi=%d)\n", neighbors, networkaddr2string_buffered(sent_by), rssi);

	// handle the node the packet has been received from
	neighbor_t *sender = neighbors_find_onehop_entry(list_neighbors, networkaddr_node_addr(), sent_by);
	if(sender == NULL) {
#if COMPONENT_TOPOLOGYCONTROL != COMPONENT_TOPOLOGYCONTROL_PKTC
		if((sender = memb_alloc(&memb_neighbors)) == NULL) {
			printf("ERROR[twohop-broadcast]: neighbor-list is full\n");
		} else {
			sender->node1 = networkaddr_reference_alloc(networkaddr_node_addr());
			sender->node2 = networkaddr_reference_alloc(sent_by);
			sender->rssi_node1_to_node2 = COMPONENT_RADIO_RSSIUNKNOWN;
			sender->rssi_node2_to_node1 = rssi;
			sender->ttl_node1_to_node2 = 0;
			sender->ttl_node2_to_node1 = NEIGHBORDISCOVERY_TWOHOPBROADCAST_LINKTTL;
			list_add(list_neighbors, sender);
		}
#else
#if BOOT_STRAPPHASE
		if((neighborexists(sent_by)) == 1 ){
#endif
			if((sender = memb_alloc(&memb_neighbors)) == NULL){
				printf("ERROR[twohop-broadcast]: neighbor-list is full\n");
				buffer_read_rawbytes(data,buffer_remaining(data) - 1);
				return;
			}
			else{
				sender->node1 = networkaddr_reference_alloc(networkaddr_node_addr());
				sender->node2 = networkaddr_reference_alloc(sent_by);
				sender->g_node1_to_node2 = getGvalue(sent_by);
				sender->g_node2_to_node1 = COMPONENT_POWERCONTROL_G_UNKNOWN;
				sender->ttl_node1_to_node2 = NEIGHBORDISCOVERY_TWOHOPBROADCAST_LINKTTL;
				sender->ttl_node2_to_node1 = 0;
				list_add(list_neighbors, sender);
			}
#if BOOT_STRAPPHASE
		}

		else{
			PRINTF("ERROR[twohop-broadcast] neighbors should have been in the list. This neighbor might not have good PRR in the bootstrap phase \n");
			buffer_read_rawbytes(data,buffer_remaining(data) - 1);
			return;
		}
#endif
#endif
	} else {

#if COMPONENT_TOPOLOGYCONTROL != COMPONENT_TOPOLOGYCONTROL_PKTC
		sender->rssi_node2_to_node1 = rssi;
		sender->ttl_node2_to_node1  = NEIGHBORDISCOVERY_TWOHOPBROADCAST_LINKTTL;
#else
		sender->g_node1_to_node2 = getGvalue(sender->node2);
		sender->ttl_node1_to_node2  = NEIGHBORDISCOVERY_TWOHOPBROADCAST_LINKTTL;
#endif
	}
//PRINTF("Handling of neighbor data \n");
	// handle all neighbor information in broadcasted packet
	int i;
	for (i = 0; i < neighbors; i++) {
		networkaddr_t node = buffer_read_networkaddr(data);
#if COMPONENT_TOPOLOGYCONTROL != COMPONENT_TOPOLOGYCONTROL_PKTC
		int8_t rssi_to_me = buffer_read_int8t(data);
		int8_t rssi_to_neighbor = buffer_read_int8t(data);
		PRINTF("DEBUG: [twohop-broadcast] * node=%s, rssi_to_me=%d, rssi_to_neighbor=%d\n", networkaddr2string_buffered(&node), rssi_to_me, rssi_to_neighbor);
#else
		int8_t g_to_me = buffer_read_int8t(data);
		int8_t g_to_neighbor = buffer_read_int8t(data);
		PRINTF("DEBUG: [twohop-broadcast] * node=%s, g_to_me=%d, g_to_neighbor=%d\n", networkaddr2string_buffered(&node), g_to_me, g_to_neighbor);
#endif
		// if we receive the rssi from the sender to us
		if(networkaddr_equal(networkaddr_node_addr(), &node) && sender != NULL) {
#if COMPONENT_TOPOLOGYCONTROL != COMPONENT_TOPOLOGYCONTROL_PKTC
			sender->rssi_node1_to_node2 = rssi_to_me;
			sender->ttl_node1_to_node2 = NEIGHBORDISCOVERY_TWOHOPBROADCAST_LINKTTL;
#else
			sender->g_node2_to_node1 = g_to_me;
			sender->ttl_node2_to_node1 = NEIGHBORDISCOVERY_TWOHOPBROADCAST_LINKTTL;
#endif

		}
		// 2-hop neighbor
		else {
			neighbor_t *twohop = neighbors_find_twohop_entry(list_neighbors, sent_by, &node);
			if(twohop == NULL) {
#if COMPONENT_TOPOLOGYCONTROL == COMPONENT_TOPOLOGYCONTROL_PKTC
			if ((sender != NULL) && (neighborexists(&node) == 1)){
#endif
				if((twohop = memb_alloc(&memb_neighbors)) == NULL) {
					printf("ERROR[twohop-broadcast]: neighbor-list is full\n");
					continue;
				} else {
					twohop->node1 = networkaddr_reference_alloc(sent_by);
					twohop->node2 = networkaddr_reference_alloc(&node);
#if COMPONENT_TOPOLOGYCONTROL != COMPONENT_TOPOLOGYCONTROL_PKTC
					twohop->rssi_node1_to_node2 = COMPONENT_RADIO_RSSIUNKNOWN;
					twohop->rssi_node2_to_node1 = COMPONENT_RADIO_RSSIUNKNOWN;
#else
					twohop->g_node1_to_node2 = COMPONENT_POWERCONTROL_G_UNKNOWN;
					twohop->g_node2_to_node1 = COMPONENT_POWERCONTROL_G_UNKNOWN;
#endif
					twohop->ttl_node1_to_node2 = 0;
					twohop->ttl_node2_to_node1 = 0;
					list_add(list_neighbors, twohop);
				}
#if COMPONENT_TOPOLOGYCONTROL == COMPONENT_TOPOLOGYCONTROL_PKTC
			}
			else
				continue;
#endif
			}

			// find out who node1 is who node2 is.
			// explanation: If a node X sends a broadcast, it always adds the RSSI from the referred neighbor to X
			if(networkaddr_equal(sent_by, twohop->node1)) {
#if COMPONENT_TOPOLOGYCONTROL != COMPONENT_TOPOLOGYCONTROL_PKTC
				twohop->rssi_node1_to_node2 = rssi_to_neighbor;
				twohop->rssi_node2_to_node1 = rssi_to_me;
#else
				twohop->g_node1_to_node2 = g_to_me;
				twohop->g_node2_to_node1 = g_to_neighbor;
#endif
				twohop->ttl_node1_to_node2 = NEIGHBORDISCOVERY_TWOHOPBROADCAST_LINKTTL;
			} else {
#if COMPONENT_TOPOLOGYCONTROL != COMPONENT_TOPOLOGYCONTROL_PKTC
				twohop->rssi_node1_to_node2 = rssi_to_me;
				twohop->rssi_node1_to_node2 = rssi_to_neighbor;
#else
				twohop->g_node2_to_node1 = g_to_me;
				twohop->g_node1_to_node2 = g_to_neighbor;
#endif
				twohop->ttl_node2_to_node1 = NEIGHBORDISCOVERY_TWOHOPBROADCAST_LINKTTL;
			}
		}
	}
}

void _twohop_broadcast_handle_attributes(uint8_t attributes, buffer_t *data) {
	PRINTF("DEBUG: [twohop-broadcast] received %d attributes\n", attributes);

	int i;
	for (i = 0; i < attributes; i++) {
		networkaddr_t node = buffer_read_networkaddr(data);
		uint8_t type = buffer_read_uint8t(data);
		uint8_t length = buffer_read_uint8t(data);
		void* attributedata = buffer_read_rawbytes(data, length);
		PRINTF("DEBUG: [twohop-broadcast] * node=%s, type=%d, length=%d\n", networkaddr2string_buffered(&node), type, length);

		// remove old attributes of broadcast
		neighbor_attribute_t *attribute = list_head(components_neighbordiscovery_attributes_all());
		while(attribute != NULL) {
			neighbor_attribute_t *attribute_next = list_item_next(attribute);
			if(networkaddr_equal(attribute->node, &node)) {
				components_neighbordiscovery_attributes_remove(attribute);
			}
			attribute = attribute_next;
		}

		// add attribute
		components_neighbordiscovery_attributes_add(&node, type, length, attributedata);
	}
}

void twohop_broadcast_decay_links() {
	PRINTF("DEBUG: [twohop-broadcast] decay links\n");

	neighbor_t *item = list_head(list_neighbors);
	while(item != NULL) {
		// decrease TTL
		if(item->ttl_node1_to_node2 > 0) {
			if(--item->ttl_node1_to_node2 == 0) {
#if DEBUG
				char node_other[NETWORKADDR_STRSIZE];
				PRINTF("DEBUG: [twohop-broadcast] link timeout ttl_node1_to_node2 %s->%s\n", networkaddr2string_buffered(item->node1), networkaddr2string(node_other, item->node2));
#endif
#if COMPONENT_TOPOLOGYCONTROL != COMPONENT_TOPOLOGYCONTROL_PKTC
				item->rssi_node1_to_node2 = COMPONENT_RADIO_RSSIUNKNOWN;
#else
				//char node_other[NETWORKADDR_STRSIZE];
				//PRINTF("DEBUG: [twohop-broadcast] link timeout ttl_node1_to_node2 %s->%s\n", networkaddr2string_buffered(item->node1), networkaddr2string(node_other, item->node2));
				item->g_node1_to_node2 = COMPONENT_POWERCONTROL_G_UNKNOWN;
#endif
			}
		}
		if(item->ttl_node2_to_node1 > 0) {
			if(--item->ttl_node2_to_node1 == 0) {
#if DEBUG
				char node_other[NETWORKADDR_STRSIZE];
				PRINTF("DEBUG: [twohop-broadcast] link timeout ttl_node2_to_node1 %s->%s\n", networkaddr2string_buffered(item->node2), networkaddr2string(node_other, item->node1));
#endif
#if COMPONENT_TOPOLOGYCONTROL != COMPONENT_TOPOLOGYCONTROL_PKTC
				item->rssi_node2_to_node1 = COMPONENT_RADIO_RSSIUNKNOWN;
#else
				//char node_other[NETWORKADDR_STRSIZE];
				//PRINTF("DEBUG: [twohop-broadcast] link timeout ttl_node2_to_node1 %s->%s\n", networkaddr2string_buffered(item->node2), networkaddr2string(node_other, item->node1));
				item->g_node2_to_node1 = COMPONENT_POWERCONTROL_G_UNKNOWN;
#endif
			}
		}

		// remove neighbor
		if(item->ttl_node1_to_node2 == 0 && item->ttl_node2_to_node1 == 0) {
#if DEBUG
			char node_other[NETWORKADDR_STRSIZE];
			PRINTF("DEBUG: [twohop-broadcast] removing link %s<->%s\n", networkaddr2string_buffered(item->node2), networkaddr2string(node_other, item->node1));
#endif
			neighbor_t *next = list_item_next(item);
#if COMPONENT_POWERCONTROL == COMPONENT_POWERCONTROL_PTPC
			//char node_other[NETWORKADDR_STRSIZE];
			//PRINTF("DEBUG: [twohop-broadcast] removing link %s<->%s\n", networkaddr2string_buffered(item->node2), networkaddr2string(node_other, item->node1));
#endif
			networkaddr_reference_free(item->node1);
			networkaddr_reference_free(item->node2);
			list_remove(list_neighbors, item);
			memb_free(&memb_neighbors, item);
			item = next;
		} else {
			item = list_item_next(item);
		}
	}

	_twohop_broadcast_print_neighborhood();
}

list_t twohop_broadcast_neighbors_get_all() {
	return list_neighbors;
}

#if COMPONENT_NEIGHBORDISCOVERY == COMPONENT_NEIGHBORDISCOVERY_TWOHOPBROADCAST
list_t components_neighbordiscovery_neighbors() {
	return twohop_broadcast_neighbors_get_all();
}
#endif
#if COMPONENT_TOPOLOGYCONTROL == COMPONENT_TOPOLOGYCONTROL_PKTC
void updateneighborinfo(const networkaddr_t * node,int8_t gvalue){
	neighbor_t * mote;
	mote = neighbors_find_onehop_entry(components_neighbordiscovery_neighbors(),networkaddr_node_addr(),node);
	if(mote == NULL){
		PRINTF("Adding new neighbors %s with value of g %d \n",networkaddr2string_buffered(node),gvalue);
		if((mote = memb_alloc(&memb_neighbors)) == NULL){
			printf("ERROR[two-hopbroadcast] Neighbor list is full \n");
		}
		else{
			mote->node1 = networkaddr_reference_alloc(networkaddr_node_addr());
			mote->node2 = networkaddr_reference_alloc(node);
			mote->g_node1_to_node2 = gvalue;
			mote->g_node2_to_node1 = COMPONENT_POWERCONTROL_G_UNKNOWN;
			mote->ttl_node1_to_node2 = NEIGHBORDISCOVERY_TWOHOPBROADCAST_LINKTTL;
			mote->ttl_node2_to_node1 = 0;
		}
	}
	else{
		mote->g_node1_to_node2 = gvalue;
		mote->ttl_node1_to_node2 = NEIGHBORDISCOVERY_TWOHOPBROADCAST_LINKTTL;
	}
}
#endif
