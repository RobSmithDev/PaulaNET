/*
 * $Id: unistd_headers.h,v 1.12 2006-09-27 09:40:06 obarthel Exp $
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
 */

#ifndef _UNISTD_HEADERS_H
#define _UNISTD_HEADERS_H

/****************************************************************************/

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

/****************************************************************************/

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

/****************************************************************************/

#ifndef _STDLIB_PROFILE_H
#include "stdlib_profile.h"
#endif /* _STDLIB_PROFILE_H */

/****************************************************************************/

extern char NOCOMMON __current_path_name[MAXPATHLEN];

/****************************************************************************/

/* Names of files and directories to delete when shutting down. */
extern struct MinList NOCOMMON			__unlink_list;
extern struct SignalSemaphore NOCOMMON	__unlink_semaphore;

/****************************************************************************/

/* Local timer I/O. */
extern struct MsgPort *	NOCOMMON __timer_port;
extern BOOL				NOCOMMON __timer_busy;
extern struct Library *	NOCOMMON __TimerBase;

/****************************************************************************/

/* A quick workaround for the timeval/timerequest->TimeVal/TimeRequest
   change in the recent OS4 header files. */

#if defined(__NEW_TIMEVAL_DEFINITION_USED__)

extern struct TimeRequest *	NOCOMMON __timer_request;

#else

extern struct timerequest *	NOCOMMON __timer_request;

#endif /* __NEW_TIMEVAL_DEFINITION_USED__ */

/****************************************************************************/

#if defined(__amigaos4__)
extern struct TimerIFace * NOCOMMON __ITimer;
#endif /* __amigaos4__ */

/****************************************************************************/

/* If the program's current directory was changed, here is where
   we find out about it. */
extern BPTR NOCOMMON __original_current_directory;
extern BOOL NOCOMMON __current_directory_changed;
extern BOOL NOCOMMON __unlock_current_directory;

/****************************************************************************/

extern BOOL NOCOMMON __unix_path_semantics;

/****************************************************************************/

extern int __set_current_path(const char * path_name);
extern int __strip_double_slash(char * file_name,int len);

/****************************************************************************/

extern unsigned int __time_delay(unsigned long seconds,unsigned long microseconds);

/****************************************************************************/

extern long __pathconf(struct MsgPort *port,int name);

/****************************************************************************/

#endif /* _UNISTD_HEADERS_H */
