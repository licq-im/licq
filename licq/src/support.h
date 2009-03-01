/* support.h -- supporting functions for systems missing things we need. */

#ifndef SUPPORT_H
#define SUPPORT_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <netinet/in.h>
#include <dirent.h>

/* AIX has weird string places */
#ifdef _AIX
#include <strings.h>
#endif


const unsigned short MAX_UIN_DIGITS  = 13;


/* Basic support functions */

void SetString(char **szDest, const char *szSource);

char *ParseDigits(char *szDest, const char *szSource, unsigned int nLen);

char *GetXmlTag(const char *szXmlSource, const char *szTagName);

int UinString(char *_szBuf, size_t _nSize, unsigned long _nUin);

char *PrintHex(char *szPrint, const char *szHex, size_t nSize);
char *ReadHex(char *szHex, const char *szRead, size_t nSize);

int Redirect(const char *);

int strlen_safe(const char *);

int scandir_alpha_r(const char *dir, struct dirent ***namelist,
              int (*select)(const struct dirent *));


/* Cross-platform support functions */

#ifndef HAVE_STRERROR
char *strerror(int errnum);
#endif

int gethostbyname_r_portable(const char *, struct hostent *, char *, size_t);


#ifdef __cplusplus
}
#endif

#endif	/* SUPPORT_H */
