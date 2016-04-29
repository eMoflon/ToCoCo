#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "contiki.h"
#include "contiki-lib.h"

#include "application-randomwalk.h"
#include "../../app-conf.h"
#include "../../lib/boot.h"
#include "../../lib/buffer.h"
#include "../../lib/components.h"
#include "../../lib/networkaddr.h"
#include "../../lib/uniqueid.h"
#include "../../lib/utilities.h"

#if COMPONENT_APPLICATION == COMPONENT_APPLICATION_RANDOMWALK

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

typedef struct broadcast_neighbor {
	networkaddr_t *address;
	uint8_t fifo_counter;
	unsigned long last_seen; // from clock.h
} broadcast_neighbor_t;

static uint8_t hopcount_init = 5;
static uint8_t messagetype_broadcast;
static uint8_t messagetype_hopwalk;
static uint8_t messagetype_routing;
static broadcast_neighbor_t neighbors[20];

static void recv_broadcast(const networkaddr_t *source, buffer_t *data, int8_t rssi) ;
static void recv_hopwalk(const networkaddr_t *source, buffer_t *data, int8_t rssi);
static void recv_routing(const networkaddr_t *source, buffer_t *data, int8_t rssi);
void sendToRandomNeighbor(const networkaddr_t *hopwalk_source, uint8_t hopwalk_count);

/**
 * Application flow:
 *  - a message is sent to a random neighbor and randomly forwarded
 *  - if hopcount = 0 a routing message is sent back to the source
 *
 * Application Idea: Use all network send primitives with completely random routes involved without knowing the
 * structure of the network. This is a good application to test the complete network stack.
 *
 * INFORMAL:
 * Every node broadcasts at a fixed with interval with a random offset an empty message with the actual set txpower.
 * These received broadcasts are used to create a neighborhood. The neighborhood does not have to store all values
 * because broadcasts are sent with a fixed with interval and random offset and the hopwalk-messages are sent at
 * totally random times, so the application will see a different neighborhood every time. The neigborhould should be
 * implemented in a FIFO manner to guarantee the described handling. The neighbordiscovery component can not be used
 * because it can be a stub with no functionality (when not topology control algorithm is running). And the nexthop
 * list of the network stack can not be used because the routing may be empty for lazy routing algorithms creating a
 * route only when needed.
 */
PROCESS(component_application, "application: randomwalk");
PROCESS_THREAD(component_application, ev, data) {
	PROCESS_BEGIN();

	BOOT_COMPONENT_WAIT(component_application);

	messagetype_broadcast = uniqueid_assign();
	messagetype_hopwalk = uniqueid_assign();
	messagetype_routing = uniqueid_assign();

	components_network_packet_subscribe(messagetype_broadcast, recv_broadcast);
	components_network_packet_subscribe(messagetype_hopwalk, recv_hopwalk);
	components_network_packet_subscribe(messagetype_routing, recv_routing);

	static struct etimer etimer_send, etimer_broadcast;
	etimer_set(&etimer_broadcast, random(CLOCK_SECOND * COMPONENT_APPLICATION_RANDOMWALK_INTERVALBROADCAST_MIN, CLOCK_SECOND * COMPONENT_APPLICATION_RANDOMWALK_INTERVALBROADCAST_MAX));
	etimer_set(&etimer_send, random(CLOCK_SECOND * COMPONENT_APPLICATION_RANDOMWALK_INTERVALHOPWALKSTART_MIN, CLOCK_SECOND * COMPONENT_APPLICATION_RANDOMWALK_INTERVALHOPWALKSTART_MAX));
	while(1) {
		PROCESS_WAIT_EVENT();

		if(etimer_expired(&etimer_broadcast)) {
			etimer_set(&etimer_broadcast, random(CLOCK_SECOND * COMPONENT_APPLICATION_RANDOMWALK_INTERVALBROADCAST_MIN, CLOCK_SECOND * COMPONENT_APPLICATION_RANDOMWALK_INTERVALBROADCAST_MAX));
			components_network_packet_send(COMPONENTS_NETWORK_TRANSMISSION_LINKLOCAL_BROADCAST, NULL, -1, messagetype_broadcast, components_network_packet_sendbuffer());
		}

		if(etimer_expired(&etimer_send)) {
			etimer_set(&etimer_send, random(CLOCK_SECOND * COMPONENT_APPLICATION_RANDOMWALK_INTERVALHOPWALKSTART_MIN, CLOCK_SECOND * COMPONENT_APPLICATION_RANDOMWALK_INTERVALHOPWALKSTART_MAX));
			printf("randomwalk start: send with hopcount=%d...\n", hopcount_init);
			sendToRandomNeighbor(networkaddr_node_addr(), hopcount_init);
		}
	}

	PROCESS_END();
}

static void recv_broadcast(const networkaddr_t *source, buffer_t *data, int8_t rssi) {
	uint8_t i;

	// reomve old addresses
	for(i = 0; i < sizeof(neighbors) / sizeof(neighbors[0]); i++) {
		if(neighbors[i].address != NULL && clock_seconds() - neighbors[i].last_seen > 60) {
			networkaddr_reference_free(neighbors[i].address);
			neighbors[i].address = NULL;
		}
	}

	// search bucket to insert new address or update old address
	uint8_t fifo_min = 255; uint8_t fifo_max = 0, bucket_min = 255, bucket_save = 255;
	for(i = 0; i < sizeof(neighbors) / sizeof(neighbors[0]); i++) {
		fifo_min = MIN(fifo_min, neighbors[i].fifo_counter);
		fifo_max = MAX(fifo_max, neighbors[i].fifo_counter);

		// remember the bucket with the lowest fifo counter as this bucket will be replaced when array is full
		if(neighbors[i].fifo_counter == fifo_min)
			bucket_min = i;

		// if address is already saved we have to increment the value
		if(neighbors[i].address != NULL && networkaddr_equal(neighbors[i].address, source))
			bucket_save = i;

		// an empty bucket is used for saving new reference
		if(bucket_save == 255 && neighbors[i].address == NULL)
			bucket_save = i;
	}
	if(bucket_save == 255)
		bucket_save = bucket_min;

	// save address in bucket
	if(neighbors[bucket_save].address != NULL)
		networkaddr_reference_free(neighbors[bucket_save].address);
	neighbors[bucket_save].address = networkaddr_reference_alloc(source);
	neighbors[bucket_save].fifo_counter = fifo_max + 1;
	neighbors[bucket_save].last_seen = clock_seconds();
}

static void recv_hopwalk(const networkaddr_t *source, buffer_t *data, int8_t rssi) {
	networkaddr_t hopwalk_source = buffer_read_networkaddr(data);
	uint8_t hopwalk_count = buffer_read_uint8t(data);

	PRINTF("DEBUG: [application-randomwalk] received hopwalk from %s with count = %d\n", networkaddr2string_buffered(source), hopwalk_count);
	if(--hopwalk_count > 0) {
		sendToRandomNeighbor(&hopwalk_source, hopwalk_count);
	} else {
		if(networkaddr_equal(&hopwalk_source, networkaddr_node_addr())) {
			printf("randomwalk finished: endpoint is /me\n");
		} else {
			PRINTF("DEBUG: [application-randomwalk] randomwalk endpoint reached, answering %s...\n", networkaddr2string_buffered(&hopwalk_source));
			components_network_packet_send(COMPONENTS_NETWORK_TRANSMISSION_ROUTING_UNICAST, &hopwalk_source, -1, messagetype_routing, components_network_packet_sendbuffer());
		}
	}
}

static void recv_routing(const networkaddr_t *source, buffer_t *data, int8_t rssi) {
	printf("randomwalk finished: received response from endpoint %s\n", networkaddr2string_buffered(source));
}

void sendToRandomNeighbor(const networkaddr_t *hopwalk_source, uint8_t hopwalk_count) {
	uint8_t i, numNeighbors = 0, numNeighbor = 0, randomNeighbor;
	for(i = 0; i < sizeof(neighbors) / sizeof(neighbors[0]); i++) {
		if(neighbors[i].address != NULL)
			numNeighbors++;
	}

	randomNeighbor = random_rand() % numNeighbors;
	for(i = 0; i < sizeof(neighbors) / sizeof(neighbors[0]); i++) {
		if(neighbors[i].address == NULL)
			continue;

		if(numNeighbor++ == randomNeighbor) {
			buffer_t *data = components_network_packet_sendbuffer();
			buffer_append_networkaddr(data, hopwalk_source);
			buffer_append_uint8t(data, hopwalk_count);
			components_network_packet_send(COMPONENTS_NETWORK_TRANSMISSION_LINKLOCAL_UNICAST, neighbors[i].address, -1, messagetype_hopwalk, data);
			PRINTF("DEBUG: [application-randomwalk] send randomwalk to %s with count=%d\n", networkaddr2string_buffered(neighbors[i].address), hopwalk_count);
		}
	}
}

#endif
