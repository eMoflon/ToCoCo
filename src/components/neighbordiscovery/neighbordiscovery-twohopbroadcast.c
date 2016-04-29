#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include "contiki.h"
#include "contiki-net.h"
#include "dev/watchdog.h"

#include "neighbordiscovery-twohopbroadcast.h"
#include "../../app-conf.h"
#include "../../lib/boot.h"
#include "../../lib/components.h"
#include "../../lib/neighbors.h"
#include "../../lib/uniqueid.h"
#include "../../lib/utilities.h"

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

MEMB(memb_neighbors, neighbor_t, COMPONENT_NEIGHBORDISCOVERY_NEIGHBORS_MEMORY);
LIST(list_neighbors);

void _twohop_broadcast_handle(const networkaddr_t *sent_by, buffer_t *data, int8_t rssi);
void _twohop_broadcast_print_neighborhood();
/**
 * message protocol:
 * +--------------------+--------------+
 * | uint8_t #neighbors | neighbordata |
 * +--------------------+--------------+
 *
 * neighbordata:
 * +--------------------+---------------------+---------------------------+
 * | networkaddr_t node | int8_t weight_to_me | int8_t weight_to_neighbor |
 * +--------------------+---------------------+---------------------------+
 */
#if COMPONENT_NEIGHBORDISCOVERY == COMPONENT_NEIGHBORDISCOVERY_TWOHOPBROADCAST

static uint8_t messagetype;

static uint16_t broadcasts = 0;

static void _twohop_broadcast_receive(const networkaddr_t *source, buffer_t *data, int8_t rssi) {
	// with a large neighbourhood the parsing and updating of the neighbor information may take
	// a looong time, so stop the watchdog that the node is not rebooted because the CPU thinks
	// there's an endless loop running
	watchdog_stop();

	_twohop_broadcast_handle(source, data, rssi);

	watchdog_start();

	_twohop_broadcast_print_neighborhood();
}

PROCESS(component_neighbordiscovery, "neighbordiscovery: twohop-broadcast");
PROCESS_THREAD(component_neighbordiscovery, ev, data) {
	PROCESS_BEGIN();

	twohop_broadcast_init();

	BOOT_COMPONENT_WAIT(component_neighbordiscovery);

	messagetype = uniqueid_assign();
	component_network_packet_subscribe(messagetype, _twohop_broadcast_receive);

	static struct etimer et_broadcast;
	static struct etimer et_decay;
	etimer_set(&et_broadcast, random(CLOCK_SECOND * COMPONENT_NEIGHBORDISCOVERY_TWOHOPBROADCAST_INTERVALBROADCAST_MIN, CLOCK_SECOND * COMPONENT_NEIGHBORDISCOVERY_TWOHOPBROADCAST_INTERVALBROADCAST_MAX));
	etimer_set(&et_decay, CLOCK_SECOND * COMPONENT_NEIGHBORDISCOVERY_TWOHOPBROADCAST_DECAYINTERVAL);
	while (1) {
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et_broadcast) || etimer_expired(&et_decay));

		// send broadcast
		if(etimer_expired(&et_broadcast)) {
			etimer_set(&et_broadcast, random(CLOCK_SECOND * COMPONENT_NEIGHBORDISCOVERY_TWOHOPBROADCAST_INTERVALBROADCAST_MIN, CLOCK_SECOND * COMPONENT_NEIGHBORDISCOVERY_TWOHOPBROADCAST_INTERVALBROADCAST_MAX));

			if(COMPONENT_NEIGHBORDISCOVERY_TWOHOPBROADCAST_MAXBROADCASTS == -1 || ++broadcasts <= COMPONENT_NEIGHBORDISCOVERY_TWOHOPBROADCAST_MAXBROADCASTS) {
				// send neighbourhood with maximum txpower so all neighbors can receive it
				PRINTF("DEBUG: [twohop-broadcast] send broadcast\n");
				buffer_t *data = component_network_packet_sendbuffer();
				twohop_broadcast_broadcastpacket_fill(data);
				component_network_packet_send(COMPONENT_NETWORK_TRANSMISSION_LINKLOCAL_BROADCAST, messagetype, NULL, data, COMPONENT_RADIO_TXPOWER_MAX, -1);
			}
		}

		// decay node links
		if(etimer_expired(&et_decay)) {
			etimer_reset(&et_decay);

			twohop_broadcast_decay_links();
		}
	}

	PROCESS_END();
}

#endif

void twohop_broadcast_init() {
	memb_init(&memb_neighbors);
	list_init(list_neighbors);
}

int8_t _twohop_broadcast_edgeweight(const networkaddr_t *sent_by, int8_t rssi) {
#if COMPONENT_NEIGHBORDISCOVERY_EDGEWEIGHT == COMPONENT_NEIGHBORDISCOVERY_EDGEWEIGHT_RSSI
	return rssi * -1;
#endif
#if COMPONENT_NEIGHBORDISCOVERY_EDGEWEIGHT == COMPONENT_NEIGHBORDISCOVERY_EDGEWEIGHT_DISTANCE
	int16_t pos_me_x = -1, pos_me_y = -1, pos_other_x = -1, pos_other_y = -1;

	networkaddr_t addr;
	int16_t pos_x = -1, pos_y = -1;
	uint8_t i = 0;
	char *ptr_parts = strtok(COMPONENT_NEIGHBORDISCOVERY_EDGEWEIGHT_DISTANCE_POSITIONS, "=|,");
	while(ptr_parts != NULL) {
		if(i % 3 == 0)
			networkaddr_fromstring(&addr, ptr_parts);
		if(i % 3 == 1)
			pos_x = atoi(ptr_parts);
		if(i % 3 == 2)
			pos_y = atoi(ptr_parts);

		if(i % 3 == 2 && networkaddr_equal(&addr, networkaddr_node_addr())) {
			pos_me_x = pos_x;
			pos_me_y = pos_y;
		}
		if(i % 3 == 2 && networkaddr_equal(&addr, sent_by)) {
			pos_other_x = pos_x;
			pos_other_y = pos_y;
		}

		ptr_parts = strtok(NULL, "=|,");
		i++;
	}

	if(pos_me_x == -1 || pos_me_y == -1) {
		printf("ERROR[twohop-broadcast]: no location information for %s\n", networkaddr2string_buffered(networkaddr_node_addr()));
		return COMPONENT_NEIGHBORDISCOVERY_WEIGHTUNKNOWN;
	}
	if(pos_other_x == -1 || pos_other_y == -1) {
		printf("ERROR[twohop-broadcast]: no location information for %s\n", networkaddr2string_buffered(sent_by));
		return COMPONENT_NEIGHBORDISCOVERY_WEIGHTUNKNOWN;
	}

	// euclidean distance (TODO make scaling factor distance-to-weight configurable)
	float x = pow((float) (pos_me_x) - (float) (pos_other_x), 2);
	float y = pow((float) (pos_me_y) - (float) (pos_other_y), 2);
	int8_t weight = (int8_t) (sqrt(x + y) / 5.0);

	return MAX(0, MIN(100, weight));
#endif
}

void twohop_broadcast_broadcastpacket_fill(buffer_t *data) {
	PRINTF("DEBUG: [twohop-broadcast] broadcast %spacket\n", list_length(list_neighbors) == 0 ? "empty " : "");

	neighbor_t *neighbor;

	uint8_t neighbors = 0;
	for(neighbor = list_head(list_neighbors); neighbor != NULL; neighbor = list_item_next(neighbor))
		neighbors += (networkaddr_equal(networkaddr_node_addr(), neighbor->node1)) ? 1 : 0;
	buffer_append_uint8t(data, neighbors);

	for(neighbor = list_head(list_neighbors); neighbor != NULL; neighbor = list_item_next(neighbor)) {
		if(networkaddr_equal(networkaddr_node_addr(), neighbor->node1)) {
			PRINTF("DEBUG: [twohop-broadcast] * neighbor[node=%s, weight_to_me=%d, weight_to_neighbor=%d]\n", networkaddr2string_buffered(neighbor->node2), neighbor->weight_node2_to_node1, neighbor->weight_node1_to_node2);
			buffer_append_networkaddr(data, neighbor->node2);
			buffer_append_int8t(data, neighbor->weight_node2_to_node1);
			buffer_append_int8t(data, neighbor->weight_node1_to_node2);
		}
	}
}

void _twohop_broadcast_print_neighborhood() {
#if DEBUG
	PRINTF("DEBUG: [twohop-broadcast] neighbor-information:\n");
	neighbor_t *neighbor;
	for (neighbor = list_head(list_neighbors); neighbor != NULL; neighbor = list_item_next(neighbor)) {
		char node_other[NETWORKADDR_STRSIZE];
		PRINTF("DEBUG: [twohop-broadcast] * edge[");
		PRINTF("node1=%s, node2=%s: ", networkaddr2string_buffered(neighbor->node1), networkaddr2string(node_other, neighbor->node2));
		PRINTF("node1->node2[weight=%d, ttl=%d] ", neighbor->weight_node1_to_node2, neighbor->ttl_node1_to_node2);
		PRINTF("node2->node1[weight=%d, ttl=%d]", neighbor->weight_node2_to_node1, neighbor->ttl_node2_to_node1);
		PRINTF("]\n");
	}
#endif
}

void _twohop_broadcast_handle(const networkaddr_t *sent_by, buffer_t *data, int8_t rssi) {
	int8_t edgeweight = _twohop_broadcast_edgeweight(sent_by, rssi);
	uint8_t neighbors = buffer_read_uint8t(data);
	PRINTF("DEBUG: [twohop-broadcast] received %d neighbors (node=%s, weight=%d)\n", neighbors, networkaddr2string_buffered(sent_by), edgeweight);

	// handle the node the packet has been received from
	neighbor_t *sender = neighbors_find_onehop_entry(list_neighbors, networkaddr_node_addr(), sent_by);
	if(sender == NULL) {
		if((sender = memb_alloc(&memb_neighbors)) == NULL) {
			printf("ERROR[twohop-broadcast]: neighbor-list is full\n");
		} else {
			sender->node1 = networkaddr_reference_alloc(networkaddr_node_addr());
			sender->node2 = networkaddr_reference_alloc(sent_by);
			sender->weight_node1_to_node2 = COMPONENT_NEIGHBORDISCOVERY_WEIGHTUNKNOWN;
			sender->weight_node2_to_node1 = edgeweight;
			sender->ttl_node1_to_node2 = 0;
			sender->ttl_node2_to_node1 = COMPONENT_NEIGHBORDISCOVERY_TWOHOPBROADCAST_LINKTTL;
			list_add(list_neighbors, sender);
		}
	} else {
		sender->ttl_node2_to_node1  = COMPONENT_NEIGHBORDISCOVERY_TWOHOPBROADCAST_LINKTTL;
		sender->weight_node2_to_node1 = edgeweight;
	}

	// handle all neighbor information in broadcasted packet
	int i;
	for (i = 0; i < neighbors; i++) {
		networkaddr_t node = buffer_read_networkaddr(data);
		int8_t weight_to_me = buffer_read_int8t(data);
		int8_t weight_to_neighbor = buffer_read_int8t(data);
		PRINTF("DEBUG: [twohop-broadcast] * node=%s, weight_to_me=%d, weight_to_neighbor=%d\n", networkaddr2string_buffered(&node), weight_to_me, weight_to_neighbor);

		// if we receive the broadcast from the sender to us
		if(networkaddr_equal(networkaddr_node_addr(), &node) && sender != NULL) {
			sender->weight_node1_to_node2 = weight_to_me;
			sender->ttl_node1_to_node2 = COMPONENT_NEIGHBORDISCOVERY_TWOHOPBROADCAST_LINKTTL;
		}
		// 2-hop neighbor
		else {
			neighbor_t *twohop = neighbors_find_twohop_entry(list_neighbors, sent_by, &node);
			if(twohop == NULL) {
				if((twohop = memb_alloc(&memb_neighbors)) == NULL) {
					printf("ERROR[twohop-broadcast]: neighbor-list is full\n");
					continue;
				} else {
					twohop->node1 = networkaddr_reference_alloc(sent_by);
					twohop->node2 = networkaddr_reference_alloc(&node);
					twohop->weight_node1_to_node2 = COMPONENT_NEIGHBORDISCOVERY_WEIGHTUNKNOWN;
					twohop->weight_node2_to_node1 = COMPONENT_NEIGHBORDISCOVERY_WEIGHTUNKNOWN;
					twohop->ttl_node1_to_node2 = 0;
					twohop->ttl_node2_to_node1 = 0;
					list_add(list_neighbors, twohop);
				}
			}

			// find out who node1 is who node2 is.
			// explanation: If a node X sends a broadcast, it always adds the weight from the referred neighbor to X
			if(networkaddr_equal(sent_by, twohop->node1)) {
				twohop->weight_node1_to_node2 = weight_to_neighbor;
				twohop->weight_node2_to_node1 = weight_to_me;
				twohop->ttl_node2_to_node1 = COMPONENT_NEIGHBORDISCOVERY_TWOHOPBROADCAST_LINKTTL;
			} else {
				twohop->weight_node1_to_node2 = weight_to_me;
				twohop->weight_node1_to_node2 = weight_to_neighbor;
				twohop->ttl_node1_to_node2 = COMPONENT_NEIGHBORDISCOVERY_TWOHOPBROADCAST_LINKTTL;
			}
		}
	}
}

void twohop_broadcast_decay_links() {
	PRINTF("DEBUG: [twohop-broadcast] decay links\n");

	neighbor_t *item = list_head(list_neighbors);
	while(item != NULL) {
		// decrease TTL
		if(item->ttl_node1_to_node2 > 0) {
			if(--item->ttl_node1_to_node2 == 0) {
#if DEBUG
				char node_other[NETWORKADDR_STRSIZE];
				PRINTF("DEBUG: [twohop-broadcast] link timeout ttl_node1_to_node2 %s->%s\n", networkaddr2string_buffered(item->node1), networkaddr2string(node_other, item->node2));
#endif
				item->weight_node1_to_node2 = COMPONENT_NEIGHBORDISCOVERY_WEIGHTUNKNOWN;
			}
		}
		if(item->ttl_node2_to_node1 > 0) {
			if(--item->ttl_node2_to_node1 == 0) {
#if DEBUG
				char node_other[NETWORKADDR_STRSIZE];
				PRINTF("DEBUG: [twohop-broadcast] link timeout ttl_node2_to_node1 %s->%s\n", networkaddr2string_buffered(item->node2), networkaddr2string(node_other, item->node1));
#endif
				item->weight_node2_to_node1 = COMPONENT_NEIGHBORDISCOVERY_WEIGHTUNKNOWN;
			}
		}

		// remove neighbor
		if(item->ttl_node1_to_node2 == 0 && item->ttl_node2_to_node1 == 0) {
#if DEBUG
			char node_other[NETWORKADDR_STRSIZE];
			PRINTF("DEBUG: [twohop-broadcast] removing link %s<->%s\n", networkaddr2string_buffered(item->node2), networkaddr2string(node_other, item->node1));
#endif
			neighbor_t *next = list_item_next(item);
			networkaddr_reference_free(item->node1);
			networkaddr_reference_free(item->node2);
			list_remove(list_neighbors, item);
			memb_free(&memb_neighbors, item);
			item = next;
		} else {
			item = list_item_next(item);
		}
	}

	_twohop_broadcast_print_neighborhood();
}

list_t twohop_broadcast_neighbors_get_all() {
	return list_neighbors;
}

#if COMPONENT_NEIGHBORDISCOVERY == COMPONENT_NEIGHBORDISCOVERY_TWOHOPBROADCAST
list_t component_neighbordiscovery_neighbors() {
	return twohop_broadcast_neighbors_get_all();
}
#endif
