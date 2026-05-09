/*
 * $Id: limits.h,v 1.12 2010-08-20 15:33:36 obarthel Exp $
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

#ifndef _LIMITS_H
#define _LIMITS_H

/****************************************************************************/

#define CHAR_BIT 8

/****************************************************************************/

#define SCHAR_MIN -128
#define SCHAR_MAX 127
#define UCHAR_MAX 255

/****************************************************************************/

/*
 * The following defines the range a 'char' can cover by checking a
 * preprocessor symbol; we support both SAS/C and GCC here.
 */

#if (defined(__GNUC__) && defined(__CHAR_UNSIGNED__)) || (defined(__SASC) && defined(_UNSCHAR))

#define CHAR_MIN 0
#define CHAR_MAX 255

#else

#define CHAR_MIN -128
#define CHAR_MAX 127

#endif /* (__GNUC__ && __CHAR_UNSIGNED) || (__SASC && _UNSCHAR) */

/****************************************************************************/

#define SHRT_MIN	-32768
#define SHRT_MAX	32767
#define USHRT_MAX	65535

/****************************************************************************/

#define INT_MIN		(-2147483647 - 1)
#define INT_MAX		2147483647
#define UINT_MAX	4294967295U

/****************************************************************************/

#define LONG_MIN	(-2147483647L - 1)
#define LONG_MAX	2147483647L
#define ULONG_MAX	4294967295UL

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard, but it should
   be part of ISO/IEC 9899:1999, also known as "C99". */

/****************************************************************************/

#define LLONG_MIN	(-0x7fffffffffffffffLL-1)
#define LLONG_MAX	0x7fffffffffffffffLL
#define ULLONG_MAX	0xffffffffffffffffULL

/****************************************************************************/

#define MB_LEN_MAX 1

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

#define SSIZE_MAX LONG_MAX

/****************************************************************************/

#define PATH_MAX 1024

/****************************************************************************/

#define WORD_BIT 32
#define LONG_BIT 32	/* For now, at least. */

/****************************************************************************/

/*
 * POSIX, POSIX... These are minimum maximum values. I.e. the maximum must be
 * at least this large. I am not sure all of these make much sense on the Amiga
 * (yet), but here they are anyway.
 *
 * Actually, these symbols should be defined only if the preprocessor symbol
 * _POSIX_SOURCE is defined. Other symbols which should be defined, too, are
 * ARG_MAX, CHILD_MAX, LINK_MAX, MAX_CANON, MAX_INPUT, NAME_MAX, NGROUPS_MAX,
 * OPEN_MAX, PATH_MAX, PIPE_BUF, SSIZE_MAX, STREAM_MAX and TZNAME_MAX.
 */

#define _POSIX_AIO_LISTIO_MAX		2
#define _POSIX_AIO_MAX				1
#define _POSIX_ARG_MAX				4096
#define _POSIX_CHILD_MAX			6
#define _POSIX_DELAYTIMER_MAX		32
#define _POSIX_LINK_MAX				8
#define _POSIX_LOGIN_NAME_MAX		9
#define _POSIX_MAX_CANON			255
#define _POSIX_MAX_INPUT			255
#define _POSIX_MQ_OPEN_MAX			8
#define _POSIX_MQ_PRIO_MAX			32
#define _POSIX_NAME_MAX				14
#define _POSIX_NGROUPS_MAX			0
#define _POSIX_OPEN_MAX				16
#define _POSIX_PATH_MAX				255
#define _POSIX_PIPE_BUF				512
#define _POSIX_RTSIG_MAX			8
#define _POSIX_SEM_NSEMS_MAX		256
#define _POSIX_SEM_VALUE_MAX		32767
#define _POSIX_SIGQUEUE_MAX			32
#define _POSIX_SSIZE_MAX			32767
#define _POSIX_STREAM_MAX			8
#define _POSIX_THREAD_DESTRUCTOR_ITERATIONS	4
#define _POSIX_THREAD_KEYS_MAX		128
#define _POSIX_THREADS_MAX			64
#define _POSIX_TIMER_MAX			32
#define _POSIX_TTY_NAME_MAX			9
#define _POSIX_TZNAME_MAX			3
#define _POSIX2_BC_BASE_MAX			99
#define _POSIX2_BC_DIM_MAX			2048
#define _POSIX2_BC_SCALE_MAX		99
#define _POSIX2_BC_STRING_MAX		1000
#define _POSIX2_COLL_WEIGHTS_MAX	2
#define _POSIX2_EXPR_NEST_MAX		32
#define _POSIX2_LINE_MAX			2048
#define _POSIX2_RE_DUP_MAX			255
#define _XOPEN_IOV_MAX				16

/* And a maximum minimum value. */
#define _POSIX_CLOCKRES_MIN		20000000 /* Nanoseconds */

/****************************************************************************/

#endif /* _LIMITS_H */
