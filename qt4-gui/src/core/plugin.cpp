/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2011 Licq developers
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

#include "plugin.h"

#include "config.h"
#include "pluginversion.h"

#include <cstdio>
#include <QApplication>
#include <QString>

#ifdef USE_KDE
# include <QInternal>
#include <KDE/KCmdLineArgs>
#include <KDE/KMessage>
#else
# include <QStyleFactory>
#endif

#include <licq/daemon.h>
#include <licq/plugin/generalbase.h>
#include <licq/logging/log.h>
#include <licq/version.h>

#include "core/gui-defines.h"
#include "core/licqgui.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::QtGuiPlugin */

QtGuiPlugin* LicqQtGui::gQtGuiPlugin = NULL;

QtGuiPlugin::QtGuiPlugin(Licq::GeneralPlugin::Params& p)
  : Licq::GeneralPlugin(p),
    myArgc(0),
    myArgv(NULL)
{
  assert(gQtGuiPlugin == NULL);
  gQtGuiPlugin = this;
}

std::string QtGuiPlugin::name() const
{
  return DISPLAY_PLUGIN_NAME;
}

std::string QtGuiPlugin::description() const
{
#ifdef USE_KDE
  return "KDE4 based GUI";
#else
  return "Qt4 based GUI";
#endif
}

std::string QtGuiPlugin::version() const
{
  return PLUGIN_VERSION_STRING;
}

std::string QtGuiPlugin::usage() const
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
      .arg(Licq::gDaemon.baseDir().c_str())
    .arg(QTGUI_DIR)
    .arg(SKINS_DIR)
    .arg(ICONS_DIR)
    .arg(EXTICONS_DIR)
    ;

  return usage.toLatin1().constData();
}

std::string QtGuiPlugin::configFile() const
{
  return QTGUI_CONFIGFILE;
}

bool QtGuiPlugin::init(int argc, char** argv)
{
  for (int i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], "-h") == 0)
    {
      printf("%s\n", usage().c_str());
      return false;
    }
  }

  if (qApp != NULL)
  {
    Licq::gLog.error("A Qt application is already loaded.\n"
        "Remove the plugin from the command line.");
    return false;
  }

  myArgc = argc;
  myArgv = argv;

  return true;
}

int QtGuiPlugin::run()
{
#ifdef USE_KDE
  // Don't use the KDE crash handler (drkonqi).
  setenv("KDE_DEBUG", "true", 0);

  KCmdLineArgs::init(myArgc, myArgv,
                     "licq", "qt4-gui",
      ki18n(DISPLAY_PLUGIN_NAME), PLUGIN_VERSION_STRING);
#endif

  LicqQtGui::LicqGui* licqQtGui = new LicqQtGui::LicqGui(myArgc, myArgv);

  int result = licqQtGui->Run();

  delete licqQtGui;

  myArgc = 0;
  myArgv = NULL;

#ifdef USE_KDE
  // When a KApplication is started a new KMessageBoxMessageHandler (from
  // libkdeui) is created and is installed as the message handler. The call to
  // setMessageHandler() triggers KMessage to create a static object that will
  // delete the handler when libkdecore is unloaded.
  //
  // The problem is that when kde4-gui is unloaded (which happens before Licq
  // exits) only libkdeui is unloaded; something makes libkdecore stay
  // loaded. When Licq exits, all libs are unloaded. Before libkdecore is
  // unloaded, the static objects are deleted. In the destructor of the static
  // object created by KMessage a call to the destructor for
  // KMessageBoxMessageHandler is attempted. But that code is in libkdeui which
  // already has been unloaded, resulting in a crash.
  //
  // The line below avoids that and stops Licq from crashing on exit.
  KMessage::setMessageHandler(0);
#endif

  return result;
}

void QtGuiPlugin::destructor()
{
  delete this;
}


Licq::GeneralPlugin* QtGuiPluginFactory(Licq::GeneralPlugin::Params& p)
{
  return new LicqQtGui::QtGuiPlugin(p);
}

LICQ_GENERAL_PLUGIN_DATA(&QtGuiPluginFactory);
