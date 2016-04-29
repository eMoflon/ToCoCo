/*
 * topologycontrol-pktc.c
 *
 *  Created on: Aug 29, 2015
 *      Author: Amit Bhanja
 */
#include <stdlib.h>
#include <stdio.h>
#include "contiki.h"
#include "contiki-lib.h"
#include "dev/watchdog.h"
#include "topologycontrol-pktc.h"
#include "../../app-conf.h"
#include "../../lib/boot.h"
#include "../../lib/components.h"
#include "../../lib/neighbors.h"
#include "../../lib/networkaddr.h"
#include "../../lib/utilities.h"

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
#include "../powercontrol/powercontrol-ptpc.h"
#include "../neighbordiscovery/neighbordiscovery-twohopbroadcast.h"
#if COMPONENT_TOPOLOGYCONTROL == COMPONENT_TOPOLOGYCONTROL_PKTC
#include "../powercontrol/powercontrol-ptpc.h"
PROCESS(component_topologycontrol, "topologycontrol: pktc");

static bool isatriangle(int8_t edge1,int8_t edge2,int8_t edge3);
PROCESS_THREAD(component_topologycontrol, ev, data){
	PROCESS_BEGIN();

	BOOT_COMPONENT_WAIT(component_topologycontrol);
/*
#if BOOT_STRAPPHASE
	PROCESS_WAIT_EVENT_UNTIL(ev == Bootstrap_finished);
#endif
*/
	//static struct etimer waittime;
	//etimer_set(&waittime, random(CLOCK_SECOND * COMPONENT_TOPOLOGYCONTROL_PKTC_INTERVAL_MIN, CLOCK_SECOND * COMPONENT_TOPOLOGYCONTROL_PKTC_INTERVAL_MAX));
	while(1){

		PROCESS_WAIT_EVENT_UNTIL(ev == Neighbor_discovered);
		//etimer_set(&waittime, random(CLOCK_SECOND * COMPONENT_TOPOLOGYCONTROL_PKTC_INTERVAL_MIN, CLOCK_SECOND * COMPONENT_TOPOLOGYCONTROL_PKTC_INTERVAL_MAX));

		watchdog_stop();
		neighbor_t *onehop;
		for(onehop = list_head(components_neighbordiscovery_neighbors()); onehop != NULL; onehop = list_item_next(onehop)) {
			if(networkaddr_equal(onehop->node1, networkaddr_node_addr())) {
				neighbor_t *nexthop = neighbors_find_triangle(components_neighbordiscovery_neighbors(), onehop, pktc_criteria_gvalue);
				if(nexthop != NULL) {
					components_network_ignoredlinks_add(onehop->node2);
				}
			}
		}
		watchdog_start();
	}
	PROCESS_END();
}
static int8_t check_divide_zero(int8_t input){
	return (input == 0) ? 0 : (100/input);
}
/**
 * This function just checks if the edges form a triangle
 */
static bool isatriangle(int8_t edge1,int8_t edge2,int8_t edge3){
	PRINTF("Edges %d %d %d \n",edge1,edge2,edge3);
	edge1 = check_divide_zero(edge1);
	edge2 = check_divide_zero(edge2);
	edge3 = check_divide_zero(edge3);
	PRINTF("Converted Edges %d %d %d \n",edge1,edge2,edge3);
	if((edge1 + edge2) > edge3)
	{
		if((edge2 + edge3) > edge1){
			if((edge1 + edge3) > edge2)
				return true;
			else
				return false;
		}
		else
			return false;
	}
	else
		return false;
}
#if EDGE_WEIGHT == 2 || EDGE_WEIGHT == 3
/**
 * This function determines if the edge has to be removed. This is implemented for the edge weights of (W(e) = PRR(e)/power(e), where e is the outgoing edge)
 * and PRR
 */
int pktc_criteria_gvalue(const neighbor_t *directhop, const neighbor_t *onehop, const neighbor_t *twohop){
	if(directhop->g_node1_to_node2 == COMPONENT_POWERCONTROL_G_UNKNOWN || directhop->g_node2_to_node1 == COMPONENT_POWERCONTROL_G_UNKNOWN)
		return 0;
	if(onehop->g_node1_to_node2 == COMPONENT_POWERCONTROL_G_UNKNOWN || onehop->g_node2_to_node1 == COMPONENT_POWERCONTROL_G_UNKNOWN)
		return 0;
	if(twohop->g_node1_to_node2 == COMPONENT_POWERCONTROL_G_UNKNOWN || twohop->g_node2_to_node1 == COMPONENT_POWERCONTROL_G_UNKNOWN)
		return 0;
	int8_t edge_directhop = MIN(directhop->g_node1_to_node2,directhop->g_node2_to_node1);
	int8_t edge_onehop = MIN(onehop->g_node1_to_node2,onehop->g_node2_to_node1);
	int8_t edge_twohop = MIN(twohop->g_node1_to_node2,twohop->g_node2_to_node1);
	if((edge_directhop != 0) && (edge_onehop != 0) && (edge_twohop != 0))
		if(isatriangle(edge_directhop,edge_onehop,edge_twohop) == false)
			return 0;
	/*int8_t edge_directhop = (directhop->g_node1_to_node2 + directhop->g_node2_to_node1) / 2;
	int8_t edge_onehop = (onehop->g_node1_to_node2 + onehop->g_node2_to_node1) / 2;
	int8_t edge_twohop = (twohop->g_node1_to_node2 + twohop->g_node2_to_node1) / 2;*/
	int8_t edge_shortest = MAX(edge_onehop,edge_twohop);
	PRINTF("Edge weight 2 \n");
#if DEBUG
	char node_other[NETWORKADDR_STRSIZE];
	PRINTF("DEBUG: [topologycontrol-aktc] possible triangle:\n");
	PRINTF("DEBUG: [topologycontrol-aktc] * directhop ");
	PRINTF("node1=%s, node2=%s: ", networkaddr2string_buffered(directhop->node1), networkaddr2string(node_other, directhop->node2));
	PRINTF("node1->node2[g=%d] ", directhop->g_node1_to_node2);
	PRINTF("node2->node1[g=%d]", directhop->g_node2_to_node1);
	PRINTF("\n");
	PRINTF("DEBUG: [topologycontrol-aktc] * onehop ");
	PRINTF("node1=%s, node2=%s: ", networkaddr2string_buffered(onehop->node1), networkaddr2string(node_other, onehop->node2));
	PRINTF("node1->node2[g=%d] ", onehop->g_node1_to_node2);
	PRINTF("node2->node1[g=%d]", onehop->g_node2_to_node1);
	PRINTF("\n");
	PRINTF("DEBUG: [topologycontrol-aktc] * twohop ");
	PRINTF("node1=%s, node2=%s: ", networkaddr2string_buffered(twohop->node1), networkaddr2string(node_other, twohop->node2));
	PRINTF("node1->node2[g=%d] ", twohop->g_node1_to_node2);
	PRINTF("node2->node1[g=%d]", twohop->g_node2_to_node1);
	PRINTF("\n");
	PRINTF("DEBUG: [topologycontrol-aktc] * edge_directhop=%d edge_onehop=%d edge_twohop=%d edge_shortest=%d\n", edge_directhop, edge_onehop, edge_twohop, edge_shortest);
#endif
	/*if(edge_directhop == COMPONENT_POWERCONTROL_G_UNKNOWN){
		PRINTF("Less information for deciding about this triangle \n");
		return 0;
	}
	if(edge_onehop == COMPONENT_POWERCONTROL_G_UNKNOWN){
		PRINTF("Less information for deciding about this triangle \n");
		return 0;
	}
	if(edge_twohop == COMPONENT_POWERCONTROL_G_UNKNOWN){
		PRINTF("Less information for deciding about this triangle \n");
		return 0;
	}*/
	if((edge_directhop >= edge_onehop) || (edge_directhop >= edge_twohop)){
		PRINTF("One hop edge is not the largest. Some other node will remove the correct edge \n");
		return 0;
	}
	if(edge_shortest > COMPONENT_TOPOLOGYCONTROL_PKTC_K * edge_directhop){
		PRINTF("[topologycontrol-pktc] Criteria fulfilled \n");
		return 1;
	}
	return 0;
}
#else
/**
 * This function removes the edge when the edge weight is power(e), power required to transmit to a particular neighbor
 */
int pktc_criteria_gvalue(const neighbor_t *directhop, const neighbor_t *onehop, const neighbor_t *twohop){
	if(directhop->g_node1_to_node2 == COMPONENT_POWERCONTROL_G_UNKNOWN || directhop->g_node2_to_node1 == COMPONENT_POWERCONTROL_G_UNKNOWN)
		return 0;
	if(onehop->g_node1_to_node2 == COMPONENT_POWERCONTROL_G_UNKNOWN || onehop->g_node2_to_node1 == COMPONENT_POWERCONTROL_G_UNKNOWN)
		return 0;
	if(twohop->g_node1_to_node2 == COMPONENT_POWERCONTROL_G_UNKNOWN || twohop->g_node2_to_node1 == COMPONENT_POWERCONTROL_G_UNKNOWN)
		return 0;
	int8_t edge_directhop = MAX(directhop->g_node1_to_node2,directhop->g_node2_to_node1);
	int8_t edge_onehop = MAX(onehop->g_node1_to_node2,onehop->g_node2_to_node1);
	int8_t edge_twohop = MAX(twohop->g_node1_to_node2,twohop->g_node2_to_node1);
	int8_t edge_shortest = MIN(edge_onehop,edge_twohop);
	PRINTF("Edge weight 1 \n");
#if DEBUG
	char node_other[NETWORKADDR_STRSIZE];
	PRINTF("DEBUG: [topologycontrol-aktc] possible triangle:\n");
	PRINTF("DEBUG: [topologycontrol-aktc] * directhop ");
	PRINTF("node1=%s, node2=%s: ", networkaddr2string_buffered(directhop->node1), networkaddr2string(node_other, directhop->node2));
	PRINTF("node1->node2[g=%d] ", directhop->g_node1_to_node2);
	PRINTF("node2->node1[g=%d]", directhop->g_node2_to_node1);
	PRINTF("\n");
	PRINTF("DEBUG: [topologycontrol-aktc] * onehop ");
	PRINTF("node1=%s, node2=%s: ", networkaddr2string_buffered(onehop->node1), networkaddr2string(node_other, onehop->node2));
	PRINTF("node1->node2[g=%d] ", onehop->g_node1_to_node2);
	PRINTF("node2->node1[g=%d]", onehop->g_node2_to_node1);
	PRINTF("\n");
	PRINTF("DEBUG: [topologycontrol-aktc] * twohop ");
	PRINTF("node1=%s, node2=%s: ", networkaddr2string_buffered(twohop->node1), networkaddr2string(node_other, twohop->node2));
	PRINTF("node1->node2[g=%d] ", twohop->g_node1_to_node2);
	PRINTF("node2->node1[g=%d]", twohop->g_node2_to_node1);
	PRINTF("\n");
	PRINTF("DEBUG: [topologycontrol-aktc] * edge_directhop=%d edge_onehop=%d edge_twohop=%d edge_shortest=%d\n", edge_directhop, edge_onehop, edge_twohop, edge_shortest);
#endif
	if((edge_directhop <= edge_onehop) || (edge_directhop <= edge_twohop)){
		PRINTF("One hop edge is not the largest. Some other node will remove the correct edge \n");
		return 0;
	}
	if(edge_directhop > COMPONENT_TOPOLOGYCONTROL_PKTC_K * edge_shortest){
		PRINTF("[topologycontrol-pktc] Criteria fulfilled \n");
		return 1;
	}
	return 0;
}
#endif
#endif
