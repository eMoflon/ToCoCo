#ifndef __RADIO_CC2420_H_
#define __RADIO_CC2420_H_

#include <stdint.h>
#include "dev/cc2420/cc2420.h"

#include "../../app-conf.h"

#if COMPONENT_RADIO == COMPONENT_RADIO_CC2420

// bugfix: cc2420_transmit checks for a PACKETBUF_ATTR_RADIO_TXPOWER > 0 for setting a packet specific transmission power,
//         therefore a packet specific txpower of CC2420_TXPOWER_MIN = 0 txpower will not be set and the packet will be
//         send with the actual (mostly much higher) transmission power of the cc2420 radio module.
#define COMPONENT_RADIO_TXPOWER_MIN 1
#define COMPONENT_RADIO_TXPOWER_MAX CC2420_TXPOWER_MAX

#endif

#endif /* __RADIO_CC2420_H_ */
