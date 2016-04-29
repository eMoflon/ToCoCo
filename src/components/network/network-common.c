#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "contiki-lib.h"

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

typedef struct subscription {
	struct subscription *next;
	uint8_t messagetype;
	void (*callback)(const networkaddr_t *source, buffer_t *data, int8_t rssi);
} subscription_t;

typedef struct linklocalsend {
	struct subscription *next;
	void (*callback)(const networkaddr_t *destination, int8_t txpower, int8_t num_tx, bool received);
} linklocalsend_t;

MEMB(memb_ignoredlinks, ignored_link_t, COMPONENT_NETWORKSTACK_IGNOREDLINKS_MEMORY);
LIST(list_ignoredlinks);

MEMB(memb_subscriptions, subscription_t, COMPONENT_NETWORKSTACK_PACKETSUBSCRIPTIONS_MEMORY);
LIST(list_subscriptions);

MEMB(memb_linklocalsends, linklocalsend_t, 5);
LIST(list_linklocalsends);

static void (*network_ignoredlink_notify)(bool added, networkaddr_t *address);

void network_common_init(void (*ignoredlink_notify)(bool added, networkaddr_t *address)) {
	memb_init(&memb_ignoredlinks);
	list_init(list_ignoredlinks);

	list_init(list_subscriptions);

	memb_init(&memb_linklocalsends);
	list_init(list_linklocalsends);

	network_ignoredlink_notify = ignoredlink_notify;
}

void network_common_subscribe(uint8_t messagetype, void (*callback)(const networkaddr_t *source, buffer_t *data, int8_t rssi)) {
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
			item->callback(source, data, rssi);
		}
	}
}

void components_network_ignoredlinks_add(networkaddr_t *address) {
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

		PRINTF("DEBUG: [network-common] added ignored link %s\n", networkaddr2string_buffered(change_from));
		network_ignoredlink_notify(true, address);
	}
}

void components_network_ignoredlinks_remove(networkaddr_t *address) {
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

list_t components_network_ignoredlinks_all() {
	return list_ignoredlinks;
}

void components_network_linklocalsend_subscribe(void (*callback)(const networkaddr_t *destination, int8_t txpower, int8_t num_tx, bool received)) {
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
#if COMPONENT_RADIO == COMPONENT_RADIO_CC2420 || COMPONENT_RADIO == COMPONENT_RADIO_CC2520
	// sometimes some packets have txpwr = 0 which should be impossible because COMPONENT_RADIO_TXPOWER_MIN is set to 1
	// with a zero txpower the actual transmission power of the radio module is used
	if(txpower == 0)
		txpower = components_radio_txpower_get();
#endif

	linklocalsend_t *item;
	for(item = list_head(list_linklocalsends); item != NULL; item = list_item_next(item)) {
		item->callback(destination, txpower, num_tx, received);
	}
}
