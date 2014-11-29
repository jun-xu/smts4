/*
 * css_errorno.h
 *
 *  Created on: 2014-6-6
 *      Author: sunshine
 */

#ifndef SMTS_ERRORNO_H_
#define SMTS_ERRORNO_H_

#include "uv/uv.h"
#define SMTS_OK 0
#define SMTS_BAD_REQUEST 400

/**
 * smts client errorcode. 7xxx
 */
#define SMTS_CLIENT_READ_ERROR  7001

/**
 * dvr client errorcode 8xxx
 */
#define DVR_CLIENT_INIT_ADDR_ERROR 			8001
#define DVR_CONNECT_REFUSED_ERROR			8002		// uv error: -61
#define DVR_SEND_PRIVIEW_CMD_ERROR 			8003
#define DVR_RECV_INVALID_PACKET_ERROR		8004
#define DVR_RECV_FRAME_ERROR				8005

const char* smts_strerror(int err);

#define SMTS_ERR_NAME_GEN(name, _) case name: return #name;
#define SMTS_STRERROR_GEN(name, msg) case name: return msg;
#define SMTS_ERRNO_MAP(XX)                                                      \
  XX(SMTS_OK,"ok")																\
  XX(SMTS_BAD_REQUEST, "bad request")                                         	\
  /* smts client error*/													 	\
  XX(SMTS_CLIENT_READ_ERROR, "smts client socket close by remote.")				\
  XX(DVR_CLIENT_INIT_ADDR_ERROR,"init dvr client addr error.")					\
  XX(DVR_CONNECT_REFUSED_ERROR,"dvr connection refused.")						\
  XX(DVR_SEND_PRIVIEW_CMD_ERROR,"send preview cmd to dvr error.")				\
  XX(DVR_RECV_INVALID_PACKET_ERROR,"recv invalid packet error.")				\
  XX(DVR_RECV_FRAME_ERROR,"recv frame packet error.")							\

#endif /* SMTS_ERRORNO_H_ */
