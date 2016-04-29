#include <stdlib.h>
#include "contiki.h"
#include "lib/list.h"

#include "../../app-conf.h"
#include "../../lib/boot.h"

#if COMPONENT_APPLICATION == COMPONENT_APPLICATION_NULL

PROCESS(component_application, "application: null");
PROCESS_THREAD(component_application, ev, data) {
	PROCESS_BEGIN();

	BOOT_COMPONENT_WAIT(component_application);

	PROCESS_END();
}
#endif
