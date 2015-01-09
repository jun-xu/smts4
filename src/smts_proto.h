/*
 * smts_proto.h
 *
 *  Created on: 下午3:08:24
 *      Author: ss
 */

#ifndef SMTS_PROTO_H_
#define SMTS_PROTO_H_

#ifndef protocol_name
/**
 * use nvmp packet format
 */
#include "nvmp_protocol_def.h"
#include "nvmp_protocol.h"
#else

#endif

int proto_cmd_t_increase_ref(abstract_cmd_t *t);
int proto_cmd_t_len(abstract_cmd_t *t);
int proto_cmd_t_decode(uv_buf_t *packet,abstract_cmd_t *t);
int proto_cmd_t_encode(abstract_cmd_t *t);
int proto_cmd_t_destroy(abstract_cmd_t *t);


#endif /* SMTS_PROTO_H_ */
