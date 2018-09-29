#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "contiki.h"
#include "contiki-lib.h"
#include "sys/ctimer.h"

#include "network-common.h"
#include "../../lib/buffer.h"
#include "../../lib/components.h"
#include "../../lib/networkaddr.h"

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#if COMPONENT_NETWORK_NEXTHOPS_CLOCKTICKS_CIRCULARBUFFER > 16
#error the circular buffer size is limited to 16 bits
#endif

typedef struct subscription {
	struct subscription *next;
	uint8_t messagetype;
	struct process *process;
	void (*callback)(const networkaddr_t *source, buffer_t *data, int8_t rssi);
} subscription_t;

typedef struct linklocalsend {
	struct subscription *next;
	void (*callback)(const networkaddr_t *destination, int8_t txpower, int8_t num_tx, bool received);
} linklocalsend_t;

MEMB(memb_ignoredlinks, ignored_link_t, COMPONENT_NETWORK_IGNOREDLINKS_MEMORY);
LIST(list_ignoredlinks);

MEMB(memb_subscriptions, subscription_t, COMPONENT_NETWORK_PACKETSUBSCRIPTIONS_MEMORY);
LIST(list_subscriptions);

MEMB(memb_linklocalsends, linklocalsend_t, 5);
LIST(list_linklocalsends);

MEMB(memb_nexthops, nexthop_t, COMPONENT_NETWORK_NEXTHOPS_MEMORY);
LIST(list_nexthops);
LIST(list_nexthops_internal);

static struct ctimer timer_nexthopclockupdate;

static networkaddr_t basestation;

static void (*network_ignoredlink_notify)(bool added, networkaddr_t *address);
static void linklocalsends_nexthops(const networkaddr_t *destination, int8_t txpower, int8_t num_tx, bool received);
static void callback_nexthopclockupdate(void *ptr);
static uint8_t count_bits(uint16_t num);

void network_common_init(void (*ignoredlink_notify)(bool added, networkaddr_t *address)) {
	memb_init(&memb_ignoredlinks);
	list_init(list_ignoredlinks);

	list_init(list_subscriptions);

	memb_init(&memb_linklocalsends);
	list_init(list_linklocalsends);

	memb_init(&memb_nexthops);
	list_init(list_nexthops);
	list_init(list_nexthops_internal);

	component_network_linklocalsend_subscribe(linklocalsends_nexthops);
	ctimer_set(&timer_nexthopclockupdate, CLOCK_SECOND * COMPONENT_NETWORK_NEXTHOPS_CLOCKTICKS_TIME, callback_nexthopclockupdate, NULL);

#if COMPONENT_NETWORK == COMPONENT_NETWORK_IPV6
	networkaddr_fromstring(&basestation, COMPONENT_NETWORK_IPV6_BASESTATION);
#endif
#if COMPONENT_NETWORK == COMPONENT_NETWORK_RIME
	networkaddr_fromstring(&basestation, COMPONENT_NETWORK_RIME_BASESTATION);
#endif

	network_ignoredlink_notify = ignoredlink_notify;
}

networkaddr_t *component_network_address_basestation() {
	return &basestation;
}

void component_network_packet_subscribe(uint8_t messagetype, void (*callback)(const networkaddr_t *source, buffer_t *data, int8_t rssi)) {
	PRINTF("DEBUG: [network-common] subscribe callback=0x%x for messagetype=%d\n", callback, messagetype);

	struct subscription *item;
	for(item = list_head(list_subscriptions); item != NULL; item = list_item_next(item)) {
		if(item->messagetype == messagetype &&  item->callback == callback) {
			PRINTF("DEBUG: [network-common] already subscribed\n");
			return;
		}
	}

	struct subscription *new = memb_alloc(&memb_subscriptions);
	if(new == NULL) {
		printf("ERROR[network-common]: adding subscription failed\n");
	} else {
		new->messagetype = messagetype;
		new->process = PROCESS_CURRENT(); // the current process who registered this subscription to change to this context when executing callback
		new->callback = callback;
		list_add(list_subscriptions, new);
	}

	PRINTF("DEBUG: [network-common] %d Subscriptions: ", list_length(list_subscriptions));
	for(item = list_head(list_subscriptions); item != NULL; item = list_item_next(item)) {
		PRINTF("(callback=0x%x, messagetype=%d), ", item->callback, item->messagetype);
	}
	PRINTF("\n");
}

void network_common_publish(buffer_t *data, const networkaddr_t *source, int8_t rssi) {
	uint8_t messagetype = buffer_read_uint8t(data);
	size_t offset = data->offset;
	struct subscription *item;
	for(item = list_head(list_subscriptions); item != NULL; item = list_item_next(item)) {
		if(item->messagetype == messagetype) {
			data->offset = offset; // if multiple receivers are registered for messagetype they should all start at beginning of data in buffer

			PROCESS_CONTEXT_BEGIN(item->process);
			item->callback(source, data, rssi);
			PROCESS_CONTEXT_END(item->process);
		}
	}
}

void component_network_ignoredlinks_add(networkaddr_t *address) {
	ignored_link_t *item;
	for(item = list_head(list_ignoredlinks); item != NULL; item = list_item_next(item)) {
		if(networkaddr_equal(item->address, address)) {
			PRINTF("DEBUG: [network-common] ignored link %s already saved\n", networkaddr2string_buffered(address));
			return;
		}
	}

	ignored_link_t *entry = memb_alloc(&memb_ignoredlinks);
	if(entry != NULL) {
		entry->address = networkaddr_reference_alloc(address);
		list_add(list_ignoredlinks, entry);

		PRINTF("DEBUG: [network-common] added ignored link %s\n", networkaddr2string_buffered(address));
		network_ignoredlink_notify(true, address);
	}
}

void component_network_ignoredlinks_remove(networkaddr_t *address) {
	ignored_link_t *item;
	for(item = list_head(list_ignoredlinks); item != NULL; item = list_item_next(item)) {
		if(networkaddr_equal(item->address, address)) {
			networkaddr_reference_free(item->address);
			list_remove(list_ignoredlinks, item);
			memb_free(&memb_ignoredlinks, item);

			PRINTF("DEBUG: [network-common] removed ignored link %s\n", networkaddr2string_buffered(address));

			network_ignoredlink_notify(false, address);
			return;
		}
	}

	PRINTF("DEBUG: [network-common] removing ignored link %s failed, because it's not ignored\n", networkaddr2string_buffered(address));
}

list_t component_network_ignoredlinks_all() {
	return list_ignoredlinks;
}

void component_network_linklocalsend_subscribe(void (*callback)(const networkaddr_t *destination, int8_t txpower, int8_t num_tx, bool received)) {
	PRINTF("DEBUG: [network-common] subscribe callback=0x%x for linklocal sends\n", callback);

	linklocalsend_t *item;
	for(item = list_head(list_linklocalsends); item != NULL; item = list_item_next(item)) {
		if(item->callback == callback) {
			PRINTF("DEBUG: [network-common] already subscribed\n");
			return;
		}
	}

	linklocalsend_t *new = memb_alloc(&memb_linklocalsends);
	if(new == NULL) {
		printf("ERROR[network-common]: adding linklocalsend subscription failed\n");
	} else {
		new->callback = callback;
		list_add(list_linklocalsends, new);
	}

	PRINTF("DEBUG: [network-common] %d linklocal subscriptions: ", list_length(list_linklocalsends));
	for(item = list_head(list_linklocalsends); item != NULL; item = list_item_next(item)) {
		PRINTF("(callback=0x%x), ", item->callback);
	}
	PRINTF("\n");
}

void network_common_linklocalsend_publish(const networkaddr_t *destination, int8_t txpower, int8_t num_tx, bool received) {
	linklocalsend_t *item;
	for(item = list_head(list_linklocalsends); item != NULL; item = list_item_next(item)) {
		item->callback(destination, txpower, num_tx, received);
	}
}

static void linklocalsends_nexthops(const networkaddr_t *destination, int8_t txpower, int8_t num_tx, bool received) {
	// a messages which was not received should not count for the nexthop algorithm because the destination may not be reachable anymore
	if(!received)
		return;

	// search for an existing entry in one of the nexthop lists
	nexthop_t *item;
	for(item = list_head(list_nexthops); item != NULL; item = list_item_next(item)) {
		if(networkaddr_equal(item->address, destination))
			goto searchend;
	}
	for(item = list_head(list_nexthops_internal); item != NULL; item = list_item_next(item)) {
		if(networkaddr_equal(item->address, destination))
			goto searchend;
	}
	searchend:

	// create new item if not existing
	if(item == NULL) {
		if((item = memb_alloc(&memb_nexthops)) == NULL) {
			printf("ERROR[network-common]: adding nexthop failed\n");
			return;
		}

		item->used_clockticks = 0;
		item->address = networkaddr_reference_alloc(destination);
		list_add(list_nexthops_internal, item);
	}

	// set clock tick bit and maybe move it to the live nexthop list
	item->used_clockticks |= 0x01;
}

list_t component_network_nexthops_all() {
	// remove nexthops which have been ignored or which have no set bit
	do {
		nexthop_t *item;
		list_t list_ignoredlinks = component_network_ignoredlinks_all();

		// remove from live nexthop list
		ignore1:
		for(item = list_head(list_nexthops); item != NULL; item = list_item_next(item)) {
			ignored_link_t *item2;
			for(item2 = list_head(list_ignoredlinks); item2 != NULL; item2 = list_item_next(item2)) {
				if(networkaddr_equal(item->address, item2->address) || item->used_clockticks == 0) {
					list_remove(list_nexthops, item);
					memb_free(&memb_nexthops, item);
					goto ignore1;
				}
			}
		}

		// remove from internal nexthop list
		ignore2:
		for(item = list_head(list_nexthops_internal); item != NULL; item = list_item_next(item)) {
			ignored_link_t *item2;
			for(item2 = list_head(list_ignoredlinks); item2 != NULL; item2 = list_item_next(item2)) {
				if(networkaddr_equal(item->address, item2->address) || item->used_clockticks == 0) {
					list_remove(list_nexthops_internal, item);
					memb_free(&memb_nexthops, item);
					goto ignore2;
				}
			}
		}
	} while(0);

	// move items from live list to internal list if their used_clockticks is below NEXTHOPS_CLOCKTICKS_REQUIRED
	do {
		nexthop_t *item;

		// move from nexthop list to internal nexthop list
		move1:
		for(item = list_head(list_nexthops); item != NULL; item = list_item_next(item)) {
			if(count_bits(item->used_clockticks) < COMPONENT_NETWORK_NEXTHOPS_CLOCKTICKS_REQUIRED) {
				list_remove(list_nexthops, item);
				list_add(list_nexthops_internal, item);
				goto move1;
			}
		}

		// move from internal nexthop list to nexthop list
		move2:
		for(item = list_head(list_nexthops_internal); item != NULL; item = list_item_next(item)) {
			if(count_bits(item->used_clockticks) >= COMPONENT_NETWORK_NEXTHOPS_CLOCKTICKS_REQUIRED) {
				list_remove(list_nexthops_internal, item);
				list_add(list_nexthops, item);
				goto move2;
			}
		}
	} while(0);

	return list_nexthops;
}

static void callback_nexthopclockupdate(void *ptr) {
	nexthop_t *item;

	// update clock ticks
	for(item = list_head(list_nexthops); item != NULL; item = list_item_next(item))
		item->used_clockticks = (item->used_clockticks << 1);
	for(item = list_head(list_nexthops_internal); item != NULL; item = list_item_next(item))
		item->used_clockticks = (item->used_clockticks << 1);

	ctimer_reset(&timer_nexthopclockupdate);
}

static uint8_t count_bits(uint16_t num) {
	// Brian Kernighan bit counting algorithm
	int8_t count;
	for(count = 0; num != 0; count++, num &= num - 1);

	return MIN(COMPONENT_NETWORK_NEXTHOPS_CLOCKTICKS_CIRCULARBUFFER, count);
}
