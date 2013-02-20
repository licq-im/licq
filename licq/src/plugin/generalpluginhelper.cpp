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

#include <licq/plugin/generalpluginhelper.h>

#include <licq/pipe.h>
#include <licq/thread/mutex.h>
#include <licq/thread/mutexlocker.h>

#include <queue>

using namespace Licq;

class GeneralPluginHelper::Private
{
public:
  Private() : mySignalMask(0) { }

  void notify(char ch) { myPipe.putChar(ch); }

  Licq::Pipe myPipe;
  unsigned long mySignalMask;

  std::queue< boost::shared_ptr<const Licq::PluginSignal> > mySignals;
  Licq::Mutex mySignalsMutex;

  std::queue< boost::shared_ptr<const Licq::Event> > myEvents;
  Licq::Mutex myEventsMutex;
};

bool GeneralPluginHelper::init(int /*argc*/, char** /*argv*/)
{
  return true;
}

void GeneralPluginHelper::shutdown()
{
  LICQ_D();
  d->notify(PipeShutdown);
}

void GeneralPluginHelper::enable()
{
  LICQ_D();
  d->notify(PipeEnable);
}

void GeneralPluginHelper::disable()
{
  LICQ_D();
  d->notify(PipeDisable);
}

bool GeneralPluginHelper::wantSignal(unsigned long signalType) const
{
  LICQ_D_CONST();
  return (signalType & d->mySignalMask);
}

void GeneralPluginHelper::pushSignal(
    boost::shared_ptr<const PluginSignal> signal)
{
  LICQ_D();
  MutexLocker locker(d->mySignalsMutex);
  d->mySignals.push(signal);
  d->notify(PipeSignal);
}

void GeneralPluginHelper::pushEvent(boost::shared_ptr<const Event> event)
{
  LICQ_D();
  MutexLocker locker(d->myEventsMutex);
  d->myEvents.push(event);
  d->notify(PipeEvent);
}

GeneralPluginHelper::GeneralPluginHelper()
  : myPrivate(new Private)
{
  // Empty
}

GeneralPluginHelper::~GeneralPluginHelper()
{
  delete myPrivate;
}

int GeneralPluginHelper::getReadPipe() const
{
  LICQ_D();
  return d->myPipe.getReadFd();
}

void GeneralPluginHelper::setSignalMask(unsigned long signalMask)
{
  LICQ_D();
  d->mySignalMask = signalMask;
}

boost::shared_ptr<const Licq::PluginSignal> GeneralPluginHelper::popSignal()
{
  LICQ_D();
  MutexLocker locker(d->mySignalsMutex);
  if (!d->mySignals.empty())
  {
    boost::shared_ptr<const PluginSignal> signal = d->mySignals.front();
    d->mySignals.pop();
    return signal;
  }
  return boost::shared_ptr<const PluginSignal>();
}

boost::shared_ptr<const Licq::Event> GeneralPluginHelper::popEvent()
{
  LICQ_D();
  MutexLocker locker(d->myEventsMutex);
  if (!d->myEvents.empty())
  {
    boost::shared_ptr<const Event> event = d->myEvents.front();
    d->myEvents.pop();
    return event;
  }
  return boost::shared_ptr<const Event>();
}
