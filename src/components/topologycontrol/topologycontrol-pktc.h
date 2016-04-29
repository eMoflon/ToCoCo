/*
 * topologycontrol-pktc.h
 *
 *  Created on: Aug 29, 2015
 *      Author: user
 */

#ifndef TOPOLOGYCONTROL_PKTC_H_
#define TOPOLOGYCONTROL_PKTC_H_
#include "../../lib/neighbors.h"
#ifndef COMPONENT_TOPOLOGYCONTROL_PKTC_INTERVAL_MIN
#define COMPONENT_TOPOLOGYCONTROL_PKTC_INTERVAL_MIN 700
#endif

#ifndef COMPONENT_TOPOLOGYCONTROL_PKTC_K
#define COMPONENT_TOPOLOGYCONTROL_PKTC_K 1.41
#endif

#ifndef COMPONENT_TOPOLOGYCONTROL_PKTC_INTERVAL_MAX
#define COMPONENT_TOPOLOGYCONTROL_PKTC_INTERVAL_MAX 760
#endif

int pktc_criteria_gvalue(const neighbor_t *directhop, const neighbor_t *onehop, const neighbor_t *twohop);
#endif /* TOPOLOGYCONTROL_PKTC_H_ */
