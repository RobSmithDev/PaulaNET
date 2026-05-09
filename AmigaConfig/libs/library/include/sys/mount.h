/*
 * $Id: mount.h,v 1.7 2006-01-08 12:06:14 obarthel Exp $
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

#ifndef _SYS_MOUNT_H
#define _SYS_MOUNT_H

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

#ifndef _SYS_TYPES_H
#include <sys/types.h>
#endif /* _SYS_TYPES_H */

/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************/

typedef struct { long val[2]; } fsid_t; /* file system id type */
 
/****************************************************************************/

#define MFSNAMELEN	16 /* length of fs type name, including nul */
#define MNAMELEN	90 /* length of buffer for returned name */

/****************************************************************************/

struct statfs
{
	long	f_bsize;					/* fundamental file system block size */
	long	f_iosize;					/* optimal transfer block size */
	long	f_blocks;					/* total data blocks in file system */
	long	f_bfree;					/* free blocks in fs */
	long	f_bavail;					/* free blocks avail to non-superuser */
	long	f_files;					/* total file nodes in file system */
	long	f_ffree;					/* free file nodes in fs */
	fsid_t	f_fsid;						/* file system id */
	uid_t	f_owner;					/* user that mounted the file system */
	long	f_flags;					/* copy of mount flags */
	long	f_syncwrites;				/* count of sync writes since mount */
	long	f_asyncwrites;				/* count of async writes since mount */
	char	f_fstypename[MFSNAMELEN];	/* fs type name */
	char	f_mntonname[MNAMELEN];		/* directory on which mounted */
	char	f_mntfromname[MNAMELEN];	/* mounted file system */
};

/****************************************************************************/

#define MNT_RDONLY		(1<<0) /* The filesystem is mounted read-only */
#define MNT_NOEXEC		0
#define MNT_NOSUID		0
#define MNT_NODEV		0
#define MNT_SYNCHRONOUS	0
#define MNT_ASYNC		0
#define MNT_UNION		0
#define MNT_NOCOREDUMP	0
#define MNT_NOATIME		(1<<1) /* Never update access times */
#define MNT_SYMPERM		(1<<2) /* Recognize symbolic link permission */
#define MNT_NODEVMTIME	0
#define MNT_SOFTDEP		0
#define MNT_LOCAL		(1<<3) /* The filesystem resides locally */
#define MNT_QUOTA		0
#define MNT_ROOTFS		0
#define MNT_EXRDONLY	0
#define MNT_EXPORTED	0
#define MNT_DEFEXPORTED	0
#define MNT_EXPORTANON	0
#define MNT_EXKERB		0
#define MNT_EXNORESPORT	0
#define MNT_EXPUBLIC	0

/****************************************************************************/

int statfs(const char *path, struct statfs *buf);
int fstatfs(int fd, struct statfs *buf);

/****************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

/****************************************************************************/

#endif /* _SYS_MOUNT_H */
