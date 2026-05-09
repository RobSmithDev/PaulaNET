/*
 * $Id: wchar.h,v 1.8 2008-04-30 14:34:03 obarthel Exp $
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

#ifndef _WCHAR_H
#define _WCHAR_H

/****************************************************************************/

#ifndef _SYS_CLIB2_STDC_H
#include <sys/clib2_stdc.h>
#endif /* _SYS_CLIB2_STDC_H */

#ifndef _STDDEF_H
#include <stddef.h>
#endif /* _STDDEF_H */

#ifndef _STDIO_H
#include <stdio.h>
#endif /* _STDIO_H */

#ifndef _STDARG_H
#include <stdarg.h>
#endif /* _STDARG_H */

#ifndef _TIME_H
#include <time.h>
#endif /* _TIME_H */

#ifndef _LOCALE_H
#include <locale.h>
#endif /* _LOCALE_H */

#ifndef _STDINT_H
#include <stdint.h>
#endif

/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************/

#define WEOF (-1)

/****************************************************************************/

typedef long int wint_t;
typedef long mbstate_t;

/****************************************************************************/

extern wint_t btowc(int c);
extern int wctob(wint_t c);
extern int mbsinit(const mbstate_t *ps);
extern size_t wcrtomb(char *s, wchar_t wc, mbstate_t *ps);
extern size_t mbsrtowcs(wchar_t *pwcs, const char **src, size_t n, mbstate_t *ps);
extern size_t wcsrtombs(char *s, const wchar_t **src, size_t n, mbstate_t *ps);

/****************************************************************************/

extern wchar_t *wcscat(wchar_t *dest, const wchar_t *src);
extern wchar_t *wcsncat(wchar_t *dest, const wchar_t *src, size_t n);
extern int wcscmp(const wchar_t *s1, const wchar_t * s2);
extern int wcsncmp(const wchar_t *s1, const wchar_t *s2, size_t n);
extern wchar_t * wcscpy(wchar_t *dest, const wchar_t *src);
extern wchar_t * wcsncpy(wchar_t *dest, const wchar_t *src, size_t n);
extern size_t wcslen(const wchar_t *s);
extern wchar_t * wcschr(const wchar_t *s, wchar_t c);
extern size_t wcsspn(const wchar_t *s, const wchar_t *set);
extern wchar_t *wcspbrk(const wchar_t *s, const wchar_t *set);
extern wchar_t *wcstok(wchar_t *str, const wchar_t *set);
extern wchar_t *wcsstr(const wchar_t *src, const wchar_t *sub);

/****************************************************************************/

extern double wcstod(const wchar_t *str, wchar_t **ptr);
extern long wcstol(const wchar_t *str, wchar_t **ptr, int base);
extern unsigned long wcstoul(const wchar_t *str, wchar_t **ptr, int base);

/****************************************************************************/

extern int wscoll(const wchar_t *s1, const wchar_t *s2);
extern size_t wcsxfrm(wchar_t *dest, const wchar_t *src, size_t len);

/****************************************************************************/

extern wchar_t *wmemchr(const wchar_t *ptr, wchar_t val, size_t len);
extern int wmemcmp(const wchar_t *ptr1, const wchar_t *ptr2, size_t len);
extern wchar_t *wmemcpy(wchar_t *dest, const wchar_t *src, size_t len);
extern wchar_t *wmemmove(wchar_t *dest, const wchar_t * src, size_t len);
extern wchar_t *wmemset(wchar_t *ptr, int val, size_t len);

/****************************************************************************/

extern int fwide(FILE *stream, int orient);

/****************************************************************************/

extern wint_t fgetwc(FILE *stream);
extern wint_t getwc(FILE *stream);
extern wint_t getwchar(void);
extern wint_t ungetwc(wint_t c,FILE *stream);

/****************************************************************************/

extern wchar_t *fgetws(wchar_t *s, int n, FILE *stream);

/****************************************************************************/

extern int fwscanf(FILE *stream, const wchar_t *format, ...);
extern int wscanf(const wchar_t *format, ...);
extern int swscanf(wchar_t *s,const wchar_t *format, ...);

/****************************************************************************/

extern wint_t fputwc(wchar_t c,FILE *stream);
extern wint_t putwc(wchar_t c,FILE *stream);
extern wint_t putwchar(wchar_t c);

/****************************************************************************/

extern wint_t fputws(const wchar_t *s, FILE *stream);

/****************************************************************************/

extern int fwprintf(FILE *stream,const wchar_t *format,...);
extern int wprintf(const wchar_t *format, ...);
extern int swprintf(wchar_t *s, const wchar_t *format, ...);

/****************************************************************************/

extern int vfwprintf(FILE *stream,const wchar_t *format,va_list arg);
extern int vwprintf(const wchar_t *format,va_list arg);
extern int vswprintf(wchar_t *s, size_t maxlen, const wchar_t *format, va_list arg);

/****************************************************************************/

extern size_t wcsftime(wchar_t *s, size_t maxsize, const wchar_t *format, const struct tm *timeptr);

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard, but it should
   be part of ISO/IEC 9899:1999, also known as "C99". */

/****************************************************************************/

#if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)) || defined(__cplusplus)

extern size_t mbrlen(const char *restrict s, size_t n, mbstate_t *restrict ps);
extern size_t mbrtowc(wchar_t *restrict pwc, const char *restrict s, size_t n, mbstate_t *restrict ps);
extern int mbsinit(const mbstate_t *ps);
extern size_t mbsnrtowcs(wchar_t *restrict dst, const char **restrict src, size_t nmc, size_t len, mbstate_t *restrict ps);
extern size_t mbsrtowcs(wchar_t *restrict dst, const char **restrict src, size_t len, mbstate_t *restrict ps);

extern size_t wcrtomb(char *restrict s, wchar_t wc, mbstate_t *restrict ps);
extern int wcscoll(const wchar_t *ws1, const wchar_t *ws2);
extern int wcscoll_l(const wchar_t *ws1, const wchar_t *ws2, locale_t loc);
extern size_t wcscspn(const wchar_t *s, const wchar_t *c);
extern size_t wcsnrtombs(char *restrict dst, const wchar_t **restrict src, size_t nwc, size_t len, mbstate_t *restrict ps);
extern wchar_t * wcsrchr(const wchar_t *ws, wchar_t wc);
extern size_t wcsrtombs(char *restrict dst, const wchar_t **restrict src, size_t len, mbstate_t *restrict ps);
extern long long wcstoll(const wchar_t *str, wchar_t **ptr, int base);
extern unsigned long long wcstoull(const wchar_t *str, wchar_t **ptr, int base);

#endif /* __STDC_VERSION__ && __STDC_VERSION__ >= 199901L */

/****************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

/****************************************************************************/

#endif /* _WCHAR_H */
