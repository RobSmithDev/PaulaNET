/*
 * $Id: macros.h,v 1.22 2006-01-08 12:04:23 obarthel Exp $
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

#ifndef _MACROS_H
#define _MACROS_H

/****************************************************************************/

#ifndef DOS_DOS_H
#include <dos/dos.h>
#endif /* DOS_DOS_H */

/****************************************************************************/

#define BUSY ((struct IORequest *)NULL)
#define CANNOT !
#define DO_NOTHING ((void)0)
#define NO !
#define NOT !
#define DO_NOT !
#define OK (0)
#define SAME (0)
#define SEEK_ERROR (-1)
#define ERROR (-1)

/****************************************************************************/

#define NUM_ENTRIES(t) (sizeof(t) / sizeof(t[0]))

/****************************************************************************/

#define FLAG_IS_SET(v,f)	(((v) & (f)) == (f))
#define FLAG_IS_CLEAR(v,f)	(((v) & (f)) ==  0 )

/****************************************************************************/

#define SET_FLAG(v,f)		((void)((v) |=  (f)))
#define CLEAR_FLAG(v,f)		((void)((v) &= ~(f)))

/****************************************************************************/

/* Macro to get longword-aligned stack space for a structure
   Uses ANSI token catenation to form a name for the char array
   based on the variable name, then creates an appropriately
   typed pointer to point to the first longword boundary in the
   char array allocated. */
#define D_S(type, name) \
	char a_##name[sizeof(type)+3]; \
	type *name = (type *)((ULONG)(a_##name+3) & ~3)

/****************************************************************************/

/* BCPL style "NULL"; this should be in <dos/dos.h>. */
#ifndef ZERO
#define ZERO ((BPTR)NULL)
#endif /* ZERO */

/****************************************************************************/

/* Special data and function attributes; for OS4 most, if not all of them
   are in a file called <amiga_compiler.h> which is pulled in by the
   <exec/types.h> header file. */

/****************************************************************************/

#ifndef IsMinListEmpty
#define IsMinListEmpty(ml) \
    ((struct MinList *)((ml)->mlh_TailPred) == (struct MinList *)(ml))
#endif

/****************************************************************************/

#ifndef AMIGA_COMPILER_H

#ifdef __SASC

#ifndef ASM
#define ASM __asm
#endif /* ASM */

#ifndef REG
#define REG(r,p) register __##r p
#endif /* REG */

#ifndef INTERRUPT
#define INTERRUPT __interrupt
#endif /* INTERRUPT */

#ifndef INLINE
#define INLINE __inline
#endif /* INLINE */
#endif /* __SASC */

#ifdef __GNUC__

#ifndef ASM
#define ASM
#endif /* ASM */

#ifndef REG
#define REG(r,p) p __asm(#r)
#endif /* REG */

#ifndef INTERRUPT
#define INTERRUPT __attribute__((__interrupt__))
#endif /* INTERRUPT */

#ifndef INLINE
#define INLINE __inline__
#endif /* INLINE */

#endif /* __GNUC__ */

#endif /* AMIGA_COMPILER_H */

/****************************************************************************/

#ifndef WEAK
#ifdef __GNUC__
#define WEAK __attribute__((weak))
#else
#define WEAK /* WEAK */
#endif /* __GNUC__ */
#endif /* WEAK */

/****************************************************************************/

#ifndef UNUSED
#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED /* UNUSED */
#endif /* __GNUC__ */
#endif /* UNUSED */

/****************************************************************************/

#ifndef NOCOMMON
#ifdef __GNUC__
#define NOCOMMON __attribute__((nocommon))
#else
#define NOCOMMON /* NOCOMMON */
#endif /* __GNUC__ */
#endif /* NOCOMMON */

/****************************************************************************/

#endif /* _MACROS_H */
