/*
 * $Id: termios_headers.h,v 1.3 2006-01-08 12:04:27 obarthel Exp $
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

#ifndef	_TERMIOS_HEADERS_H
#define	_TERMIOS_HEADERS_H

/****************************************************************************/

#ifndef	_STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

/****************************************************************************/

#ifndef	_TERMIOS_H
#include <termios.h>
#endif /* _TERMIOS_H */

/****************************************************************************/

extern int __termios_console_hook(struct fd *,struct file_action_message *);
extern int __termios_serial_hook(struct fd *,struct file_action_message *);
extern struct termios *__get_termios(struct fd *);
extern int openserial(const char *device_name,int unit,int mode);

/****************************************************************************/

enum	/* Termios types. */
{
	TIOST_INVALID = 0,
	TIOST_CONSOLE,
	TIOST_SERIAL
};

enum
{
	TIOS_FLUSH_OUTPUT,
	TIOS_FLUSH_SYNC,
	TIOS_FLUSH_INPUT,
	TIOS_FLUSH_IO
};

/****************************************************************************/

#endif /* _TERMIOS_HEADERS_H */
