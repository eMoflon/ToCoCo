#ifndef __COMPONENTS_H_
#define __COMPONENTS_H_

#include <stdbool.h>
#include "contiki.h"
#include "lib/list.h"

#include "buffer.h"
#include "networkaddr.h"

// import COMPONENT_RADIO_TXPOWER_MIN AND COMPONENT_RADIO_TXPOWER_MAX
#include "../components/radio/radio-cc2420.h"
#include "../components/radio/radio-cc2520.h"
#include "../components/radio/radio-cooja.h"

#define COMPONENT_RADIO_RSSIUNKNOWN -127
#define COMPONENT_POWERCONTROL_G_UNKNOWN -1
#define COMPONENTS_NETWORK_TRANSMISSION_LINKLOCAL_BROADCAST 1
#define COMPONENTS_NETWORK_TRANSMISSION_LINKLOCAL_UNICAST   2
#define COMPONENTS_NETWORK_TRANSMISSION_ROUTING_UNICAST     3

#ifndef COMPONENTS_NEIGHBORDISCOVERY_NEIGHBORS_MEMORY
#define COMPONENTS_NEIGHBORDISCOVERY_NEIGHBORS_MEMORY 50
#endif

#ifndef COMPONENTS_NEIGHBORDISCOVERY_NEIGHBORATTRIBUTE_MEMORY
#define COMPONENTS_NEIGHBORDISCOVERY_NEIGHBORATTRIBUTE_MEMORY 20
#endif

#ifndef COMPONENTS_NEIGHBORDISCOVERY_NEIGHBORATTRIBUTE_MAXDATALENGTH
#define COMPONENTS_NEIGHBORDISCOVERY_NEIGHBORATTRIBUTE_MAXDATALENGTH 5
#endif

#ifndef COMPONENT_NETWORKSTACK_NEXTHOPS_MEMORY
#define COMPONENT_NETWORKSTACK_NEXTHOPS_MEMORY 30
#endif

#ifndef COMPONENT_NETWORKSTACK_IGNOREDLINKS_MEMORY
#define COMPONENT_NETWORKSTACK_IGNOREDLINKS_MEMORY 30
#endif

/**
 * Nexthop address of the currently used networkstack
 */
typedef struct nexthop {
	struct nexthop *next;
	networkaddr_t *address;
} nexthop_t;

/**
 * A physical link to an address which will be ignored
 */
typedef struct ignored_link {
	struct ignored_link *next;
	networkaddr_t *address;
} ignored_link_t;


typedef struct neighbor_attribute {
	struct neighbor_attribute* next;
	networkaddr_t *node;
	uint8_t type;
	uint8_t length;
	uint8_t data[COMPONENTS_NEIGHBORDISCOVERY_NEIGHBORATTRIBUTE_MAXDATALENGTH];
} neighbor_attribute_t;

/**
 * Neighbordiscovery neighbors
 *
 * All neighbors the neighbordiscovery finds within a 2-hop
 * neighborhoud.
 *
 * The return value is of type list_t(neighbor_t)
 *
 * \note do not modify the data, it may be an internal structure
 * of the neighbordiscovery algorithm
 */
list_t components_neighbordiscovery_neighbors();

/**
 * Neighbordiscovery node attributes
 *
 * All attributes of neighbors the neighbordiscovery received within
 * a 2-hop neighborhoud including all attributes of the node the software
 * is running on.
 *
 * The return value is of type list_t(neighbor_attribute_t)
 *
 * \note do not modify the data, it may be an internal structure
 * of the neighbordiscovery algorithm
 */
list_t components_neighbordiscovery_attributes_all();

/**
 * Add an attribute to neighbor discovery
 *
 * \note You should use uniqueid_assign() to assign a new type for yourself to prevent type cluttering
 * when you manually assign them and an type is reused.
 */
void components_neighbordiscovery_attributes_add(const networkaddr_t *node, uint8_t type, uint8_t length, const void *data);

/**
 * Remove an attribute for the actual node from neighbor discovery
 */
void components_neighbordiscovery_attributes_remove(neighbor_attribute_t *attribute);

/**
 * Routing next hops
 *
 * All nodes which are nexthops of the used routing algorithm.
 *
 * \note do not modify the data, it may be an internal structure
 * of the networkstack
 */
list_t components_network_nexthops_all();

/**
 * Next hop to basestation
 *
 * The next hop if sending packets to the base station.
 */
networkaddr_t* components_network_nexthops_basestation();

/**
 * Request send buffer
 *
 * Request a buffer to send data
 *
 * \note There is only one sendbuffer. You should request the buffer and send the data without
 * yielding control to other proccesses as they may use the send buffer too.
 */
buffer_t *components_network_packet_sendbuffer();

/**
 * Sends a packet
 *
 * Sends a packet with the actual compiled network stack
 *
 * \param transmission_type One of the NETWORK_TRANSMISSION_* constants
 * \param destination       The destination address or NULL when broadcasting messages
 * \param txpower           The txpower to send the message or -1 if the network stack is allowed to choose the best value
 * \param messagetype       The messagetype to use for this message and which has to be used with components_packet_subscribe()
 *
 * \note The network stack does only guarantee a typower for the first hop, there's no guarantee for further hops
 * while every network stack may handle this different.
 *
 * \note You should use uniqueid_assign() to assign a new unique messagetype for yourself to prevent messagetype cluttering
 * when you manually assign them and an messagetype is reused.
 */
void components_network_packet_send(uint8_t transmission_type, const networkaddr_t *destination, int8_t txpower, uint8_t messagetype, const buffer_t *data);

/**
 * Registers a callback function for receiving packets
 *
 * Callback function is called everytime a packet is received with
 * the specified messagetype.
 */
void components_network_packet_subscribe(uint8_t messagetype, void (*callback)(const networkaddr_t *source, buffer_t *data, int8_t rssi));

/**
 * Registers a callback function for receiving linklocal send information
 *
 * Callback function is called everytime a packet is sent from this mote to any other mote with the status of the packet.
 *
 * \note You will receive information not only for application layer packets, you will even be called for packets of the
 * underlying network topology (e.g. route discovery).
 * \note does only work with ContikiMac RDC and CSMA mac
 */
void components_network_linklocalsend_subscribe(void (*callback)(const networkaddr_t *destination, int8_t txpower, int8_t num_tx, bool received));

/**
 * Adds an ignored link
 *
 * Adds an ignored link to the network stack which will do it's best to pretend that a link to this
 * address does not exist physically effectively hiding it from all upper layers.
 *
 * Note: the actual mote will drop all received messages (unicast and broadcast) from the address
 * simply pretending that the message has never been received (but without any retransmit overhead
 * for the sender). But any message which is sent from this mote to the address (e.g. broadcast) is
 * received by the address, because it's emulated that the actual is mote not aware of the other mote
 * but this information is not reciprocal.
 * So be aware of this behaviour. Rime's routing algorithms for example will find a one-hop link from
 * this mote to the address but can not transmit it to this mote so a route passing this address as
 * nexthop will never be created. You have to add the ignored link at the other mote too.
 */
void components_network_ignoredlinks_add(networkaddr_t *address);

/**
 * Removes an ignored link
 *
 * Removes an ignored link so the network stack will receive again all one-hop packets from the address.
 */
void components_network_ignoredlinks_remove(networkaddr_t *address);

/**
 * List of all ignored links
 *
 * Returns a list with all ignored links.
 */
list_t components_network_ignoredlinks_all();

/**
 * Last RSSI.
 *
 * The RSSI value of the last received packet by the radio.
 *
 * \note The RSSI value has to be in range [-100, 0] with -100dBM indicating a node which is far away and 0dBM to be very near.
 * If no RSSI information is present, the radio module is allowed to return COMPONENT_RADIO_RSSIUNKNOWN
 */
int8_t components_radio_lastrssi();

/**
 * Sets transmission power
 *
 * Sets transmission power of radio to a specified level.
 */
void components_radio_txpower_set(int8_t txpower);

/**
 * Gets transmission power
 *
 * Gets actual transmission power of radio
 */
int8_t components_radio_txpower_get();

/**
 * Transmission power to reach a specific node
 *
 * The maximum needed transmission power to reach the requested destination.
 * If the destination is unknown to the power control algorithm the power
 * "-1" will be returned.
 *
 * \note All nodes the maximum power needs to to be calcuted for, are defined by
 * components_nexthops().
 */
int8_t components_powercontrol_destinationtxpower(const networkaddr_t *destination);

#endif /* __COMPONENTS_H_ */
