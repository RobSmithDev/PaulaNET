/*
 * $Id: termios.h,v 1.5 2006-01-08 12:06:14 obarthel Exp $
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

#ifndef _TERMIOS_H
#define _TERMIOS_H

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************/

/*
 * Almost-but-not-quite termios.
 */

typedef unsigned int	tcflag_t;
typedef unsigned char	cc_t;
typedef unsigned int	speed_t;

#define NCCS	16	/* Number of Control Characters = Length of the c_cc array.  */

enum {	/* All the control characters in c_cc[] in a struct termios (below) */
	VINTR=0,	/* ^C (break)      		*/
	VQUIT,		/* ^\ (quit)       		*/
	VERASE,		/* ^H (eraase)     		*/
	VKILL,		/* ^X (erase-line) 		*/
	VEOF,		/* ^D (end-of-file)		*/
	VMIN,		/* Minimum number of characters to read. Interacts with VTIME.  */
	VEOL,		/* NUL 					*/
	VTIME,		/* Timeout in 1/10:ths of a second. Interacts with VMIN. */
	VSTART,		/* ^Q (continue) 		*/
	VSTOP,		/* ^S (stop) 			*/
	VSUSP,		/* ^Z (suspend) 		*/
	VWERASE,	/* ^W (word-erase)	 	*/
};

struct termios {
	tcflag_t	c_iflag;
	tcflag_t	c_oflag;
	tcflag_t	c_cflag;
	tcflag_t	c_lflag;
	cc_t	c_cc[NCCS];
	/* Private */
	speed_t	c_ispeed;
	speed_t	c_ospeed;
	int type;
	unsigned int flags;
};

/* c_iflag */

#define IGNBRK	(1<<0L)		/* Ignore VINTR */
#define BRKINT	(1<<1L)		/* Unimpl. */
#define IGNPAR	(1<<2L)		/* Unimpl. */
#define PARMRK	(1<<3L)		/* Unimpl. */
#define INPCK	(1<<4L)		/* Unimpl. */
#define ISTRIP	(1<<5L)		/* 7-bit data (strip high bit) */
#define INLCR	(1<<6L)		/* Map NL->CR */
#define IGNCR	(1<<7L)		/* Map CR->nothing   */
#define ICRNL	(1<<8L)		/* Map CR->NL */
#define IXON	(1<<9L)		/* Unimpl. Enable XON/XOFF for output. */
#define IXOFF	(1<<10L)	/* Unimpl. Enable XON/XOFF for input. */

/* c_oflag */

#define OPOST	(1<<0L)		/* Enable output processing. */
#define ONLCR	(1<<1L)		/* Map NL->CR+NL */
#define OCRNL	(1<<2L)		/* Map CR->NL */
#define ONOCR	(1<<3L)		/* Map CR->nothing, but only in column 0. */
#define ONLRET	(1<<4L)		/* Map CR->nothing */

/* c_cflag */

#define CSIZE	(0x07)	/* Bit-width mask. */
#define CS5		(0x01)	/* 5-bits */
#define CS6		(0x02)	/* 6-bits */
#define CS7		(0x03)	/* 7-bits */
#define CS8		(0x04)	/* 8-bits */
#define CSTOPB	(1<<3L)	/* Use 2 stop bits. */
#define CREAD	(1<<4L)	/* Enable reading/receiving. */
#define PARENB	(1<<5L)	/* Enable parity generation/checking. */
#define PARODD	(1<<6L)	/* Parity is odd. */
#define HUPCL	(1<<7L)	/* Hangup on close (when the device is closed). */
#define CLOCAL	(1<<8L)	/* Ignore modem control lines (i.e. a null-modem) */

/* c_lflag */

#define ISIG	(1<<0L)	/* Send SIGINT for ^C, SIGQUIT for ^\ etc. */
#define ICANON	(1<<1L)	/* Canonical mode. Line buffered with EOF, EOL etc. enabled. */
#define ECHO	(1<<2L)	/* Echo input characters. */
#define ECHONL	(1<<3L)	/* Echo newline characters even if ECHO is not set. */
#define IEXTEN	(1<<4L)	/* Enable implementation defined input processing. (none) */

/* Speeds  */

enum {
	B0=0,		/* 0 baud => disconnect. */
	B50=50,
	B75=75,
	B110=110,
	B134=134,
	B150=150,
	B200=200,
	B300=300,
	B600=600,
	B1200=1200,
	B1800=1800,
	B2400=2400,
	B4800=4800,
	B9600=9600,
	B19200=19200,
	B31250=31250,
	B38400=38400,
	B57600=57600,
	B115200=115200,
	B230400=230400,
	B460800=460800,
	B576000=576000,
	B1152000=1152000
};

#define EXTA	1	/* Tell applications we have speeds > 9600 baud. */

/* Action types for tcsetattr() */

enum {
	TCSANOW,
	TCSADRAIN,
	TCSAFLUSH
};

/* Action types for tcflush() */

enum {
	TCIFLUSH,
	TCOFLUSH,
	TCIOFLUSH
};

/* Action types for tcflow() */

enum {
	TCOOFF,
	TCOON,
	TCIOFF,
	TCION
};

/* Prototypes. */

extern int tcgetattr(int file_descriptor,struct termios *tios_p);
extern int tcsetattr(int file_descriptor,int action,struct termios *tios_p);
extern int tcsendbreak(int file_descriptor,int duration);
extern int tcdrain(int file_descriptor);
extern int tcflush(int file_descriptor,int what);
extern int tcflow(int file_descriptor,int action);
extern int cfmakeraw(struct termios *tios_p);
extern speed_t cfgetispeed(struct termios *tios_p);
extern speed_t cfgetospeed(struct termios *tios_p);
extern int cfsetispeed(struct termios *tios_p,speed_t input_speed);
extern int cfsetospeed(struct termios *tios_p,speed_t output_speed);

/****************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

/****************************************************************************/
#endif /* _TERMIOS_H */
