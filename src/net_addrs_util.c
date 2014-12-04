/*
 * local_net_addr_util.c
 *
 *  Created on: 下午5:07:21
 *      Author: ss
 */

#include "net_addrs_util.h"
#include "uv/uv.h"
#include "css_logger.h"
#include "smts_errorno.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
/**
 * for mem watch.
 */
#include "mem_guard.h"

static stms_addrs_t addrs = { 0, 0, NULL };
static uv_mutex_t smts_addr_mutex;
static uv_once_t init_csmts_addrs_ = UV_ONCE_INIT;

static void smts_init_mutex_cb(void)
{
	uv_mutex_init(&smts_addr_mutex);
}

int init_smts_addrs()
{
	int r = 0, count, i;
	uv_interface_address_t* interfaces;
	uv_once(&init_csmts_addrs_, smts_init_mutex_cb);
	r = uv_interface_addresses(&interfaces, &count);
	if (r != 0) {
		CL_INFO("get addr error:%d %s\n", r, smts_strerror(r));
		return r;
	}
	for (i = 0; i < count; i++) {
		if (interfaces[i].address.address4.sin_family == AF_INET && interfaces[i].is_internal == 0) {
			addrs.count++;
		}
	}
	addrs.ips = malloc(addrs.count * MAX_IP_SIZE);
	addrs.index = 0;
//	printf("addrs.count:%d\n", addrs.count);
	for (i = 0; i < count; i++) {
		if (interfaces[i].address.address4.sin_family == AF_INET && interfaces[i].is_internal == 0) {
			uv_ip4_name(&interfaces[i].address.address4, (*addrs.ips)[addrs.index], MAX_IP_SIZE);
			CL_INFO("scan local ip addr[%d]:%s:%s.\n", addrs.index, interfaces[i].name, (*addrs.ips)[addrs.index]);
			addrs.index++;
		}
	}
	addrs.index = 0;
	return addrs.count;
}

void destory_smts_addrs()
{
	if (addrs.count > 0) {
		FREE(addrs.ips);
	}
}

char* smts_get_one_addrs()
{
	if (addrs.count > 0) {
		uv_mutex_lock(&smts_addr_mutex);
		char *ip = (*addrs.ips)[addrs.index++];
		addrs.index = addrs.index % addrs.count;
		uv_mutex_unlock(&smts_addr_mutex);
		return ip;
	}
	return NULL;
}

/**
 * test unit.
 */
void test_net_addrs_suite()
{
	int i;
	char *ip;
	assert(init_smts_addrs() > 0);
	for (i = 0; i < 10; i++) {
		ip = smts_get_one_addrs();
		printf("get ip:%s\n", ip);
		assert(ip!=NULL);
	}
	destory_smts_addrs();
}
