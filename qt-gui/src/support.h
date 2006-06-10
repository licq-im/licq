#ifndef SUPPORT_H
#define SUPPORT_H

/* 
 * Class for Support for LICQ Windows
 *
 * Currently used for Sticky Window Feature
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qwindowdefs.h>

class CSupport {
public:
  static void changeWinSticky(WId win, bool _bStick);
};

#endif

