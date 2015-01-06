/*
 * smts_dispatch.c
 *
 *  Created on: 上午11:57:01
 *      Author: ss
 */

#include "smts_dispatch.h"
#include "smts_util.h"
#include "css_logger.h"
#include "smts_session.h"

#include "encode_decode_util.h"
#include "smts_proto.h"
#include "smts_errorno.h"

#include "test_mock_dvr.h"

/**
 * for mem watch.
 */
#include "mem_guard.h"

int dispatch_packet(abstract_tcp_client_t *client, uv_buf_t *buf, int status)
{
	int r;
	smts_client_t* smts_client = (smts_client_t*) client;
	if (status == 0) {
		int32_t cmd = decode_int32(buf->base + 4);
		switch (cmd) {
		/**
		 * 1. new cmd packet. 	e.g. preview_cmd_t *t = (preview_cmd_t*)malloc(sizeof(preview_cmd_t));			\
		 * 2. init packet. 		e.g. preview_cmd_t_init(t);												\
		 * 3. decode packet. 	e.g. decode(buf,(abstract_cmd_t*)t);
		 * 4. call function.  	e.g. fun(client,(abstract_cmd_t*)t)
		 * 5. destroy packet. 	e.g. nvmp_cmd_t_destroy((abstract_cmd_t*)t)
		 */
		PROTOCOL_MAP(GEN_DISPATCH_SWTICH_CASE, GEN_SWITCH_CASE_ARGS, GEN_SWITCH_CASE_3ARGS)
		default:
			CL_ERROR("ignore cmd:%d.\n", cmd);
			FREE(buf->base);
		}
	} else {
		// errorcode SMTS_CLIENT_READ_ERROR
		if (status < 0) {
			status = SMTS_CLIENT_READ_ERROR;
		}
		CL_ERROR("smts client read error:%d,%s\n", status, smts_strerror(status));
		stop_smts_client(smts_client, status);
	}
	return r;
}

/**
 * dispatch packet.
 * decode packet and do something.
 */
int fun_no_impl(abstract_tcp_client_t* socket, abstract_cmd_t *abstract_cmd)
{
	CL_ERROR("cmd:%s(%d) not implement", abstract_cmd->name, abstract_cmd->cmd);
	return -1;
}

