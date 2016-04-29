#include "uniqueid.h"

#include <stdlib.h>
#include <stdio.h>

static uint8_t counter = 0;

uint8_t uniqueid_assign() {
	if(counter == UINT8_MAX) {
		printf("ERROR[uniqueid]: all ids assigned\n");
		return 0;
	}

	return ++counter;
}
