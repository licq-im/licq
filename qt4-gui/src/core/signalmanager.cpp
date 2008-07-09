// -*- c-basic-offset: 2; -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2006 Licq developers
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

#include <licq_icqd.h>
#include <licq_log.h>

#include "dialogs/ownereditdlg.h"

using namespace LicqQtGui;

SignalManager::SignalManager(int pipe)
  : myPipe(pipe)
{
  sn = new QSocketNotifier(myPipe, QSocketNotifier::Read);
  connect(sn, SIGNAL(activated(int)), SLOT(process()));
  sn->setEnabled(true);
}

SignalManager::~SignalManager()
{
  delete sn;
}

void SignalManager::ProcessSignal(CICQSignal* sig)
{
  switch (sig->Signal())
  {
    case SIGNAL_UPDATExLIST:
      emit updatedList(sig);
      break;

    case SIGNAL_UPDATExUSER:
      emit updatedUser(sig);

      if (gUserManager.FindOwner(sig->Id(), sig->PPID()) != NULL &&
          sig->SubSignal() == USER_STATUS)
        emit updatedStatus(sig);
      break;

    case SIGNAL_LOGON:
      emit logon();
      break;

    case SIGNAL_LOGOFF:
      if (sig->SubSignal() == LOGOFF_PASSWORD)
        new OwnerEditDlg(sig->PPID());

      emit logoff();
      break;

    case SIGNAL_UI_VIEWEVENT:
      emit ui_viewevent(sig->Id());
      break;

    case SIGNAL_UI_MESSAGE:
      //TODO
      emit ui_message(sig->Id(), sig->PPID());
      break;

    case SIGNAL_ADDxSERVERxLIST:
      //TODO
      gLicqDaemon->icqRenameUser(sig->Id());
      break;

    case SIGNAL_NEWxPROTO_PLUGIN:
      emit protocolPlugin(sig->SubSignal());
      break;

    case SIGNAL_EVENTxID:
      emit eventTag(sig->Id(), sig->PPID(), sig->Argument());
      break;

    case SIGNAL_SOCKET:
      emit socket(sig->Id(), sig->PPID(), sig->CID());
      break;

    case SIGNAL_CONVOxJOIN:
      emit convoJoin(sig->Id(), sig->PPID(), sig->CID());
      break;

    case SIGNAL_CONVOxLEAVE:
      emit convoLeave(sig->Id(), sig->PPID(), sig->CID());
      break;

    case SIGNAL_VERIFY_IMAGE:
      emit verifyImage(sig->PPID());
      break;

    case SIGNAL_NEW_OWNER:
      emit newOwner(sig->Id(), sig->PPID());
      break;

    default:
      gLog.Warn("%sInternal error: SignalManager::ProcessSignal(): "
          "Unknown signal command received from daemon: %ld.\n",
          L_WARNxSTR, sig->Signal());
      break;
  }

  delete sig;
}

void SignalManager::ProcessEvent(ICQEvent* ev)
{
  if (ev->Command() == ICQ_CMDxTCP_START) // direct connection check
  {
    emit doneUserFcn(ev);
    delete ev;
    return;
  }

  if (ev->SNAC() == 0)
  {
    // Not from ICQ
    emit doneUserFcn(ev); //FIXME
    return;
  }

  switch (ev->SNAC())
  {
    // Event commands for a user
    case MAKESNAC(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SERVERxMESSAGE):
    case MAKESNAC(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SERVERxREPLYxMSG):
    case MAKESNAC(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SENDxSERVER):
    case MAKESNAC(ICQ_SNACxFAM_LOCATION, ICQ_SNACxREQUESTxUSERxINFO):
    case MAKESNAC(ICQ_SNACxFAM_LOCATION, ICQ_SNACxLOC_INFOxREQ):
    case MAKESNAC(ICQ_SNACxFAM_BART, ICQ_SNACxBART_DOWNLOADxREQUEST):
      emit doneUserFcn(ev);
      break;

    // The all being meta snac
    case MAKESNAC(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxMETA):
      if (ev->SubCommand() == ICQ_CMDxMETA_SEARCHxWPxLAST_USER ||
          ev->SubCommand() == ICQ_CMDxMETA_SEARCHxWPxFOUND)
        emit searchResult(ev);
      else
        if (ev->SubCommand() == ICQ_CMDxSND_SYSxMSGxREQ ||
            ev->SubCommand() == ICQ_CMDxSND_SYSxMSGxDONExACK)
          emit doneOwnerFcn(ev);
        else
          emit doneUserFcn(ev);
      break;

    // Commands related to the basic operation
    case MAKESNAC(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_SETxSTATUS):
    case MAKESNAC(ICQ_SNACxFAM_BUDDY, ICQ_SNACxBDY_ADDxTOxLIST):
    case MAKESNAC(ICQ_SNACxFAM_BUDDY, ICQ_SNACxBDY_REMOVExFROMxLIST):
    case MAKESNAC(ICQ_SNACxFAM_AUTH, ICQ_SNACxREGISTER_USER):
      emit doneOwnerFcn(ev);
      break;

    default:
      gLog.Warn("%sInternal error: SignalManager::ProcessEvent(): "
          "Unknown event SNAC received from daemon: 0x%08lX.\n",
          L_WARNxSTR, ev->SNAC());
      break;
  }

  delete ev;
}

void SignalManager::process()
{
  char buf[16];

  read(myPipe, buf, 1);

  switch (buf[0])
  {
    case 'S':  // A signal is pending
    {
      CICQSignal* s = gLicqDaemon->PopPluginSignal();
      ProcessSignal(s);
      break;
    }

    case 'E':  // An event is pending
    {
      ICQEvent* e = gLicqDaemon->PopPluginEvent();
      ProcessEvent(e);
      break;
    }

    case 'X':  // Shutdown
    {
      gLog.Info("%sExiting main window (qt gui).\n", L_ENDxSTR);
      qApp->quit();
      break;
    }

    case '0':
    case '1':
      break;

    default:
      gLog.Warn("%sUnknown notification type from daemon: %c.\n", L_WARNxSTR, buf[0]);
  }
}
