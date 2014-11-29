/*
 * smts_nvmp_protocol.c
 *
 *  Created on: 下午1:04:49
 *      Author: ss
 */

#include <stdlib.h>
#include <stdio.h>
#include "css_logger.h"
#include "uv/uv.h"
#include "nvmp_protocol.h"
#include "nvmp_protocol_def.h"
#include "encode_decode_util.h"
#include "smts_session.h"


/**
 * for mem watch.
 */
#include "mem_guard.h"
/**
 * init packet
 */
#define Int16_init(name)	t->name=0;
#define Int32_init(name)	t->name=0;
#define Int64_init(name)	t->name=0;
#define FixedString_init(name,length) t->name[0]=0;
#define BinaryBuf_init(name,_len)	t->name.base=NULL;t->name.len=0;
#define BinaryBufRef_init(name,_len)

#define GEN_STRUCT_ARGS_INIT(type,name)			type##_init(name)
#define GEN_STRUCT_3ARGS_INIT(type,name,length)	type##_init(name,length)
#define GEN_STRUCT_INIT(cmd_name,_packet,pcmd,fileds,_fun)	\
int cmd_name##_t_init(cmd_name##_t *t){				\
	t->name = #cmd_name;							\
	t->cmd = pcmd;									\
	fileds											\
	/** private*/									\
	t->original_buf = NULL;							\
	t->original_buf_len = 0;						\
	t->ref = 0;										\
	t->original_buf_ref_bit_map = 0;				\
	t->data=NULL;									\
	return 0;										\
}

PROTOCOL_MAP(GEN_STRUCT_INIT, GEN_STRUCT_ARGS_INIT, GEN_STRUCT_3ARGS_INIT);

/**
 * calculate packet length
 */
#define Int16_len(name)	len += 2;
#define Int32_len(name)	len += 4;
#define Int64_len(name)	len += 8;
#define FixedString_len(name,length) len+=length;
#define BinaryBuf_len(name,_len)	if(t->name.base != NULL){len+=t->name.len;}
#define BinaryBufRef_len(name,_len) if(t->name.base != NULL){len+=t->name.len;}

#define GEN_STRUCT_ARGS_LEN(type,name)			type##_len(name)
#define GEN_STRUCT_3ARGS_LEN(type,name,length)	type##_len(name,length)
#define GEN_STRUCT_LEN(cmd_name,_packet,cmd,fileds,_fun)		\
int cmd_name##_t_len(cmd_name##_t *t){						\
	int len = 0;											\
	fileds													\
	return len;												\
}
PROTOCOL_MAP(GEN_STRUCT_LEN, GEN_STRUCT_ARGS_LEN, GEN_STRUCT_3ARGS_LEN);

/**
 * decode packet.
 * packet will ref to original_buf.
 */
#define Int16_decode(name)	t->name = decode_int16(buf); buf+=2;
#define Int32_decode(name)	t->name = decode_int32(buf); buf+=4;
#define Int64_decode(name)	t->name = decode_int64(buf); buf+=8;
#define FixedString_decode(name,len) decode_fixed_string(buf,(len),t->name);buf+=len;
#define BinaryBuf_decode(name,length) decode_binary_buf(buf,(length),&t->name);t->name.len=(length);buf+=(length);
#define BinaryBufRef_decode(name,length) t->name.base=buf;t->name.len=(length);buf+=(length);

#define GEN_STRUCT_ARGS_DECODE(type,name)		type##_decode(name)
#define GEN_STRUCT_3ARGS_DECODE(type,name,arg)	type##_decode(name,arg)
#define GEN_STRUCT_DECODE(cmd_name,packet,cmd,fileds,_fun)		\
int cmd_name##_t_decode(uv_buf_t *packet_buf,cmd_name##_t *t){	\
	char *buf = packet_buf->base;								\
	fileds														\
	/*private,ref packet buf which recv from socket to original_buf;*/\
	t->original_buf = (uv_buf_t*)malloc(sizeof(uv_buf_t));		\
	t->original_buf->base = packet_buf->base;					\
	t->original_buf->len = packet_buf->len;						\
	t->original_buf_len = 1;									\
	t->packet_len += packet;									\
	return 0;													\
}

PROTOCOL_MAP(GEN_STRUCT_DECODE, GEN_STRUCT_ARGS_DECODE, GEN_STRUCT_3ARGS_DECODE);

/**
 * aclloc buf before encode packet
 */
#define Int16_inner_alloc(name)	if(is_new){is_new=0;}buf_size+=2;
#define Int32_inner_alloc(name)	if(is_new){is_new=0;}buf_size+=4;
#define Int64_inner_alloc(name)	if(is_new){is_new=0;}buf_size+=8;
#define FixedString_inner_alloc(name,len)	if(is_new){is_new=0;}buf_size+=len;
#define BinaryBuf_inner_alloc(name,length)	if(is_new){if(t->name.base!=NULL){buf_size+=t->name.len}is_new=0;}
#define BinaryBufRef_inner_alloc(name,length) 							\
	if(t->name.base!=NULL){												\
		buf->base=(char*)malloc(buf_size);buf->len=buf_size;					\
		buf_size=0;is_new=1;buf++;index++;								\
		buf->base=t->name.base;buf->len=t->name.len;					\
		t->original_buf_ref_bit_map|=1<<index;buf++;				\
	}

#define GEN_STRUCT_ARGS_INNER_ALLOC(type,name)		type##_inner_alloc(name)
#define GEN_STRUCT_3ARGS_INNER_ALLOC(type,name,arg)	type##_inner_alloc(name,arg)
#define GEN_STRUCT_BUFS_INNER_ALLOC(cmd_name,_packet,pcmd,fileds,_fun)				\
static int cmd_name##_t_bufs_inner_alloc(cmd_name##_t *t){							\
	int is_first=1,is_new=1,buf_size = 0,index=0;									\
	t->original_buf_ref_bit_map =0;													\
	uv_buf_t *buf = t->original_buf;												\
	fileds																			\
	if(!is_new){																	\
		buf->base=(char*)malloc(buf_size);											\
		buf->len=buf_size;is_first=0;												\
	}																				\
	return 0;																		\
}

PROTOCOL_MAP(GEN_STRUCT_BUFS_INNER_ALLOC, GEN_STRUCT_ARGS_INNER_ALLOC, GEN_STRUCT_3ARGS_INNER_ALLOC);

#define Int16_alloc(name)	if(is_new){buf_len++;is_new=0;}
#define Int32_alloc(name)	if(is_new){buf_len++;is_new=0;}
#define Int64_alloc(name)	if(is_new){buf_len++;is_new=0;}
#define FixedString_alloc(name,len)	if(is_new){buf_len++;is_new=0;}
#define BinaryBuf_alloc(name,length)	if(is_new){if(t->name.base!=NULL){buf_len++;is_new=0;}}
#define BinaryBufRef_alloc(name,length) if(t->name.base!=NULL){buf_len++;is_new=1;}

#define GEN_STRUCT_ARGS_ALLOC(type,name)		type##_alloc(name)
#define GEN_STRUCT_3ARGS_ALLOC(type,name,arg)	type##_alloc(name,arg)
#define GEN_STRUCT_BUFS_ALLOC(cmd_name,_packet,pcmd,fileds,_fun)		\
int cmd_name##_t_bufs_alloc(cmd_name##_t *t){							\
	int buf_len = 0,is_new = 1,buf_size = 0,i=0;						\
	fileds																\
	t->original_buf = (uv_buf_t*)malloc(buf_len*sizeof(uv_buf_t));		\
	t->original_buf_len = buf_len;										\
	cmd_name##_t_bufs_inner_alloc(t);									\
	return 0;															\
}

PROTOCOL_MAP(GEN_STRUCT_BUFS_ALLOC, GEN_STRUCT_ARGS_ALLOC, GEN_STRUCT_3ARGS_ALLOC);
/**
 * encode packet
 */
#define Int16_encode(name)	if(is_new){buf=packet->base;is_new=0;}encode_int16(buf,offset,t->name); buf+=2;
#define Int32_encode(name)	if(is_new){buf=packet->base;is_new=0;}encode_int32(buf,offset,t->name); buf+=4;
#define Int64_encode(name)	if(is_new){buf=packet->base;is_new=0;}encode_int64(buf,offset,t->name); buf+=8;
#define FixedString_encode(name,len) if(is_new){buf=packet->base;is_new=0;}encode_fixed_string(buf,len,t->name);buf+=len;
#define BinaryBuf_encode(name,_len)	if(is_new)				\
		{if(t->name.base!=NULL){							\
			buf=packet->base;is_new=0;						\
			encode_binary_buf(buf,t->name.len,&t->name);	\
			buf+=t->name.len;}								\
		}

#define BinaryBufRef_encode(name,length) 			\
		if(t->name.base!=NULL){						\
			packet++;packet->base=t->name.base;		\
			packet->len=t->name.len;				\
			packet++;is_new=1;						\
		}

#define GEN_STRUCT_ARGS_ENCODE(type,name)		type##_encode(name)
#define GEN_STRUCT_3ARGS_ENCODE(type,name,arg)	type##_encode(name,arg)
#define GEN_STRUCT_ENCODE(cmd_name,packet_opt,pcmd,fileds,_fun)		\
int cmd_name##_t_encode(cmd_name##_t *t){							\
	int offset = 0,is_new=1;										\
	uv_buf_t *packet = t->original_buf;								\
	char *buf;														\
	t->packet_len = cmd_name##_t_len(t) - packet_opt;				\
	t->cmd = pcmd;													\
	fileds															\
	t->packet_len += packet_opt;									\
	t->ref = 0;														\
	return 0;														\
}

PROTOCOL_MAP(GEN_STRUCT_ENCODE, GEN_STRUCT_ARGS_ENCODE, GEN_STRUCT_3ARGS_ENCODE);

/**
 * destroy packet.
 */
#define Int16_destroy(name)
#define Int32_destroy(name)
#define Int64_destroy(name)
#define FixedString_destroy(name,length)
#define BinaryBuf_destroy(name,_len)	FREE(t->name.base);
#define BinaryBufRef_destroy(name,_len)

#define GEN_STRUCT_ARGS_DESTROY(type,name)			type##_destroy(name)
#define GEN_STRUCT_3ARGS_DESTROY(type,name,length)	type##_destroy(name,length)
#define GEN_STRUCT_DESTROY(cmd_name,_packet,cmd,fileds,_fun)					\
int cmd_name##_t_destroy(cmd_name##_t *t){										\
	if(t->ref > 0) {t->ref --;return 0;};										\
	fileds																		\
	/* private*/																\
	if(t->original_buf != NULL){												\
		int i=0;																\
		for(;i < t->original_buf_len;i++){										\
			if(t->original_buf[i].base != NULL){								\
				if((t->original_buf_ref_bit_map & (1<<i)) == 0){				\
					FREE(t->original_buf[i].base);  							\
				}else{															\
					t->original_buf[i].base = NULL;								\
				}																\
			}																	\
		}																		\
		FREE(t->original_buf);													\
	}																			\
	FREE(t);																	\
	return 0;																	\
}

PROTOCOL_MAP(GEN_STRUCT_DESTROY, GEN_STRUCT_ARGS_DESTROY, GEN_STRUCT_3ARGS_DESTROY);



/**
 * abstract methods.
 */
//
//#define GEN_PROTO_CMD_CASE_ARG(_type,_name)
//#define GEN_PROTO_CMD_CASE_ARG3(_type,_name,_)
//#define GEN_PROTO_CMD_INIT(cmd_name,_packet,cmd,fileds,_fun)					\
//case cmd:																		\
//	cmd_name##_t_init((cmd_name##_t*)cmd_t);										\
//	break;																		\
//
//int proto_cmd_init(abstract_cmd_t *cmd_t){
//	int r = 0;
//	switch(cmd_t->cmd){
//			PROTOCOL_MAP(GEN_PROTO_CMD_INIT, GEN_PROTO_CMD_CASE_ARG, GEN_PROTO_CMD_CASE_ARG3);
//		default:
//			CL_ERROR("no init fun to call of cmd:%d\n",cmd_t->cmd);
//			r = -1;
//	}
//	return r;
//}
//
//#define GEN_PROTO_CMD_LEN(cmd_name,_packet,cmd,fileds,_fun)						\
//case cmd:																		\
//	cmd_name##_t_len((cmd_name##_t*)cmd_t);										\
//	break;																		\
//
//int proto_cmd_len(abstract_cmd_t *cmd_t){
//	int r = 0;
//	switch((cmd_t)->cmd){
//			PROTOCOL_MAP(GEN_PROTO_CMD_LEN, GEN_PROTO_CMD_CASE_ARG, GEN_PROTO_CMD_CASE_ARG3);
//		default:
//			CL_ERROR("no len fun to call of cmd:%d\n",(cmd_t)->cmd);
//			r = -1;
//	}
//	return r;
//}
//
//#define GEN_PROTO_CMD_BUFS_ALLOC(cmd_name,_packet,cmd,fileds,_fun)						\
//case cmd:																				\
//	cmd_name##_t_bufs_alloc((cmd_name##_t*)cmd_t);										\
//	break;
//
//int proto_cmd_bufs_alloc(abstract_cmd_t *cmd_t)
//{
//	int r = 0;
//	switch((cmd_t)->cmd){
//			PROTOCOL_MAP(GEN_PROTO_CMD_BUFS_ALLOC, GEN_PROTO_CMD_CASE_ARG, GEN_PROTO_CMD_CASE_ARG3);
//		default:
//			CL_ERROR("no alloc fun to call of cmd:%d\n",(cmd_t)->cmd);
//			r = -1;
//	}
//	return r;
//
//}
//
//#define GEN_PROTO_CMD_DECODE(cmd_name,_packet,cmd,fileds,_fun)						\
//case cmd:																			\
//	cmd_name##_t_decode(packet,(cmd_name##_t*)cmd_t);										\
//	break;
//
//int proto_cmd_decode(uv_buf_t *packet,abstract_cmd_t *cmd_t)
//{
//	int r = 0;
//	switch((cmd_t)->cmd){
//			PROTOCOL_MAP(GEN_PROTO_CMD_DECODE, GEN_PROTO_CMD_CASE_ARG, GEN_PROTO_CMD_CASE_ARG3);
//		default:
//			CL_ERROR("no decode fun to call of cmd:%d\n",(cmd_t)->cmd);
//			r = -1;
//	}
//	return r;
//}
//
//#define GEN_PROTO_CMD_ENCODE(cmd_name,_packet,cmd,fileds,_fun)						\
//case cmd:																			\
//	cmd_name##_t_encode((cmd_name##_t*)cmd_t);										\
//	break;
//
//int proto_cmd_encode(abstract_cmd_t *cmd_t)
//{
//	int r = 0;
//	switch((cmd_t)->cmd){
//			PROTOCOL_MAP(GEN_PROTO_CMD_ENCODE, GEN_PROTO_CMD_CASE_ARG, GEN_PROTO_CMD_CASE_ARG3);
//		default:
//			CL_ERROR("no encode fun to call of cmd:%d\n",(cmd)->cmd);
//			r = -1;
//	}
//	return r;
//}
//
//#define GEN_PROTO_CMD_DESTROY(cmd_name,_packet,cmd,fileds,_fun)						\
//case cmd:																			\
//	cmd_name##_t_destroy((cmd_name##_t*)cmd);										\
//	break;
//
//int proto_cmd_destroy(abstract_cmd_t *cmd)
//{
//	int r = 0;
//	switch((cmd)->cmd){
//			PROTOCOL_MAP(GEN_PROTO_CMD_DESTROY, GEN_PROTO_CMD_CASE_ARG, GEN_PROTO_CMD_CASE_ARG3);
//		default:
//			CL_ERROR("no destroy fun to call of cmd:%d\n",(cmd)->cmd);
//			r = -1;
//	}
//	return r;
//}









