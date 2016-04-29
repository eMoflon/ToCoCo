#ifndef __BUFFER_H_
#define __BUFFER_H_

#include <stdint.h>
#include "contiki-net.h"

#include "networkaddr.h"

#define BUFFER_NETWORKADDR_SIZE NETWORKADDR_SIZE_COMPRESSED

typedef struct buffer {
	void *bufferptr;
	size_t length;
	size_t offset;
} buffer_t;

int buffer_remaining(buffer_t *buffer);

int buffer_append_rawbytes(buffer_t *buffer, const void *data, size_t length);

int buffer_append_networkaddr(buffer_t *buffer, const networkaddr_t *address);

int buffer_append_uint8t(buffer_t *buffer, uint8_t num);

int buffer_append_int8t(buffer_t *buffer, int8_t num);

int buffer_append_uint16t(buffer_t *buffer, uint16_t num);

int buffer_append_int16t(buffer_t *buffer, int16_t num);

/**
 * \note only a pointer to the start address for the data in the buffer is returned
 */
void *buffer_read_rawbytes(buffer_t *buffer, size_t length);

networkaddr_t buffer_read_networkaddr(buffer_t *buffer);

uint8_t buffer_read_uint8t(buffer_t *buffer);

int8_t buffer_read_int8t(buffer_t *buffer);

uint16_t buffer_read_uint16t(buffer_t *buffer);

int16_t buffer_read_int16t(buffer_t *buffer);

#endif /* __BUFFER_H_ */
