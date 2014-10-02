/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2013 Licq Developers <licq-dev@googlegroups.com>
 *
 * Please refer to the COPYRIGHT file distributed with this source
 * distribution for the names of the individual contributors.
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

#include <licq/daemon.h>
#include <licq/plugin/generalpluginfactory.h>
#include <licq/version.h>

#include "gui-defines.h"
#include "plugin.h"
#include "pluginversion.h"

namespace LicqQtGui
{

class Factory : public Licq::GeneralPluginFactory
{
public:
  // From Licq::PluginFactory
  std::string name() const { return DISPLAY_PLUGIN_NAME; }
  std::string version() const { return PLUGIN_VERSION_STRING; }
  void destroyPlugin(Licq::PluginInterface* plugin) { delete plugin; }

  // From Licq::GeneralPluginFactory
  std::string description() const;
  std::string usage() const;
  std::string configFile() const;
  Licq::GeneralPluginInterface* createPlugin();
};

std::string Factory::description() const
{
#ifdef USE_KDE
  return "KDE4 based GUI";
#else
  return "Qt4 based GUI";
#endif
}

std::string Factory::usage() const
{
  const std::string qtGuiDir = Licq::gDaemon.baseDir() + QTGUI_DIR;
  const std::string usage =
      "Usage: licq [options] -p " PLUGIN_NAME
      " -- [-dD] [-s skinname] [-i iconpack] [-e extendediconpack]\n"
      " -d : start hidden (dock icon only)\n"
      " -D : disable dock icon for this session"
      " (does not affect dock icon settings)\n"
      " -s : set the skin to use (must be in " + qtGuiDir + SKINS_DIR ")\n"
      " -i : set the icons to use (must be in " + qtGuiDir + ICONS_DIR ")\n"
      " -e : set the extended icons to use (must be in " + qtGuiDir +
      EXTICONS_DIR ")";
  return usage;
}

std::string Factory::configFile() const
{
  return QTGUI_CONFIGFILE;
}

Licq::GeneralPluginInterface* Factory::createPlugin()
{
  return new QtGuiPlugin;
}

} // namespace LicqQtGui

static Licq::GeneralPluginFactory* createFactory()
{
  static LicqQtGui::Factory factory;
  return &factory;
}

static void destroyFactory(Licq::GeneralPluginFactory*)
{
  // Empty
}

LICQ_GENERAL_PLUGIN_DATA(&createFactory, &destroyFactory);
