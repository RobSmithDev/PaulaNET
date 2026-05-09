/*
 * $Id: dos.h,v 1.28 2006-09-27 09:40:06 obarthel Exp $
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

#ifndef _DOS_H
#define _DOS_H

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

#ifndef _STDIO_H
#include <stdio.h>
#endif /* _STDIO_H */

#ifndef _STDDEF_H
#include <stddef.h>
#endif /* _STDDEF_H */

/****************************************************************************/

#ifndef EXEC_LIBRARIES_H
#include <exec/libraries.h>
#endif /* EXEC_LIBRARIES_H */

#ifndef WORKBENCH_STARTUP_H
#include <workbench/startup.h>
#endif /* WORKBENCH_STARTUP_H */

/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************/

/*
 * The Workbench startup message passed to this program; this may be NULL
 * if the program was started from shell instead.
 *
 * The variable name is set up to be __WBenchMsg to make it fit into the
 * ISO 'C' standard context. It is redefined to WBenchMsg to make it
 * compatible with the original Amiga program startup code which would
 * refer to the variable under that name.
 *
 * It is recommended to access the Workbench startup message as part of your
 * program's main() function, like this:
 *
 *    int
 *    main(int argc,char ** argv)
 *    {
 *       struct WBStartup * wb_startup_message;
 *
 *       if(argc == 0)
 *          wb_startup_message = (struct WBStartup *)argv;
 *       else
 *          wb_startup_message = NULL;
 *
 *       ...
 *    }
 *
 * This approach has the advantage that your program does not rely upon a
 * global variable initialized by the startup code, whose name you might
 * not even know exactly.
 */
extern struct WBStartup * __WBenchMsg;
#define WBenchMsg __WBenchMsg

/****************************************************************************/

/* This is filled in with a pointer to the name of the program being run. */
extern char * __program_name;

/****************************************************************************/

/* Set this to FALSE to disable all Ctrl+C checking in the program. */
extern BOOL __check_abort_enabled;

/*
 * You can replace this function with your own and perform your own
 * Ctrl+C checking.
 */
extern void __check_abort(void);

/****************************************************************************/

/*
 * You can override the default break signal mask which is used by
 * __check_abort() and other functions. This must be done at link
 * time because the break signal checking is set up very early on
 * while the program startup code is preparing your code to be run.
 * In particular, this affects the socket I/O functionality which
 * configures the break signal only once. The default value of the
 * __break_signal_mask variable is SIGBREAKF_CTRL_C.
 */
extern ULONG __break_signal_mask;

/****************************************************************************/

/*
 * Call this function to set up the environment information for your
 * program to access its data. This is typically used in Task or Process
 * functions, as launched from the main program.
 *
 * Note: this function is unavailable for residentable programs and may
 *       not be available for PowerPC programs.
 */
extern void geta4(void);

/****************************************************************************/

/*
 * Read and modify the current value of register A4; helpful for
 * programs which are residentable, yet need to be able to pass the
 * context information around A4 points to.
 *
 * Note: this function may not be available for PowerPC programs.
 */
extern unsigned long __get_a4(void);
extern void __set_a4(unsigned long value);

/****************************************************************************/

/*
 * Obtain the low level 'file' handle or socket ID bound to a file
 * descriptor. This function returns 0 for success and non-zero
 * otherwise (if, for example, the file descriptor value you
 * provided is not valid; errno will be set to the appropriate
 * error code, too). Put a pointer to the file handle variable you
 * want to be filled in into the second parameter to this function.
 */
extern int __get_default_file(int file_descriptor,long * file_ptr);

/****************************************************************************/

/*
 * If your program is intended to be launched by the Internet superserver
 * (inetd) to run as a daemon, receiving and sending data through a network
 * connection set up for it, declare the following variable in your program
 * and set it to TRUE. This will make the program's network startup code
 * link the stdin/stdout/stderr streams with the daemon socket passed to it.
 * Whether or not your program is running as a daemon can be tested by
 * checking the global __is_daemon variable described below.
 */
extern BOOL __check_daemon_startup;

/****************************************************************************/

/*
 * This will be set to TRUE if the current program was launched from
 * the internet superserver ('inetd') or an equivalent facility. Note
 * that you need to have set __check_daemon_startup to TRUE before
 * you can rely upon __is_daemon to be initialized correctly.
 */
extern BOOL __is_daemon;

/****************************************************************************/

/*
 * If the library is built with memory debugging features enabled,
 * the following variable controls whether memory allocated, to be
 * released, will actually get released. If set to TRUE all memory
 * allocations will persist until the program exits.
 */
extern BOOL __never_free;

/****************************************************************************/

/*
 * Before memory is allocated, a quick check can be performed in order
 * to avoid draining so much system memory that both the operating system
 * and application software becomes unstable. This test checks for the
 * largest available block of memory, which has to be larger than a
 * threshold value for the memory allocation to proceed. That size
 * can be configured here. It defaults to 0, which means that no test
 * is performed. If this feature is enabled, a minimum threshold value
 * of 250000 bytes is recommended.
 */
extern ULONG __free_memory_threshold;

/****************************************************************************/

/*
 * You can monitor how much memory, and in how many chunks, is allocated
 * over the lifetime of your program. Call the following function with
 * non-NULL parameters to obtain the current state of memory allocations.
 * Parameters called with NULL instead of pointer to the counters to
 * be filled in will be ignored.
 */
extern void __get_mem_stats(size_t * current_memory,size_t * max_memory,
	size_t * current_chunks,size_t * max_chunks);

/*
 * The following function will reset the counters for "maximum amount
 * of memory used" and "maximum number of chunks used" to the figures
 * for the current memory usage.
 */
extern void __reset_max_mem_stats(void);

/****************************************************************************/

/*
 * If you use the clib2-supplied alloca() function, which will allocate
 * memory from the system pool rather than extending the current stack
 * frame, then your program will use a lot less stack space. It might
 * use a lot more system memory, though. While the system memory usage
 * solves the problem of a program crashing due to ever increasing
 * stack usage, which is hard to gauge, there is problem in that many
 * programs which call alloca() never test if the value returned is
 * not NULL. They just assume that alloca() will always succeed. If
 * you use the clib2-supplied alloca() function then the result may in
 * fact be NULL. In which case the program making the call might just
 * crash because of a missing NULL test.
 *
 * You can avoid trouble by filling in a pointer to a function which
 * will be called when the clib2-supplied alloca() function finds that it
 * must return NULL. That function is expected to print an error message
 * and to call abort().
 */
extern void (*__alloca_trap)(void);

/****************************************************************************/

/*
 * The following section lists variables and function pointers which are used
 * by the startup code right after the program is launched. These variables are
 * looked up before your program's main() function is invoked. Therefore, you
 * would have to declare these variables in your program's data section and have
 * them initialized to certain well-defined values to get the desired effect.
 */

/*
 * The minimum required operating system version number is 37, which
 * corresponds to Workbench/Kickstart 2.04. You may request a higher
 * version number by defining the following variable; if you do so,
 * please provide a fitting error message, too. Note that you cannot
 * request a minimum version number lower than 37.
 */
extern int __minimum_os_lib_version;
extern char * __minimum_os_lib_error;

/*
 * If your program is launched from Workbench it will not necessarily
 * have a window to send console output to and from which console
 * input can be read. The startup code attempts to set up such a console
 * window for you, but it uses defaults for the window specification.
 * These defaults can be overridden by your program if you define a
 * variable to point to the specification string. Note that if you
 * request a specific window specification, this will also override
 * any tool window definition stored in the program's icon.
 */
extern char * __stdio_window_specification;

/*
 * If set to TRUE, your program's process->pr_WindowPtr will be set to -1
 * when it starts. The process->pr_WindowPtr will be automatically restored
 * to the initial value before it exits.
 */
extern BOOL __disable_dos_requesters;

/*
 * If set to TRUE, your program will disconnect itself from the shell it was
 * launched from and keep running in the background. This service is unavailable
 * for residentable programs. Note that you should not use this feature for
 * programs which are supposed to be launched by the internet superserver.
 * Also, note that when a program is run in the background, its input and
 * output streams will be connected to NIL:.
 */
extern BOOL __detach;

/*
 * If this function pointer is not NULL, it must point to a function which
 * figures out whether the program should detach itself from the shell it
 * was launched from. The function return value replaces the value of the
 * __detach variable.
 *
 * At the time this function is invoked, dos.library and utility.library
 * have already been opened for you.
 */
extern BOOL (* __check_detach)(void);

/*
 * If this pointer is not NULL, it refers the name that will be given to
 * the process which is created when the program detaches. The default
 * is to reuse the program name instead.
 */
extern char * __process_name;

/*
 * This variable controls the task priority of the program, when running.
 * It must be in the range -128..127 to be useful. By default, the task
 * priority will not be changed.
 */
extern int __priority;

/*
 * This variable can be set up to contain the minimum stack size the program
 * should be launched with. If the startup code finds that there is not enough
 * stack space available to start with, it will attempt to allocate more and
 * then proceed to run your program.
 *
 * If this variable is set to 0 (the default) then no stack size test will
 * be performed upon startup.
 */
extern unsigned int __stack_size;

/*
 * If this function pointer is not NULL, it must point to a function which
 * figures out how much stack space is required to run the program. The
 * function return value replaces the value of the __stack_size variable
 * if it is not equal to zero.
 *
 * At the time this function is invoked, dos.library and utility.library
 * have already been opened for you.
 */
extern unsigned int (* __get_default_stack_size)(void);

/****************************************************************************/

/*
 * This library falls back onto locale.library to perform string collation
 * in strcoll(), character conversion in toupper() and various other
 * functions. This may not be your intention. To restrict the library to use
 * only the "C" language locale, declare the following variable in your
 * code and set it to FALSE, so that it overrides the default settings.
 * The variable value is checked during program startup and, if set to
 * TRUE, has the effect of opening locale.library and obtaining the
 * default locale.
 */
extern BOOL __open_locale;

/*
 * Two functions control how this library uses the locale.library API to
 * perform string collation, character and time conversion, etc.
 *
 * __locale_init() opens locale.library and attempts to open the default
 * locale. If it succeeds, it will return 0 and -1 otherwise.
 *
 * __locale_exit() releases the default locale and closes locale.library.
 */
extern int __locale_init(void);
extern void __locale_exit(void);

/****************************************************************************/

/*
 * If linked with -lunix, Unix path names are translated to Amiga path
 * names (and the other way round). If you wish to disable this, set the
 * following variable to FALSE. Only the path name translation is affected
 * by setting this variable to FALSE. You will always get Unix-like behaviour
 * from certain functions regardless of whether the path names are translated
 * or not.
 */
extern BOOL __unix_path_semantics;

/****************************************************************************/

/* A data structures used by the path translation routines below. */
struct name_translation_info
{
	char	substitute[MAXPATHLEN];
	char *	original_name;
	int		is_root;
};

/****************************************************************************/

extern int __translate_relative_path_name(char const ** name_ptr,char *replace,size_t max_replace_len);
extern void __restore_path_name(char const ** name_ptr,struct name_translation_info * nti);
extern int __translate_amiga_to_unix_path_name(char const ** name_ptr,struct name_translation_info * nti);
extern int __translate_unix_to_amiga_path_name(char const ** name_ptr,struct name_translation_info * nti);
extern int __translate_io_error_to_errno(LONG io_error);

/****************************************************************************/

/*
 * Routines for use with shared libraries: invoke __lib_init() in your own
 * shared library initialization function and __lib_exit() in your shared
 * library cleanup function.
 *
 * __lib_init() will initialize the global SysBase/DOSBase variables
 * (and the IExec/IDOS variables for OS4) and invoke the constructor
 * functions required by your library code. It returns FALSE upon
 * failure and TRUE otherwise. Make this the very first function you
 * call in your shared library initialization function. The __lib_init()
 * function expects to be called with a pointer to the exec.library
 * base, which is normally passed to your shared library as part of the
 * library startup code initialization performed by the operating
 * system.
 *
 * __lib_exit() will undo all the initializations performed by the
 * __lib_init() function, but leave the global SysBase variable
 * (and the IExec variable for OS4) intact. Make this the very last
 * function you call in your shared library cleanup function.
 *
 * Note that neither __lib_init() nor __lib_exit() are reentrant. You must
 * make sure that while you are calling them no other library user can
 * call them by accident.
 *
 * Both functions are only available as part of the thread-safe clib2
 * linker library.
 */
extern VOID __lib_exit(VOID);
extern BOOL __lib_init(struct Library * _SysBase);

/****************************************************************************/

/*
 * The following variables are part of libnet.a, which provides for
 * a BSD-Unix-like socket networking API. Traditionally, only one process
 * at a time may use the underlying bsdsocket.library base, but with a
 * multithreaded application you may want all of them to share the same
 * library base. As of this writing there is one single TCP/IP stack which
 * supports this feature (Roadshow) and it must be enabled early on. If
 * this worked out well you can test through the following variable which
 * will be set to TRUE:
 */
extern BOOL __can_share_socket_library_base;

/*
 * The next global variable is also part of the thread-safe libnet.a and
 * indicates that the TCP/IP stack will call the functions __set_errno()
 * and __set_h_errno() when it modifies the global errno and h_errno
 * variables, respectively. If you want to save the error codes for each
 * of the Processes in your multithreaded application then you should
 * override these functions with your. The following variable will be
 * set to TRUE if the __set_errno() and __set_h_errno() functions will
 * be used to change the corresponding variables:
 */
extern BOOL __thread_safe_errno_h_errno;

/****************************************************************************/

/*
 * If you link against libunix.a then the default command line processing
 * function will attempt to expand every single wildcard parameter on the
 * command line into a series of file and directories names matching the
 * wildcards. The idea is to provide functionality which on Unix the
 * shell is responsible for. On AmigaDOS the shell commands need to perform
 * the expansion. However, if you are mixing AmigaDOS commands which expand
 * wildcard patterns with a shell that already does the job, you may run into
 * big trouble. To disable the expansion, declare the global variable named
 * "__expand_wildcard_args" in your code and have it set to FALSE. Because
 * the program startup code checks this variable early on, its value must
 * be available at that time, i.e. you cannot just set it differently in
 * your code lateron because by that time the startup code will have already
 * checked it.
 *
 * Note that the startup code will disable wildcard expansion if the local
 * shell environment variable "DISABLE_COMMANDLINE_WILDCARD_EXPANSION"
 * is set.
 */
extern BOOL __expand_wildcard_args;

/*
 * Similar to the boolean flag value __expand_wildcard_args described above,
 * a function can be called which may be used to enable/disable wildcard
 * expansion at runtime. The function is undefined by default, which means
 * that the __expand_wildcard_args value will take precedence. If you want
 * to override the effects of the __expand_wildcard_args variable, declare
 * your own check function and then assign it to the
 * __expand_wildcard_args_check pointer.
 */
extern BOOL (*__expand_wildcard_args_check)(void);

/****************************************************************************/

/*
 * Defaults for path delimiter (":") and the shell search path
 * ("/gcc/bin:/SDK/C:/SDK/Local/C:/C:.") as used by the execvp()
 * function.
 */
extern const char * __default_path_delimiter;
extern const char * __default_path;

/****************************************************************************/

/*
 * 'environ' is the default environment variable table as used by the execl(),
 * execv() and execvp() functions. This needs to be initialized before you
 * can use it. The table has the following form:
 *
 *    char ** environ =
 *    { 
 *       "variable1=value",
 *       "variable2=value",
 *       NULL
 *    };
 *
 * Note that if you initialize the 'environ' table you will also have to
 * provide for a function which prepares its contents in execve() for use
 * by the command to be executed. That function is called
 * __execve_environ_init(). Should program execution fail, you need to
 * clean up after what __execve_environ_init() set up. To do this, call
 * __execve_environ_exit(). There are stubs in clib2 for these functions
 * which essentially do nothing at all. You will have to implement these
 * yourself if you want to use them.
 */
extern char ** environ;

extern int __execve_environ_init(char * const envp[]);
extern void __execve_environ_exit(char * const envp[]);

/****************************************************************************/

/*
 * The __execve_exit() function is called by execve() if the command
 * executed correctly and control should be returned to the shell. The
 * default behaviour is to eventually call exit(). You can, however,
 * replace __execve_exit() with a stub which does nothing at all. In
 * that case the execve() function will return control to the caller
 * instead.
 */

extern void __execve_exit(int return_code);

/****************************************************************************/

/*
 * The unlink() and remove() functions in libunix.a may return success even
 * though deletion failed because the file/directory/link in question is still
 * reported as being "in use". This is the default behaviour. If you want the
 * deletion to fail instead, set '__unlink_retries' to FALSE.
 */
extern BOOL __unlink_retries;

/****************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

/****************************************************************************/

#endif /* _DOS_H */
