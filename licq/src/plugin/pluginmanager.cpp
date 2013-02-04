/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2013 Licq developers <licq-dev@googlegroups.com>
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

#include <licq/logging/log.h>
#include <licq/event.h>
#include <licq/exceptions/exception.h>
#include <licq/logging/logservice.h>
#include <licq/plugin/generalplugininterface.h>
#include <licq/plugin/protocolplugininterface.h>
#include <licq/pluginsignal.h>
#include <licq/protocolsignal.h>
#include <licq/thread/mutexlocker.h>
#include <licq/version.h>

#include <algorithm>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <cassert>
#include <cctype>
#include <cerrno>
#include <iterator>
#include <glob.h>

#include "../contactlist/usermanager.h"
#include "../daemon.h"
#include "../utils/dynamiclibrary.h"
#include "generalplugin.h"
#include "plugin.h"
#include "pluginthread.h"
#include "protocolplugin.h"

using Licq::MutexLocker;
using Licq::Owner;
using Licq::StringList;
using Licq::User;
using Licq::UserId;
using Licq::gLog;
using Licq::gLogService;
using namespace LicqDaemon;
using namespace std;

// Declare global PluginManager (internal for daemon)
LicqDaemon::PluginManager LicqDaemon::gPluginManager;

// Initialize global Licq::PluginManager to refer to the internal PluginManager
Licq::PluginManager& Licq::gPluginManager(LicqDaemon::gPluginManager);

namespace
{

// Called in the plugin's thread just before the init function
static void initPluginCallback(const Plugin& plugin)
{
  string name = plugin.name();
  std::transform(name.begin(), name.end(), name.begin(), ::tolower);
  gLogService.createThreadLog(name);
}

static void exitPluginCallback(const Plugin& plugin)
{
  gPluginManager.pluginHasExited(plugin.id());
}

} // namespace

PluginManager::PluginManager() :
  myNextPluginId(1)
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
  PluginThread::Ptr pluginThread;
  if (myGuiThread && name.find("-gui") != std::string::npos)
  {
    gLog.debug(tr("Running %s in GUI thread"), name.c_str());
    pluginThread.swap(myGuiThread);
  }
  else
    pluginThread = boost::make_shared<PluginThread>();

  DynamicLibrary::Ptr lib = loadPlugin(pluginThread, name, "licq");
  if (!lib)
    return GeneralPlugin::Ptr();

  try
  {
    // Get plugin data from library
    Licq::GeneralPluginData* pluginData;
    lib->getSymbol("LicqGeneralPluginData", &pluginData);

    // Verify plugin data
    if (pluginData == NULL)
    {
      gLog.error(tr("Library %s does not contain a Licq plugin"), name.c_str());
      return GeneralPlugin::Ptr();
    }

    if (!verifyPluginMagic(name, pluginData->magic)
        || !verifyPluginVersion(name, pluginData->licqVersion))
      return GeneralPlugin::Ptr();

    // Create the plugin
    GeneralPlugin::Ptr plugin = boost::make_shared<GeneralPlugin>(
        getNewPluginId(), lib, pluginThread, pluginData->pluginFactory);

    // Let the plugin initialize itself
    if (!plugin->init(argc, argv, &initPluginCallback))
    {
      gLog.error(tr("Failed to initialize plugin (%s)"),
          plugin->name().c_str());
      throw std::exception();
    }

    if (keep)
    {
      MutexLocker generalLocker(myGeneralPluginsMutex);
      myGeneralPlugins.push_back(plugin);
    }
    return plugin;
  }
  catch (const DynamicLibrary::Exception& ex)
  {
    std::string error = ex.getSystemError();
    gLog.error(tr("Failed to load plugin (%s): %s"),
        name.c_str(), error.c_str());
  }
  catch (const std::exception&)
  {
    // Empty
  }

  return GeneralPlugin::Ptr();
}

ProtocolPlugin::Ptr PluginManager::
loadProtocolPlugin(const std::string& name, bool keep)
{
  PluginThread::Ptr pluginThread = boost::make_shared<PluginThread>();
  DynamicLibrary::Ptr lib = loadPlugin(pluginThread, name, "protocol");
  if (!lib)
    return ProtocolPlugin::Ptr();

  try
  {
    // Get plugin data from library
    Licq::ProtocolPluginData* pluginData;
    lib->getSymbol("LicqProtocolPluginData", &pluginData);

    // Verify plugin data
    if (pluginData == NULL)
    {
      gLog.error(tr("Library %s does not contain a Licq plugin"), name.c_str());
      return ProtocolPlugin::Ptr();
    }

    if (!verifyPluginMagic(name, pluginData->magic)
        || !verifyPluginVersion(name, pluginData->licqVersion))
      return ProtocolPlugin::Ptr();

    // Create the plugin
    ProtocolPlugin::Ptr plugin = boost::make_shared<ProtocolPlugin>(
        getNewPluginId(), lib, pluginThread, pluginData->pluginFactory);

    {
      // Check if we already got a plugin for this protocol
      MutexLocker protocolLocker(myProtocolPluginsMutex);
      BOOST_FOREACH(ProtocolPlugin::Ptr proto, myProtocolPlugins)
      {
        if (proto->protocolId() == plugin->protocolId())
          throw std::exception();
      }
    }

    // Let the plugin initialize itself
    if (!plugin->init(0, NULL, &initPluginCallback))
    {
      gLog.error(tr("Failed to initialize plugin (%s)"),
          plugin->name().c_str());
      throw std::exception();
    }

    if (keep)
    {
      MutexLocker protocolLocker(myProtocolPluginsMutex);
      myProtocolPlugins.push_back(plugin);
    }

    // Let the plugins know about the new protocol plugin
    pushPluginSignal(new Licq::PluginSignal(
        Licq::PluginSignal::SignalNewProtocol, plugin->protocolId()));

    return plugin;
  }
  catch (const DynamicLibrary::Exception& ex)
  {
    std::string error = ex.getSystemError();
    gLog.error(tr("Failed to load plugin (%s): %s"),
        name.c_str(), error.c_str());
  }
  catch (const std::exception&)
  {
    // Empty
  }

  return ProtocolPlugin::Ptr();
}

void PluginManager::startAllPlugins()
{
  list<unsigned long> ppids;
  {
    MutexLocker protocolLocker(myProtocolPluginsMutex);
    BOOST_FOREACH(ProtocolPlugin::Ptr plugin, myProtocolPlugins)
      ppids.push_back(plugin->protocolId());
  }

  // Must call loadProtocol without holding mutex
  BOOST_FOREACH(unsigned long protocolId, ppids)
    gUserManager.loadProtocol(protocolId);

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

void PluginManager::pluginHasExited(unsigned short id)
{
  MutexLocker locker(myExitListMutex);
  myExitList.push(id);
  gDaemon.notifyPluginExited();
}

void PluginManager::reapPlugin()
{
  unsigned short exitId = myExitList.front();
  myExitList.pop();

  MutexLocker generalLocker(myGeneralPluginsMutex);
  MutexLocker protocolLocker(myProtocolPluginsMutex);

  // Check general plugins first
  for (list<GeneralPlugin::Ptr>::iterator plugin = myGeneralPlugins.begin();
       plugin != myGeneralPlugins.end(); ++plugin)
  {
    if ((*plugin)->id() == exitId)
    {
      int result = (*plugin)->joinThread();
      gLog.info(tr("Plugin %s exited with code %d"),
          (*plugin)->name().c_str(), result);
      myGeneralPlugins.erase(plugin);
      return;
    }
  }
  generalLocker.unlock();

  // Then check protocol plugins
  for (list<ProtocolPlugin::Ptr>::iterator plugin = myProtocolPlugins.begin();
       plugin != myProtocolPlugins.end(); ++plugin)
  {
    if ((*plugin)->id() == exitId)
    {
      unsigned long protocolId = (*plugin)->protocolId();
      int result = (*plugin)->joinThread();
      gLog.info(tr("Protocol plugin %s exited with code %d"),
          (*plugin)->name().c_str(), result);

      // Should already been done, but if protocol exited by itself clean it up here
      gUserManager.unloadProtocol(protocolId);

      // Remove plugin from list, if this was only reference it will unload library
      myProtocolPlugins.erase(plugin);

      // Notify plugins about the removed protocol
      pushPluginSignal(new Licq::PluginSignal(
          Licq::PluginSignal::SignalRemoveProtocol, protocolId));

      return;
    }
  }

  gLog.error(tr("Invalid plugin id (%d) in exit signal"), exitId);
}

void PluginManager::cancelAllPlugins()
{
  {
    MutexLocker locker(myGeneralPluginsMutex);
    BOOST_FOREACH(GeneralPlugin::Ptr plugin, myGeneralPlugins)
    {
      gLog.warning(tr("Plugin %s failed to exit"), plugin->name().c_str());
      plugin->cancelThread();
    }
  }

  {
    MutexLocker locker(myProtocolPluginsMutex);
    BOOST_FOREACH(ProtocolPlugin::Ptr plugin, myProtocolPlugins)
    {
      gLog.warning(tr("Protocol plugin %s failed to exit"), plugin->name().c_str());
      plugin->cancelThread();
    }
  }
}

size_t PluginManager::getGeneralPluginsCount() const
{
  MutexLocker locker(myGeneralPluginsMutex);
  return myGeneralPlugins.size();
}

size_t PluginManager::pluginCount() const
{
  MutexLocker generalLocker(myGeneralPluginsMutex);
  MutexLocker protocolLocker(myProtocolPluginsMutex);
  return myGeneralPlugins.size() + myProtocolPlugins.size();
}

User* PluginManager::createProtocolUser(const UserId& id, bool temporary)
{
  ProtocolPlugin::Ptr plugin = boost::dynamic_pointer_cast<ProtocolPlugin>(
      getProtocolPlugin(id.protocolId()));
  if (!plugin)
    return NULL;

  return plugin->createUser(id, temporary);
}

Owner* PluginManager::createProtocolOwner(const UserId& id)
{
  ProtocolPlugin::Ptr plugin = boost::dynamic_pointer_cast<ProtocolPlugin>(
      getProtocolPlugin(id.protocolId()));
  if (!plugin)
    return NULL;

  return plugin->createOwner(id);
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
      string name = plugin->libraryName();
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
      string name = plugin->libraryName();
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
  BOOST_FOREACH(ProtocolPlugin::Ptr protocol, myProtocolPlugins)
  {
    if (protocol->protocolId() == protocolId)
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
    // Load contacts and owners for the new protocol
    gUserManager.loadProtocol(plugin->protocolId());

    MutexLocker locker(myProtocolPluginsMutex);
    startPlugin(plugin);
    return true;
  }
  return false;
}

void PluginManager::unloadGeneralPlugin(Licq::GeneralPlugin::Ptr p)
{
  GeneralPlugin::Ptr plugin = boost::dynamic_pointer_cast<GeneralPlugin>(p);
  if (!plugin)
    return;

  plugin->shutdown();
}

void PluginManager::unloadProtocolPlugin(Licq::ProtocolPlugin::Ptr p)
{
  ProtocolPlugin::Ptr plugin = boost::dynamic_pointer_cast<ProtocolPlugin>(p);
  if (!plugin)
    return;

  // Check with user manager first if unloading is allowed
  if (!gUserManager.allowUnloadProtocol(plugin->protocolId()))
    return;

  gUserManager.unloadProtocol(plugin->protocolId());
  plugin->shutdown();
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
    gLog.error(tr("Unable to load plugin (%s): %s"),
               name.c_str(), error.c_str());

    if (error.find("No such file") == std::string::npos)
    {
      gLog.warning(tr("This usually happens when your plugin\n"
                      "is not kept in sync with the daemon.\n"
                      "Please try recompiling the plugin.\n"
                      "If you are still having problems, see\n"
                      "the FAQ at www.licq.org"));
    }
  }

  return DynamicLibrary::Ptr();
}

bool PluginManager::verifyPluginMagic(const std::string& name, char magic[4])
{
  const char expected[4] = { 'L', 'i', 'c', 'q' };

  for (size_t i = 0; i < sizeof(expected); ++i)
  {
    if (magic[i] != expected[i])
    {
      gLog.error(tr("Library %s does not contain a valid Licq plugin"),
                 name.c_str());
      return false;
    }
  }
  return true;
}

bool PluginManager::verifyPluginVersion(const std::string& name, int version)
{
  // We expect plugin API to stay the same between releases of the same
  // major/minor version.
  const unsigned int major = Licq::extractMajorVersion(version);
  const unsigned int minor = Licq::extractMinorVersion(version);
  if (major != LICQ_VERSION_MAJOR || minor != LICQ_VERSION_MINOR)
  {
    gLog.error(tr("Plugin in library %s was built for another Licq version "
                  "(%u.%u.%u)"), name.c_str(),
               major, minor, Licq::extractReleaseVersion(version));
    return false;
  }

  return true;
}

int PluginManager::getNewPluginId()
{
  // Lock both plugin mutexes to avoid race for myNextPluginId
  MutexLocker generalLocker(myGeneralPluginsMutex);
  MutexLocker protocolLocker(myProtocolPluginsMutex);
  return myNextPluginId++;
}

void PluginManager::startPlugin(GeneralPlugin::Ptr plugin)
{
  gLog.info(tr("Starting plugin %s (version %s)"),
      plugin->name().c_str(), plugin->version().c_str());

  plugin->run(NULL, exitPluginCallback);
}

void PluginManager::startPlugin(ProtocolPlugin::Ptr plugin)
{
  gLog.info(tr("Starting protocol plugin %s (version %s)"),
      plugin->name().c_str(), plugin->version().c_str());

  plugin->run(NULL, exitPluginCallback);
}

void PluginManager::pushPluginEvent(Licq::Event* rawEvent)
{
  boost::shared_ptr<Licq::Event> event(rawEvent);

  MutexLocker locker(myGeneralPluginsMutex);
  BOOST_FOREACH(GeneralPlugin::Ptr plugin, myGeneralPlugins)
  {
    if (plugin->isThread(event->thread_plugin))
    {
      plugin->pushEvent(event);
      return;
    }
  }
}

void PluginManager::pushPluginSignal(Licq::PluginSignal* rawSignal)
{
  boost::shared_ptr<Licq::PluginSignal> signal(rawSignal);

  MutexLocker locker(myGeneralPluginsMutex);
  BOOST_FOREACH(GeneralPlugin::Ptr plugin, myGeneralPlugins)
  {
    if (plugin->wantSignal(signal->signal()))
      plugin->pushSignal(signal);
  }
}

void PluginManager::pushProtocolSignal(Licq::ProtocolSignal* rawSignal)
{
  boost::shared_ptr<Licq::ProtocolSignal> signal(rawSignal);
  const unsigned long protocolId = signal->userId().protocolId();

  MutexLocker locker(myProtocolPluginsMutex);
  BOOST_FOREACH(ProtocolPlugin::Ptr plugin, myProtocolPlugins)
  {
    if (plugin->protocolId() == protocolId)
    {
      plugin->pushSignal(signal);
      return;
    }
  }

  Licq::gLog.error(tr("Invalid protocol plugin requested (%ld)"), protocolId);
}
