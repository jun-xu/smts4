/*
 * smts_util.c
 *
 *  Created on: 下午1:16:09
 *      Author: ss
 */

#include "smts_util.h"
#include "css_logger.h"


/**
 * for mem watch.
 */
#include "mem_guard.h"

int ensure_dir(const char *sPathName)
{
	char DirName[MAX_PATH];
	int i, len = 0;
	strcpy(DirName, sPathName);
	len = (int)strlen(DirName);
	if (DirName[len - 1] != '/')
		strcat(DirName, "/");

	len = (int)strlen(DirName);

	for (i = 1; i < len; i++) {
		if (DirName[i] == '/' || DirName[i] == '\\') {
			DirName[i] = 0;
//			printf("%s\n", DirName);
			if (ACCESS(DirName, 0) != 0) {
				if (MKDIR(DirName) != 0) {
					CL_ERROR("mkdir %s error %d\n", DirName, GETLASTERROR);
					return -1;
				}
			}
			DirName[i] = '/';
		}
	}

	return 0;
}
#ifdef _WIN32
int gettimeofday(struct timeval *tp, void* notuse/*NULL*/)
{
	uint64_t intervals;
	FILETIME ft;

	GetSystemTimeAsFileTime(&ft);

	/*
	 * A file time is a 64-bit value that represents the number
	 * of 100-nanosecond intervals that have elapsed since
	 * January 1, 1601 12:00 A.M. UTC.
	 *
	 * Between January 1, 1970 (Epoch) and January 1, 1601 there were
	 * 134744 days,
	 * 11644473600 seconds or
	 * 11644473600,000,000,0 100-nanosecond intervals.
	 *
	 * See also MSKB Q167296.
	 */

	intervals = ((uint64_t) ft.dwHighDateTime << 32) | ft.dwLowDateTime;
	intervals -= 116444736000000000;

	tp->tv_sec = (long) (intervals / 10000000);
	tp->tv_usec = (long) ((intervals % 10000000) / 10);
	return 0;
}
#else
char* itoa(int value, char* result, int base)
{
	if (base < 2 || base > 36) {
		*result = '\0';
		return result;
	}
	char* ptr = result, *ptr1 = result, tmp_char;
	int tmp_value;
	do {
		tmp_value = value;
		value /= base;
		*ptr++ =
				"zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35
						+ (tmp_value - value * base)];
	} while (value);
	if (tmp_value < 0)
		*ptr++ = '-';
	*ptr-- = '\0';
	while (ptr1 < ptr) {
		tmp_char = *ptr;
		*ptr-- = *ptr1;
		*ptr1++ = tmp_char;
	}
	return result;
}
#endif
