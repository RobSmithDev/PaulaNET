/*
 * $Id: stat.h,v 1.10 2006-07-28 13:19:12 obarthel Exp $
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

#ifndef _SYS_STAT_H
#define _SYS_STAT_H

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

#ifndef _SYS_TYPES_H
#include <sys/types.h>
#endif /* _SYS_TYPES_H */

#ifndef _STDDEF_H
#include <stddef.h>
#endif /* _STDDEF_H */

#ifndef _TIME_H
#include <time.h>
#endif /* _TIME_H */

/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************/

#define	S_ISUID	0004000		/* set user id on execution */
#define	S_ISGID	0002000		/* set group id on execution */
#define	S_ISVTX	0001000		/* save swapped text even after use */

#define	S_IRWXU	0000700		/* RWX mask for owner */
#define	S_IRUSR	0000400		/* R for owner */
#define	S_IWUSR	0000200		/* W for owner */
#define	S_IXUSR	0000100		/* X for owner */

#define	S_IRWXG	0000070		/* RWX mask for group */
#define	S_IRGRP	0000040		/* R for group */
#define	S_IWGRP	0000020		/* W for group */
#define	S_IXGRP	0000010		/* X for group */

#define	S_IRWXO	0000007		/* RWX mask for other */
#define	S_IROTH	0000004		/* R for other */
#define	S_IWOTH	0000002		/* W for other */
#define	S_IXOTH	0000001		/* X for other */

/****************************************************************************/

/* Aliases for the owner RWX mask. */
#define	S_IREAD		S_IRUSR
#define	S_IWRITE	S_IWUSR
#define	S_IEXEC		S_IXUSR

/****************************************************************************/

#define	S_IFMT		0170000	/* type of file */
#define	S_IFIFO		0010000	/* named pipe (fifo) */
#define	S_IFCHR		0020000	/* character special */
#define	S_IFDIR		0040000	/* directory */
#define	S_IFBLK		0060000	/* block special */
#define	S_IFREG		0100000	/* regular */
#define	S_IFLNK		0120000	/* symbolic link */
#define	S_IFSOCK	0140000	/* socket */

/****************************************************************************/

#define	S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)		/* directory */
#define	S_ISCHR(m)	(((m) & S_IFMT) == S_IFCHR)		/* char special */
#define	S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)		/* regular file */
#define	S_ISLNK(m)	(((m) & S_IFMT) == S_IFLNK)		/* symbolic link */
#define	S_ISFIFO(m)	(((m) & S_IFMT) == S_IFIFO)		/* fifo */
#define	S_ISBLK(m)	(0)								/* block special */
#define S_ISSOCK(m)	(((m) & S_IFSOCK) == S_IFSOCK)	/* socket */

/****************************************************************************/

struct stat
{
	mode_t	st_mode;
	ino_t	st_ino;
	dev_t	st_dev;
	dev_t	st_rdev;
	nlink_t	st_nlink;
	uid_t	st_uid;
	gid_t	st_gid;
	off_t	st_size;
	time_t	st_atime;
	time_t	st_mtime;
	time_t	st_ctime;
	long	st_blksize;
	long	st_blocks;
};

/****************************************************************************/

extern int stat(const char * path_name, struct stat * buffer);
extern int fstat(int file_descriptor, struct stat * buffer);
extern int lstat(const char * path_name, struct stat * buffer);
extern int chmod(const char * path_name, mode_t mode);
extern int fchmod(int file_descriptor, mode_t mode);
extern int mkdir(const char * path_name, mode_t mode);
extern int rmdir(const char * path_name);

/*
 * The following prototypes may clash with the bsdsocket.library or
 * usergroup.library API definitions.
 */

#ifndef __NO_NET_API

extern mode_t umask(mode_t new_mask);

#endif /* __NO_NET_API */

/****************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

/****************************************************************************/

#endif /* _SYS_STAT_H */
