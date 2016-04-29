#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "contiki.h"
#include "contiki-net.h"
#include "contiki-lib.h"
#include "net/rime/broadcast.h"
#include "lib/crc16.h"

#include "network-rime.h"
#include "network-common.h"
#include "../../app-conf.h"
#include "../../lib/boot.h"
#include "../../lib/buffer.h"
#include "../../lib/components.h"
#include "../../lib/networkaddr.h"

#if COMPONENT_NETWORK == COMPONENT_NETWORK_RIME

#define DEBUG 0

static buffer_t sendbuffer;

static void network_ignoredlink_notify(bool added, networkaddr_t *address);
static void network_linklocalsend(const networkaddr_t *destination, int8_t txpower, int8_t num_tx, bool received);

static void modified_rime_driver_init(void);
static void modified_rime_driver_input(void);
const struct network_driver modified_rime_driver = {
  "Modified-Rime",
  modified_rime_driver_init,
  modified_rime_driver_input
};

void network_rime_init() {
	network_common_init(network_ignoredlink_notify);

	component_network_linklocalsend_subscribe(network_linklocalsend);
}

static struct broadcast_conn bc;
static void broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from);
static struct broadcast_callbacks bc_callback = {broadcast_recv};

static struct mesh_conn mesh;
static void mesh_recv(struct mesh_conn *c, const linkaddr_t *from, uint8_t hops);
const static struct mesh_callbacks mesh_callbacks = {mesh_recv};

static struct unicast_conn uc;
static void unicast_recv(struct unicast_conn *c, const linkaddr_t *from);
const static struct unicast_callbacks uc_callbacks = {unicast_recv};

PROCESS(component_network, "network: rime");
PROCESS_THREAD(component_network, ev, data) {
	PROCESS_BEGIN();

	network_rime_init();

	// init radio module txpower
	component_radio_txpower_set(COMPONENT_RADIO_TXPOWER_MAX);

	mesh_open(&mesh, 100, &mesh_callbacks);
	broadcast_open(&bc, 200, &bc_callback);
	unicast_open(&uc, 300, &uc_callbacks);

	BOOT_COMPONENT_WAIT(component_network);

	PROCESS_END();
}

void print_received_packet(const linkaddr_t *from, char *transmissiontype) {
#if DEBUG
	// memcpy and no buffer operation while the messagetype in the buffer is needed later
	uint8_t messagetype;
	memcpy(&messagetype, packetbuf_dataptr() + sizeof(uint16_t), sizeof(messagetype));

	printf(
		"DEBUG: [network-rime] RECEIVE type=%s source=%s messagetype=%d datalength=%d\n",
		transmissiontype,
		networkaddr2string_buffered(from),
		messagetype,
		packetbuf_datalen() - sizeof(uint16_t) - sizeof(messagetype)
	);
#endif
}

static bool is_crc_ok(buffer_t *data) {
	uint16_t checksum_exepected = buffer_read_uint16t(data);
	uint16_t checksum_data = crc16_data(((uint8_t *) data->bufferptr) + data->offset, data->length - sizeof(uint16_t), 0xFFFF);

	if(checksum_exepected != checksum_data) {
#if DEBUG
		printf("ERROR[network-rime]: drop received packet because of CRC-16 error: %d != %d\n", checksum_exepected, checksum_data);
#endif
		return false;
	}

	return true;
}

static void mesh_recv(struct mesh_conn *c, const linkaddr_t *from, uint8_t hops) {
	print_received_packet(from, "routing");
	buffer_t data = {packetbuf_dataptr(), packetbuf_datalen(), 0};
	if(is_crc_ok(&data))
		network_common_publish(&data, from, component_radio_lastrssi());
}

static void broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from) {
	print_received_packet(from, "broadcast");
	buffer_t data = {packetbuf_dataptr(), packetbuf_datalen(), 0};
	if(is_crc_ok(&data))
		network_common_publish(&data, from, component_radio_lastrssi());
}

static void unicast_recv(struct unicast_conn *c, const linkaddr_t *from) {
	print_received_packet(from, "unicast");
	buffer_t data = {packetbuf_dataptr(), packetbuf_datalen(), 0};
	if(is_crc_ok(&data))
		network_common_publish(&data, from, component_radio_lastrssi());
}

buffer_t *component_network_packet_sendbuffer() {
	static uint8_t buffer[PACKETBUF_SIZE];

	sendbuffer.bufferptr = buffer;
	sendbuffer.offset = sizeof(uint16_t) + sizeof(uint8_t); // reserve space to save crc16-hash and messagetype
	sendbuffer.length = sizeof(buffer);

	return &sendbuffer;
}

void component_network_packet_send(transmissiontype_t type, uint8_t messagetype, const networkaddr_t *destination, const buffer_t *data, int8_t txpower, int8_t maxtransmits) {
	// (1) save messagetype in buffer, (2) calculate crc16 checksum of messagetype + application data, (3) prepend checksum at beginning
	memcpy(((uint8_t *) data->bufferptr) + sizeof(uint16_t), &messagetype, sizeof(uint8_t));
	uint16_t checksum = crc16_data(((uint8_t *) data->bufferptr) + sizeof(uint16_t), data->offset - sizeof(uint16_t), 0xFFFF);
	memcpy(data->bufferptr, &checksum, sizeof(uint16_t));

	// save data with headers to rime packet buffer
	packetbuf_clear();
	packetbuf_copyfrom(data->bufferptr, data->offset);
	if(txpower != -1)
		packetbuf_set_attr(PACKETBUF_ATTR_RADIO_TXPOWER, txpower);
	if(maxtransmits != -1)
		packetbuf_set_attr(PACKETBUF_ATTR_MAX_MAC_TRANSMISSIONS, maxtransmits);

#if DEBUG
	printf("DEBUG: [network-rime] SEND ");
	if(type == COMPONENT_NETWORK_TRANSMISSION_LINKLOCAL_BROADCAST) {
		printf("type=broadcast ");
	} else if(type == COMPONENT_NETWORK_TRANSMISSION_LINKLOCAL_UNICAST) {
		printf("type=unicast destination=%s ", networkaddr2string_buffered(destination));
	} else if(type == COMPONENT_NETWORK_TRANSMISSION_ROUTING_UNICAST) {
		printf("type=routing destination=%s ", networkaddr2string_buffered(destination));
	}
	printf("txpower=%d messagetype=%d datalength=%d\n", txpower, messagetype, data->offset - sizeof(uint16_t) - sizeof(uint8_t));
#endif

	if(type == COMPONENT_NETWORK_TRANSMISSION_LINKLOCAL_BROADCAST) {
		broadcast_send(&bc);
	} else if(type == COMPONENT_NETWORK_TRANSMISSION_LINKLOCAL_UNICAST) {
		unicast_send(&uc, destination);
	} else if(type == COMPONENT_NETWORK_TRANSMISSION_ROUTING_UNICAST) {
		mesh_send(&mesh, destination);
	}
}

networkaddr_t *component_network_nexthops_basestation() {
	struct route_entry *route = route_lookup(component_network_address_basestation());
	if(route == NULL)
		return NULL;

	nexthop_t *nexthop;
	for(nexthop = list_head(component_network_nexthops_all()); nexthop != NULL; nexthop = list_item_next(nexthop)) {
		if(networkaddr_equal(&route->nexthop, nexthop->address))
			return nexthop->address;
	}

	return NULL;
}

static void network_ignoredlink_notify(bool added, networkaddr_t *address) {
	if(added) {
		int i;
		for(i = 0; i < route_num(); i++) {
			struct route_entry *route = route_get(i);
			if(route != NULL && networkaddr_equal(address, (networkaddr_t *) &route->nexthop)) {
				// route discovery will have to search a new route to the destination
				route_remove(route);
				i--;
			}
		}
	}
}

static void network_linklocalsend(const networkaddr_t *destination, int8_t txpower, int8_t num_tx, bool received) {
	// if a network send operation by the nexthop succeeded we can refresh all routes with this nexthop
	// because the nexthop is reachable => broken route entries will be removed after some time in
	// contrast to standard contiki rime networking
	if(received) {
		struct route_entry *entry;
		for(entry = route_get(0); entry != NULL; entry = list_item_next(entry)) {
			if(networkaddr_equal(&entry->nexthop, destination)) {
				route_refresh(entry);
			}
		}
	}
}

static void modified_rime_driver_init(void) {
	rime_driver.init();
}

static void modified_rime_driver_input(void) {
	ignored_link_t *link;
	for(link = list_head(component_network_ignoredlinks_all()); link != NULL; link = list_item_next(link)) {
		if(networkaddr_equal(link->address, (const networkaddr_t *) packetbuf_addr(PACKETBUF_ADDR_SENDER))) {
			// we drop the message by not calling the rime driver to handle the message
			return;
		}
	}

	rime_driver.input();
}

#endif
