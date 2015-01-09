/*
 * test_tcp_server.c
 *
 *  Created on: 下午5:21:52
 *      Author: ss
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
	uv_thread_t tid;
	uv_async_t req;
	smts_tcp_server_t *tcp_server = (smts_tcp_server_t*) malloc(sizeof(smts_tcp_server_t));
	uv_loop_init(&loop);
	assert(0==init_tcp_server(tcp_server,&loop,DEFAUTL_LISTEN_PORT));
	uv_thread_create(&tid, start_service_thread, tcp_server);

	Sleep(1000);
	assert(0 == stop_tcp_server(tcp_server,NULL));
	/**
	 * __APPLE__ : kqueue not return if no other events. so, send aync event to weakup kqueue.
	 */
	uv_async_init(&loop, &req, close_async_handle);
	uv_async_send(&req);

	uv_thread_join(&tid);
	assert(0 == destroy_tcp_server(tcp_server));
}

void test_tcp_server()
{
	test_common_tcp_server();
}
