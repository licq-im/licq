/* support.h -- supporting functions for systems missing things we need. */

#ifndef SUPPORT_H
#define SUPPORT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
extern int h_errno;

#include "support.h"


void SetString(char **szDest, const char *szSource)
{
  if (*szDest != NULL) free (*szDest);
  if (szSource == NULL)
    *szDest = strdup("");
  else
    *szDest = strdup(szSource);
}


int Redirect(const char *_szFile)
{
	int fd = open(_szFile, O_WRONLY);
  if (fd == -1) return 0;
	if (dup2(fd, STDOUT_FILENO) == -1) return 0;
	if (dup2(fd, STDERR_FILENO) == -1) return 0;
	return 1;
}


char *inet_ntoa_r(struct in_addr in, char *buf)
{
  register char *p;
  p = (char *)&in;
#define UC(b)   (((int)b)&0xff)
  sprintf(buf, "%d.%d.%d.%d", UC(p[0]), UC(p[1]), UC(p[2]), UC(p[3]));
  return buf;
}



/*------------------------------------------------------------------------------
 * scandir_r
 * rewrite of BSD scandir to be re-entrant
 * arg1 = directory name
 * arg2 = unallocated array of pointers to dirent(direct) strctures
 * arg3 = specifier for which objects to pick (pointer to function)
 * arg4 = sorting function pointer to pass to qsort
 *----------------------------------------------------------------------------*/
int scandir_r(char *dirname, struct dirent *(*namelist[]),
              int (*select)(const struct dirent *),
#ifdef ALPHASORT_VOID
              int (*compar)(const void *, const void *) )
#else
              int (*compar)(const struct dirent *const *, const struct dirent *const *) )
#endif
{
  DIR *dirp;
  struct dirent *result, *entry;
  int tdirsize = sizeof(struct dirent);
  register int i = 0;

  if ((dirp = opendir(dirname)) == NULL)
    return -1;

  /* 256 should be big enough for a spool directory of any size
     big enough for 128 jobs anyway. */
  if ((*namelist = (struct dirent **) calloc(256, sizeof(struct dirent *))) == NULL)
    return -1;


#ifndef HAVE_READDIR_R
#warning Using thread-unsafe readdir function.
  while ( (entry = readdir(dirp)) != NULL)
  {
#else
  if ((entry = (struct dirent *) malloc(tdirsize + _POSIX_PATH_MAX)) == NULL)
    return -1;

  while (readdir_r(dirp, entry, &result) == 0)
  {
    if (result == NULL) break;
#endif
    if (select == NULL || select(entry))
    {
      if (((*namelist)[i] = (struct dirent *)malloc(tdirsize + _POSIX_PATH_MAX)) == NULL)
        return -1;
      memcpy((*namelist)[i], entry, sizeof(entry)+_POSIX_PATH_MAX);
      i++;
    }
  }

  if (compar != NULL)
    qsort((char *) &((*namelist)[0]), i, sizeof(struct dirent *), compar);

  return i;
}


/*=====STRERROR===============================================================*/

#ifndef HAVE_STRERROR

#ifdef HAVE_SYS_ERRLIST
extern char *sys_errlist[];
#endif

char *strerror(int errnum)
{
#ifdef HAVE_SYS_ERRLIST
	return sys_errlist[errnum];
#else
	static char buf[32];

	sprintf(buf, "Unknown error: %u", errnum);

	return buf;
#endif
}

#endif	/* HAVE_STRERROR */

int gethostbyname_r_portable(const char *szHostName, struct hostent *h)
{
// Linux
#if defined(__GLIBC__)
  struct hostent *h_buf;
  char temp[1024];
  int herror = 0;
  gethostbyname_r(szHostName, h, temp, 1024, &h_buf, &herror);
  return herror;
// Solaris
#elif defined(sun)
  struct hostent *h_buf;
  char temp[1024];
  int herror = 0;
  h_buf = gethostbyname_r(szHostName, h, temp, 1024, &herror);
  return herror;
// not sure about this one (actually pretty sure it's wrong)
// who uses OSF anyway?
#elif defined(__osf__)
  h = gethostbyname(szHostName);
  return h_error;
// Default to thread unsafe version
#else
#warning "I don't know how to do reentrant gethostbyname on this machine."
#warning "Using thread-unsafe version."
  struct hostent *h_buf;
  h_buf = gethostbyname(szHostName);
  if (h_buf != NULL) memcpy(h, h_buf, sizeof(struct hostent));
  return (h == NULL ? h_errno : 0);
#endif
}


/*=====ALPHASORT==============================================================*/

#ifndef HAVE_ALPHASORT

int alphasort(const void *a, const void *b)
/*const struct dirent **a, const struct dirent **b)*/
{
  return strcmp( (*(struct dirent **)a)->d_name, (*(struct dirent **)b)->d_name );
}

#endif /* HAVE_ALPHASORT */



/*=====INET_ATON==============================================================*/

/*
 * Copyright (c) 1983, 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)inet_addr.c	8.1 (Berkeley) 6/17/93";
#endif /* LIBC_SCCS and not lint */

/* Changed function declarations to ANSI style. --ML, 25/08/1998 */

#ifndef INADDR_NONE
#define INADDR_NONE (0xffffffff)
#endif /* INADDR_NONE */

/* 
 * Check whether "cp" is a valid ascii representation
 * of an Internet address and convert to a binary address.
 * Returns 1 if the address is valid, 0 if not.
 * This replaces inet_addr, the return value from which
 * cannot distinguish between failure and a local broadcast address.
 */
#ifndef HAVE_INET_ATON
int inet_aton(const char *cp, struct in_addr *addr)
{
	register u_long val;
	register int base, n;
	register char c;
	u_int parts[4];
	register u_int *pp = parts;

	for (;;) {
		/*
		 * Collect number up to ``.''.
		 * Values are specified as for C:
		 * 0x=hex, 0=octal, other=decimal.
		 */
		val = 0; base = 10;
		if (*cp == '0') {
			if (*++cp == 'x' || *cp == 'X')
				base = 16, cp++;
			else
				base = 8;
		}
		while ((c = *cp) != '\0') {
			if (isascii(c) && isdigit(c)) {
				val = (val * base) + (c - '0');
				cp++;
				continue;
			}
			if (base == 16 && isascii(c) && isxdigit(c)) {
				val = (val << 4) + 
					(c + 10 - (islower(c) ? 'a' : 'A'));
				cp++;
				continue;
			}
			break;
		}
		if (*cp == '.') {
			/*
			 * Internet format:
			 *	a.b.c.d
			 *	a.b.c	(with c treated as 16-bits)
			 *	a.b	(with b treated as 24 bits)
			 */
			if (pp >= parts + 3 || val > 0xff)
				return (0);
			*pp++ = val, cp++;
		} else
			break;
	}
	/*
	 * Check for trailing characters.
	 */
	if (*cp && (!isascii(*cp) || !isspace(*cp)))
		return (0);
	/*
	 * Concoct the address according to
	 * the number of parts specified.
	 */
	n = pp - parts + 1;
	switch (n) {

	case 1:				/* a -- 32 bits */
		break;

	case 2:				/* a.b -- 8.24 bits */
		if (val > 0xffffff)
			return (0);
		val |= parts[0] << 24;
		break;

	case 3:				/* a.b.c -- 8.8.16 bits */
		if (val > 0xffff)
			return (0);
		val |= (parts[0] << 24) | (parts[1] << 16);
		break;

	case 4:				/* a.b.c.d -- 8.8.8.8 bits */
		if (val > 0xff)
			return (0);
		val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
		break;
	}
	if (addr)
		addr->s_addr = htonl(val);
	return (1);
}

#endif	/* HAVE_INET_ATON */


#endif	/* SUPPORT_H */
