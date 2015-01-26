/*
 * test_smts_client.c
 *
 *  Created on: 上午9:51:02
 *      Author: ss
 */

#include "test_smts_client.h"
#include "smts_proto.h"
#include <stddef.h>
#include <stdlib.h>
#include "css_logger.h"
#include "tcp_server.h"
#include "encode_decode_util.h"
#include "test_mock_dvr.h"
#include "smts_client_impl.h"
#include "session_manager.h"
#include "net_addrs_util.h"
#include <assert.h>

/**
 * for mem watch.
 */
#include "mem_guard.h"
static int test_client_on_read_packet(abstract_tcp_client_t *client, uv_buf_t *packet, int status);
static int init_test_smts_client(uv_loop_t *loop, test_smts_client_t *client)
{
	init_abstract_tcp_client((abstract_tcp_client_t*) client, loop, PACK0);
	client->cur_frame_seq = -1;
	client->status = 0;
	client->recv_frame_account = 0;
	return 0;
}

static void test_smts_client_close_cb(abstract_tcp_client_t* aclient, int status)
{
	test_smts_client_t *test_client = (test_smts_client_t*) aclient;
	FREE(test_client);
}

static void test_client_on_send_PTZ_cmd(abstract_tcp_client_t *client, abstract_cmd_t *packet, int status)
{
//	test_PTZ_cmd_t *req = (test_PTZ_cmd_t*)packet;
	proto_cmd_t_destroy(packet);
}

static void test_client_send_PTZ_cmd(abstract_tcp_client_t *aclient)
{
	test_PTZ_cmd_t *req;
	CL_DEBUG("test client fd:%d send ptz cmd.\n", aclient->socket.io_watcher.fd);
	req = (test_PTZ_cmd_t *) malloc(sizeof(test_PTZ_cmd_t));
	assert(0 == test_PTZ_cmd_t_init(req));
	req->ptz = 12;
	assert(0 == proto_cmd_t_encode((abstract_cmd_t* )req));
	assert(0 == tcp_client_send_msg(aclient, (abstract_cmd_t* ) req, test_client_on_send_PTZ_cmd));
}

static int test_client_on_read_packet(abstract_tcp_client_t *client, uv_buf_t *packet, int status)
{
//	assert(0 == status);
	test_smts_client_t *test_client = (test_smts_client_t*) client;
	if (status != 0) {
		return status;
	}

	int cmd = decode_int32((packet)->base + 4);
//	printf("client:%d recv cmd:%x status:%d\n", client->socket.io_watcher.fd, cmd, test_client->status);
	if (test_client->status == 0) {
		///read preview response.
		preview_cmd_res_t *res;
		CHECK_RES_CMD(PREVIEW_RES_CMD, packet);
		res = (preview_cmd_res_t*) malloc(sizeof(preview_cmd_res_t));
		assert(0 == preview_cmd_res_t_init(res));
		assert(0 == proto_cmd_t_decode(packet, (abstract_cmd_t* )res));
		CL_DEBUG("read preview response,status:%d.\n", res->status);
		assert(0 == res->status);
		test_client->status = 1;
		proto_cmd_t_destroy((abstract_cmd_t*) res);
		test_client_send_PTZ_cmd(client);
	} else {
		switch (cmd) {
		case SEND_FRAME_CMD: {
			/// read frame
			smts_frame_res_t *frame;
			CHECK_RES_CMD(SEND_FRAME_CMD, packet);
			frame = (smts_frame_res_t*) malloc(sizeof(smts_frame_res_t));
			assert(0 == smts_frame_res_t_init(frame));
			assert(0 == proto_cmd_t_decode(packet, (abstract_cmd_t* )frame));
			CL_DEBUG("test client read frame:%d type:%d, status:%d\n", frame->seqno, frame->frame_type, status);
			test_client->recv_frame_account++;
			if (test_client->cur_frame_seq == -1) {
				test_client->cur_frame_seq = frame->seqno;
			} else {
				assert(++test_client->cur_frame_seq == frame->seqno);
			}
			proto_cmd_t_destroy((abstract_cmd_t*) frame);
			if (test_client->recv_frame_account > TERMINATE_SEQNO) {
				test_client->socket.data = test_client;
				CL_DEBUG("test smts client fd:%d closed.\n", test_client->socket.io_watcher.fd);
				close_abstract_tcp_client(client, test_smts_client_close_cb);
				//			uv_close((uv_handle_t*) &test_client->socket, test_smts_client_close_cb);
			}
		}
			break;
		case COMMON_RES_CMD: {  ///test send ptz cmd
			common_res_t *res;
			CHECK_RES_CMD(COMMON_RES_CMD, packet);
			res = (common_res_t*) malloc(sizeof(common_res_t));
			assert(0 == common_res_t_init(res));
			assert(0 == proto_cmd_t_decode(packet, (abstract_cmd_t* )res));
			CL_DEBUG("recv common res:%d\n", res->status);
			assert(0 == res->status);
			proto_cmd_t_destroy((abstract_cmd_t*) res);
			break;
		}
		default:
			break;
		}

	}
	return 0;
}

static void test_client_on_send_preview_cmd(abstract_tcp_client_t *client, abstract_cmd_t *packet, int status)
{
	preview_cmd_t *req = (preview_cmd_t *) packet;
	proto_cmd_t_destroy((abstract_cmd_t*) req);
// 3. recv frame
	assert(0 == tcp_client_start_read(client, test_client_on_read_packet));

}

static void test_client_on_connected(abstract_tcp_client_t *aclient, int status)
{
	test_smts_client_t *client;
	preview_cmd_t *req;
	CL_DEBUG("test client fd:%d connected:%d\n", aclient->socket.io_watcher.fd, status);
	client = (test_smts_client_t*) aclient;
// 2. send preview cmd.
	req = (preview_cmd_t *) malloc(sizeof(preview_cmd_t));
	assert(0 == preview_cmd_t_init(req));
	req->src_addr = 2;
	req->dest_addr = 3;
	req->seqno = 4;
	req->dvr_id = client->dvr_id;
	req->channel_no = 6;
	memcpy(req->token, "token", sizeof("token"));
	req->frame_mode = 7;
	req->packet_len = proto_cmd_t_len((abstract_cmd_t*) req);
// alloc send bufs;
//	assert(0 == preview_cmd_t_bufs_alloc(req));
// encode packet
	assert(0 == proto_cmd_t_encode((abstract_cmd_t* )req));
	assert(0 == tcp_client_send_msg(aclient, (abstract_cmd_t* ) req, test_client_on_send_preview_cmd));

}

int test_smts_start_preivew(uv_loop_t *loop, test_smts_client_t *client, int64_t dvr_id)
{
	int r = 0;
// 1. connect to server.
	assert(0 == init_test_smts_client(loop, client));
	client->dvr_id = dvr_id;
	r = tcp_client_connect((abstract_tcp_client_t*) client, "127.0.0.1", DEFAUTL_LISTEN_PORT, test_client_on_connected,
	PACK0);
	return r;
}

void test_struct_encode_decode_suite()
{
	int len;
	char *token = "token";
	uv_buf_t *packet;
	preview_cmd_t *req, *req0;
	req = (preview_cmd_t*) malloc(sizeof(preview_cmd_t));
	req0 = (preview_cmd_t*) malloc(sizeof(preview_cmd_t));
	assert(0 == preview_cmd_t_init(req));
	req->src_addr = 2;
	req->dest_addr = 3;
	req->seqno = 4;
	req->dvr_id = 5;
	req->channel_no = 6;
	memcpy(req->token, "token", sizeof(token));
	req->frame_mode = 7;

	len = proto_cmd_t_len((abstract_cmd_t*) req);
	req->packet_len = len;

	assert(0 == preview_cmd_t_init(req0));
	assert(0 == proto_cmd_t_encode((abstract_cmd_t* )req));
	assert(1 == req->codec_buf.original_buf_len);
	packet = req->codec_buf.original_buf;
	req->codec_buf.original_buf = NULL;
	req->codec_buf.original_buf_len = 0;
	assert(0 == proto_cmd_t_decode(packet, (abstract_cmd_t* )req0));
	assert(req0->packet_len == req->packet_len);
	assert(req0->cmd == req->cmd);
	assert(req0->src_addr == req->src_addr);
	assert(req0->dest_addr == req->dest_addr);
	assert(req0->seqno == req->seqno);
	assert(strcmp(req0->token, req->token) == 0);

	assert(0 == proto_cmd_t_destroy((abstract_cmd_t* )req));
	assert(0 == proto_cmd_t_destroy((abstract_cmd_t* )req0));

}
/**
 * test preview.
 */
static uv_loop_t loop;
static uv_loop_t dvr_loop;
static uv_timer_t timer;
static smts_tcp_server_t *tcp_server = NULL;
static smts_tcp_server_t *dvr_tcp_server = NULL;
#define WAIT_TO_EXIT_TIMEOUT 5000
static int repert = 1;
#define TEST_CLIENT_SIZE 1

#ifdef MEM_GUARD
static void printf_mem_info(uv_timer_t *t) {
	printf_all_ptrs();
}
#endif

static void smts_client_start_new(uv_timer_t *t)
{
	int i;
	if (TEST_CLIENT_SIZE == 1)
		return;
	for (i = 0; i < TEST_CLIENT_SIZE; i++) {
		test_smts_client_t *test_client = (test_smts_client_t*) malloc(sizeof(test_smts_client_t));
		assert(0 == test_smts_start_preivew(&loop, test_client, i / 2));
	}
	if (--repert > 0) {
		uv_timer_start(&timer, smts_client_start_new, WAIT_TO_EXIT_TIMEOUT, 0);
	} else {
#ifdef MEM_GUARD
		uv_timer_start(&timer,printf_mem_info,WAIT_TO_EXIT_TIMEOUT,0);
#endif
	}
}

static void test_preview_start_tcp_server_cb()
{

	int i;
	for (i = 0; i < TEST_CLIENT_SIZE; i++) {
		test_smts_client_t *test_client = (test_smts_client_t*) malloc(sizeof(test_smts_client_t));
		assert(0 == test_smts_start_preivew(&loop, test_client, i / 2));
	}
	if (--repert > 0) {
		uv_timer_start(&timer, smts_client_start_new, WAIT_TO_EXIT_TIMEOUT, 0);
	} else {
#ifdef MEM_GUARD
		uv_timer_start(&timer,printf_mem_info,WAIT_TO_EXIT_TIMEOUT,0);
#endif
	}
}

void start_mock_dvr_server()
{
	uv_loop_init(&dvr_loop);
	dvr_tcp_server = (smts_tcp_server_t*) malloc(sizeof(smts_tcp_server_t));
	assert(0 == start_mock_dvr(dvr_tcp_server,&dvr_loop,DEFAULT_MOCK_DVR_PORT));
	uv_run(&dvr_loop, UV_RUN_DEFAULT);
	stop_tcp_server(dvr_tcp_server, NULL);
	destroy_tcp_server(dvr_tcp_server);
}

void start_smts_server()
{
	uv_loop_init(&loop);
	uv_timer_init(&loop, &timer);
	tcp_server = (smts_tcp_server_t*) malloc(sizeof(smts_tcp_server_t));
	assert(0==init_tcp_server(tcp_server,&loop,DEFAUTL_LISTEN_PORT));
	assert(0 == start_tcp_server(tcp_server, test_preview_start_tcp_server_cb, client_on_connection));

	uv_run(&loop, UV_RUN_DEFAULT);
}

void test_preview_suite()
{
	uv_thread_t thread_t;
	init_smts_addrs();
	init_session_manager();

	uv_thread_create(&thread_t, start_mock_dvr_server, NULL);
	Sleep(1000);
	start_smts_server();
	uv_thread_join(&thread_t);
	destroy_tcp_server(tcp_server);
}

