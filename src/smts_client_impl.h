/*
 * smts_client.h
 *
 *  Created on: 上午9:47:00
 *      Author: ss
 */

#ifndef SMTS_CLIENT_IMPL_H_
#define SMTS_CLIENT_IMPL_H_

/**
 * new client connected.
 */
#include "uv/uv.h"
#include "smts_util.h"
#include "queue.h"
#include "smts_abstract_tcp_client.h"
#include "smts_abstract_session.h"

typedef enum
{
	SMTS_CLIENT_ON_INIT = 0, SMTS_CLIENT_ON_SEND_PREVIEW_RES = 1, SMTS_CLIENT_ON_SEND_FRAMES, SMTS_CLIENT_ON_STOP
} smts_client_status_t;

/**
 * inherit from {@link abstract_tcp_client_s}
 * connect to client. read preview msg, send frames.
 */
typedef struct smts_client_s
{
	ABSTRACT_TCP_CLIENT_FILEDS
	struct smts_session_s *session; /* smts session. */
	QUEUE queue;	/// use by smts server to queue all stms clients.
	smts_client_status_t status;

} smts_client_t;

/**
 * callback by {@link tcp_server} uv_listen method.
 * @param server tcp listen socket.
 * @param status
 */
void client_on_connection(uv_stream_t* server, int status);

/**
 * preview response.
 * @param slient  no callback if 1 (true) or 0(false) NOT callback to session.
 * @return 0:ok, other:errorcode.
 */
void smts_client_send_preview_res(smts_client_t *client, int status);
/**
 * send frame to client call by smts_session.
 * @param client smts client.
 * @param frame  frame.
 * @return 0:ok, other:errorcode.
 */
int smts_client_send_frame(smts_client_t *client, smts_frame_res_t *frame);

/**
 * stop client.  NOT thread safe, JUST callback this function in loop thread.
 * @param client smts client.
 * @param status
 * @return 0:ok, other:errorcode.
 */
int stop_smts_client(smts_client_t *client, int status);

/**
 * inherit from {@link cmd_imp}
 */
int nvmp_smts_preview(abstract_tcp_client_t* client, abstract_cmd_t *preview_cmd);
/**
 * inherit from {@link cmd_imp}
 * transparent channel. recv cmd packet from client and send to dvr.
 */
int nvmp_channel(abstract_tcp_client_t* aclient, abstract_cmd_t *ptz_cmd);

#endif /* SMTS_CLIENT_IMPL_H_ */
