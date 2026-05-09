/*
 * $Id: clib2_io.h,v 1.4 2006-11-16 14:39:23 obarthel Exp $
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

#ifndef	_SYS_CLIB2_IO_H
#define	_SYS_CLIB2_IO_H

/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

/* Operations that can be performed by the file action function. */
enum file_action_t
{
	file_action_read,
	file_action_write,
	file_action_seek,
	file_action_close,
	file_action_set_blocking,
	file_action_set_async,
	file_action_examine
};

/****************************************************************************/

/* A message sent to a file action function. */
struct file_action_message
{
	enum file_action_t		fam_Action;		/* What to do */
	char *					fam_Data;		/* Where to read/write the data */
	int						fam_Size;		/* How much data to write */

	long int				fam_Offset;		/* The seek offset */
	int						fam_Mode;		/* The seek mode */

	int						fam_Arg;		/* Action parameters, e.g. whether or not
											   this file should be set non-blocking or
											   use asynchronous I/O */

	struct FileInfoBlock *	fam_FileInfo;	/* File information to be filled in */
	struct MsgPort *		fam_FileSystem;	/* File system pointer to be filled in */

	int						fam_Error;		/* Error code, if any... */
};

/****************************************************************************/

/* A forward declaration to make the following typedef work. */
struct _fd;

/****************************************************************************/

/* The file action function for unbuffered files. */
typedef int (*_file_action_fd_t)(struct _fd * _fd,struct file_action_message * fam);

/****************************************************************************/

/* A low level unbuffered file or socket. */
struct _fd
{
	int					fd_Version;		/* Version number of this definition
										   of the '_fd' data structure (see
										   below). */
	_file_action_fd_t	fd_Action;		/* Function to invoke to perform actions */
	void *				fd_UserData;	/* To be used by custom file action
										   functions */
	ULONG				fd_Flags;		/* File properties */

	union
	{
		BPTR			fdu_File;		/* A dos.library file handle */
		LONG			fdu_Socket;		/* A socket identifier */
	} fdu_Default;
};

/****************************************************************************/

/* The version of the '_fd' data structure, covering the fields 'fd_Version'
   through 'fdu_Default' bears version number 1. */

/****************************************************************************/

/* Sneaky preprocessor tricks to make access to the file/socket IDs
   work smoothly. */
#define fd_DefaultFile	fdu_Default.fdu_File
#define fd_File			fdu_Default.fdu_File
#define fd_Socket		fdu_Default.fdu_Socket

/****************************************************************************/

/* Flag bits that can be set in _fd->fd_Flags; not all of these may be in
   use, or can be changed. */
#define FDF_READ			(1UL<<0)	/* Data can be read from this file */
#define FDF_WRITE			(1UL<<1)	/* Data can be written to this file */
#define FDF_APPEND			(1UL<<2)	/* Before any data is written to it,
										   the file position must be set to the
										   end of the file */
#define FDF_NO_CLOSE		(1UL<<3)	/* Never close this file */
#define FDF_NON_BLOCKING	(1UL<<4)	/* File was switched into non-blocking
										   mode (console streams only) */
#define FDF_IS_SOCKET		(1UL<<5)	/* This is not a disk file but a socket */
#define FDF_IS_LOCKED		(1UL<<6)	/* This file has an advisory record lock set */
#define FDF_IN_USE			(1UL<<7)	/* This file is in use */
#define FDF_CREATED			(1UL<<8)	/* This file was newly created and may need
										   to have its protection bits updated after
										   it has been closed */
#define FDF_CACHE_POSITION	(1UL<<9)	/* Cache the file position. */
#define FDF_ASYNC_IO		(1UL<<10)	/* File was switched into asynchronous I/O
										   mode (sockets only). */
#define FDF_IS_INTERACTIVE	(1UL<<11)	/* File is attached to a console window or
										   something like it. */
#define FDF_STDIO			(1UL<<12)	/* File is to be attached to one of the
										   standard input/output/error streams. */
#define FDF_TERMIOS			(1UL<<13)	/* File is under termios control.
										   FDF_IS_INTERACTIVE should also be set. */

/****************************************************************************/

/* Obtain a pointer to the _fd data structure associated with a file
   descriptor number. Note that this does not perform any locking, which
   means that you have to be absolutely certain that the file will not be
   closed while you are still looking at it. This function can return
   NULL if the file descriptor you provided is not valid. */
extern struct _fd * __get_fd(int file_descriptor);

/* Replaces the action callback function and (optionally) returns the old
   function pointer; returns 0 for success and -1 for failure if you
   provided an invalid file descriptor. This function performs proper locking
   and is thus safe to use in a thread-safe environment. */
extern int __change_fd_action(int file_descriptor,_file_action_fd_t new_action,_file_action_fd_t * old_action_ptr);

/* Replaces the user data pointer and (optionally) returns the old user
   data pointer; returns 0 for success and -1 for failure if you
   provided an invalid file descriptor. This function performs proper locking
   and is thus safe to use in a thread-safe environment. */
extern int __change_fd_user_data(int file_descriptor,void * new_user_data,void ** old_user_data_ptr);

/****************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

/****************************************************************************/

#endif /* _SYS_CLIB2_IO_H */
