/*
 * $Id: debug_headers.h,v 1.4 2006-01-08 12:04:22 obarthel Exp $
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

#ifndef _DEBUG_HEADERS_H
#define _DEBUG_HEADERS_H

/****************************************************************************/

#ifndef EXEC_LIBRARIES_H
#include <exec/libraries.h>
#endif /* EXEC_LIBRARIES_H */

/****************************************************************************/

#if defined(__SASC)

#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>

LONG RawMayGetChar(VOID);
VOID RawPutChar(UBYTE c);

#pragma libcall SysBase RawMayGetChar 1fe 00
#pragma libcall SysBase RawPutChar 204 001

#endif /* __SASC */

/****************************************************************************/

#if defined(__amigaos4__)

#ifndef EXEC_EMULATION_H
#include <exec/emulation.h>
#endif /* EXEC_EMULATION_H */

#endif /* __amigaos4__ */

/****************************************************************************/

#if defined(__GNUC__)

#define __NOLIBBASE__
#include <proto/exec.h>

#ifndef __PPC__

// modified by JOB: jsr a6@(-510:W) -> jsr %%a6@(-510:W)
#define RawMayGetChar() ({ \
  UBYTE _RawMayGetChar__re = \
  ({ \
  register struct Library * const __RawMayGetChar__bn __asm("a6") = SysBase;\
  register UBYTE __RawMayGetChar__re __asm("d0"); \
  __asm volatile ("jsr %%a6@(-510:W)" \
  : "=r"(__RawMayGetChar__re) \
  : "r"(__RawMayGetChar__bn) \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __RawMayGetChar__re; \
  }); \
  _RawMayGetChar__re; \
})

// modified by JOB: jsr a6@(-516:W) -> jsr %%a6@(-516:W)
#define RawPutChar(c) ({ \
  ULONG _RawPutChar_c = (c); \
  { \
  register struct Library * const __RawPutChar__bn __asm("a6") = SysBase;\
  register ULONG __RawPutChar_c __asm("d0") = (_RawPutChar_c); \
  __asm volatile ("jsr %%a6@(-516:W)" \
  : \
  : "r"(__RawPutChar__bn), "r"(__RawPutChar_c) \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#endif /* __PPC___ */

#endif /* __GNUC__ */

/****************************************************************************/

#define SysBase (*(struct Library **)4)
#define IExec ((struct ExecIFace *)((struct ExecBase *)SysBase)->MainInterface)

/****************************************************************************/

#ifndef _MACROS_H
#include "macros.h"
#endif /* _MACROS_H */

#ifndef _DEBUG_H
#include "debug.h"
#endif /* _DEBUG_H */

/****************************************************************************/

#include <string.h>
#include <stdarg.h>

/****************************************************************************/

extern LONG cmpstrexec(const char *source1, const char *source2);
extern LONG KCmpStr(const char *source1, const char *source2);
extern LONG KMayGetCh(VOID);
extern LONG KMayGetChar(VOID);
extern LONG kgetc(VOID);
extern LONG kgetchar(VOID);
extern LONG kgetch(VOID);
extern LONG KGetCh(VOID);
extern LONG KGetChar(VOID);
extern LONG kgetnum(VOID);
extern LONG KGetNum(void);
extern VOID KDoFmt(const char *format_string, APTR data_stream, APTR put_char_routine, APTR put_char_data);
extern VOID KPutFmt(const char *format, va_list arg);
extern VOID KVPrintF(const char * format, va_list arg);
extern VOID kvprintf(const char *format, va_list arg);
extern VOID kprintf(const char *format, ...);
extern VOID KPrintF(const char *format, ...);
extern VOID kputc(UBYTE c);
extern VOID kputchar(UBYTE c);
extern VOID kputch(UBYTE c);
extern VOID KPutCh(UBYTE c);
extern VOID KPutChar(UBYTE c);
extern VOID kputstr(const UBYTE *s);
extern VOID KPutS(const UBYTE *s);
extern VOID kputs(const UBYTE *s);
extern VOID KPutStr(const UBYTE *s);

/****************************************************************************/

#endif /* _DEBUG_HEADERS_H */
