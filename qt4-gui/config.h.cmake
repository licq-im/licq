#ifndef CONFIG_H
#define CONFIG_H

/* Licq Qt-GUI version */
#define VERSION "@VERSION_MAJOR@.@VERSION_MINOR@.@VERSION_RELEASE@"

/* Use KDE support */
#cmakedefine USE_KDE 1

/* Use X screensaver support */
#cmakedefine USE_SCRNSAVER 1

/* Existence of some headers */
#cmakedefine HAVE_LOCALE_H 1

/* Define if GPGME is available */
#cmakedefine HAVE_LIBGPGME 1

#endif
