/*
 * $Id: ftw.h,v 1.5 2006-01-08 12:06:14 obarthel Exp $
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

#ifndef	_FTW_H
#define	_FTW_H

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

#include <sys/stat.h>

/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************/

/* Values passed to the user function (argument #3). */
enum
{
	FTW_F,		/* Regular file. */
	FTW_D,		/* Directory. */
	FTW_DP,		/* Directory, already visited. (nftw() only) */
	FTW_SL,		/* Symlink. (nftw() only) */
	FTW_SLN,	/* Broken Symlink. (does not point to an existing file, nftw() only) */
	FTW_DNR,	/* Directory which can not be read (e.g. not enough permissions) */
	FTW_NS		/* Stat failed. */
};

/****************************************************************************/

/* Flags for FTW.quit */
#define	FTW_SKD		(1L<<0)	/* Skip directory. */
#define	FTW_PRUNE	(1L<<1)	/* Prune traversal. (skip up) */

/****************************************************************************/

/* Flags for nftw() */
#define	FTW_PHYS		(1L<<0)
#define	FTW_MOUNT		(1L<<1)		/* Currently unsupported. */
#define	FTW_DEPTH		(1L<<2)
#define	FTW_CHDIR		(1L<<3)
#define	FTW_ALL_FLAGS	(0x0000000f)

/****************************************************************************/

struct FTW
{
	int quit;	/* Flags passed out from the user function to ftw()/nftw() */
	int base;	/* Index of current item from start of string. e.g. 4 for "foo/bar" */
	int level;	/* Current depth. (counted from 0) */
};

/****************************************************************************/

extern int ftw(const char *path,int (*func)(const char *,const struct stat *,int),int depth);
extern int nftw(const char *path,int (*func)(const char *,const struct stat *,int,struct FTW *),int depth,int flags);

/****************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

/****************************************************************************/

#endif /* _FTW_H */
