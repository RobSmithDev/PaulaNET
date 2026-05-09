/*
 * $Id: stdlib.h,v 1.19 2008-04-30 14:34:03 obarthel Exp $
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

#ifndef _STDLIB_H
#define _STDLIB_H

/****************************************************************************/

#ifndef _STDDEF_H
#include <stddef.h>
#endif /* _STDDEF_H */

#ifndef _SYS_CLIB2_STDC_H
#include <sys/clib2_stdc.h>
#endif /* _SYS_CLIB2_STDC_H */

/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************/

/* Maximum number of bytes in a multibyte character */
#define MB_CUR_MAX 2

/****************************************************************************/

/* The maximum value that can be returned by the rand() function */
#define RAND_MAX 2147483647

/****************************************************************************/

/* Return values to be passed to exit() */
#define EXIT_FAILURE 20
#define EXIT_SUCCESS 0

/****************************************************************************/

/* Data structures used by the div() and ldiv() functions */
typedef struct { int quot; int rem; } div_t;
typedef struct { long quot; long rem; } ldiv_t;

/****************************************************************************/

extern int mblen(const char *s, size_t n);
extern int mbtowc(wchar_t *pwc, const char *s, size_t n);
extern int wctomb(char *s, wchar_t wchar);
extern size_t mbstowcs(wchar_t *pwcs, const char *s, size_t n);
extern size_t wcstombs(char *s, const wchar_t *pwcs, size_t n);

/****************************************************************************/

extern void *malloc(size_t size);
extern void *calloc(size_t num_elements,size_t element_size);
extern void free(void *ptr);
extern void *realloc(void *ptr,size_t size);

#ifdef __MEM_DEBUG
extern void *__malloc(size_t size,const char *file,int line);
extern void *__calloc(size_t num_elements,size_t element_size,const char *file,int line);
extern void __free(void *ptr,const char *file,int line);
extern void *__realloc(void *ptr,size_t size,const char *file,int line);

#define malloc(size) __malloc((size),__FILE__,__LINE__)
#define calloc(num_elements,element_size) __calloc((num_elements),(element_size),__FILE__,__LINE__)
#define free(ptr) __free((ptr),__FILE__,__LINE__)
#define realloc(ptr,size) __realloc((ptr),(size),__FILE__,__LINE__)
#endif /* __MEM_DEBUG */

/****************************************************************************/

extern int abs(int x);
extern long labs(long x);
extern div_t div(int n,int d);
extern ldiv_t ldiv(long n,long d);

/****************************************************************************/

extern int rand(void);
extern void srand(unsigned seed);

/****************************************************************************/

extern int system(const char *command);

/****************************************************************************/

extern void exit(int status);
extern void abort(void);

/****************************************************************************/

extern int atexit(void (*)(void));

/****************************************************************************/

extern char * getenv(const char *name);

/****************************************************************************/

extern void * bsearch(const void *key, const void *base, size_t count, size_t size,
	int (*compare)(const void * key,const void * value));
extern void qsort(void *base,size_t count,size_t size,
	int (*compare)(const void * element1,const void * element2));

/****************************************************************************/

extern double strtod(const char *str, char ** ptr);
extern long strtol(const char *str, char **ptr, int base);
extern unsigned long strtoul(const char *str, char **ptr, int base);

/****************************************************************************/

extern double atof(const char *str);
extern int atoi(const char *str);
extern long atol(const char *str);

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

extern void _exit(int status);
extern int rand_r(unsigned int * seed);

/****************************************************************************/

/*
 * You can switch the built-in memory allocator, which is a thin wrapper
 * around the AmigaOS built-in memory management system, to use a slab
 * allocator. For this to work, you need to declare a global variable
 * and set it to the size of the slabs to be used. This variable must
 * be initialized at load time when the clib2 startup code runs:
 *
 * unsigned long __slab_max_size = 4096;
 */

extern unsigned long __slab_max_size;

/*
 * The slab allocator will periodically free all currently unused memory.
 * You can control how much memory should be released, instead of
 * releasing everything.
 *
 * This would make the slab allocator release only up to 512 KBytes of
 * unused memory at a time:
 *
 * unsigned long __slab_purge_threshold = 512 * 1024;
 */

extern unsigned long __slab_purge_threshold;

/****************************************************************************/

/*
 * If you are using the slab allocator and need to quickly release the
 * memory of all slabs which are currently unused, you can call the
 * following function to do so.
 *
 * Please note that this function works within the context of the memory
 * allocation system and is not safe to call from interrupt code. It may
 * break a Forbid() or Disable() condition.
 */
extern void __free_unused_slabs(void);

/****************************************************************************/

/*
 * You can accelerate the reuse of currently unused slabs by calling
 * the __decay_unused_slabs() function. Each call decrements the decay
 * counter until it reaches 0, at which point an unused slab can be
 * reused instead of allocating a new slab. Also, at 0 unused slabs
 * will be freed by the allocator.
 *
 * Please note that this function works within the context of the memory
 * allocation system and is not safe to call from interrupt code. It may
 * break a Forbid() or Disable() condition.
 */
extern void __decay_unused_slabs(void);

/****************************************************************************/

/*
 * You can obtain runtime statistics about the slab allocator by
 * invoking the __get_slab_usage() function which in turn invokes
 * your callback function for each single slab currently in play.
 *
 * Your callback function must return 0 if it wants to be called again,
 * for the next slab, or return -1 to stop. Note that your callback
 * function may not be called if the slab allocator is currently
 * not operational.
 *
 * Please note that this function works within the context of the memory
 * allocation system and is not safe to call from interrupt code. It may
 * break a Forbid() or Disable() condition.
 */

/****************************************************************************/

/* This is what your callback function will see when it is invoked. */
struct __slab_usage_information
{
	/* The size of all slabs, in bytes. */
	size_t	sui_slab_size;

	/* Number of allocations which are not managed by slabs, but
	 * are handled separate.
	 */
	size_t	sui_num_single_allocations;

	/* Total number of bytes allocated for memory not managed
	 * by slabs. This includes the management overhead for
	 * each allocation.
	 */
	size_t	sui_total_single_allocation_size;

	/* Number of slabs currently in play. This can be 0. */
	size_t	sui_num_slabs;

	/* Number of currently unused slabs which contain no data. */
	size_t	sui_num_empty_slabs;

	/* Number of slabs in use which are completely filled with data. */
	size_t	sui_num_full_slabs;

	/* Total number of bytes allocated for all slabs. */
	size_t	sui_total_slab_allocation_size;

	/*
	 * The following data is updated for each slab which
	 * your callback function sees.
	 */

	/* Index number of the slab being reported (0 = no slabs are in use). */
	int		sui_slab_index;

	/* How large are the memory chunks managed by this slab? */
	size_t	sui_chunk_size;

	/* How many memory chunks fit into this slab? */
	size_t	sui_num_chunks;

	/* How many memory chunks in this slab are being used? */
	size_t	sui_num_chunks_used;

	/* How many time was this slab reused without reinitializing
	 * it all over again from scratch?
	 */
	size_t	sui_num_reused;
};

/****************************************************************************/

typedef int (*__slab_usage_callback)(const struct __slab_usage_information * sui);

/****************************************************************************/

void __get_slab_usage(__slab_usage_callback callback);

/****************************************************************************/

/*
 * You can obtain runtime statistics about the memory allocations
 * which the slab allocator did not fit into slabs. This works
 * just like __get_slab_usage() in that the callback function
 * you provide will be called for each single allocation that
 * is not part of a slab.
 *
 * Your callback function must return 0 if it wants to be called again,
 * for the next slab, or return -1 to stop. Note that your callback
 * function may not be called if the slab allocator did not
 * allocate memory outside of slabs.
 *
 * Please note that this function works within the context of the memory
 * allocation system and is not safe to call from interrupt code. It may
 * break a Forbid() or Disable() condition.
 */

/* This is what your callback function will see when it is invoked. */
struct __slab_allocation_information
{
	/* Number of allocations which are not managed by slabs, but
	 * are handled separate.
	 */
	size_t	sai_num_single_allocations;

	/* Total number of bytes allocated for memory not managed
	 * by slabs. This includes the management overhead for
	 * each allocation.
	 */
	size_t	sai_total_single_allocation_size;

	/*
	 * The following data is updated for each slab which
	 * your callback function sees.
	 */

	/* Index number of the allocation being reported (0 = no allocations
	 * outside of slabs are in use).
	 */
	int		sai_allocation_index;

	/* Size of this allocation, as requested by the program which
	 * called malloc(), realloc() or alloca().
	 */
	size_t	sai_allocation_size;

	/* Total size of this allocation, including management data
	 * structure overhead.
	 */
	size_t	sai_total_allocation_size;
};

/****************************************************************************/

typedef int (*__slab_allocation_callback)(const struct __slab_allocation_information * sui);

/****************************************************************************/

void __get_slab_allocations(__slab_allocation_callback callback);

/****************************************************************************/

/*
 * You can obtain information about the memory managed by the slab allocator,
 * as well as additional information about the slab allocator's performance
 * in JSON format. This format can be used for more detailed analysis.
 *
 * You supply a function which will be called for each line of the JSON
 * data produced. You can store this data in a file, or in the clipboard,
 * for later use. Your function must return 0 if it wants to be called
 * again, or return -1 if it wants to stop (e.g. if an error occured
 * when writing the JSON data to disk). The same "user_data" pointer which
 * you pass to __get_slab_stats() will be passed to your callback function.
 *
 * Please note that this function works within the context of the memory
 * allocation system and is not safe to call from interrupt code. It may
 * break a Forbid() or Disable() condition.
 */

typedef int (* __slab_status_callback)(void * user_data, const char * line, size_t line_length);

/****************************************************************************/

extern void __get_slab_stats(void * user_data, __slab_status_callback callback);

/****************************************************************************/

/*
 * You can request to use the alloca() variant that actually does allocate
 * memory from the system rather than the current stack frame, which will
 * ease stack requirements but may not release allocate memory immediately.
 * In order to do so, add #define __USE_CLIB2_ALLOCA to your program, or
 * -D__USE_CLIB2_ALLOCA to your build makefile. For GCC you will also have
 * to compile your program with the -fno-builtin switch to override the
 * built-in alloca() function.
 */

#if defined(alloca)
#undef alloca
#endif /* alloca */

#if defined(__GNUC__) && !defined(__USE_CLIB2_ALLOCA)
 #define alloca(size) __builtin_alloca(size)
#else
 extern void * alloca(size_t size);

 #ifdef __MEM_DEBUG
  extern void * __alloca(size_t size,const char *file,int line);

  #define alloca(size) __alloca((size),__FILE__,__LINE__)
 #else
  /* This is necessary because some applications key off the fact that
     alloca is a symbol defined by the preprocessor. */
  #define alloca alloca
 #endif /* __MEM_DEBUG */
#endif /* __GNUC__ */

/****************************************************************************/

extern int setenv(const char *name, const char *value, int overwrite);
extern int putenv(const char *string);
extern int unsetenv(const char *name);
extern char * mktemp(char * name_template);
extern int mkstemp(char *name_template);
extern char * mkdtemp(char *name_template);

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard, but it should
   be part of ISO/IEC 9899:1999, also known as "C99". */

/****************************************************************************/

#if defined(__GNUC__) || (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L))

/****************************************************************************/

extern long long strtoll(const char *str, char **ptr, int base);
extern unsigned long long strtoull(const char *str, char **ptr, int base);
extern long long atoll(const char *str);

/****************************************************************************/

typedef struct { long long quot; long long rem; } lldiv_t;

/****************************************************************************/

extern long long llabs(long long x);
extern lldiv_t lldiv(long long n,long long d);

/****************************************************************************/

extern int mbtowc(wchar_t *restrict pwc, const char *restrict s, size_t n);
extern int wctomb(char *s, wchar_t wchar);

/****************************************************************************/

#endif /* __GNUC__ || (__STDC_VERSION__ && __STDC_VERSION__ >= 199901L) */

/****************************************************************************/

extern float strtof(const char *str, char ** ptr);

/****************************************************************************/

extern void _Exit(int status);

/****************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

/****************************************************************************/

#endif /* _STDLIB_H */
