/*
 * mem_guard.c
 *
 *  Created on: 下午5:17:20
 *      Author: ss
 */

#include <stdlib.h>
#include "css_logger.h"
#include "queue.h"

#ifdef _WIN32

#else
#include <execinfo.h>
#endif

#define MAX_DUMP_STRACK_TRACE_DEPTH 10

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
	/**
	 * stack of ptr malloc.
	 */
	char **stack_strings;
	int stack_depth;
	QUEUE q;

} malloc_entity;

static malloc_pool pool = { 0 };
static uv_mutex_t mem_mutex;

void init_mem_guard()
{
	QUEUE_INIT(&pool.entity);
	uv_mutex_init(&mem_mutex);
}

void *malloc_guard(char *file, int32_t line, const char *fun, size_t s)
{
	malloc_entity *e = (malloc_entity*) malloc(sizeof(malloc_entity));
	void *stack_trace[MAX_DUMP_STRACK_TRACE_DEPTH] = { 0 };
	void *ptr = malloc(s);
	QUEUE_INIT(&e->q);
	e->file = file;
	e->fun = fun;
	e->line = line;
	e->s = s;
	e->ptr = ptr;
#ifdef _WIN32
	e->stack_depth = 0;
	e->stack_strings = NULL;
#else
	/**
	 * get all fun addr of stack traces.
	 */
	e->stack_depth = backtrace(stack_trace, MAX_DUMP_STRACK_TRACE_DEPTH);
	/**
	 * change to fun names;
	 */
	e->stack_strings = (char**) backtrace_symbols(stack_trace, e->stack_depth);
#endif
	uv_mutex_lock(&mem_mutex);
	QUEUE_INSERT_HEAD(&pool.entity, &e->q);
	uv_mutex_unlock(&mem_mutex);
//	printf("malloc %s.%d.%s:%p\n",file,line,fun,ptr);
	return ptr;
}

void free_guard(void *ptr)
{
	QUEUE *q;
	malloc_entity *c = NULL;
	uv_mutex_lock(&mem_mutex);
	QUEUE_FOREACH(q,&pool.entity)
	{
		c = QUEUE_DATA(q, malloc_entity, q);
		if (c->ptr == ptr) {
			break;
		}
	}
	uv_mutex_unlock(&mem_mutex);
	free(ptr);
	if (c == NULL) {
		CL_DEBUG("ptr:%p not guard.\n", ptr);
	} else {
		uv_mutex_lock(&mem_mutex);
		QUEUE_REMOVE(&c->q);
		uv_mutex_unlock(&mem_mutex);
		free(c->stack_strings);
		free(c);
	}

}

void printf_all_ptrs()
{
	QUEUE *q;
	malloc_entity *c = NULL;
	int count = 0;
	int i;
	CL_DEBUG("------------printf all not free ptrs.---------------\n");
	QUEUE_FOREACH(q,&pool.entity)
	{
		c = QUEUE_DATA(q, malloc_entity, q);
		CL_DEBUG("not free ptr:%p of %s:%d.%s.\nStack Trace:\n", c->ptr, c->file, c->line, c->fun);
		for (i = 1; i < c->stack_depth; i++) {
			CL_DEBUG("[%d] %s \n", i - 1, c->stack_strings[i]);
		}
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

