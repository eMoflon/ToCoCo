#include <stdlib.h>
#include "contiki.h"

#include "../../app-conf.h"
#include "../../lib/boot.h"
#include "../../lib/components.h"

#if COMPONENT_POWERCONTROL == COMPONENT_POWERCONTROL_NULL
PROCESS(component_powercontrol, "powercontrol: null");
PROCESS_THREAD(component_powercontrol, ev, data) {
	PROCESS_BEGIN();

	BOOT_COMPONENT_WAIT(component_powercontrol);

	PROCESS_END();
}

int8_t component_powercontrol_destinationtxpower(const networkaddr_t *destination) {
	return -1;
}

#endif
