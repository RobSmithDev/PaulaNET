/*
 * $Id: debug.h,v 1.4 2006-01-08 12:04:22 obarthel Exp $
 *
 * :ts=8
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

#ifndef _DEBUG_H
#define _DEBUG_H

/****************************************************************************/

/* IMPORTANT: If DEBUG is redefined, it must happen only here. This
 *            will cause all modules to depend upon it to be rebuilt
 *            by the smakefile (that is, provided the smakefile has
 *            all the necessary dependency lines in place).
 */

/*#define DEBUG*/

/****************************************************************************/

#define PUSH_REPORTS()	PUSHDEBUGLEVEL(1)
#define PUSH_CALLS()	PUSHDEBUGLEVEL(2)
#define PUSH_ALL()	PUSHDEBUGLEVEL(2)
#define POP()		POPDEBUGLEVEL()

/****************************************************************************/

#if defined(DEBUG)

/****************************************************************************/

#ifndef _STDLIB_H
#include <stdlib.h>
#endif /* _STDLIB_H */

#ifndef _DOS_H
#include <dos.h>
#endif /* _DOS_H */

/****************************************************************************/

extern void kprintf(const char *format,...);

/****************************************************************************/

 void _SHOWVALUE(unsigned long value,int size,const char *name,const char *file,int line);
 void _SHOWPOINTER(const void *p,const char *name,const char *file,int line);
 void _SHOWSTRING(const char *string,const char *name,const char *file,int line);
 void _SHOWMSG(const char *msg,const char *file,int line);
 void _ENTER(const char *file,int line,const char *function);
 void _LEAVE(const char *file,int line,const char *function);
 void _RETURN(const char *file,int line,const char *function,unsigned long result);
 void _DPRINTF_HEADER(const char *file,int line);
 void _DPRINTF(const char *format,...);
 void _DLOG(const char *format,...);
 int  _SETDEBUGLEVEL(int level);
 void _PUSHDEBUGLEVEL(int level);
 void _POPDEBUGLEVEL(void);
 int  _GETDEBUGLEVEL(void);
 void _SETPROGRAMNAME(char *name);

 #if defined(__SASC)
  #define ENTER()		_ENTER(__FILE__,__LINE__,__FUNC__)
  #define LEAVE()		_LEAVE(__FILE__,__LINE__,__FUNC__)
  #define RETURN(r)		_RETURN(__FILE__,__LINE__,__FUNC__,(unsigned long)r)
  #define SHOWVALUE(v)		_SHOWVALUE((unsigned long)v,sizeof(v),#v,__FILE__,__LINE__)
  #define SHOWPOINTER(p)	_SHOWPOINTER(p,#p,__FILE__,__LINE__)
  #define SHOWSTRING(s)		_SHOWSTRING(s,#s,__FILE__,__LINE__)
  #define SHOWMSG(s)		_SHOWMSG(s,__FILE__,__LINE__)
  #define D(s)			do { _DPRINTF_HEADER(__FILE__,__LINE__); _DPRINTF s; } while(0)
  #define PRINTHEADER()		_DPRINTF_HEADER(__FILE__,__LINE__)
  #define PRINTF(s)		_DLOG s
  #define LOG(s)		do { _DPRINTF_HEADER(__FILE__,__LINE__); _DLOG("<%s()>:",__FUNC__); _DLOG s; } while(0)
  #define SETDEBUGLEVEL(l)	_SETDEBUGLEVEL(l)
  #define PUSHDEBUGLEVEL(l)	_PUSHDEBUGLEVEL(l)
  #define POPDEBUGLEVEL()	_POPDEBUGLEVEL()
  #define SETPROGRAMNAME(n)	_SETPROGRAMNAME(n)
  #define GETDEBUGLEVEL()	_GETDEBUGLEVEL()
 #elif defined(__GNUC__)
  #define ENTER()		_ENTER(__FILE__,__LINE__,__FUNCTION__)
  #define LEAVE()		_LEAVE(__FILE__,__LINE__,__FUNCTION__)
  #define RETURN(r)		_RETURN(__FILE__,__LINE__,__FUNCTION__,(unsigned long)r)
  #define SHOWVALUE(v)		_SHOWVALUE((unsigned long)v,sizeof(v),#v,__FILE__,__LINE__)
  #define SHOWPOINTER(p)	_SHOWPOINTER(p,#p,__FILE__,__LINE__)
  #define SHOWSTRING(s)		_SHOWSTRING(s,#s,__FILE__,__LINE__)
  #define SHOWMSG(s)		_SHOWMSG(s,__FILE__,__LINE__)
  #define D(s)			do { _DPRINTF_HEADER(__FILE__,__LINE__); _DPRINTF s; } while(0)
  #define PRINTHEADER()		_DPRINTF_HEADER(__FILE__,__LINE__)
  #define PRINTF(s)		_DLOG s
  #define LOG(s)		do { _DPRINTF_HEADER(__FILE__,__LINE__); _DLOG("<%s()>:",__FUNCTION__); _DLOG s; } while(0)
  #define SETDEBUGLEVEL(l)	_SETDEBUGLEVEL(l)
  #define PUSHDEBUGLEVEL(l)	_PUSHDEBUGLEVEL(l)
  #define POPDEBUGLEVEL()	_POPDEBUGLEVEL()
  #define SETPROGRAMNAME(n)	_SETPROGRAMNAME(n)
  #define GETDEBUGLEVEL()	_GETDEBUGLEVEL()
 #else
  #define __FUNC__ ""
  #define ENTER()		_ENTER(__FILE__,__LINE__,__FUNC__)
  #define LEAVE()		_LEAVE(__FILE__,__LINE__,__FUNC__)
  #define RETURN(r)		_RETURN(__FILE__,__LINE__,__FUNC__,(unsigned long)r)
  #define SHOWVALUE(v)		_SHOWVALUE((unsigned long)v,sizeof(v),#v,__FILE__,__LINE__)
  #define SHOWPOINTER(p)	_SHOWPOINTER(p,#p,__FILE__,__LINE__)
  #define SHOWSTRING(s)		_SHOWSTRING(s,#s,__FILE__,__LINE__)
  #define SHOWMSG(s)		_SHOWMSG(s,__FILE__,__LINE__)
  #define D(s)			do { _DPRINTF_HEADER(__FILE__,__LINE__); _DPRINTF s; } while(0)
  #define PRINTHEADER()		_DPRINTF_HEADER(__FILE__,__LINE__)
  #define PRINTF(s)		_DLOG s
  #define LOG(s)		do { _DPRINTF_HEADER(__FILE__,__LINE__); _DLOG("<%s()>:",__FUNC__); _DLOG s; } while(0)
  #define SETDEBUGLEVEL(l)	_SETDEBUGLEVEL(l)
  #define PUSHDEBUGLEVEL(l)	_PUSHDEBUGLEVEL(l)
  #define POPDEBUGLEVEL()	_POPDEBUGLEVEL()
  #define SETPROGRAMNAME(n)	_SETPROGRAMNAME(n)
  #define GETDEBUGLEVEL()	_GETDEBUGLEVEL()
 #endif

 #undef DEBUG
 #define DEBUG 1

 #define assert(expression)                     \
	((void)                                \
	 ((expression) ? 0 :                   \
	  (                                    \
	   *(char *)0 = 0, /* Enforcer hit */  \
	   _SETDEBUGLEVEL(2),                  \
	   kprintf("[%s] "                     \
	    "%s:%ld: failed assertion '%s'\n", \
	    __program_name,                    \
	    __FILE__,                          \
	    __LINE__,                          \
	    #expression),                      \
	   abort(),                            \
	   0                                   \
	  )                                    \
	 )                                     \
	)

#else

/****************************************************************************/

#ifdef NDEBUG
#define assert(expression) ((void)0)
#else
#include <assert.h>
#endif /* NDEBUG */

/****************************************************************************/

 #define ENTER()		((void)0)
 #define LEAVE()		((void)0)
 #define RETURN(r)		((void)0)
 #define SHOWVALUE(v)		((void)0)
 #define SHOWPOINTER(p)		((void)0)
 #define SHOWSTRING(s)		((void)0)
 #define SHOWMSG(s)		((void)0)
 #define D(s)			((void)0)
 #define PRINTHEADER()		((void)0)
 #define PRINTF(s)		((void)0)
 #define LOG(s)			((void)0)
 #define SETDEBUGLEVEL(l)	((void)0)
 #define PUSHDEBUGLEVEL(l)	((void)0)
 #define POPDEBUGLEVEL()	((void)0)
 #define SETPROGRAMNAME(n)	((void)0)
 #define GETDEBUGLEVEL()	(0)

 #define DEBUG 0
#endif /* DEBUG */

/****************************************************************************/

#endif /* _DEBUG_H */
