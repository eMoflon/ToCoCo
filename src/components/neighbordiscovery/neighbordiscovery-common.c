#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "contiki.h"
#include "contiki-lib.h"

#include "neighbordiscovery-common.h"
#include "../../lib/components.h"
#include "../../lib/neighbors.h"
#include "../../lib/networkaddr.h"

MEMB(memb_attributes, neighbor_attribute_t, COMPONENTS_NEIGHBORDISCOVERY_NEIGHBORATTRIBUTE_MEMORY);
LIST(list_attributes);

#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

void _clean_attributes_of_unknown_nodes();
void _print_attributes();
void _remove_attribute(neighbor_attribute_t *attribute);

void neighbordiscovery_common_init() {
	memb_init(&memb_attributes);
	list_init(list_attributes);
}

list_t components_neighbordiscovery_attributes_all() {
	_clean_attributes_of_unknown_nodes();

	return list_attributes;
}

void components_neighbordiscovery_attributes_add(const networkaddr_t *node, uint8_t type, uint8_t length, const void *data) {
	if(length > COMPONENTS_NEIGHBORDISCOVERY_NEIGHBORATTRIBUTE_MAXDATALENGTH) {
		printf("ERROR[neighbordiscovery-common]: maximum attribute data length is set to %d bytes\n", COMPONENTS_NEIGHBORDISCOVERY_NEIGHBORATTRIBUTE_MAXDATALENGTH);
		return;
	}

	// removing any attribute of an unknown node may get back the needed space for the new attribute
	if(memb_numfree(&memb_attributes) == 0) {
		_clean_attributes_of_unknown_nodes();
	}

	neighbor_attribute_t *attribute;
	for(attribute = list_head(list_attributes); attribute != NULL; attribute = list_item_next(attribute)) {
		if(!networkaddr_equal(attribute->node, node))
			continue;
		if(attribute->type != type)
			continue;
		if(attribute->length != length)
			continue;
		if(memcmp(attribute->data, data, attribute->length) != 0)
			continue;

		// exactly same attribute
		goto debug;
	}

	if((attribute = memb_alloc(&memb_attributes)) != NULL) {
		attribute->node = networkaddr_reference_alloc(node);
		attribute->type = type;
		attribute->length = length;
		memset(attribute->data, 0x0, COMPONENTS_NEIGHBORDISCOVERY_NEIGHBORATTRIBUTE_MAXDATALENGTH);
		memcpy(attribute->data, data, length);
		list_add(list_attributes, attribute);
	} else {
		printf("ERROR[neighbordiscovery-common]: attribute space is full\n");
	}

	debug:
		_print_attributes();
}

void components_neighbordiscovery_attributes_remove(neighbor_attribute_t *attribute) {
	neighbor_attribute_t *saved_attribute;
	for(saved_attribute = list_head(list_attributes); saved_attribute != NULL; saved_attribute = list_item_next(saved_attribute)) {
		if(saved_attribute == attribute) {
			_remove_attribute(attribute);
			return;
		}
	}

	_print_attributes();
}

void _clean_attributes_of_unknown_nodes() {
	neighbor_attribute_t *attribute;

	restart: for(attribute = list_head(list_attributes); attribute != NULL; attribute = list_item_next(attribute)) {
		int found = networkaddr_equal(attribute->node, networkaddr_node_addr());
		neighbor_t *neighbor;
		for(neighbor = list_head(components_neighbordiscovery_neighbors()); neighbor != NULL; neighbor = list_item_next(neighbor))
			found |= (networkaddr_equal(attribute->node, neighbor->node1) || networkaddr_equal(attribute->node, neighbor->node2));

		if(!found) {
			_remove_attribute(attribute);
			goto restart;
		}
	}
}

void _print_attributes() {
#if DEBUG
	_clean_attributes_of_unknown_nodes();
#endif
	PRINTF("DEBUG: [neighbordiscovery-common] neighbordiscovery attributes:\n");
	neighbor_attribute_t *attribute;
	for(attribute = list_head(list_attributes); attribute != NULL; attribute = list_item_next(attribute)) {
		PRINTF("DEBUG: [neighbordiscovery-common] * node=%s, type=%d, length=%d, data='%s'\n", networkaddr2string_buffered(attribute->node), attribute->type, attribute->length, attribute->data);
	}
}

void _remove_attribute(neighbor_attribute_t *attribute) {
	networkaddr_reference_free(attribute->node);
	list_remove(list_attributes, attribute);
	memb_free(&memb_attributes, attribute);
}
