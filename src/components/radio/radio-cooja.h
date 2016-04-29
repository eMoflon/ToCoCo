#ifndef __RADIO_COOJA_H_
#define __RADIO_COOJA_H_

#include <stdint.h>

#include "../../app-conf.h"

#if COMPONENT_RADIO == COMPONENT_RADIO_COOJA

#ifndef COMPONENT_RADIO_TXPOWER_MIN
	#define COMPONENT_RADIO_TXPOWER_MIN 0
#endif
#ifndef COMPONENT_RADIO_TXPOWER_MAX
	#define COMPONENT_RADIO_TXPOWER_MAX 100
#endif

#endif

#endif /* __RADIO_COOJA_H_ */
