/*
 * mem_guard.c
 *
 *  Created on: 下午5:17:20
 *      Author: ss
 */

#include <stdlib.h>
#include "css_logger.h"
#include "queue.h"

typedef struct
{
	QUEUE entity;
} malloc_pool;

typedef struct
{
	char *file;
	int32_t line;
	const char *fun;
	size_t s;
	void *ptr;
	QUEUE q;

} malloc_entity;

static malloc_pool pool = { 0 };
void init_mem_guard()
{
	QUEUE_INIT(&pool.entity);
}

void *malloc_guard(char *file, int32_t line, const char *fun, size_t s)
{
	malloc_entity *e = (malloc_entity*) malloc(sizeof(malloc_entity));
	QUEUE_INIT(&e->q);
	e->file = file;
	e->fun = fun;
	e->line = line;
	e->s = s;
	void *ptr = malloc(s);
	e->ptr = ptr;
	QUEUE_INSERT_HEAD(&pool.entity, &e->q);
//	printf("malloc %s.%d.%s:%p\n",file,line,fun,ptr);
	return ptr;
}

void free_guard(void *ptr)
{
	QUEUE *q;
	malloc_entity *c = NULL;
	QUEUE_FOREACH(q,&pool.entity)
	{
		c = QUEUE_DATA(q, malloc_entity, q);
		if (c->ptr == ptr) {
			break;
		}
	}
	free(ptr);
	if (c == NULL) {
		CL_DEBUG("ptr:%p not guard.\n", ptr);
	} else {
		QUEUE_REMOVE(&c->q);
		free(c);
	}

}

void printf_all_ptrs()
{
	QUEUE *q;
	malloc_entity *c = NULL;
	int count = 0;
	CL_DEBUG("------------printf all not free ptrs.---------------\n");
	QUEUE_FOREACH(q,&pool.entity)
	{
		c = QUEUE_DATA(q, malloc_entity, q);
		CL_DEBUG("not free ptr:%p of %s:%d.%s.\n", c->ptr, c->file, c->line, c->fun);
		count++;
	}
	CL_DEBUG("------------total:%d ptrs not free!.\n-------------", count);
}

#include <assert.h>
void test_mem_guard_suite()
{
	int i;
	void *ptrs[10];
	init_mem_guard();
	for (i = 0; i < 10; i++) {
		ptrs[i] = malloc_guard("1", 2, "3", 4);
	}
	for (i = 0; i < 10; i++) {
		free_guard(ptrs[i]);
	}
	printf_all_ptrs();
}

