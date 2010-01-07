/* Name of package */
#define PACKAGE "licq"

/* Enable translation of program messages */
#cmakedefine ENABLE_NLS 1

/* Use Licq FIFO */
#cmakedefine USE_FIFO 1

/* Include support for hebrew reverse string */
#cmakedefine USE_HEBREW 1

/* Enable secure communication channels */
#cmakedefine USE_OPENSSL 1

/* Enable socks5 support */
#cmakedefine USE_SOCKS5 1

/* 2nd dlopen parameter */
#define DLOPEN_POLICY @DLOPEN_POLICY@

/* Define as const if the declaration of iconv() needs const. */
#define ICONV_CONST @ICONV_CONST@

/* Define if GPGME is available */
#cmakedefine HAVE_LIBGPGME 1

/* Define if readdir_r function is available */
#cmakedefine HAVE_READDIR_R 1

/* Define if backtrace function is available */
#cmakedefine HAVE_BACKTRACE 1

/* Directory where plugins go */
#define INSTALL_LIBDIR "@Licq_PLUGIN_DIR@/"

/* Directory where shared data go */
#define INSTALL_SHAREDIR "@Licq_SHARE_DIR@/"

/* Directory where translations go */
#define LOCALEDIR "@Licq_LOCALE_DIR@/"
