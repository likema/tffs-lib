/*
 * crtdef.h
 *
 * C Run-Time Libraries functions definitions
 * head file
 *
 * knightray@gmail.com
 * 10/28 2008
 */
#ifndef _CRT_DEF_H
#define _CRT_DEF_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define Memset(ptr, b, size) memset(ptr, b, size)

typedef struct _tffs_sys_time {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
} tffs_sys_time_t;

void Getcurtime(tffs_sys_time_t * ptime);

#endif
