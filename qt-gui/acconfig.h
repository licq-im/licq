
/* acconfig.h -- autoheader config file
 *
 * Copyright (C) 1998 Graham Roff, Nic Bellamy.
 *
 */

#ifndef _CONFIG_H
#define _CONFIG_H

/* Undefine this if you want to use the old Licq icons and not the
   new dockable ones. */
#define USE_DOCK

/* Undefine this if you want a 64x48 icon, better suited for docking in
   the KDE or Gnome panel. */
#define SIXTYFOUR

/* Change this to MotifStyle if you want the old-school Motif
   widget look. */
#define STYLE "WINDOWS"

/* The name of the package we're compiling */
#undef PACKAGE

/* The version of the package we're compiling */
#undef VERSION

/* Whether or not to include KDE support */
#undef USE_KDE

/* Define this if the system headers don't */
#undef socklen_t

/* Do we have sys_errlist[] available? */
#undef HAVE_SYS_ERRLIST

/* This gets defined if the system has the MIT X11 screensaver extensions */
#undef USE_SCRNSAVER

@TOP@
@BOTTOM@

#endif	/* _CONFIG_H */
