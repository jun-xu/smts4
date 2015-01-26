/*
 * @file nvmp_protocol_def.h
 * @author: ss
 */


#ifndef NVMP_PROTOCOL_DEF_H_
#define NVMP_PROTOCOL_DEF_H_

#include "uv/uv.h"

#define SMTS_PROTO_DEFINE
//
//#define PACK0	0
//#define PACK4	4


#define PREVIEW_CMD 	0x00050104
#define PREVIEW_RES_CMD 0x80050101
#define SEND_FRAME_CMD 	0x80050102
#define COMMON_RES_CMD 	0x80000001

#define PROTOCOL_TOP_FILED(ARG2)					\
	/* total packet length */						\
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
T(abstract_cmd,0x0,									\
  PROTOCOL_TOP_FILED(ARG2),							\
  NULL												\
)													\
/**
 * start preview request.
 * client --> smts server.
 */													\
T(preview_cmd,PREVIEW_CMD,							\
  PROTOCOL_HEAD_FILED(ARG2)							\
  ARG3(FixedString,token,128)						\
  ARG2(Int64,dvr_id)								\
  ARG2(Int16,channel_no)							\
  /* frame_mode: 0:primary  1:sub*/					\
  ARG2(Int32,frame_mode),							\
  smts_start_preview								\
)													\
/**
 * preview response to client.
 * smts server --> client.
 * status: error code.  0: success.
 */													\
T(preview_cmd_res,PREVIEW_RES_CMD,					\
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
T(smts_frame_res,SEND_FRAME_CMD,					\
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
T(mock_dvr_preview,0x00018001,						\
  PROTOCOL_TOP_FILED(ARG2)							\
  ARG2(Int32,bitrate)								\
  ARG2(Int32,frame_rate),							\
  mock_dvr_preview_impl								\
)													\
T(mock_dvr_frame,0x00018002,						\
  PROTOCOL_TOP_FILED(ARG2)							\
  ARG2(Int32,seqno)									\
  ARG2(Int32,frame_type)							\
  ARG2(Int64,st)									\
  ARG3(BinaryBufRef,frame,t->packet_len-24),		\
  NULL												\
)													\
/**
 * test send PTZ cmd
 */													\
T(test_PTZ_cmd,0x00018010,							\
  PROTOCOL_HEAD_FILED(ARG2)							\
  ARG2(Int32,ptz),									\
  msg_channel										\
)													\
T(common_res,COMMON_RES_CMD,						\
  PROTOCOL_HEAD_FILED(ARG2)							\
  ARG2(Int32,status),								\
  NULL												\
)													\

#endif /* NVMP_PROTOCOL_DEF_H_ */
