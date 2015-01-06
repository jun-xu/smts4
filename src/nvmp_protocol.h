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
struct abstract_cmd_s;

typedef int (*cmd_destroy_fun)(struct abstract_cmd_s *cmd);
typedef int (*cmd_decode_fun)(uv_buf_t *packet,struct abstract_cmd_s *cmd);
typedef int (*cmd_encode_fun)(struct abstract_cmd_s *cmd);
typedef int (*cmd_len_fun)(struct abstract_cmd_s *cmd);


#define Int16_filed(name)	int16_t name;
#define Int32_filed(name)	int32_t name;
#define Int64_filed(name)	int64_t name;
#define FixedString_filed(name,len)	char name[len];
#define BinaryBuf_filed(name,_len)	uv_buf_t name;
#define BinaryBufRef_filed(name,_len) uv_buf_t name;

#define GEN_STRUCT_FILED(type,name)			type##_filed(name)
#define GEN_STRUCT_3FILED(type,name,arg) 	type##_filed(name,arg)
#define GEN_STRUCT(cmd_name,_packet,_cmd,fileds,_fun)			\
typedef struct cmd_name##_s{									\
	char 			*name;	/*private*/							\
	/** private
	 *	total buf recv by socket or encode bufs.
	 */															\
	cmd_destroy_fun destroy_fun;								\
	cmd_decode_fun 	decode_fun;									\
	cmd_encode_fun 	encode_fun;									\
	cmd_len_fun 	len_fun;									\
	uv_buf_t 		*original_buf;								\
	int32_t 		original_buf_len;							\
	int32_t 		original_buf_ref_bit_map;					\
	/* public */												\
	void 			*data;										\
	volatile int 	ref;/*ref count. destory it when ref equal zero.*/\
	fileds														\
}cmd_name##_t;
PROTOCOL_MAP(GEN_STRUCT, GEN_STRUCT_FILED, GEN_STRUCT_3FILED);

/**
 * declare public API methods
 */
#define GEN_STRUCT_FILED_METHOD(type,name)
#define GEN_STRUCT_3FILED_METHOD(type,name,len)
#define GEN_STRUCT_METHOD(cmd_name,_packet,cmd,fileds,_fun)			\
int cmd_name##_t_init(cmd_name##_t *t);								\

PROTOCOL_MAP(GEN_STRUCT_METHOD, GEN_STRUCT_FILED_METHOD, GEN_STRUCT_3FILED_METHOD)
int nvmp_cmd_t_increase_ref(abstract_cmd_t *t);
int nvmp_cmd_t_len(abstract_cmd_t *t);
int nvmp_cmd_t_decode(uv_buf_t *packet,abstract_cmd_t *t);
int nvmp_cmd_t_encode(abstract_cmd_t *t);
int nvmp_cmd_t_destroy(abstract_cmd_t *t);

#define PACKET_INVALID -1
#define PACKET_LEN_FIELD_SIZE 4
#define MAX_PACKET_LEN 64*1024*1024

#endif /* NVMP_PROTOCOL_H_ */
