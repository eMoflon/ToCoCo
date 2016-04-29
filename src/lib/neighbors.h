#ifndef __TWOHOP_NEIGHBORS_H_
#define __TWOHOP_NEIGHBORS_H_

#include <stdlib.h>

#include "networkaddr.h"

typedef struct neighbor {
	/*
	 * element needed by contiki's list
	 */
	struct neighbor *next;

	/**
	 * a 1-hop neighbor or yourself
	 */
	networkaddr_t *node1;

	/**
	 * neighbor node of node1
	 */
	networkaddr_t *node2;
#if COMPONENT_TOPOLOGYCONTROL != COMPONENT_TOPOLOGYCONTROL_PKTC
	/**
	 * RSSI between these nodes
	 */
	int8_t rssi_node1_to_node2;
	int8_t rssi_node2_to_node1;
#else
	/**
	 * G = PRR/power between these nodes
	 */
	int8_t g_node1_to_node2;
	int8_t g_node2_to_node1;
#endif
	/**
	 * TTL flag for the edge.
	 *
	 * This value is initially NEIGHBOR_DISCOVERY_LINK_TTL and decremented occasionally.
	 * If the values reaches 0, the links is removed by the implementation
	 */
	uint8_t ttl_node1_to_node2;
	uint8_t ttl_node2_to_node1;
} neighbor_t;

/**
 * neighbors list is a param that unit tests can pass a different neigborslist for different implementations
 */

uint8_t neighbors_count_all(const list_t neighbors);

uint8_t neighbors_count_onehop(const list_t neighbors);

uint8_t neighbors_count_twohop(const list_t neighbors);

uint8_t neighbors_is_onehop(const list_t neighbors, const networkaddr_t *node);

neighbor_t *neighbors_find_onehop_entry(const list_t neighbors, const networkaddr_t *node1, const networkaddr_t *node2);

neighbor_t *neighbors_find_twohop_entry(const list_t neighbors, const networkaddr_t *node1, const networkaddr_t *node2);

/**
 * Find neighborhod rectangle
 *
 * Tries to find a triangle in the neighbor information for a given destination whith specified
 * evaluation function.
 *
 * \return pointer to replacement nexthop or NULL
 */
neighbor_t *neighbors_find_triangle(const list_t neighbors, const neighbor_t *neighbor, int (*triangle_is_ok)(const neighbor_t *directhop, const neighbor_t *onehop, const neighbor_t *twohop));

#endif /* __TWOHOP_NEIGHBORS_H_ */
