/*
 * $Id: stdlib_constructor.h,v 1.3 2006-01-08 12:04:25 obarthel Exp $
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

#ifndef _STDLIB_CONSTRUCTOR_H
#define _STDLIB_CONSTRUCTOR_H

/****************************************************************************/

/* Constructor and destructor functions, as used by the library for data
   initialization and cleanup. These particular functions are invoked by
   the startup code before and after the main() function is/was called.
   How this works is very compiler specific. We support three flavours
   below. */

/****************************************************************************/

#ifdef __SASC

#define CONSTRUCTOR(name,pri) \
	int __stdargs _STI_##pri##_##name(void); \
	int __stdargs _STI_##pri##_##name(void)

#define DESTRUCTOR(name,pri) \
	void __stdargs _STD_##pri##_##name(void); \
	void __stdargs _STD_##pri##_##name(void)

#define CONSTRUCTOR_SUCCEED() \
	return(0)

#define CONSTRUCTOR_FAIL() \
	return(1)

#endif /* __SASC */

/****************************************************************************/

#ifdef __GNUC__

#if defined(__amigaos4__)

#define CONSTRUCTOR(name,pri) \
	STATIC VOID __attribute__((used)) name##_ctor(VOID); \
	STATIC VOID (*__##name##_ctor)(VOID) __attribute__((used,section(".ctors._" #pri))) = name##_ctor; \
	STATIC VOID name##_ctor(VOID)

#define DESTRUCTOR(name,pri) \
	STATIC VOID __attribute__((used)) name##_dtor(VOID); \
	STATIC VOID (*__##name##_dtor)(VOID) __attribute__((used,section(".dtors._" #pri))) = name##_dtor; \
	STATIC VOID name##_dtor(VOID)

#else

// modified by JOB
//#define CONSTRUCTOR(name,pri) \
	asm(".stabs \"___INIT_LIST__\",22,0,0,___ctor_" #name); \
	asm(".stabs \"___INIT_LIST__\",20,0,0," #pri); \
	VOID __ctor_##name##(VOID); \
	VOID __ctor_##name##(VOID)

#define CONSTRUCTOR(name,pri) \
	VOID __ctor_##name(VOID); \
	VOID __ctor_##name(VOID)	

// modified by JOB
//#define DESTRUCTOR(name,pri) \
	asm(".stabs \"___EXIT_LIST__\",22,0,0,___dtor_" #name); \
	asm(".stabs \"___EXIT_LIST__\",20,0,0," #pri); \
	VOID __dtor_##name##(VOID); \
	VOID __dtor_##name##(VOID)

#define DESTRUCTOR(name,pri) \
	VOID __dtor_##name(VOID); \
	VOID __dtor_##name(VOID)	

#endif /* __amigaos4__ */

#define CONSTRUCTOR_SUCCEED() \
	return

#define CONSTRUCTOR_FAIL() \
	exit(RETURN_FAIL)

#endif /* __GNUC__ */

/****************************************************************************/

/* These macros are for declaring functions to serve as constructors or
   destructors. In which order these should be invoked is defined by the
   priority, which is a number in the range 0-999. User-supplied
   constructor/destructor functions should have priority 0. That way,
   the user-supplied constructors will be invoked after the library
   constructors and the user-supplied destructors before the library
   destructors. */

#define STDLIB_CONSTRUCTOR(name)	CONSTRUCTOR(name,	9)
#define STDLIB_DESTRUCTOR(name)		DESTRUCTOR(name,	9)

#define STK_CONSTRUCTOR(name)		CONSTRUCTOR(name,	8)
#define STK_DESTRUCTOR(name)		DESTRUCTOR(name,	8)

#define STDIO_CONSTRUCTOR(name)		CONSTRUCTOR(name,	7)
#define STDIO_DESTRUCTOR(name)		DESTRUCTOR(name,	7)

#define FILE_CONSTRUCTOR(name)		CONSTRUCTOR(name,	6)
#define FILE_DESTRUCTOR(name)		DESTRUCTOR(name,	6)

#define MATH_CONSTRUCTOR(name)		CONSTRUCTOR(name,	5)
#define MATH_DESTRUCTOR(name)		DESTRUCTOR(name,	5)

#define SOCKET_CONSTRUCTOR(name)	CONSTRUCTOR(name,	4)
#define SOCKET_DESTRUCTOR(name)		DESTRUCTOR(name,	4)

#define ARG_CONSTRUCTOR(name)		CONSTRUCTOR(name,	3)
#define ARG_DESTRUCTOR(name)		DESTRUCTOR(name,	3)

#define CLIB_CONSTRUCTOR(name)		CONSTRUCTOR(name,	2)
#define CLIB_DESTRUCTOR(name)		DESTRUCTOR(name,	2)

#define PROFILE_CONSTRUCTOR(name)	CONSTRUCTOR(name,	1)
#define PROFILE_DESTRUCTOR(name)	DESTRUCTOR(name,	1)

/****************************************************************************/

#endif /* _STDLIB_CONSTRUCTOR_H */
