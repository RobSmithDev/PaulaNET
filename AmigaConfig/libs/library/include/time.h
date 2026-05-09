/*
 * $Id: time.h,v 1.7 2006-01-08 12:06:14 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2015 by Olaf Barthel <obarthel (at) gmx.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   - Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   - Neither the name of Olaf Barthel nor the names of contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************
 *
 * Documentation and source code for this library, and the most recent library
 * build are available from <http://sourceforge.net/projects/clib2>.
 *
 *****************************************************************************
 */

#ifndef _TIME_H
#define _TIME_H

/****************************************************************************/

#ifndef _STDDEF_H
#include <stddef.h>
#endif /* _STDDEF_H */

/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************/

/*
 * Divide the number returned by clock() by CLOCKS_PER_SEC to obtain
 * the elapsed time in seconds
 */
#define CLOCKS_PER_SEC 50

/****************************************************************************/

typedef unsigned long clock_t;
typedef unsigned long time_t;

/****************************************************************************/

struct tm
{
	int tm_sec;		/* Number of seconds past the minute (0..59) */
	int tm_min;		/* Number of minutes past the hour (0..59) */
	int tm_hour;	/* Number of hours past the day (0..23) */
	int tm_mday;	/* Day of the month (1..31) */
	int tm_mon;		/* Month number (0..11) */
	int tm_year;	/* Year number minus 1900 */
	int tm_wday;	/* Day of the week (0..6; 0 is Sunday) */
	int tm_yday;	/* Day of the year (0..365) */
	int tm_isdst;	/* Is this date using daylight savings time? */
};

/****************************************************************************/

extern clock_t clock(void);
extern time_t time(time_t * t);
extern char *asctime(const struct tm *tm);
extern char *ctime(const time_t *t);
extern struct tm *gmtime(const time_t *t);
extern struct tm *localtime(const time_t *t);
extern time_t mktime(struct tm *tm);

/****************************************************************************/

extern double difftime(time_t t1,time_t t0);

/****************************************************************************/

extern size_t strftime(char *s, size_t maxsize, const char *format,
	const struct tm *tm);

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

/* Timespec declaration */
struct timespec
{
	time_t tv_secs;
	long tv_nsec;
};

#ifndef tv_sec
//#define tv_sec tv_secs // modified by JOB
#endif /* tv_sec */

/****************************************************************************/

extern char * asctime_r(const struct tm *tm,char * buffer);
extern char * ctime_r(const time_t *tptr,char * buffer);
extern struct tm * gmtime_r(const time_t *t,struct tm * tm_ptr);
extern struct tm * localtime_r(const time_t *t,struct tm * tm_ptr);
extern void tzset(void);

/****************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

/****************************************************************************/

#endif /* _TIME_H */
