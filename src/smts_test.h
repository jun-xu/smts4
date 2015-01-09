/*
 * my_test.h
 *
 *  Created on: 上午11:04:31
 *      Author: ss
 */

#ifndef SMTS_TEST_H_
#define SMTS_TEST_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "smts_util.h"
#include "css_logger.h"
#include "mem_guard.h"
/**
 * test case function interface.
 */
typedef void (*test_case)(void);

typedef enum
{
	TCASE_INACTIVE = 0, TCASE_ACTIVE
} test_case_active_t;

/**
 * add test func here!   func name must start with 'test_'
 * XX(name,active) like XX(name,0) or XX(name,TCASE_ACTIVE)
 */
#ifdef SMTS_TEST
#define SMTS_TEST_CASES(XX)  			\
	XX(tcp_server,0)					\
	XX(css_logger_suite,0)				\
	XX(struct_encode_decode_suite,0)	\
	XX(preview_suite,1)					\
	XX(session_manager_suite,0)			\
	XX(dvrs_info_suite,0)				\
	XX(mem_guard_suite,0)				\
	XX(net_addrs_suite,0)				\

#else
#define SMTS_TEST_CASES(XX)
#endif

/**
 * private
 */
typedef struct my_test_case_s
{
	char *name;		/// test case name
	test_case main; /// test case function
	int active;		/// 1:active 0:inactive
} my_test_case_t;

/**
 * declare all test case func.
 */
#define CTEST_DECLARE(name,_) \
	void test_##name(void);
SMTS_TEST_CASES(CTEST_DECLARE)

/**
 * add test case to suite.
 */
#define CTEST_ENTITY(name,active) \
	{ #name, &test_##name, active },

/**
 * test cases.
 */
my_test_case_t suite[] = {
SMTS_TEST_CASES(CTEST_ENTITY) { 0, 0, 0 } };

#define PRINTF_TEST_CAST_HEADER(name)	\
	printf(CL_CLR(GREEN,"Running test_case: test_%s\n---------------------------------------------------\n"),(name))

#define PRINTF_TEST_CASE_TAIL(name,cost)		\
	printf(CL_CLR(GREEN,"Test end of test case:%s , cost:") 				\
		   CL_CLR(MAGENTA,"%ldms") CL_CLR(GREEN,".\n---------------------------------------------------\n\n"),(name),(cost))

#define PRINTF_DO_TEST_HEADER()				 \
do {																					\
	printf(CL_CLR(GREEN,"---------------------------------------------------\n"));		\
	printf(CL_CLR(GREEN,"   R U N N I N G    T E S T S \n"));							\
	printf(CL_CLR(GREEN,"---------------------------------------------------\n\n"));	\
}while(0)

#define PRINTF_my_DO_TEST_TAIL(t,active,ignore,total)						\
do{																			\
	printf(CL_CLR(GREEN,"\nTotal ") 										\
		   CL_CLR(MAGENTA,"%d") 											\
		   CL_CLR(GREEN," test case, active:")								\
		   CL_CLR(MAGENTA,"%d")	CL_CLR(GREEN,", skipped:")					\
		   CL_CLR(RED,"%d") CL_CLR(GREEN,", total cost:")					\
		   CL_CLR(MAGENTA,"%ldms")											\
		   CL_CLR(GREEN,".\n"), (t), (active), (ignore), (total));  		\
}while(0)

static long difftimeval(struct timeval* tv1, struct timeval* tv0)
{
	int64_t t0, t1;
	t0 = tv0->tv_sec * 1000 + tv0->tv_usec / 1000;
	t1 = tv1->tv_sec * 1000 + tv1->tv_usec / 1000;
	return (long) (0xffffffff & (t1 - t0));
}
/**
 * do test case by name.
 */
void my_do_test_by(char *name);
void my_do_test_by(char *name)
{
	int i = 0, ignore = 0, active = 0;
	struct timeval st, et;
	long cost, total_cost = 0;
	PRINTF_DO_TEST_HEADER();
	while (suite[i].name != NULL) {
		if (strcmp(name, suite[i].name) == 0) {
			gettimeofday(&st, NULL);
			PRINTF_TEST_CAST_HEADER(suite[i].name);
			suite[i].main();
			gettimeofday(&et, NULL);
			cost = difftimeval(&et, &st);
			total_cost += cost;
			PRINTF_TEST_CASE_TAIL(suite[i].name, cost);
			active++;
			break;
		}
		i++;
	}
	PRINTF_my_DO_TEST_TAIL(1, active, 1 - active, total_cost);
}
/**
 * test main function, test all.
 */
void my_do_test_all();
void my_do_test_all()
{
	int i = 0, ignore = 0, active = 0;
	struct timeval st, et;
	long cost, total_cost = 0;
#ifdef MEM_GUARD
	clear_guard();
#endif
	PRINTF_DO_TEST_HEADER();
	while (suite[i].main != NULL) {
		if (suite[i].active) {
			gettimeofday(&st, NULL);
			PRINTF_TEST_CAST_HEADER(suite[i].name);
			suite[i].main();
			gettimeofday(&et, NULL);
			cost = difftimeval(&et, &st);
			total_cost += cost;
#ifdef MEM_GUARD
			printf_all_ptrs();
#endif
			PRINTF_TEST_CASE_TAIL(suite[i].name, cost);
			active++;
		} else {
			ignore++;
		}
		i++;
	}
	PRINTF_my_DO_TEST_TAIL(i, active, ignore, total_cost);
}

void test_test_case(void)
{
	printf("start test case test body.\n");
	printf("start test case test body.\n");
	printf("start test case test body.\n");
	printf("start test case test body.\n");
}

#endif /* SMTS_TEST_H_ */
