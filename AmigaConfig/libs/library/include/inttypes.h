/*
 * $Id: inttypes.h,v 1.6 2006-01-08 12:06:14 obarthel Exp $
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

#ifndef	_INTTYPES_H
#define	_INTTYPES_H

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard, but it should
   be part of ISO/IEC 9899:1999, also known as "C99". */

/****************************************************************************/

/* Integral types with specified size; contributed by Peter Bengtsson */

/****************************************************************************/

#ifndef	_STDINT_H
#include <stdint.h>
#endif /* _STDINT_H */

/****************************************************************************/

#ifndef _STDDEF_H
#include <stddef.h>
#endif /* _STDDEF_H */

/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************/

/* printf() format specifications for different types. */

/* "Decimal" */
#define	PRId8			"d"
#define	PRId16			"d"
#define	PRId32			"d"
#define	PRId64			"lld"

#define	PRIdLEAST8		"d"
#define	PRIdLEAST16		"d"
#define	PRIdLEAST32		"d"
#define	PRIdLEAST64		"lld"

#define	PRIdFAST8		"d"
#define	PRIdFAST16		"d"
#define	PRIdFAST32		"d"
#define	PRIdFAST64		"lld"

#define	PRIdPTR			"ld"

/* "Integer" */
#define	PRIi8			"i"
#define	PRIi16			"i"
#define	PRIi32			"i"
#define	PRIi64			"lli"

#define	PRIiLEAST8		"i"
#define	PRIiLEAST16		"i"
#define	PRIiLEAST32		"i"
#define	PRIiLEAST64		"lli"

#define	PRIiFAST8		"i"
#define	PRIiFAST16		"i"
#define	PRIiFAST32		"i"
#define	PRIiFAST64		"lli"

#define	PRIiPTR			"li"

/* "Unsigned" */
#define	PRIu8			"u"
#define	PRIu16			"u"
#define	PRIu32			"u"
#define	PRIu64			"llu"

#define	PRIuLEAST8		"u"
#define	PRIuLEAST16		"u"
#define	PRIuLEAST32		"u"
#define	PRIuLEAST64		"llu"

#define	PRIuFAST8		"u"
#define	PRIuFAST16		"u"
#define	PRIuFAST32		"u"
#define	PRIuFAST64		"llu"

#define	PRIuPTR			"lu"

/* "Hexadecimal - lowercase " */
#define	PRIx8			"x"
#define	PRIx16			"x"
#define	PRIx32			"x"
#define	PRIx64			"llx"

#define	PRIxLEAST8		"x"
#define	PRIxLEAST16		"x"
#define	PRIxLEAST32		"x"
#define	PRIxLEAST64		"llx"

#define	PRIxFAST8		"x"
#define	PRIxFAST16		"x"
#define	PRIxFAST32		"x"
#define	PRIxFAST64		"llx"

#define	PRIxPTR			"lx"

/* "Hexadecimal - Uppercase" */
#define	PRIX8			"X"
#define	PRIX16			"X"
#define	PRIX32			"X"
#define	PRIX64			"llX"

#define	PRIXLEAST8		"X"
#define	PRIXLEAST16		"X"
#define	PRIXLEAST32		"X"
#define	PRIXLEAST64		"llX"

#define	PRIXFAST8		"X"
#define	PRIXFAST16		"X"
#define	PRIXFAST32		"X"
#define	PRIXFAST64		"llX"

#define	PRIXPTR			"lX"

/* "Octal" */
#define	PRIo8			"o"
#define	PRIo16			"o"
#define	PRIo32			"o"
#define	PRIo64			"llo"

#define	PRIoLEAST8		"o"
#define	PRIoLEAST16		"o"
#define	PRIoLEAST32		"o"
#define	PRIoLEAST64		"llo"

#define	PRIoFAST8		"o"
#define	PRIoFAST16		"o"
#define	PRIoFAST32		"o"
#define	PRIoFAST64		"llo"

#define	PRIoPTR			"lo"

/* intmax_t is 32 bits for SAS/C, 64-bits for GCC or if using a conforming C99 compiler. */

#if defined(__GNUC__) || (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L))
#define	PRIdMAX			"lld"
#define	PRIiMAX			"lli"
#define	PRIuMAX			"llu"
#define	PRIxMAX			"llx"
#define	PRIXMAX			"llX"
#define	PRIoMAX			"llo"
#else
#define	PRIdMAX			"ld"
#define	PRIiMAX			"li"
#define	PRIuMAX			"lu"
#define	PRIxMAX			"lx"
#define	PRIXMAX			"lX"
#define	PRIoMAX			"lo"
#endif /* __GNUC__ || (__STDC_VERSION__ && __STDC_VERSION__ >= 199901L) */

/* scanf() format specifiers. */

/* "Decimal" */
#define	SCNd8			"hhd"
#define	SCNd16			"hd"
#define	SCNd32			"d"
#define	SCNd64			"lld"

#define	SCNdLEAST8		"hhd"
#define	SCNdLEAST16		"hd"
#define	SCNdLEAST32		"d"
#define	SCNdLEAST64		"lld"

#define	SCNdFAST8		"hhd"
#define	SCNdFAST16		"d"
#define	SCNdFAST32		"d"
#define	SCNdFAST64		"lld"

#define	SCNdPTR			"d"

/* "Integer" */
#define	SCNi8			"hhi"
#define	SCNi16			"hi"
#define	SCNi32			"i"
#define	SCNi64			"lli"

#define	SCNiLEAST8		"hhi"
#define	SCNiLEAST16		"hi"
#define	SCNiLEAST32		"i"
#define	SCNiLEAST64		"lli"

#define	SCNiFAST8		"hhi"
#define	SCNiFAST16		"i"
#define	SCNiFAST32		"i"
#define	SCNiFAST64		"lli"

#define	SCNiPTR			"i"

/* "Unsigned" */
#define	SCNu8			"hhu"
#define	SCNu16			"hu"
#define	SCNu32			"u"
#define	SCNu64			"llu"

#define	SCNuLEAST8		"hhu"
#define	SCNuLEAST16		"hu"
#define	SCNuLEAST32		"u"
#define	SCNuLEAST64		"llu"

#define	SCNuFAST8		"hhu"
#define	SCNuFAST16		"u"
#define	SCNuFAST32		"u"
#define	SCNuFAST64		"llu"

#define	SCNuPTR			"u"

/* "Hexadecimal" */
#define	SCNx8			"hhx"
#define	SCNx16			"hx"
#define	SCNx32			"x"
#define	SCNx64			"llx"

#define	SCNxLEAST8		"hhx"
#define	SCNxLEAST16		"hx"
#define	SCNxLEAST32		"x"
#define	SCNxLEAST64		"llx"

#define	SCNxFAST8		"hhx"
#define	SCNxFAST16		"x"
#define	SCNxFAST32		"x"
#define	SCNxFAST64		"llx"

#define	SCNxPTR			"x"

/* "Octal" */
#define	SCNo8			"hho"
#define	SCNo16			"ho"
#define	SCNo32			"o"
#define	SCNo64			"llo"

#define	SCNoLEAST8		"hho"
#define	SCNoLEAST16		"ho"
#define	SCNoLEAST32		"o"
#define	SCNoLEAST64		"llo"

#define	SCNoFAST8		"hho"
#define	SCNoFAST16		"o"
#define	SCNoFAST32		"o"
#define	SCNoFAST64		"llo"

#define	SCNoPTR			"o"

#if defined(__GNUC__) || (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L))
#define	SCNdMAX			"lld"
#define	SCNiMAX			"lli"
#define	SCNuMAX			"llu"
#define	SCNxMAX			"llx"
#define	SCNoMAX			"llo"
#else
#define	SCNdMAX			"ld"
#define	SCNiMAX			"li"
#define	SCNuMAX			"lu"
#define	SCNxMAX			"lx"
#define	SCNoMAX			"lo"
#endif /* __GNUC__ || (__STDC_VERSION__ && __STDC_VERSION__ >= 199901L) */

/****************************************************************************/

#if defined(__GNUC__) || (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L))
typedef struct { long long quot; long long rem; } imaxdiv_t;
#else
typedef struct { long quot; long rem; } imaxdiv_t;
#endif /* __GNUC__ || (__STDC_VERSION__ && __STDC_VERSION__ >= 199901L) */

/****************************************************************************/

extern intmax_t imaxabs(intmax_t x);
extern imaxdiv_t imaxdiv(intmax_t n,intmax_t d);

/****************************************************************************/

extern intmax_t strtoimax(const char *str, char **ptr, int base);
extern uintmax_t strtoumax(const char *str, char **ptr, int base);

/****************************************************************************/

extern intmax_t wcstoimax(const wchar_t *str, char **ptr, int base);
extern uintmax_t wcstoumax(const wchar_t *str, char **ptr, int base);

/****************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

/****************************************************************************/

#endif /* _INTTYPES_H */
