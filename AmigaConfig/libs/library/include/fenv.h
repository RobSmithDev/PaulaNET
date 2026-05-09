/*
 * $Id: fenv.h,v 1.4 2006-01-08 12:06:14 obarthel Exp $
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

#ifndef _FENV_H
#define _FENV_H

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard, but it should
   be part of ISO/IEC 9899:1999, also known as "C99". */

/****************************************************************************/

#ifndef _STDDEF_H
#include <stddef.h>
#endif /* _STDDEF_H */

/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************/

typedef int fenv_t;

/****************************************************************************/

#define FE_DEFL_ENV ((fenv_t *)NULL))

/****************************************************************************/

extern int fegetenv(fenv_t * envp);
extern int fesetenv(fenv_t * envp);
extern int feholdexcept(fenv_t * envp);
extern int feupdateenv(const fenv_t * envp);

/****************************************************************************/

#define FE_DIVBYZERO	(1<<0)
#define FE_INEXACT		(1<<1)
#define FE_INVALID		(1<<2)
#define FE_OVERFLOW		(1<<3)
#define FE_UNDERFLOW	(1<<4)
#define FE_ALL_EXCEPT	(FE_DIVBYZERO|FE_INEXACT|FE_INVALID|FE_OVERFLOW|FE_UNDERFLOW)

/****************************************************************************/

typedef int fexcept_t;

/****************************************************************************/

extern int fegetexceptflag(fexcept_t *flagp,int excepts);
extern int fesetexceptflag(const fexcept_t *flagp,int excepts);
extern int fetestexcept(int excepts);
extern int feraiseexcept(int excepts);
extern int feclearexcept(int excepts);

/****************************************************************************/

#define FE_DOWNWARD		0
#define FE_UPWARD		1
#define FE_TONEAREST	2
#define FE_TOWARDZERO	3

/****************************************************************************/

extern int fegetround(void);
extern int fetestround(int rounds);

/****************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

/****************************************************************************/

#endif /* _FENV_H */
