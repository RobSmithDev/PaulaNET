/*
 * $Id: netdb.h,v 1.4 2006-01-08 12:06:14 obarthel Exp $
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

#ifndef _NETDB_H
#define _NETDB_H

/****************************************************************************/

#ifndef _SYS_SOCKET_H
#include <sys/socket.h>
#endif /* _SYS_SOCKET_H */

#ifndef _NETINET_IN_H
#include <netinet/in.h>
#endif /* _NETINET_IN_H */

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************/

#ifdef __GNUC__
 #ifdef __PPC__
  #pragma pack(2)
 #endif
#elif defined(__VBCC__)
 #pragma amiga-align
#endif

/****************************************************************************/

#define MAXHOSTNAMELEN 256 /* max hostname size */

/****************************************************************************/

/*
 * Structures returned by network data base library.  All addresses are
 * supplied in host order, and returned in network order (suitable for
 * use in system calls).
 */
struct hostent
{
	char *	h_name;				/* official name of host */
	char **	h_aliases;			/* alias list */
	int		h_addrtype;			/* host address type */
	int		h_length;			/* length of address */
	char **	h_addr_list;		/* list of addresses from name server */
};

#define	h_addr h_addr_list[0] /* address, for backward compatiblity */

/*
 * Assumption here is that a network number
 * fits in an unsigned long -- probably a poor one.
 */
struct netent
{
	char *			n_name;		/* official name of net */
	char **			n_aliases;	/* alias list */
	int				n_addrtype;	/* net address type */
	in_addr_t		n_net;		/* network # */
};

struct servent
{
	char *		s_name;		/* official service name */
	char **		s_aliases;	/* alias list */
	int			s_port;		/* port # */
	char *		s_proto;	/* protocol to use */
};

struct protoent
{
	char *		p_name;		/* official protocol name */
	char **		p_aliases;	/* alias list */
	int			p_proto;	/* protocol # */
};

/****************************************************************************/

/*
 * Error return codes from gethostbyname() and gethostbyaddr()
 * (left in extern int h_errno).
 */

#define	NETDB_INTERNAL	-1	/* see errno */
#define	NETDB_SUCCESS	0	/* no problem */
#define	HOST_NOT_FOUND	1 	/* Authoritative Answer Host not found */
#define	TRY_AGAIN		2 	/* Non-Authoritive Host not found, or SERVERFAIL */
#define	NO_RECOVERY		3 	/* Non recoverable errors, FORMERR, REFUSED, NOTIMP */
#define	NO_DATA			4 	/* Valid name, no data record of requested type */
#define	NO_ADDRESS		NO_DATA	/* no address, look for MX record */

/****************************************************************************/

extern int h_errno;

/****************************************************************************/

/*
 * The following prototypes may clash with the bsdsocket.library or
 * usergroup.library API definitions.
 */

#ifndef __NO_NET_API

extern struct hostent *gethostbyaddr(const void *addr, socklen_t len, int type);
extern struct hostent *gethostbyname(const char *name);
extern struct netent *getnetbyaddr(in_addr_t net, int type);
extern struct netent *getnetbyname(const char *name);
extern struct protoent *getprotobyname(const char *name);
extern struct protoent *getprotobynumber(int proto);
extern struct servent *getservbyname(const char *name, const char *proto);
extern struct servent *getservbyport(int port, const char *proto);
extern const char *hstrerror(int err);

#endif /* __NO_NET_API */

/****************************************************************************/

#ifdef __GNUC__
 #ifdef __PPC__
  #pragma pack()
 #endif
#elif defined(__VBCC__)
 #pragma default-align
#endif

/****************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

/****************************************************************************/

#endif /* _NETDB_H */
