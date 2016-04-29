/*
 * powercontrol-ptpc.h
 *
 *  Created on: Jul 10, 2015
 *      Author: Amit
 */

#ifndef POWERCONTROL_PTPC_H_
#define POWERCONTROL_PTPC_H_
#include "../../lib/networkaddr.h"
#ifndef EDGE_WEIGHT
#define EDGE_WEIGHT 2 // 1 for Power and 2 for g = PRR/power
#endif
#ifndef BOOT_STRAPPHASE
#define BOOT_STRAPPHASE 1
#endif
process_event_t Bootstrap_finished;
/**
 * It returns the edge weight for the particular neighbor
 */
int8_t getGvalue(const networkaddr_t * node);
/**
 * This function tells if the concerned neighbor has its corresponding edge weights stored in power control module.
 */
uint8_t neighborexists(const networkaddr_t * node);
/**
 * This function deletes the memory chunks taken by the particular node
 */
void deleteneighbor(const networkaddr_t * node);
#endif /* POWERCONTROL_PTPC_H_ */

