/*
 * $Id: locale.h,v 1.5 2006-01-08 12:06:14 obarthel Exp $
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

#ifndef _LOCALE_H
#define _LOCALE_H

/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************/

#define LC_ALL		0	/* all behaviour */
#define LC_COLLATE	1	/* behaviour of strcoll() and strxfrm() */
#define LC_CTYPE	2	/* character handling */
#define LC_MONETARY	3	/* monetary information returned by localeconv() */
#define LC_NUMERIC	4	/* decimal point and non-monetary information
						   returned by localeconv() */
#define LC_TIME		5	/* behaviour of strftime() */

/****************************************************************************/

typedef void *locale_t;

/****************************************************************************/

struct lconv
{
	char *	decimal_point;		/* Decimal point character (non-monetary). */
	char *	thousands_sep;		/* Non-monetary digit group separator
								   character(s). */
	char *	grouping;			/* Non-monetary digit groupings. */
	char *	int_curr_symbol;	/* International currency symbol, followed
								   by the character that separates it from
								   the monetary quantity. */
	char *	currency_symbol;	/* The local currency symbol for the
								   current locale. */
	char *	mon_decimal_point;	/* Decimal point character for monetary
								   quantities. */
	char *	mon_thousands_sep;	/* Monetary digit group separator
								   character(s). */
	char *	mon_grouping;		/* Monetary digit groupings. */
	char *	positive_sign;		/* Sign character(s) for non-negative
								   monetary quantities. */
	char *	negative_sign;		/* Sign character(s) for negative
								   monetary quantities. */
	char	int_frac_digits;	/* Digits shown right of the decimal
								   point in international monetary
								   format. */
	char	frac_digits;		/* Digits shown to the right of the decimal
								   point in other monetary formats. */
	char	p_cs_precedes;		/* If currency symbol precedes non-negative
								   monetary values this will be 1, otherwise
								   it will be 0. */
	char	p_sep_by_space;		/* If currency symbol is separated from
								   non-negative monetary values by a blank
								   space this will be 1, otherwise it will
								   be 0. */
	char	n_cs_precedes;		/* If currency symbol precedes negative
								   monetary values this will be 1, otherwise
								   it will be 0. */
	char	n_sep_by_space;		/* If currency symbol is separated from
								   negative monetary values by a blank
								   space this will be 1, otherwise it will
								   be 0. */
	char	p_sign_posn;		/* Where to place the positive sign and the
								   currency symbol for a non-negative monetary
								   quantity. */
	char	n_sign_posn;		/* Where to place the negative sign and the
								   currency symbol for a negative monetary
								   quantity. */
};

/****************************************************************************/

extern char *setlocale(int category, const char *locale);
extern struct lconv *localeconv(void);

/****************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

/****************************************************************************/

#endif /* _LOCALE_H */
