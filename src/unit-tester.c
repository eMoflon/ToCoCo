#include <stdio.h>
#include <string.h>
#include "contiki.h"

#include "app-conf.h"
#include "lib/unit-test.h"
#include "tests/test-topologycontrol-aktc.c"

PROCESS(unittester, "unittester");
PROCESS_THREAD(unittester, ev, data) {
    PROCESS_BEGIN();

#if COMPONENT_NETWORK == COMPONENT_NETWORK_IPV6
    printf("Unit-Test Process (network=ipv6):\n");
#else
    printf("Unit-Test Process (network=rime):\n");
#endif

	UNIT_TEST_RUN(topologycontrol_aktc)

    PROCESS_END();
}
AUTOSTART_PROCESSES(&unittester);
