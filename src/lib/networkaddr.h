#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <limits.h>
#include "contiki-net.h"

#if NETSTACK_CONF_WITH_IPV6
#define NETWORKADDR_STRSIZE 8 * 4 + 8
#else
// 3 bytes for every number of the rime address, RIMEADDR_SIZE - 1 bytes for the point separator and a NULL terminator
#define NETWORKADDR_STRSIZE LINKADDR_SIZE * 4 * CHAR_BIT
#endif

#if NETSTACK_CONF_WITH_IPV6
typedef uip_ip6addr_t networkaddr_t;
#else
typedef linkaddr_t networkaddr_t;
#endif

#if NETSTACK_CONF_WITH_IPV6
// an IPv6 address can be divided into a 64 bit subnet prefix and a 64 it interface identifier
// as we only have one ipv6-subnet we can omit the subnet prefix if compression is needed
#define NETWORKADDR_SIZE_COMPRESSED 8
#else
// there's no efficient compression for rime addresses :(
#define NETWORKADDR_SIZE_COMPRESSED LINKADDR_SIZE
#endif

int networkaddr_cmp(const networkaddr_t *address1, const networkaddr_t *address2);

int networkaddr_equal(const networkaddr_t *address1, const networkaddr_t *address2);

void networkaddr_copy(networkaddr_t *dest, const networkaddr_t *src);

void networkaddr_fromstring(networkaddr_t *dest, const char *src);

networkaddr_t *networkaddr_node_addr();

void networkaddr_fill_random(networkaddr_t *dest);

/**
 * Allocs a network address reference
 *
 * Sensor motes have small memory and have to keep much network addresses statically
 * reserved (e.g. neighbor nodes, nexthops). With IPv6 these addresses are really
 * large (16 bytes) and reserving space for them is hard under memory pressure. And
 * maybe many parts of the application will have to store the same address over and over.
 * To solve this problem one can ask the "network address reference system" to store a
 * specific network address and the system will return a pointer to the stored address.
 * The returned pointer may be used by other modules and therefore the memory is shared.
 * When you no longer need the reference you can ask the reference system to free your
 * reference and the address will be removed from memory if all created references to the
 * address have been freed.
 *
 * CAUTION: this is memory management and therefore prone to memory leaks but needed because
 * of small ram amounts and large ram usage because of ipv6
 *
 * \note for every reference created with networkaddr_reference_alloc() there has to be an call
 * to networkaddr_reference_free() to free the reference
 */
networkaddr_t *networkaddr_reference_alloc(const networkaddr_t *address);

/**
 * Frees a network address reference
 *
 * \note read networkaddr_reference_alloc()
 */
void networkaddr_reference_free(networkaddr_t *address);

void networkaddr_compression_compress(const networkaddr_t *address, void *data);

void networkaddr_compression_decompress(networkaddr_t *address, const void *data);

char* networkaddr2string_buffered(const networkaddr_t *address);

char* networkaddr2string(char* buffer, const networkaddr_t *address);

#endif /* __NETWORK_H__ */
