#ifndef __NETWORK_RIME_H_
#define __NETWORK_RIME_H_

#include "../../app-conf.h"

#ifndef COMPONENT_NETWORK_RIME_BASESTATION
	#error you have to provide a COMPONENT_NETWORK_RIME_BASESTATION
#endif

extern const struct network_driver modified_rime_driver;

void network_rime_init();

#endif /* __NETWORK_RIME_H_ */
