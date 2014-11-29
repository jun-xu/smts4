/*
 * test_mock_dvr.c
 *
 *  Created on: 上午10:24:11
 *      Author: ss
 */

#ifndef TEST_MOCK_DVR_H_
#define TEST_MOCK_DVR_H_
#include <stdlib.h>
#include <stdint.h>
#include "smts_abstract_tcp_client.h"
#include "tcp_server.h"
#include "uv/uv.h"

#define DEFAULT_MOCK_DVR_PORT 50001

#define IFRAME_INTERVAL_NUM 50
#define IFRAME_TIMES_OF_P_FREAME 20

typedef struct test_mock_dvr_s
{
	ABSTRACT_TCP_CLIENT_FILEDS
	int32_t bitrate;
	int32_t frame_rate;
	int32_t seqno;
	int32_t interval;
	uv_buf_t frame_bin;
	uv_buf_t iframe_bin;
	uv_timer_t timer;
} test_mock_dvr_t;

/**
 * API. start mock dvr on port.
 */
int start_mock_dvr(smts_tcp_server_t *tcp_server, uv_loop_t *loop, int port);

int init_test_mock_dvr(test_mock_dvr_t *dvr, uv_loop_t *loop);

int destroy_test_mock_dvr(test_mock_dvr_t *dvr, int status);

/**
 * inherit from {@link cmd_imp}
 */
int mock_dvr_preview_impl(abstract_tcp_client_t* client, abstract_cmd_t *preview_cmd);

#endif /* TEST_MOCK_DVR_H_ */
