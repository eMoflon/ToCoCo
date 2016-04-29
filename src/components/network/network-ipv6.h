#ifndef __NETWORK_IPV6_H_
#define __NETWORK_IPV6_H_

#include "../../app-conf.h"

#ifndef COMPONENT_NETWORKSTACK_IPV6_ROOTNODE
#error node id which will create the RPL tree has to be set
#endif

void network_ipv6_init();

#endif /* __NETWORK_IPV6_H_ */
