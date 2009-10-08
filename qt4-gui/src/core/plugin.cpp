// -*- c-basic-offset: 2; -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2009 Licq developers
 *
 * Licq is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Licq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Licq; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"

#include <QApplication>
#include <QString>

#ifdef USE_KDE
# include <QInternal>
#include <KDE/KCmdLineArgs>
#else
# include <QStyleFactory>
#endif

#include <licq_log.h>
#include <licq_plugin.h>

#include "core/gui-defines.h"
#include "core/licqgui.h"

const char* LP_Name()
{
#ifdef USE_KDE
  static const char name[] = "KDE4 GUI";
#else
  static const char name[] = "Qt4 GUI";
#endif
  return name;
}

const char* LP_Description()
{
  static const char desc[] = "Qt4 based GUI";
  return desc;
}

const char* LP_Version()
{
  static const char version[] = VERSION;
  return version;
}

const char* LP_Usage()
{
  static QString usage = QString(
    "Usage:  Licq [options] -p %1 -- [-hdD] [-s skinname] [-i iconpack] [-e extendediconpack]"
    "\n"
    " -h : this help screen\n"
    " -d : start hidden (dock icon only)\n"
    " -D : disable dock icon for this session (does not affect dock icon settings)\n"
    " -s : set the skin to use (must be in %2%3%4)\n"
    " -i : set the icons to use (must be in %2%3%5)\n"
    " -e : set the extended icons to use (must be in %2%3%6)"
    )
    .arg(PLUGIN_NAME)
    .arg(BASE_DIR)
    .arg(QTGUI_DIR)
    .arg(SKINS_DIR)
    .arg(ICONS_DIR)
    .arg(EXTICONS_DIR)
    ;

  return usage.toLatin1().constData();
}

const char* LP_ConfigFile()
{
  static const char file[] = QTGUI_CONFIGFILE;
  return file;
}

const char* LP_Status()
{
  static const char status[] = "running";
  return status;
}

static int myArgc = 0;
static char** myArgv = NULL;

bool LP_Init(int argc, char** argv)
{
  for (int i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], "-h") == 0)
    {
      printf("%s\n", LP_Usage());
      return false;
    }
  }

  if (qApp != NULL)
  {
    gLog.Error("%sA Qt application is already loaded.\n"
        "%sRemove the plugin from the command line.\n",
        L_ERRORxSTR, L_BLANKxSTR);
    return false;
  }

  myArgc = argc;
  myArgv = argv;

  return true;
}

int LP_Main(CICQDaemon* daemon)
{
#ifdef USE_KDE
  // Since plugin is loaded in Licq daemon before thread is spawned, any code
  // executed during library init is run as the daemon thread. KDE library
  // init functions calls Qt functions causing theMainThread in
  // QCoreApplication to be set to daemon thread. When we later get called here
  // with our own thread Qt will complain that we're not running from the main
  // thread.
  // This is a hack using the undocumented call in QInternal to switch main
  // thread in QCoreApplication.
  QInternal::callFunction(QInternal::SetCurrentThreadToMainThread, NULL);

  // Don't use the KDE crash handler (drkonqi).
  setenv("KDE_DEBUG", "true", 0);

  KCmdLineArgs::init(myArgc, myArgv,
                     "licq", "qt4-gui",
                     ki18n(LP_Name()), VERSION);
#endif

  LicqQtGui::LicqGui* licqQtGui = new LicqQtGui::LicqGui(myArgc, myArgv);

  int result = licqQtGui->Run(daemon);

  delete licqQtGui;

  myArgc = 0;
  myArgv = NULL;

  return result;
}
