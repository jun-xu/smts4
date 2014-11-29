/*
 * smts_client_impl.c
 *
 *  Created on: 下午1:36:53
 *      Author: ss
 */

#ifndef SMTS_CLIENT_C_
#define SMTS_CLIENT_C_
#include "smts_dispatch.h"
#include "smts_util.h"
#include "css_logger.h"
#include "smts_session.h"

#include "encode_decode_util.h"
#include "smts_proto.h"
#include "smts_client_impl.h"
#include "smts_errorno.h"

/**
 * for mem watch.
 */
#include "mem_guard.h"
/**
 * inherit from {@link tcp_client_connect_cb}
 */
static void socket_close_cb(abstract_tcp_client_t* client, int status)
{
	smts_client_t *c = (smts_client_t*) (client);
	CL_DEBUG("client:%d closed.\n", client->socket.io_watcher.fd);
	FREE(c);

}

int stop_smts_client(smts_client_t *c, int status)
{
	if (c != NULL) {
		if (c->status != SMTS_CLIENT_ON_STOP) {
			c->status = SMTS_CLIENT_ON_STOP;
			CL_INFO("stop smts_client fd:%d,status:%d.\n", c->socket.io_watcher.fd, status);
			if (c->session != NULL) {
				smts_client_stop_preview(c->session, c);
			}
			close_abstract_tcp_client((abstract_tcp_client_t*) c, socket_close_cb);
		}
	}
	return 0;
}

/**
 * when new client connected.
 */
static void init_smts_client(smts_client_t *smts_client, uv_loop_t *loop)
{
	init_abstract_tcp_client((abstract_tcp_client_t*) smts_client, loop, PACK0);
	QUEUE_INIT(&smts_client->queue);
	smts_client->session = NULL;
	smts_client->status = SMTS_CLIENT_ON_INIT;
}

void client_on_connection(uv_stream_t* server, int status)
{
	smts_client_t *smts_client = (smts_client_t*) malloc(sizeof(smts_client_t));
	init_smts_client(smts_client, server->loop);
	uv_tcp_t* socket = &smts_client->socket;
	int r = uv_accept(server, (uv_stream_t*) socket);
	if (r != 0) {
		CL_ERROR("accept error:%d,%s\n", r, smts_strerror(r));
		close_abstract_tcp_client((abstract_tcp_client_t*) smts_client, socket_close_cb);
		return;
	}
	CL_DEBUG("new client fd:%d connection.\n", socket->io_watcher.fd);
	r = tcp_client_start_read((abstract_tcp_client_t*) smts_client, dispatch_packet);
	if (r != 0) {
		CL_ERROR("start read error:%d,%s\n", r, smts_strerror(r));
		close_abstract_tcp_client((abstract_tcp_client_t*) smts_client, socket_close_cb);
		return;
	}
}

static void on_smts_client_send_preview_res_cb_slient(abstract_tcp_client_t *aclient, abstract_cmd_t *packet,
		int status)
{
	preview_cmd_res_t *preview_res = (preview_cmd_res_t*) packet;
	preview_cmd_res_t_destroy(preview_res);

	CL_DEBUG("send preview res to client:%d status:%d\n", aclient->socket.io_watcher.fd, status);
	smts_client_t *smts_client = (smts_client_t*) aclient;
	smts_client->status = SMTS_CLIENT_ON_SEND_FRAMES;
}

void smts_client_send_preview_res(smts_client_t *client, int status)
{
	int r = status;
	if (client->status == SMTS_CLIENT_ON_INIT) {
		preview_cmd_res_t *preview_res = (preview_cmd_res_t*) malloc(sizeof(preview_cmd_res_t));
		preview_cmd_res_t_init(preview_res);
		preview_res->status = r;
		preview_res->Hue = 0;
		preview_res->brightness = 0;
		preview_res->contrast = 0;
		preview_res->saturation = 0;
		preview_cmd_res_t_bufs_alloc(preview_res);
		preview_cmd_res_t_encode(preview_res);
		client->status = SMTS_CLIENT_ON_SEND_PREVIEW_RES; // to avoid repeat send preview response
		r = tcp_client_send_msg((abstract_tcp_client_t*) client, (abstract_cmd_t*) preview_res,
				on_smts_client_send_preview_res_cb_slient);
		if(r != 0){
			CL_ERROR("send preview res error:%d,%s.\n",r,smts_strerror(r));
			preview_cmd_res_t_destroy(preview_res);
		}
	}
	if (r != 0) {
		CL_ERROR("send preview response error:%d,%s\n", r, smts_strerror(r));
		stop_smts_client(client, r);
	}
	return;
}

static void on_smts_client_send_frame_cb(abstract_tcp_client_t *aclient, abstract_cmd_t *packet, int status)
{
	smts_client_t *client = (smts_client_t*) aclient;
	smts_frame_res_t *frame = (smts_frame_res_t*) packet;
	on_smts_client_send_frame(client->session, client, frame);
	if (status != 0) {
		CL_ERROR("send frame error:%d,%s\n", status, smts_strerror(status));
		stop_smts_client(client, status);
	}
}
int smts_client_send_frame(smts_client_t *client, smts_frame_res_t *frame)
{
	int r = 0;
	if (client->status == SMTS_CLIENT_ON_SEND_FRAMES) {
//	CL_DEBUG("send frame:%d to client:%d.\n", frame->seqno, client->socket.io_watcher.fd);
		r = tcp_client_send_msg((abstract_tcp_client_t*) client, (abstract_cmd_t*) frame, on_smts_client_send_frame_cb);
		if (r != 0) {
			CL_ERROR("send frame error:%d,%s\n", r, smts_strerror(r));
			on_smts_client_send_frame(client->session, client, frame);
			stop_smts_client(client, r);
		}
	} else {
		// ignore, wait client to send preview response first.
		on_smts_client_send_frame(client->session, client, frame);
	}
	return r;
}

static void on_smts_client_send_error_preview_res_cb(abstract_tcp_client_t *aclient, abstract_cmd_t *packet, int status)
{
	smts_client_t *smts_client = (smts_client_t*) aclient;
	preview_cmd_res_t *preview_res = (preview_cmd_res_t*) packet;
	preview_cmd_res_t_destroy(preview_res);
	stop_smts_client(smts_client, status);
}

/**
 * start preview.
 */
int nvmp_smts_preview(abstract_tcp_client_t* aclient, abstract_cmd_t *preview_cmd)
{

	preview_cmd_t *play_cmd = (preview_cmd_t*) preview_cmd;
	smts_client_t *client = (smts_client_t*) aclient;
	play_cmd_t play;
	int r;
	play.dvr_id = play_cmd->dvr_id;
	play.channel_no = play_cmd->channel_no;
	play.frame_mode = play_cmd->frame_mode;
	CL_DEBUG("dispatch preview play cmd:{%lld,%d,%d}.\n", play.dvr_id, play.channel_no, play.frame_mode);
	r = smts_start_preview(client, &play);
	if (r != 0) {
		CL_ERROR("start preview error:%d,%s\n", r, smts_strerror(r));
		preview_cmd_res_t *preview_res = (preview_cmd_res_t*) malloc(sizeof(preview_cmd_res_t));

		preview_res->status = r;
		preview_res->Hue = 0;
		preview_res->brightness = 0;
		preview_res->contrast = 0;
		preview_res->saturation = 0;
		preview_cmd_res_t_bufs_alloc(preview_res);
		preview_cmd_res_t_encode(preview_res);
		r = tcp_client_send_msg((abstract_tcp_client_t*) aclient, (abstract_cmd_t*) preview_res,
				on_smts_client_send_error_preview_res_cb);
	}

	return r;
}

#endif /* SMTS_CLIENT_C_ */
