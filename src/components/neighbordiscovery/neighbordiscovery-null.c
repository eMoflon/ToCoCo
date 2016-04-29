#include <stdlib.h>
#include "contiki.h"
#include "lib/list.h"

#include "../../app-conf.h"
#include "../../lib/boot.h"
#include "../../lib/components.h"

#if COMPONENT_NEIGHBORDISCOVERY == COMPONENT_NEIGHBORDISCOVERY_NULL

LIST(list_neighbors);

PROCESS(component_neighbordiscovery, "neighbor-discovery: null");
PROCESS_THREAD(component_neighbordiscovery, ev, data) {
	PROCESS_BEGIN();

	list_init(list_neighbors);

	BOOT_COMPONENT_WAIT(component_neighbordiscovery);

	PROCESS_END();
}

list_t components_neighbordiscovery_neighbors() {
	return list_neighbors;
}

#endif
