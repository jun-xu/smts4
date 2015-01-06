/*
 * @file nvmp_protocol_def.h
 * @author: ss
 */


#ifndef NVMP_PROTOCOL_DEF_H_
#define NVMP_PROTOCOL_DEF_H_

#include "uv/uv.h"

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
 *	int nvmp_cmd_t_len(abstract_cmd_t *t)						calculate total length of struct cmd_name##_t;
 *	int nvmp_cmd_t_decode(uv_buf_t *packet,abstract_cmd_t *t)	decode cmd_name##_t from uv_buf_t.
 *	int nvmp_cmd_t_encode(abstract_cmd_t *t,uv_buf_t *packet)	encode cmd_name##_t to uv_buf_t.
 *	int nvmp_cmd_t_destroy(abstract_cmd_t *t)					destory cmd_name##_t.
 * @endcode
 * 	all of protocol method implement in {@link nvmp_protocol.h}.
 */
#define SMTS_PROTO_DEFINE

#define PACK0	0
#define PACK4	4


#define PREVIEW_CMD 0x00050104
#define PREVIEW_RES_CMD 0x80050101
#define SEND_FRAME_CMD 0x80050102
#define COMMON_RES_CMD 0x80000001

#define PROTOCOL_TOP_FILED(ARG2)					\
	/* packet length */								\
	ARG2(Int32,packet_len)							\
	ARG2(Int32,cmd)									\

#define PROTOCOL_HEAD_FILED(ARG2)					\
	PROTOCOL_TOP_FILED(ARG2)						\
	ARG2(Int32,seqno)								\
	/* not use */									\
	ARG2(Int32,src_addr)							\
	/* not use */									\
	ARG2(Int32,dest_addr)

#define PROTOCOL_MAP(T,ARG2,ARG3)					\
T(abstract_cmd,PACK0,								\
  0x0,												\
  PROTOCOL_TOP_FILED(ARG2),							\
  NULL												\
)													\
/**
 * start preview request.
 * client --> smts server.
 */													\
T(preview_cmd,PACK0,								\
  PREVIEW_CMD,										\
  PROTOCOL_HEAD_FILED(ARG2)							\
  ARG3(FixedString,token,128)						\
  ARG2(Int64,dvr_id)								\
  ARG2(Int16,channel_no)							\
  /* frame_mode: 0:primary  1:sub*/					\
  ARG2(Int32,frame_mode),							\
  nvmp_smts_preview									\
)													\
/**
 * preview response to client.
 * smts server --> client.
 * status: error code.  0: success.
 */													\
T(preview_cmd_res,PACK0,							\
  PREVIEW_RES_CMD,									\
  PROTOCOL_HEAD_FILED(ARG2)							\
  /* error code. 0:no error.*/						\
  ARG2(Int32,status)								\
  ARG2(Int32,brightness)							\
  ARG2(Int32,contrast)								\
  ARG2(Int32,saturation)							\
  ARG2(Int32,Hue),									\
  NULL												\
)													\
/**
 * smts server send frame to clients.
 * smts server --> client.
 */													\
T(smts_frame_res,PACK0,								\
  SEND_FRAME_CMD,									\
  PROTOCOL_HEAD_FILED(ARG2)							\
  ARG2(Int32,frame_type)							\
  ARG3(BinaryBufRef,frame,t->packet_len-20),		\
  NULL												\
)													\
													\
													\
/**
 * --------------------------------------------
 * mock dvr protocol.
 * ---------------------------------------------
 * start preview from mock dvr.
 * smts server --> mock dvr (erlang).
 */													\
T(mock_dvr_preview,PACK4,							\
  0x00018001,										\
  PROTOCOL_TOP_FILED(ARG2)							\
  ARG2(Int32,bitrate)								\
  ARG2(Int32,frame_rate),							\
  mock_dvr_preview_impl								\
)													\
T(mock_dvr_frame,PACK4,								\
  0x00018002,										\
  PROTOCOL_TOP_FILED(ARG2)							\
  ARG2(Int32,seqno)									\
  ARG2(Int32,frame_type)							\
  ARG2(Int64,st)									\
  ARG3(BinaryBufRef,frame,t->packet_len-24+PACK4),	\
  NULL												\
)													\
/**
 * test send PTZ cmd
 */													\
T(test_PTZ_cmd,PACK0,								\
  0x00018010,										\
  PROTOCOL_HEAD_FILED(ARG2)							\
  ARG2(Int32,ptz),									\
  msg_channel										\
)													\
T(common_res,PACK0,									\
  COMMON_RES_CMD,									\
  PROTOCOL_HEAD_FILED(ARG2)							\
  ARG2(Int32,status),								\
  NULL												\
)													\

#endif /* NVMP_PROTOCOL_DEF_H_ */
