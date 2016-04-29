#include <stdlib.h>
#include <stdio.h>
#include "contiki.h"
#include "contiki-lib.h"
#include "dev/watchdog.h"

#include "topologycontrol-lktc.h"
#include "topologycontrol-aktc.h"
#include "../../app-conf.h"
#include "../../lib/boot.h"
#include "../../lib/components.h"
#include "../../lib/neighbors.h"
#include "../../lib/networkaddr.h"
#include "../../lib/uniqueid.h"
#include "../../lib/utilities.h"

/**
 * Implementation Concept
 * ======================
 *
 * Spreading the Hopcount
 * ----------------------
 * Starting at the base station the hopcount is spread through the whole sensor network. The base station
 * is sharing (by the neighbor discovery) its hopcount=0 to the base station (itself). Every other node
 * that has the base station as nexthop for transmitting messages to the base station now knows that it has
 * hopcount=1 and shares this information. This will (slowly) spread the information of the required hops
 * to the base station for every node and will update (slowly) when routing changes occur.
 *
 * Removing edges
 * --------------
 * The l-ktc algorithm defines that any edge e = (u, v) may only be removed if the distance from u and v only
 * increases by a specific increase factor a. Removing an edge would need coordination between node u and v to
 * discuss how the hopcount would increase if the edge is dropped, but this is complex and would rise the
 * message overhead of the system.
 * This implementation is working completely on local information by using a simple trick: The l-ktc algorithm
 * is only defined for data collection applications sending messages from any node to the base station. So any
 * implementation would only need to drop an edge e = (u, v) which is directed towards the base station. Removing
 * any other edges would not have any effect because they are not used for sending messages. So these edges could
 * be removed by u and v, but v would never use this edge. The only node (for this implementation) which will
 * remove the edge is u because it can do this completely on local information as the hopcount on v will
 * not change.
 * The implementation will evaluate every triangle if the hopcount of the current node (u) is higher (or equal) than the
 * directhop (v). This rule ensures only edges are tested that when removing the edge node v's hopcount will not increase,
 * because if (1) the hopcount of v is lower, node u is farer away and v is a possible nexthop to the base station, (2)
 * if hopcount of u and v are same neither node is forwarding any packet for each other and the hopcount of v is
 * impossible to increase.
 *
 * Violation of edge-direction removal rule
 * ----------------------------------------
 * l-ktc implies that only edges towards the base station are dropped. But edges can only be dropped unidirectional at a mote.
 * So if a mote drops an edge to a nexthop any message sent from the mote to the nexthop will still be received. Route discovery
 * may nevertheless want to create a route towards the base station by the dropped nexthop. This is because this mote is doing
 * netflooding by broadcasting a route discovery request (RREQ). This mote may drop all messages it receives from the nexthop but
 * the nexthop is not aware of this dropped link and still received the RREQ and correctly handles it. So it would answer this mote
 * that it can route any message by him but this route reply (RREP) would be discarded by this mote. Finally no route can ever be
 * constructed because the dropped link is unidirectional.
 * To solve this problem the edge-direction rule has to be violated: If a mote x will drop an edge to mote y according to the original
 * l-ktc rule, the mote y will drop the edge to mote x as well. This can be done solely on local information without any error
 * because both motes will calculate the same triangle information. So mote y can drop the edge as well because it is able to calculate
 * that mote x will correctly drop the edge according to the l-ktc ruleset.
 */
#if COMPONENT_TOPOLOGYCONTROL == COMPONENT_TOPOLOGYCONTROL_LKTC

#if COMPONENT_NETWORKSTACK_MODE != COMPONENT_NETWORKSTACK_MODE_SINK
#error l-ktc implementation is designed for network configurations sending to a sink
#endif

#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

static uint8_t attribute_type;
static networkaddr_t sink;

void _hopcount_update();
int8_t _hopcount_get(networkaddr_t *node);

PROCESS(component_topologycontrol, "topologycontrol: lktc");
PROCESS_THREAD(component_topologycontrol, ev, data) {
	PROCESS_BEGIN();

#if COMPONENT_NETWORKSTACK == COMPONENT_NETWORKSTACK_IPV6
	networkaddr_fromstring(&sink, COMPONENT_APPLICATION_DATACOLLECTION_SINK_IPV6);
#endif
#if COMPONENT_NETWORKSTACK == COMPONENT_NETWORKSTACK_RIME
	networkaddr_fromstring(&sink, COMPONENT_APPLICATION_DATACOLLECTION_SINK_RIME);
#endif

	BOOT_COMPONENT_WAIT(component_topologycontrol);

	attribute_type = uniqueid_assign();

	static struct etimer etimer_hopcount;
	etimer_set(&etimer_hopcount, CLOCK_SECOND * COMPONENT_TOPOLOGYCONTROL_LKTC_INTERVALHOPCOUNTUPDATE);

	static struct etimer etimer_ktc;
	etimer_set(&etimer_ktc, random(CLOCK_SECOND * COMPONENT_TOPOLOGYCONTROL_LKTC_INTERVAL_MIN, CLOCK_SECOND * COMPONENT_TOPOLOGYCONTROL_LKTC_INTERVAL_MAX));

	while(1) {
		PROCESS_WAIT_EVENT();

		if(etimer_expired(&etimer_hopcount)) {
			etimer_reset(&etimer_hopcount);

			_hopcount_update();
		}

		if(etimer_expired(&etimer_ktc)) {
			etimer_set(&etimer_ktc, random(CLOCK_SECOND * COMPONENT_TOPOLOGYCONTROL_LKTC_INTERVAL_MIN, CLOCK_SECOND * COMPONENT_TOPOLOGYCONTROL_LKTC_INTERVAL_MAX));

			// with a large neighbourhood the algorithm may take a looong time, so stop the watchdog
			// that the node is not rebooted because the CPU thinks there's an endless loop running
			watchdog_stop();

			// find triangles and drop edges with l-ktc criteria
			neighbor_t *onehop;
			for(onehop = list_head(components_neighbordiscovery_neighbors()); onehop != NULL; onehop = list_item_next(onehop)) {
				if(networkaddr_equal(onehop->node1, networkaddr_node_addr())) {
					neighbor_t *nexthop = neighbors_find_triangle(components_neighbordiscovery_neighbors(), onehop, lktc_criteria);
					if(nexthop != NULL) {
						components_network_ignoredlinks_add(onehop->node2);
					}
				}
			}

			watchdog_start();
		}
	}

	PROCESS_END();
}

void _hopcount_update() {
	// remove actual hopcount
	neighbor_attribute_t *attribute;
	for(attribute = list_head(components_neighbordiscovery_attributes_all()); attribute != NULL; attribute = list_item_next(attribute)) {
		if(networkaddr_equal(attribute->node, networkaddr_node_addr()) && attribute->type == attribute_type) {
			components_neighbordiscovery_attributes_remove(attribute);
			break;
		}
	}

	// set new hopcount based on neighborhood and routing
	if(networkaddr_equal(&sink, networkaddr_node_addr())) {
		int8_t myhopcount = 0;
		components_neighbordiscovery_attributes_add(networkaddr_node_addr(), attribute_type, sizeof(myhopcount), &myhopcount);
	} else {
		if(components_network_nexthops_basestation() == NULL)
			return;

		neighbor_attribute_t *attribute;
		for(attribute = list_head(components_neighbordiscovery_attributes_all()); attribute != NULL; attribute = list_item_next(attribute)) {
			if(networkaddr_equal(attribute->node, components_network_nexthops_basestation()) && attribute->type == attribute_type) {
				int8_t myhopcount = ((int8_t) *attribute->data) + 1;
				components_neighbordiscovery_attributes_add(networkaddr_node_addr(), attribute_type, sizeof(myhopcount), &myhopcount);
			}
		}
	}
}

int8_t _hopcount_get(networkaddr_t *node) {
	neighbor_attribute_t *attribute;
	for(attribute = list_head(components_neighbordiscovery_attributes_all()); attribute != NULL; attribute = list_item_next(attribute)) {
		if(networkaddr_equal(attribute->node, node) && attribute->type == attribute_type)
			return ((int8_t) *attribute->data);
	}

	return -1;
}

int lktc_criteria(const neighbor_t *directhop, const neighbor_t *onehop, const neighbor_t *twohop) {
	if(!aktc_criteria_rssi(directhop, onehop, twohop))
		return 0;

	int8_t hopcount_self = _hopcount_get(networkaddr_node_addr());
	int8_t hopcount_newnexthop = _hopcount_get(onehop->node2);
	int8_t hopcount_destination = _hopcount_get(directhop->node2);

#if DEBUG
	PRINTF("DEBUG: [topologycontrol-lktc] possible triangle:\n");
	PRINTF("DEBUG: [topologycontrol-lktc] * directhop ");
	PRINTF("node1[%s, %d hops] -> ", networkaddr2string_buffered(directhop->node1), hopcount_self);
	PRINTF("node2[%s, %d hops]\n", networkaddr2string_buffered(directhop->node2), hopcount_destination);

	PRINTF("DEBUG: [topologycontrol-lktc] * onehop ");
	PRINTF("node1[%s, %d hops] -> ", networkaddr2string_buffered(onehop->node1), hopcount_self);
	PRINTF("node2[%s, %d hops]\n", networkaddr2string_buffered(onehop->node2), hopcount_newnexthop);

	PRINTF("DEBUG: [topologycontrol-lktc] * twohop ");
	PRINTF("node1[%s, %d hops] -> ", networkaddr2string_buffered(twohop->node1), hopcount_newnexthop);
	PRINTF("node2[%s, %d hops]\n", networkaddr2string_buffered(twohop->node2), hopcount_destination);
#endif

	// one member of the triangle is still in an invalid state
	if(hopcount_self == -1 || hopcount_newnexthop == -1|| hopcount_destination == -1)
		goto skip;

	// option 1: check if i would drop the edge
	float increase_factor = (hopcount_self == 0) ? 1 : (((float) hopcount_newnexthop + 1) / ((float) hopcount_self));
	if(hopcount_destination <= hopcount_self && increase_factor <= COMPONENT_TOPOLOGYCONTROL_LKTC_STRETCHFACTOR) {
		goto drop;
	}

	// option 2: check if the other mote would drop the edge
	float increase_factor_inverse = (hopcount_destination == 0) ? 1 : (((float) hopcount_newnexthop + 1) / ((float) hopcount_destination));
	if(hopcount_destination > hopcount_self && increase_factor_inverse <= COMPONENT_TOPOLOGYCONTROL_LKTC_STRETCHFACTOR) {
		goto drop;
	}

	skip:
		PRINTF("DEBUG: [topologycontrol-lktc] * => criteria not fulfilled\n");
		return 0;

	drop:
		PRINTF("DEBUG: [topologycontrol-lktc] * => criteria fulfilled\n");
		return 1;
}

#endif
