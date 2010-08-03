/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2010 Licq developers
 *
 * Licq is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Licq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Licq; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/* support.h -- supporting functions for systems missing things we need. */

#ifndef SUPPORT_H
#define SUPPORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"

#include <sys/types.h>
#include <netinet/in.h>
#include <dirent.h>

/* AIX has weird string places */
#ifdef _AIX
#include <strings.h>
#endif


const unsigned short MAX_UIN_DIGITS  = 13;


/* Basic support functions */

char *ParseDigits(char *szDest, const char *szSource, unsigned int nLen);

int UinString(char *_szBuf, size_t _nSize, unsigned long _nUin);

char *PrintHex(char *szPrint, const char *szHex, size_t nSize);
char *ReadHex(char *szHex, const char *szRead, size_t nSize);

int Redirect(const char *);

int strlen_safe(const char *);

int scandir_alpha_r(const char *dir, struct dirent ***namelist,
              int (*select)(const struct dirent *));


/* Cross-platform support functions */

int gethostbyname_r_portable(const char *, struct hostent *, char *, size_t);


#ifdef __cplusplus
}
#endif

#endif	/* SUPPORT_H */
