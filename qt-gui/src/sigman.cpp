// -*- c-basic-offset: 2; -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2009 Licq developers
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

#include <qapplication.h>
#include <qsocketnotifier.h>

#include "sigman.h"
#include "licq_log.h"
#include "licq_events.h"
#include "licq_icqd.h"
#include "licq_user.h"
#include "licq_icq.h"
#include "securitydlg.h"

//=====CSignalManager===========================================================


CSignalManager::CSignalManager(CICQDaemon *d, int _nPipe)
{
  licqDaemon = d;
  m_nPipe = _nPipe;
  sn  = new QSocketNotifier(m_nPipe, QSocketNotifier::Read);
  connect(sn, SIGNAL(activated(int)), this, SLOT(slot_incoming()));
  sn->setEnabled(true);
}

CSignalManager::~CSignalManager()
{
  delete sn;
}

void CSignalManager::slot_incoming()
{
  char buf[16];
  read(m_nPipe, buf, 1);
  switch (buf[0])
  {
  case 'S':  // A signal is pending
  {
    LicqSignal* s = licqDaemon->popPluginSignal();
    ProcessSignal(s);
    break;
  }

  case 'E':  // An event is pending
  {
    ICQEvent *e = licqDaemon->PopPluginEvent();
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


void CSignalManager::ProcessSignal(LicqSignal* s)
{
  UserId userId = s->userId();

  // Temporary code to get account id and ppid until the rest of the gui is updated to use user id directly
  QString accountId;
  unsigned long ppid = 0;
  if (USERID_ISVALID(userId))
  {
    LicqUser* user = gUserManager.fetchUser(userId, LOCK_R);
    if (user != NULL)
    {
      accountId = user->accountId().c_str();
      ppid = user->ppid();
      gUserManager.DropUser(user);
    }
  }

  switch (s->Signal())
  {
  case SIGNAL_UPDATExLIST:
      emit signal_updatedList(s->SubSignal(), s->Argument(), userId);
    break;
  case SIGNAL_UPDATExUSER:
      emit signal_updatedUser(userId, s->SubSignal(), s->Argument(), s->CID());
      if (gUserManager.isOwner(userId) && s->SubSignal() == USER_STATUS)
        emit signal_updatedStatus(ppid);
    break;
  case SIGNAL_LOGON:
    emit signal_logon();
    break;
  case SIGNAL_LOGOFF:
    if( s->SubSignal() == LOGOFF_PASSWORD)
    {
    	SecurityDlg *sec = new SecurityDlg(licqDaemon, this);
    	sec->show();
    }

    emit signal_logoff();
    break;
  case SIGNAL_UI_VIEWEVENT:
      emit signal_ui_viewevent(userId);
    break;
  case SIGNAL_UI_MESSAGE:
  //TODO
      emit signal_ui_message(userId);
    break;
  case SIGNAL_ADDxSERVERxLIST:
  //TODO
      licqDaemon->updateUserAlias(userId);
      break;
  case SIGNAL_NEWxPROTO_PLUGIN:
    emit signal_protocolPlugin(s->SubSignal());
    break;
  case SIGNAL_EVENTxID:
      emit signal_eventTag(userId, s->Argument());
    break;
  case SIGNAL_SOCKET:
      emit signal_socket(userId, s->CID());
    break;
  case SIGNAL_CONVOxJOIN:
      emit signal_convoJoin(userId, ppid, s->CID());
    break;
  case SIGNAL_CONVOxLEAVE:
      emit signal_convoLeave(userId, ppid, s->CID());
    break;
  case SIGNAL_VERIFY_IMAGE:
      emit signal_verifyImage(ppid);
    break;
  case SIGNAL_NEW_OWNER:
      emit signal_newOwner(accountId, ppid);
    break;
  default:
    gLog.Warn("%sInternal error: CSignalManager::ProcessSignal(): Unknown signal command received from daemon: %ld.\n",
              L_WARNxSTR, s->Signal());
    break;
  }

  delete s;
}

void CSignalManager::ProcessEvent(ICQEvent *e)
{
  if (e->Command() == ICQ_CMDxTCP_START) // direct connection check
  {
    emit signal_doneUserFcn(e);
    delete e;
    return;
  }

  if (e->SNAC() == 0)
  {
    // Not from ICQ
    emit signal_doneUserFcn(e); //FIXME
    return;
  }

  switch (e->SNAC())
  {
  // Event commands for a user
  case MAKESNAC(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SERVERxMESSAGE):
  case MAKESNAC(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SERVERxREPLYxMSG):
  case MAKESNAC(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SENDxSERVER):
  case MAKESNAC(ICQ_SNACxFAM_LOCATION, ICQ_SNACxREQUESTxUSERxINFO):
  case MAKESNAC(ICQ_SNACxFAM_LOCATION, ICQ_SNACxLOC_INFOxREQ):
  case MAKESNAC(ICQ_SNACxFAM_BART, ICQ_SNACxBART_DOWNLOADxREQUEST):
    emit signal_doneUserFcn(e);
    break;

  // The all being meta snac
  case MAKESNAC(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxMETA):
    if (e->SubCommand() == ICQ_CMDxMETA_SEARCHxWPxLAST_USER ||
        e->SubCommand() == ICQ_CMDxMETA_SEARCHxWPxFOUND)
      emit signal_searchResult(e);
    else if (e->SubCommand() == ICQ_CMDxSND_SYSxMSGxREQ ||
             e->SubCommand() == ICQ_CMDxSND_SYSxMSGxDONExACK)
      emit signal_doneOwnerFcn(e);
    else
      emit signal_doneUserFcn(e);
    break;

  // Commands related to the basic operation
  case MAKESNAC(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_SETxSTATUS):
  case MAKESNAC(ICQ_SNACxFAM_BUDDY, ICQ_SNACxBDY_ADDxTOxLIST):
  case MAKESNAC(ICQ_SNACxFAM_BUDDY, ICQ_SNACxBDY_REMOVExFROMxLIST):
  case MAKESNAC(ICQ_SNACxFAM_AUTH, ICQ_SNACxREGISTER_USER):
    emit signal_doneOwnerFcn(e);
    break;

  default:
    gLog.Warn("%sInternal error: CSignalManager::ProcessEvent(): Unknown event SNAC received from daemon: 0x%08lX.\n",
              L_WARNxSTR, e->SNAC());
    break;
  }

  delete e;
}

#include "sigman.moc"
