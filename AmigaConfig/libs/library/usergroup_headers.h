/*
 * $Id: usergroup_headers.h,v 1.8 2006-01-08 12:04:27 obarthel Exp $
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

#ifndef _USERGROUP_HEADERS_H
#define _USERGROUP_HEADERS_H

/****************************************************************************/

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

/****************************************************************************/

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

/****************************************************************************/

#ifndef _STDLIB_PROFILE_H
#include "stdlib_profile.h"
#endif /* _STDLIB_PROFILE_H */

/****************************************************************************/

#ifndef _PWD_H
#include <pwd.h>
#endif /* _PWD_H */

#ifndef _GRP_H
#include <grp.h>
#endif /* _GRP_H */

/****************************************************************************/

extern struct Library * NOCOMMON __UserGroupBase;

/****************************************************************************/

#if defined(__amigaos4__)

extern struct UserGroupIFace * NOCOMMON __IUserGroup;

#endif /* __amigaos4__ */

/****************************************************************************/

extern BOOL NOCOMMON __root_mode;

/****************************************************************************/

extern int NOCOMMON __root_uid;
extern int NOCOMMON __root_gid;
extern int NOCOMMON __root_euid;
extern int NOCOMMON __root_egid;

/****************************************************************************/

#if defined(__GNUC__)

#if defined(__amigaos4__)

#include <interfaces/usergroup.h>

#define __ug_SetupContextTagList(name, tags) __IUserGroup->ug_SetupContextTagList(name, tags)
#define __ug_GetErr() __IUserGroup->ug_GetErr()
#define __ug_StrError(err) __IUserGroup->ug_StrError(err)
#define __getuid() __IUserGroup->getuid()
#define __geteuid() __IUserGroup->geteuid()
#define __setreuid(real, effective) __IUserGroup->setreuid(real, effective)
#define __setuid(uid) __IUserGroup->setuid(uid)
#define __getgid() __IUserGroup->getgid()
#define __getegid() __IUserGroup->getegid()
#define __setregid(real, effective) __IUserGroup->setregid(real, effective)
#define __setgid(gid) __IUserGroup->setgid(gid)
#define __getgroups(gidsetlen, gidset) __IUserGroup->getgroups(gidsetlen, gidset)
#define __setgroups(gidsetlen, gidset) __IUserGroup->setgroups(gidsetlen, gidset)
#define __initgroups(name, basegid) __IUserGroup->initgroups(name, basegid)
#define __getpwnam(login) __IUserGroup->getpwnam(login)
#define __getpwuid(uid) __IUserGroup->getpwuid(uid)
#define __setpwent() __IUserGroup->setpwent()
#define __getpwent() __IUserGroup->getpwent()
#define __endpwent() __IUserGroup->endpwent()
#define __getgrnam(name) __IUserGroup->getgrnam(name)
#define __getgrgid(gid) __IUserGroup->getgrgid(gid)
#define __setgrent() __IUserGroup->setgrent()
#define __getgrent() __IUserGroup->getgrent()
#define __endgrent() __IUserGroup->endgrent()
#define __crypt(key, set) __IUserGroup->crypt(key, set)
#define __ug_GetSalt(user, buf, size) __IUserGroup->ug_GetSalt(user, buf, size)
#define __getpass(prompt) __IUserGroup->getpass(prompt)
#define __umask(mask) __IUserGroup->umask(mask)
#define __getumask() __IUserGroup->getumask()
#define __setsid() __IUserGroup->setsid()
#define __getpgrp() __IUserGroup->getpgrp()
#define __getlogin() __IUserGroup->getlogin()
#define __setlogin(name) __IUserGroup->setlogin(name)
#define __setutent() __IUserGroup->setutent()
#define __getutent() __IUserGroup->getutent()
#define __endutent() __IUserGroup->endutent()
#define __getlastlog(uid) __IUserGroup->getlastlog(uid)
#define __setlastlog(uid, name, host) __IUserGroup->setlastlog(uid, name, host)
#define __getcredentials(task) __IUserGroup->getcredentials(task) 

#else

// modified by JOB: added args to "r" list for some functions
#define __ug_SetupContextTagList(name, tags) ({ \
  UBYTE * _ug_SetupContextTagList_name = (name); \
  struct TagItem * _ug_SetupContextTagList_tags = (tags); \
  LONG _ug_SetupContextTagList__re = \
  ({ \
  register struct Library * const __ug_SetupContextTagList__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  register LONG __ug_SetupContextTagList__re __asm("d0"); \
  register UBYTE * __ug_SetupContextTagList_name __asm("a0") = (_ug_SetupContextTagList_name); \
  register struct TagItem * __ug_SetupContextTagList_tags __asm("a1") = (_ug_SetupContextTagList_tags); \
  __asm volatile ("jsr %%a6@(-30:W)" \
  : "=r"(__ug_SetupContextTagList__re) \
  : "r"(__ug_SetupContextTagList__bn),"r"(__ug_SetupContextTagList_name),"r"(__ug_SetupContextTagList_tags)  \
  : "d1", "fp0", "fp1", "cc", "memory"); \
  __ug_SetupContextTagList__re; \
  }); \
  _ug_SetupContextTagList__re; \
})

#define __ug_GetErr() ({ \
  LONG _ug_GetErr__re = \
  ({ \
  register struct Library * const __ug_GetErr__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  register LONG __ug_GetErr__re __asm("d0"); \
  __asm volatile ("jsr %%a6@(-36:W)" \
  : "=r"(__ug_GetErr__re) \
  : "r"(__ug_GetErr__bn)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __ug_GetErr__re; \
  }); \
  _ug_GetErr__re; \
})

#define __ug_StrError(err) ({ \
  LONG _ug_StrError_err = (err); \
  UBYTE * _ug_StrError__re = \
  ({ \
  register struct Library * const __ug_StrError__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  register UBYTE * __ug_StrError__re __asm("d0"); \
  register LONG __ug_StrError_err __asm("d1") = (_ug_StrError_err); \
  __asm volatile ("jsr %%a6@(-42:W)" \
  : "=r"(__ug_StrError__re) \
  : "r"(__ug_StrError__bn), "r"(__ug_StrError_err)  \
  : "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __ug_StrError__re; \
  }); \
  _ug_StrError__re; \
})

#define __getuid() ({ \
  LONG _getuid__re = \
  ({ \
  register struct Library * const __getuid__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  register LONG __getuid__re __asm("d0"); \
  __asm volatile ("jsr %%a6@(-48:W)" \
  : "=r"(__getuid__re) \
  : "r"(__getuid__bn)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __getuid__re; \
  }); \
  _getuid__re; \
})

#define __geteuid() ({ \
  LONG _geteuid__re = \
  ({ \
  register struct Library * const __geteuid__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  register LONG __geteuid__re __asm("d0"); \
  __asm volatile ("jsr %%a6@(-54:W)" \
  : "=r"(__geteuid__re) \
  : "r"(__geteuid__bn)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __geteuid__re; \
  }); \
  _geteuid__re; \
})

#define __setreuid(real, effective) ({ \
  LONG _setreuid_real = (real); \
  LONG _setreuid_effective = (effective); \
  LONG _setreuid__re = \
  ({ \
  register struct Library * const __setreuid__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  register LONG __setreuid__re __asm("d0"); \
  register LONG __setreuid_real __asm("d0") = (_setreuid_real); \
  register LONG __setreuid_effective __asm("d1") = (_setreuid_effective); \
  __asm volatile ("jsr %%a6@(-60:W)" \
  : "=r"(__setreuid__re) \
  : "r"(__setreuid__bn), "r"(__setreuid_real),  "r"(__setreuid_effective)  \
  : "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __setreuid__re; \
  }); \
  _setreuid__re; \
})

#define __setuid(uid) ({ \
  LONG _setuid_uid = (uid); \
  LONG _setuid__re = \
  ({ \
  register struct Library * const __setuid__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  register LONG __setuid__re __asm("d0"); \
  register LONG __setuid_uid __asm("d0") = (_setuid_uid); \
  __asm volatile ("jsr %%a6@(-66:W)" \
  : "=r"(__setuid__re) \
  : "r"(__setuid__bn), "r"(__setuid_uid)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __setuid__re; \
  }); \
  _setuid__re; \
})

#define __getgid() ({ \
  LONG _getgid__re = \
  ({ \
  register struct Library * const __getgid__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  register LONG __getgid__re __asm("d0"); \
  __asm volatile ("jsr %%a6@(-72:W)" \
  : "=r"(__getgid__re) \
  : "r"(__getgid__bn)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __getgid__re; \
  }); \
  _getgid__re; \
})

#define __getegid() ({ \
  LONG _getegid__re = \
  ({ \
  register struct Library * const __getegid__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  register LONG __getegid__re __asm("d0"); \
  __asm volatile ("jsr %%a6@(-78:W)" \
  : "=r"(__getegid__re) \
  : "r"(__getegid__bn)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __getegid__re; \
  }); \
  _getegid__re; \
})

#define __setregid(real, effective) ({ \
  LONG _setregid_real = (real); \
  LONG _setregid_effective = (effective); \
  LONG _setregid__re = \
  ({ \
  register struct Library * const __setregid__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  register LONG __setregid__re __asm("d0"); \
  register LONG __setregid_real __asm("d0") = (_setregid_real); \
  register LONG __setregid_effective __asm("d1") = (_setregid_effective); \
  __asm volatile ("jsr %%a6@(-84:W)" \
  : "=r"(__setregid__re) \
  : "r"(__setregid__bn), "r"(__setregid_real), "r"(__setregid_effective)  \
  : "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __setregid__re; \
  }); \
  _setregid__re; \
})

#define __setgid(gid) ({ \
  LONG _setgid_gid = (gid); \
  LONG _setgid__re = \
  ({ \
  register struct Library * const __setgid__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  register LONG __setgid__re __asm("d0"); \
  register LONG __setgid_gid __asm("d0") = (_setgid_gid); \
  __asm volatile ("jsr %%a6@(-90:W)" \
  : "=r"(__setgid__re) \
  : "r"(__setgid__bn), "r"(__setgid_gid)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __setgid__re; \
  }); \
  _setgid__re; \
})

#define __getgroups(gidsetlen, gidset) ({ \
  LONG _getgroups_gidsetlen = (gidsetlen); \
  LONG * _getgroups_gidset = (gidset); \
  LONG _getgroups__re = \
  ({ \
  register struct Library * const __getgroups__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  register LONG __getgroups__re __asm("d0"); \
  register LONG __getgroups_gidsetlen __asm("d0") = (_getgroups_gidsetlen); \
  register LONG * __getgroups_gidset __asm("a1") = (_getgroups_gidset); \
  __asm volatile ("jsr %%a6@(-96:W)" \
  : "=r"(__getgroups__re) \
  : "r"(__getgroups__bn), "r"(__getgroups_gidsetlen), "r"(__getgroups_gidset)  \
  : "d1", "a0", "fp0", "fp1", "cc", "memory"); \
  __getgroups__re; \
  }); \
  _getgroups__re; \
})

#define __setgroups(gidsetlen, gidset) ({ \
  LONG _setgroups_gidsetlen = (gidsetlen); \
  LONG * _setgroups_gidset = (gidset); \
  LONG _setgroups__re = \
  ({ \
  register struct Library * const __setgroups__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  register LONG __setgroups__re __asm("d0"); \
  register LONG __setgroups_gidsetlen __asm("d0") = (_setgroups_gidsetlen); \
  register LONG * __setgroups_gidset __asm("a1") = (_setgroups_gidset); \
  __asm volatile ("jsr %%a6@(-102:W)" \
  : "=r"(__setgroups__re) \
  : "r"(__setgroups__bn), "r"(__setgroups_gidsetlen), "r"(__setgroups_gidset)  \
  : "d1", "a0", "fp0", "fp1", "cc", "memory"); \
  __setgroups__re; \
  }); \
  _setgroups__re; \
})

#define __initgroups(name, basegid) ({ \
  UBYTE * _initgroups_name = (name); \
  LONG _initgroups_basegid = (basegid); \
  LONG _initgroups__re = \
  ({ \
  register struct Library * const __initgroups__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  register LONG __initgroups__re __asm("d0"); \
  register UBYTE * __initgroups_name __asm("a1") = (_initgroups_name); \
  register LONG __initgroups_basegid __asm("d0") = (_initgroups_basegid); \
  __asm volatile ("jsr %%a6@(-108:W)" \
  : "=r"(__initgroups__re) \
  : "r"(__initgroups__bn), "r"(__initgroups_basegid), "r"(__initgroups_name)  \
  : "d1", "a0", "fp0", "fp1", "cc", "memory"); \
  __initgroups__re; \
  }); \
  _initgroups__re; \
})

#define __getpwnam(login) ({ \
  UBYTE * _getpwnam_login = (login); \
  struct passwd * _getpwnam__re = \
  ({ \
  register struct Library * const __getpwnam__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  register struct passwd * __getpwnam__re __asm("d0"); \
  register UBYTE * __getpwnam_login __asm("a1") = (_getpwnam_login); \
  __asm volatile ("jsr %%a6@(-114:W)" \
  : "=r"(__getpwnam__re) \
  : "r"(__getpwnam__bn), "r"(__getpwnam_login) \
  : "d1", "a0", "fp0", "fp1", "cc", "memory"); \
  __getpwnam__re; \
  }); \
  _getpwnam__re; \
})

#define __getpwuid(uid) ({ \
  LONG _getpwuid_uid = (uid); \
  struct passwd * _getpwuid__re = \
  ({ \
  register struct Library * const __getpwuid__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  register struct passwd * __getpwuid__re __asm("d0"); \
  register LONG __getpwuid_uid __asm("d0") = (_getpwuid_uid); \
  __asm volatile ("jsr %%a6@(-120:W)" \
  : "=r"(__getpwuid__re) \
  : "r"(__getpwuid__bn), "r"(__getpwuid_uid)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __getpwuid__re; \
  }); \
  _getpwuid__re; \
})

#define __setpwent() ({ \
  register struct Library * const __setpwent__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  __asm volatile ("jsr %%a6@(-126:W)" \
  : \
  : "r"(__setpwent__bn)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
})

#define __getpwent() ({ \
  struct passwd * _getpwent__re = \
  ({ \
  register struct Library * const __getpwent__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  register struct passwd * __getpwent__re __asm("d0"); \
  __asm volatile ("jsr %%a6@(-132:W)" \
  : "=r"(__getpwent__re) \
  : "r"(__getpwent__bn)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __getpwent__re; \
  }); \
  _getpwent__re; \
})

#define __endpwent() ({ \
  register struct Library * const __endpwent__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  __asm volatile ("jsr %%a6@(-138:W)" \
  : \
  : "r"(__endpwent__bn)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
})

#define __getgrnam(name) ({ \
  UBYTE * _getgrnam_name = (name); \
  struct group * _getgrnam__re = \
  ({ \
  register struct Library * const __getgrnam__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  register struct group * __getgrnam__re __asm("d0"); \
  register UBYTE * __getgrnam_name __asm("a1") = (_getgrnam_name); \
  __asm volatile ("jsr %%a6@(-144:W)" \
  : "=r"(__getgrnam__re) \
  : "r"(__getgrnam__bn), "r"(__getgrnam_name)  \
  : "d1", "a0", "fp0", "fp1", "cc", "memory"); \
  __getgrnam__re; \
  }); \
  _getgrnam__re; \
})

#define __getgrgid(gid) ({ \
  LONG _getgrgid_gid = (gid); \
  struct group * _getgrgid__re = \
  ({ \
  register struct Library * const __getgrgid__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  register struct group * __getgrgid__re __asm("d0"); \
  register LONG __getgrgid_gid __asm("d0") = (_getgrgid_gid); \
  __asm volatile ("jsr %%a6@(-150:W)" \
  : "=r"(__getgrgid__re) \
  : "r"(__getgrgid__bn), "r"(__getgrgid_gid)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __getgrgid__re; \
  }); \
  _getgrgid__re; \
})

#define __setgrent() ({ \
  register struct Library * const __setgrent__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  __asm volatile ("jsr %%a6@(-156:W)" \
  : \
  : "r"(__setgrent__bn)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
})

#define __getgrent() ({ \
  struct group * _getgrent__re = \
  ({ \
  register struct Library * const __getgrent__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  register struct group * __getgrent__re __asm("d0"); \
  __asm volatile ("jsr %%a6@(-162:W)" \
  : "=r"(__getgrent__re) \
  : "r"(__getgrent__bn)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __getgrent__re; \
  }); \
  _getgrent__re; \
})

#define __endgrent() ({ \
  register struct Library * const __endgrent__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  __asm volatile ("jsr %%a6@(-168:W)" \
  : \
  : "r"(__endgrent__bn)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
})

#define __crypt(key, set) ({ \
  UBYTE * _crypt_key = (key); \
  UBYTE * _crypt_set = (set); \
  UBYTE * _crypt__re = \
  ({ \
  register struct Library * const __crypt__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  register UBYTE * __crypt__re __asm("d0"); \
  register UBYTE * __crypt_key __asm("a0") = (_crypt_key); \
  register UBYTE * __crypt_set __asm("a1") = (_crypt_set); \
  __asm volatile ("jsr %%a6@(-174:W)" \
  : "=r"(__crypt__re) \
  : "r"(__crypt__bn), "r"(__crypt_key), "r"(__crypt_set)  \
  : "d1", "fp0", "fp1", "cc", "memory"); \
  __crypt__re; \
  }); \
  _crypt__re; \
})

#define __ug_GetSalt(user, buf, size) ({ \
  struct passwd * _ug_GetSalt_user = (user); \
  UBYTE * _ug_GetSalt_buf = (buf); \
  ULONG _ug_GetSalt_size = (size); \
  UBYTE * _ug_GetSalt__re = \
  ({ \
  register struct Library * const __ug_GetSalt__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  register UBYTE * __ug_GetSalt__re __asm("d0"); \
  register struct passwd * __ug_GetSalt_user __asm("a0") = (_ug_GetSalt_user); \
  register UBYTE * __ug_GetSalt_buf __asm("a1") = (_ug_GetSalt_buf); \
  register ULONG __ug_GetSalt_size __asm("d0") = (_ug_GetSalt_size); \
  __asm volatile ("jsr %%a6@(-180:W)" \
  : "=r"(__ug_GetSalt__re) \
  : "r"(__ug_GetSalt__bn), "r"(__ug_GetSalt_user), "r"(__ug_GetSalt_buf), "r"(__ug_GetSalt_size)  \
  : "d1", "fp0", "fp1", "cc", "memory"); \
  __ug_GetSalt__re; \
  }); \
  _ug_GetSalt__re; \
})

#define __getpass(prompt) ({ \
  UBYTE * _getpass_prompt = (prompt); \
  UBYTE * _getpass__re = \
  ({ \
  register struct Library * const __getpass__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  register UBYTE * __getpass__re __asm("d0"); \
  register UBYTE * __getpass_prompt __asm("a1") = (_getpass_prompt); \
  __asm volatile ("jsr %%a6@(-186:W)" \
  : "=r"(__getpass__re) \
  : "r"(__getpass__bn), "r"(__getpass_prompt)  \
  : "d1", "a0", "fp0", "fp1", "cc", "memory"); \
  __getpass__re; \
  }); \
  _getpass__re; \
})

#define __umask(mask) ({ \
  ULONG _umask_mask = (mask); \
  ULONG _umask__re = \
  ({ \
  register struct Library * const __umask__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  register ULONG __umask__re __asm("d0"); \
  register ULONG __umask_mask __asm("d0") = (_umask_mask); \
  __asm volatile ("jsr %%a6@(-192:W)" \
  : "=r"(__umask__re) \
  : "r"(__umask__bn), "r"(__umask_mask) \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __umask__re; \
  }); \
  _umask__re; \
})

#define __getumask() ({ \
  UWORD _getumask__re = \
  ({ \
  register struct Library * const __getumask__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  register UWORD __getumask__re __asm("d0"); \
  __asm volatile ("jsr %%a6@(-198:W)" \
  : "=r"(__getumask__re) \
  : "r"(__getumask__bn)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __getumask__re; \
  }); \
  _getumask__re; \
})

#define __setsid() ({ \
  LONG _setsid__re = \
  ({ \
  register struct Library * const __setsid__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  register LONG __setsid__re __asm("d0"); \
  __asm volatile ("jsr %%a6@(-204:W)" \
  : "=r"(__setsid__re) \
  : "r"(__setsid__bn)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __setsid__re; \
  }); \
  _setsid__re; \
})

#define __getpgrp() ({ \
  LONG _getpgrp__re = \
  ({ \
  register struct Library * const __getpgrp__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  register LONG __getpgrp__re __asm("d0"); \
  __asm volatile ("jsr %%a6@(-210:W)" \
  : "=r"(__getpgrp__re) \
  : "r"(__getpgrp__bn)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __getpgrp__re; \
  }); \
  _getpgrp__re; \
})

#define __getlogin() ({ \
  UBYTE * _getlogin__re = \
  ({ \
  register struct Library * const __getlogin__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  register UBYTE * __getlogin__re __asm("d0"); \
  __asm volatile ("jsr %%a6@(-216:W)" \
  : "=r"(__getlogin__re) \
  : "r"(__getlogin__bn)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __getlogin__re; \
  }); \
  _getlogin__re; \
})

#define __setlogin(name) ({ \
  UBYTE * _setlogin_name = (name); \
  LONG _setlogin__re = \
  ({ \
  register struct Library * const __setlogin__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  register LONG __setlogin__re __asm("d0"); \
  register UBYTE * __setlogin_name __asm("a1") = (_setlogin_name); \
  __asm volatile ("jsr %%a6@(-222:W)" \
  : "=r"(__setlogin__re) \
  : "r"(__setlogin__bn), "r"(__setlogin_name)  \
  : "d1", "a0", "fp0", "fp1", "cc", "memory"); \
  __setlogin__re; \
  }); \
  _setlogin__re; \
})

#define __setutent() ({ \
  register struct Library * const __setutent__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  __asm volatile ("jsr %%a6@(-228:W)" \
  : \
  : "r"(__setutent__bn)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
})

#define __getutent() ({ \
  struct utmp * _getutent__re = \
  ({ \
  register struct Library * const __getutent__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  register struct utmp * __getutent__re __asm("d0"); \
  __asm volatile ("jsr %%a6@(-234:W)" \
  : "=r"(__getutent__re) \
  : "r"(__getutent__bn)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __getutent__re; \
  }); \
  _getutent__re; \
})

#define __endutent() ({ \
  register struct Library * const __endutent__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  __asm volatile ("jsr %%a6@(-240:W)" \
  : \
  : "r"(__endutent__bn)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
})

#define __getlastlog(uid) ({ \
  LONG _getlastlog_uid = (uid); \
  struct lastlog * _getlastlog__re = \
  ({ \
  register struct Library * const __getlastlog__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  register struct lastlog * __getlastlog__re __asm("d0"); \
  register LONG __getlastlog_uid __asm("d0") = (_getlastlog_uid); \
  __asm volatile ("jsr %%a6@(-246:W)" \
  : "=r"(__getlastlog__re) \
  : "r"(__getlastlog__bn), "r"(__getlastlog_uid)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __getlastlog__re; \
  }); \
  _getlastlog__re; \
})

#define __setlastlog(uid, name, host) ({ \
  LONG _setlastlog_uid = (uid); \
  UBYTE * _setlastlog_name = (name); \
  UBYTE * _setlastlog_host = (host); \
  LONG _setlastlog__re = \
  ({ \
  register struct Library * const __setlastlog__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  register LONG __setlastlog__re __asm("d0"); \
  register LONG __setlastlog_uid __asm("d0") = (_setlastlog_uid); \
  register UBYTE * __setlastlog_name __asm("a0") = (_setlastlog_name); \
  register UBYTE * __setlastlog_host __asm("a1") = (_setlastlog_host); \
  __asm volatile ("jsr %%a6@(-252:W)" \
  : "=r"(__setlastlog__re) \
  : "r"(__setlastlog__bn), "r"(__setlastlog_uid), "r"(__setlastlog_name), "r"(__setlastlog_host)  \
  : "d1", "fp0", "fp1", "cc", "memory"); \
  __setlastlog__re; \
  }); \
  _setlastlog__re; \
})

#define __getcredentials(task) ({ \
  struct Task * _getcredentials_task = (task); \
  struct UserGroupCredentials * _getcredentials__re = \
  ({ \
  register struct Library * const __getcredentials__bn __asm("a6") = (struct Library *) (__UserGroupBase);\
  register struct UserGroupCredentials * __getcredentials__re __asm("d0"); \
  register struct Task * __getcredentials_task __asm("a0") = (_getcredentials_task); \
  __asm volatile ("jsr %%a6@(-258:W)" \
  : "=r"(__getcredentials__re) \
  : "r"(__getcredentials__bn), "r"(__getcredentials_task)  \
  : "d1", "a1", "fp0", "fp1", "cc", "memory"); \
  __getcredentials__re; \
  }); \
  _getcredentials__re; \
})

#endif /* __amigaos4__ */

#endif /* __GNUC__ */

/****************************************************************************/

#if defined(__SASC)

LONG __ug_SetupContextTagList(UBYTE *name,struct TagItem *tags);
LONG __ug_GetErr(VOID);
UBYTE *__ug_StrError(LONG err);
LONG __getuid(VOID);
LONG __geteuid(VOID);
LONG __setreuid(LONG real,LONG effective);
LONG __setuid(LONG uid);
LONG __getgid(VOID);
LONG __getegid(VOID);
LONG __setregid(LONG real,LONG effective);
LONG __setgid(LONG gid);
LONG __getgroups(LONG gidsetlen,LONG *gidset);
LONG __setgroups(LONG gidsetlen,LONG *gidset);
LONG __initgroups(UBYTE *name,LONG basegid);
struct passwd *__getpwnam(UBYTE *login);
struct passwd *__getpwuid(LONG uid);
VOID __setpwent(VOID);
struct passwd *__getpwent(VOID);
VOID __endpwent(VOID);
struct group *__getgrnam(UBYTE *name);
struct group *__getgrgid(LONG gid);
VOID __setgrent(VOID);
struct group *__getgrent(VOID);
VOID __endgrent(VOID);
UBYTE *__crypt(UBYTE *key,UBYTE *set);
UBYTE *__ug_GetSalt(struct passwd *user,UBYTE *buf,ULONG size);
UBYTE *__getpass(UBYTE *prompt);
UWORD __umask(ULONG mask);
UWORD __getumask(VOID);
LONG __setsid(VOID);
LONG __getpgrp(VOID);
UBYTE *__getlogin(VOID);
LONG __setlogin(UBYTE *name);
VOID __setutent(VOID);
struct utmp *__getutent(VOID);
VOID __endutent(VOID);
struct lastlog *__getlastlog(LONG uid);
LONG __setlastlog(LONG uid,UBYTE *name,UBYTE *host);
struct UserGroupCredentials *__getcredentials(struct Task *task);

#pragma libcall __UserGroupBase __ug_SetupContextTagList 1e 9802
#pragma libcall __UserGroupBase __ug_GetErr 24 00
#pragma libcall __UserGroupBase __ug_StrError 2a 101
#pragma libcall __UserGroupBase __getuid 30 00
#pragma libcall __UserGroupBase __geteuid 36 00
#pragma libcall __UserGroupBase __setreuid 3c 1002
#pragma libcall __UserGroupBase __setuid 42 001
#pragma libcall __UserGroupBase __getgid 48 00
#pragma libcall __UserGroupBase __getegid 4e 00
#pragma libcall __UserGroupBase __setregid 54 1002
#pragma libcall __UserGroupBase __setgid 5a 001
#pragma libcall __UserGroupBase __getgroups 60 9002
#pragma libcall __UserGroupBase __setgroups 66 9002
#pragma libcall __UserGroupBase __initgroups 6c 0902
#pragma libcall __UserGroupBase __getpwnam 72 901
#pragma libcall __UserGroupBase __getpwuid 78 001
#pragma libcall __UserGroupBase __setpwent 7e 00
#pragma libcall __UserGroupBase __getpwent 84 00
#pragma libcall __UserGroupBase __endpwent 8a 00
#pragma libcall __UserGroupBase __getgrnam 90 901
#pragma libcall __UserGroupBase __getgrgid 96 001
#pragma libcall __UserGroupBase __setgrent 9c 00
#pragma libcall __UserGroupBase __getgrent a2 00
#pragma libcall __UserGroupBase __endgrent a8 00
#pragma libcall __UserGroupBase __crypt ae 9802
#pragma libcall __UserGroupBase __ug_GetSalt b4 09803
#pragma libcall __UserGroupBase __getpass ba 901
#pragma libcall __UserGroupBase __umask c0 001
#pragma libcall __UserGroupBase __getumask c6 00
#pragma libcall __UserGroupBase __setsid cc 00
#pragma libcall __UserGroupBase __getpgrp d2 00
#pragma libcall __UserGroupBase __getlogin d8 00
#pragma libcall __UserGroupBase __setlogin de 901
#pragma libcall __UserGroupBase __setutent e4 00
#pragma libcall __UserGroupBase __getutent ea 00
#pragma libcall __UserGroupBase __endutent f0 00
#pragma libcall __UserGroupBase __getlastlog f6 001
#pragma libcall __UserGroupBase __setlastlog fc 98003
#pragma libcall __UserGroupBase __getcredentials 102 801

#endif /* __SASC */

/****************************************************************************/

#endif /* _USERGROUP_HEADERS_H */
