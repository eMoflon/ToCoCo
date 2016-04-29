#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include "../components/neighbordiscovery/neighbordiscovery-twohopbroadcast.h"
#include "../lib/unit-test.h"
#include "../lib/neighbors.h"
#include "../lib/networkaddr.h"

UNIT_TEST_START(neighbordiscovery_twohopbroadcast)

UNIT_TEST_SKIP();

/*list_t neighborlist;

// network addresses
networkaddr_t me, hop1, hop2;
networkaddr_copy(&me, networkaddr_node_addr());
networkaddr_fill_random(&hop1);
networkaddr_fill_random(&hop2);

// initial neighbor list is empty
neighborlist = twohop_broadcast_neighbors_get_all();
UNIT_TEST_ASSERT(neighbors_count_all(neighborlist) == 0)

// add 1-hop and 2-hop entry
twohop_broadcast_neighbor_t transportpacket[1];
networkaddr_compression_compress(&hop2, transportpacket[0].neighbor);
transportpacket[0].rssi_to_me = -12;
transportpacket[0].rssi_to_neighbor = -24;
twohop_broadcast_handle_broadcastpacket(&hop1, transportpacket, 1, -6);
neighborlist = twohop_broadcast_neighbors_get_all();

// neighbor list size changed correctly
UNIT_TEST_ASSERT(neighbors_count_all(neighborlist) == 2)
UNIT_TEST_ASSERT(neighbors_count_onehop(neighborlist) == 1)
UNIT_TEST_ASSERT(neighbors_count_twohop(neighborlist) == 1)

// neighborlist has correct entries
neighbor_t *entry1hop = neighbors_find_onehop_entry(neighborlist, &me, &hop1);
UNIT_TEST_ASSERT(entry1hop != NULL)
UNIT_TEST_ASSERT(networkaddr_equal(entry1hop->node1, &me));
UNIT_TEST_ASSERT(networkaddr_equal(entry1hop->node2, &hop1));
UNIT_TEST_ASSERT(entry1hop->rssi_node1_to_node2 == COMPONENT_RADIO_RSSIUNKNOWN)
UNIT_TEST_ASSERT(entry1hop->rssi_node2_to_node1 == -6)
UNIT_TEST_ASSERT(entry1hop->ttl_node1_to_node2 == NEIGHBORDISCOVERY_TWOHOPBROADCAST_LINKTTL)
UNIT_TEST_ASSERT(entry1hop->ttl_node2_to_node1 == NEIGHBORDISCOVERY_TWOHOPBROADCAST_LINKTTL)
neighbor_t *entry2hop = neighbors_find_twohop_entry(neighborlist, &hop1, &hop2);
UNIT_TEST_ASSERT(entry2hop != NULL)
UNIT_TEST_ASSERT(networkaddr_equal(entry2hop->node1, &hop1))
UNIT_TEST_ASSERT(networkaddr_equal(entry2hop->node2, &hop2));
UNIT_TEST_ASSERT(entry2hop->rssi_node1_to_node2 == -24)
UNIT_TEST_ASSERT(entry2hop->rssi_node2_to_node1 == -12)
UNIT_TEST_ASSERT(entry2hop->ttl_node1_to_node2 == NEIGHBORDISCOVERY_TWOHOPBROADCAST_LINKTTL)
UNIT_TEST_ASSERT(entry2hop->ttl_node2_to_node1 == NEIGHBORDISCOVERY_TWOHOPBROADCAST_LINKTTL)

// test broadcast packet
twohop_broadcast_fill_broadcastpacket(transportpacket);
uint8_t hop1_compressed[NETWORKADDR_SIZE_COMPRESSED];
networkaddr_compression_compress(&hop1, hop1_compressed);
UNIT_TEST_ASSERT(memcmp(transportpacket[0].neighbor, hop1_compressed, NETWORKADDR_SIZE_COMPRESSED) == 0);
UNIT_TEST_ASSERT(transportpacket[0].rssi_to_me == -6);
UNIT_TEST_ASSERT(transportpacket[0].rssi_to_neighbor = COMPONENT_RADIO_RSSIUNKNOWN);

// link decay decreases TTL
twohop_broadcast_decay_links();
neighborlist = twohop_broadcast_neighbors_get_all();
UNIT_TEST_ASSERT(entry1hop->ttl_node1_to_node2 == NEIGHBORDISCOVERY_TWOHOPBROADCAST_LINKTTL - 1);
UNIT_TEST_ASSERT(entry1hop->ttl_node2_to_node1 == NEIGHBORDISCOVERY_TWOHOPBROADCAST_LINKTTL - 1);
UNIT_TEST_ASSERT(entry2hop->ttl_node1_to_node2 == NEIGHBORDISCOVERY_TWOHOPBROADCAST_LINKTTL - 1);
UNIT_TEST_ASSERT(entry2hop->ttl_node2_to_node1 == NEIGHBORDISCOVERY_TWOHOPBROADCAST_LINKTTL - 1);

// neighbors are (only) removed when both ttl counters reach zero
entry2hop->ttl_node1_to_node2 += 1;
while(entry1hop->ttl_node1_to_node2 > 0) {
	twohop_broadcast_decay_links();
	neighborlist = twohop_broadcast_neighbors_get_all();
}

UNIT_TEST_ASSERT(neighbors_count_all(neighborlist) == 1);
UNIT_TEST_ASSERT(neighbors_count_onehop(neighborlist) == 0)
UNIT_TEST_ASSERT(neighbors_count_twohop(neighborlist) == 1)
twohop_broadcast_decay_links();
neighborlist = twohop_broadcast_neighbors_get_all();
UNIT_TEST_ASSERT(neighbors_count_all(neighborlist) == 0);*/

UNIT_TEST_END()
