/*
 * $Id: stdlib_memory.h,v 1.4 2006-01-08 12:04:26 obarthel Exp $
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

#ifndef _STDLIB_MEMORY_H
#define _STDLIB_MEMORY_H

/****************************************************************************/

/*
 * Uncomment this to build a library which has the memory debugging features
 * enabled.
 */
/*#define __MEM_DEBUG*/

/****************************************************************************/

/*
 * Uncomment this to see reports of where and how much memory is allocated
 * or released.
 */
/*#define __MEM_DEBUG_LOG*/

/****************************************************************************/

/*
 * Uncomment this to speed up memory data maintenance operations when
 * the memory debugging mode is enabled.
 */
/*#define __USE_MEM_TREES*/

/****************************************************************************/

/*
 * Uncomment this to enable the slab allocator.
 */
#define __USE_SLAB_ALLOCATOR

/****************************************************************************/

/*
 * Memory debugging parameters; note that the head/tail size must be
 * multiples of four, or you will break code that depends upon malloc()
 * and friends to return long word aligned data structures! Better
 * still, there are assert()s in the library which will blow your code
 * out of the water if the data returned by malloc() and realloc() is
 * not long word aligned...
 */

#define MALLOC_HEAD_SIZE 512	/* How many bytes to allocate in front of
								   each memory chunk */
#define MALLOC_TAIL_SIZE 512	/* How many bytes to allocate behind each
								   memory chunk */

/****************************************************************************/

#define MALLOC_NEW_FILL  0xA3	/* The byte value to fill newly created
								   memory allocations with */
#define MALLOC_FREE_FILL 0xA5	/* The byte value to fill freed memory
								   allocations with */
#define MALLOC_HEAD_FILL 0xA7	/* The byte value to fill the memory in
								   front of each allocation with */
#define MALLOC_TAIL_FILL 0xA9	/* The byte value to fill the memory behind
								   each allocation with */

/****************************************************************************/

#ifndef	EXEC_MEMORY_H
#include <exec/memory.h>
#endif /* EXEC_MEMORY_H */

#ifndef EXEC_NODES_H
#include <exec/nodes.h>
#endif /* EXEC_NODES_H */

/****************************************************************************/

#ifndef _MACROS_H
#include "macros.h"
#endif /* _MACROS_H */

/****************************************************************************/

#include <stddef.h>

/****************************************************************************/

/* We shuffle things around a bit for the debug code. This works by joining
   related code which shares the same name. The debug code symbols also have
   to be completely separate from the "regular" code. */
#if defined(__MEM_DEBUG)

#define __static

#define __find_memory_node __find_memory_node_debug
#define __free_memory_node __free_memory_node_debug

#define __allocate_memory __allocate_memory_debug

#define __memory_pool __memory_pool_debug
#define __memory_list __memory_list_debug

#define __vasprintf_hook_entry __vasprintf_hook_entry_debug

extern void * __alloca(size_t size,const char * file,int line);
extern void * __calloc(size_t num_elements,size_t element_size,const char * file,int line);
extern void __free(void * ptr,const char * file,int line);
extern void * __malloc(size_t size,const char * file,int line);
extern void * __realloc(void *ptr,size_t size,const char * file,int line);
extern char * __strdup(const char *s,const char * file,int line);
extern char * __getcwd(char * buffer,size_t buffer_size,const char *file,int line);

#else

#define __static STATIC

#define __free(mem,file,line) free(mem)
#define __malloc(size,file,line) malloc(size)

#endif /* __MEM_DEBUG */

/****************************************************************************/

/* If this flag is set in mn_Size, then this memory allocation
 * cannot be released with free() or used with realloc(). This
 * flag is set by alloca().
 */
#define MN_SIZE_NEVERFREE (0x80000000UL)

/* This obtains the allocation size from a memory node, ignoring
 * the "never free" flag altogether.
 */
#define GET_MN_SIZE(mn) ((mn)->mn_Size & ~MN_SIZE_NEVERFREE)

struct MemoryNode
{
#ifdef __MEM_DEBUG
	struct MinNode		mn_MinNode;

	UBYTE				mn_AlreadyFree;
	UBYTE				mn_Pad0[3];

	void *				mn_Allocation;
	size_t				mn_AllocationSize;

	char *				mn_FreeFile;
	int					mn_FreeLine;

	char *				mn_File;
	int					mn_Line;

#ifdef __USE_MEM_TREES
	struct MemoryNode *	mn_Left;
	struct MemoryNode *	mn_Right;
	struct MemoryNode *	mn_Parent;
	UBYTE				mn_IsRed;
	UBYTE				mn_Pad1[3];
#endif /* __USE_MEM_TREES */

#endif /* __MEM_DEBUG */

	ULONG				mn_Size;
};

#ifdef __USE_MEM_TREES

struct MemoryTree
{
	struct MemoryNode *	mt_Root;
	struct MemoryNode	mt_RootNode;
	struct MemoryNode	mt_NullNode;
};

#endif /* __USE_MEM_TREES */

/****************************************************************************/

/* This keeps track of individual slabs. Each slab begins with this
 * header and is followed by the memory it manages. The size of that
 * memory "slab" is fixed and matches what is stored in
 * SlabData.sd_StandardSlabSize.
 *
 * Each slab manages allocations of a specific maximum size, e.g. 8, 16, 32,
 * 64, etc. bytes. Multiple slabs can exist which manage allocations of the same
 * size, in case one such slab is not enough. Allocations are made from chunks,
 * and for each slab, all chunks are the same size.
 */
struct SlabNode
{
	struct MinNode	sn_MinNode;

	/* If this slab is empty, it goes into a list of slabs to be
	 * purged when memory is tight, or if it has stuck around long
	 * enough without getting purged. This is what the sn_EmptyDecay
	 * field is for. sn_EmptyDecay is decreased whenever an allocation
	 * suceeds which did not use this slab, and when sn_EmptyDecay
	 * reaches 0, the empty slab is purged.
	 */
	struct MinNode	sn_EmptyLink;
	ULONG			sn_EmptyDecay;

	/* How many chunks of memory does this slab contain? */
	ULONG			sn_Count;
	/* How large are the individual chunks? */
	ULONG			sn_ChunkSize;
	/* How many chunks of this slab are currently in use? */
	ULONG			sn_UseCount;

	/* How many times was this slab reused instead of allocating
	 * it from system memory?
	 */
	ULONG			sn_NumReused;

	/* This contains all the chunks of memory which are available
	 * for allocation.
	 */
	struct MinList	sn_FreeList;
};

/* Memory allocations which are not part of a slab are
 * tracked using this data structure.
 */
struct SlabSingleAllocation
{
	struct MinNode	ssa_MinNode;
	ULONG			ssa_Size;
};

/* This is the global bookkeeping information for managing
 * memory allocations from the slab data structure.
 */
struct SlabData
{
	/* This table contains slabs which manage memory chunks
	 * which are a power of 2 bytes in size, e.g. 8, 16, 32,
	 * 64, 128 bytes. Hence, sd_Slabs[3] keeps track of the slabs
	 * which are 8 bytes in size, sd_Slabs[4] is for 16 byte
	 * chunks, etc. The minimum chunk size is 8, which is why
	 * lists 0..2 are not used. Currently, there is an upper limit
	 * of 2^17 bytes per chunk, but you should not be using slab
	 * chunks much larger than 4096 bytes.
	 */
	struct MinList	sd_Slabs[17];

	/* Memory allocations which are larger than the limit
	 * found in the sd_StandardSlabSize field are kept in this list.
	 * They are never associated with a slab.
	 */
	struct MinList	sd_SingleAllocations;

	/* All slabs which currently are empty, i.e. none of their
	 * memory is being used, are registered in this list.
	 * The list linkage uses the SlabNode.sn_EmptyLink field.
	 */
	struct MinList	sd_EmptySlabs;

	/* This is the standard size of a memory allocation which may
	 * be made from a slab that can accommodate it. This number
	 * is initialized from the __slab_max_size global variable,
	 * if > 0, and unless it already is a power of two, it will
	 * be rounded up to the next largest power of two.
	 */
	size_t			sd_StandardSlabSize;

	/* These fields keep track of how many entries there are in
	 * the sd_SingleAllocations list, and how much memory these
	 * allocations occupy.
	 */
	size_t			sd_NumSingleAllocations;
	size_t			sd_TotalSingleAllocationSize;

	/* If this is set to TRUE, then memory allocations will be
	 * be managed through slabs.
	 */
	BOOL			sd_InUse;
};

/****************************************************************************/

extern struct SlabData NOCOMMON	__slab_data;
extern unsigned long NOCOMMON	__slab_max_size;
extern unsigned long NOCOMMON	__slab_purge_threshold;

/****************************************************************************/

extern void __free_unused_slabs(void);
extern void * __slab_allocate(size_t allocation_size);
extern void __slab_free(void * address,size_t allocation_size);
extern void __slab_init(size_t slab_size);
extern void __slab_exit(void);

/****************************************************************************/

extern struct MemoryTree NOCOMMON	__memory_tree;
extern struct MinList NOCOMMON		__memory_list;
extern APTR NOCOMMON				__memory_pool;

/****************************************************************************/

extern unsigned long NOCOMMON __maximum_memory_allocated;
extern unsigned long NOCOMMON __current_memory_allocated;
extern unsigned long NOCOMMON __maximum_num_memory_chunks_allocated;
extern unsigned long NOCOMMON __current_num_memory_chunks_allocated;

/****************************************************************************/

extern int NOCOMMON __default_pool_size;
extern int NOCOMMON __default_puddle_size;

/****************************************************************************/

#if defined(__THREAD_SAFE)

/****************************************************************************/

extern void __memory_lock(void);
extern void __memory_unlock(void);

/****************************************************************************/

#else

/****************************************************************************/

#define __memory_lock()		((void)0)
#define __memory_unlock()	((void)0)

/****************************************************************************/

#endif /* __THREAD_SAFE */

/****************************************************************************/

#endif /* _STDLIB_MEMORY_H */
