/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998-2011 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

/* support.h -- supporting functions for systems missing things we need. */

#ifndef SUPPORT_H
#define SUPPORT_H

#include "config.h"

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
#include <errno.h>
#include <pthread.h>

#include "support.h"


char *ParseDigits(char *szDest, const char *szSource, unsigned int nLen)
{
  if (szDest == NULL || szSource == NULL)
    return szDest;
  unsigned int n = 0;
  char *szCur = szDest;

  while ((*szSource) && (n < nLen))
  {
    if (isdigit(*szSource))
    {
      *szCur++ = *szSource++;
      n++;
    } else
      szSource++;
  }
  *szCur = '\0';
  
  return szDest;
}

int Redirect(const char *_szFile)
{
  int fd = open(_szFile, O_WRONLY | O_CREAT | O_APPEND, 00660);
  if (fd == -1) return 0;
  if (dup2(fd, STDERR_FILENO) == -1) return 0;
  return 1;
}


int strlen_safe(const char *sz)
{
  return sz == NULL ? 0 : strlen(sz);
}

/*=====ALPHASORT==============================================================*/
int my_alphasort(const void *a, const void *b)
/*const struct dirent **a, const struct dirent **b)*/
{
  return strcmp( (*(const struct dirent* const*)a)->d_name, (*(const struct dirent* const*)b)->d_name );
}



/*------------------------------------------------------------------------------
 * scandir_r
 * rewrite of BSD scandir to be re-entrant
 * arg1 = directory name
 * arg2 = unallocated array of pointers to dirent(direct) strctures
 * arg3 = specifier for which objects to pick (pointer to function)
 * arg4 = sorting function pointer to pass to qsort
 *----------------------------------------------------------------------------*/
int scandir_alpha_r(char *dirname, struct dirent *(*namelist[]),
              int (*select)(const struct dirent *))
{
  if (namelist == NULL)
    return -1;

  DIR *dirp;
  struct dirent *result, *entry;
  int tdirsize = sizeof(struct dirent);
  register int i = 0, j;

  if ((dirp = opendir(dirname)) == NULL)
    return -1;

  /* 256 should be big enough for a spool directory of any size
     big enough for 128 jobs anyway. */
  if ((*namelist = (struct dirent **) calloc(256, sizeof(struct dirent *))) == NULL)
  {
    closedir(dirp);
    return -1;
  }


#ifndef HAVE_READDIR_R
#warning Using thread-unsafe readdir function.
  while ( (entry = readdir(dirp)) != NULL)
  {
#else
  if ((entry = (struct dirent *) malloc(tdirsize + _POSIX_PATH_MAX)) == NULL)
  {
    closedir(dirp);
    return -1;
  }

  while (readdir_r(dirp, entry, &result) == 0)
  {
    if (result == NULL) break;
#endif
    if (select == NULL || select(entry))
    {

      if (((*namelist)[i] = (struct dirent *)malloc(tdirsize + _POSIX_PATH_MAX)) == NULL)
      {
#ifdef HAVE_READDIR_R
        free(entry);
#endif
        for (j = 0; j < i; j++) free((*namelist)[j]);
        free(*namelist);

        closedir(dirp);
        return -1;
      }
      memcpy((*namelist)[i], entry, sizeof(entry)+_POSIX_PATH_MAX);
      i++;
    }
  }

#ifdef HAVE_READDIR_R
  free(entry);
#endif

  qsort((char *) &((*namelist)[0]), i, sizeof(struct dirent *), my_alphasort);

  closedir(dirp);
  return i;
}


int gethostbyname_r_portable(const char *szHostName, struct hostent *h, char *buf, size_t buflen)
{
// Linux
#if defined(__GLIBC__)
  struct hostent *h_buf;
  int herror = 0;
  int retval = gethostbyname_r(szHostName, h, buf, buflen, &h_buf, &herror);
  return ((retval == 0 && h_buf != NULL) ? 0 /* success */ : herror);
// Solaris, Irix
#elif defined(sun) || defined(__sgi)
  struct hostent *h_buf;
  int herror = 0;
  h_buf = gethostbyname_r(szHostName, h, buf, buflen, &herror);
  return herror;
// Default to mutex version
#else
  (void)buf;
  (void)buflen;

  static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_lock(&mutex);

  struct hostent *h_buf;
  h_buf = gethostbyname(szHostName);
  if (h_buf != NULL)
    memcpy(h, h_buf, sizeof(struct hostent));
  int retval = (h_buf == NULL ? h_errno : 0);

  pthread_mutex_unlock(&mutex);
  return retval;
#endif
}

#if defined(__APPLE__) && defined(__amd64__)
#define LIBICONV_PLUG 1
#include <iconv.h>
// The following symbols are not defined on 64-bit OS X but are needed by
// libintl. Taken in part from
// http://opensource.apple.com/source/libiconv/libiconv-26/patches/unix03.patch
iconv_t libiconv_open(const char* tocode, const char* fromcode)
{
  return iconv_open(tocode, fromcode);
}

size_t libiconv(iconv_t cd, const char** inbuf, size_t* inbytesleft,
		char** outbuf, size_t* outbytesleft)
{
  return iconv(cd, (char **)inbuf, inbytesleft, outbuf, outbytesleft);
}

int libiconv_close(iconv_t cd)
{
  return iconv_close(cd);
}
#endif

#endif	/* SUPPORT_H */
