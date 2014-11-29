/*
 * smts_dispatch.h
 *
 *  Created on: 上午11:56:39
 *      Author: ss
 */

#ifndef SMTS_DISPATCH_H_
#define SMTS_DISPATCH_H_

#include "smts_client_impl.h"
#include "smts_proto.h"

typedef int (*cmd_imp)(abstract_tcp_client_t *client, abstract_cmd_t *abstrct_cmd);

/**
 * inherit from {@link cmd_imp}
 */
int fun_no_impl(abstract_tcp_client_t *client, abstract_cmd_t *abstrct_cmd);

/**
 *	dispatch packet
 */
#define GEN_SWITCH_CASE_ARGS(_type,_name)
#define GEN_SWITCH_CASE_3ARGS(_type,_name,_len)
#define GEN_DISPATCH_FUN(fun) (fun) == NULL?fun_no_impl:(fun)

#define GEN_DISPATCH_SWTICH_CASE(cmd_name,_packet,cmd,_fileds,fun)	\
case cmd:{													\
	cmd_name##_t *t = (cmd_name##_t*)malloc(sizeof(cmd_name##_t));			\
	r = cmd_name##_t_init(t);								\
	r = cmd_name##_t_decode(buf,t);							\
	if(r != 0){												\
		CL_ERROR("decode packet:%s error.\n",#cmd_name);	\
	}else{													\
		/*callback fun */									\
		cmd_imp f = GEN_DISPATCH_FUN(fun);					\
		f(client,(abstract_cmd_t*)t);						\
	}														\
	cmd_name##_t_destroy(t);								\
	}														\
	break;

/**
 * inherit from {@link tcp_client_read_packet_cb}
 */
int dispatch_packet(abstract_tcp_client_t* client, uv_buf_t *buf, int status);

#ifdef SMTS_TEST
int mock_dvr_preview_impl(abstract_tcp_client_t* client, abstract_cmd_t *preview_cmd);
#endif

#endif /* SMTS_DISPATCH_H_ */
