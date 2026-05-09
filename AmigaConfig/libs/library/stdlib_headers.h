/*
 * $Id: stdlib_headers.h,v 1.22 2010-10-20 13:50:17 obarthel Exp $
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

#ifndef _STDLIB_HEADERS_H
#define _STDLIB_HEADERS_H

/****************************************************************************/

#ifndef EXEC_LIBRARIES_H
#include <exec/libraries.h>
#endif /* EXEC_LIBRARIES_H */

/****************************************************************************/

/* This enables the legacy compatible 'struct AnchorPathOld'. */
#ifndef __amigaos4__
#define USE_OLD_ANCHORPATH
#endif /* USE_OLD_ANCHORPATH */

/****************************************************************************/

#if (INCLUDE_VERSION >= 50)
#ifndef DOS_ANCHORPATH_H
#include <dos/anchorpath.h>
#endif /* DOS_ANCHORPATH_H */
#endif /* (INCLUDE_VERSION >= 50) */

/****************************************************************************/

#ifndef DOS_DOSEXTENS_H
#include <dos/dosextens.h>
#endif /* DOS_DOSEXTENS_H */

#ifndef DOS_DOSTAGS_H
#include <dos/dostags.h>
#endif /* DOS_DOSTAGS_H */

#ifndef DOS_DOSASL_H
#include <dos/dosasl.h>
#endif /* DOS_DOSASL_H */

/****************************************************************************/

#if defined(__amigaos4__)
#include <dos/obsolete.h>
#endif /* __amigaos4__ */

/****************************************************************************/

#ifndef WORKBENCH_STARTUP_H
#include <workbench/startup.h>
#endif /* WORKBENCH_STARTUP_H */

/****************************************************************************/

#ifndef CLIB_ALIB_PROTOS_H
#include <clib/alib_protos.h>
#endif /* CLIB_ALIB_PROTOS_H */

/****************************************************************************/

#ifndef PROTO_EXEC_H
#include <proto/exec.h>
#endif /* PROTO_EXEC_H */

#ifndef PROTO_DOS_H
#include <proto/dos.h>
#endif /* PROTO_DOS_H */

/****************************************************************************/

#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <dos.h>

/****************************************************************************/

#ifndef _STDLIB_LOCALEBASE_H
#include "stdlib_localebase.h"
#endif /* _STDLIB_LOCALEBASE_H */

#ifndef _STDLIB_UTILITYBASE_H
#include "stdlib_utilitybase.h"
#endif /* _STDLIB_UTILITYBASE_H */

/****************************************************************************/

#ifndef _MATH_FP_SUPPORT_H
#include "math_fp_support.h"
#endif /* _MATH_FP_SUPPORT_H */

/****************************************************************************/

#ifndef _STDLIB_PROFILE_H
#include "stdlib_profile.h"
#endif /* _STDLIB_PROFILE_H */

/****************************************************************************/

#if defined(FLOATING_POINT_SUPPORT)
#include <math.h>
#endif /* FLOATING_POINT_SUPPORT */

/****************************************************************************/

#ifndef _MACROS_H
#include "macros.h"
#endif /* _MACROS_H */

#ifndef _DEBUG_H
#include "debug.h"
#endif /* _DEBUG_H */

/****************************************************************************/

extern unsigned int NOCOMMON __random_seed;

/****************************************************************************/

extern jmp_buf NOCOMMON	__exit_jmp_buf;
extern int NOCOMMON		__exit_value;
extern BOOL	NOCOMMON	__exit_blocked;

/****************************************************************************/

extern void NOCOMMON (*__alloca_cleanup)(const char * file,int line);

/****************************************************************************/

extern unsigned int NOCOMMON (* __get_default_stack_size)(void);

/****************************************************************************/

extern unsigned int NOCOMMON __stack_size;

/****************************************************************************/

extern BOOL NOCOMMON __is_resident;

/****************************************************************************/

extern UBYTE NOCOMMON __shell_escape_character;

/****************************************************************************/

extern char ** NOCOMMON	__argv;
extern int NOCOMMON		__argc;

/****************************************************************************/

extern BOOL NOCOMMON __lib_startup;

/****************************************************************************/

extern void NOCOMMON (*__alloca_trap)(void);

/****************************************************************************/

#ifndef _STDLIB_PROTOS_H
#include "stdlib_protos.h"
#endif /* _STDLIB_PROTOS_H */

/****************************************************************************/

extern void kprintf(const char * format,...);

/****************************************************************************/

#endif /* _STDLIB_HEADERS_H */
