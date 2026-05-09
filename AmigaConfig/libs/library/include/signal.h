/*
 * $Id: signal.h,v 1.8 2006-01-08 12:06:14 obarthel Exp $
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

#ifndef _SIGNAL_H
#define _SIGNAL_H

/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************/

#define SIG_IGN ((void (*)(int))0)
#define SIG_DFL ((void (*)(int))1)
#define SIG_ERR ((void (*)(int))-1)

/****************************************************************************/

#define SIGABRT	1
#define SIGFPE	2
#define SIGILL	3
#define SIGINT	4
#define SIGSEGV	5
#define SIGTERM	6

/****************************************************************************/

extern void (*signal(int sig, void (*)(int)))(int);
extern int raise(int sig);

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard, but it should
   be part of ISO/IEC 9899:1999, also known as "C99". */

/****************************************************************************/

typedef int sig_atomic_t;

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

#ifndef _SYS_TYPES_H
#include <sys/types.h>
#endif /* _SYS_TYPES_H */

/****************************************************************************/

typedef void (*sig_t)(int);

/****************************************************************************/

typedef int sigset_t;

/****************************************************************************/

#define SIG_BLOCK	0
#define SIG_UNBLOCK	1
#define SIG_SETMASK	2

/****************************************************************************/

extern int sigmask(int signum);
extern int sigblock(int signal_mask);
extern int sigsetmask(int signal_mask);
extern int sigprocmask(int how, const sigset_t *set, sigset_t *oset);
extern int sigemptyset(sigset_t * set);
extern int sigaddset(sigset_t * set,int sig);
extern int kill(pid_t pid, int signal_number);

/****************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

/****************************************************************************/

#endif /* _SIGNAL_H */
