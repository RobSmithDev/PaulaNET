/*
 * $Id: stdlib_protos.h,v 1.18 2010-10-20 13:50:17 obarthel Exp $
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

#ifndef _STDLIB_PROTOS_H
#define _STDLIB_PROTOS_H

/****************************************************************************/

#ifndef EXEC_TASKS_H
#include <exec/tasks.h>
#endif /* EXEC_TASKS_H */

/****************************************************************************/

#ifndef _STDDEF_H
#include <stddef.h>
#endif /* _STDDEF_H */

/****************************************************************************/

#ifndef _STDLIB_MEMORY_H
#include "stdlib_memory.h"
#endif /* _STDLIB_MEMORY_H */

/****************************************************************************/

/* math_init_exit.c */
extern int	__math_init(void);

/****************************************************************************/

/* stdlib_machine_test.c */
extern int __machine_test(void);

/****************************************************************************/

/* stdio_init_exit.c */
extern int	__stdio_init(void);

/****************************************************************************/

/* stdlib_init_exit.c */
extern int	__stdlib_init(void);
extern void	__stdlib_exit(void);

/****************************************************************************/

/* stdlib_atexit.c */
extern void	__exit_trap_trigger(void);

/****************************************************************************/

/* socket_init_exit.c */
extern int	__socket_init(void);

/****************************************************************************/

/* stdlib_showerror.c */
extern void __show_error(const char * message);

/****************************************************************************/

/* stdlib_swap_stack.c/stdlib_swap_stack.s/stdlib_swap_stack.asm */
extern int __swap_stack_and_call(struct StackSwapStruct * stk,APTR function);

/****************************************************************************/

/* stdlib_get_sp.c/stdlib_get_sp.s/stdlib_get_sp.asm */
extern void * __get_sp(void);

/****************************************************************************/

/* stdlib_wildcard_expand.c */
extern int __wildcard_quote_parameter(unsigned int parameter);
extern int __wildcard_expand_init(void);

/****************************************************************************/

/* stdlib_stackextension.c */
extern int	__stk_init(void);

/****************************************************************************/

/* stdlib_stack_usage.c */
extern void __stack_usage_init(struct StackSwapStruct * stk);
extern void __stack_usage_exit(void);

/****************************************************************************/

/* stdlib_exit.c */
extern void	_exit(int return_code);

/****************************************************************************/

/* stdlib_constructor_begin.c */
void _init(void);
void _fini(void);

/****************************************************************************/

/* stdlib_arg.c */
extern int __arg_init(void);

/****************************************************************************/

/* stdlib_malloc.c */
extern size_t __get_allocation_size(size_t size);
extern void * __allocate_memory(size_t size,BOOL never_free,const char * file,int line);

/* stdlib_free.c */
extern struct MemoryNode * __find_memory_node(void * address);
extern void __check_memory_allocations(const char * file,int line);
extern void __free_memory(void * ptr,BOOL force,const char * file,int line);
extern void __free_memory_node(struct MemoryNode * mn,const char * file,int line);

/****************************************************************************/

/* signal_checkabort.c */
extern void __check_abort(void);

/****************************************************************************/

/* stdlib_red_black.c */
extern void __initialize_red_black_tree(struct MemoryTree *new_tree);
extern void __red_black_tree_insert(struct MemoryTree * tree, struct MemoryNode *x);
extern struct MemoryNode * __red_black_tree_find(struct MemoryTree * tree,void * allocation);
extern void __red_black_tree_remove(struct MemoryTree * tree, struct MemoryNode * z);

/****************************************************************************/

/* stdlib_main.c */
extern int _main(void);

/****************************************************************************/

/* stdlib_assertion_failure.c */
extern void __assertion_failure(const char *file_name,int line_number,const char *expression);

/****************************************************************************/

/* stdlib_termination_message.c */
extern void __print_termination_message(const char * termination_message);

/****************************************************************************/

/* stdlib_set_process_window.c */
extern APTR __set_process_window(APTR new_window_pointer);

/****************************************************************************/

/* stdlib_set_errno.c */
extern void __set_errno(int new_errno);

/****************************************************************************/

/* stdlib_get_errno.c */
extern int __get_errno(void);

/****************************************************************************/

/* stdlib_semaphore.c */
extern struct SignalSemaphore * __create_semaphore(void);
extern void __delete_semaphore(struct SignalSemaphore * semaphore);

/****************************************************************************/

#endif /* _STDLIB_PROTOS_H */
