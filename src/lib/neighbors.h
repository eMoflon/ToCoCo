#ifndef __TWOHOP_NEIGHBORS_H_
#define __TWOHOP_NEIGHBORS_H_

#include <stdlib.h>

#include "networkaddr.h"
#include "components.h"

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
