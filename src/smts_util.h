/*
 * smts_util.h
 *
 *  Created on: 下午1:06:16
 *      Author: ss
 */

#ifndef SMTS_UTIL_H_
#define SMTS_UTIL_H_

#ifdef _WIN32
# include <direct.h>
# include <io.h>
# ifndef S_IRUSR
#  define S_IRUSR _S_IREAD
# endif
# ifndef S_IWUSR
#  define S_IWUSR _S_IWRITE
# endif
# define unlink _unlink
# define rmdir _rmdir
# define stat _stati64
# define open _open
# define write _write
# define lseek _lseek
# define close _close
#define snprintf _snprintf
#define vsnprintf _vsnprintf

#define GETLASTERROR GetLastError()
#define MKDIR(path) _mkdir(path)
#define ACCESS(x,y) _access((x),(y))

#include <time.h>
int gettimeofday(struct timeval *tp, void* /*NULL*/);

#else
#include <sys/time.h>
# include <unistd.h> /* unlink, rmdir, etc. */
# define Sleep(T) sleep((T)/1000)
#define GETLASTERROR errno
#define MKDIR(path) mkdir((path),0755)
#define ACCESS(x,y) access((x),(y))

char* itoa(int value, char* result, int base);

#endif

#ifndef MAX_PATH
#define MAX_PATH 256
#endif

#define FREE(x) do{if ((x) != NULL) {free((x));(x) = NULL;}}while(0)
/**
 * change struct member ptr to struct ptr.
 */
#define CHANGE_MEMBER_TO_TYPE(ptr, type, member) \
	((type *) ((char *) (ptr) - offsetof(type, member)))

int ensure_dir(const char *sPathName);

#endif /* SMTS_UTIL_H_ */
