#ifndef __NEIGHBORDISCOVERY_TWOHOP_BROADCAST_H_
#define __NEIGHBORDISCOVERY_TWOHOP_BROADCAST_H_

#include <stdint.h>

#include "../../app-conf.h"
#include "../../lib/buffer.h"
#include "../../lib/networkaddr.h"

#ifndef NEIGHBORDISCOVERY_TWOHOPBROADCAST_INTERVALBROADCAST_MIN
#define NEIGHBORDISCOVERY_TWOHOPBROADCAST_INTERVALBROADCAST_MIN 60
#endif

#ifndef NEIGHBORDISCOVERY_TWOHOPBROADCAST_INTERVALBROADCAST_MAX
#define NEIGHBORDISCOVERY_TWOHOPBROADCAST_INTERVALBROADCAST_MAX 90
#endif

#ifndef NEIGHBORDISCOVERY_TWOHOPBROADCAST_DECAYINTERVAL
#define NEIGHBORDISCOVERY_TWOHOPBROADCAST_DECAYINTERVAL 2000
#endif

#ifndef NEIGHBORDISCOVERY_TWOHOPBROADCAST_LINKTTL
#define NEIGHBORDISCOVERY_TWOHOPBROADCAST_LINKTTL 5
#endif

void twohop_broadcast_init();

void twohop_broadcast_broadcastpacket_fill(buffer_t  *data);

void twohop_broadcast_broadcastpacket_handle(const networkaddr_t *sent_by, buffer_t *data, int8_t rssi);

void twohop_broadcast_decay_links();

list_t twohop_broadcast_neighbors_get_all();
process_event_t Neighbor_discovered;
#if COMPONENT_TOPOLOGYCONTROL == COMPONENT_TOPOLOGYCONTROL_PKTC

	void updateneighborinfo(const networkaddr_t * node,int8_t gvalue);
#endif

#endif /* __NEIGHBORDISCOVERY_TWOHOP_BROADCAST_H_ */
