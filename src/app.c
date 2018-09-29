#include <stdio.h>
#include "contiki.h"
#include "contiki-net.h"

#include "lib/boot.h"
#include "lib/evaluation.h"
#include "lib/powerstats.h"
#include "lib/utilities.h"

#ifndef APP_PRINT_EVALUATION
#define APP_PRINT_EVALUATION 1
#endif

#ifndef APP_GIT_VERSION
#define APP_GIT_VERSION "unknown"
#endif

#define DESIRED_POWER_LEVEL 30

extern struct process component_application;
extern struct process component_neighbordiscovery;
extern struct process component_network;
extern struct process component_powercontrol;
extern struct process component_topologycontrol;

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

void broadcast_processevent(process_event_t ev, void *data) {
	// process event queue is really small, posting events to all processes
	// will overflow the queue and events are thrown away. So posting happens
	// synchronous as the event queue is never used.
	process_post_synch(&component_application,       ev, data);
	process_post_synch(&component_neighbordiscovery, ev, data);
	process_post_synch(&component_network,           ev, data);
	process_post_synch(&component_powercontrol,      ev, data);
	process_post_synch(&component_topologycontrol,   ev, data);
}

PROCESS(mainprocess, "app-" APP_GIT_VERSION);
PROCESS_THREAD(mainprocess, ev, data) {
    PROCESS_BEGIN();
    // start power profiling
    powerstats_init();

    // initializes contiki random function with a really "random" seed
    contikirandom_init();

    // one evaluation profile before booting to have a profile for minute = 0 (because booting ipv6 takes a long time)
    static struct etimer etimer_evaluation;
    etimer_set(&etimer_evaluation, CLOCK_SECOND * 60);
#if APP_PRINT_EVALUATION
    		evaluation_print();
#endif

    // boot components
    static struct etimer timer_sendboot;
    etimer_set(&timer_sendboot, CLOCK_SECOND * 2); // when setting the timer to every seconds it triggers every few milliseconds...
    static uint8_t bootedApplication = 0, bootedNeigbordiscovery = 0, bootedNetwork = 0, bootedPowercontrol = 0, bootedTopologycontrol = 0;
    while(!bootedApplication || !bootedNeigbordiscovery || !bootedNetwork || !bootedPowercontrol || !bootedTopologycontrol) {
    	PROCESS_WAIT_EVENT_UNTIL(ev == BOOT_COMPONENT_COMPLETE || etimer_expired(&timer_sendboot));

    	bootedApplication      |= (ev == BOOT_COMPONENT_COMPLETE && data == &component_application);
    	bootedNeigbordiscovery |= (ev == BOOT_COMPONENT_COMPLETE && data == &component_neighbordiscovery);
    	bootedNetwork          |= (ev == BOOT_COMPONENT_COMPLETE && data == &component_network);
    	bootedPowercontrol     |= (ev == BOOT_COMPONENT_COMPLETE && data == &component_powercontrol);
    	bootedTopologycontrol  |= (ev == BOOT_COMPONENT_COMPLETE && data == &component_topologycontrol);

    	// broadcast the inprogress event regulary because components may not react on one or more events because
    	// they have internal timers running in their initialization phase and waiting only for their expiration
    	broadcast_processevent(BOOT_SYSTEM_INPROGRESS, &mainprocess);
    	if(etimer_expired(&timer_sendboot))
    		etimer_reset(&timer_sendboot);

    	PRINTF("DEBUG: [mainprocess] booting status: application=%d, neighbordiscovery=%d, network=%d, powercontrol=%d, topologycontrol=%d\n",bootedApplication, bootedNeigbordiscovery, bootedNetwork, bootedPowercontrol, bootedTopologycontrol);
    }

    broadcast_processevent(BOOT_SYSTEM_COMPLETE, &mainprocess);
    PRINTF("DEBUG: [mainprocess] app booting complete...\n");
	printf("SetPowerLevel %d\n",DESIRED_POWER_LEVEL);
	cc2420_set_txpower(DESIRED_POWER_LEVEL);
	printf("GetPowerLevel %d\n",cc2420_get_txpower());
    while(1) {
    	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&etimer_evaluation));
    	etimer_reset(&etimer_evaluation);

#if APP_PRINT_EVALUATION
    		evaluation_print();
#endif
    }

    PROCESS_END();
}
AUTOSTART_PROCESSES(
	&mainprocess,
	&component_application,
	&component_neighbordiscovery,
	&component_network,
	&component_powercontrol,
	&component_topologycontrol
);
