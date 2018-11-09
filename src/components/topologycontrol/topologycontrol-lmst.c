#include <stdbool.h>
#include <stdio.h>
#include "contiki.h"
#include "contiki-lib.h"
#include "dev/watchdog.h"

#include "topologycontrol-lmst.h"
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

// Returns the larger of the two network addresses of the given link 'edge' : neighbor_t*
#define NETWORKADDR_MAX(edge) (networkaddr_cmp(edge->node1, edge->node2) < 0 ? edge->node2 : edge->node1)

typedef struct node {
  // Necessary for usage in lists
	struct neighbor *next;
  // The corresponding node in the tree/neighborhood
	networkaddr_t *address;
  // The link that caused the connection of address to the tree
	neighbor_t *edge;
} node_t;

void _lmst_nodelist_reconstruct();
bool _lmst_nodelist_hasunconnected();
bool _lmst_nodelist_isconnected(networkaddr_t *address);
void _lmst_nodelist_connect(neighbor_t *edge);

#if COMPONENT_TOPOLOGYCONTROL == COMPONENT_TOPOLOGYCONTROL_LMST

MEMB(memb_nodelist, node_t, COMPONENT_NETWORK_NEXTHOPS_MEMORY);
LIST(list_nodelist);

PROCESS(component_topologycontrol, "topologycontrol: lmst");
PROCESS_THREAD(component_topologycontrol, ev, data) {
	PROCESS_BEGIN();

	memb_init(&memb_nodelist);
	list_init(list_nodelist);

	BOOT_COMPONENT_WAIT(component_topologycontrol);

	static struct etimer waittime;
	etimer_set(&waittime, CLOCK_SECOND * COMPONENT_TOPOLOGYCONTROL_LMST_UPDATEINTERVAL);
	while(1) {
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&waittime));
		etimer_reset(&waittime);

		// with a large neighbourhood the algorithm may take a looong time, so stop the watchdog
		// that the node is not rebooted because the CPU thinks there's an endless loop running
		watchdog_stop();

    
    // Detailed evaluation output - begin
    unsigned long start=RTIMER_NOW();
    printf("[topologycontrol]: STATUS: Run\n");
    // Detailed evaluation output - end

		// Prim algorithm
		// (connects edge of every node except the graph's root node - the node this code is running on
		_lmst_nodelist_reconstruct();
		while(_lmst_nodelist_hasunconnected()) {

			// find best edge according to PRIM and LMST algorithm
			neighbor_t *edge_current;
      neighbor_t *edge_best = NULL;
			for(edge_current = list_head(component_neighbordiscovery_neighbors()); edge_current != NULL; edge_current = list_item_next(edge_current)) {

        // Only node1 is connected
				bool node1_connected = _lmst_nodelist_isconnected(edge_current->node1) && !_lmst_nodelist_isconnected(edge_current->node2);
        // Only node2 is connected
				bool node2_connected = _lmst_nodelist_isconnected(edge_current->node2) && !_lmst_nodelist_isconnected(edge_current->node1);

				if(node1_connected ^ node2_connected) {

          // Option 1: no edge_best candidate found until now
          if (edge_best == NULL) {
            edge_best = edge_current;
          } else {

            int weight_actual = MAX(edge_current->weight_node1_to_node2, edge_current->weight_node2_to_node1);
            int weight_best = MAX(edge_best->weight_node1_to_node2, edge_best->weight_node2_to_node1);

            // Option 2: weight(edge_current) < weight(edge_best)
					  bool criteria1 = weight_actual < weight_best;

            // Option 3: weight(edge_current) < weight(edge_best)					
            bool criteria2 = weight_actual == weight_best && networkaddr_cmp(NETWORKADDR_MAX(edge_current), NETWORKADDR_MAX(edge_best)) < 0;

					  if(criteria1 || criteria2)
						  edge_best = edge_current;
          }
				}

			}

			if(edge_best == NULL) {
				printf("ERROR[topologycontrol-lmst]: no edge for spanning tree found\n");
				watchdog_reboot(); // we would end in an endless loop
			}

			_lmst_nodelist_connect(edge_best);
		}

#if DEBUG
		node_t *item_node;
		PRINTF("DEBUG: [topologycontrol-lmst] spanning tree: ");
		for(item_node = list_head(list_nodelist); item_node != NULL; item_node = list_item_next(item_node)) {
			if(networkaddr_equal(item_node->address, networkaddr_node_addr()))
				continue;

			PRINTF("%s<->", networkaddr2string_buffered(item_node->edge->node1));
			PRINTF("%s", networkaddr2string_buffered(item_node->edge->node2));
			PRINTF("%s", item_node->next == NULL ? "\n" : ", ");
		}
#endif

		/* Ignore the address associated with an ItemNode if 
     * (i)   the address is not self
     * (ii)  neither node1 nor node2 of the edge that connects the ItemNode to the tree are self 
     */
		node_t *node;
		for(node = list_head(list_nodelist); node != NULL; node = list_item_next(node)) {
			if(!networkaddr_equal(node->address,     networkaddr_node_addr()) && 
         !networkaddr_equal(node->edge->node1, networkaddr_node_addr()) && 
         !networkaddr_equal(node->edge->node2, networkaddr_node_addr())) {
				component_network_ignoredlinks_add(node->address);
			}
		}

    // Detailed evaluation output - begin
    unsigned long finish=RTIMER_NOW();
    unsigned long ticks= finish > start ? finish-start : start-finish;
    unsigned long runtimeMillis = (ticks * 1000)/ RTIMER_SECOND;
    printf("[topologycontrol]: Ticks: %lu\n", ticks);
    printf("[topologycontrol]: RTIMER_SECOND: %u ticks per second\n", RTIMER_SECOND);
    printf("[topologycontrol]: TIME: %lu ms.\n", runtimeMillis);
    // Detailed evaluation output - end

		watchdog_start();

		// LMST is only run once because if links have been ignored they are no longer available in the neighbor discovery
		// and hence a spanning tree can no longer be built
		PRINTF("DEBUG: [topologycontrol-lmst] LMST algorithm is finished and will run no more\n");
		break;
	}

	PROCESS_END();
}

/*
 * Cleans the list list_nodelist of ItemNodes and 
 * reinitializes it with one ItemNode for each node that is part of a neighbor_t in the neighborhood
 */
void _lmst_nodelist_reconstruct() {
	// clean list
	while(list_length(list_nodelist) > 0) {
		node_t *item = list_pop(list_nodelist);
		networkaddr_reference_free(item->address);
		memb_free(&memb_nodelist, item);
	}

	// add all nodes to list
	neighbor_t *item_neighbor;
	for(item_neighbor = list_head(component_neighbordiscovery_neighbors()); item_neighbor != NULL; item_neighbor = list_item_next(item_neighbor)) {
		node_t *item_node;
		bool found;

		// check for node1
		found = false;
		for(item_node = list_head(list_nodelist); item_node != NULL; item_node = list_item_next(item_node)) {
			if(networkaddr_equal(item_neighbor->node1, item_node->address)) {
				found = true;
				break;
			}
		}
		if(!found) {
			if((item_node = memb_alloc(&memb_nodelist)) == NULL) {
				printf("ERROR[topologycontrol-lmst]: nodelist is full\n");
			} else {
				item_node->address = networkaddr_reference_alloc(item_neighbor->node1);
				item_node->edge = NULL;
				list_add(list_nodelist, item_node);
			}
		}

		// check for node2
		found = false;
		for(item_node = list_head(list_nodelist); item_node != NULL; item_node = list_item_next(item_node)) {
			if(networkaddr_equal(item_neighbor->node2, item_node->address)) {
				found = true;
				break;
			}
		}
		if(!found) {
			if((item_node = memb_alloc(&memb_nodelist)) == NULL) {
				printf("ERROR[topologycontrol-lmst]: nodelist is full\n");
			} else {
				item_node->address = networkaddr_reference_alloc(item_neighbor->node2);
				item_node->edge = NULL;
				list_add(list_nodelist, item_node);
			}
		}
	}
}

/*
 * Returns true if there is an ItemNode with a missing edge
 * Special case: The ItemNode for the self-node does not have an edge
 */
bool _lmst_nodelist_hasunconnected() {
	node_t *item_node;
	for(item_node = list_head(list_nodelist); item_node != NULL; item_node = list_item_next(item_node)) {
		if(item_node->edge == NULL && !networkaddr_equal(networkaddr_node_addr(), item_node->address))
			return true;
	}

	return false;
}

/*
 * Checks whether the edge of the ItemNode corresponding to the given address is already set.
 * Special case: The ItemNode for the self-node does not have an edge
 */
bool _lmst_nodelist_isconnected(networkaddr_t *address) {
	node_t *item_node;
	for(item_node = list_head(list_nodelist); item_node != NULL; item_node = list_item_next(item_node)) {
		if(networkaddr_equal(item_node->address, address) && 
        (item_node->edge != NULL || networkaddr_equal(networkaddr_node_addr(), item_node->address))
      )
		return true;
	}

	return false;
}

/*
 * Adds the given edge to the ItemNodes that correspond to node1 and node2 of the edge
 * Special case: The edge of the ItemNode of the self-node is never set.
 */
void _lmst_nodelist_connect(neighbor_t *edge) {
	node_t *item_node;
	for(item_node = list_head(list_nodelist); item_node != NULL; item_node = list_item_next(item_node)) {
		
    // Only one of the following two cases applies because the given edge was selected such that one of its nodes is already connected
    if(networkaddr_equal(item_node->address, edge->node1) && 
        item_node->edge == NULL && 
        !networkaddr_equal(networkaddr_node_addr(), edge->node1))
			item_node->edge = edge;
		
    if(networkaddr_equal(item_node->address, edge->node2) && 
        item_node->edge == NULL && 
        !networkaddr_equal(networkaddr_node_addr(), edge->node2))
			item_node->edge = edge;

	}
}

#endif
