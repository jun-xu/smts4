/*
 * mem_guard.h
 *
 *  Created on: 下午5:17:13
 *      Author: ss
 */

#ifndef MEM_GUARD_H_
#define MEM_GUARD_H_
/**
 * just for debug!.
 */
void init_mem_guard();
void *malloc_guard(char *file, int32_t line,const char *fun, size_t t);
void free_guard(void *ptr);

#ifdef MEM_GUARD
#define malloc(t) malloc_guard(__FILE__,__LINE__,__FUNCTION__,t)
#define free free_guard
#endif

void printf_all_ptrs();
#endif /* MEM_GUARD_H_ */
