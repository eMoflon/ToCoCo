#include <stdlib.h>
#include <stdio.h>
#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/rpl/rpl.h"

#include "../components/network/network-ipv6.h"
#include "../lib/unit-test.h"
#include "../lib/components.h"

#if COMPONENT_NETWORKSTACK == COMPONENT_NETWORKSTACK_IPV6
static void add_route(networkaddr_t* destination, networkaddr_t* nexthop) {
	uip_lladdr_t *lladr = malloc(sizeof(uip_lladdr_t));
	lladr->addr[0] = destination->u8[15 - 0];
	lladr->addr[1] = destination->u8[15 - 1];

	uip_ds6_nbr_add(nexthop, lladr, 0, NBR_REACHABLE);
	uip_ds6_route_add(destination, sizeof(networkaddr_t) << 3, nexthop); // length is shifted >> 3 and used for memcmp of ip addresses

}
#endif

UNIT_TEST_START(network_ipv6)

#if COMPONENT_NETWORKSTACK == COMPONENT_NETWORKSTACK_IPV6

network_ipv6_init();

/**
 * test nexthops:
 */

networkaddr_t oldentry, parent, hop1_1, hop1_2, hop2_1, hop2_2, route_substitution;
networkaddr_fill_random(&oldentry);
networkaddr_fill_random(&parent);
networkaddr_fill_random(&hop1_1);
networkaddr_fill_random(&hop1_2);
networkaddr_fill_random(&hop2_1);
networkaddr_fill_random(&hop2_2);
networkaddr_fill_random(&route_substitution);

// next hops are initially empty
while(uip_ds6_route_num_routes() > 0)
	uip_ds6_route_rm(uip_ds6_route_head());
UNIT_TEST_ASSERT(list_length(components_network_nexthops_all()) == 0);

// "old" entries are deleted from list
add_route(&oldentry, &oldentry);
UNIT_TEST_ASSERT(list_length(components_network_nexthops_all()) == 1);
uip_ds6_route_rm_by_nexthop(&oldentry);
UNIT_TEST_ASSERT(list_length(components_network_nexthops_all()) == 0);

// nexthops ignored by route substitution are not part of list
/*add_route(&route_substitution, &route_substitution);
UNIT_TEST_ASSERT(list_length(components_network_nexthops_all()) == 1);
components_network_routesubstitution_add(&route_substitution, &route_substitution);
UNIT_TEST_ASSERT(list_length(components_network_nexthops_all()) == 0);
components_network_routesubstitution_remove(&route_substitution, &route_substitution);
UNIT_TEST_ASSERT(list_length(components_network_nexthops_all()) == 1);
components_network_routesubstitution_add(&route_substitution, &route_substitution); // add it so next tests will not interfere*/

// only nexthops and parent are used from routing table and only once
// TODO was not able to insert a specific parent to RPL
add_route(&hop1_1, &hop1_1);
add_route(&hop2_1, &hop1_1);
add_route(&hop1_2, &hop1_2);
add_route(&hop2_2, &hop1_2);
UNIT_TEST_ASSERT(list_length(components_network_nexthops_all()) == 2);
nexthop_t *entry1 = list_head(components_network_nexthops_all());
nexthop_t *entry2 = list_item_next(entry1);
UNIT_TEST_ASSERT(networkaddr_equal(&hop1_1, entry1->address));
UNIT_TEST_ASSERT(networkaddr_equal(&hop1_2, entry2->address));

#else

UNIT_TEST_SKIP();

#endif

UNIT_TEST_END()
