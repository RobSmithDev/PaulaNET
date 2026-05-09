/*
 * $Id: stdio_protos.h,v 1.19 2006-01-08 12:04:24 obarthel Exp $
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

#ifndef _STDIO_PROTOS_H
#define _STDIO_PROTOS_H

/****************************************************************************/

#ifndef UTILITY_HOOKS_H
#include <utility/hooks.h>
#endif /* UTILITY_HOOKS_H */

/****************************************************************************/

#ifndef _STDIO_H
#include <stdio.h>
#endif /* _STDIO_H */

/****************************************************************************/

#ifndef _MATH_FP_SUPPORT_H
#include "math_fp_support.h"
#endif /* _MATH_FP_SUPPORT_H */

/****************************************************************************/

#ifndef _STDLIB_MEMORY_H
#include "stdlib_memory.h"
#endif /* _STDLIB_MEMORY_H */

/****************************************************************************/

#ifndef _STDIO_HEADERS_H

/* Forward declarations */
struct file_action_message;
struct iob;
struct fd;

#endif /* _STDIO_HEADERS_H */

/****************************************************************************/

/* stdio_init_exit.c */
void __close_all_files(void);

/****************************************************************************/

/* stdio_translateioerror.c */
extern int __translate_io_error_to_errno(LONG io_error);
extern int __translate_access_io_error_to_errno(LONG io_error);

/****************************************************************************/

/* stdio_get_file_descriptor.c */
extern struct fd * __get_file_descriptor(int file_descriptor);
extern struct fd * __get_file_descriptor_dont_resolve(int file_descriptor);

/****************************************************************************/

/* stdio_iobhookentry.c */
extern int __iob_hook_entry(struct iob * iob,struct file_action_message * fam);

/****************************************************************************/

/* stdio_fdhookentry.c */
extern int __fd_hook_entry(struct fd * fd,struct file_action_message * fam);

/****************************************************************************/

/* stdio_initializefd.c */
extern void __initialize_fd(struct fd * fd,file_action_fd_t action_function,BPTR default_file,ULONG flags,struct SignalSemaphore * lock);

/****************************************************************************/

/* stdio_findvacantfdentry.c */
extern BOOL __is_valid_fd(struct fd * fd);
extern int __find_vacant_fd_entry(void);

/****************************************************************************/

/* stdio_initializeiob.c */
extern void __initialize_iob(struct iob * iob,file_action_iob_t action_function,STRPTR custom_buffer,STRPTR buffer,int buffer_size,int file_descriptor,int slot_number,ULONG flags,struct SignalSemaphore * lock);

/****************************************************************************/

/* stdio_findvacantiobentry.c */
extern BOOL __is_valid_iob(struct iob * iob);
extern int __find_vacant_iob_entry(void);

/****************************************************************************/

/* stdio_growfdtable.c */
extern int __grow_fd_table(int max_fd);

/****************************************************************************/

/* stdio_growiobtable.c */
extern int __grow_iob_table(int max_fd);

/****************************************************************************/

/* stdio_openiob.c */
extern int __open_iob(const char *filename, const char *mode, int file_descriptor, int slot_number);

/****************************************************************************/

/* stdio_filliobreadbuffer.c */
extern int __fill_iob_read_buffer(struct iob * file);

/****************************************************************************/

/* stdio_dropiobreadbuffer.c */
extern int __drop_iob_read_buffer(struct iob * file);

/****************************************************************************/

/* stdio_flushiobwritebuffer.c */
extern int __flush_iob_write_buffer(struct iob * file);

/****************************************************************************/

/* stdio_fgetc.c */
extern int __fgetc_check(FILE * stream);
extern int __fgetc(FILE *stream);

/****************************************************************************/

/* stdio_fputc.c */
extern int __fputc_check(FILE *stream);
extern int __fputc(int c,FILE *stream,int buffer_mode);

/****************************************************************************/

/* stdio_sscanf_hook_entry.c */
extern int __sscanf_hook_entry(struct iob *string,struct file_action_message *fam);

/****************************************************************************/

/* stdio_vasprintf_hook_entry.c */
extern int __vasprintf_hook_entry(struct iob *string,struct file_action_message *fam);

/****************************************************************************/

/* stdio_vsprintf_hook_entry.c */
extern int __vsprintf_hook_entry(struct iob *string,struct file_action_message *fam);

/****************************************************************************/

/* stdio_vsnprintf_hook_entry.c */
extern int __vsnprintf_hook_entry(struct iob *string,struct file_action_message *fam);

/****************************************************************************/

/* stdio_duplicate_fd.c */
extern void __duplicate_fd(struct fd * duplicate_fd,struct fd * original_fd);

/****************************************************************************/

/* stdio_examine_fh.c */
extern LONG __safe_examine_file_handle(BPTR file_handle,struct FileInfoBlock *fib);

/****************************************************************************/

/* stdio_parent_of_fh.c */
extern BPTR __safe_parent_of_file_handle(BPTR file_handle);

/****************************************************************************/

/* stdio_grow_file.c */
extern int __grow_file_size(struct fd * fd,int num_bytes);

/****************************************************************************/

/* unistd_sync_fd.c */
extern int __sync_fd(struct fd * fd,int mode);

/****************************************************************************/

/* stdio_record_locking.c */
extern int __handle_record_locking(int cmd,struct flock * l,struct fd * fd,int * error_ptr);

/****************************************************************************/

/* stdio_remove_fd_alias.c */
extern void __remove_fd_alias(struct fd * fd);

/****************************************************************************/

/* stdio_file_init.c */
extern int __stdio_file_init(void);

/****************************************************************************/

/* stdio_flush_all_files.c */
extern int __flush_all_files(int buffer_mode);

/****************************************************************************/

#endif /* _STDIO_PROTOS_H */
