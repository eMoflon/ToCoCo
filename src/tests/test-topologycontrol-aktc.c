#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "contiki.h"

#include "../components/topologycontrol/topologycontrol-aktc.h"
#include "../lib/components.h"
#include "../lib/neighbors.h"
#include "../lib/networkaddr.h"
#include "../lib/unit-test.h"
#if COMPONENT_TOPOLOGYCONTROL == COMPONENT_TOPOLOGYCONTROL_AKTC
// TODO test link aggregation
UNIT_TEST_START(topologycontrol_aktc)

// initialize neighbor structures
networkaddr_t destination, newnexthop;
networkaddr_fill_random(&destination);
networkaddr_fill_random(&newnexthop);
neighbor_t directhop, onehop, twohop;
directhop.node1 = networkaddr_node_addr();
directhop.node2 = &destination;
onehop.node1    = networkaddr_node_addr();
onehop.node2    = &newnexthop;
twohop.node1    = &newnexthop;
twohop.node2    = &destination;

// if all rules match -> criteria is true
directhop.rssi_node1_to_node2 = directhop.rssi_node2_to_node1 = -1 * COMPONENT_TOPOLOGYCONTROL_AKTC_K /* to be longer: */- 1;
onehop.rssi_node1_to_node2 = onehop.rssi_node2_to_node1 = -1;
twohop.rssi_node1_to_node2 = twohop.rssi_node2_to_node1 = -1;
UNIT_TEST_ASSERT(aktc_criteria_rssi(&directhop, &onehop, &twohop) == 1);

// if one rssi is unknown but all other rules apply -> criteria fails
int8_t *rssi_values[6] = {
	&directhop.rssi_node1_to_node2, &directhop.rssi_node2_to_node1,
	&onehop.rssi_node1_to_node2, &onehop.rssi_node2_to_node1,
	&twohop.rssi_node1_to_node2, &twohop.rssi_node2_to_node1
};
int i;
for(i = 0; i < 6; i++) {
	directhop.rssi_node1_to_node2 = directhop.rssi_node2_to_node1 = -1 * COMPONENT_TOPOLOGYCONTROL_AKTC_K /* to be longer: */- 1;
	onehop.rssi_node1_to_node2 = onehop.rssi_node2_to_node1 = -1;
	twohop.rssi_node1_to_node2 = twohop.rssi_node2_to_node1 = -1;
	*rssi_values[i] = COMPONENT_RADIO_RSSIUNKNOWN;
	UNIT_TEST_ASSERT(aktc_criteria_rssi(&directhop, &onehop, &twohop) == 0);
}

// ktc criteria does not match if directhop is shorter than onehop
directhop.rssi_node1_to_node2 = directhop.rssi_node2_to_node1 = -1;
onehop.rssi_node1_to_node2 = onehop.rssi_node2_to_node1 = -1 * COMPONENT_TOPOLOGYCONTROL_AKTC_K /* to be longer: */- 1;
twohop.rssi_node1_to_node2 = twohop.rssi_node2_to_node1 = -1;
UNIT_TEST_ASSERT(aktc_criteria_rssi(&directhop, &onehop, &twohop) == 0);

// ktc criteria does not match if directhop is shorter than twohop
directhop.rssi_node1_to_node2 = directhop.rssi_node2_to_node1 = -1;
onehop.rssi_node1_to_node2 = onehop.rssi_node2_to_node1 = -1;
twohop.rssi_node1_to_node2 = twohop.rssi_node2_to_node1 = -1 * COMPONENT_TOPOLOGYCONTROL_AKTC_K /* to be longer: */- 1;
UNIT_TEST_ASSERT(aktc_criteria_rssi(&directhop, &onehop, &twohop) == 0);

UNIT_TEST_END()
#endif
