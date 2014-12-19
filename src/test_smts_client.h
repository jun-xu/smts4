/*
 * test_smts_client.h
 *
 *  Created on: 上午9:50:51
 *      Author: ss
 */

#ifndef TEST_SMTS_CLIENT_H_
#define TEST_SMTS_CLIENT_H_

#include "uv/uv.h"
#include "smts_abstract_tcp_client.h"


#define CHECK_RES_CMD(cmd,buf) assert((cmd) == decode_int32((buf)->base+4))

#define TERMINATE_SEQNO 5

typedef struct test_smts_client_s
{
	ABSTRACT_TCP_CLIENT_FILEDS
	int64_t dvr_id;
	int cur_frame_seq;
	int recv_frame_account;
	int status;
} test_smts_client_t;

int test_smts_start_preivew(uv_loop_t *loop, test_smts_client_t *client, int64_t dvr_id);

#endif /* TEST_SMTS_CLIENT_H_ */
