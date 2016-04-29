#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "contiki.h"
#if CONTIKI_REPLACEMENTS_LOCAL
#include "../../lib/contiki-replacements/packetbuf.h" // has to be loaded before original packetbuf.h
#endif
#include "contiki-net.h"
#include "contiki-lib.h"
#include "net/rpl/rpl.h"
#include "net/rpl/rpl-private.h"
#include "sys/node-id.h"

#include "network-ipv6.h"
#include "network-common.h"
#include "../../app-conf.h"
#include "../../lib/boot.h"
#include "../../lib/buffer.h"
#include "../../lib/components.h"
#include "../../lib/networkaddr.h"

#define DEBUG 0

#define UIP_IP_BUF ((struct uip_ip_hdr*) &uip_buf[UIP_LLH_LEN])

#if COMPONENT_NETWORKSTACK == COMPONENT_NETWORKSTACK_IPV6

MEMB(memb_nexthops, nexthop_t, COMPONENT_NETWORKSTACK_NEXTHOPS_MEMORY);
LIST(list_nexthops);

static buffer_t sendbuffer;

static struct uip_udp_conn *udp_conn;

void print_received_packet(uint8_t transmissiontype, const networkaddr_t *from, const buffer_t *data);
static void network_ignoredlink_notify(bool added, networkaddr_t *address);

void network_ipv6_init() {
	memb_init(&memb_nexthops);
	list_init(list_nexthops);

	network_common_init(network_ignoredlink_notify);
}

// All messages are sent with UDP either with a normal ip address that RPL takes care of the routing or with
// a link-local broadcast ip address.
// Messages are prepended with the desired transmission type (routing, link-local broadcast, link-local unicast)
// that the receive handler is aware of the handling it has to take. For link-local unicasts a destination
// ip-address is appended that every receiver of the message can lookup whether the message is intended for the node.
PROCESS(component_network, "network: ipv6");
PROCESS_THREAD(component_network, ev, data) {
	PROCESS_BEGIN();

	network_ipv6_init();

	// init radio module txpower
	components_radio_txpower_set(COMPONENT_RADIO_TXPOWER_MAX);

	// init RPL tree
	if(node_id == COMPONENT_NETWORKSTACK_IPV6_ROOTNODE) {
		uip_ipaddr_t ipaddr;
		uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 1);
		uip_ds6_addr_add(&ipaddr, 0, ADDR_MANUAL);
		if(uip_ds6_addr_lookup(&ipaddr) != NULL) {
			rpl_dag_t *dag;
			dag = rpl_set_root(RPL_DEFAULT_INSTANCE, (uip_ip6addr_t*) &ipaddr);
			uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
			rpl_set_prefix(dag, &ipaddr, 64);
		} else {
			// failed creating rpl tree (but this never happens...)
		}
	} else {
		uip_ipaddr_t ipaddr;
		uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
		uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
		uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);
	}

	static struct etimer etimer_rplinit;
	etimer_set(&etimer_rplinit, CLOCK_SECOND * 60);
	PROCESS_WAIT_UNTIL(etimer_expired(&etimer_rplinit));

	printf("[network-ipv6] Routing IPv6 address %s\n", networkaddr2string_buffered(networkaddr_node_addr()));

	udp_conn = udp_new(NULL, UIP_HTONS(4242), NULL);
	udp_bind(udp_conn, UIP_HTONS(4242));

	BOOT_COMPONENT_WAIT(component_network);

	while(1) {
		PROCESS_WAIT_EVENT();

		if(ev == tcpip_event) {
			// check whether we have to drop the message because it has been received from an ignored local-link
			if(UIP_IP_BUF->srcipaddr.u16[0] == UIP_HTONS(0xfe80)) {
				bool ignored = false;

				ignored_link_t *item;
				for(item = list_head(components_network_ignoredlinks_all()); item != NULL; item = list_item_next(item)) {
					if(networkaddr_equal(&UIP_IP_BUF->srcipaddr, item->address))
						ignored = true;
				}

				if(ignored)
					continue; // simply drop message by not handling it :)
			}

			buffer_t data = {uip_appdata, uip_datalen(), 0};
			uint8_t transmissiontype = buffer_read_uint8t(&data);

			if(transmissiontype == COMPONENTS_NETWORK_TRANSMISSION_LINKLOCAL_UNICAST && UIP_IP_BUF->srcipaddr.u16[0] == UIP_HTONS(0xfe80)) {
				networkaddr_t destination = buffer_read_networkaddr(&data);

				// drop packet because it's an broadcast message emulated as link-local unicast
				// message and destination is another node's IP address
				if(!networkaddr_equal(&destination, networkaddr_node_addr()))
					continue;
			}

			// for broadcasts the ipv6 prefix will start with fe80 but this is a link-local address, so fake
			// address and pretend it has prefix "aaaa" that message was received from a routable addressable
			// ipv6 address
			uip_ip6addr_t source;
			networkaddr_copy(&source, &UIP_IP_BUF->srcipaddr);
			source.u16[0] = 0xaaaa;

			print_received_packet(transmissiontype, &source, &data);
			network_common_publish(&data, &source, components_radio_lastrssi());
		}
	}

	PROCESS_END();
}

buffer_t *components_network_packet_sendbuffer() {
	static uint8_t buffer[UIP_BUFSIZE - UIP_LLH_LEN - UIP_IPUDPH_LEN];

	sendbuffer.bufferptr = buffer;
	sendbuffer.offset = sizeof(uint8_t) + BUFFER_NETWORKADDR_SIZE + sizeof(uint8_t); // reserve space to save networktype, destination address and messagetype
	sendbuffer.length = sizeof(buffer);

	return &sendbuffer;
}

void components_network_packet_send(uint8_t transmission_type, const networkaddr_t *destination, int8_t txpower, uint8_t messagetype, const buffer_t* data) {
	nexthop_t *nexthop;
	bool destination_needed = (transmission_type == COMPONENTS_NETWORK_TRANSMISSION_LINKLOCAL_UNICAST);
	for(nexthop = list_head(components_network_nexthops_all()); nexthop != NULL; nexthop = list_item_next(nexthop)) {
		if(networkaddr_equal(destination, nexthop->address)) {
			destination_needed = false;
		}
	}

	// hack: we have to prepend the transmission type, maybe a network address and the message type. We pretend that the
	// provided buffer has never changed by manipulating the internal buffer the provided buffer is modified from and
	// changing the offset before returning from the function to the original value. So any calling code does think
	// that the const qualifier is not violated.
	size_t offset_calling = data->offset;
	size_t offset_sending = (destination_needed ? 0 : BUFFER_NETWORKADDR_SIZE);
	sendbuffer.offset = offset_sending;
	buffer_append_uint8t(&sendbuffer, transmission_type);
	if(destination_needed)
		buffer_append_networkaddr(&sendbuffer, destination);
	buffer_append_uint8t(&sendbuffer, messagetype);
	sendbuffer.offset = offset_calling;

	// set packet destination according to transmission type
	static uip_ipaddr_t udp_destination;
	uip_ipaddr_copy(&udp_destination, destination);
	if(transmission_type == COMPONENTS_NETWORK_TRANSMISSION_LINKLOCAL_BROADCAST || (transmission_type == COMPONENTS_NETWORK_TRANSMISSION_LINKLOCAL_UNICAST && destination_needed)) {
		uip_create_linklocal_allnodes_mcast(&udp_destination);
	}

#if DEBUG
	printf("DEBUG: [network-ipv6] SEND ");
	if(transmission_type == COMPONENTS_NETWORK_TRANSMISSION_LINKLOCAL_BROADCAST) {
		printf("type=broadcast ");
	} else if(transmission_type == COMPONENTS_NETWORK_TRANSMISSION_LINKLOCAL_UNICAST) {
		printf("type=unicast destination=%s ", networkaddr2string_buffered(destination));
	} else if(transmission_type == COMPONENTS_NETWORK_TRANSMISSION_ROUTING_UNICAST) {
		printf("type=routing destination=%s ", networkaddr2string_buffered(destination));
	}
	printf("txpower=%d messagetype=%d datalength=%d\n", txpower, messagetype, offset_calling - sizeof(uint8_t) - BUFFER_NETWORKADDR_SIZE - sizeof(uint8_t));
#endif

	// send packet with requested transmission power
	// note: if currently the txpower_reset timer is running the txpower is set to the highest value
	// of the actual requested transmission power and the requested transmission power the timer was
	// started for. When setting the txpower to a lower level the packet send by the actually yet
	// running timer would be sent with to less txpower and not been received by the destination
	packetbuf_clear();
	if(txpower != -1) {
		packetbuf_attr_clear_keeptxpoweronce();
		packetbuf_set_attr(PACKETBUF_ATTR_RADIO_TXPOWER, txpower);
	}
	uip_udp_packet_sendto(udp_conn, ((uint8_t *) data->bufferptr) + offset_sending, offset_calling - offset_sending, &udp_destination, UIP_HTONS(4242));
}

void components_network_packet_subscribe(uint8_t messagetype, void (*callback)(const networkaddr_t *source, buffer_t *data, int8_t rssi)) {
	network_common_subscribe(messagetype, callback);
}

static void _addNexthop(networkaddr_t *address) {
	if(address == NULL)
		return;

	// address to save
	uip_ip6addr_t substituted;
	networkaddr_copy(&substituted, address);
	substituted.u16[0] = 0xaaaa; // make sure it's globally routable, some entries may have an fe80::-prefix

	// check for duplicates
	bool found = false;
	nexthop_t *nexthop;
	for(nexthop = list_head(list_nexthops); nexthop != NULL; nexthop = list_item_next(nexthop))
		found |= networkaddr_equal(&substituted, nexthop->address);

	// add it :)
	if(!found) {
		nexthop_t *hop = memb_alloc(&memb_nexthops);
		if(hop != NULL) {
			hop->address = networkaddr_reference_alloc(&substituted);
			list_add(list_nexthops, hop);
		}
	}
}

list_t components_network_nexthops_all() {
	uip_ds6_route_t *route;
	rpl_dag_t *dag = rpl_get_any_dag();

	// step 1: clean list
	while(list_length(list_nexthops) > 0) {
		nexthop_t *item = list_head(list_nexthops);

		networkaddr_reference_free(item->address);
		list_remove(list_nexthops, item);
		memb_free(&memb_nexthops, item);
	}

	// step 2: add parent
	if(dag->preferred_parent != NULL)
		_addNexthop(rpl_get_parent_ipaddr(dag->preferred_parent));

	// step 3: add missing next hops
	for(route = uip_ds6_route_head(); route != NULL; route = uip_ds6_route_next(route))
		_addNexthop(uip_ds6_route_nexthop(route));

	return list_nexthops;
}

networkaddr_t* components_network_nexthops_basestation() {
	// rpl_get_parent_ipaddr(rpl_get_any_dag()->preferred_parent) can't be returned because it has an fe80::-prefix :(
	nexthop_t *nexthop;
	for(nexthop = list_head(components_network_nexthops_all()); nexthop != NULL; nexthop = list_item_next(nexthop)) {
		if(networkaddr_equal(nexthop->address, rpl_get_parent_ipaddr(rpl_get_any_dag()->preferred_parent)))
			return nexthop->address;
	}

	return NULL;
}

// NOTE 1: There are no routing tables for RPL. When routing with RPL the node sends a message the
//         RPL tree down if the node has the requested destination in it's children table which
//         contains ALL (even multi-hop) children of th enode. If the destination the message is
//         route upwards to the parent until any parent has the destination in it's children list.
// NOTE 2: There's no possibility in RPL to restrict the process of choosing the RPL children of
//         a node. The child is choosing the parent and the parent has simply to accept it.
//         So "removing" a route to a children simply removes the children from the possible
//         parents that this node never chooses this node for RPL upwards-routing. This node is
//         still considered for RPL downwards-routing. For effectively stopping sending any message
//         to the *nexthop, the *nexthop has also to "remove" this route.
static rpl_parent_t *(*original_best_parent)(rpl_parent_t *, rpl_parent_t *);
static rpl_parent_t *ignore_parent_bestparent(rpl_parent_t *p1, rpl_parent_t *p2) {
	int ignored_p1 = 0, ignored_p2 = 0;
	ignored_link_t *item;

	rpl_parent_t *best = original_best_parent(p1, p2), *best_without_ignoring = original_best_parent(p1, p2);
	for(item = list_head(components_network_ignoredlinks_all()); item != NULL; item = list_item_next(item)) {
		if(networkaddr_equal(item->address, rpl_get_parent_ipaddr(p1))) {
			ignored_p1 = 1;
			best = p2;
		}
		if(networkaddr_equal(item->address, rpl_get_parent_ipaddr(p2))) {
			ignored_p2 = 1;
			best = p1;
		}
	}

	// if both parents should be ignored the best one is selected, because if all possible parents are marked
	// to ignore RPL has anyway to choose a parent and the best one of the original selector function will be
	// the best possible choice. As well choosing a new parent in the worst case everytime would create a
	// large message transmitting overhead while routing table changes have to be announced.
	if(ignored_p1 && ignored_p2)
		best = best_without_ignoring;

	return best;
}
static void network_ignoredlink_notify(bool added, networkaddr_t *address) {
	// change internal parent selector function with wrapper-function ignoring certain parents
	if(original_best_parent == NULL) {
		rpl_dag_t *dag = rpl_get_any_dag();
		original_best_parent = dag->instance->of->best_parent;
		dag->instance->of->best_parent = ignore_parent_bestparent;

#if DEBUG
		printf("DEBUG: [network-ipv6] replaced RPL parent selector function\n");
#endif
	}

	// a very naive topology control implementation may remove all ignored links and then add all
	// again. When removing an ignored link a new RPL parent may be chosen updating the whole
	// RPL-tree up to the root with many messages send by all nodes. And if the now chosen parent
	// should be ignored again a few operations later the whole RPL tree has to be updated again.
	// If we don't instantly select a new RPL parent a naive implementation may work as described
	// and a new RPL parent may be chosen asynchronous by the RPL implementation at it's own
	// (every few seconds and on every received packet).
}

void print_received_packet(uint8_t transmissiontype, const networkaddr_t *from, const buffer_t *data) {
#if DEBUG
	uint8_t messagetype;
	memcpy(&messagetype, data->bufferptr + data->offset, sizeof(uint8_t));

	printf("DEBUG: [network-ipv6] RECEIVE ");
	if(transmissiontype == COMPONENTS_NETWORK_TRANSMISSION_LINKLOCAL_BROADCAST)
		printf("type=broadcast ");
	else if(transmissiontype == COMPONENTS_NETWORK_TRANSMISSION_LINKLOCAL_UNICAST)
		printf("type=unicast ");
	else if(transmissiontype == COMPONENTS_NETWORK_TRANSMISSION_ROUTING_UNICAST)
		printf("type=routing ");
	printf("source=%s messagetype=%d datalength=%d\n", networkaddr2string_buffered(from), messagetype, data->length - data->offset - sizeof(uint8_t));
#endif
}

#endif
