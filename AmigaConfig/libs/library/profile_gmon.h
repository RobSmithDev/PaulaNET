/*
* $Id$
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

#ifndef _GMON_H
#define _GMON_H

#include <exec/types.h>

struct gmonhdr
{
	uint32 	lpc;
	uint32 	hpc;
	int 	ncnt;
	int 	version;
	int 	profrate;
	int		reserved[3];
};

#define GMONVERSION 0x00051879

#define HISTCOUNTER uint16
// I am sure we can make these bigger
#define HISTFRACTION 2
#define HASHFRACTION 4

#define ARCDENSITY 	2
#define MINARCS		50
#define MAXARCS		((1 << (8 * sizeof(HISTCOUNTER)))-2)

struct tostruct
{
	uint32 	selfpc;
	int32	count;
	uint16	link;
	uint16 	pad;
};

struct rawarc
{
	uint32 	raw_frompc;
	uint32	raw_selfpc;
	int32	raw_count;
};

#define ROUNDDOWN(x,y) (((x)/(y))*(y))
#define ROUNDUP(x,y)   ((((x)+(y)-1)/(y))*(y))

struct gmonparam
{
	int 				state;
	uint16 *			kcount;
	uint32				kcountsize;
	uint16 *			froms;
	uint32				fromssize;
	struct tostruct *	tos;
	uint32				tossize;
	int32 				tolimit;
	uint32				lowpc;
	uint32				highpc;
	uint32				textsize;
	uint32				hashfraction;
	uint8 *				memory;
};

extern struct gmonparam _gmonparam;

enum 
{
	kGmonProfOn 		= 0,
	kGmonProfBusy 		= 1,
	kGmonProfError		= 2,
	kGmonProfOff		= 3
};

enum
{
	kGprofState 		= 0,
	kGprofCount			= 1,
	kGprofFroms			= 2,
	kGprofTos			= 3,
	kGprofGmonParam		= 4
};

#endif
