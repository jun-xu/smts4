/*
 * local_net_addr_util.h
 *
 *  Created on: 下午5:07:03
 *      Author: ss
 */

#ifndef NET_ADDR_UTIL_H_
#define NET_ADDR_UTIL_H_

#define MAX_IP_SIZE 16

typedef struct smts_addrs_s
{
	int count;
	int index;
	char (*ips)[][MAX_IP_SIZE];
} stms_addrs_t;

/**
 * return n>0:addrs count other:error_code
 */
int init_smts_addrs();

char* smts_get_one_addrs();

void destory_smts_addrs();
#endif /* NET_ADDR_UTIL_H_ */
