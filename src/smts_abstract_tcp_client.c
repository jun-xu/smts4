/*
 * smts_tcp_client.c
 *
 *  Created on: 下午7:27:32
 *      Author: ss
 */

#include "smts_abstract_tcp_client.h"
#include <stddef.h>
#include "css_logger.h"
#include "smts_proto.h"
#include "encode_decode_util.h"
#include "smts_errorno.h"
#include "net_addrs_util.h"

/**
 * for mem watch.
 */
#include "mem_guard.h"

int init_abstract_tcp_client(abstract_tcp_client_t *client, uv_loop_t *loop, int pack_opt)
{
	client->socket.data = client;
	client->read_packet_cb = NULL;
	client->close_cb = NULL;
	client->packet_opt = pack_opt;
	client->loop = loop;
	uv_tcp_init(loop, &client->socket);
	init_client_read_tmp_buf(&client->recv_tmp_buf);
	return 0;

}

static void socket_close_cb(uv_handle_t* handle)
{
	abstract_tcp_client_t *c = (abstract_tcp_client_t*) (handle->data);
	//free abstract packet if exist.
	destroy_client_read_tmp_buf(&c->recv_tmp_buf);
	if (c->close_cb != NULL) {
		c->close_cb(c, SMTS_OK);
	}
}

int close_abstract_tcp_client(abstract_tcp_client_t *client, tcp_client_close_cb close_cb)
{
	if (client != NULL) {
		client->socket.data = client;
		client->close_cb = close_cb;
		uv_close((uv_handle_t*) (&client->socket), socket_close_cb);
	}
	return 0;
}

void init_client_read_tmp_buf(client_read_tmp_buf_t *packet)
{
	packet->buf.base = NULL;
	packet->buf.len = 0;
	packet->packet_len = 0;
	packet->offset = 0;
}

void destroy_client_read_tmp_buf(client_read_tmp_buf_t *buf)
{
	FREE(buf->buf.base);
}

static void tcp_client_send_msg_cb(uv_write_t* req, int status)
{
	client_write_req_t *send_req = (client_write_req_t*) req->data;
	if (send_req->cb != NULL) {
		send_req->cb(send_req->client, send_req->abstract_cmd, status);
	}
	FREE(send_req);
}

int tcp_client_send_msg(abstract_tcp_client_t *client, abstract_cmd_t *packet, tcp_client_send_packet_cb cb)
{
	client_write_req_t *send_req = (client_write_req_t*) malloc(sizeof(client_write_req_t));
	send_req->client = client;
	send_req->abstract_cmd = packet;
	send_req->req.data = send_req;
	send_req->cb = cb;
	return uv_write(&send_req->req, (uv_stream_t*) &client->socket, packet->codec_buf.original_buf,
			packet->codec_buf.original_buf_len, tcp_client_send_msg_cb);
}
/**
 * inherit from {@link uv_alloc_cb}
 */
static void tcp_client_alloc_cb(uv_handle_t* handle, size_t size, uv_buf_t* buf)
{
//	CL_DEBUG("read freame alloc.\n");
	if (size > 0) {
		abstract_tcp_client_t *client = (abstract_tcp_client_t*) handle->data;
		client_read_tmp_buf_t *packet = &client->recv_tmp_buf;
		if (packet->offset < PACKET_LEN_FIELD_SIZE) {
			buf->base = (char*) &packet->packet_len + packet->offset;
			buf->len = PACKET_LEN_FIELD_SIZE - packet->offset;
		} else if (packet->offset == PACKET_LEN_FIELD_SIZE) {
			// read packet body. packet_len should +4 when read from mock dvr which write by erlang.
			int32_t packet_len = decode_int32(buf->base) + client->packet_opt;
			if (packet_len <= PACKET_LEN_FIELD_SIZE || packet_len > MAX_PACKET_LEN) {
				// packet invalid, close handle in read_cb.
				CL_ERROR("invalide packet len:%d.\n", packet_len);
				packet->packet_len = PACKET_INVALID;
				buf->base = NULL;
				buf->len = 0;
			} else {
				packet->packet_len = packet_len;
				packet->buf.base = (char*) malloc(packet->packet_len);
				packet->buf.len = packet_len;
				encode_int32(packet->buf.base, 0, packet_len);
				buf->base = packet->buf.base + PACKET_LEN_FIELD_SIZE;
				buf->len = packet_len - PACKET_LEN_FIELD_SIZE;
			}
		} else {
			//continue read body.
			buf->base = packet->buf.base + packet->offset;
			buf->len = packet->packet_len - packet->offset;
		}
	}
}
/**
 * inherit from {@link uv_read_cb}
 */
static void tcp_client_read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{
	abstract_tcp_client_t *c = (abstract_tcp_client_t*) stream->data;
	client_read_tmp_buf_t *packet = &c->recv_tmp_buf;
	//	CL_DEBUG("read frame: %ld bytes.\n", nread);
	if (packet->packet_len == PACKET_INVALID) {
		CL_ERROR("read packet error.\n");

		c->read_packet_cb(c, NULL, DVR_RECV_INVALID_PACKET_ERROR);
		return;
	}
	if (nread < 0) {
		CL_INFO("fd:%d read cmd data error:%ld,%s\n", c->socket.io_watcher.fd, nread, smts_strerror(nread));
//		CL_INFO("fd:%d alloc:%d recv:%d\n", c->socket.io_watcher.fd, packet->offset, packet->packet_len);
		FREE(packet->buf.base);
		init_client_read_tmp_buf(packet);
		if (c->read_packet_cb != NULL) {
			c->read_packet_cb(c, NULL, nread);
		} else {
			CL_WARN("no read packet cb impl when error!.\n");
		}
		return;
	}

	if (nread > 0) {
		if (((nread + packet->offset) == packet->packet_len) && packet->packet_len > PACKET_LEN_FIELD_SIZE) {
			//read body completed.
			packet->offset += nread;
			//			CL_DEBUG("read packet:%d\n", packet->offset);
			if (c->read_packet_cb != NULL) {
				// warnning: should take care of packet.buf.base buffer in read_pack_cb.
				c->read_packet_cb(c, &packet->buf, 0);
			} else {
				CL_WARN("no read packet cb impl.\n");
				FREE(packet->buf.base);
			}
			init_client_read_tmp_buf(packet);

		} else {
			//continue read.
			packet->offset += nread;
		}
	}
}

int tcp_client_start_read(abstract_tcp_client_t *client, tcp_client_read_packet_cb read_packet_cb)
{
	client->socket.data = client;
	client->read_packet_cb = read_packet_cb;
	init_client_read_tmp_buf(&client->recv_tmp_buf);
	return uv_read_start((uv_stream_t*) &client->socket, tcp_client_alloc_cb, tcp_client_read_cb);
}

int tcp_client_read_stop(abstract_tcp_client_t *client)
{
	CL_INFO("stop read of client:%d\n", client->socket.io_watcher.fd);
	return uv_read_stop((uv_stream_t*) &client->socket);
}

void on_tcp_client_connect_cb(uv_connect_t* req, int status)
{
	client_connect_req_t *creq = (client_connect_req_t*) req->data;

	if (creq->cb != NULL) {
		creq->cb(creq->client, status);
	} else {
		CL_WARN("no connect cb.\n");
	}
	FREE(creq);
}

int tcp_client_connect(abstract_tcp_client_t *client, char *ip, int32_t port, tcp_client_connect_cb connect_cb,
		int packet_opt)
{
	int r = 0;
	char *local_ip;
	client_connect_req_t *req = (client_connect_req_t*) malloc(sizeof(client_connect_req_t));
	struct sockaddr_in addr;
	client->packet_opt = packet_opt;
	r = uv_tcp_init(client->loop, &client->socket);
	local_ip = smts_get_one_addrs();
	if (local_ip == NULL) {
		r = NET_ADDR_NOT_FOUND;
		CL_ERROR("get local addr error:%d,%s.\n", r, smts_strerror(r));
		return r;
	}
	r = uv_ip4_addr(local_ip, 0, &addr);
	if (r != 0) {
		CL_ERROR("uv_ip4_addr error\n");
		return r;
	}
	r = uv_tcp_bind(&client->socket, (struct sockaddr *) &addr, 0);
	if (r != 0) {
		CL_ERROR("bind ip:%s error:%d,%s\n", ip, r, smts_strerror(r));
		return r;
	}

	CL_DEBUG("bind local addr:%s .\n", local_ip);
	r = uv_ip4_addr(ip, port, &client->addr);
	if (r != 0) {
		CL_ERROR("uv_ip4_addr error\n");
		return r;
	}
	req->client = client;
	req->req.data = req;
	req->cb = connect_cb;
	r = uv_tcp_connect(&req->req, &client->socket, (const struct sockaddr*) &client->addr, on_tcp_client_connect_cb);
	return r;
}

