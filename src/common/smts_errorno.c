/*
 * smts_errorno.c
 *
 *  Created on: 下午3:00:00
 *      Author: ss
 */

#include "smts_errorno.h"

/**
 * for mem watch.
 */
#include "mem_guard.h"
const char* smts_strerror(int err)
{
	if (err < 0) {
		return uv_strerror(err);
	} else {
		switch (err) {
		SMTS_ERRNO_MAP(SMTS_STRERROR_GEN);
		}
	}
	return "unknow err";
}
