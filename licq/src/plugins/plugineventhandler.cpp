/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Licq developers
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

#include "plugineventhandler.h"

#include <licq/event.h>
#include <licq/pluginsignal.h>
#include <licq/protocolsignal.h>

#include "gettext.h"
#include <licq/logging/log.h>
#include <licq/thread/mutexlocker.h>

#include <boost/foreach.hpp>

using Licq::MutexLocker;
using namespace LicqDaemon;

PluginEventHandler::PluginEventHandler(GeneralPluginsList& generalPlugins,
                                       Licq::Mutex& generalPluginsMutex,
                                       ProtocolPluginsList& protocolPlugins,
                                       Licq::Mutex& protocolPluginsMutex) :
  myGeneralPlugins(generalPlugins),
  myGeneralPluginsMutex(generalPluginsMutex),
  myProtocolPlugins(protocolPlugins),
  myProtocolPluginsMutex(protocolPluginsMutex)
{
  // Empty
}

PluginEventHandler::~PluginEventHandler()
{
  // Empty
}

void PluginEventHandler::pushGeneralEvent(Licq::Event* event)
{
  MutexLocker locker(myGeneralPluginsMutex);
  BOOST_FOREACH(GeneralPlugin::Ptr plugin, myGeneralPlugins)
  {
    if (plugin->isThread(event->thread_plugin))
    {
      plugin->pushEvent(event);
      return;
    }
  }

  // If no plugin got the event, then just delete it
  delete event;
}

Licq::Event* PluginEventHandler::popGeneralEvent()
{
  MutexLocker locker(myGeneralPluginsMutex);
  BOOST_FOREACH(GeneralPlugin::Ptr plugin, myGeneralPlugins)
  {
    if (plugin->isThisThread())
      return plugin->popEvent();
  }
  return NULL;
}

void PluginEventHandler::pushGeneralSignal(Licq::PluginSignal* signal)
{
  MutexLocker locker(myGeneralPluginsMutex);
  BOOST_FOREACH(GeneralPlugin::Ptr plugin, myGeneralPlugins)
  {
    if (plugin->wantSignal(signal->signal()))
      plugin->pushSignal(new Licq::PluginSignal(signal));
  }
  delete signal;
}

Licq::PluginSignal* PluginEventHandler::popGeneralSignal()
{
  MutexLocker locker(myGeneralPluginsMutex);
  BOOST_FOREACH(GeneralPlugin::Ptr plugin, myGeneralPlugins)
  {
    if (plugin->isThisThread())
      return plugin->popSignal();
  }
  return NULL;
}

void PluginEventHandler::pushProtocolSignal(Licq::ProtocolSignal* signal,
                                            unsigned long ppid)
{
  MutexLocker locker(myProtocolPluginsMutex);
  BOOST_FOREACH(ProtocolPlugin::Ptr plugin, myProtocolPlugins)
  {
    if (plugin->getProtocolId() == ppid)
    {
      plugin->pushSignal(signal);
      return;
    }
  }

  Licq::gLog.error(tr("Invalid protocol plugin requested (%ld)"), ppid);
  delete signal;
}

Licq::ProtocolSignal* PluginEventHandler::popProtocolSignal()
{
  MutexLocker locker(myProtocolPluginsMutex);
  BOOST_FOREACH(ProtocolPlugin::Ptr plugin, myProtocolPlugins)
  {
    if (plugin->isThisThread())
      return plugin->popSignal();
  }
  return NULL;
}
