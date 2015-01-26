/*
 * smts_proto.h
 *
 *  Created on: 下午3:08:24
 *      Author: ss
 */

#ifndef SMTS_PROTO_H_
#define SMTS_PROTO_H_


#include "uv/uv.h"
#include <stdint.h>

/** protocol define.
 *
 * PROTOCOL_MAP format:
 * @code
 * 	T( packet_name, packet, cmd,  fileds, fun)	=>
 * 			struct packet_name##_s{char *name,				//private
 * 								  ...,						// defined by user
 * 								  uv_buf_t original_buf;	//private
 * 								  }packet_name##_t;
 * 			int cmd_name##_t_init();
 * 			int cmd_name##_t_len();
 *			int cmd_name##_decode(uv_buf_t *packet,cmd_name##_t *t);
 *			int cmd_name##_encode(cmd_name##_t *t,uv_buf_t *packet);
 *			int cmd_name##_t_destroy();
 *
 *	packet_name : 	the name of packet.
 *	packet :  		PACK0 or PACK4.  	PACK0: packet_len contain self 4 byte.
 *										PACK4: packet_len NOT contain self 4 byte.
 *	cmd: 			cmd. int32_t
 *	fileds:			use defined fileds.
 *	fun: 			callback function when recv cmd.
 * @endcode
 *  all fun use by {@link smts_dispatch::dispatch_packet}
 *
 *
 * @code
 *		macro			|			mean
 * 	----------------------------------------------------------------
 * 	PROTOCOL_HEAD_FILED		packet header  20byte of nvmp protocol.
 * 		T					struct define
 * 		ARG2				type-name format. 		like: ARG2(type,name)
 * 		ARG3				type-name-arg format. 	like: ARG3(type,name,arg)
 *
 * 	all of protocol struct&&method defined in {@link nvmp_protocol.h}.
 * @endcode
 *
 *
 *	fileds types and methods:
 *	@code
 * 	type_alise	|	type		|	encode_method	   |	decode_metod
 * 	-----------------------------------------------------------------
 * 	Int32			int32_t			encode_int32			decode_int32
 * 	Int16			int16_t			encode_int32			decode_int32
 * 	Int64			int64_t			encode_int32			decode_int32
 * 	FixedString		uv_buf_t		encode_fixed_string		decode_fixed_string
 * 	BinaryBuf		uv_buf_t		encode_binary_buf		decode_binary_buf		//deep copy data to binaryBuf.
 * 	BinaryBufRef	uv_buf_t		other					other			//ref data to buf.not free in destroy method.
 *
 * @endcode
 *	all of encode decode methods in {@link encode_decode_util.h}
 *
 * @code
 * 		method						|					mean
 * 	-------------------------------------------------------------------
 * 	int cmd_name##_t_init(cmd_name##_t *t)
 *	int proto_cmd_t_len(abstract_cmd_t *t)						calculate total length of struct cmd_name##_t;
 *	int proto_cmd_t_decode(uv_buf_t *packet,abstract_cmd_t *t)	decode cmd_name##_t from uv_buf_t.
 *	int proto_cmd_t_encode(abstract_cmd_t *t,uv_buf_t *packet)	encode cmd_name##_t to uv_buf_t.
 *	int proto_cmd_t_destroy(abstract_cmd_t *t)					destory cmd_name##_t.
 * @endcode
 * 	all of protocol method implement in {@link smts_proto.h}.
 */


#ifndef protocol_name
/**
 * use nvmp packet format
 */
#include "nvmp_protocol_def.h"
#else

#endif

struct abstract_cmd_s;

typedef int (*cmd_destroy_fun)(struct abstract_cmd_s *cmd);
typedef int (*cmd_decode_fun)(uv_buf_t *packet,struct abstract_cmd_s *cmd);
typedef int (*cmd_encode_fun)(struct abstract_cmd_s *cmd);
typedef int (*cmd_len_fun)(struct abstract_cmd_s *cmd);

/**
 * cmd encode && decode funs.
 */
typedef struct cmd_actions_s{
	cmd_destroy_fun destroy_fun;
	cmd_decode_fun 	decode_fun;
	cmd_encode_fun 	encode_fun;
	cmd_len_fun 	len_fun;
}cmd_action_t;

typedef struct cmd_codec_buf_s{
	uv_buf_t 		*original_buf;			/*encode && decode buffer.*/
	uint32_t 		original_buf_len;		/*encode && decode buffer size*/
	uint32_t 		original_buf_ref_bit_map;/*private*/
}cmd_codec_buf_t;

/**
 * declare struct
 */
#define Int16_filed(name)				int16_t name;
#define Int32_filed(name)				int32_t name;
#define Int64_filed(name)				int64_t name;
#define FixedString_filed(name,len)		char name[len];
#define BinaryBuf_filed(name,_len)		uv_buf_t name;
#define BinaryBufRef_filed(name,_len) 	uv_buf_t name;

#define GEN_STRUCT_FILED(type,name)			type##_filed(name)
#define GEN_STRUCT_3FILED(type,name,arg) 	type##_filed(name,arg)
#define GEN_STRUCT(cmd_name,_packet,_cmd,fileds,_fun)					\
typedef struct cmd_name##_s{											\
	char 				*name;	/*private*/								\
	/** private*/														\
	cmd_action_t		actions;										\
	cmd_codec_buf_t		codec_buf;										\
	/* public */														\
	void 				*data;											\
	volatile uint32_t 	ref;/*ref count. destory it when ref equal zero.*/\
	fileds																\
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

#define PACKET_INVALID -1
#define PACKET_LEN_FIELD_SIZE 4
#define MAX_PACKET_LEN 64*1024*1024

int proto_cmd_t_increase_ref(abstract_cmd_t *t);
int proto_cmd_t_len(abstract_cmd_t *t);
int proto_cmd_t_decode(uv_buf_t *packet,abstract_cmd_t *t);
int proto_cmd_t_encode(abstract_cmd_t *t);
int proto_cmd_t_destroy(abstract_cmd_t *t);


#endif /* SMTS_PROTO_H_ */
