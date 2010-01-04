/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Erik Johansson <erijo@licq.org>
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

#include "pluginmanager.h"

#include "gettext.h"
#include "licq_events.h"
#include "licq_log.h"
#include "licq/thread/mutexlocker.h"

#include <boost/exception/get_error_info.hpp>
#include <boost/foreach.hpp>
#include <cassert>

using Licq::MutexLocker;
using namespace LicqDaemon;

PluginManager::PluginManager() :
  myDaemon(NULL),
  myNextPluginId(1)
{
  // Empty
}

PluginManager::~PluginManager()
{
  // Empty
}

GeneralPlugin::Ptr PluginManager::loadGeneralPlugin(
    const std::string& name, int argc, char** argv)
{
  DynamicLibrary::Ptr lib = loadPlugin(name, "licq");
  if (!lib)
    return GeneralPlugin::Ptr();

  try
  {
    // Create plugin and resolve all symbols
    GeneralPlugin::Ptr plugin(new GeneralPlugin(lib));

    // Let the plugin initialize itself
    if (!plugin->init(argc, argv))
    {
      gLog.Error(tr("%sFailed to initialize plugin (%s).\n"), L_ERRORxSTR,
                 plugin->getName());
      throw std::exception();
    }

    // Give plugin a unique ID
    plugin->setId(myNextPluginId++);

    MutexLocker locker(myGeneralPluginsMutex);
    myGeneralPlugins.push_back(plugin);
    return plugin;
  }
  catch (const DynamicLibrary::Exception& ex)
  {
    std::string error = ex.getSystemError();
    std::string symbol =
        *boost::get_error_info<Plugin::errinfo_symbol_name>(ex);
    gLog.Error(tr("%sFailed to find %s in plugin (%s): %s\n"),
                  L_ERRORxSTR, symbol.c_str(), name.c_str(), error.c_str());
  }
  catch (const std::exception&)
  {
    // Empty
  }

  return GeneralPlugin::Ptr();
}

ProtocolPlugin::Ptr PluginManager::loadProtocolPlugin(const std::string& name)
{
  DynamicLibrary::Ptr lib = loadPlugin(name, "protocol");
  if (!lib)
    return ProtocolPlugin::Ptr();

  try
  {
    // Create plugin and resolve all symbols
    ProtocolPlugin::Ptr plugin(new ProtocolPlugin(lib));

    // Let the plugin initialize itself
    if (!plugin->init())
    {
      gLog.Error(tr("%sFailed to initialize plugin (%s).\n"), L_ERRORxSTR,
                 plugin->getName());
      throw std::exception();
    }

    // Give plugin a unique ID
    plugin->setId(myNextPluginId++);

    // Check if the plugin is already loaded
    MutexLocker locker(myProtocolPluginsMutex);
    BOOST_FOREACH(ProtocolPlugin::Ptr proto, myProtocolPlugins)
    {
      if (proto->getProtocolId() == plugin->getProtocolId())
        throw std::exception();
    }

    myProtocolPlugins.push_back(plugin);
    return plugin;
  }
  catch (const DynamicLibrary::Exception& ex)
  {
    std::string error = ex.getSystemError();
    std::string symbol =
        *boost::get_error_info<Plugin::errinfo_symbol_name>(ex);
    gLog.Error(tr("%sFailed to find %s in plugin (%s): %s\n"),
                  L_ERRORxSTR, symbol.c_str(), name.c_str(), error.c_str());
  }
  catch (const std::exception&)
  {
    // Empty
  }

  return ProtocolPlugin::Ptr();
}

void PluginManager::startPlugin(Plugin::Ptr plugin)
{
  assert(plugin);

  Licq::Mutex* mutex;
  if (dynamic_cast<ProtocolPlugin*>(plugin.get()))
  {
    mutex = &myProtocolPluginsMutex;
    gLog.Info(tr("%sStarting protocol plugin %s (version %s).\n"), L_INITxSTR,
              plugin->getName(), plugin->getVersion());
  }
  else
  {
    mutex = &myGeneralPluginsMutex;
    gLog.Info(tr("%sStarting plugin %s (version %s).\n"), L_INITxSTR,
              plugin->getName(), plugin->getVersion());
  }

  assert(myDaemon != NULL);
  MutexLocker locker(*mutex);
  plugin->startThread(myDaemon);
}

void PluginManager::getGeneralPluginsList(Licq::GeneralPluginsList& plugins)
{
  plugins.clear();
  MutexLocker locker(myGeneralPluginsMutex);
  std::copy(myGeneralPlugins.begin(), myGeneralPlugins.end(),
            plugins.begin());
}

void PluginManager::getProtocolPluginsList(Licq::ProtocolPluginsList& plugins)
{
  plugins.clear();
  MutexLocker locker(myProtocolPluginsMutex);
  std::copy(myProtocolPlugins.begin(), myProtocolPlugins.end(),
            plugins.begin());
}

bool PluginManager::
startGeneralPlugin(const std::string& name, int argc, char** argv)
{
  GeneralPlugin::Ptr plugin = loadGeneralPlugin(name, argc, argv);
  if (plugin)
  {
    startPlugin(plugin);
    return true;
  }
  return false;
}

bool PluginManager::startProtocolPlugin(const std::string& name)
{
  ProtocolPlugin::Ptr plugin = loadProtocolPlugin(name);
  if (plugin)
  {
    startPlugin(plugin);
    return true;
  }
  return false;
}

template<typename PluginsList>
static int registerPlugin(PluginsList& plugins, unsigned long signalMask)
{
  BOOST_FOREACH(typename PluginsList::value_type plugin, plugins)
  {
    if (plugin->isThisThread())
    {
      plugin->setSignalMask(signalMask);
      return plugin->getReadPipe();
    }
  }

  gLog.Error(tr("%sInvalid thread in registration attempt.\n"),
             L_ERRORxSTR);
  return -1;
}

template<typename PluginsList>
static void unregisterPlugin(PluginsList& plugins)
{
  BOOST_FOREACH(typename PluginsList::value_type plugin, plugins)
  {
    if (plugin->isThisThread())
    {
      plugin->setSignalMask(0);
      return;
    }
  }

  gLog.Error(tr("%sInvalid thread in unregistration attempt.\n"),
             L_ERRORxSTR);
}

int PluginManager::registerGeneralPlugin(unsigned long signalMask)
{
  MutexLocker locker(myGeneralPluginsMutex);
  return registerPlugin(myGeneralPlugins, signalMask);
}

void PluginManager::unregisterGeneralPlugin()
{
  MutexLocker locker(myGeneralPluginsMutex);
  unregisterPlugin(myGeneralPlugins);
}

int PluginManager::registerProtocolPlugin()
{
  MutexLocker locker(myProtocolPluginsMutex);
  return registerPlugin(myProtocolPlugins, SIGNAL_ALL);
}

void PluginManager::unregisterProtocolPlugin()
{
  MutexLocker locker(myProtocolPluginsMutex);
  unregisterPlugin(myProtocolPlugins);
}

DynamicLibrary::Ptr PluginManager::loadPlugin(
    const std::string& name, const std::string& prefix)
{
  std::string path;
  if (!name.empty() && name[0] != '/' && name[0] != '.')
    path = LIB_DIR + prefix + "_" + name + ".so";
  else
    path = name;

  try
  {
    DynamicLibrary::Ptr lib(new DynamicLibrary(path));
    return lib;
  }
  catch (const DynamicLibrary::Exception& ex)
  {
    std::string error = ex.getSystemError();
    gLog.Error(tr("%sUnable to load plugin (%s): %s\n"), L_ERRORxSTR,
               name.c_str(), error.c_str());

    if (error.find("No such file") == std::string::npos)
    {
      gLog.Warn(tr("%sThis usually happens when your plugin\n"
                   "%sis not kept in sync with the daemon.\n"
                   "%sPlease try recompiling the plugin.\n"
                   "%sIf you are still having problems, see\n"
                   "%sthe FAQ at www.licq.org\n"),
                L_WARNxSTR, L_BLANKxSTR, L_BLANKxSTR, L_BLANKxSTR,
                L_BLANKxSTR);
    }
  }

  return DynamicLibrary::Ptr();
}
