/*
 * $Id: stdio_headers.h,v 1.33 2010-10-20 13:12:59 obarthel Exp $
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

#ifndef _STDIO_HEADERS_H
#define _STDIO_HEADERS_H

/****************************************************************************/

#ifndef EXEC_LIBRARIES_H
#include <exec/libraries.h>
#endif /* EXEC_LIBRARIES_H */

#ifndef EXEC_MEMORY_H
#include <exec/memory.h>
#endif /* EXEC_MEMORY_H */

#ifndef DEVICES_TIMER_H
#include <devices/timer.h>
#endif /* DEVICES_TIMER_H */

#ifndef DOS_DOSEXTENS_H
#include <dos/dosextens.h>
#endif /* DOS_DOSEXTENS_H */

#ifndef DOS_DOSTAGS_H
#include <dos/dostags.h>
#endif /* DOS_DOSTAGS_H */

#ifndef DOS_DOSASL_H
#include <dos/dosasl.h>
#endif /* DOS_DOSASL_H */

#ifndef DOS_RECORD_H
#include <dos/record.h>
#endif /* DOS_RECORD_H */

#ifndef LIBRARIES_LOCALE_H
#include <libraries/locale.h>
#endif /* LIBRARIES_LOCALE_H */

#ifndef UTILITY_HOOKS_H
#include <utility/hooks.h>
#endif /* UTILITY_HOOKS_H */

/****************************************************************************/

#if defined(__amigaos4__)
#include <dos/obsolete.h>
#endif /* __amigaos4__ */

/****************************************************************************/

#ifndef CLIB_ALIB_PROTOS_H
#include <clib/alib_protos.h>
#endif /* CLIB_ALIB_PROTOS_H */

/****************************************************************************/

#ifndef PROTO_EXEC_H
#include <proto/exec.h>
#endif /* PROTO_EXEC_H */

#ifndef PROTO_DOS_H
#include <proto/dos.h>
#endif /* PROTO_DOS_H */

/****************************************************************************/

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>

/****************************************************************************/

#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <utime.h>
#include <fcntl.h>
#include <dos.h>

/****************************************************************************/

#if defined(FLOATING_POINT_SUPPORT)
#include <math.h>
#endif /* FLOATING_POINT_SUPPORT */

/****************************************************************************/

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

/****************************************************************************/

#ifndef _MATH_FP_SUPPORT_H
#include "math_fp_support.h"
#endif /* _MATH_FP_SUPPORT_H */

/****************************************************************************/

#ifndef _STDLIB_PROFILE_H
#include "stdlib_profile.h"
#endif /* _STDLIB_PROFILE_H */

/****************************************************************************/

#ifndef _MACROS_H
#include "macros.h"
#endif /* _MACROS_H */

#ifndef _DEBUG_H
#include "debug.h"
#endif /* _DEBUG_H */

/****************************************************************************/

#ifndef _SYS_CLIB2_IO_H
#include <sys/clib2_io.h>
#endif /* _SYS_CLIB2_IO_H */

/****************************************************************************/

/* Forward declarations for below... */
struct fd;
struct iob;

/****************************************************************************/

/* The directory entry type a socket is identified with (in a FileInfoBlock). */
#define ST_SOCKET (31082002)

/* The same for a console stream. */
#define ST_CONSOLE (20050402)

/* And something for NIL: */
#define ST_NIL (20060920)

/****************************************************************************/

/* The file action function for buffered files. */
typedef int (*file_action_iob_t)(struct iob * iob,struct file_action_message * fam);

/****************************************************************************/

/* The three file buffering modes supported. */
#define IOBF_BUFFER_MODE_FULL	_IOFBF	/* Buffer is flushed when it fills up */
#define IOBF_BUFFER_MODE_LINE	_IOLBF	/* Buffer is flushed when it fills up
										   or when a line end character is sent
										   to it */
#define IOBF_BUFFER_MODE_NONE	_IONBF	/* The buffer is never used */

/****************************************************************************/

/* CAREFUL: these mask/flag definitions must match the public definitions
            in <stdio.h> or there will be trouble! */

#define IOBF_BUFFER_MODE	3		/* This is actually a mask which covers
									   the first two bits, both of which
									   contain the buffering mode to use. */
#define IOBF_EOF_REACHED	(1<<2)	/* If this flag is set, then the end of
									   the file was reached while reading
									   from it. */
#define IOBF_READ			(1<<3)	/* If this flag is set then data can
									   be read from this file. */
#define IOBF_WRITE			(1<<4)	/* If this flag is set then data can be
									   written to this file. */
#define IOBF_IN_USE			(1<<5)	/* If this flag is set then this file
									   is not available for allocation */
#define IOBF_ERROR			(1<<6)	/* An error was generated by an I/O
									   operation. */
#define IOBF_TEMP			(1<<7)	/* If this flag is set then the file will
									   be deleted after it has been closed */
#define IOBF_NO_NUL			(1<<8)	/* Don't NUL-terminate strings written to
									   this stream. */
#define IOBF_INTERNAL		(1<<9)	/* Not a regular file, but a buffered
									   file handle made up on the spot in
									   vsprintf(), etc. */

/****************************************************************************/

/* Each file handle is represented by the following structure. Note that this
   must match the layout of the public 'FILE' structure in <stdio.h> or
   things will take a turn for the worse! */
struct iob
{
	ULONG				iob_Flags;				/* Properties and options
												   associated with this file */

	UBYTE *				iob_Buffer;				/* Points to the file buffer */
	LONG				iob_BufferSize;			/* Size of the buffer in bytes */
	LONG				iob_BufferPosition;		/* Current read position
												   in the buffer (grows when any
												   data is read from the buffer) */
	LONG				iob_BufferReadBytes;	/* Number of bytes available for
												   reading (shrinks when any data
												   is read from the buffer) */
	LONG				iob_BufferWriteBytes;	/* Number of bytes written to the
												   buffer which still need to be
												   flushed to disk (grows when any
												   data is written to the buffer) */

	/************************************************************************/
	/* Public portion ends here                                             */
	/************************************************************************/

	file_action_iob_t	iob_Action;				/* The function to invoke for file
												   operations, such as read,
												   write and seek. */

	int					iob_SlotNumber;			/* Points back to the iob table
												   entry number. */

	int					iob_Descriptor;			/* Associated file descriptor */

	STRPTR				iob_String;				/* Alternative source of data;
												   a pointer to a string */
	LONG				iob_StringSize;			/* Number of bytes that may be
												   stored in the string */
	LONG				iob_StringPosition;		/* Current read/write position
												   in the string */
	LONG				iob_StringLength;		/* Number of characters stored
												   in the string */

	char *				iob_File;				/* For access tracking with the
												   memory allocator. */
	int					iob_Line;

	APTR				iob_CustomBuffer;		/* A custom buffer allocated
												   by setvbuf() */

	char *				iob_TempFileName;		/* If this is a temporary
												   file, this is its name */
	BPTR				iob_TempFileLock;		/* The directory in which this
												   temporary file is stored */

	UBYTE				iob_SingleByte;			/* Fall-back buffer for 'unbuffered'
												   files */

	struct SignalSemaphore * iob_Lock;			/* For thread locking */
};

/****************************************************************************/

#define __getc(f) \
	((((struct iob *)(f))->iob_BufferPosition < ((struct iob *)(f))->iob_BufferReadBytes) ? \
	  ((struct iob *)(f))->iob_Buffer[((struct iob *)(f))->iob_BufferPosition++] : \
	  __fgetc((FILE *)(f)))

/* Caution: this putc() variant will evaluate the 'c' parameter more than once. */
#define __putc(c,f,m) \
	(((((struct iob *)(f))->iob_BufferWriteBytes < ((struct iob *)(f))->iob_BufferSize)) ? \
	  (((struct iob *)(f))->iob_Buffer[((struct iob *)(f))->iob_BufferWriteBytes++] = (c), \
	  (((m) == IOBF_BUFFER_MODE_LINE && (c) == '\n') ? \
	   __flush(f) : \
	   ((c) & 255))) : \
	  __fputc((c),(f),(m)))

#define __putc_fully_buffered(c,f) \
	(((((struct iob *)(f))->iob_BufferWriteBytes < ((struct iob *)(f))->iob_BufferSize)) ? \
	  (((struct iob *)(f))->iob_Buffer[((struct iob *)(f))->iob_BufferWriteBytes++] = (c)) : \
	  __fputc((c),(f),IOBF_BUFFER_MODE_FULL))

/* Caution: this putc() variant will evaluate the 'c' parameter more than once. */
#define __putc_line_buffered(c,f) \
	(((((struct iob *)(f))->iob_BufferWriteBytes < ((struct iob *)(f))->iob_BufferSize)) ? \
	  (((struct iob *)(f))->iob_Buffer[((struct iob *)(f))->iob_BufferWriteBytes++] = (c), \
	  (((c) == '\n') ? \
	   __flush(f) : \
	   ((c) & 255))) : \
	  __fputc((c),(f),IOBF_BUFFER_MODE_LINE))

/****************************************************************************/

/* The file action function for unbuffered files. */
typedef int (*file_action_fd_t)(struct fd * fd,struct file_action_message * fam);

/****************************************************************************/

/* Function to be called before a file descriptor is "closed". */
typedef void (*fd_cleanup_t)(struct fd * fd);

/****************************************************************************/

struct fd
{
	int							fd_Version;			/* Version number of this definition
													   of the 'fd' data structure. */
	file_action_fd_t			fd_Action;			/* Function to invoke to perform actions */
	void *						fd_UserData;		/* To be used by custom file action
													   functions */
	ULONG						fd_Flags;			/* File properties */

	union
	{
		BPTR					fdu_File;			/* A dos.library file handle */
		LONG					fdu_Socket;			/* A socket identifier */
	} fdu_Default;

	/************************************************************************/
	/* Public portion ends here                                             */
	/************************************************************************/

	struct SignalSemaphore *	fd_Lock;			/* For thread locking */
	ULONG						fd_Position;		/* Cached file position (seek offset). */
	fd_cleanup_t				fd_Cleanup;			/* Cleanup function, if any. */

	struct fd *					fd_Original;		/* NULL if this is not a dup()ed file
													   descriptor; points to original
													   descriptor if non-NULL */
	struct fd *					fd_NextAlias;		/* Points to next duplicate of this
													   file descriptor; NULL for none */
	void *						fd_Aux;				/* Auxilliary data for "special" files,
													   e.g. termios support. */
};

/****************************************************************************/

/* Files and directories to be unlinked when the program exits. */
struct UnlinkNode
{
	struct MinNode	uln_MinNode;

	BPTR			uln_Lock;		/* Lock on the directory to which the
									   name is relative */
	STRPTR			uln_Name;		/* The name of the file to remove */
};

/****************************************************************************/

struct bcpl_name
{
	unsigned char name[256];
};

/****************************************************************************/

extern struct WBStartup * NOCOMMON __WBenchMsg;

/****************************************************************************/

/* The file handle table. */
extern struct iob ** NOCOMMON	__iob;
extern int NOCOMMON				__num_iob;

/****************************************************************************/

/* The file descriptor table. */
extern struct fd ** NOCOMMON	__fd;
extern int NOCOMMON				__num_fd;

/****************************************************************************/

extern const char * NOCOMMON __file_lock_semaphore_name;

/****************************************************************************/

extern BOOL NOCOMMON __no_standard_io;

/****************************************************************************/

/* CPU cache line size; used to align I/O buffers for best performance. */
extern ULONG __cache_line_size;

/****************************************************************************/

/*extern int __iob_write_buffer_is_full(struct iob * file);*/

#define __iob_write_buffer_is_full(file) \
	(((struct iob *)file)->iob_BufferSize > 0 && \
	((struct iob *)file)->iob_BufferWriteBytes == ((struct iob *)file)->iob_BufferSize)

/****************************************************************************/

#define __iob_write_buffer_is_valid(file) \
	(((struct iob *)file)->iob_BufferWriteBytes > 0)

/****************************************************************************/

#define __iob_num_unwritten_bytes(file) \
	(((struct iob *)file)->iob_BufferWriteBytes)

/****************************************************************************/

/*extern int __iob_read_buffer_is_empty(struct iob * file);*/

#define __iob_read_buffer_is_empty(file) \
	(((struct iob *)file)->iob_BufferReadBytes == 0 || \
	((struct iob *)file)->iob_BufferPosition == ((struct iob *)file)->iob_BufferReadBytes)

/****************************************************************************/

#define __iob_num_unread_bytes(file) \
	((((struct iob *)file)->iob_BufferReadBytes > 0 && \
	 ((struct iob *)file)->iob_BufferPosition < ((struct iob *)file)->iob_BufferReadBytes) \
	  ? (((struct iob *)file)->iob_BufferReadBytes - ((struct iob *)file)->iob_BufferPosition) \
	  : 0)

/****************************************************************************/

#define __iob_read_buffer_is_valid(file) \
	(((struct iob *)file)->iob_BufferReadBytes > 0)

/****************************************************************************/

#define __fd_is_aliased(fd) \
	((fd)->fd_Original != NULL || (fd)->fd_NextAlias != NULL)

/****************************************************************************/

#if defined(__THREAD_SAFE)

/****************************************************************************/

extern void	__stdio_lock(void);
extern void	__stdio_unlock(void);
extern void	__stdio_lock_exit(void);
extern int	__stdio_lock_init(void);

/****************************************************************************/

extern void __fd_lock(struct fd *fd);
extern void __fd_unlock(struct fd *fd);

/****************************************************************************/

extern BPTR __resolve_fd_file(struct fd * fd);

/****************************************************************************/

#else

/****************************************************************************/

#define __stdio_lock()		((void)0)
#define __stdio_unlock()	((void)0)
#define __stdio_lock_exit()	((void)0)
#define __stdio_lock_init()	(0)

/****************************************************************************/

#define __fd_lock(fd)		((void)0)
#define __fd_unlock(fd)		((void)0)

/****************************************************************************/

#define __resolve_fd_file(fd) (fd->fd_File)

/****************************************************************************/

#endif /* __THREAD_SAFE */

/****************************************************************************/

#ifndef _STDIO_PROTOS_H
#include "stdio_protos.h"
#endif /* _STDIO_PROTOS_H */

/****************************************************************************/

#endif /* _STDIO_HEADERS_H */
