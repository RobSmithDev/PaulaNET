/*
 * $Id: unistd.h,v 1.23 2007-01-06 10:09:49 obarthel Exp $
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

#ifndef _UNISTD_H
#define _UNISTD_H

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

#ifndef _FCNTL_H
#include <fcntl.h>
#endif /* _FCNTL_H */

#ifndef _STDIO_H
#include <stdio.h>
#endif /* _STDIO_H */

#if !defined(__NO_NET_API) && !defined(_SYS_SELECT_H)
#include <sys/select.h>
#endif /* __NO_NET_API && _SYS_SELECT_H */

/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************/

/* Endianness: we assume a big endian memory layout (for 68k and PowerPC). */
#define LITTLE_ENDIAN	1234
#define BIG_ENDIAN		4321
#define BYTE_ORDER		BIG_ENDIAN

/****************************************************************************/

#define STDIN_FILENO	0
#define STDOUT_FILENO	1
#define STDERR_FILENO	2

/****************************************************************************/

#define R_OK 0
#define W_OK 1
#define X_OK 2
#define F_OK 4

/****************************************************************************/

#define F_ULOCK	0
#define F_LOCK	1
#define F_TLOCK	2
#define F_TEST	3

/****************************************************************************/

extern int opterr;
extern int optind;
extern int optopt;
extern char * optarg;

/****************************************************************************/

extern int isatty(int file_descriptor);
extern int dup(int file_descriptor);
extern int dup2(int file_descriptor1, int file_descriptor2);
extern int access(const char * path_name, int mode);
extern int chown(const char * path_name, uid_t owner, gid_t group);
extern int fchown(int file_descriptor, uid_t owner, gid_t group);
extern int lchown(const char * path_name, uid_t owner, gid_t group);
extern int truncate(const char * path_name, off_t length);
extern int ftruncate(int file_descriptor, off_t length);
extern int link(const char * existing_path,const char * new_path);
extern int unlink(const char * path_name);
extern int symlink(const char * actual_path, const char * symbolic_path);
extern int readlink(const char * path_name, char * buffer, int buffer_size);
extern int chdir(const char * path_name);
extern int lockf(int file_descriptor, int function, off_t size);
extern unsigned int sleep(unsigned int seconds);
extern void usleep(unsigned long microseconds);
extern int getopt(int argc, char * const argv[], const char *opts);
extern pid_t getpid(void);
extern char *realpath(const char *file_name, char *resolved_name);
extern int fsync(int file_descriptor);
extern int fdatasync(int file_descriptor);
extern char *ttyname(int);
extern int ttyname_r(int file_descriptor,char *name,size_t buflen);
extern int ttyname_t(int,char *,size_t);
extern int execl(const char *path,const char *arg0,...);
extern int execle(const char *path,const char *arg0,...);
extern int execlp(const char *path,const char *arg0,...);
extern int execv(const char *path,char * const argv[]);
extern int execve(const char *path,char *const argv[],char *const envp[]);
extern int execvp(const char *command,char * const argv[]);
extern int profil(unsigned short *buffer, size_t bufSize, size_t offset, unsigned int scale);

/****************************************************************************/

extern char * getcwd(char * buffer, size_t buffer_size);

#ifdef __MEM_DEBUG
extern char * __getcwd(char * buffer,size_t buffer_size,const char *file,int line);

#define getcwd(buffer,buffer_size) __getcwd((buffer),(buffer_size),__FILE__,__LINE__)
#endif /* __MEM_DEBUG */

/****************************************************************************/

/*
 * The following prototypes may clash with the bsdsocket.library or
 * usergroup.library API definitions.
 */

#ifndef __NO_NET_API

extern long gethostid(void);
extern int gethostname(const char *name, size_t namelen);
extern char *crypt(const char *key, const char *salt);
extern gid_t getegid(void);
extern uid_t geteuid(void);
extern gid_t getgid(void);
extern int getgroups(int gidsetsize, gid_t grouplist[]);
extern char * getpass(const char *prompt);
extern uid_t getuid(void);
extern int initgroups(const char *name, gid_t basegid);
extern int setegid(gid_t gid);
extern int seteuid(uid_t uid);
extern int setgid(gid_t gid);
extern int setgroups(int ngroups, const gid_t *gidset);
extern int setregid(gid_t rgid, gid_t egid);
extern int setreuid(uid_t ruid, uid_t euid);
extern pid_t setsid(void);
extern int setuid(uid_t uid);

#endif /* __NO_NET_API */

/****************************************************************************/

/* The following is for use with pathconf()/fpathconf() */
#define	_PC_LINK_MAX				0
#define	_PC_MAX_CANON				1
#define	_PC_MAX_INPUT				2
#define	_PC_NAME_MAX				3
#define	_PC_PATH_MAX				4
#define	_PC_PIPE_BUF				5
#define	_PC_CHOWN_RESTRICTED		6
#define	_PC_NO_TRUNC				7
#define	_PC_VDISABLE				8
#define	_PC_SYNC_IO					9
#define	_PC_ASYNC_IO				10
#define	_PC_PRIO_IO					11
#define	_PC_SOCK_MAXBUF				12
#define	_PC_FILESIZEBITS			13
#if 0
#define	_PC_REC_INCR_XFER_SIZE		14
#define	_PC_REC_MAX_XFER_SIZE		15
#define	_PC_REC_MIN_XFER_SIZE		16
#define	_PC_REC_XFER_ALIGN			17
#define	_PC_ALLOC_SIZE_MIN			18
#endif
#define	_PC_SYMLINK_MAX				19
#define	_PC_XATTR_EXISTS			20
#define	_PC_XATTR_ENABLED			21

/* Amiga Specific */
#define	_PC_DOSTYPE					300

/****************************************************************************/

extern long pathconf(const char *path,int name);
extern long fpathconf(int file_descriptor,int name);

/****************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

/****************************************************************************/

#endif /* _UNISTD_H */
