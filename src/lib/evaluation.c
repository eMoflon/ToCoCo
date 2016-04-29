#include <stdio.h>
#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include "../app-conf.h"
#include "evaluation.h"
#include "lib/components.h"
#include "lib/neighbors.h"
#include "lib/networkaddr.h"
#include "lib/powerstats.h"
#if NETSTACK_CONF_WITH_IPV6
#include "net/rpl/rpl.h"
#endif

static void print_nexthops() {
	nexthop_t *nexthop;

	printf("[evaluation] nexthops: ");
	for(nexthop = list_head(component_network_nexthops_all()); nexthop != NULL; nexthop = list_item_next(nexthop)) {
		printf("%s", networkaddr2string_buffered(nexthop->address));
		if(nexthop->next != NULL)
			printf(", ");
	}
	printf("\n");
}

static void print_routing() {
	printf("[evaluation] routing: ");
#if NETSTACK_CONF_WITH_IPV6
	uip_ds6_route_t *route;
	rpl_dag_t *dag = rpl_get_any_dag();

	for(route = uip_ds6_route_head(); route != NULL; route = uip_ds6_route_next(route)) {
		printf("%s~>", networkaddr2string_buffered(uip_ds6_route_nexthop(route)));
		printf("%s",  networkaddr2string_buffered(&route->ipaddr));
		if(route->next!= NULL || dag->preferred_parent != NULL)
			printf(", ");
	}

	if(dag->preferred_parent != NULL) {
		printf("%s~>", networkaddr2string_buffered(rpl_get_parent_ipaddr(dag->preferred_parent)));
		printf("%s, ", networkaddr2string_buffered(rpl_get_parent_ipaddr(dag->preferred_parent)));
		printf("%s~>$OTHER", networkaddr2string_buffered(rpl_get_parent_ipaddr(dag->preferred_parent)));
	}
#else
	struct route_entry *route;
	int i;

	for(i = 0; i < route_num(); i++) {
		// there may be multiple routes to a single destination, by looking up the route
		// we will definitely get the route to the destination rime will use
		route = route_lookup(&route_get(i)->dest);

		printf("%s~>", networkaddr2string_buffered(&route->nexthop));
		printf("%s", networkaddr2string_buffered(&route->dest));
		if(i < route_num() - 1)
			printf(", ");
	}

#endif
	printf("\n");
}

static void print_neighborhood() {
	neighbor_t *neighbor;

	printf("[evaluation] neighborhood: ");
	for(neighbor = list_head(component_neighbordiscovery_neighbors()); neighbor != NULL; neighbor = list_item_next(neighbor)) {
		printf("%s->", networkaddr2string_buffered(neighbor->node1));
		printf("%s", networkaddr2string_buffered(neighbor->node2));
		printf("[%d,%d]", neighbor->weight_node1_to_node2, neighbor->weight_node2_to_node1);
		if(neighbor->next != NULL)
			printf(", ");
	}
	printf("\n");
}

static void print_ignoredlinks() {
	ignored_link_t *link;

	printf("[evaluation] ignoredlinks: ");
	for(link = list_head(component_network_ignoredlinks_all()); link != NULL; link = list_item_next(link)) {
		printf("%s", networkaddr2string_buffered(link->address));
		if(link->next != NULL)
			printf(", ");
	}
	printf("\n");
}

static void print_powercontrol() {
	nexthop_t *nexthop;

	printf("[evaluation] powercontrol: ");
	for(nexthop = list_head(component_network_nexthops_all()); nexthop != NULL; nexthop = list_item_next(nexthop)) {
		printf("%s=%d", networkaddr2string_buffered(nexthop->address), component_powercontrol_destinationtxpower(nexthop->address));
		if(nexthop->next != NULL)
			printf(", ");
	}
	printf("\n");
}

void evaluation_print() {
	powerstats_print("evaluation");
	print_nexthops();
	print_routing();
	print_neighborhood();
	print_ignoredlinks();
	print_powercontrol();
}
