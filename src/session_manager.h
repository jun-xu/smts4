/*
 * session_manager.h
 *
 *  Created on: 下午9:15:49
 *      Author: ss
 */

#ifndef SESSION_MANAGER_H_
#define SESSION_MANAGER_H_

#include "smts_abstract_session.h"
#include "queue.h"

typedef struct session_manager_s
{
	QUEUE head;
	int32_t size;
} session_manager_t;

void init_session_manager();

/**
 * compare to keys
 * return 0: equal, -1: diff.
 */
int compare_session_key(session_key_t *key1, session_key_t *key2);

/**
 * add session to session_manager
 * return 0: ok, other:error_code.
 */
int add_session(abstract_session_t *session);
/**
 * delete session from session_manager
 * return 0: ok, other:error_code.
 */
int delete_session(abstract_session_t *session);
/**
 * get session by key.
 */
abstract_session_t* get_session(session_key_t *key);

/**
 * return size of sessions.
 */
int size_of_sessions();
/**
 * printf all session by key.
 */
void to_string();

#endif /* SESSION_MANAGER_H_ */
