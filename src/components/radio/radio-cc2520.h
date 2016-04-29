#ifndef __RADIO_CC2520_H_
#define __RADIO_CC2520_H_

#include <stdint.h>
#include "dev/cc2520/cc2520.h"

#include "../../app-conf.h"

#if COMPONENT_RADIO == COMPONENT_RADIO_CC2520

// bugfix: cc2520_transmit checks for a PACKETBUF_ATTR_RADIO_TXPOWER > 0 for setting a packet specific transmission power,
//         therefore a packet specific txpower of CC2520_TXPOWER_MIN = 0 txpower will not be set and the packet will be
//         send with the actual (mostly much higher) transmission power of the cc2520 radio module.
#define COMPONENT_RADIO_TXPOWER_MIN 1
#define COMPONENT_RADIO_TXPOWER_MAX CC2520_TXPOWER_MAX

#endif

#endif /* __RADIO_CC2520_H_ */
