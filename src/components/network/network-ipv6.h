#ifndef __NETWORK_IPV6_H_
#define __NETWORK_IPV6_H_

#include "../../app-conf.h"

#ifndef COMPONENT_NETWORK_IPV6_BASESTATION
	#error you have to provide a COMPONENT_NETWORK_IPV6_BASESTATION
#endif

void network_ipv6_init();

#endif /* __NETWORK_IPV6_H_ */
