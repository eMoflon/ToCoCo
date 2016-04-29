#include <stdint.h>

#include "radio-cc2520.h"
#include "../../app-conf.h"
#include "../../lib/components.h"

#if COMPONENT_RADIO == COMPONENT_RADIO_CC2520

#include "dev/cc2520/cc2520.h"

int8_t components_radio_lastrssi() {
	// information from CC2520 datasheet (v. 19 Dec 2007):
	// * RSSI range: –100 dBm to 0 dBm
	// * RSSI accuracy: +-4dBM
	// * RSSI offset: approximately -76 (received power P(dBm) = RSSI_VAL + RSSI_OFFSET)

	int8_t rssi = cc2520_last_rssi - 76;
	rssi = (rssi >= -100) ? rssi : -100;
	rssi = (rssi <=    0) ? rssi : 0;

	return rssi;
}

void components_radio_txpower_set(int8_t txpower) {
	cc2520_set_txpower(txpower);
}

int8_t components_radio_txpower_get() {
	return cc2520_get_txpower();
}

#endif
