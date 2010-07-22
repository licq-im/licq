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

#include <licq/log.h>
#include <licq/daemon.h>
#include <licq/exceptions/exception.h>
#include <licq/logservice.h>
#include <licq/pluginsignal.h>
#include <licq/thread/mutexlocker.h>

#include <algorithm>
#include <boost/exception/get_error_info.hpp>
#include <boost/foreach.hpp>
#include <cassert>
#include <cctype>
#include <cerrno>
#include <iterator>
#include <glob.h>

// Plugin variables
pthread_cond_t LP_IdSignal = PTHREAD_COND_INITIALIZER;
pthread_mutex_t LP_IdMutex = PTHREAD_MUTEX_INITIALIZER;
std::list<unsigned short> LP_Ids;

using Licq::MutexLocker;
using Licq::StringList;
using Licq::gDaemon;
using Licq::gLog;
using namespace LicqDaemon;
using namespace std;

// Declare global PluginManager (internal for daemon)
LicqDaemon::PluginManager LicqDaemon::gPluginManager;

// Initialize global Licq::PluginManager to refer to the internal PluginManager
Licq::PluginManager& Licq::gPluginManager(LicqDaemon::gPluginManager);


PluginManager::PluginManager() :
  myNextPluginId(1),
  myPluginEventHandler(myGeneralPlugins, myGeneralPluginsMutex,
                       myProtocolPlugins, myProtocolPluginsMutex)
{
  // Empty
}

PluginManager::~PluginManager()
{
  // Empty
}

GeneralPlugin::Ptr PluginManager::loadGeneralPlugin(
    const std::string& name, int argc, char** argv, bool keep)
{
  PluginThread::Ptr pluginThread(new PluginThread());
  DynamicLibrary::Ptr lib = loadPlugin(pluginThread, name, "licq");
  if (!lib)
    return GeneralPlugin::Ptr();

  try
  {
    // Create plugin and resolve all symbols
    GeneralPlugin::Ptr plugin(new GeneralPlugin(lib, pluginThread));

    // Let the plugin initialize itself
    if (!plugin->init(argc, argv))
    {
      gLog.error(tr("Failed to initialize plugin (%s)"),
                 plugin->getName());
      throw std::exception();
    }

    // Give plugin a unique ID
    plugin->setId(myNextPluginId++);

    if (keep)
    {
      MutexLocker locker(myGeneralPluginsMutex);
      myGeneralPlugins.push_back(plugin);
    }
    return plugin;
  }
  catch (const DynamicLibrary::Exception& ex)
  {
    std::string error = ex.getSystemError();
    std::string symbol =
        *boost::get_error_info<Plugin::errinfo_symbol_name>(ex);
    gLog.error(tr("Failed to find %s in plugin (%s): %s"),
               symbol.c_str(), name.c_str(), error.c_str());
  }
  catch (const std::exception&)
  {
    // Empty
  }

  return GeneralPlugin::Ptr();
}

ProtocolPlugin::Ptr PluginManager::
loadProtocolPlugin(const std::string& name, bool keep, bool icq)
{
  PluginThread::Ptr pluginThread(new PluginThread());
  DynamicLibrary::Ptr lib = loadPlugin(pluginThread, name, "protocol");
  if (!lib)
    return ProtocolPlugin::Ptr();

  try
  {
    // Create plugin and resolve all symbols
    ProtocolPlugin::Ptr plugin(new ProtocolPlugin(lib, pluginThread, icq));

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

    if (keep)
      myProtocolPlugins.push_back(plugin);
    locker.unlock();

    // Let the plugins know about the new protocol plugin
    myPluginEventHandler.pushGeneralSignal(
        new Licq::PluginSignal(Licq::PluginSignal::SignalNewProtocol, plugin->getProtocolId()));

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

void PluginManager::startAllPlugins()
{
  MutexLocker generalLocker(myGeneralPluginsMutex);
  MutexLocker protocolLocker(myProtocolPluginsMutex);

  BOOST_FOREACH(GeneralPlugin::Ptr plugin, myGeneralPlugins)
  {
    startPlugin(plugin);
  }

  BOOST_FOREACH(ProtocolPlugin::Ptr plugin, myProtocolPlugins)
  {
    startPlugin(plugin);
  }
}

void PluginManager::shutdownAllPlugins()
{
  {
    MutexLocker locker(myGeneralPluginsMutex);
    BOOST_FOREACH(GeneralPlugin::Ptr plugin, myGeneralPlugins)
    {
      plugin->shutdown();
    }
  }

  {
    MutexLocker locker(myProtocolPluginsMutex);
    BOOST_FOREACH(ProtocolPlugin::Ptr plugin, myProtocolPlugins)
    {
      plugin->shutdown();
    }
  }
}

unsigned short PluginManager::waitForPluginExit(unsigned int timeout)
{
  MutexLocker generalLocker(myGeneralPluginsMutex);
  MutexLocker protocolLocker(myProtocolPluginsMutex);

  if (myGeneralPlugins.empty() && myProtocolPlugins.empty())
    LICQ_THROW(Licq::Exception());

  ::pthread_mutex_lock(&LP_IdMutex);
  protocolLocker.unlock();
  generalLocker.unlock();

  while (LP_Ids.empty())
  {
    if (timeout)
    {
      struct timespec abstime;
      abstime.tv_sec = time(NULL) + timeout;
      abstime.tv_nsec = 0;
      if (::pthread_cond_timedwait(&LP_IdSignal, &LP_IdMutex,
                                   &abstime) == ETIMEDOUT)
      {
        ::pthread_mutex_unlock(&LP_IdMutex);
        LICQ_THROW(Licq::Exception());
      }
    }
    else
      ::pthread_cond_wait(&LP_IdSignal, &LP_IdMutex);
  }

  unsigned short exitId = LP_Ids.front();
  LP_Ids.pop_front();

  generalLocker.relock();
  protocolLocker.relock();
  ::pthread_mutex_unlock(&LP_IdMutex);

  // 0 means daemon
  if (exitId == 0)
    return 0;

  // Check general plugins first
  for (GeneralPluginsList::iterator plugin = myGeneralPlugins.begin();
       plugin != myGeneralPlugins.end(); ++plugin)
  {
    if ((*plugin)->getId() == exitId)
    {
      int result = (*plugin)->joinThread();
      gLog.Info(tr("%sPlugin %s exited with code %d.\n"),
                L_ENDxSTR, (*plugin)->getName(), result);
      myGeneralPlugins.erase(plugin);
      return exitId;
    }
  }

  // Then check protocol plugins
  for (ProtocolPluginsList::iterator plugin = myProtocolPlugins.begin();
       plugin != myProtocolPlugins.end(); ++plugin)
  {
    if ((*plugin)->getId() == exitId)
    {
      int result = (*plugin)->joinThread();
      gLog.Info(tr("%sProtocol plugin %s exited with code %d.\n"),
                L_ENDxSTR, (*plugin)->getName(), result);
      myProtocolPlugins.erase(plugin);
      return exitId;
    }
  }

  gLog.Error(tr("%sInvalid plugin id (%d) in exit signal.\n"),
             L_ERRORxSTR, exitId);
  return Plugin::INVALID_ID;
}

void PluginManager::cancelAllPlugins()
{
  {
    MutexLocker locker(myGeneralPluginsMutex);
    BOOST_FOREACH(GeneralPlugin::Ptr plugin, myGeneralPlugins)
    {
      gLog.Info(tr("%sPlugin %s failed to exit.\n"),
                L_WARNxSTR, plugin->getName());
      plugin->cancelThread();
    }
  }

  {
    MutexLocker locker(myProtocolPluginsMutex);
    BOOST_FOREACH(ProtocolPlugin::Ptr plugin, myProtocolPlugins)
    {
      gLog.Info(tr("%sProtocol plugin %s failed to exit.\n"),
                L_WARNxSTR, plugin->getName());
      plugin->cancelThread();
    }
  }
}

size_t PluginManager::getGeneralPluginsCount() const
{
  MutexLocker locker(myGeneralPluginsMutex);
  return myGeneralPlugins.size();
}

void PluginManager::
getGeneralPluginsList(Licq::GeneralPluginsList& plugins) const
{
  plugins.clear();
  MutexLocker locker(myGeneralPluginsMutex);
  std::copy(myGeneralPlugins.begin(), myGeneralPlugins.end(),
            std::back_inserter(plugins));
}

void PluginManager::
getProtocolPluginsList(Licq::ProtocolPluginsList& plugins) const
{
  plugins.clear();
  MutexLocker locker(myProtocolPluginsMutex);
  std::copy(myProtocolPlugins.begin(), myProtocolPlugins.end(),
            std::back_inserter(plugins));
}

void PluginManager::getAvailableGeneralPlugins(
    StringList& plugins, bool includeLoaded) const
{
  getAvailablePlugins(plugins, "licq");

  if (!includeLoaded)
  {
    MutexLocker locker(myGeneralPluginsMutex);
    BOOST_FOREACH(GeneralPlugin::Ptr plugin, myGeneralPlugins)
    {
      std::string name = plugin->getLibraryName();
      size_t pos = name.find_last_of('/');
      name.erase(0, pos+6);
      name.erase(name.size() - 3);
      plugins.remove(name);
    }
  }
}

void PluginManager::getAvailableProtocolPlugins(
    StringList& plugins, bool includeLoaded) const
{
  getAvailablePlugins(plugins, "protocol");

  if (!includeLoaded)
  {
    MutexLocker locker(myProtocolPluginsMutex);
    BOOST_FOREACH(ProtocolPlugin::Ptr plugin, myProtocolPlugins)
    {
      std::string name = plugin->getLibraryName();
      // Special case, the internal ICQ plugin has no library
      if (name.empty())
        continue;
      size_t pos = name.find_last_of('/');
      name.erase(0, pos+10);
      name.erase(name.size() - 3);
      plugins.remove(name);
    }
  }
}

void PluginManager::getAvailablePlugins(
    StringList& plugins, const std::string& prefix) const
{
  plugins.clear();

  string pattern = gDaemon.libDir() + prefix + "_*.so";
  glob_t globbuf;
  if (glob(pattern.c_str(), 0, NULL, &globbuf) != 0)
    return;

  for (typeof(globbuf.gl_pathc) i = 0; i < globbuf.gl_pathc; ++i)
  {
    std::string name = globbuf.gl_pathv[i];
    size_t pos = name.find_last_of('/');
    name.erase(0, pos + prefix.size() + 2);
    name.erase(name.size() - 3);
    plugins.push_front(name);
  }
  globfree(&globbuf);
}

Licq::ProtocolPlugin::Ptr PluginManager::getProtocolPlugin(
    unsigned long protocolId) const
{
  MutexLocker locker(myProtocolPluginsMutex);
  BOOST_FOREACH(Licq::ProtocolPlugin::Ptr protocol, myProtocolPlugins)
  {
    if (protocol->getProtocolId() == protocolId)
      return protocol;
  }
  return Licq::ProtocolPlugin::Ptr();
}

bool PluginManager::
startGeneralPlugin(const std::string& name, int argc, char** argv)
{
  GeneralPlugin::Ptr plugin = loadGeneralPlugin(name, argc, argv);
  if (plugin)
  {
    MutexLocker locker(myGeneralPluginsMutex);
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
    MutexLocker locker(myProtocolPluginsMutex);
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
  return registerPlugin(myProtocolPlugins, 0xffffffff);
}

void PluginManager::unregisterProtocolPlugin()
{
  MutexLocker locker(myProtocolPluginsMutex);
  unregisterPlugin(myProtocolPlugins);
}

DynamicLibrary::Ptr PluginManager::loadPlugin(
    PluginThread::Ptr pluginThread, const std::string& name,
    const std::string& prefix)
{
  std::string path;
  if (!name.empty() && name[0] != '/' && name[0] != '.')
    path = gDaemon.libDir() + prefix + "_" + name + ".so";
  else
    path = name;

  try
  {
    // Since some libraries do special things when starting, we load them in
    // the same thread as they will later be executed in.
    DynamicLibrary::Ptr lib = pluginThread->loadPlugin(path);
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

// Called in the plugin's thread just before the main entry point
static void startPluginCallback(Plugin& plugin)
{
  std::string name = plugin.getName();
  std::transform(name.begin(), name.end(), name.begin(), ::tolower);
  gDaemon.getLogService().createThreadLog(name);
}

void PluginManager::startPlugin(Plugin::Ptr plugin)
{
  if (dynamic_cast<ProtocolPlugin*>(plugin.get()))
  {
    gLog.Info(tr("%sStarting protocol plugin %s (version %s).\n"), L_INITxSTR,
              plugin->getName(), plugin->getVersion());
  }
  else
  {
    gLog.Info(tr("%sStarting plugin %s (version %s).\n"), L_INITxSTR,
              plugin->getName(), plugin->getVersion());
  }

  plugin->startThread(startPluginCallback);
}
