/*
 * $Id: fcntl.h,v 1.9 2006-01-08 12:06:14 obarthel Exp $
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

#ifndef _FCNTL_H
#define _FCNTL_H

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

#ifndef _SYS_TYPES_H
#include <sys/types.h>
#endif /* _SYS_TYPES_H */

#ifndef _STDDEF_H
#include <stddef.h>
#endif /* _STDDEF_H */

/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************/

#define O_RDONLY	0
#define O_WRONLY	1
#define O_RDWR		2

#define O_APPEND	(1<<2)
#define O_CREAT		(1<<3)
#define O_EXCL		(1<<4)
#define O_TRUNC		(1<<5)
#define O_NONBLOCK	(1<<6)
#define O_NDELAY	O_NONBLOCK
#define O_SYNC		(0)
#define O_NOCTTY	(0)
#define O_ASYNC		(1<<7)

/****************************************************************************/

#define F_DUPFD		0
#define F_GETFD		1
#define F_SETFD		2
#define F_GETFL		3
#define F_SETFL		4
#define F_GETOWN	5
#define F_SETOWN	6

/****************************************************************************/

/*
 * Advisory file segment locking data type -
 * information passed to system by user
 */
struct flock
{
	short	l_type;		/* lock type: read/write, etc. */
	short	l_whence;	/* type of l_start */
	off_t	l_start;	/* starting offset */
	off_t	l_len;		/* len = 0 means until end of file */
	pid_t	l_pid;		/* lock owner */
};

#define F_GETLK		100	/* get record locking information */
#define F_SETLK		101	/* set record locking information */
#define F_SETLKW	102	/* F_SETLK; wait if blocked */

#define F_RDLCK		1	/* shared or read lock */
#define F_UNLCK		2	/* unlock */
#define F_WRLCK		3	/* exclusive or write lock */

/****************************************************************************/

extern int open(const char *path_name, int open_flag, ... /* mode_t mode */ );
extern int creat(const char * path_name, mode_t mode);
extern int close(int file_descriptor);
extern off_t lseek(int file_descriptor, off_t offset, int mode);
extern ssize_t read(int file_descriptor, void * buffer, size_t num_bytes);
extern ssize_t write(int file_descriptor, const void * buffer, size_t num_bytes);
extern int fcntl(int file_descriptor, int cmd, ... /* int arg */ );

/****************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

/****************************************************************************/

#endif /* _FCNTL_H */
