/*
 * session_manager.c
 *
 *  Created on: 下午9:18:36
 *      Author: ss
 */

#include <stdint.h>
#include <stdlib.h>
#include "css_logger.h"
#include "session_manager.h"
#include "smts_abstract_session.h"

/**
 * for mem watch.
 */
#include "mem_guard.h"
static session_manager_t session_manager = { 0 };

void init_session_manager()
{
	QUEUE_INIT(&session_manager.head);
	session_manager.size = 0;
}

int compare_session_key(session_key_t *k1, session_key_t *k2)
{
	return memcmp(k1, k2, sizeof(session_key_t));

}

int size_of_sessions()
{
	return session_manager.size;
}
/**
 * find session from manager.
 * return 0: not found, -1: already exit.
 */
static int is_contain(session_key_t *key)
{
	QUEUE *q;
	abstract_session_t *t = NULL;
	QUEUE_FOREACH(q,&session_manager.head)
	{
		t = QUEUE_DATA(q, abstract_session_t, session);
		if (compare_session_key(key, &t->key) == 0)
			return -1;
	}
	return 0;
//	abstract_session_t *t = session_manager.head;
//	while (t != NULL) {
//		if (compare_session_key(key, &t->key) == 0)
//			return -1;
//		t = t->next;
//	}
//	return 0;

}

int add_session(abstract_session_t *session)
{
	int r = 0;
	if (session == NULL)
		return -1;
	if ((r = is_contain(&session->key)) != 0) {
		CL_ERROR("session:%s already exit.\n", session->name);
		return r;
	}
	QUEUE_INSERT_HEAD(&session_manager.head, &session->session);

//	session->next = NULL;
//	if (session_manager.head == NULL) {
//		session_manager.head = session_manager.tail = session;
//	} else {
//		session_manager.tail->next = session;
//		session_manager.tail = session;
//	}
	session_manager.size++;
	return r;
}

abstract_session_t* get_session(session_key_t *key)
{
	QUEUE *q;
	abstract_session_t *t = NULL;
	QUEUE_FOREACH(q,&session_manager.head)
	{
		t = QUEUE_DATA(q, abstract_session_t, session);
		if (compare_session_key(key, &t->key) == 0)
			return t;
	}
//	CL_DEBUG("get session:{%lld,%d,%d} :%p\n", key->dvr_id, key->channel_no, key->frame_mode, t);
	return NULL;
}

int delete_session(abstract_session_t *session)
{
	int r = 0;
	if (session == NULL)
		return -1;
	if (is_contain(&session->key) == 0) {
		CL_ERROR("no session:%s to delete.\n", session->name);
		return r;
	}
	QUEUE_REMOVE(&session->session);
	session_manager.size--;
	return r;

}

void to_string()
{

	CL_ERROR("no implement.\n");
}
