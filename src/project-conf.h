#ifndef __PROJECT_CONF_H__
#define __PROJECT_CONF_H__

#include <stdint.h>

#include "app-conf.h"

// uncomment in case msp430-gcc overflowed your rom-size and you want to test the code with cooja
// (1. profiling for transmit power levels will not work)
// (2. will disable some power saving features)
// (3. for testbed compilation use the commercial IAR Workbench)
#ifdef ROM_OVERFLOWED
	#undef SICSLOWPAN_CONF_COMPRESSION
	#define SICSLOWPAN_CONF_COMPRESSION 0
	#undef PROCESS_CONF_NO_PROCESS_NAMES
	#define PROCESS_CONF_NO_PROCESS_NAMES 1
#endif

// packetbuf saves txpower for any packet. if packet attribute access is not inlined the packetbuf_attr_clear function
// can be overwritten to set specific power levels for every packet (even internal routing) with the help of
// component_powercontrol_destinationtxpower(networkaddr_t *destination)
#undef PACKETBUF_CONF_ATTRS_INLINE
#define PACKETBUF_CONF_ATTRS_INLINE 0

// force per-packet power profiling that systemstats can generate statistics how long the radio module has been in transmit
// phase for specific power levels
#undef CONTIKIMAC_CONF_COMPOWER
#define CONTIKIMAC_CONF_COMPOWER 1
#undef XMAC_CONF_COMPOWER
#define XMAC_CONF_COMPOWER 1
#undef CXMAC_CONF_COMPOWER
#define CXMAC_CONF_COMPOWER 1

// disables tcp functionality (saves ~3KB)
#if NETSTACK_CONF_WITH_IPV6
	#undef UIP_CONF_TCP
	#define UIP_CONF_TCP 0
#endif

// use RPL's parent selection which uses link metrics if no power control is active
// and the hopcount if power control is active. Because link metrics will interfere
// with power control which will make the link quality "worse" to use the least needed
// energy
#if COMPONENT_POWERCONTROL == COMPONENT_POWERCONTROL_NULL
	#define RPL_CONF_OF rpl_mrhof
#else
	#define RPL_CONF_OF rpl_of0
#endif

// contiki's routing table for rime is really small
#ifndef ROUTE_CONF_ENTRIES
	#define ROUTE_CONF_ENTRIES 50
#endif

// sensible default for a route's lifetime
#ifndef ROUTE_CONF_DEFAULT_LIFETIME
	#define ROUTE_CONF_DEFAULT_LIFETIME UINT8_MAX // 4.25min
#endif

// use modified rime driver which allows dropping links
#if COMPONENT_NETWORK == COMPONENT_NETWORK_RIME || (COMPONENT_NETWORK == COMPONENT_NETWORK_MAKEFILE && !NETSTACK_CONF_WITH_IPV6)
	#undef NETSTACK_CONF_NETWORK
	#define NETSTACK_CONF_NETWORK modified_rime_driver
#endif

#if COMPONENT_NEIGHBORDISCOVERY_EDGEWEIGHT == COMPONENT_NEIGHBORDISCOVERY_EDGEWEIGHT_DISTANCE && !defined(COMPONENT_NEIGHBORDISCOVERY_EDGEWEIGHT_DISTANCE_POSITIONS)
	#error COMPONENT_NEIGHBORDISCOVERY_EDGEWEIGHT_DISTANCE_POSITIONS is required for distance edge weights
#endif
#if UTILITIES_CONTIKIRANDOM_SEED != UTILITIES_CONTIKIRANDOM_SEED_STATIC && UTILITIES_CONTIKIRANDOM_SEED != UTILITIES_CONTIKIRANDOM_SEED_RANDOM
	#error no valid random seed mode set
#endif
#if COMPONENT_NETWORK == COMPONENT_NETWORK_MAKEFILE
	#undef COMPONENT_NETWORK
	#if NETSTACK_CONF_WITH_IPV6
		#define COMPONENT_NETWORK COMPONENT_NETWORK_IPV6
	#else
		#define COMPONENT_NETWORK COMPONENT_NETWORK_RIME
	#endif
#endif
#if !NETSTACK_CONF_WITH_IPV6 && COMPONENT_NETWORK == COMPONENT_NETWORK_IPV6
	#error Makefile flag "NETWORK_IPV6" has to be activated for network=ipv6
#endif
#if NETSTACK_CONF_WITH_IPV6 && COMPONENT_NETWORK == COMPONENT_NETWORK_RIME
	#error Makefile flag "NETWORK_IPV6" has to be deactivated for network=rime
#endif
#if COMPONENT_RADIO == COMPONENT_RADIO_AUTODETECT
	#if CONTIKI_TARGET_SKY || CONTIKI_TARGET_Z1
		#undef COMPONENT_RADIO
		#define COMPONENT_RADIO COMPONENT_RADIO_CC2420
	#elif CONTIKI_TARGET_WISMOTE
		#undef COMPONENT_RADIO
		#define COMPONENT_RADIO COMPONENT_RADIO_CC2520
	#elif CONTIKI_TARGET_COOJA
		#undef COMPONENT_RADIO
		#define COMPONENT_RADIO COMPONENT_RADIO_COOJA
	#else
		#error radio device for platform can not be autodetected
	#endif
#endif

#endif /* __PROJECT_CONF_H__ */
