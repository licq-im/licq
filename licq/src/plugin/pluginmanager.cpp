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

#include "config.h"

#include "pluginmanager.h"
#include "gettext.h"

#include <licq/logging/log.h>
#include <licq/event.h>
#include <licq/exceptions/exception.h>
#include <licq/logging/logservice.h>
#include <licq/plugin/generalpluginfactory.h>
#include <licq/plugin/protocolpluginfactory.h>
#include <licq/pluginsignal.h>
#include <licq/protocolsignal.h>
#include <licq/thread/mutexlocker.h>
#include <licq/version.h>

#include <algorithm>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>
#include <cassert>
#include <cctype>
#include <cerrno>
#include <iterator>
#include <glob.h>

#if HAVE_PRCTL
#include <sys/prctl.h>
#endif

#include "../contactlist/usermanager.h"
#include "../daemon.h"
#include "../utils/dynamiclibrary.h"

using namespace LicqDaemon;
using Licq::MutexLocker;
using Licq::Owner;
using Licq::StringList;
using Licq::User;
using Licq::UserId;
using Licq::gLog;
using Licq::gLogService;
using std::list;
using std::string;

// Declare global PluginManager (internal for daemon)
LicqDaemon::PluginManager LicqDaemon::gPluginManager;

// Initialize global Licq::PluginManager to refer to the internal PluginManager
Licq::PluginManager& Licq::gPluginManager(LicqDaemon::gPluginManager);

namespace
{

// Called in the plugin's thread just before creating the instance
static void createPluginCallback(const PluginInstance& instance)
{
  string name;

  try
  {
    name = dynamic_cast<
        const ProtocolPluginInstance&>(instance).plugin()->name();
    // Append /<id> to protocols to be able to separate instances
    name += "/" + boost::lexical_cast<std::string>(instance.id());
  }
  catch (std::bad_cast&)
  {
    name = dynamic_cast<
        const GeneralPluginInstance&>(instance).plugin()->name();
  }

  std::transform(name.begin(), name.end(), name.begin(), ::tolower);
  gLogService.createThreadLog(name);

#if HAVE_PRCTL && defined(PR_SET_NAME)
  prctl(PR_SET_NAME, name.c_str());
#endif
}

static void exitPluginCallback(const PluginInstance& instance)
{
  gPluginManager.pluginHasExited(instance.id());
}

struct IsPluginInstance
{
  int myId;
  IsPluginInstance(int id) : myId(id) { }
  bool operator()(const PluginInstance::Ptr& instance)
  {
    return instance->id() == myId;
  }
  bool operator()(const std::pair<Licq::UserId, PluginInstance::Ptr>& instance)
  {
    return instance.second->id() == myId;
  }
};

struct IsProtocolPlugin
{
  unsigned long myProtocolId;
  IsProtocolPlugin(unsigned long ppid) : myProtocolId(ppid) { }
  bool operator()(const ProtocolPlugin::Ptr& protocol)
  {
    return protocol->protocolId() == myProtocolId;
  }
  bool operator()(const std::pair<Licq::UserId,
                                  ProtocolPluginInstance::Ptr>& instance)
  {
    return instance.second->plugin()->protocolId() == myProtocolId;
  }
};

} // namespace

PluginManager::PluginManager() :
  myNextPluginId(1),
  myIsProtocolsStarted(false)
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

    boost::shared_ptr<Licq::GeneralPluginFactory> factory(
        (*pluginData->createFactory)(), pluginData->destroyFactory);

    GeneralPlugin::Ptr plugin =
        boost::make_shared<GeneralPlugin>(lib, factory, pluginThread);

    if (!keep)
      return plugin;

    // Create the plugin instance
    GeneralPluginInstance::Ptr instance = plugin->createInstance(
        getNewPluginId(), &createPluginCallback);
    if (!instance)
      throw std::exception();

    // Let the plugin initialize itself
    if (!instance->init(argc, argv))
    {
      gLog.error(tr("Failed to initialize plugin (%s)"),
          plugin->name().c_str());
      throw std::exception();
    }

    {
      MutexLocker generalLocker(myGeneralPluginsMutex);
      myGeneralPlugins.push_back(plugin);
      myGeneralInstances.push_back(instance);
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

    boost::shared_ptr<Licq::ProtocolPluginFactory> factory(
        (*pluginData->createFactory)(), pluginData->destroyFactory);

    {
      // Check if we already got a plugin for this protocol
      MutexLocker protocolLocker(myProtocolPluginsMutex);
      if (find_if(myProtocolPlugins.begin(), myProtocolPlugins.end(),
                  IsProtocolPlugin(factory->protocolId()))
          != myProtocolPlugins.end())
        throw std::exception();
    }

    ProtocolPlugin::Ptr plugin =
        boost::make_shared<ProtocolPlugin>(lib, factory, pluginThread);

    if (!keep)
      return plugin;

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
  std::set<unsigned long> ppids;
  {
    MutexLocker protocolLocker(myProtocolPluginsMutex);
    BOOST_FOREACH(ProtocolPlugin::Ptr plugin, myProtocolPlugins)
      ppids.insert(plugin->protocolId());
  }

  // Must call loadProtocol without holding mutex
  BOOST_FOREACH(unsigned long protocolId, ppids)
    gUserManager.loadProtocol(protocolId);

  {
    MutexLocker generalLocker(myGeneralPluginsMutex);
    BOOST_FOREACH(GeneralPluginInstance::Ptr instance, myGeneralInstances)
      startInstance(instance);
  }

  {
    MutexLocker protocolLocker(myProtocolPluginsMutex);
    BOOST_FOREACH(ProtocolOwnerInstances::value_type instance,
                  myProtocolInstances)
      startInstance(instance.second);
    myIsProtocolsStarted = true;
  }
}

void PluginManager::shutdownAllPlugins()
{
  {
    MutexLocker generalLocker(myGeneralPluginsMutex);
    BOOST_FOREACH(GeneralPluginInstance::Ptr instance, myGeneralInstances)
      instance->shutdown();
    myGeneralPlugins.clear();
  }

  {
    MutexLocker protocolLocker(myProtocolPluginsMutex);
    BOOST_FOREACH(ProtocolOwnerInstances::value_type instance,
                  myProtocolInstances)
      instance.second->shutdown();
    myProtocolPlugins.clear();
  }
}

void PluginManager::shutdownProtocolInstance(const Licq::UserId& ownerId)
{
  MutexLocker protocolLocker(myProtocolPluginsMutex);
  ProtocolOwnerInstances::iterator it = myProtocolInstances.find(ownerId);
  if (it != myProtocolInstances.end())
    it->second->shutdown();
}

void PluginManager::pluginHasExited(int id)
{
  MutexLocker locker(myExitListMutex);
  myExitList.push(id);
  gDaemon.notifyPluginExited();
}

void PluginManager::reapPlugin()
{
  int exitId;
  {
    MutexLocker locker(myExitListMutex);
    exitId = myExitList.front();
    myExitList.pop();
  }

  // Check general plugins first
  if (reapGeneralInstance(exitId))
    return;

  // Then check protocol plugins
  if (reapProtocolInstance(exitId))
    return;

  gLog.error(tr("Invalid plugin id (%d) in exit signal"), exitId);
}

void PluginManager::cancelAllPlugins()
{
  {
    MutexLocker locker(myGeneralPluginsMutex);
    BOOST_FOREACH(GeneralPluginInstance::Ptr instance, myGeneralInstances)
    {
      gLog.warning(tr("Plugin %s failed to exit"),
                   instance->plugin()->name().c_str());
      instance->cancelThread();
    }
    myGeneralInstances.clear();
  }

  {
    MutexLocker locker(myProtocolPluginsMutex);
    BOOST_FOREACH(ProtocolOwnerInstances::value_type instance,
                  myProtocolInstances)
    {
      gLog.warning(tr("Protocol plugin %s (id %d) failed to exit"),
                   instance.second->plugin()->name().c_str(),
                   instance.second->id());
      instance.second->cancelThread();
    }
    myProtocolInstances.clear();
  }
}

size_t PluginManager::getGeneralPluginsCount() const
{
  MutexLocker locker(myGeneralPluginsMutex);
  return myGeneralInstances.size();
}

size_t PluginManager::pluginCount() const
{
  MutexLocker generalLocker(myGeneralPluginsMutex);
  MutexLocker protocolLocker(myProtocolPluginsMutex);
  return myGeneralInstances.size() + myProtocolInstances.size();
}

User* PluginManager::createProtocolUser(const UserId& id, bool temporary)
{
  ProtocolPlugin::Ptr plugin;
  {
    MutexLocker locker(myProtocolPluginsMutex);
    list<ProtocolPlugin::Ptr>::iterator it = find_if(
        myProtocolPlugins.begin(), myProtocolPlugins.end(),
        IsProtocolPlugin(id.protocolId()));

    assert(it != myProtocolPlugins.end());
    plugin = *it;
  }

  return plugin->createUser(id, temporary);
}

Owner* PluginManager::createProtocolOwner(const UserId& id)
{
  gLog.debug("Create new protocol instance for %s", id.toString().c_str());

  ProtocolPlugin::Ptr plugin;
  {
    MutexLocker locker(myProtocolPluginsMutex);
    list<ProtocolPlugin::Ptr>::iterator it = find_if(
        myProtocolPlugins.begin(), myProtocolPlugins.end(),
        IsProtocolPlugin(id.protocolId()));

    assert(it != myProtocolPlugins.end());
    plugin = *it;
  }

  ProtocolPluginInstance::Ptr instance = plugin->createInstance(
      getNewPluginId(), id, &createPluginCallback);
  if (!instance || !instance->init(0, NULL))
  {
    gLog.error(tr("Failed to create and initialize protocol instance"
                  " for %s"), id.toString().c_str());
    return NULL;
  }

  {
    MutexLocker locker(myProtocolPluginsMutex);
    assert(myProtocolInstances.find(id) == myProtocolInstances.end());
    myProtocolInstances.insert(std::make_pair(instance->ownerId(), instance));
    if (myIsProtocolsStarted)
      startInstance(instance);
  }

  return plugin->createOwner(id);
}

void PluginManager::
getGeneralPluginsList(Licq::GeneralPluginsList& plugins) const
{
  plugins.clear();

  MutexLocker locker(myGeneralPluginsMutex);
  copy(myGeneralPlugins.begin(), myGeneralPlugins.end(),
       back_inserter(plugins));
}

void PluginManager::
getProtocolPluginsList(Licq::ProtocolPluginsList& plugins) const
{
  plugins.clear();

  MutexLocker locker(myProtocolPluginsMutex);
  copy(myProtocolPlugins.begin(), myProtocolPlugins.end(),
       back_inserter(plugins));
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

  for (size_t i = 0; i < static_cast<size_t>(globbuf.gl_pathc); ++i)
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
  list<ProtocolPlugin::Ptr>::const_iterator it = find_if(
      myProtocolPlugins.begin(), myProtocolPlugins.end(),
      IsProtocolPlugin(protocolId));
  if (it != myProtocolPlugins.end())
    return *it;

  return Licq::ProtocolPlugin::Ptr();
}

Licq::ProtocolPluginInstance::Ptr PluginManager::getProtocolInstance(
    const Licq::UserId& ownerId) const
{
  MutexLocker locker(myProtocolPluginsMutex);
  ProtocolOwnerInstances::const_iterator it =
      myProtocolInstances.find(ownerId);
  if (it != myProtocolInstances.end())
    return it->second;
  return Licq::ProtocolPluginInstance::Ptr();
}

bool PluginManager::
startGeneralPlugin(const std::string& name, int argc, char** argv)
{
  GeneralPlugin::Ptr plugin = loadGeneralPlugin(name, argc, argv);
  if (!plugin)
    return false;

  startInstance(boost::dynamic_pointer_cast<GeneralPluginInstance>(
                    plugin->instance()));
  return true;
}

bool PluginManager::startProtocolPlugin(const std::string& name)
{
  ProtocolPlugin::Ptr plugin = loadProtocolPlugin(name);
  if (!plugin)
    return false;

  // Load contacts and owners for the new protocol
  gUserManager.loadProtocol(plugin->protocolId());

  ProtocolPlugin::Instances instances = plugin->instances();
  BOOST_FOREACH(Licq::ProtocolPluginInstance::Ptr instance, instances)
  {
    startInstance(
        boost::dynamic_pointer_cast<ProtocolPluginInstance>(instance));
  }
  return true;
}

void PluginManager::unloadGeneralPlugin(Licq::GeneralPlugin::Ptr licqPlugin)
{
  GeneralPlugin::Ptr plugin;
  {
    MutexLocker locker(myGeneralPluginsMutex);
    list<GeneralPlugin::Ptr>::iterator it = find(
        myGeneralPlugins.begin(), myGeneralPlugins.end(), licqPlugin);
    if (it == myGeneralPlugins.end())
      return;

    plugin = *it;
    // Remove the plugin from the list so that the plugin is unloaded once the
    // instance has stopped.
    myGeneralPlugins.erase(it);
  }

  GeneralPluginInstance::Ptr instance =
      boost::dynamic_pointer_cast<GeneralPluginInstance>(plugin->instance());
  if (instance)
    instance->shutdown();
}

void PluginManager::unloadProtocolPlugin(Licq::ProtocolPlugin::Ptr licqPlugin)
{
  ProtocolPlugin::Ptr plugin;
  {
    MutexLocker locker(myProtocolPluginsMutex);
    list<ProtocolPlugin::Ptr>::iterator it = find(
        myProtocolPlugins.begin(), myProtocolPlugins.end(), licqPlugin);
    if (it == myProtocolPlugins.end())
      return;

    plugin = *it;

    // Check with user manager first if unloading is allowed
    if (!gUserManager.allowUnloadProtocol(plugin->protocolId()))
      return;

    // Remove the plugin from the list so that the plugin is unloaded once all
    // instances have stopped.
    myProtocolPlugins.erase(it);
  }

  gUserManager.unloadProtocol(plugin->protocolId());

  ProtocolPlugin::Instances instances = plugin->instances();

  // Notify plugins about the removed protocol here if there wasn't any running
  // instance left. Otherwise the signal will be sent once the last instance
  // has exited.
  if (instances.empty())
  {
    pushPluginSignal(new Licq::PluginSignal(
        Licq::PluginSignal::SignalRemoveProtocol, plugin->protocolId()));
  }
  else
  {
    // Shutdown all instances
    BOOST_FOREACH(Licq::ProtocolPluginInstance::Ptr instance, instances)
      boost::dynamic_pointer_cast<ProtocolPluginInstance>(instance)->shutdown();
  }
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

void PluginManager::startInstance(GeneralPluginInstance::Ptr instance)
{
  Licq::GeneralPlugin::Ptr plugin = instance->plugin();
  gLog.info(tr("Starting plugin %s (id %d; version %s)"),
      plugin->name().c_str(), instance->id(), plugin->version().c_str());

  instance->run(NULL, exitPluginCallback);
}

void PluginManager::startInstance(ProtocolPluginInstance::Ptr instance)
{
  ProtocolPlugin::Ptr plugin = instance->plugin();
  gLog.info(tr("Starting protocol plugin %s (id %d; version %s)"),
      plugin->name().c_str(), instance->id(), plugin->version().c_str());

  instance->run(NULL, exitPluginCallback);
}

bool PluginManager::reapGeneralInstance(int exitId)
{
  GeneralPluginInstance::Ptr instance;
  {
    MutexLocker locker(myGeneralPluginsMutex);

    list<GeneralPluginInstance::Ptr>::iterator it = find_if(
        myGeneralInstances.begin(), myGeneralInstances.end(),
        IsPluginInstance(exitId));
    if (it == myGeneralInstances.end())
      return false;

    instance = *it;
    myGeneralInstances.erase(it);

    // Unloads the plugin in case the instance exited on its own
    myGeneralPlugins.remove(instance->plugin());
  }

  int result = instance->joinThread();
  gLog.info(tr("Plugin %s exited with code %d"),
      instance->plugin()->name().c_str(), result);

  return true;
}

bool PluginManager::reapProtocolInstance(int exitId)
{
  ProtocolPluginInstance::Ptr instance;
  {
    MutexLocker locker(myProtocolPluginsMutex);

    ProtocolOwnerInstances::iterator it = find_if(
        myProtocolInstances.begin(), myProtocolInstances.end(),
        IsPluginInstance(exitId));
    if (it == myProtocolInstances.end())
      return false;

    instance = it->second;
    myProtocolInstances.erase(it);
  }

  ProtocolPlugin::Ptr plugin = instance->plugin();

  int result = instance->joinThread();
  gLog.info(tr("Protocol %s instance %d (owner %s) exited with code %d"),
      plugin->name().c_str(), exitId, instance->ownerId().accountId().c_str(),
      result);

  const unsigned long protocolId = plugin->protocolId();

  bool isPluginUnloaded = false;
  {
    MutexLocker locker(myProtocolPluginsMutex);

    // See if there is any more instances of this protocol
    if (find_if(myProtocolInstances.begin(), myProtocolInstances.end(),
                IsProtocolPlugin(protocolId))
        != myProtocolInstances.end())
      return true;

    // If the plugin has been removed from the list we got a request to unload
    // the plugin and as this is the final instance, we must then notify
    // plugins about the removed protocol.
    if (find(myProtocolPlugins.begin(), myProtocolPlugins.end(), plugin)
        == myProtocolPlugins.end())
      isPluginUnloaded = true;
  }

  if (isPluginUnloaded)
  {
    pushPluginSignal(new Licq::PluginSignal(
        Licq::PluginSignal::SignalRemoveProtocol, protocolId));

    // Needs to be done here in case the instance was shut down by
    // shutdownAllPlugins
    gUserManager.unloadProtocol(protocolId);
  }

  return true;
}

void PluginManager::pushPluginEvent(Licq::Event* rawEvent)
{
  boost::shared_ptr<Licq::Event> event(rawEvent);

  MutexLocker locker(myGeneralPluginsMutex);
  BOOST_FOREACH(GeneralPluginInstance::Ptr instance, myGeneralInstances)
  {
    if (instance->isThread(event->thread_plugin))
    {
      instance->pushEvent(event);
      return;
    }
  }
}

void PluginManager::pushPluginSignal(Licq::PluginSignal* rawSignal)
{
  boost::shared_ptr<Licq::PluginSignal> signal(rawSignal);

  MutexLocker locker(myGeneralPluginsMutex);
  BOOST_FOREACH(GeneralPluginInstance::Ptr instance, myGeneralInstances)
  {
    if (instance->wantSignal(signal->signal()))
      instance->pushSignal(signal);
  }
}

void PluginManager::pushProtocolSignal(Licq::ProtocolSignal* rawSignal)
{
  boost::shared_ptr<Licq::ProtocolSignal> signal(rawSignal);
  const Licq::UserId ownerId = signal->userId().ownerId();

  MutexLocker locker(myProtocolPluginsMutex);
  ProtocolOwnerInstances::iterator it = myProtocolInstances.find(ownerId);
  if (it != myProtocolInstances.end())
  {
    it->second->pushSignal(signal);
    return;
  }

  gLog.error(tr("Invalid protocol plugin requested (%s)"),
      ownerId.toString().c_str());
}
