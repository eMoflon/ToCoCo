#ifndef __NETWORK_COMMON_H_
#define __NETWORK_COMMON_H_

#include <stdbool.h>

#include "../../app-conf.h"
#include "../../lib/buffer.h"
#include "../../lib/networkaddr.h"

#ifndef COMPONENT_NETWORK_PACKETSUBSCRIPTIONS_MEMORY
#define COMPONENT_NETWORK_PACKETSUBSCRIPTIONS_MEMORY 15
#endif

#ifndef COMPONENT_NETWORK_NEXTHOPS_CLOCKTICKS_TIME
#define COMPONENT_NETWORK_NEXTHOPS_CLOCKTICKS_TIME 60
#endif

#ifndef COMPONENT_NETWORK_NEXTHOPS_CLOCKTICKS_REQUIRED
#define COMPONENT_NETWORK_NEXTHOPS_CLOCKTICKS_REQUIRED 3
#endif

#ifndef COMPONENT_NETWORK_NEXTHOPS_CLOCKTICKS_CIRCULARBUFFER
#define COMPONENT_NETWORK_NEXTHOPS_CLOCKTICKS_CIRCULARBUFFER 9
#endif

/**
 * Initialize
 *
 * Initializes internal structures
 */
void network_common_init(void (*ignoredlink_notify)(bool added, networkaddr_t *address));

/**
 * Registers a subscription for a specific messagetype
 *
 * Registers a subscription for a specific messagetype which network_abstract_subscriber_publish()
 * can use for publishing those messages.
 */
void network_common_subscribe(uint8_t messagetype, void (*callback)(const networkaddr_t *source, buffer_t *data, int8_t rssi));

/**
 * Publishes an received message.
 *
 * Publishes an received message to all subscribers which have an subscription for
 * the messagetype saved in the first byte(s) of the messagedata
 */
void network_common_publish(buffer_t *data, const networkaddr_t *source, int8_t rssi);

/**
 * Publishes information about a linklocal send operation
 */
void network_common_linklocalsend_publish(const networkaddr_t *destination, int8_t txpower, int8_t num_tx, bool received);

#endif /* __NETWORK_COMMON_H_ */
