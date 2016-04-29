#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "buffer.h"
#include "networkaddr.h"

int buffer_remaining(buffer_t *buffer) {
	return buffer->length - buffer->offset;
}

int buffer_append_rawbytes(buffer_t *buffer, const void *data, size_t length) {
	if(buffer->offset + length >= buffer->length) {
		printf("ERROR[buffer]: buffer is not large enough to save data\n");
		return -1;
	}

	memcpy(((uint8_t *) buffer->bufferptr) + buffer->offset, data, length);
	buffer->offset += length;
	return buffer->offset - length;
}

int buffer_append_networkaddr(buffer_t *buffer, const networkaddr_t *address) {
	int offset = buffer_append_rawbytes(buffer, address, BUFFER_NETWORKADDR_SIZE);
	if(offset != -1)
		networkaddr_compression_compress(address, ((uint8_t *) buffer->bufferptr) + offset);

	return offset;
}

int buffer_append_uint8t(buffer_t *buffer, uint8_t num) {
	return buffer_append_rawbytes(buffer, &num, sizeof(uint8_t));
}

int buffer_append_int8t(buffer_t *buffer, int8_t num) {
	return buffer_append_rawbytes(buffer, &num, sizeof(int8_t));
}


int buffer_append_uint16t(buffer_t *buffer, uint16_t num) {
	return buffer_append_rawbytes(buffer, &num, sizeof(uint16_t));
}

int buffer_append_int16t(buffer_t *buffer, int16_t num) {
	return buffer_append_rawbytes(buffer, &num, sizeof(int16_t));
}

void *buffer_read_rawbytes(buffer_t *buffer, size_t length) {
	if(buffer->offset + length > buffer->length) {
		printf("ERROR[buffer]: buffer is not large enough to read data\n");
		return NULL;
	}

	size_t offset = buffer->offset;
	buffer->offset += length;

	return ((uint8_t *) buffer->bufferptr) + offset;
}

networkaddr_t buffer_read_networkaddr(buffer_t *buffer) {
	networkaddr_t address;
	networkaddr_compression_decompress(&address, buffer_read_rawbytes(buffer, BUFFER_NETWORKADDR_SIZE));
	return address;
}

uint8_t buffer_read_uint8t(buffer_t *buffer) {
	uint8_t num;
	memcpy(&num, buffer_read_rawbytes(buffer, sizeof(uint8_t)), sizeof(uint8_t));
	return num;
}

int8_t buffer_read_int8t(buffer_t *buffer) {
	int8_t num;
	memcpy(&num, buffer_read_rawbytes(buffer, sizeof(int8_t)), sizeof(int8_t));
	return num;
}

uint16_t buffer_read_uint16t(buffer_t *buffer) {
	uint16_t num;
	memcpy(&num, buffer_read_rawbytes(buffer, sizeof(uint16_t)), sizeof(uint16_t));
	return num;
}

int16_t buffer_read_int16t(buffer_t *buffer) {
	int16_t num;
	memcpy(&num, buffer_read_rawbytes(buffer, sizeof(int16_t)), sizeof(int16_t));
	return num;
}
