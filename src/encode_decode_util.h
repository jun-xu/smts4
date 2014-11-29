/*
 * encode_decode_util.h
 *
 *  Created on: 下午1:55:41
 *      Author: ss
 */

#ifndef ENCODE_DECODE_UTIL_H_
#define ENCODE_DECODE_UTIL_H_
#include <stdint.h>
#include "uv/uv.h"

void encode_int16(char *buffer, int32_t offset, int16_t i);
void encode_int32(char *buffer, int32_t offset, int32_t i);
void encode_int64(char *buffer, int32_t offset, int64_t i);
/**
 * copy binary from str to buf.
 */
void encode_fixed_string(char *buf, size_t len, char *str);
void encode_binary_buf(char *buf, size_t len, uv_buf_t *buffer);
int16_t decode_int16(char* data);
int32_t decode_int32(char* data);
int64_t decode_int64(char* data);
/**
 * copy binary from buf to str.
 */
void decode_fixed_string(char *buf, size_t len, char *str);
void decode_binary_buf(char *buf, size_t len, uv_buf_t *buffer);

#endif /* ENCODE_DECODE_UTIL_H_ */
