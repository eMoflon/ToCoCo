#include <stdlib.h>
#include <stdio.h>
#include "contiki.h"
#include "contiki-lib.h"
#include "dev/watchdog.h"

#include "topologycontrol-aktc.h"
#include "../../app-conf.h"
#include "../../lib/boot.h"
#include "../../lib/components.h"
#include "../../lib/neighbors.h"
#include "../../lib/networkaddr.h"
#include "../../lib/utilities.h"
#include "../neighbordiscovery/neighbordiscovery-twohopbroadcast.h"
#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
#if COMPONENT_TOPOLOGYCONTROL == COMPONENT_TOPOLOGYCONTROL_AKTC

PROCESS(component_topologycontrol, "topologycontrol: aktc");
PROCESS_THREAD(component_topologycontrol, ev, data) {
	PROCESS_BEGIN();

	BOOT_COMPONENT_WAIT(component_topologycontrol);
	static struct etimer waittime;
	PROCESS_WAIT_EVENT_UNTIL(ev == Neighbor_discovered);
	etimer_set(&waittime, random(CLOCK_SECOND * COMPONENT_TOPOLOGYCONTROL_AKTC_INTERVAL_MIN, CLOCK_SECOND * COMPONENT_TOPOLOGYCONTROL_AKTC_INTERVAL_MAX));
	while(1) {
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&waittime));
		//etimer_set(&waittime, random(CLOCK_SECOND * COMPONENT_TOPOLOGYCONTROL_AKTC_INTERVAL_MIN, CLOCK_SECOND * COMPONENT_TOPOLOGYCONTROL_AKTC_INTERVAL_MAX));

		// with a large neighbourhood the algorithm may take a looong time, so stop the watchdog
		// that the node is not rebooted because the CPU thinks there's an endless loop running
		watchdog_stop();

		// find triangles and drop edges with a-ktc criteria
		neighbor_t *onehop;
		for(onehop = list_head(components_neighbordiscovery_neighbors()); onehop != NULL; onehop = list_item_next(onehop)) {
			if(networkaddr_equal(onehop->node1, networkaddr_node_addr())) {
				neighbor_t *nexthop = neighbors_find_triangle(components_neighbordiscovery_neighbors(), onehop, aktc_criteria_rssi);
				if(nexthop != NULL) {
					components_network_ignoredlinks_add(onehop->node2);
				}
			}
		}

		watchdog_start();
	}

	PROCESS_END();
}



int aktc_criteria_rssi(const neighbor_t *directhop, const neighbor_t *onehop, const neighbor_t *twohop) {
	int8_t edge_directhop = MIN(directhop->rssi_node1_to_node2, directhop->rssi_node2_to_node1);
	int8_t edge_onehop    = MIN(onehop->rssi_node1_to_node2, onehop->rssi_node2_to_node1);
	int8_t edge_twohop    = MIN(twohop->rssi_node1_to_node2, twohop->rssi_node2_to_node1);
	int8_t edge_shortest =  MAX(edge_onehop, edge_twohop); // higher (smaller negative) RSSI means edge is shorter

#if DEBUG
	char node_other[NETWORKADDR_STRSIZE];
	PRINTF("DEBUG: [topologycontrol-aktc] possible triangle:\n");
	PRINTF("DEBUG: [topologycontrol-aktc] * directhop ");
	PRINTF("node1=%s, node2=%s: ", networkaddr2string_buffered(directhop->node1), networkaddr2string(node_other, directhop->node2));
	PRINTF("node1->node2[rssi=%d] ", directhop->rssi_node1_to_node2);
	PRINTF("node2->node1[rssi=%d]", directhop->rssi_node2_to_node1);
	PRINTF("\n");
	PRINTF("DEBUG: [topologycontrol-aktc] * onehop ");
	PRINTF("node1=%s, node2=%s: ", networkaddr2string_buffered(onehop->node1), networkaddr2string(node_other, onehop->node2));
	PRINTF("node1->node2[rssi=%d] ", onehop->rssi_node1_to_node2);
	PRINTF("node2->node1[rssi=%d]", onehop->rssi_node2_to_node1);
	PRINTF("\n");
	PRINTF("DEBUG: [topologycontrol-aktc] * twohop ");
	PRINTF("node1=%s, node2=%s: ", networkaddr2string_buffered(twohop->node1), networkaddr2string(node_other, twohop->node2));
	PRINTF("node1->node2[rssi=%d] ", twohop->rssi_node1_to_node2);
	PRINTF("node2->node1[rssi=%d]", twohop->rssi_node2_to_node1);
	PRINTF("\n");
	PRINTF("DEBUG: [topologycontrol-aktc] * edge_directhop=%d, edge_onehop=%d, edge_twohop=%d, edge_shortest=%d\n", edge_directhop, edge_onehop, edge_twohop, edge_shortest);
#endif

	if(edge_directhop == COMPONENT_RADIO_RSSIUNKNOWN)
		goto skip;
	if(edge_onehop == COMPONENT_RADIO_RSSIUNKNOWN)
		goto skip;
	if(edge_twohop == COMPONENT_RADIO_RSSIUNKNOWN)
		goto skip;
	if(edge_directhop >= edge_onehop || edge_directhop >= edge_twohop) // higher (smaller negative) RSSI means edge is shorter
		goto skip;

	if(edge_directhop < COMPONENT_TOPOLOGYCONTROL_AKTC_K * edge_shortest) {
		PRINTF("DEBUG: [topologycontrol-aktc] * => criteria fulfilled\n");
		return 1;
	}

	skip:
		PRINTF("DEBUG: [topologycontrol-aktc] * => criteria not fulfilled\n");
		return 0;
}
#endif
