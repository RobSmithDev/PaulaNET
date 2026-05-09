/*
 * $Id: float.h,v 1.5 2006-01-08 12:06:14 obarthel Exp $
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

#ifndef _FLOAT_H
#define _FLOAT_H

/****************************************************************************/

#define FLT_EPSILON		1.19209290E-07F
#define FLT_DIG			6
#define FLT_MANT_DIG	24
#define FLT_MAX			3.40282347E+38F
#define FLT_MAX_EXP		128
#define FLT_MAX_10_EXP	38
#define FLT_MIN			1.17549435E-38F
#define FLT_MIN_EXP		(-125)
#define FLT_MIN_10_EXP	(-37)
#define FLT_RADIX		2
#define FLT_ROUNDS		1

/****************************************************************************/

#define DBL_EPSILON		2.2204460492503131E-16
#define DBL_DIG			15
#define DBL_MANT_DIG	53
#define DBL_MAX			1.7976931348623157E+308
#define DBL_MAX_EXP		1024
#define DBL_MAX_10_EXP	308
#define DBL_MIN			2.2250738585072014E-308
#define DBL_MIN_EXP		(-1021)
#define DBL_MIN_10_EXP	(-307)

/****************************************************************************/

#define LDBL_EPSILON	2.2204460492503131E-16L
#define LDBL_DIG		15
#define LDBL_MANT_DIG	53
#define LDBL_MAX		1.7976931348623157E+308L
#define LDBL_MAX_EXP	1024
#define LDBL_MAX_10_EXP	308
#define LDBL_MIN		2.2250738585072014E-308L
#define LDBL_MIN_EXP	(-1021)
#define LDBL_MIN_10_EXP	(-307)

/****************************************************************************/

#endif /* _FLOAT_H */
