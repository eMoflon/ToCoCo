#include <stdbool.h>
#include <stdio.h>
#include "contiki.h"

#include "powercontrol-adaptive.h"
#include "../../app-conf.h"
#include "../../lib/boot.h"
#include "../../lib/components.h"
#include "../../lib/networkaddr.h"

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#if COMPONENT_POWERCONTROL == COMPONENT_POWERCONTROL_ADAPTIVE

#define OPTIMIZATIONSTEP ((int) ((COMPONENT_RADIO_TXPOWER_MAX - COMPONENT_RADIO_TXPOWER_MIN) * COMPONENT_POWERCONTROL_ADAPTIVE_OPTIMIZATIONPERCENT))

typedef struct power_optimization {
	struct power_optimization *next;

	networkaddr_t *node;
	int8_t power;
	int16_t messages_sent;
	int16_t messages_received;
	int16_t messages_consecutivefailed;

	struct etimer optimization_timer;
	bool optimization_active;
	int8_t optimization_power;
} power_optimization_t;

MEMB(memb_optimizations, power_optimization_t, COMPONENT_NETWORK_NEXTHOPS_MEMORY);
LIST(list_optimizations);

static void _powercontrol_linklocal(const networkaddr_t *destination, int8_t txpower, int8_t num_tx, bool received);

PROCESS(component_powercontrol, "powercontrol: adaptive");
PROCESS_THREAD(component_powercontrol, ev, data) {
	PROCESS_BEGIN();

	memb_init(&memb_optimizations);
	list_init(list_optimizations);

	BOOT_COMPONENT_WAIT(component_powercontrol);

	component_network_linklocalsend_subscribe(_powercontrol_linklocal);

	while(1) {
		PROCESS_WAIT_EVENT();

		power_optimization_t *optimization;
		for(optimization = list_head(list_optimizations); optimization != NULL; optimization = list_item_next(optimization)) {
			if(etimer_expired(&optimization->optimization_timer)) {
				if(optimization->optimization_active) {
					// if optimization is still active and timer fires within optimization simply restart later
					etimer_restart(&optimization->optimization_timer);
				} else {
					optimization->messages_sent = 0;
					optimization->messages_received = 0;
					optimization->messages_consecutivefailed = 0;
					optimization->optimization_active = true;
					optimization->optimization_power = optimization->power - OPTIMIZATIONSTEP;
					etimer_set(&optimization->optimization_timer, CLOCK_SECOND * COMPONENT_POWERCONTROL_ADAPTIVE_UPDATEINTERVAL);
					PRINTF("DEBUG: [powercontrol-adaptive] testing optimization: node=%s power=%d\n", networkaddr2string_buffered(optimization->node), optimization->optimization_power);
				}
			}
		}
	}

	PROCESS_END();
}

static void _etimer_timeout(struct etimer *timer, clock_time_t timeout) {
	PROCESS_CONTEXT_BEGIN(&component_powercontrol);
	etimer_set(timer, timeout);
	PROCESS_CONTEXT_END(&component_powercontrol);
}

static void _powercontrol_linklocal(const networkaddr_t *destination, int8_t txpower, int8_t num_tx, bool received) {
	power_optimization_t *optimization;
	for(optimization = list_head(list_optimizations); optimization != NULL; optimization = list_item_next(optimization)) {
		if(networkaddr_equal(optimization->node, destination)) {
			optimization->messages_sent++;
			if(received) {
				optimization->messages_received++;
				optimization->messages_consecutivefailed = 0;
			} else {
				optimization->messages_consecutivefailed++;
			}

			// decide whether the transmission power should be increased to meet QoS
			float prr = ((float) optimization->messages_received) / ((float) optimization->messages_sent);
			if(optimization->messages_consecutivefailed >= COMPONENT_POWERCONTROL_ADAPTIVE_CONSECUTIVEFAILED || (optimization->messages_sent > COMPONENT_POWERCONTROL_ADAPTIVE_MESSAGES && prr < COMPONENT_POWERCONTROL_ADAPTIVE_RATIO)) {
#if DEBUG
				if(optimization->optimization_active) {
					PRINTF("DEBUG: [powercontrol-adaptive] testing optimization failed QoS: node=%s, txpower=%d, prr=%03d%%, consecutivefialed=%d\n", networkaddr2string_buffered(optimization->node), optimization->optimization_power, (int) (prr * 100), optimization->messages_consecutivefailed);
					PRINTF("DEBUG: [powercontrol-adaptive] finished optimization: node=%s, txpower=%d\n", networkaddr2string_buffered(optimization->node), optimization->power);
				} else {
					PRINTF("DEBUG: [powercontrol-adaptive] stable power failed QoS: node=%s, txpower=%d, prr=%03d%%, consecutivefialed=%d\n", networkaddr2string_buffered(optimization->node), optimization->power, (int) (prr * 100), optimization->messages_consecutivefailed);
					PRINTF("DEBUG: [powercontrol-adaptive] new stable power: node=%s, txpower=%d\n", networkaddr2string_buffered(optimization->node), MIN(COMPONENT_RADIO_TXPOWER_MAX, optimization->power + OPTIMIZATIONSTEP));
				}
#endif

				if(!optimization->optimization_active)
					optimization->power = MIN(COMPONENT_RADIO_TXPOWER_MAX, optimization->power + OPTIMIZATIONSTEP);

				_etimer_timeout(&optimization->optimization_timer, CLOCK_SECOND * COMPONENT_POWERCONTROL_ADAPTIVE_UPDATEINTERVAL);
				optimization->messages_sent = 0;
				optimization->messages_received = 0;
				optimization->messages_consecutivefailed = 0;
				optimization->optimization_active = false;
			}

			// decide whether the transmission power can be decreased
			if(optimization->optimization_active && optimization->messages_sent > COMPONENT_POWERCONTROL_ADAPTIVE_MESSAGES && prr > COMPONENT_POWERCONTROL_ADAPTIVE_RATIO) {
				optimization->power = optimization->optimization_power;
				optimization->messages_sent = 0;
				optimization->messages_received = 0;
				optimization->messages_consecutivefailed = 0;
				optimization->optimization_power -= OPTIMIZATIONSTEP;
				PRINTF("DEBUG: [powercontrol-adaptive] testing optimization succeeded: node=%s, txpower=%d, prr=%03d%%, consecutivefialed=%d\n", networkaddr2string_buffered(optimization->node), optimization->power, (int) (prr * 100), optimization->messages_consecutivefailed);
				PRINTF("DEBUG: [powercontrol-adaptive] new stable power: node=%s, txpower=%d\n", networkaddr2string_buffered(optimization->node), optimization->power);
				PRINTF("DEBUG: [powercontrol-adaptive] testing optimization: node=%s power=%d\n", networkaddr2string_buffered(optimization->node), optimization->optimization_power);
			}
		}
	}
}

int8_t component_powercontrol_destinationtxpower(const networkaddr_t *destination) {
	power_optimization_t *optimization;
	for(optimization = list_head(list_optimizations); optimization != NULL; optimization = list_item_next(optimization)) {
		if(networkaddr_equal(optimization->node, destination)) {
			if(optimization->optimization_active)
				return optimization->optimization_power;

			return optimization->power;
		}
	}

	// check if destination should be optimized
	nexthop_t *nexthop;
	bool found = false;
	for(nexthop = list_head(component_network_nexthops_all()); nexthop != NULL; nexthop = list_item_next(nexthop))
		found |= networkaddr_equal(nexthop->address, destination);
	if(!found)
		return -1;

	// no existing optimization, start it
	if((optimization = memb_alloc(&memb_optimizations)) == NULL) {
		printf("ERROR[powercontrol-adaptive]: optimization-list is full\n");
		return -1;
	}
	optimization->node = networkaddr_reference_alloc(destination);
	optimization->power = COMPONENT_RADIO_TXPOWER_MAX;
	optimization->messages_sent = 0;
	optimization->messages_received = 0;
	optimization->messages_consecutivefailed = 0;
	_etimer_timeout(&optimization->optimization_timer, 0);
	optimization->optimization_active = false;
	list_add(list_optimizations, optimization);
	PRINTF("DEBUG: [powercontrol-adaptive] added %s\n", networkaddr2string_buffered(optimization->node));

	return -1;
}

#endif
