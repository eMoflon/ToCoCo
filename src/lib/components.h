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

#define COMPONENT_NEIGHBORDISCOVERY_WEIGHTUNKNOWN -1

typedef enum transmissiontype {
	COMPONENT_NETWORK_TRANSMISSION_LINKLOCAL_BROADCAST,
	COMPONENT_NETWORK_TRANSMISSION_LINKLOCAL_UNICAST,
	COMPONENT_NETWORK_TRANSMISSION_ROUTING_UNICAST
} transmissiontype_t;

#ifndef COMPONENT_NEIGHBORDISCOVERY_NEIGHBORS_MEMORY
#define COMPONENT_NEIGHBORDISCOVERY_NEIGHBORS_MEMORY 50
#endif

#ifndef COMPONENT_NEIGHBORDISCOVERY_NEIGHBORATTRIBUTE_MEMORY
#define COMPONENT_NEIGHBORDISCOVERY_NEIGHBORATTRIBUTE_MEMORY 20
#endif

#ifndef COMPONENT_NEIGHBORDISCOVERY_NEIGHBORATTRIBUTE_MAXDATALENGTH
#define COMPONENT_NEIGHBORDISCOVERY_NEIGHBORATTRIBUTE_MAXDATALENGTH 5
#endif

#ifndef COMPONENT_NETWORK_NEXTHOPS_MEMORY
#define COMPONENT_NETWORK_NEXTHOPS_MEMORY 30
#endif

#ifndef COMPONENT_NETWORK_IGNOREDLINKS_MEMORY
#define COMPONENT_NETWORK_IGNOREDLINKS_MEMORY 30
#endif

/**
 * Nexthop address of the currently used networkstack
 */
typedef struct nexthop {
	struct nexthop *next;
	networkaddr_t *address;
	uint16_t used_clockticks;
} nexthop_t;

/**
 * A physical link to an address which will be ignored
 */
typedef struct ignored_link {
	struct ignored_link *next;
	networkaddr_t *address;
} ignored_link_t;

/**
 * Visible Neighbor in topology
 */
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

	/**
	 * edge weight between these nodes
	 *
	 * from 0 to 100 with a higher value indicating a worse weight
	 * -1 means an unknown edge weight
	 */
	int8_t weight_node1_to_node2;
	int8_t weight_node2_to_node1;

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
list_t component_neighbordiscovery_neighbors();

/**
 * Basestation
 *
 * Every network has a defined base station but does not have to operate in a source-to-sink network mode.
 * You can use the network in a mesh-mode and the base station address will not have any effect. It's
 * there for some components which need to operate in a source-to-sink communication mode.
 */
networkaddr_t *component_network_address_basestation();

/**
 * Basestation next hops
 *
 * The nexthop of the network stack which will be used if you sent any data to the basestation.
 *
 * \note the nexthop may not be live because the network layer has first to detect the nexthop to the
 * basestation by looking at this motes packet destinations
 */
networkaddr_t *component_network_nexthops_basestation();

/**
 * Routing next hops
 *
 * All nodes which are nexthops of this mote. The nexthops list may not be live because the network
 * layer may take some time to detect the nexthops this mote is really sending packets to.
 *
 * \note do not modify the data, it may be an internal structure
 * of the networkstack
 */
list_t component_network_nexthops_all();

/**
 * Request send buffer
 *
 * Request a buffer to send data
 *
 * \note There is only one sendbuffer. You should request the buffer and send the data without
 * yielding control to other proccesses as they may use the send buffer too.
 */
buffer_t *component_network_packet_sendbuffer();

/**
 * Sends a packet
 *
 * Sends a packet with the actual compiled network stack
 *
 * \param type        One of the COMPONENT_NETWORK_TRANSMISSION_* values
 * \param destination The destination address or NULL when broadcasting messages
 * \param txpower     The txpower to send the message or -1 if the network stack is allowed to choose the best value
 * \param messagetype The messagetype to use for this message and which has to be used with component_packet_subscribe()
 *
 * \note The network stack does only guarantee a typower for the first hop, there's no guarantee for further hops
 * while every network stack may handle this different.
 *
 * \note You should use uniqueid_assign() to assign a new unique messagetype for yourself to prevent messagetype cluttering
 * when you manually assign them and an messagetype is reused.
 */
void component_network_packet_send(transmissiontype_t type, uint8_t messagetype, const networkaddr_t *destination, const buffer_t *data, int8_t txpower, int8_t maxtransmits);

/**
 * Registers a callback function for receiving packets
 *
 * Callback function is called everytime a packet is received with
 * the specified messagetype.
 */
void component_network_packet_subscribe(uint8_t messagetype, void (*callback)(const networkaddr_t *source, buffer_t *data, int8_t rssi));

/**
 * Registers a callback function for receiving linklocal send information
 *
 * Callback function is called everytime a packet is sent from this mote to any other mote with the status of the packet.
 *
 * \note You will receive information not only for application layer packets, you will even be called for packets of the
 * underlying network topology (e.g. route discovery).
 * \note does only work with ContikiMac RDC and CSMA mac
 */
void component_network_linklocalsend_subscribe(void (*callback)(const networkaddr_t *destination, int8_t txpower, int8_t num_tx, bool received));

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
void component_network_ignoredlinks_add(networkaddr_t *address);

/**
 * Removes an ignored link
 *
 * Removes an ignored link so the network stack will receive again all one-hop packets from the address.
 */
void component_network_ignoredlinks_remove(networkaddr_t *address);

/**
 * List of all ignored links
 *
 * Returns a list with all ignored links.
 */
list_t component_network_ignoredlinks_all();

/**
 * Last RSSI.
 *
 * The RSSI value of the last received packet by the radio.
 *
 * \note The RSSI value has to be in range [-100, 0] with -100dBM indicating a node which is far away and 0dBM to be very near.
 * If no RSSI information is present, the radio module is allowed to return COMPONENT_RADIO_RSSIUNKNOWN
 */
int8_t component_radio_lastrssi();

/**
 * Sets transmission power
 *
 * Sets transmission power of radio to a specified level.
 */
void component_radio_txpower_set(int8_t txpower);

/**
 * Gets transmission power
 *
 * Gets actual transmission power of radio
 */
int8_t component_radio_txpower_get();

/**
 * Transmission power to reach a specific node
 *
 * The maximum needed transmission power to reach the requested destination.
 * If the destination is unknown to the power control algorithm the power
 * "-1" will be returned.
 *
 * \note All nodes the maximum power needs to to be calcuted for, are defined by
 * component_nexthops().
 */
int8_t component_powercontrol_destinationtxpower(const networkaddr_t *destination);

#endif /* __COMPONENTS_H_ */
