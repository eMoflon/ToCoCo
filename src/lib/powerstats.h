#ifndef __POWERSTATS_H_
#define __POWERSTATS_H_

#include "contiki.h"

#include "components.h"

#define POWERSTATS_TICKS_PER_SECOND RTIMER_SECOND

typedef struct powerstats {
	unsigned long cpu_active;
	unsigned long cpu_lpm;
	unsigned long cpu_irq;
	unsigned long radio_transmit;
	unsigned long radio_transmit_powelevels[COMPONENT_RADIO_TXPOWER_MAX - COMPONENT_RADIO_TXPOWER_MIN + 1];
	unsigned long radio_listen;
} powerstats_t;

void powerstats_init();

powerstats_t *powerstats_now();

void powerstats_print(char *label);

#endif /* __POWERSTATS_H_ */
