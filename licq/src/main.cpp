// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998-2010 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

#include "config.h"

#include <signal.h>
#include <cstdlib>
#include <cstring>

#include <locale>

#include "licq.h"

#ifdef USE_SOCKS5
#define SOCKS
#define INCLUDE_PROTOTYPES
extern "C" {
#include <socks.h>
}
#endif

// Localization
#include "gettext.h"

// sighandler.cpp
void licq_install_signal_handlers();

int main(int argc, char **argv)
{
#if ENABLE_NLS
  // prepare daemon localization
  setlocale(LC_ALL, "");
  bindtextdomain(PACKAGE, LOCALEDIR);
  textdomain(PACKAGE);
#endif
  
// Make sure argv[0] is defined otherwise licq will crash if it is NULL
  if (argv[0] == NULL)
    argv[0] = strdup("licq");
#ifdef USE_SOCKS5
   SOCKSinit(argv[0]);
#endif

  licq_install_signal_handlers();

  CLicq licq;
  if (!licq.Init(argc, argv))
    return 1;
  return licq.Main();
}
