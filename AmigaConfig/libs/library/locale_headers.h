/*
 * $Id: locale_headers.h,v 1.8 2006-01-08 12:04:23 obarthel Exp $
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

#ifndef _LOCALE_HEADERS_H
#define _LOCALE_HEADERS_H

/****************************************************************************/

#ifndef EXEC_LIBRARIES_H
#include <exec/libraries.h>
#endif /* EXEC_LIBRARIES_H */

#ifndef LIBRARIES_LOCALE_H
#include <libraries/locale.h>
#endif /* LIBRARIES_LOCALE_H */

/****************************************************************************/

#ifndef PROTO_EXEC_H
#include <proto/exec.h>
#endif /* PROTO_EXEC_H */

/****************************************************************************/

#include <locale.h>
#include <limits.h>
#include <string.h>
#include <errno.h>

/****************************************************************************/

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

/****************************************************************************/

#ifndef _STDLIB_PROFILE_H
#include "stdlib_profile.h"
#endif /* _STDLIB_PROFILE_H */

/****************************************************************************/

#ifndef _MACROS_H
#include "macros.h"
#endif /* _MACROS_H */

#ifndef _DEBUG_H
#include "debug.h"
#endif /* _DEBUG_H */

/****************************************************************************/

#define NUM_LOCALES			(LC_TIME - LC_ALL + 1)
#define MAX_LOCALE_NAME_LEN	256

/****************************************************************************/

extern struct Locale * NOCOMMON __default_locale;
extern struct Locale * NOCOMMON __locale_table[NUM_LOCALES];

/****************************************************************************/

extern char NOCOMMON __locale_name_table[NUM_LOCALES][MAX_LOCALE_NAME_LEN];

/****************************************************************************/

#if defined(__THREAD_SAFE)

/****************************************************************************/

extern void __locale_lock(void);
extern void __locale_unlock(void);

/****************************************************************************/

#else

/****************************************************************************/

#define __locale_lock()		((void)0)
#define __locale_unlock()	((void)0)

/****************************************************************************/

#endif /* __THREAD_SAFE */

/****************************************************************************/

extern void __close_all_locales(void);

/****************************************************************************/

#endif /* _LOCALE_HEADERS_H */
