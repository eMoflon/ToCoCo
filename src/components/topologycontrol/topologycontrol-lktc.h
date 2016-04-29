#ifndef __TOPOLOGYCONTROL_LKTC_H_
#define __TOPOLOGYCONTROL_LKTC_H_

#include "../../app-conf.h"
#include "../../lib/neighbors.h"

#ifndef COMPONENT_TOPOLOGYCONTROL_LKTC_STRETCHFACTOR
#define COMPONENT_TOPOLOGYCONTROL_LKTC_STRETCHFACTOR 1.4
#endif

#ifndef COMPONENT_TOPOLOGYCONTROL_LKTC_INTERVALHOPCOUNTUPDATE
#define COMPONENT_TOPOLOGYCONTROL_LKTC_INTERVALHOPCOUNTUPDATE 60
#endif

#ifndef COMPONENT_TOPOLOGYCONTROL_LKTC_INTERVAL_MIN
#define COMPONENT_TOPOLOGYCONTROL_LKTC_INTERVAL_MIN 270
#endif

#ifndef COMPONENT_TOPOLOGYCONTROL_LKTC_INTERVAL_MAX
#define COMPONENT_TOPOLOGYCONTROL_LKTC_INTERVAL_MAX 330
#endif

int lktc_criteria(const neighbor_t *directhop, const neighbor_t *onehop, const neighbor_t *twohop);

#endif /* __TOPOLOGYCONTROL_LKTC_H_ */
