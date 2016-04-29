#include <stdint.h>

#include "radio-cc2420.h"
#include "../../app-conf.h"
#include "../../lib/components.h"

#if COMPONENT_RADIO == COMPONENT_RADIO_CC2420

#include "dev/cc2420/cc2420.h"

int8_t components_radio_lastrssi() {
	// information from CC2420 datasheet (v. 07 Mar 2013):
	// * RSSI range: –100 dBm to 0 dBm
	// * RSSI accuracy: +-6dBM

	int8_t rssi = cc2420_last_rssi;
	rssi = (rssi >= -100) ? rssi : -100;
	rssi = (rssi <=    0) ? rssi : 0;

	return rssi;
}

void components_radio_txpower_set(int8_t txpower) {
	cc2420_set_txpower(txpower);
}

int8_t components_radio_txpower_get() {
	return cc2420_get_txpower();
}

#endif
