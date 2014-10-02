/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2011, 2013 Licq developers
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

#include "plugin.h"
#include "pluginversion.h"

#include <cstdio>
#include <QApplication>
#include <QMetaType>
#include <QString>

#ifdef USE_KDE
# include <QInternal>
#include <KDE/KCmdLineArgs>
#include <KDE/KMessage>
#else
# include <QStyleFactory>
#endif

#include <licq/event.h>
#include <licq/logging/log.h>
#include <licq/pluginsignal.h>
#include <licq/version.h>

#include "core/gui-defines.h"
#include "core/licqgui.h"

Q_DECLARE_METATYPE(boost::shared_ptr<const Licq::PluginSignal>);
Q_DECLARE_METATYPE(boost::shared_ptr<const Licq::Event>);

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::QtGuiPlugin */

QtGuiPlugin* LicqQtGui::gQtGuiPlugin = NULL;

QtGuiPlugin::QtGuiPlugin()
  : myArgc(0),
    myArgv(NULL)
{
  assert(gQtGuiPlugin == NULL);
  gQtGuiPlugin = this;

  qRegisterMetaType< boost::shared_ptr<const Licq::PluginSignal> >();
  qRegisterMetaType< boost::shared_ptr<const Licq::Event> >();
}

bool QtGuiPlugin::init(int argc, char** argv)
{
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

void QtGuiPlugin::shutdown()
{
  emit pluginShutdown();
}

bool QtGuiPlugin::isEnabled() const
{
  // Always enabled
  return true;
}

void QtGuiPlugin::enable()
{
  // Always enabled
}

void QtGuiPlugin::disable()
{
  // Always enabled
}

bool QtGuiPlugin::wantSignal(unsigned long signalType) const
{
  return (signalType & Licq::PluginSignal::SignalAll) != 0;
}

void QtGuiPlugin::pushSignal(
    boost::shared_ptr<const Licq::PluginSignal> signal)
{
  emit pluginSignal(signal);
}

void QtGuiPlugin::pushEvent(boost::shared_ptr<const Licq::Event> event)
{
  emit pluginEvent(event);
}
