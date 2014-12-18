/*
 * smts_tcp_client_interface.h
 *
 *  Created on: 下午6:16:31
 *      Author: ss
 */

#ifndef SMTS_TCP_CLIENT_INTERFACE_H_
#define SMTS_TCP_CLIENT_INTERFACE_H_
#include "uv/uv.h"
#include "smts_proto.h"

/**
 * abstract tcp client module.
 */
struct abstract_tcp_client_s;

/**
 * inherit from {@link abstract_tcp_client_s}
 * connect to dvr. send preview msg, read frames.
 */
struct smts_dvr_client_s;

/**
 * inherit from {@link abstract_tcp_client_s}
 * connect to client. read preview msg, send frames.
 */
struct smts_client_s;

/**
 * abstract read package callback function.
 * callback when read a package.
 */
typedef int (*tcp_client_read_packet_cb)(struct abstract_tcp_client_s *client, uv_buf_t *packet, int status);

typedef void (*tcp_client_close_cb)(struct abstract_tcp_client_s *client, int status);
/**
 * abstract send package callback function.
 * callback when send a package.
 */
typedef void (*tcp_client_send_packet_cb)(struct abstract_tcp_client_s *client, abstract_cmd_t *packet, int status);
/**
 * callback when close socket.
 */
typedef void (*tcp_client_connect_cb)(struct abstract_tcp_client_s *client, int status);
/**
 * used by tcp read.
 */
typedef struct
{
	int32_t packet_len;
	uv_buf_t buf;
	int32_t offset;
} client_read_tmp_buf_t;

#define ABSTRACT_TCP_CLIENT_FILEDS														\
		uv_loop_t *loop; /* private. uv loop. init by {@link init_smts_dvr_client}*/  	\
		uv_tcp_t socket; /* private. socket. init by {@link init_smts_dvr_client}*/  	\
		struct sockaddr_in addr; /* private */											\
		int32_t packet_opt;	/* packet size. 0 for nvmp proto or 4 for erlang server.*/	\
		client_read_tmp_buf_t recv_tmp_buf; /* private. buf for read frame.*/			\
		tcp_client_read_packet_cb read_packet_cb;/*private. callback function when read package.used in method:{@link tcp_client_start_read}*/\
		tcp_client_close_cb close_cb;/*callback when close tcp client.*/				\
/**
 * abstract tcp client module.
 */
typedef struct abstract_tcp_client_s
{
	ABSTRACT_TCP_CLIENT_FILEDS
} abstract_tcp_client_t;

/**
 * write request.
 */
typedef struct client_write_req_s
{
	uv_write_t req;			/// used in uv_write method.
	abstract_tcp_client_t *client;	/// tcp client.
	abstract_cmd_t *abstract_cmd;	/// send package.
	/**
	 * callback function after send package
	 * used in method {@link tcp_client_send_msg}
	 */
	tcp_client_send_packet_cb cb;
} client_write_req_t;

typedef struct client_connect_req_s
{
	uv_connect_t req;
	abstract_tcp_client_t *client;	/// tcp client.
	tcp_client_connect_cb cb;		/// on tcp client connected.
} client_connect_req_t;

int init_abstract_tcp_client(abstract_tcp_client_t *client, uv_loop_t *loop, int pack_opt);

int close_abstract_tcp_client(abstract_tcp_client_t *client, tcp_client_close_cb close_cb);

void init_client_read_tmp_buf(client_read_tmp_buf_t *buf);

void destroy_client_read_tmp_buf(client_read_tmp_buf_t *buf);
/**
 * connect to server.
 * @param client tcp_client
 * @param ip ip
 * @param port
 * @connect_cb	 callback when connected.
 * @packet_opt  0 or 4.
 * @return 0:ok, other:errorcode
 */
int tcp_client_connect(abstract_tcp_client_t *client, char *ip, int32_t port, tcp_client_connect_cb connect_cb,
		int packet_opt);
/**
 * start read package from client socket.
 * @param client tcp client
 * @param read_packet_cb callback when read package
 * @return 0:ok, other:errorcode.
 */
int tcp_client_start_read(abstract_tcp_client_t *client, tcp_client_read_packet_cb read_packet_cb);
int tcp_client_read_stop(abstract_tcp_client_t *client);
/**
 * write msg to tcp socket.
 * @param client tcp_client.
 * @param packetage send package.
 * @param send_packet_cb callback after send msg.
 * @return 0:ok, other:errorcode.
 */
int tcp_client_send_msg(abstract_tcp_client_t *client, abstract_cmd_t *packet, tcp_client_send_packet_cb send_packet_cb);

#endif /* SMTS_TCP_CLIENT_INTERFACE_H_ */
