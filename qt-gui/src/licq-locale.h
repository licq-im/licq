#ifndef _LICQ_LOCALE_H

#define _LICQ_LOCALE_H 1
#include <locale.h>

#ifdef ENABLE_NLS
#    include "libintl.h"
#    define _(String) QString::fromLocal8Bit(gettext (String))
#    ifdef gettext_noop
#        define N_(String) gettext_noop (String)
#        define N_(String) QString::fromLocal8Bit(gettext_noop (String))
#    else
#        define N_(String) (String)
#    endif
#else
/* Stubs that do something close enough.  */
#    define textdomain(String) (String)
#    define gettext(String) (String)
#    define dgettext(Domain,Message) (Message)
#    define dcgettext(Domain,Message,Type) (Message)
#    define bindtextdomain(Domain,Directory) (Domain)
#    define _(String) (String)
#    define N_(String) (String)
#endif

#endif
