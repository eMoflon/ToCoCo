#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "contiki.h"
#include "contiki-lib.h"
#include "lib/crc16.h"
#include "sys/node-id.h"
#include "dev/watchdog.h"
#if CONTIKI_TARGET_SKY || CONTIKI_TARGET_Z1 || CONTIKI_TARGET_WISMOTE
	#include "dev/battery-sensor.h"
	#include "dev/sht11/sht11-sensor.h"
	#define RANDOMSEED_HAS_BATTERY 1
	#define RANDOMSEED_HAS_SHT11 1
#elif CONTIKI_TARGET_COOJA
	// cooja's random source will be the SEEDSTRING compile time, because for every run this file will be re-compiled
#else
	#error no random seed source information for target platform
#endif

#include "../app-conf.h"
#include "utilities.h"

#if UTILITIES_CONTIKIRANDOM_SEED == UTILITIES_CONTIKIRANDOM_SEED_RANDOM
#if defined(__DATE__) && defined(__TIME__)
#define SEEDSTRING __DATE__ " " __TIME__
#else
#error preprocessor macro "__DATE__" or "__TIME__" for UTILITIES_CONTIKIRANDOM_SEED_RANDOMEVERYCOMPILE not available with compiler
#endif
#endif

// cooja should save the seed to this variable, but it's currently broken (maybe it is working again any day...)
uint16_t rseed = 0;

void contikirandom_init() {
#if UTILITIES_CONTIKIRANDOM_SEED == UTILITIES_CONTIKIRANDOM_SEED_STATIC
	uint16_t seed = node_id ^ rseed ^ UTILITIES_CONTIKIRANDOM_STATICSEED;
#else
	// collect random information to have run-time different seeds with same firmware
	uint16_t random_information = 0xFFFF;
	int i;
	for(i = 0; i < 10; i++) {
		watchdog_stop();

#if RANDOMSEED_HAS_BATTERY
		bool sensor_on_battery = (battery_sensor.status(SENSORS_ACTIVE) == 1);
		if(!sensor_on_battery)
			SENSORS_ACTIVATE(battery_sensor);

		random_information = crc16_add(battery_sensor.value(0), random_information);

		if(!sensor_on_battery)
			SENSORS_DEACTIVATE(battery_sensor);
#endif
#if RANDOMSEED_HAS_SHT11
		bool sensor_on_sht11 = (sht11_sensor.status(SENSORS_ACTIVE) == 1);
		if(!sensor_on_sht11)
			SENSORS_ACTIVATE(sht11_sensor);

		random_information = crc16_add(sht11_sensor.value(SHT11_SENSOR_BATTERY_INDICATOR), random_information);
		random_information = crc16_add(sht11_sensor.value(SHT11_SENSOR_TEMP), random_information);
		random_information = crc16_add(sht11_sensor.value(SHT11_SENSOR_HUMIDITY), random_information);

		if(!sensor_on_sht11)
			SENSORS_DEACTIVATE(sht11_sensor);
#endif

		watchdog_start();
	}

	uint16_t seed = node_id ^ rseed ^ crc16_data((unsigned char *) SEEDSTRING, strlen(SEEDSTRING), 0xFFFF) ^ random_information;
#endif

	random_init(seed);
}

uint32_t random(uint32_t min, uint32_t max) {
	// it's hard to generate unbiased numbers and really depends on the quality
	// of the underlying random generator's quality.
	// http://stackoverflow.com/a/5009006 is a simple and ROM efficient solution
	uint32_t rand = (((uint32_t) random_rand()) << 16) | random_rand();
	return min + (rand % (max - min + 1));
}
