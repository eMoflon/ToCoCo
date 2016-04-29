#include <stdio.h>
#include "contiki.h"
#include "contiki-lib.h"

#include "application-powercalibration.h"
#include "../../app-conf.h"
#include "../../lib/boot.h"
#include "../../lib/buffer.h"
#include "../../lib/components.h"
#include "../../lib/networkaddr.h"
#include "../../lib/uniqueid.h"
#include "../../lib/utilities.h"

#define TESTSAMPLES ((COMPONENT_APPLICATION_POWERCALIBRATION_TESTINTERVAL_LENGTH) / (COMPONENT_APPLICATION_POWERCALIBRATION_SENDINTERVAL_LENGTH))
#define POWERLEVELS (COMPONENT_RADIO_TXPOWER_MAX - COMPONENT_RADIO_TXPOWER_MIN + 1)

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#if COMPONENT_APPLICATION == COMPONENT_APPLICATION_POWERCALIBRATION

typedef struct received_samples {
	struct received_sample *next;
	networkaddr_t *source;
	uint16_t received_powelevels[COMPONENT_RADIO_TXPOWER_MAX - COMPONENT_RADIO_TXPOWER_MIN + 1];
} received_samples_t;

static uint8_t messagetype;

static void recv(const networkaddr_t *source, buffer_t *data, int8_t rssi);

static struct etimer etimer_testwaittime;
static struct etimer etimer_testsample;

MEMB(memb_receivedsamples, received_samples_t, COMPONENT_APPLICATION_POWERCALIBRATION_MEMORY);
LIST(list_receivedsamples);

PROCESS(component_application, "application: powercalibration");
PROCESS_THREAD(component_application, ev, data) {
	PROCESS_BEGIN();

	BOOT_COMPONENT_WAIT(component_application);

	memb_init(&memb_receivedsamples);
	list_init(list_receivedsamples);

	messagetype = uniqueid_assign();
	component_network_packet_subscribe(messagetype, recv);

	printf("[application-powercalibration] testing power range %d-%d\n", COMPONENT_RADIO_TXPOWER_MIN, COMPONENT_RADIO_TXPOWER_MAX);

	// phase 1: wait for all motes to be running
	etimer_set(&etimer_testwaittime, CLOCK_SECOND * COMPONENT_APPLICATION_POWERCALIBRATION_TESTINTERVAL_WAIT);
	PRINTF("DEBUG: [application-powercalibration] waiting %ds for all motes to be started\n", COMPONENT_APPLICATION_POWERCALIBRATION_TESTINTERVAL_WAIT);
	PROCESS_WAIT_UNTIL(etimer_expired(&etimer_testwaittime));

	// phase 2: send all powerlevel samples
	static uint16_t testsamples_remaining = TESTSAMPLES;
	while(testsamples_remaining-- > 0) {
		PRINTF("DEBUG: [application-powercalibration] starting testphase %d\n",  TESTSAMPLES - testsamples_remaining);

		// random distribution calculation variables
		static uint32_t acc_interval_sum = 0;
		static uint32_t acc_interval_used = 0;
		static uint32_t interval_ticks = (COMPONENT_APPLICATION_POWERCALIBRATION_SENDINTERVAL_LENGTH * CLOCK_SECOND) / POWERLEVELS;
		static uint32_t waitingtime;

		// fill & shuffle power levels
		int i;
		static int8_t powerlevels[COMPONENT_RADIO_TXPOWER_MAX - COMPONENT_RADIO_TXPOWER_MIN + 1];
		for(i = 0; i < POWERLEVELS; i++) {
			powerlevels[i] = COMPONENT_RADIO_TXPOWER_MIN + i;
		}
		for(i = 0; i < POWERLEVELS; i++) {
			int random1 = random(0, POWERLEVELS - 1);
			int tmp = powerlevels[random1];
			powerlevels[random1] = powerlevels[i];
			powerlevels[i] = tmp;
		}

		// broadcast powersamples with a random distribution
		static int8_t powerlevel_i;
		for(powerlevel_i = 0; powerlevel_i < POWERLEVELS; powerlevel_i++) {
			// random offset waiting
			acc_interval_sum += interval_ticks;
			acc_interval_used += (waitingtime = random(0, acc_interval_sum - acc_interval_used));
			etimer_set(&etimer_testsample, waitingtime);
			PROCESS_WAIT_UNTIL(etimer_expired(&etimer_testsample));

			// broadcasting
			buffer_t *sendbuffer = component_network_packet_sendbuffer();
			buffer_append_int8t(sendbuffer, powerlevels[powerlevel_i]);
			component_network_packet_send(COMPONENT_NETWORK_TRANSMISSION_LINKLOCAL_BROADCAST, messagetype, NULL, sendbuffer, powerlevels[powerlevel_i], -1);
		}

		// wait rest of sampling interval to have waited exactly COMPONENT_APPLICATION_POWERCONTROL_SENDINTERVAL_LENGTH seconds
		acc_interval_used += (waitingtime = acc_interval_sum - acc_interval_used);
		etimer_set(&etimer_testsample, waitingtime);
		PROCESS_WAIT_UNTIL(etimer_expired(&etimer_testsample));
	}

	// phase 3 debug received samples
#if DEBUG
	received_samples_t *sample_debug;
	for(sample_debug = list_head(list_receivedsamples); sample_debug != NULL; sample_debug = list_item_next(sample_debug)) {
		PRINTF("DEBUG: [application-powercalibration] received from %s(%d): ", networkaddr2string_buffered(sample_debug->source), TESTSAMPLES);

		int8_t powerlevel;
		for(powerlevel = COMPONENT_RADIO_TXPOWER_MIN; powerlevel <= COMPONENT_RADIO_TXPOWER_MAX; powerlevel++) {
			printf("%d=%d", powerlevel, sample_debug->received_powelevels[powerlevel - COMPONENT_RADIO_TXPOWER_MIN]);
			if(powerlevel < COMPONENT_RADIO_TXPOWER_MAX)
				printf(",");
		}

		PRINTF("\n");
	}
#endif

	// phase 4 print best powerlevel
	received_samples_t *sample;
	for(sample = list_head(list_receivedsamples); sample != NULL; sample = list_item_next(sample)) {
		// search powerlevel matching desired QoS
		int8_t powerlevel;
		for(powerlevel = COMPONENT_RADIO_TXPOWER_MIN; powerlevel <= COMPONENT_RADIO_TXPOWER_MAX; powerlevel++) {
			float ratio = ((float) sample->received_powelevels[powerlevel - COMPONENT_RADIO_TXPOWER_MIN]) / TESTSAMPLES;
			if(ratio >= COMPONENT_APPLICATION_POWERCALIBRATION_RATIO) {
				char networkaddr2[NETWORKADDR_STRSIZE];
				printf("[application-powercalibration] optimum txpower=%d for %s->%s\n", powerlevel, networkaddr2string(networkaddr2, sample->source), networkaddr2string_buffered(networkaddr_node_addr()));
				break;
			}
		}
	}

	PROCESS_END();
}

static void recv(const networkaddr_t *source, buffer_t *data, int8_t rssi) {
	int8_t powerlevel = buffer_read_int8t(data);

	// search (or create) received sample entry for source address
	received_samples_t *sample;
	for(sample = list_head(list_receivedsamples); sample != NULL; sample = list_item_next(sample)) {
		if(networkaddr_equal(sample->source, source))
			break;
	}
	if(sample == NULL) {
		if((sample = memb_alloc(&memb_receivedsamples)) == NULL) {
			printf("ERROR: [application-powercalibration]: received samples memory is too small\n");
			return;
		}

		sample->source = networkaddr_reference_alloc(source);
		list_add(list_receivedsamples, sample);
	}

	// save received sample
	sample->received_powelevels[powerlevel - COMPONENT_RADIO_TXPOWER_MIN]++;
}

#endif
