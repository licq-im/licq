
/* acconfig.h -- autoheader config file
 *
 * Copyright (C) 1998 Graham Roff, Nic Bellamy.
 *
 */

#ifndef _CONFIG_H
#define _CONFIG_H

/* Undefine this if the fifo causes problems */
#define USE_FIFO

/* Undefine this if the compile complains about scandir_r and alphasort */
#define ALPHASORT_VOID

/* The name of the package we're compiling */
#undef PACKAGE

/* The version of the package we're compiling */
#undef VERSION

/* The internal Licq version number */
#undef INT_VERSION

/* Whether or not to include SOCKS5 firewall support */
#undef USE_SOCKS5

/* Whether or not to include OpenSSL encryption support */
#undef USE_OPENSSL

/* Define this if hebrew reverse string support is desired */
#undef USE_HEBREW

/* Define this if the system headers don't */
#undef socklen_t

/* Do we have sys_errlist[] available? */
#undef HAVE_SYS_ERRLIST

/* Policy used for dlopen() mode */
#undef DLOPEN_POLICY
@TOP@
@BOTTOM@

/* define this if your struct tm has a tm_gmtoff field */
#undef USE_GMTOFF

/* define this if your system has a global extern long timezone */
#undef USE_TIMEZONE

#endif	/* _CONFIG_H */
