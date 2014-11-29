/*
 * smts_session_interface.h
 *
 *  Created on: 下午2:10:48
 *      Author: ss
 */

#ifndef SMTS_ABSTRACT_SESSION_H_
#define SMTS_ABSTRACT_SESSION_H_
#include "smts_proto.h"
#include "smts_abstract_tcp_client.h"

/**
 * struct defined.
 */
struct smts_session_s;
struct smts_frame_s;

typedef enum
{
	SESSION_TYPE_SMTS = 0
} session_type_t;
/**
 * session key.
 */
//#pragma pack(push)
//#pragma pack(1)
typedef struct session_key_s
{
	int64_t dvr_id;
	int16_t channel_no;
	int32_t frame_mode;
	session_type_t type;
} session_key_t;

//#pragma pack(pop)

#define SESSION_NAME_FILED_SIZE 64
#define ABSTRACT_SESSION_FILEDS 		\
	/* private*/						\
	char name[SESSION_NAME_FILED_SIZE];	\
	struct abstract_session_s *next;	\
	/* public */						\
	session_key_t key;					\

/**
 * private.
 * abstract session.
 */
typedef struct abstract_session_s
{
	ABSTRACT_SESSION_FILEDS

} abstract_session_t;

#endif /* SMTS_ABSTRACT_SESSION_H_ */
