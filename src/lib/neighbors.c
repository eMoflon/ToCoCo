#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "lib/list.h"

#include "neighbors.h"
#include "components.h"

uint8_t neighbors_count_all(const list_t neighbors) {
	return list_length(neighbors);
}

uint8_t neighbors_count_onehop(const list_t neighbors) {
	uint8_t count = 0;
	neighbor_t *item, *head = list_head(neighbors);
	for (item = head; item != NULL; item = list_item_next(item)) {
		if(networkaddr_equal(networkaddr_node_addr(), item->node1))
			count++;
	}

	return count;
}

uint8_t neighbors_count_twohop(const list_t neighbors) {
	return neighbors_count_all(neighbors) - neighbors_count_onehop(neighbors);
}

uint8_t neighbors_is_onehop(const list_t neighbors, const networkaddr_t *node) {
	neighbor_t *item;
	for (item = list_head(neighbors); item != NULL; item = list_item_next(item)) {
		if(networkaddr_equal(item->node1, networkaddr_node_addr()) && networkaddr_equal(item->node2, node))
			return 1;
	}

	return 0;
}

neighbor_t *neighbors_find_onehop_entry(const list_t neighbors, const networkaddr_t *node1, const networkaddr_t *node2) {
	neighbor_t *item, *head = list_head(neighbors);
	for (item = head; item != NULL; item = list_item_next(item)) {
		if(networkaddr_equal(node1, item->node1) && networkaddr_equal(node2, item->node2))
			return item;
	}

	return NULL;
}

neighbor_t *neighbors_find_twohop_entry(const list_t neighbors, const networkaddr_t *node1, const networkaddr_t *node2) {
	neighbor_t *item, *head = list_head(neighbors);
	for (item = head; item != NULL; item = list_item_next(item)) {
		int direct  = networkaddr_equal(node1, item->node1) && networkaddr_equal(node2, item->node2);
		int reverse = networkaddr_equal(node1, item->node2) && networkaddr_equal(node2, item->node1);
		if(direct || reverse)
			return item;
	}

	return NULL;
}

neighbor_t *neighbors_find_triangle(const list_t neighbors, const neighbor_t *neighbor, int (*triangle_is_ok)(const neighbor_t *directhop, const neighbor_t *onehop, const neighbor_t *twohop)) {
	neighbor_t *onehop, *twohop;

	// search a different onehop + twohop entry to destination
	for(onehop = list_head(neighbors); onehop != NULL; onehop = list_item_next(onehop)) {
		if(!networkaddr_equal(onehop->node1, neighbor->node1))
			continue;
		if(networkaddr_equal(onehop->node2, neighbor->node2))
			continue;

		// for a triangle we need a twohop entry connecting onehop to directhop
		for(twohop = list_head(neighbors); twohop != NULL; twohop = list_item_next(twohop)) {
			int direct  = networkaddr_equal(onehop->node2, twohop->node1) && networkaddr_equal(twohop->node2, neighbor->node2);
			int reverse = networkaddr_equal(onehop->node2, twohop->node2) && networkaddr_equal(twohop->node1, neighbor->node2);
			if(!direct && !reverse)
				continue;

			// create twohop entry which ensures that node1-node2 relationship is not inverse
			// WARNING: the pointer to the addresses is simply copied and not allocated, but this is ok for this local part
			neighbor_t twohop_ordered;
			if(networkaddr_equal(neighbor->node2, twohop->node2)) {
				memcpy(&twohop_ordered, twohop, sizeof(neighbor_t));
			} else {
				twohop_ordered.node1 = twohop->node2;
				twohop_ordered.node2 = twohop->node1;
#if COMPONENT_TOPOLOGYCONTROL != COMPONENT_TOPOLOGYCONTROL_PKTC
				twohop_ordered.rssi_node1_to_node2 = twohop->rssi_node2_to_node1;
				twohop_ordered.rssi_node2_to_node1 = twohop->rssi_node1_to_node2;
#else
				twohop_ordered.g_node1_to_node2 = twohop->g_node2_to_node1;
				twohop_ordered.g_node2_to_node1 = twohop->g_node1_to_node2;
#endif
				twohop_ordered.ttl_node1_to_node2 = twohop->ttl_node2_to_node1;
				twohop_ordered.ttl_node2_to_node1 = twohop->ttl_node1_to_node2;
			}

			if(triangle_is_ok(neighbor, onehop, &twohop_ordered))
				return onehop;
		}
	}

	return NULL;
}
