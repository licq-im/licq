// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998 - 2003 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <signal.h>
#include <cstdlib>
#include <cstring>

#include <locale>

#include "licq.h"
#include "licq_sighandler.h"

#ifdef USE_SOCKS5
#define SOCKS
#define INCLUDE_PROTOTYPES
extern "C" {
#include <socks.h>
}
#endif

// Localization
#include "gettext.h"

// define a global variable for the base directory containing the data and config subdirectories
char BASE_DIR[MAX_FILENAME_LEN];
char SHARE_DIR[MAX_FILENAME_LEN];
char LIB_DIR[MAX_FILENAME_LEN];
unsigned short DEBUG_LEVEL;


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

  licq_signal_handler();

  CLicq licq;
  if (!licq.Init(argc, argv))
    return 1;
  return licq.Main();
}
