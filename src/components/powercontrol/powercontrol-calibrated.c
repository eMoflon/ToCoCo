#include <stdio.h>
#include "contiki.h"

#include "powercontrol-calibrated.h"
#include "../../app-conf.h"
#include "../../lib/boot.h"
#include "../../lib/components.h"
#include "../../lib/networkaddr.h"

#if COMPONENT_POWERCONTROL == COMPONENT_POWERCONTROL_CALIBRATED

#if !defined(COMPONENT_POWERCONTROL_CALIBRATED_MATRIX_SIZE) || !defined(COMPONENT_POWERCONTROL_CALIBRATED_MATRIX_MOTES) || !defined(COMPONENT_POWERCONTROL_CALIBRATED_MATRIX_POWERS)
	#error power calibration matrix information must  be provided
#endif

static const networkaddr_t matrix_addresses[] = COMPONENT_POWERCONTROL_CALIBRATED_MATRIX_MOTES;
static const int8_t matrix_txpower[][COMPONENT_POWERCONTROL_CALIBRATED_MATRIX_SIZE] = COMPONENT_POWERCONTROL_CALIBRATED_MATRIX_POWERS;
static int8_t matrix_source = -1;

PROCESS(component_powercontrol, "powercontrol: calibrated");
PROCESS_THREAD(component_powercontrol, ev, data) {
	PROCESS_BEGIN();

	// find matrix row for this mote
	uint8_t i;
	for(i = 0; i < COMPONENT_POWERCONTROL_CALIBRATED_MATRIX_SIZE; i++) {
		if(networkaddr_equal(networkaddr_node_addr(), &matrix_addresses[i]))
			matrix_source = i;
	}
	if(matrix_source == -1)
		printf("ERROR[powercontrol-calibrated]: no power optimizations for mote %s\n", networkaddr2string_buffered(networkaddr_node_addr()));

	BOOT_COMPONENT_WAIT(component_powercontrol);

	PROCESS_END();
}

int8_t components_powercontrol_destinationtxpower(const networkaddr_t *destination) {
	uint8_t matrix_destination = -1, i;
	for(i = 0; i < COMPONENT_POWERCONTROL_CALIBRATED_MATRIX_SIZE; i++) {
		if(networkaddr_equal(destination, &matrix_addresses[i]))
			matrix_destination = i;
	}

	// no known optimization
	if(matrix_source == -1 || matrix_destination == -1 || matrix_txpower[matrix_source][matrix_destination] == -1)
		return -1;

	return MIN(COMPONENT_RADIO_TXPOWER_MAX, matrix_txpower[matrix_source][matrix_destination] + ((int8_t) (COMPONENT_POWERCONTROL_CALIBRATED_SAFETYMARGIN * COMPONENT_RADIO_TXPOWER_MAX)));
}

#endif
