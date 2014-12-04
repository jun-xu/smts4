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
#include "net_addrs_util.h"
#include "smts_client_impl.h"
#include "session_manager.h"

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

static void signal_handler(uv_signal_t *handle, int signum)
{
	char c;
	printf("BREAK: (a)bort (c)ontinue (m)em_guard\n");
	do {
		c = getc(stdin);
		switch (c) {
		case 'a':
			printf("exit(%d).\n", signum);
			exit(signum);
			break;
		case 'c':
			printf("continue.\n");
			break;
		case 'm':
			printf("printf mem.\n");
#ifdef MEM_GUARD
			printf_all_ptrs();
#endif
			break;
		default:
			if (c != '\n')
				printf("ingore '%c'.\n", c);
			break;
		}
	} while (c != '\n');
}

int main(int argc, char* argv[])
{
#ifdef MEM_GUARD
	init_mem_guard();
#endif

#ifdef SMTS_TEST
	my_do_test_all();

#else
	uv_signal_t sig_int;
	smts_tcp_server_t *tcp_server = (smts_tcp_server_t*) malloc(sizeof(smts_tcp_server_t));
	init_smts_addrs();
	init_session_manager();
	uv_loop_init(&loop);
	uv_signal_init(&loop, &sig_int);
	uv_signal_start(&sig_int, signal_handler, SIGINT);
	init_tcp_server(tcp_server, &loop, DEFAUTL_LISTEN_PORT);
	start_tcp_server(tcp_server, on_start_tcp_server_cb, client_on_connection);
	uv_run(&loop, UV_RUN_DEFAULT);
	destroy_tcp_server(tcp_server);
#endif
	return 0;
}

