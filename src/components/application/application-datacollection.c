#include <stdlib.h>
#include <stdio.h>
#include "contiki.h"

#include "application-datacollection.h"
#include "../../app-conf.h"
#include "../../lib/boot.h"
#include "../../lib/components.h"
#include "../../lib/networkaddr.h"
#include "../../lib/uniqueid.h"
#include "../../lib/utilities.h"

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#if COMPONENT_APPLICATION == COMPONENT_APPLICATION_DATACOLLECTION

static void recv(const networkaddr_t *source, buffer_t *data, int8_t rssi);

PROCESS(component_application, "application: data-collection");
PROCESS_THREAD(component_application, ev, data) {
	PROCESS_BEGIN();

	BOOT_COMPONENT_WAIT(component_application);

	static uint8_t messagetype;
	messagetype = uniqueid_assign();
	component_network_packet_subscribe(messagetype, recv);

	if(!networkaddr_equal(networkaddr_node_addr(), component_network_address_basestation()))
		printf("[application-datacollection] sending targets: %s\n", networkaddr2string_buffered(component_network_address_basestation()));

	static struct etimer etimer_send;
	etimer_set(&etimer_send, random(CLOCK_SECOND * COMPONENT_APPLICATION_DATACOLLECTION_INTERVALSEND_MIN, CLOCK_SECOND * COMPONENT_APPLICATION_DATACOLLECTION_INTERVALSEND_MAX));
	while(1) {
		PROCESS_WAIT_EVENT();

		if(etimer_expired(&etimer_send)) {
			etimer_set(&etimer_send, random(CLOCK_SECOND * COMPONENT_APPLICATION_DATACOLLECTION_INTERVALSEND_MIN, CLOCK_SECOND * COMPONENT_APPLICATION_DATACOLLECTION_INTERVALSEND_MAX));

			if(!networkaddr_equal(networkaddr_node_addr(), component_network_address_basestation())) {
				int i;
				buffer_t *data = component_network_packet_sendbuffer();
				for(i = 1; i <= COMPONENT_APPLICATION_DATACOLLECTION_MESSAGESIZE; i++)
					buffer_append_uint8t(data, random_rand() % UINT8_MAX);

				printf("[application-datacollection] sending data to %s\n", networkaddr2string_buffered(component_network_address_basestation()));
				component_network_packet_send(COMPONENT_NETWORK_TRANSMISSION_ROUTING_UNICAST, messagetype, component_network_address_basestation(), data, -1, -1);
			}
		}
	}

	PROCESS_END();
}

static void recv(const networkaddr_t *source, buffer_t *data, int8_t rssi) {
	printf("[application-datacollection] received data from %s\n", networkaddr2string_buffered(source));
}

#endif
