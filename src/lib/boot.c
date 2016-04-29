#include <stdlib.h>
#include <stdio.h>
#include "contiki.h"

#include "boot.h"

void __process_post(void *p, process_event_t ev, process_data_t data) {
	process_post((struct process *) p, ev, data);
}
