/*
 * smts_dvr_client.h
 *
 *  Created on: 上午10:54:26
 *      Author: ss
 */

#ifndef SMTS_MOCK_DVR_IMPL_H_
#define SMTS_MOCK_DVR_IMPL_H_
/**
 * start preview or replay from dvr.
 */
#include "smts_proto.h"
#include "smts_abstract_session.h"
#include "smts_abstract_tcp_client.h"
#include "smts_dvrs_config.h"
#include "uv/uv.h"
#include "smts_util.h"

typedef void (*dvr_client_close_cb)(struct smts_session_s *s);
/**
 * inherit from {@link abstract_tcp_client_s}
 * connect to dvr. send preview msg, read frames.
 */
typedef struct smts_dvr_client_s
{
	ABSTRACT_TCP_CLIENT_FILEDS

	/**
	 * public
	 */
	struct smts_session_s *session; /* smts session. */
	dvr_info_t dvr_info;				/// dvr info. e.g. ip. port ...

} smts_dvr_client_t;

/**
 * init frame recv from dvr.
 * @param frame  frame point should be NULL.
 * @param buf 	recv data from socket.
 * @return 0:ok, other:error_code.
 */
int init_smts_frame(struct smts_frame_s **frame, uv_buf_t *buf);

/**
 * free frame recv from dvr.
 * @param frame  recv from dvr.
 * @return 0:ok, other:error_code.
 */
int destroy_smts_frame(struct smts_frame_s *frame);

/**
 * init dvr client.
 * @return 0:ok. other:errorcode.
 */
int init_smts_dvr_client(smts_dvr_client_t *dvr_client, uv_loop_t *loop, int64_t dvr_id, int16_t channel_no,
		int32_t frame_mode);
/**
 * session call after init dvr client.
 * connect to dvr. and callback to session.
 * @return 0:ok. other:errorcode.
 */
int smts_dvr_client_connect(smts_dvr_client_t *dvr_client, int packet_opt);
/**
 * session call after connect to dvr successful.
 * send preview cmd to dvr, and callback to session.
 * @return 0:ok. other:errorcode.
 */
int smts_dvr_client_preview(smts_dvr_client_t *dvr_client);

/**
 *  start read frame from dvr
 * @return 0:ok. other:errorcode.
 */
int smts_dvr_client_start_read_frames(smts_dvr_client_t *dvr_client);

/**
 * stop dvr client called by session.
 */
int smts_dvr_client_stop(smts_dvr_client_t *client);

/**
 * inherit from {@link tcp_client_read_packet_cb}
 */
int on_dvr_client_read_frame_cb(abstract_tcp_client_t *aclient, uv_buf_t *buf, int status);

#endif /* SMTS_MOCK_DVR_IMPL_H_ */
