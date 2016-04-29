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
#if COMPONENT_POWERCONTROL == COMPONENT_POWERCONTROL_PTPC
	#include "../powercontrol/powercontrol-ptpc.h"
#endif
#if COMPONENT_APPLICATION == COMPONENT_APPLICATION_DATACOLLECTION

static void recv(const networkaddr_t *source, buffer_t *data, int8_t rssi);

PROCESS(component_application, "application: data-collection");
PROCESS_THREAD(component_application, ev, data) {
	PROCESS_BEGIN();

	static networkaddr_t destination;
#if COMPONENT_NETWORKSTACK == COMPONENT_NETWORKSTACK_IPV6
	networkaddr_fromstring(&destination, COMPONENT_APPLICATION_DATACOLLECTION_SINK_IPV6);
#endif
#if COMPONENT_NETWORKSTACK == COMPONENT_NETWORKSTACK_RIME
	networkaddr_fromstring(&destination, COMPONENT_APPLICATION_DATACOLLECTION_SINK_RIME);
#endif

	BOOT_COMPONENT_WAIT(component_application);

	static uint8_t messagetype;
	messagetype = uniqueid_assign();
	components_network_packet_subscribe(messagetype, recv);
#if COMPONENT_POWERCONTROL == COMPONENT_POWERCONTROL_PTPC
#if BOOT_STRAPPHASE
	PROCESS_WAIT_EVENT_UNTIL(ev == Bootstrap_finished);
#endif
#endif
	printf("[application-datacollection] sending targets: %s\n", networkaddr2string_buffered(&destination));

	static struct etimer etimer_send;
	etimer_set(&etimer_send, random(CLOCK_SECOND * COMPONENT_APPLICATION_DATACOLLECTION_INTERVALSEND_MIN, CLOCK_SECOND * COMPONENT_APPLICATION_DATACOLLECTION_INTERVALSEND_MAX));
	while(1) {
		PROCESS_WAIT_EVENT();

		if(etimer_expired(&etimer_send)) {
			etimer_set(&etimer_send, random(CLOCK_SECOND * COMPONENT_APPLICATION_DATACOLLECTION_INTERVALSEND_MIN, CLOCK_SECOND * COMPONENT_APPLICATION_DATACOLLECTION_INTERVALSEND_MAX));

			if(!networkaddr_equal(&destination, networkaddr_node_addr())) {
				int i;
				buffer_t *data = components_network_packet_sendbuffer();
				for(i = 1; i <= COMPONENT_APPLICATION_DATACOLLECTION_MESSAGESIZE; i++)
					buffer_append_uint8t(data, random_rand() % UINT8_MAX);
#if COMPONENT_POWERCONTROL == COMPONENT_POWERCONTROL_PTPC
				networkaddr_t * nexthop;
				struct route_entry * r = route_lookup(&destination);
				if(r == NULL)
					nexthop = NULL;
				else
					nexthop = &(r->nexthop);
				printf("[application-datacollection] sending data to %s\n", networkaddr2string_buffered(&destination));
				components_network_packet_send(COMPONENTS_NETWORK_TRANSMISSION_ROUTING_UNICAST, &destination, components_powercontrol_destinationtxpower(nexthop), messagetype, data);
#else
				printf("[application-datacollection] sending data to %s\n", networkaddr2string_buffered(&destination));
				components_network_packet_send(COMPONENTS_NETWORK_TRANSMISSION_ROUTING_UNICAST, &destination, COMPONENT_RADIO_TXPOWER_MAX, messagetype, data);
#endif
			}
		}
	}

	PROCESS_END();
}

static void recv(const networkaddr_t *source, buffer_t *data, int8_t rssi) {
	printf("[application-datacollection] received data from %s\n", networkaddr2string_buffered(source));
}

#endif
