
/* acconfig.h -- autoheader config file
 *
 * Copyright (C) 1998 Graham Roff, Nic Bellamy.
 *
 */

#ifndef _CONFIG_H
#define _CONFIG_H

/* Undefine this if the fifo causes problems */
#define USE_FIFO

/* The name of the package we're compiling */
#undef PACKAGE

/* The version of the package we're compiling */
#undef VERSION

/* The internal Licq version number */
#undef INT_VERSION

/* Whether or not to include SOCKS5 firewall support */
#undef USE_SOCKS5

/* Define this if the system headers don't */
#undef socklen_t

/* Do we have sys_errlist[] available? */
#undef HAVE_SYS_ERRLIST

/* Policy used for dlopen() mode */
#undef DLOPEN_POLICY
@TOP@
@BOTTOM@

#endif	/* _CONFIG_H */
