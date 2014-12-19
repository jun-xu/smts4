/*
 * smts_session.h
 *
 *  Created on: 下午8:44:22
 *      Author: ss
 */

#ifndef SMTS_SESSION_H_
#define SMTS_SESSION_H_
#include <stdint.h>
#include "queue.h"
#include "smts_abstract_session.h"
#include "smts_client_impl.h"
#include "smts_mock_dvr_impl.h"
#include "smts_proto.h"

#ifdef SMTS_TEST
#define	SESSION_WAIT_EXIT_TIMEOUT 500
#else
#define SESSION_WAIT_EXIT_TIMEOUT 5000
#endif

typedef enum
{
	SMTS_SESSION_INIT = 0, SMTS_SESSION_RUNNING, SMTS_SESSION_STOP
} smts_session_status;
/**
 * smts server session
 * inherit {@link abstract_session_t}
 */
typedef struct smts_session_s
{
	ABSTRACT_SESSION_FILEDS
	uv_loop_t *loop;	/// {@link uv_loop_t}
	int32_t client_size; /// clients list size.
	QUEUE client;	/// clients list.
	struct smts_dvr_client_s dvr; /// dvr client, connect,preview,read frame and so on. {@link smts_dvr_client_t}
	smts_session_status status; //session status. {@link smts_session_status}
	/* for terminate */
	uv_timer_t wait_exit_timer;
	int exit_code;
} smts_session_t;

typedef struct play_cmd_s
{
	int64_t dvr_id;
	int16_t channel_no;
	int32_t frame_mode;
} play_cmd_t;

typedef enum
{
	FRAME_TYPE_INDEX = 1, FRAME_TYPE_PB
} frame_type_t;

/**
 * video frame.
 * create by {@link smts_dvr_client}
 */
typedef struct smts_frame_s
{
	void* data;		///  recv packet.
	uv_buf_t frame_data; /// frame data.
	frame_type_t type;	/// {@link frame_type_t}
	int32_t seqno;		/// seqno of frame.
	int64_t st;			/// start time of frame. ms.

} smts_frame_t;

/**
 * start preview.
 * @param client {@link smts_client_t}
 * @param play {@link play_cmd_t}
 * @return 0:ok. other:errorcode
 */
int media_client_start_preview(smts_client_t* client, play_cmd_t *play);

/**
 * smts client stop preview.
 * @param client {@link smts_client_t}
 * @param s {@link smts_session_t} preview session.
 * @return 0:ok. other:errorcode
 */
int media_client_stop_preview(smts_session_t *s, smts_client_t* client);

/**
 * send cmd to dvr.
 */
int media_client_send_cmd(smts_client_t *client, abstract_cmd_t* cmd);

/**
 * init smts session.
 * @param s {@link smts_session_t} smts session.
 * @param loop  {@link uv_loop_t}
 * @param dvr_id  dvr id.
 * @param channel_no channel_no
 * @param frame_mod  0 or 1.
 * @return 0:ok.  other:errorcode
 */
int init_media_session(smts_session_t *s, uv_loop_t *loop, int64_t dvr_id, int16_t channel_no, int32_t frame_mode);

int stop_media_session(smts_session_t *s);
/**
 * private. destory smts session.
 * @param s {@link smts_session_t} smts session.
 * @return 0:ok.  other:errorcode
 */
int destroy_smts_session(smts_session_t *s);

/**
 * private. callback when smts client send frame.
 */
void on_smts_client_send_frame(smts_session_t* session, smts_client_t *client, smts_frame_res_t *frame);
/**
 * private. callback when dvr client read frame.
 */
void on_smts_dvr_client_recv_frame(smts_session_t* s, smts_frame_t *frame, int status);
/**
 * private. callback when dvr client send preview.
 */
void on_smts_dvr_client_send_preview(smts_session_t* s, abstract_tcp_client_t *client, int status);
/**
 * private. callback when connect to dvr.
 */
void on_connected_to_dvr(smts_session_t* s, abstract_tcp_client_t *client, int status);
/**
 * private. callback when close dvr client.
 */
void on_dvr_client_close_cb(smts_session_t *s);

#endif /* SMTS_SESSION_H_ */
