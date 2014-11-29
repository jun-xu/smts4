/*
 * smts_app.c
 *
 *  Created on: 上午10:19:09
 *      Author: ss
 */

#include "smts_test.h"
#include "uv/uv.h"
#include <stddef.h>
#include <stdlib.h>
#include "css_logger.h"
#include "tcp_server.h"
#include "smts_client_impl.h"


/**
 * for mem watch.
 */
#include "mem_guard.h"
static uv_loop_t loop;
static void on_start_tcp_server_cb()
{
	css_logger_init(&loop);
	css_logger_start();

	CL_INFO("start smts server successful.\n");
}

int main(int argc, char* argv[])
{
#ifdef MEM_GUARD
	init_mem_guard();
#endif

#ifdef SMTS_TEST
	my_do_test_all();

#else
	smts_tcp_server_t *tcp_server = (smts_tcp_server_t*) malloc(sizeof(smts_tcp_server_t));
	uv_loop_init(&loop);
	init_tcp_server(tcp_server, &loop, DEFAUTL_LISTEN_PORT);
	start_tcp_server(tcp_server, on_start_tcp_server_cb, client_on_connection);
	uv_run(&loop, UV_RUN_DEFAULT);
	destroy_tcp_server(tcp_server);
#endif

	return 0;
}

