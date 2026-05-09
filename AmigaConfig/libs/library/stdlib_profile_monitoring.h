/*
 * $Id: stdlib_profile_monitoring.h,v 1.2 2006-01-08 12:04:26 obarthel Exp $
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

#ifndef _STDLIB_PROFILE_MONITORING_H
#define _STDLIB_PROFILE_MONITORING_H

/****************************************************************************/

/* The SAS/C profiling hooks can be used to track call chains. Neat
   trick, but not always necessary. Don't enable this unless you know
   what you're doing... */

/****************************************************************************/

/*#define USE_PROFILE_MONITORING*/

/****************************************************************************/

#if defined(__SASC) && defined(USE_PROFILE_MONITORING)

/****************************************************************************/

extern void __show_profile_names(void);
extern void __hide_profile_names(void);

/****************************************************************************/

#else

/****************************************************************************/

#define __show_profile_names() ((void)0)
#define __hide_profile_names() ((void)0)

/****************************************************************************/

#endif /* __SASC && USE_PROFILE_MONITORING */

/****************************************************************************/

#endif /* _STDLIB_PROFILE_MONITORING_H */
