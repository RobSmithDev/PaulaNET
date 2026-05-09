/*
 * $Id: stdarg.h,v 1.7 2006-01-08 12:06:14 obarthel Exp $
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

#ifndef _STDARG_H
#define _STDARG_H

/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************/

// modified by JOB
//#ifndef __amigaos4__
#if !defined(__amigaos4__) && !defined(__GNUC__)

/****************************************************************************/

typedef char * va_list;

/****************************************************************************/

#define va_start(ap, last)	((void)(ap = (va_list)&(last) + sizeof(last)))
#define va_arg(ap, type)	((type *)(ap += sizeof(type)))[-1]
#define va_end(ap)			((void)0)

/****************************************************************************/

/* The following macro is not part of the ISO 'C' (1994) standard, but it should
   be part of ISO/IEC 9899:1999, also known as "C99". */

/****************************************************************************/

#define va_copy(dst,src) ((void)((dst) = (src)))

/****************************************************************************/

#else

/****************************************************************************/

#if defined(__GNUC__)

/* Use the compiler supplied, machine specific <stdarg.h> file. */
#undef _STDARG_H
#include_next "stdarg.h"

#include <sys/amigaos-va.h>

#else

#error "Unknown compiler"

#endif /* __GNUC__ */

/****************************************************************************/

#endif /* __amigaos4__ */

/****************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

/****************************************************************************/

#endif /* _STDARG_H */
