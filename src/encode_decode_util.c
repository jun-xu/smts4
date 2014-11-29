/*
 * encode_decode_util.c
 *
 *  Created on: 下午2:15:18
 *      Author: ss
 */

#include "encode_decode_util.h"
#include "smts_util.h"
#include <stdlib.h>
#include <string.h>

/**
 * for mem watch.
 */
#include "mem_guard.h"

/**
 * encode integer.
 */
#define ENCODE_INT(buffer,offset,val,size)				\
	do{													\
		int len = size;									\
		for (; len > 0; len--, offset++) {				\
			buffer[offset] = (val >> ((len - 1) * 8));	\
		}												\
	}while(0)											\

void encode_int16(char *buffer, int32_t offset, int16_t val)
{
	ENCODE_INT(buffer, offset, val, 2);
}

void encode_int32(char *buffer, int32_t offset, int32_t val)
{
	ENCODE_INT(buffer, offset, val, 4);
}
void encode_int64(char *buffer, int32_t offset, int64_t val)
{
	ENCODE_INT(buffer, offset, val, 8);
}

/**
 * decode integer
 */
#define DECODE_INT(val,buf,size) 						\
	do{													\
		int len = 0;									\
		for (; len < size; len++) {						\
			val = ((val << 8) | (0xff & buf[len]));		\
		}												\
	}while(0)											\


int16_t decode_int16(char* buf)
{
	int16_t val = 0;
	DECODE_INT(val, buf, 2);
	return val;

}

int32_t decode_int32(char* buf)
{
	int32_t val = 0;
	DECODE_INT(val, buf, 4);
	return val;
}

int64_t decode_int64(char* buf)
{
	int64_t val = 0;
	DECODE_INT(val, buf, 8);
	return val;
}

void encode_fixed_string(char *buf, size_t len, char *str)
{
	memcpy(buf, str, len);
}

void decode_fixed_string(char *buf, size_t len, char *str)
{
	memcpy(str, buf, len);
}

void encode_binary_buf(char *buf, size_t len, uv_buf_t *buffer)
{
	memcpy(buf, buffer->base, len);
}

void decode_binary_buf(char *buf, size_t len, uv_buf_t *buffer)
{
	memcpy(buffer->base, buf, len);
}

#ifdef SMTS_TEST
#include <assert.h>

#endif
