/*
 * smts_mock_dvr_impl.c
 *
 *  Created on: 上午11:13:03
 *      Author: ss
 */

#include "smts_mock_dvr_impl.h"
#include <stdlib.h>
#include "css_logger.h"
#include "smts_util.h"
#include "smts_proto.h"
#include "encode_decode_util.h"
#include "smts_session.h"
#include "smts_errorno.h"

/**
 * for mem watch.
 */
#include "mem_guard.h"
/**
 * inherit from {@link tcp_client_read_packet_cb}
 */
int on_dvr_client_read_frame_cb(abstract_tcp_client_t *aclient, uv_buf_t *buf, int status)
{
//	CL_DEBUG("read frame...\n");
	smts_dvr_client_t *c = (smts_dvr_client_t*) aclient;
	if (status == 0) {
		smts_frame_t *frame = (smts_frame_t*) malloc(sizeof(smts_frame_t));
		mock_dvr_frame_t *dvr_frame = (mock_dvr_frame_t*) malloc(sizeof(mock_dvr_frame_t));
		mock_dvr_frame_t_init(dvr_frame);
		proto_cmd_t_decode(buf, (abstract_cmd_t*)dvr_frame);
		frame->data = dvr_frame;
		frame->frame_data.base = dvr_frame->frame.base;
		frame->frame_data.len = dvr_frame->frame.len;
		frame->seqno = dvr_frame->seqno;
		frame->type = dvr_frame->frame_type;
		frame->st = dvr_frame->st;
//		CL_DEBUG("dvr client recv frame:%d,%d\n", frame->seqno, frame->type);
		///increase ref.
		proto_cmd_t_increase_ref((abstract_cmd_t*)dvr_frame);
		proto_cmd_t_destroy((abstract_cmd_t*)dvr_frame);
		on_smts_dvr_client_recv_frame(c->session, frame, 0);
	} else {
		CL_ERROR("read frame error:%d,%s\n", status, smts_strerror(status));
		tcp_client_read_stop(aclient);
		on_smts_dvr_client_recv_frame(c->session, NULL, DVR_RECV_FRAME_ERROR);

	}
	return 0;

}

int init_smts_dvr_client(smts_dvr_client_t *dvr_client, uv_loop_t *loop, int64_t dvr_id, int16_t channel_no,
		int32_t frame_mode)
{
	int r;
	r = init_dvr_connect_config(dvr_id, channel_no, frame_mode, &dvr_client->dvr_info);
	if (r != 0) {
		return r;
	}
	r = init_abstract_tcp_client((abstract_tcp_client_t*) dvr_client, loop, PACK4);

	r = uv_ip4_addr(dvr_client->dvr_info.ip, dvr_client->dvr_info.port, &dvr_client->addr);
	if (r != 0) {
		CL_ERROR("uv_ip4_addr ip:%s port:%d error\n", dvr_client->dvr_info.ip, dvr_client->dvr_info.port);
		r = DVR_CLIENT_INIT_ADDR_ERROR;
		return r;
	}
	CL_DEBUG("init dvr:{%s:%d} client. [ok]\n", dvr_client->dvr_info.ip, dvr_client->dvr_info.port);
	return r;
}

/**
 * inherit from {@link tcp_client_connect_cb}
 */
void connected_dvr_cb(abstract_tcp_client_t *client, int status)
{
	smts_dvr_client_t *dvr_client = (smts_dvr_client_t*) client;
	on_connected_to_dvr(dvr_client->session, client, status);
}

int smts_dvr_client_connect(smts_dvr_client_t *dvr_client, int packet_opt)
{
	/// 1. conncet to dvr.
	int r = 0;
	r = tcp_client_connect((abstract_tcp_client_t *) dvr_client, dvr_client->dvr_info.ip, dvr_client->dvr_info.port,
			connected_dvr_cb, packet_opt);
	return r;
}
/**
 * inherit from {@link tcp_client_send_packet_cb}
 */
static void on_dvr_client_send_preview_msg_cb(abstract_tcp_client_t *aclient, abstract_cmd_t *packet, int status)
{
// 4. callback to session.
	smts_dvr_client_t *client = (smts_dvr_client_t*) aclient;
//	mock_dvr_preview_t *cmd = (mock_dvr_preview_t*) packet;

	CL_DEBUG("send preview to dvr status:%d.\n", status);
	proto_cmd_t_destroy(packet);
	on_smts_dvr_client_send_preview(client->session, aclient, status);

}

int smts_dvr_client_preview(smts_dvr_client_t *dvr_client)
{
	int r = 0;
	/// 2. alloc ,decode preview request;
	mock_dvr_preview_t *preview_packet = (mock_dvr_preview_t*) malloc(sizeof(mock_dvr_preview_t));
	mock_dvr_preview_t_init(preview_packet);
	preview_packet->bitrate = DEFAULT_DVR_BITRATE;
	preview_packet->frame_rate = DEFAULT_DVR_FRAMERATE;

	r = proto_cmd_t_encode((abstract_cmd_t*)preview_packet);
	/// 3. send preview msg.
	r = tcp_client_send_msg((abstract_tcp_client_t*) dvr_client, (abstract_cmd_t*) preview_packet,
			on_dvr_client_send_preview_msg_cb);
	if (r != 0) {
		CL_ERROR("send preview cmd to dvr error:%s.\n", uv_err_name(r));
		on_smts_dvr_client_send_preview(dvr_client->session, (abstract_tcp_client_t*) dvr_client,
		DVR_SEND_PRIVIEW_CMD_ERROR);
	}
	return r;
}

int destroy_smts_frame(smts_frame_t *frame)
{
//	CL_DEBUG("free frame:%d,%d\n", frame->seqno, frame->type);
	proto_cmd_t_destroy((abstract_cmd_t*) frame->data);
	FREE(frame);
	return 0;
}

int smts_dvr_client_start_read_frames(smts_dvr_client_t *dvr_client)
{
	int r = 0;
// 5. start read frames
	init_client_read_tmp_buf(&dvr_client->recv_tmp_buf);
	dvr_client->socket.data = dvr_client;
	CL_DEBUG("start read frame, fd:%d.\n", dvr_client->socket.io_watcher.fd);
	r = tcp_client_start_read((abstract_tcp_client_t*) dvr_client, on_dvr_client_read_frame_cb);
	if (r != 0) {
		CL_ERROR("start read error:%s\n", uv_strerror(r));
		tcp_client_read_stop((abstract_tcp_client_t*) dvr_client);
		return r;
	}
	return r;
}

/**
 * inherit from {@link tcp_client_connect_cb}
 */
static void smts_dvr_client_close_cb(abstract_tcp_client_t *c, int status)
{
	smts_dvr_client_t *client = (smts_dvr_client_t*) c;
	on_dvr_client_close_cb(client->session);

}

int smts_dvr_client_stop(smts_dvr_client_t *client)
{
	CL_INFO("dvr client:%lld stop.\n", client->dvr_info.id);
//	uv_close((uv_handle_t*) &client->socket, smts_dvr_client_close_cb);
	close_abstract_tcp_client((abstract_tcp_client_t*) client, smts_dvr_client_close_cb);
	return 0;
}

