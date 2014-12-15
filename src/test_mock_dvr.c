/*
 * test_mock_dvr.c
 *
 *  Created on: 上午10:33:45
 *      Author: ss
 */

#include "test_mock_dvr.h"
#include "smts_util.h"
#include "css_logger.h"
#include "tcp_server.h"
#include "smts_abstract_tcp_client.h"
#include "smts_errorno.h"
#include "smts_dispatch.h"
#include "smts_proto.h"
#include "smts_session.h"
#include "encode_decode_util.h"
#include <math.h>
#include <assert.h>

/**
 * for mem watch.
 */
#include "mem_guard.h"

int init_test_mock_dvr(test_mock_dvr_t *dvr, uv_loop_t *loop)
{
	init_abstract_tcp_client((abstract_tcp_client_t*) dvr, loop, PACK4);
	dvr->iframe_bin.base = NULL;
	dvr->iframe_bin.len = 0;
	dvr->frame_bin.base = NULL;
	dvr->frame_bin.len = 0;
	dvr->timer.data = dvr;
	uv_timer_init(loop, &dvr->timer);
	return 0;
}
/**
 * inherit from {@link tcp_client_connect_cb}
 */
static void mock_dvr_close_cb(abstract_tcp_client_t *aclient, int status)
{
	test_mock_dvr_t *dvr = (test_mock_dvr_t*) aclient;
	FREE(dvr->iframe_bin.base);
	FREE(dvr->frame_bin.base);
	FREE(aclient);
	CL_INFO("mock dvr stop.\n");
}

int destroy_test_mock_dvr(test_mock_dvr_t *dvr, int status)
{
	uv_timer_stop(&dvr->timer);
	close_abstract_tcp_client((abstract_tcp_client_t*) dvr, mock_dvr_close_cb);
	return 0;
}

static int moc_dvr_dispatch_packet(abstract_tcp_client_t *client, uv_buf_t *buf, int status)
{
	int r;
	test_mock_dvr_t* dvr = (test_mock_dvr_t*) client;
	if (status == 0) {

		int32_t cmd = decode_int32(buf->base + 4);
		switch (cmd) {
		PROTOCOL_MAP(GEN_DISPATCH_SWTICH_CASE, GEN_SWITCH_CASE_ARGS, GEN_SWITCH_CASE_3ARGS)
		default:
			CL_ERROR("ignore cmd:%d.\n", cmd);
			FREE(buf->base);
		}
	} else {
		CL_ERROR("connect to dvr error:%d,%s\n", status, smts_strerror(status));
		tcp_client_read_stop((abstract_tcp_client_t*) dvr);
//		destroy_test_mock_dvr(dvr, status);
	}
	return r;
}

/**
 * inheril from {@link uv_connection_cb}
 */
static void on_mock_dvr_connect_cb(uv_stream_t* server, int status)
{
	test_mock_dvr_t *dvr = (test_mock_dvr_t*) malloc(sizeof(test_mock_dvr_t));
	uv_tcp_t* socket;
	int r;
	init_test_mock_dvr(dvr, server->loop);
	socket = &dvr->socket;
	r = uv_accept(server, (uv_stream_t*) socket);
	if (r != 0) {
		CL_ERROR("accept error:%d,%s\n", r, smts_strerror(r));
		close_abstract_tcp_client((abstract_tcp_client_t*) dvr, mock_dvr_close_cb);
		return;
	}
	CL_DEBUG("new client fd:%d connection.\n", socket->io_watcher.fd);
	r = tcp_client_start_read((abstract_tcp_client_t*) dvr, moc_dvr_dispatch_packet);
	if (r != 0) {
		CL_ERROR("start read error:%d,%s\n", r, smts_strerror(r));
		close_abstract_tcp_client((abstract_tcp_client_t*) dvr, mock_dvr_close_cb);
		return;
	}

}

int start_mock_dvr(smts_tcp_server_t *tcp_server, uv_loop_t *loop, int port)
{
	int r = 0;
	r = init_tcp_server(tcp_server, loop, port);
	r = start_tcp_server(tcp_server, NULL, on_mock_dvr_connect_cb);
	CL_INFO("start mock dvr on port:%d.\n", port);
	return r;
}

static void on_mock_dvr_send_frame_cb(abstract_tcp_client_t *aclient, abstract_cmd_t *packet, int status)
{

	mock_dvr_frame_t *frame = (mock_dvr_frame_t*) packet;
	CL_DEBUG("free mock dvr frame:%d\n", frame->seqno);
	mock_dvr_frame_t_destroy(frame);
	if (status != 0) {
		test_mock_dvr_t *dvr;
		CL_ERROR("mock dvr send frame error:%d,%s\n", status, smts_strerror(status));
		dvr = (test_mock_dvr_t*) aclient;
		destroy_test_mock_dvr(dvr, status);
	}

}

static void dvr_send_frame0(uv_timer_t* handle)
{
	test_mock_dvr_t *dvr = (test_mock_dvr_t*) handle->data;
	int r = 0;
	mock_dvr_frame_t *frame = (mock_dvr_frame_t*) malloc(sizeof(mock_dvr_frame_t));
	mock_dvr_frame_t_init(frame);
	frame->seqno = dvr->seqno;
	if (dvr->seqno % IFRAME_INTERVAL_NUM == 0) {
		/// send i frame.
		frame->frame_type = FRAME_TYPE_INDEX;
		frame->frame.base = dvr->iframe_bin.base;
		frame->frame.len = dvr->iframe_bin.len;
	} else {
		/// send p frame.
		frame->frame_type = FRAME_TYPE_PB;
		frame->frame.base = dvr->frame_bin.base;
		frame->frame.len = dvr->frame_bin.len;
	}
	frame->st = dvr->seqno * dvr->interval;
	assert(0 == mock_dvr_frame_t_encode(frame));
	r = tcp_client_send_msg((abstract_tcp_client_t*) dvr, (abstract_cmd_t*) frame, on_mock_dvr_send_frame_cb);
	if (r != 0) {
		CL_ERROR("mock dvr send frame error:%d,%s\n", r, smts_strerror(r));
		mock_dvr_frame_t_destroy(frame);
		destroy_test_mock_dvr(dvr, r);
	}
	dvr->seqno++;
}

static int dvr_start_send_frame(test_mock_dvr_t *dvr)
{

	return uv_timer_start(&dvr->timer, dvr_send_frame0, 0, dvr->interval);
}

int mock_dvr_preview_impl(abstract_tcp_client_t* client, abstract_cmd_t *preview_cmd)
{
	int r = 0;
	int p_frame_bitreate, i_frame_bitrate;
	mock_dvr_preview_t *mock_preview_cmd = (mock_dvr_preview_t*) preview_cmd;
	test_mock_dvr_t *dvr = (test_mock_dvr_t*) client;
	CL_DEBUG("start mock dvr bitrate:%dkb, frameRate:%d.\n", mock_preview_cmd->bitrate, mock_preview_cmd->frame_rate);
	dvr->bitrate = mock_preview_cmd->bitrate;
	dvr->frame_rate = mock_preview_cmd->frame_rate;
	p_frame_bitreate = dvr->bitrate * 1024 * (IFRAME_INTERVAL_NUM / dvr->frame_rate)
			/ (IFRAME_TIMES_OF_P_FREAME + IFRAME_INTERVAL_NUM - 1);
	i_frame_bitrate = p_frame_bitreate * IFRAME_TIMES_OF_P_FREAME;
	dvr->interval = 1000 / dvr->frame_rate;

	dvr->frame_bin.base = (char*) malloc(p_frame_bitreate - 64 - 32);
	dvr->frame_bin.len = p_frame_bitreate - 64 - 32;

	dvr->iframe_bin.base = (char*) malloc(i_frame_bitrate);
	dvr->iframe_bin.len = i_frame_bitrate;

	dvr->seqno = 0;
	r = dvr_start_send_frame(dvr);
	return r;
}

