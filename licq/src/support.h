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


/* Basic support functions */

void SetString(char **szDest, const char *szSource);

int Redirect(const char *);

int strlen_safe(const char *);

int scandir_alpha_r(const char *dir, struct dirent ***namelist,
              int (*select)(const struct dirent *));


/* Cross-platform support functions */

#ifndef HAVE_INET_ATON
int inet_aton(const char *cp, struct in_addr *addr);
#endif  /* HAVE_INET_ATON */

#ifndef HAVE_STRERROR
char *strerror(int errnum);
#endif

int gethostbyname_r_portable(const char *, struct hostent *);


#ifdef __cplusplus
}
#endif

#endif	/* SUPPORT_H */
