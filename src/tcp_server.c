/*
 * tcp_server.c
 *
 *  Created on: 2014-6-4
 *      Author: sunshine
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "uv/uv.h"
#include "smts_errorno.h"
//#include "util.h"
#include "css_logger.h"
#include "tcp_server.h"
#include "smts_proto.h"
#include "smts_client_impl.h"
#include "smts_dispatch.h"

/**
 * for mem watch.
 */
#include "mem_guard.h"

//static smts_tcp_server_t smts_tcp_server = { NULL, DEFAUTL_LISTEN_PORT,
//SELF_LISTEN_IP, TCP_SERVER_STATUS_UNINIT };

/**
 * init tcp server
 * return 0:success 1:error
 */
int init_tcp_server(smts_tcp_server_t *tcp_server, uv_loop_t *loop, int port)
{
	int r = 0;
	tcp_server->listen_ip = SELF_LISTEN_IP;
	tcp_server->loop = loop;
	tcp_server->port = port;
	tcp_server->status = TCP_SERVER_STATUS_INIT;
	return r;
}
/**
 * start listen.
 */
int start_tcp_server(smts_tcp_server_t *tcp_server, start_stop_cb cb, uv_connection_cb connect_cb)
{
	int r;
	if (tcp_server->status != TCP_SERVER_STATUS_INIT) {
		CL_ERROR("tcp server status(%d) error.\n", tcp_server->status);
		return -1;
	}
	r = uv_ip4_addr(tcp_server->listen_ip, tcp_server->port, &tcp_server->addr);
	if (r != 0) {
		CL_INFO("uv_ip4_addr error\n");
		return r;
	}
	r = uv_tcp_init(tcp_server->loop, &tcp_server->server_socket);
	if (r != 0) {
		CL_INFO("Socket creation error\n");
		return r;
	}
	r = uv_tcp_bind(&tcp_server->server_socket, (const struct sockaddr*) &tcp_server->addr, SOCKET_TCP);
	if (r != 0) {
		CL_INFO("bind socket error\n");
		return r;
	}
	r = uv_listen((uv_stream_t*) &tcp_server->server_socket, DEFAULT_BACKLOG_LENGTH, connect_cb);
	if (r) {
		CL_INFO("Listen error %s\n", uv_strerror(r));
		return r;
	}
	tcp_server->start_cb = cb;
	CL_INFO("start listen on %d [ok].\n", tcp_server->port);
	tcp_server->status = TCP_SERVER_STATUS_RUNNING;
	if (tcp_server->start_cb != NULL) {
		tcp_server->start_cb();
	}
	return 0;
}
/**
 * stop listen server.
 */
static void server_socket_close_cb(uv_handle_t* handle)
{
	smts_tcp_server_t *tcp_server = (smts_tcp_server_t*) handle->data;
	tcp_server->status = TCP_SERVER_STATUS_INIT;
	if (tcp_server->stop_cb != NULL) {
		tcp_server->stop_cb();
	}
	CL_INFO("stop tcp server [ok].\n");
	FREE(tcp_server);
}
int stop_tcp_server(smts_tcp_server_t *tcp_server, start_stop_cb cb)
{
	int r = 0;
	if (tcp_server->status != TCP_SERVER_STATUS_RUNNING) {
		CL_ERROR("tcp server status(%d) error.\n", tcp_server->status);
		return -1;
	}
	tcp_server->stop_cb = cb;
	CL_INFO("stop tcp server...\n");
	tcp_server->server_socket.data = tcp_server;
	uv_close((uv_handle_t*) (&tcp_server->server_socket), server_socket_close_cb);
	return r;
}

/**
 * destroy tcp server
 */
int destroy_tcp_server(smts_tcp_server_t *tcp_server)
{
	int r = 0;
	if (tcp_server->status == TCP_SERVER_STATUS_UNINIT)
		return 0;
	if (tcp_server->status != TCP_SERVER_STATUS_INIT) {
		CL_ERROR("tcp server status(%d) error.\n", tcp_server->status);
		return -1;
	}

	CL_INFO("destroy tcp server.\n");
	FREE(tcp_server);
	return 0;
}

/**
 * test util
 */
#include <assert.h>
uv_loop_t loop;

static void start_service_thread(void* arg)
{
	smts_tcp_server_t *tcp_server = (smts_tcp_server_t*) arg;
	assert(0 == start_tcp_server(tcp_server,NULL,client_on_connection));
	uv_run(&loop, UV_RUN_DEFAULT);
}

static void close_async_handle(uv_async_t* handle)
{
	uv_close((uv_handle_t*) handle, NULL);
}

static void test_common_tcp_server()
{

	smts_tcp_server_t *tcp_server = (smts_tcp_server_t*) malloc(sizeof(smts_tcp_server_t));
	uv_loop_init(&loop);
	assert(0==init_tcp_server(tcp_server,&loop,DEFAUTL_LISTEN_PORT));
	uv_thread_t tid;
	uv_thread_create(&tid, start_service_thread, tcp_server);

	Sleep(1000);
	assert(0 == stop_tcp_server(tcp_server,NULL));
	/**
	 * __APPLE__ : kqueue not return if no other events. so, send aync event to weakup kqueue.
	 */
	uv_async_t req;
	uv_async_init(&loop, &req, close_async_handle);
	uv_async_send(&req);

	uv_thread_join(&tid);
	assert(0 == destroy_tcp_server(tcp_server));
}

void test_tcp_server()
{
	test_common_tcp_server();
}

