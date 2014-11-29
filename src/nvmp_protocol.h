/*
 * smts_nvmp_protocol.h
 *
 *  Created on: 下午1:03:02
 *      Author: ss
 */

#ifndef NVMP_PROTOCOL_H_
#define NVMP_PROTOCOL_H_
#include "uv/uv.h"
#include <stdint.h>
#include "nvmp_protocol_def.h"

/**
 * declare struct
 */
#define Int16_filed(name)	int16_t name;
#define Int32_filed(name)	int32_t name;
#define Int64_filed(name)	int64_t name;
#define FixedString_filed(name,len)	char name[len];
#define BinaryBuf_filed(name,_len)	uv_buf_t name;
#define BinaryBufRef_filed(name,_len) uv_buf_t name;

#define GEN_STRUCT_FILED(type,name)			type##_filed(name)
#define GEN_STRUCT_3FILED(type,name,arg) 	type##_filed(name,arg)
#define GEN_STRUCT(cmd_name,_packet,_cmd,fileds,_fun)	\
typedef struct cmd_name##_s{					\
	char *name;	/*private*/						\
	/** private
	 *	total buf recv by socket or encode bufs.
	 */											\
	uv_buf_t *original_buf;						\
	int32_t original_buf_len;					\
	int32_t original_buf_ref_bit_map;			\
	/* public */								\
	void *data;									\
	volatile int ref;/*ref count. destory it when ref equal zero.*/\
	fileds										\
}cmd_name##_t;
PROTOCOL_MAP(GEN_STRUCT, GEN_STRUCT_FILED, GEN_STRUCT_3FILED);

/**
 * declare methods
 */
#define GEN_STRUCT_FILED_METHOD(type,name)
#define GEN_STRUCT_3FILED_METHOD(type,name,len)
#define GEN_STRUCT_METHOD(cmd_name,_packet,cmd,fileds,_fun)		\
int cmd_name##_t_init(cmd_name##_t *t);							\
int cmd_name##_t_len(cmd_name##_t *t);							\
int cmd_name##_t_decode(uv_buf_t *packet,cmd_name##_t *t);		\
int cmd_name##_t_encode(cmd_name##_t *t);						\
int cmd_name##_t_destroy(cmd_name##_t *t);						\

PROTOCOL_MAP(GEN_STRUCT_METHOD, GEN_STRUCT_FILED_METHOD, GEN_STRUCT_3FILED_METHOD)


#define PACKET_INVALID -1
#define PACKET_LEN_FIELD_SIZE 4
#define MAX_PACKET_LEN 64*1024*1024

#endif /* NVMP_PROTOCOL_H_ */
