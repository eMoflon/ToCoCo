#include <stdint.h>

#include "radio-cc2420.h"
#include "../../app-conf.h"
#include "../../lib/components.h"

#if COMPONENT_RADIO == COMPONENT_RADIO_CC2420

#include "dev/cc2420/cc2420.h"

static int8_t actual_txpower = 0;

int8_t component_radio_lastrssi() {
	// information from CC2420 datasheet (v. 07 Mar 2013):
	// * RSSI range: –100 dBm to 0 dBm
	// * RSSI accuracy: +-6dBM

	int8_t rssi = cc2420_last_rssi;
	rssi = (rssi >= -100) ? rssi : -100;
	rssi = (rssi <=    0) ? rssi : 0;

	return rssi;
}

void component_radio_txpower_set(int8_t txpower) {
	printf("SetPowerLevel %d\n",txpower);
	cc2420_set_txpower(actual_txpower = txpower);
	printf("GetPowerLevel %d\n",cc2420_get_txpower());
}

int8_t component_radio_txpower_get() {
	/**
	 * bugfix: cc2420_get_txpower() sometimes returns false power levels (txpower = 0), so the
	 * actual power level is not read from the radio module but remembered by code
	 */
	return actual_txpower;
}

#endif
