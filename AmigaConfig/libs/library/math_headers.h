/*
 * $Id: math_headers.h,v 1.15 2006-11-13 09:51:53 obarthel Exp $
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

#ifndef _MATH_HEADERS_H
#define _MATH_HEADERS_H

/****************************************************************************/

#include <limits.h>
#include <errno.h>
#include <float.h>
#include <math.h>
#include <fenv.h>

/****************************************************************************/

#ifndef _MACROS_H
#include "macros.h"
#endif /* _MACROS_H */

#ifndef _DEBUG_H
#include "debug.h"
#endif /* _DEBUG_H */

/****************************************************************************/

#ifndef _MATH_FP_SUPPORT_H
#include "math_fp_support.h"
#endif /* _MATH_FP_SUPPORT_H */

/****************************************************************************/

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

/****************************************************************************/

#ifndef _STDLIB_PROFILE_H
#include "stdlib_profile.h"
#endif /* _STDLIB_PROFILE_H */

/****************************************************************************/

#if defined(IEEE_FLOATING_POINT_SUPPORT)

/****************************************************************************/

#ifndef PROTO_MATHIEEESINGBAS_H
#include <proto/mathieeesingbas.h>
#endif /* PROTO_MATHIEEESINGBAS_H */

#ifndef PROTO_MATHIEEEDOUBBAS_H
#include <proto/mathieeedoubbas.h>
#endif /* PROTO_MATHIEEEDOUBBAS_H */

#ifndef PROTO_MATHIEEEDOUBTRANS_H
#include <proto/mathieeedoubtrans.h>
#endif /* PROTO_MATHIEEEDOUBTRANS_H */

/****************************************************************************/

extern struct Library * NOCOMMON MathIeeeSingBasBase;
extern struct Library * NOCOMMON MathIeeeDoubBasBase;
extern struct Library * NOCOMMON MathIeeeDoubTransBase;

/****************************************************************************/

#endif /* IEEE_FLOATING_POINT_SUPPORT */

/****************************************************************************/

/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 */

#define FLT_UWORD_IS_ZERO(x) ((x)==0)
#define FLT_UWORD_IS_SUBNORMAL(x) ((x)<0x00800000L)
#define FLT_UWORD_MIN 0x00000001
#define FLT_UWORD_EXP_MIN 0x43160000
#define FLT_UWORD_LOG_MIN 0x42cff1b5
#define FLT_SMALLEST_EXP -22

typedef union
{
	double value;
	struct 
	{
		unsigned int msw;
		unsigned int lsw;
	} parts;
} ieee_double_shape_type;

#define EXTRACT_WORDS(ix0,ix1,d)					\
do {												\
  ieee_double_shape_type ew_u;						\
  ew_u.value = (d);									\
  (ix0) = ew_u.parts.msw;							\
  (ix1) = ew_u.parts.lsw;							\
} while (0)

/* Get the more significant 32 bit int from a double.  */

#define GET_HIGH_WORD(i,d)							\
do {												\
  ieee_double_shape_type gh_u;						\
  gh_u.value = (d);									\
  (i) = gh_u.parts.msw;								\
} while (0)

/* Get the less significant 32 bit int from a double.  */

#define GET_LOW_WORD(i,d)							\
do {												\
  ieee_double_shape_type gl_u;						\
  gl_u.value = (d);									\
  (i) = gl_u.parts.lsw;								\
} while (0)

/* Set a double from two 32 bit ints.  */

#define INSERT_WORDS(d,ix0,ix1)						\
do {												\
  ieee_double_shape_type iw_u;						\
  iw_u.parts.msw = (ix0);							\
  iw_u.parts.lsw = (ix1);							\
  (d) = iw_u.value;									\
} while (0)

/* Set the more significant 32 bits of a double from an int.  */

#define SET_HIGH_WORD(d,v)							\
do {												\
  ieee_double_shape_type sh_u;						\
  sh_u.value = (d);									\
  sh_u.parts.msw = (v);								\
  (d) = sh_u.value;									\
} while (0)

/* Set the less significant 32 bits of a double from an int.  */

#define SET_LOW_WORD(d,v)							\
do {												\
  ieee_double_shape_type sl_u;						\
  sl_u.value = (d);									\
  sl_u.parts.lsw = (v);								\
  (d) = sl_u.value;									\
} while (0)

/* A union which permits us to convert between a float and a 32 bit
   int. */

typedef union
{
  float value;
  unsigned int word;
} ieee_float_shape_type;

/* Get a 32 bit int from a float.  */

#define GET_FLOAT_WORD(i,d)		\
do {							\
  ieee_float_shape_type gf_u;	\
  gf_u.value = (d);				\
  (i) = gf_u.word;				\
} while (0)

/* Set a float from a 32 bit int.  */

#define SET_FLOAT_WORD(d,i)		\
do {							\
  ieee_float_shape_type sf_u;	\
  sf_u.word = (i);				\
  (d) = sf_u.value;				\
} while (0)

extern double __kernel_cos(double x, double y);
extern double __kernel_sin(double x, double y, int iy);
extern int __rem_pio2(double x, double *y);
extern double __kernel_tan(double x, double y, int iy);
extern double __expm1(double x);
extern float __kernel_cosf(float x, float y);
extern float __kernel_sinf(float x, float y, int iy);
extern LONG __rem_pio2f(float x, float *y);
extern float __kernel_tanf(float x, float y, int iy);
extern double __lgamma(double x,int * gamma_sign_ptr);
extern float __lgammaf(float x,int * gamma_sign_ptr);

/****************************************************************************/

/* math_huge_val.c */
extern double __get_huge_val(void);

/****************************************************************************/

/* math_huge_valf.c */
extern float __get_huge_valf(void);

/****************************************************************************/

#endif /* _MATH_HEADERS_H */
