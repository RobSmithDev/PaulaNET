/*
 * $Id: select.h,v 1.9 2006-04-10 15:08:11 obarthel Exp $
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

#ifndef _SYS_SELECT_H
#define _SYS_SELECT_H

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

#ifndef _STRING_H
#include <string.h>
#endif /* _STRING_H */

#ifndef _SYS_TIME_H
#include <sys/time.h>
#endif /* _SYS_TIME_H */

/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************/

/*
 * In case the select() data structures and macros are already
 * defined by somebody else...
 */
#ifndef FD_SET

/****************************************************************************/

/*
 * select() uses bit masks of file descriptors in longs. These macros
 * manipulate such bit fields.
 *
 * FD_SETSIZE may be defined by the user, but the default here should
 * be enough for most uses.
 */
#ifndef	FD_SETSIZE
#define	FD_SETSIZE 256
#endif

typedef	struct fd_set
{
	unsigned long bits[(FD_SETSIZE + 31) / 32];
} fd_set;

#define	FD_SET(n,p)		((void)(((unsigned long)n) < FD_SETSIZE ? (p)->bits[((unsigned long)n) >> 5] |=  (1UL << (((unsigned long)n) & 31)) : 0))
#define	FD_CLR(n,p)		((void)(((unsigned long)n) < FD_SETSIZE ? (p)->bits[((unsigned long)n) >> 5] &= ~(1UL << (((unsigned long)n) & 31)) : 0))
#define	FD_ISSET(n,p)	(((unsigned long)n) < FD_SETSIZE && ((p)->bits[((unsigned long)n) >> 5] & (1UL << (((unsigned long)n) & 31))) != 0)
#define	FD_COPY(f,t)	((void)memmove(t,f,sizeof(*(f))))
#define	FD_ZERO(p)		((void)memset(p,0,sizeof(*(p))))

/****************************************************************************/

#endif /* FD_SET */

/****************************************************************************/

/*
 * The following prototypes may clash with the bsdsocket.library or
 * usergroup.library API definitions.
 */

#ifndef __NO_NET_API

extern int select(int nfds, fd_set *readfds,fd_set *writefds, fd_set *errorfds,struct timeval *timeout);

/* This is a special select() function which takes an extra Amiga signal
   bit mask pointer parameter. This function works like select(), but it will
   also return if any of the signals indicated by the 'signal_mask' parameter
   are set. When this function returns, the variable pointed to by the
   'signal_mask' parameter will have all the bits set which were set at the
   time the function was called and for which signals arrived while the
   function was still running. When this function returns, any signals
   received while it was running for which bits were set in the 'signal_mask'
   parameter value will be cleared with the exception of SIGBREAKF_CTRL_C.
   In brief, wait_select() works exactly like the bsdsocket.library/WaitSelect()
   function. */

#ifndef __NO_WAITSELECT
extern int waitselect(int num_fds,fd_set *read_fds,fd_set *write_fds,fd_set *except_fds,struct timeval *timeout,unsigned long * signal_mask);
#endif /* __NO_WAITSELECT */

#endif /* __NO_NET_API */

/****************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

/****************************************************************************/

#endif /* _SYS_SELECT_H */
