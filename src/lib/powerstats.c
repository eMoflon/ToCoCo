#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "contiki.h"
#include "contiki-net.h"

#include "powerstats.h"
#include "components.h"

static powerstats_t _powerstats_stats;

static void _powerstats_outputsniffer(int mac_status);
static void _powerstats_inputsniffer(void);
RIME_SNIFFER(_powerstats_rimesniffer, _powerstats_inputsniffer, _powerstats_outputsniffer);

void powerstats_init() {
	rime_sniffer_add(&_powerstats_rimesniffer);

	uint8_t i;
	for(i = 0; i < COMPONENT_RADIO_TXPOWER_MAX - COMPONENT_RADIO_TXPOWER_MIN + 1; i++)
		_powerstats_stats.radio_transmit_powelevels[i] = 0;
}

powerstats_t *powerstats_now() {
	energest_flush();
	_powerstats_stats.cpu_active = energest_type_time(ENERGEST_TYPE_CPU);
	_powerstats_stats.cpu_lpm = energest_type_time(ENERGEST_TYPE_LPM);
	_powerstats_stats.cpu_irq = energest_type_time(ENERGEST_TYPE_IRQ);
	_powerstats_stats.radio_transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT);
	// stats.radio_transmit_powelevels is set by output_printsniffer
	_powerstats_stats.radio_listen = energest_type_time(ENERGEST_TYPE_LISTEN);

	return &_powerstats_stats;
}

void powerstats_print(char *label) {
	powerstats_t *stats = powerstats_now();
	int8_t powerlevel;
	bool printed_powerlevel = false;

#if CONTIKI_TARGET_SKY
	printf("[%s] energy-power: cpu_active=1.8, cpu_lpm=0.0545, cpu_irq=1.8, radio_listen=19.7, radio_transmit=[0:7.525,1:7.85,2:8.175,3:8.5,4:8.85,5:9.2,6:9.55,7:9.9,8:10.225,9:10.55,10:10.875,11:11.2,12:11.525,13:11.85,14:12.175,15:12.5,16:12.85,17:13.2,18:13.55,19:13.9,20:14.225,21:14.55,22:14.875,23:15.2,24:15.525,25:15.85,26:16.175,27:16.5,28:16.725,29:16.95,30:17.175,31:17.4]\n", (label == NULL) ? "powerstats" : label);
#else
	#error energy profile unknown for mote type
#endif
	printf("[%s] energy-ticks: ticks_per_second=%u, cpu_active=%lu, cpu_lpm=%lu, cpu_irq=%lu, radio_listen=%lu, radio_transmit(%d)=%lu[",
		(label == NULL) ? "powerstats" : label,
		POWERSTATS_TICKS_PER_SECOND,
		stats->cpu_active,
		stats->cpu_lpm,
		stats->cpu_irq,
		stats->radio_listen,
		COMPONENT_RADIO_TXPOWER_MAX,
		stats->radio_transmit
	);
	for(powerlevel = COMPONENT_RADIO_TXPOWER_MIN; powerlevel <= COMPONENT_RADIO_TXPOWER_MAX; powerlevel++) {
		if(stats->radio_transmit_powelevels[powerlevel - COMPONENT_RADIO_TXPOWER_MIN] == 0)
			continue;

		if(printed_powerlevel)
			printf(",");

		printed_powerlevel = true;
		printf("%d:%lu", powerlevel, stats->radio_transmit_powelevels[powerlevel - COMPONENT_RADIO_TXPOWER_MIN]);
	}
	printf("]\n");
}

static void _powerstats_outputsniffer(int mac_status) {
	uint16_t txtime = packetbuf_attr(PACKETBUF_ATTR_TRANSMIT_TIME);
	uint16_t txpwr =  packetbuf_attr(PACKETBUF_ATTR_RADIO_TXPOWER);

	if(txpwr < COMPONENT_RADIO_TXPOWER_MIN || txpwr > COMPONENT_RADIO_TXPOWER_MAX) {
		printf("ERROR[powerstats]: txpower %u is out of range [%d, %d]\n", txpwr, COMPONENT_RADIO_TXPOWER_MIN, COMPONENT_RADIO_TXPOWER_MAX);
		return;
	}

	_powerstats_stats.radio_transmit_powelevels[txpwr - COMPONENT_RADIO_TXPOWER_MIN] += txtime;
}

static void _powerstats_inputsniffer(void) {
}
