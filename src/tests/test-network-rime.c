#include <stdlib.h>
#include <stdio.h>
#include "contiki.h"
#include "contiki-net.h"
#include "net/rime/route.h"

#include "../components/network/network-rime.h"
#include "../lib/unit-test.h"
#include "../lib/components.h"

UNIT_TEST_START(network_rime)

#if COMPONENT_NETWORKSTACK == COMPONENT_NETWORKSTACK_RIME

network_rime_init();

networkaddr_t oldentry, hop1_1, hop1_2, hop2, route_substitution;
networkaddr_fill_random(&oldentry);
networkaddr_fill_random(&hop1_1);
networkaddr_fill_random(&hop1_2);
networkaddr_fill_random(&hop2);
networkaddr_fill_random(&route_substitution);

// next hops are initially empty
route_flush_all();
UNIT_TEST_ASSERT(list_length(components_network_nexthops_all()) == 0);

#if COMPONENT_NETWORKSTACK_MODE == COMPONENT_NETWORKSTACK_MODE_MESH
	// "old" entries are deleted from list
	route_add(&oldentry, &oldentry, 0, 0);
	UNIT_TEST_ASSERT(list_length(components_network_nexthops_all()) == 1);
	route_flush_all();
	UNIT_TEST_ASSERT(list_length(components_network_nexthops_all()) == 0);

	// nexthops ignored by route substitution are not part of list
	route_add(&hop1_1, &hop1_1, 0, 0);
	route_add(&route_substitution, &route_substitution, 0, 0);
	UNIT_TEST_ASSERT(list_length(components_network_nexthops_all()) == 2);
	components_network_routesubstitution_add(&route_substitution, &hop1_1);
	UNIT_TEST_ASSERT(list_length(components_network_nexthops_all()) == 1);
	components_network_routesubstitution_remove(&route_substitution, &hop1_1);
	UNIT_TEST_ASSERT(list_length(components_network_nexthops_all()) == 2);
	route_flush_all();

	// only nexthops are used from routing table and only once
	route_add(&hop2, &hop1_1, 0, 0);
	route_add(&hop2, &hop1_2, 0, 0);
	route_add(&hop1_1, &hop1_2, 0, 0);
	route_add(&hop1_2, &hop1_1, 0, 0);
	UNIT_TEST_ASSERT(list_length(components_network_nexthops_all()) == 2);
	nexthop_t *entry1 = list_head(components_network_nexthops_all());
	nexthop_t *entry2 = list_item_next(list_head(components_network_nexthops_all()));
	UNIT_TEST_ASSERT(networkaddr_equal(&hop1_1, entry1->address));
	UNIT_TEST_ASSERT(networkaddr_equal(&hop1_2, entry2->address));

	// route-substitutions can be substituted again
	while(list_length(components_network_routesubstitution_all()) > 0) {
		route_substitution_t *substitution = list_head(components_network_routesubstitution_all());
		components_network_routesubstitution_remove(substitution->change_from, substitution->change_to);
	}
	route_add(&hop2, &oldentry, 0, 0);
	UNIT_TEST_ASSERT(networkaddr_equal(&route_lookup(&hop2)->nexthop, &oldentry));
	UNIT_TEST_ASSERT(networkaddr_equal(&route_get(0)->nexthop, &oldentry));
	components_network_routesubstitution_add(&oldentry, &hop1_1);
	UNIT_TEST_ASSERT(networkaddr_equal(&route_lookup(&hop2)->nexthop, &hop1_1));
	UNIT_TEST_ASSERT(networkaddr_equal(&route_get(0)->nexthop, &hop1_1));
	components_network_routesubstitution_add(&hop1_1, &hop1_2);
	UNIT_TEST_ASSERT(networkaddr_equal(&route_lookup(&hop2)->nexthop, &hop1_2));
	UNIT_TEST_ASSERT(networkaddr_equal(&route_get(0)->nexthop, &hop1_2));
#else
	printf("[test in mesh mode please ;)] ");
#endif

#else

UNIT_TEST_SKIP();

#endif

UNIT_TEST_END()
