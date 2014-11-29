/*
 * smts_dvrs.h
 *
 *  Created on: 下午2:08:58
 *      Author: ss
 */

#ifndef SMTS_DVRS_CONFIG_H_
#define SMTS_DVRS_CONFIG_H_

#include <stdint.h>

typedef enum
{
	DVR_TYPE_HD = 0, DVR_TYPE_D1 = 1
} dvr_type_t;

typedef struct dvr_info_s
{
	int64_t id;
	char *ip;
	int32_t port;
	int32_t frame_mode;
} dvr_info_t;

/**
 * gen test dvrs config.
 */

#define DEFAULT_DVR_BITRATE 1000		//bitrate.  KB
#define DEFAULT_DVR_FRAMERATE 25	//frame rate 25/s
#define DEFAULT_DVR_PORT 50001

int init_dvr_connect_config(int64_t dvr_id, int16_t channel_no, int32_t frame_mode, dvr_info_t *info);

#endif /* SMTS_DVRS_CONFIG_H_ */
