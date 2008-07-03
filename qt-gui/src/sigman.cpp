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
    CICQSignal *s = licqDaemon->PopPluginSignal();
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


void CSignalManager::ProcessSignal(CICQSignal *s)
{
  switch (s->Signal())
  {
  case SIGNAL_UPDATExLIST:
    emit signal_updatedList(s);
    break;
  case SIGNAL_UPDATExUSER:
    emit signal_updatedUser(s);
    if (gUserManager.FindOwner(s->Id(), s->PPID()) != NULL && s->SubSignal() == USER_STATUS)
    {
      emit signal_updatedStatus(s);
    }
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
    emit signal_ui_viewevent(s->Id());
    break;
  case SIGNAL_UI_MESSAGE:
  //TODO
    emit signal_ui_message(s->Id(), s->PPID());
    break;
  case SIGNAL_ADDxSERVERxLIST:
  //TODO
    licqDaemon->icqRenameUser(s->Id());
    break;
  case SIGNAL_NEWxPROTO_PLUGIN:
    emit signal_protocolPlugin(s->SubSignal());
    break;
  case SIGNAL_EVENTxID:
    emit signal_eventTag(s->Id(), s->PPID(), s->Argument());
    break;
  case SIGNAL_SOCKET:
    emit signal_socket(s->Id(), s->PPID(), s->CID());
    break;
  case SIGNAL_CONVOxJOIN:
    emit signal_convoJoin(s->Id(), s->PPID(), s->CID());
    break;
  case SIGNAL_CONVOxLEAVE:
    emit signal_convoLeave(s->Id(), s->PPID(), s->CID());
    break;
  case SIGNAL_VERIFY_IMAGE:
    emit signal_verifyImage(s->PPID());
    break;
  case SIGNAL_NEW_OWNER:
    emit signal_newOwner(s->Id(), s->PPID());
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
