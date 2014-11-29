/*
 * tcp_server.h
 *
 *  Created on: 2014-6-4
 *      Author: sunshine
 */

#ifndef TCP_SERVER_H_
#define TCP_SERVER_H_

#include "uv/uv.h"

#define DEFAUTL_LISTEN_PORT 65003
#define DEFAULT_BACKLOG_LENGTH 128

#define SELF_LISTEN_IP "0.0.0.0"

typedef enum
{
	SOCKET_TCP = 0, SOCKET_UDP, PIPE
} stream_type;

typedef enum
{
	TCP_SERVER_STATUS_UNINIT = 0, TCP_SERVER_STATUS_INIT, TCP_SERVER_STATUS_RUNNING,
} tcp_server_status;

typedef void (*start_stop_cb)();

typedef struct smts_tcp_server_s
{
	uv_loop_t *loop;
	int port;
	char *listen_ip;
	tcp_server_status status;
	struct sockaddr_in addr;
	uv_tcp_t server_socket;
	start_stop_cb start_cb;
	start_stop_cb stop_cb;
} smts_tcp_server_t;

int init_tcp_server(smts_tcp_server_t *tcp_server, uv_loop_t *loop, int port);
int start_tcp_server(smts_tcp_server_t *tcp_server, start_stop_cb cb, uv_connection_cb connect_cb);
int stop_tcp_server(smts_tcp_server_t *tcp_server, start_stop_cb cb);
int destroy_tcp_server(smts_tcp_server_t *tcp_server);

#endif /* TCP_SERVER_H_ */
