#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include "application-meshmessages.h"
#include "../../app-conf.h"
#include "lib/boot.h"
#include "lib/components.h"
#include "lib/networkaddr.h"
#include "lib/uniqueid.h"
#include "lib/utilities.h"
#include "net/rime/route.h"
#if COMPONENT_POWERCONTROL == COMPONENT_POWERCONTROL_PTPC
	#include "../powercontrol/powercontrol-ptpc.h"
#endif
#if COMPONENT_APPLICATION == COMPONENT_APPLICATION_MESHMESSAGES

#if COMPONENT_NETWORKSTACK == COMPONENT_NETWORKSTACK_IPV6
	#error mesh communication would not really be wise on a RPL tree
#endif

static void recv(const networkaddr_t *source, buffer_t *data, int8_t rssi);
static void randomaddresses_fill();
static uint8_t _nodelist_num();
static networkaddr_t _nodelist_get(uint8_t num);

networkaddr_t randomaddresses[COMPONENT_APPLICATION_MESHMESSAGES_RANDOM_NODES];

PROCESS(component_application, "application: mesh-messages");
PROCESS_THREAD(component_application, ev, data) {
	PROCESS_BEGIN();

	// choose random addresses
	randomaddresses_fill();

	BOOT_COMPONENT_WAIT(component_application);

	static uint8_t messagetype;
	messagetype = uniqueid_assign();
	components_network_packet_subscribe(messagetype, recv);
#if COMPONENT_POWERCONTROL == COMPONENT_POWERCONTROL_PTPC
#if BOOT_STRAPPHASE
	PROCESS_WAIT_EVENT_UNTIL(ev == Bootstrap_finished);
#endif
#endif
	int i;
	printf("[application-meshmessages] sending targets: ");
	for(i = 0; i < COMPONENT_APPLICATION_MESHMESSAGES_RANDOM_NODES; i++) {
		printf("%s", networkaddr2string_buffered(&randomaddresses[i]));
		printf("%s", i < COMPONENT_APPLICATION_MESHMESSAGES_RANDOM_NODES - 1 ? ", " : "\n");
	}

	static struct etimer etimer_send;
	etimer_set(&etimer_send, random(CLOCK_SECOND * COMPONENT_APPLICATION_MESHMESSAGES_INTERVALSEND_MIN, CLOCK_SECOND * COMPONENT_APPLICATION_MESHMESSAGES_INTERVALSEND_MAX));
	while(1) {
		PROCESS_WAIT_EVENT();

		if(etimer_expired(&etimer_send)) {
			etimer_set(&etimer_send, random(CLOCK_SECOND * COMPONENT_APPLICATION_MESHMESSAGES_INTERVALSEND_MIN, CLOCK_SECOND * COMPONENT_APPLICATION_MESHMESSAGES_INTERVALSEND_MAX));

			int i;
			buffer_t *data = components_network_packet_sendbuffer();
			for(i = 1; i <= COMPONENT_APPLICATION_MESHMESSAGES_MESSAGESIZE; i++)
				buffer_append_uint8t(data, random_rand() % UINT8_MAX);

			networkaddr_t destination = randomaddresses[random(0, COMPONENT_APPLICATION_MESHMESSAGES_RANDOM_NODES - 1)];
			networkaddr_t * nexthop;
			struct route_entry * r = route_lookup(&destination);
			if(r == NULL)
				nexthop = NULL;
			else
				nexthop = &(r->nexthop);
			printf("[application-meshmessages] sending data to %s\n", networkaddr2string_buffered(&destination));
			components_network_packet_send(COMPONENTS_NETWORK_TRANSMISSION_ROUTING_UNICAST, &destination, components_powercontrol_destinationtxpower(nexthop), messagetype, data);
			//printf("[application-contention] contention count %lu \n",RIMESTATS_GET(contentiondrop));
		}
	}

	PROCESS_END();
}

static void recv(const networkaddr_t *source, buffer_t *data, int8_t rssi) {
	printf("[application-meshmessages] received data from %s\n", networkaddr2string_buffered(source));
}

static void randomaddresses_fill() {
	uint8_t i, cnt = _nodelist_num();
	for(i = 0; i < COMPONENT_APPLICATION_MESHMESSAGES_RANDOM_NODES; i++) {
		networkaddr_t random_networkaddr;

		// try to add unique network addresses into array, but if there are too
		// less random nodes or the probability to select them is too low just
		// allow duplicates (-> make a few attempts for uniqueness)
		uint8_t attempts = 10;
		bool is_unique;
		do {
			random_networkaddr = _nodelist_get(random(0, cnt - 1));
			is_unique = true;

			uint8_t j;
			for(j = 0; j < COMPONENT_APPLICATION_MESHMESSAGES_RANDOM_NODES; j++)
				is_unique &= (!networkaddr_equal(&randomaddresses[j], &random_networkaddr));

			// choosing yourself would be a bad bad idea...
			if(networkaddr_equal(&random_networkaddr, networkaddr_node_addr())) {
				is_unique = false;
				attempts++;
			}
		} while(!is_unique && --attempts > 0);

		randomaddresses[i] = random_networkaddr;
	}
}

static uint8_t _nodelist_num() {
	uint8_t nodes = 0;

	char str[] = COMPONENT_APPLICATION_MESHMESSAGES_NODESLIST_RIME;
	char *pch;
	for(pch = strtok(str, ","); pch != NULL; pch = strtok(NULL, ","))
	    nodes++;

	return nodes;
}

static networkaddr_t _nodelist_get(uint8_t num) {
	char str[] = COMPONENT_APPLICATION_MESHMESSAGES_NODESLIST_RIME, *pch;
	uint8_t pos;

	pch = strtok(str, ",");
	for(pos = 0; pos < num; pos++) {
		pch = strtok(NULL, ",");

		if(pch == NULL)
			return linkaddr_null;
	}

	networkaddr_t node;
	networkaddr_fromstring(&node, pch);

	return node;
}

#endif
