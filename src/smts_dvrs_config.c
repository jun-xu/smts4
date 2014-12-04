/*
 * smts_dvrs.c
 *
 *  Created on: 下午2:56:40
 *      Author: ss
 */

#include "smts_dvrs_config.h"
#include <stdlib.h>
#include "smts_util.h"
#include "string.h"

/**
 * for mem watch.
 */
#include "mem_guard.h"
/**
 * test dvrs config.
 */

#define DEFAULT_DVR_SIZE 5
static char *dvr_ips[] = { "192.168.3.19","192.168.3.14","192.168.3.15","192.168.3.11","192.168.3.18" };
//
//#define DEFAULT_DVR_SIZE 1
//static char *dvr_ips[] = { "127.0.0.1" };

int init_dvr_connect_config(int64_t dvr_id, int16_t channel_no, int32_t frame_mode, dvr_info_t *dvr)
{
	dvr->id = dvr_id;
	dvr->port = DEFAULT_DVR_PORT;
	dvr->frame_mode = frame_mode;

	dvr->ip = dvr_ips[(dvr_id % DEFAULT_DVR_SIZE)];
	return 0;

}

#ifdef SMTS_TEST
#include <assert.h>
void test_dvrs_info_suite()
{
	dvr_info_t info;
	assert(0 == init_dvr_connect_config(1, 2, 0,&info));
	assert(info.id == 1);
	assert(info.port == DEFAULT_DVR_PORT);
	assert(strcmp(info.ip, "192.168.3.12") == 0);

}
#endif
