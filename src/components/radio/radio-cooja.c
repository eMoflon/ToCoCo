#include <stdint.h>

#include "../../app-conf.h"
#include "../../lib/components.h"

#if COMPONENT_RADIO == COMPONENT_RADIO_COOJA

#include "cooja-radio.h"

static int8_t radiotxpower = -1;

int8_t components_radio_lastrssi() {
	// there's no specification on the rssi range of the cooja radio
	// but in tests the range has been approx. [-94, -12]. The range
	// will be set to be in [-100, 0] which is exactly the specfication
	// of the cc2420 and cc2520

	int8_t rssi = radio_signal_strength_last();
	rssi = (rssi >= -100) ? rssi : -100;
	rssi = (rssi <=    0) ? rssi : 0;

	return rssi;
}

void components_radio_txpower_set(int8_t txpower) {
	radio_set_txpower(radiotxpower = txpower);
}

int8_t components_radio_txpower_get() {
	return radiotxpower;
}

#endif
