/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2012 Licq developers <licq-dev@googlegroups.com>
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

#include "signalmanager.h"

#include <unistd.h>

#include <QApplication>
#include <QSocketNotifier>

#include <licq/logging/log.h>
#include <licq/contactlist/usermanager.h>
#include <licq/event.h>
#include <licq/plugin/generalplugin.h>
#include <licq/pluginsignal.h>
#include <licq/protocolmanager.h>

#include "dialogs/ownereditdlg.h"

#include "plugin.h"

using Licq::gLog;
using Licq::gProtocolManager;
using namespace LicqQtGui;

SignalManager* LicqQtGui::gGuiSignalManager = NULL;

SignalManager::SignalManager()
  : myPipe(gQtGuiPlugin->getReadPipe())
{
  assert(gGuiSignalManager == NULL);
  gGuiSignalManager = this;

  gQtGuiPlugin->setSignalMask(Licq::PluginSignal::SignalAll);

  sn = new QSocketNotifier(myPipe, QSocketNotifier::Read);
  connect(sn, SIGNAL(activated(int)), SLOT(process()));
  sn->setEnabled(true);
}

SignalManager::~SignalManager()
{
  delete sn;

  gGuiSignalManager = NULL;
}

void SignalManager::ProcessSignal(Licq::PluginSignal* sig)
{
  const Licq::UserId& userId = sig->userId();
  unsigned long ppid = userId.protocolId();

  switch (sig->signal())
  {
    case Licq::PluginSignal::SignalList:
      switch (sig->subSignal())
      {
        case Licq::PluginSignal::ListOwnerAdded:
          emit ownerAdded(userId);
          break;
        case Licq::PluginSignal::ListOwnerRemoved:
          emit ownerRemoved(userId);
          break;
        default:
          emit updatedList(sig->subSignal(), sig->argument(), userId);
      }
      break;

    case Licq::PluginSignal::SignalUser:
      emit updatedUser(userId, sig->subSignal(), sig->argument(), sig->cid());

      if (Licq::gUserManager.isOwner(userId) && sig->subSignal() == Licq::PluginSignal::UserStatus)
        emit updatedStatus(ppid);
      break;

    case Licq::PluginSignal::SignalLogon:
      emit logon();
      break;

    case Licq::PluginSignal::SignalLogoff:
      if (sig->subSignal() == Licq::PluginSignal::LogoffPassword)
        new OwnerEditDlg(ppid);

      emit logoff();
      break;

    case Licq::PluginSignal::SignalPluginEvent:
    {
      switch (sig->subSignal())
      {
        case Licq::PluginSignal::PluginViewEvent:
          emit ui_viewevent(userId);
          break;

        case Licq::PluginSignal::PluginStartMessage:
          emit ui_message(userId);
          break;

        case Licq::PluginSignal::PluginShowUserList:
          emit ui_showuserlist();
          break;

        case Licq::PluginSignal::PluginHideUserList:
          emit ui_hideuserlist();
          break;
      }
      break;
    }
    case Licq::PluginSignal::SignalAddedToServer:
      //TODO
      gProtocolManager.updateUserAlias(userId);
      break;

    case Licq::PluginSignal::SignalNewProtocol:
      emit protocolPluginLoaded(sig->subSignal());
      break;

    case Licq::PluginSignal::SignalRemoveProtocol:
      emit protocolPluginUnloaded(sig->subSignal());
      break;

    case Licq::PluginSignal::SignalConversation:
      switch (sig->subSignal())
      {
        case Licq::PluginSignal::ConvoCreate:
          emit socket(userId, sig->cid());
          break;
        case Licq::PluginSignal::ConvoJoin:
          emit convoJoin(userId, ppid, sig->cid());
          break;
        case Licq::PluginSignal::ConvoLeave:
          emit convoLeave(userId, ppid, sig->cid());
          break;
      }
      break;

    case Licq::PluginSignal::SignalVerifyImage:
      emit verifyImage(ppid);
      break;

    case Licq::PluginSignal::SignalNewOwner:
      emit newOwner(userId);
      break;

    default:
      gLog.warning("Internal error: SignalManager::ProcessSignal(): "
          "Unknown signal command received from daemon: %d",
          sig->signal());
      break;
  }

  delete sig;
}

void SignalManager::ProcessEvent(Licq::Event* ev)
{
  if (ev->command() == Licq::Event::CommandSearch)
    emit searchResult(ev);
  else
    emit doneUserFcn(ev);
  delete ev;
}

void SignalManager::process()
{
  char buf[16];

  read(myPipe, buf, 1);

  switch (buf[0])
  {
    case Licq::GeneralPlugin::PipeSignal:
    {
      Licq::PluginSignal* s = gQtGuiPlugin->popSignal();
      ProcessSignal(s);
      break;
    }

    case Licq::GeneralPlugin::PipeEvent:
    {
      Licq::Event* e = gQtGuiPlugin->popEvent();
      ProcessEvent(e);
      break;
    }

    case Licq::GeneralPlugin::PipeShutdown:
    {
      gLog.info("Exiting main window (qt gui)");
      qApp->quit();
      break;
    }

    case Licq::GeneralPlugin::PipeDisable:
    case Licq::GeneralPlugin::PipeEnable:
      break;

    default:
      gLog.warning("Unknown notification type from daemon: %c", buf[0]);
  }
}
