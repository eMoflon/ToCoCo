#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include "networkaddr.h"

#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

typedef struct networkaddr_reference {
	networkaddr_t address;
	uint8_t count;
} networkaddr_reference_t;

static networkaddr_reference_t addresses[50];

static char network_string[NETWORKADDR_STRSIZE];

#if DEBUG
static char network_string_debugging[NETWORKADDR_STRSIZE];
#endif

int networkaddr_cmp(const networkaddr_t *address1, const networkaddr_t *address2) {
#if NETSTACK_CONF_WITH_IPV6
	// uip_ipaddr_cmp() has problems comparing IPs with aaaa:: and fe80:: prefix, they
	// are not binary equal, so they are normalized before comparing
	static networkaddr_t address1_normalized, address2_normalized;
	networkaddr_copy(&address1_normalized, address1);
	networkaddr_copy(&address2_normalized, address2);
	address1_normalized.u16[0] = 0xaaaa;
	address2_normalized.u16[0] = 0xaaaa;
	return memcmp(&address1_normalized, &address2_normalized, sizeof(networkaddr_t));
#else
	return memcmp(address1, address2, sizeof(networkaddr_t));
#endif
}

int networkaddr_equal(const networkaddr_t *address1, const networkaddr_t *address2) {
	return networkaddr_cmp(address1, address2) == 0;
}

void networkaddr_copy(networkaddr_t *dest, const networkaddr_t *src) {
#if NETSTACK_CONF_WITH_IPV6
	uip_ipaddr_copy(dest, src);
#else
	linkaddr_copy(dest, src);
#endif
}


void networkaddr_fromstring(networkaddr_t *dest, const char *src) {
#if NETSTACK_CONF_WITH_IPV6
	uip_ip6addr(dest, 0, 0, 0, 0, 0, 0, 0, 0);

	const char *str = src;
	int fillFromLeft = true, nextfillFromLeft = 0, nextfillFromRight = 7;

	while(str[0] != '\0') {
		// convert max. 4 char hex ip part to 16 bit integer
		uint16_t strtol = 0;
		for (;; ++str) {
			if(str[0] >= '0' && str[0] <= '9')
				strtol = (strtol << 4) | (str[0] - '0');
			else if(str[0] >= 'a' && str[0] <= 'f')
				strtol = (strtol << 4) | (str[0] - 'a' + 10);
			else
				break;
		}

		// insert ip octet from left or from right depending on actual inserting mode
		if(fillFromLeft) {
			dest->u16[nextfillFromLeft] = UIP_HTONS(strtol);
			nextfillFromLeft++;
		} else {
			int i;
			for(i = nextfillFromRight; i < 7; i++)
				dest->u16[i] = dest->u16[i + 1];
			dest->u16[7] = UIP_HTONS(strtol);
			nextfillFromRight--;
		}

		// search for next character after colon.
		// if the next character is a colon again it's a "::"-sequence
		// and we have to start filling from right as there are some
		// hextets which are zero
		if(str[0] == ':')
			str++;
		if(str[0] == ':') {
			fillFromLeft = false;
			str++;
		}
	}
#else
	dest->u8[0] = atoi(src);
	dest->u8[1] = atoi(strchr(src, '.') + 1);
#endif
}

networkaddr_t *networkaddr_node_addr() {
#if NETSTACK_CONF_WITH_IPV6
	static uip_ip6addr_t linklocal;
	uip_create_unspecified(&linklocal);

	int i, state;
	for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
		state = uip_ds6_if.addr_list[i].state;
		if(uip_ds6_if.addr_list[i].isused && (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
			// try to return only the global address, not the link-local address
			// but in unit tests there happens no routing so the link-local address
			// should be returned
			// => return global address in best-effort mode
			if(uip_ds6_if.addr_list[i].ipaddr.u16[0] != 0xaaaa) {
				networkaddr_copy(&linklocal, &uip_ds6_if.addr_list[i].ipaddr);
				continue;
			}

			return &uip_ds6_if.addr_list[i].ipaddr;
		}
	}

	if(uip_is_addr_unspecified(&linklocal))
		return NULL;
	return &linklocal;
#else
	return &linkaddr_node_addr;
#endif
}

void networkaddr_fill_random(networkaddr_t *dest) {
	uint8_t rand;
	while((rand = (random_rand() % 256)) == 0);

#if NETSTACK_CONF_WITH_IPV6
	uip_ip6addr(dest, 0xfe80, 0, 0, 0, 0, 0, 0, rand);
#else
	dest->u8[0] = rand;
	dest->u8[1] = 0;
#endif
}

networkaddr_t *networkaddr_reference_alloc(const networkaddr_t *address) {
	int i;

	for(i = 0; i < sizeof(addresses) / sizeof(addresses[0]); i++) {
		if(networkaddr_equal(&addresses[i].address, address) && addresses[i].count < 254) {
			addresses[i].count++;
			PRINTF("DEBUG: [networkaddr] increase to count=%d for address=%s at memory 0x%d\n", addresses[i].count, networkaddr2string(network_string_debugging, address), &addresses[i].address);
			return &addresses[i].address;
		}
	}

	for(i = 0; i < sizeof(addresses) / sizeof(addresses[0]); i++) {
		if(addresses[i].count == 0) {
			networkaddr_copy(&addresses[i].address, address);
			addresses[i].count = 1;
			PRINTF("DEBUG: [networkaddr] assign address=%s to memory 0x%d\n", networkaddr2string(network_string_debugging, address), &addresses[i].address);
			return &addresses[i].address;
		}
	}

	printf("ERROR[networkaddr]: no more network reference available\n");
	return NULL;
}

void networkaddr_reference_free(networkaddr_t *address) {
	int i;
	for(i = 0; i < sizeof(addresses) / sizeof(addresses[0]); i++) {
		// check for addresses[i].count > 0 because somebody could try to remove NULL reference
		if(&addresses[i].address == address && addresses[i].count > 0) {
			addresses[i].count--;
			PRINTF("DEBUG: [networkaddr] decrease to count=%d for address=%s at memory 0x%d\n", addresses[i].count, networkaddr2string(network_string_debugging, address), &addresses[i].address);

			if(addresses[i].count == 0) {
				PRINTF("DEBUG: [networkaddr] freed address=%s at memory 0x%d\n", networkaddr2string(network_string_debugging, address), &addresses[i].address);
#if NETSTACK_CONF_WITH_IPV6
				uip_ip6addr(address, 0, 0, 0, 0, 0, 0, 0, 0);
#else
				linkaddr_copy(address, &linkaddr_null);
#endif
			}
		}
	}
}

void networkaddr_compression_compress(const networkaddr_t *address, void *data) {
#if NETSTACK_CONF_WITH_IPV6
	memcpy(data, address->u8 + 8, 8);
#else
	memcpy(data, address, NETWORKADDR_SIZE_COMPRESSED);
#endif
}

void networkaddr_compression_decompress(networkaddr_t *address, const void *data) {
#if NETSTACK_CONF_WITH_IPV6
	networkaddr_copy(address, networkaddr_node_addr());
	memcpy(address->u8 + 8, data, 8);
#else
	memcpy(address, data, NETWORKADDR_SIZE_COMPRESSED);
#endif
}

char* networkaddr2string_buffered(const networkaddr_t *address) {
	return networkaddr2string(network_string, address);
}

char* networkaddr2string(char* buffer, const networkaddr_t *address) {
	if(address == NULL)
		return strcpy(buffer, "NULL");

	buffer[0] = '\0';
#if NETSTACK_CONF_WITH_IPV6
	uint8_t i, lastBlockZero = 0, finishedZeroBlock = 0;
	for(i = 0; i < 8; i++) {
		if(address->u16[i] > 0 || i == 0 || i == 7 || finishedZeroBlock) {
			sprintf(buffer + strlen(buffer), "%x", (address->u16[i] << 8) | (address->u16[i] >> 8));
			strcat(buffer, (i < 7) ? ":" : "");
		} else if(address->u16[i] == 0 && !lastBlockZero) {
			strcat(buffer, ":");
		}

		finishedZeroBlock = finishedZeroBlock || (lastBlockZero && address->u16[i] > 0);
		lastBlockZero = (address->u16[i] == 0 && i > 0);
	}
#else
	sprintf(buffer, "%d.%d", address->u8[0], address->u8[1]);
#endif
	return buffer;
}
