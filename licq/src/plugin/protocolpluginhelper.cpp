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

#include <licq/plugin/protocolpluginhelper.h>

#include <licq/contactlist/owner.h>
#include <licq/contactlist/user.h>
#include <licq/pipe.h>
#include <licq/thread/mutex.h>
#include <licq/thread/mutexlocker.h>

#include <queue>

using namespace Licq;

class ProtocolPluginHelper::Private
{
public:
  void notify(char ch) { myPipe.putChar(ch); }

  Licq::Pipe myPipe;
  std::queue< boost::shared_ptr<const Licq::ProtocolSignal> > mySignals;
  Licq::Mutex mySignalsMutex;
};

bool ProtocolPluginHelper::init(int /*argc*/, char** /*argv*/)
{
  return true;
}

void ProtocolPluginHelper::shutdown()
{
  LICQ_D();
  d->notify(PipeShutdown);
}

void ProtocolPluginHelper::pushSignal(
    boost::shared_ptr<const ProtocolSignal> signal)
{
  LICQ_D();
  MutexLocker locker(d->mySignalsMutex);
  d->mySignals.push(signal);
  d->notify(PipeSignal);
}

ProtocolPluginHelper::ProtocolPluginHelper()
  : myPrivate(new Private)
{
  // Empty
}

ProtocolPluginHelper::~ProtocolPluginHelper()
{
  delete myPrivate;
}

int ProtocolPluginHelper::getReadPipe() const
{
  LICQ_D();
  return d->myPipe.getReadFd();
}

boost::shared_ptr<const Licq::ProtocolSignal> ProtocolPluginHelper::popSignal()
{
  LICQ_D();
  MutexLocker locker(d->mySignalsMutex);
  if (!d->mySignals.empty())
  {
    boost::shared_ptr<const ProtocolSignal> signal = d->mySignals.front();
    d->mySignals.pop();
    return signal;
  }
  return boost::shared_ptr<const ProtocolSignal>();
}
