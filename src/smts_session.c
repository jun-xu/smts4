/*
 * smts_session.c
 *
 *  Created on: 下午8:44:42
 *      Author: ss
 *
 *	preview.x  :  preview workflow.
 *
 *  exit case:
 *  exit.1.x  :  when smts client close by remote.
 *	exit.2.1.x  :  init dvr client error.
 *	exit.2.2.x	:  connect to dvr error
 *	exit.2.3.x	:  send preview cmd to dvr error.
 *
 */
#include "uv/uv.h"

#include "smts_util.h"
#include "smts_session.h"
#include "smts_errorno.h"
#include "css_logger.h"
#include "session_manager.h"
#include "queue.h"
#include "smts_errorno.h"

/**
 * for mem watch.
 */
#include "mem_guard.h"

static int add_smts_client(smts_session_t *s, smts_client_t *client)
{
	CL_DEBUG("client preview start,fd:%d.\n", client->socket.io_watcher.fd);
	QUEUE_INSERT_HEAD(&s->client, &client->queue);
	s->client_size++;
	client->session = s;
	return 0;
}

int stop_smts_session(smts_session_t *s)
{
	QUEUE *q;
	delete_session((abstract_session_t*) s);
	QUEUE_FOREACH(q,&s->client)
	{
		smts_client_t *c = QUEUE_DATA(q, smts_client_t, queue);
		smts_client_send_preview_res(c, s->status);
		stop_smts_client(c, s->status);
	}
//	smts_dvr_client_stop(&s->dvr);
	return 0;
}

void on_dvr_client_close_cb(smts_session_t *s)
{
	// exit.1.6. callback when dvr client close. free session.
	CL_INFO("free session:%s\n", s->name);
	FREE(s);
}

static void session_exit_cb(uv_timer_t* handle)
{
	smts_session_t *s = (smts_session_t*) handle->data;
	if (s->client_size == 0) {
		CL_INFO("session%s terminate:%d,%s.\n", s->name, s->exit_code, smts_strerror(s->exit_code));
		// exit.1.4. delete session from session manager
		delete_session((abstract_session_t*) s);
		// exit.1.5. close dvr client.
		smts_dvr_client_stop(&s->dvr);
	} else {
		CL_INFO("session:%s wake up.\n", s->name);
		s->exit_code = SMTS_OK;
	}
}

static void wait_exit_session(smts_session_t *s)
{
	// exit.1.3 waite some time to exit.
	CL_INFO("all client disconnected, session:%s sleep:%d to wait to stop.\n", s->name, SESSION_WAIT_EXIT_TIMEOUT);
	uv_timer_start(&s->wait_exit_timer, session_exit_cb, SESSION_WAIT_EXIT_TIMEOUT, 0);
}

int smts_client_stop_preview(smts_session_t *s, smts_client_t* client)
{
	// exit.1.1 client stop.
	CL_DEBUG("client preview stop,fd:%d.\n", client->socket.io_watcher.fd);
	QUEUE_REMOVE(&client->queue);
	s->client_size--;
	client->session = NULL;
	if (s->client_size == 0) {
		// exit.1.2 wait exit when client list is empty.
		wait_exit_session(s);
	}

	return 0;
}

// preview.5. send preview cmd.
void on_connected_to_dvr(smts_session_t* s, abstract_tcp_client_t *client, int status)
{
	QUEUE *q;
	int r = status;
	if (status != 0) {
		//exit.2.2. connect to dvr error.
		CL_ERROR("connect to dvr:%d error:%d,%s.\n", client->socket.io_watcher.fd, status, smts_strerror(status));
		r = DVR_CONNECT_REFUSED_ERROR;
		s->exit_code = r;
		stop_smts_session(s);
	} else {
		CL_DEBUG("connect to dvr.[ok].\n");
		r = smts_dvr_client_preview(&s->dvr);
		if (r != 0) {
			//exit.2.3 send preview cmd to dvr error;
			r = DVR_SEND_PRIVIEW_CMD_ERROR;
			s->exit_code = r;
			stop_smts_session(s);
		}
	}

}

void on_smts_client_send_frame(smts_session_t* session, smts_client_t *client, smts_frame_res_t *frame)
{
//	CL_DEBUG("after send frame, status:%d\n", status);
	frame->ref--;
	if (frame->ref == 0) {
		smts_frame_t *f = (smts_frame_t *) frame->data;
		smts_frame_res_t_destroy(frame);
		destroy_smts_frame(f);
	}
}

void on_smts_dvr_client_recv_frame(smts_session_t* s, smts_frame_t *frame, int status)
{
	QUEUE *q;
	int r = 0;
	if (status != 0) {
		CL_ERROR("recv frame error:%d,%s\n", status, smts_strerror(status));
		s->exit_code = status;
		stop_smts_session(s);
		return;
	}

	CL_DEBUG("recv frame:%d,%d, client size:%d\n", frame->seqno, frame->type, s->client_size);
	// preview.7. send frame to clients.
	if (s->client_size > 0) {
		smts_frame_res_t *frame_res = (smts_frame_res_t*) malloc(sizeof(smts_frame_res_t));
		smts_frame_res_t_init(frame_res);
		frame_res->seqno = frame->seqno;
		frame_res->frame.base = frame->frame_data.base;
		frame_res->frame.len = frame->frame_data.len;
		frame_res->data = frame;
		smts_frame_res_t_encode(frame_res);
		frame_res->ref += s->client_size;
		QUEUE_FOREACH(q,&s->client)
		{
			smts_client_t *c = QUEUE_DATA(q, smts_client_t, queue);
//			printf("start send frame to c:%d\n", c->socket.io_watcher.fd);
			smts_client_send_frame(c, frame_res);
		}
	} else {
		destroy_smts_frame(frame);
	}

}

// preview.6. response to clients and start read frame.
void on_smts_dvr_client_send_preview(smts_session_t* s, abstract_tcp_client_t *client, int status)
{

	QUEUE *q;
	if (status != 0) {
		CL_ERROR("send dvr preview cmd error:%d,%s\n", status, smts_strerror(status));
		//exit.2.3 send preview cmd to dvr error;
		s->exit_code = status;
		stop_smts_session(s);
		return;
	}
	if (s->client_size > 0) {
		QUEUE_FOREACH(q,&s->client)
		{
			smts_client_t *c = QUEUE_DATA(q, smts_client_t, queue);
			smts_client_send_preview_res(c, SMTS_OK);
		}
	}
	s->status = SMTS_SESSION_RUNNING;
	status = smts_dvr_client_start_read_frames(&s->dvr);
	if (status != 0) {
		CL_ERROR("send dvr preview cmd error:%d,%s\n", status, smts_strerror(status));
		s->exit_code = status;
		stop_smts_session(s);
		return;
	}

}

int smts_start_preview(smts_client_t* client, play_cmd_t *play)
{
	int r = 0;
	session_key_t key = { 0 };
	smts_session_t *s;
	CL_INFO("start play:{%lld,%d,%d}.\n", play->dvr_id, play->channel_no, play->frame_mode);
	key.dvr_id = play->dvr_id;
	key.channel_no = play->channel_no;
	key.frame_mode = play->frame_mode;
	key.type = SESSION_TYPE_SMTS;
	// preview.1. add to session manager, create it if not find.
	s = (smts_session_t*) get_session(&key);
	if (s == NULL) {
		// new smts session.
		s = (smts_session_t*) malloc(sizeof(smts_session_t));
		init_smts_session(s, client->loop, play->dvr_id, play->channel_no, play->frame_mode);

		CL_DEBUG("create new session:%s\n", s->name);
		// preview.2. add smts client to session.
		r = add_smts_client(s, client);
		CL_DEBUG("add client. client size:%d\n", s->client_size);
		if (r != 0) {
			CL_ERROR("add client to session error:%d\n", r);
			goto init_session_error;
		}
		// preview.3. init dvr client.
		r = init_smts_dvr_client(&s->dvr, s->loop, play->dvr_id, play->channel_no, play->frame_mode);
		if (r != 0) {
			CL_ERROR("init dvr client error:%d,%s\n", r, smts_strerror(r));
			//exit.2.1. init dvr client error.
			goto init_session_error;
		}
		r = add_session((abstract_session_t*) s);
		if (r != 0) {
			CL_ERROR("add to session manager error:%d\n", r);
			goto init_session_error;
		}
		// preview.4. connect to dvr.
		r = smts_dvr_client_connect(&s->dvr, PACK4);
		// callback when dvr client connected. method: when_smts_dvr_client_connected(int status).
		if (r != 0) {
			CL_ERROR("connect to dvr error:%d.\n", r);
			delete_session((abstract_session_t*) s);
			goto init_session_error;
		}
		return r;
		// when error happend.
		init_session_error: client->session = NULL;
		FREE(s);

		return r;

	} else {
		// 2. add smts client to session.
//		client->session = s;
		add_smts_client(s, client);
		smts_client_send_preview_res(client, SMTS_OK);
	}

	return r;
}

int init_smts_session(smts_session_t *s, uv_loop_t *loop, int64_t dvr_id, int16_t channel_no, int32_t frame_mode)
{
	s->loop = loop;
	memset(&s->key, 0, sizeof(session_key_t));
	uv_timer_init(loop, &s->wait_exit_timer);
	s->wait_exit_timer.data = s;
	s->exit_code = SMTS_OK;
	s->key.type = SESSION_TYPE_SMTS;
	QUEUE_INIT(&s->client);
	s->next = NULL;
	s->client_size = 0;
	s->key.dvr_id = dvr_id;
	s->key.channel_no = channel_no;
	s->key.frame_mode = frame_mode;
	s->dvr.session = s;
	s->exit_code = SMTS_OK;
	sprintf(s->name, "%s_%lld_%d_%d", "smts_session", dvr_id, channel_no, frame_mode);
	s->status = SMTS_SESSION_INIT;
	return 0;
}

int destroy_smts_session(smts_session_t *s)
{
	return 0;
}
