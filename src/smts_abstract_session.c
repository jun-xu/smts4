/*
 * smts_abstract_session.c
 *
 *  Created on: 下午6:18:14
 *      Author: ss
 */

#include "smts_abstract_session.h"
#include <string.h>

void init_abstract_session(abstract_session_t *session)
{
	QUEUE_INIT(&session->session);
	memset(&session->key, 0, sizeof(session_key_t));;
}
