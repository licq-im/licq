// -*- c-basic-offset: 2; -*-
/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#include <qapplication.h>
#include <qsocketnotifier.h>

#include "sigman.h"
#include "licq_log.h"
#include "licq_events.h"
#include "licq_icqd.h"
#include "licq_user.h"
#include "licq_socket.h"
#include "licq_icq.h"

//=====CSignalManager===========================================================


CSignalManager::CSignalManager(CICQDaemon *d, int _nPipe)
{
  licqDaemon = d;
  m_nPipe = _nPipe;
  sn  = new QSocketNotifier(m_nPipe, QSocketNotifier::Read);
  connect(sn, SIGNAL(activated(int)), this, SLOT(slot_incoming()));
  sn->setEnabled(true);
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
    if (s->Uin() == gUserManager.OwnerUin() && s->SubSignal() == USER_STATUS)
    {
      emit signal_updatedStatus();
    }
    break;
  case SIGNAL_LOGON:
    emit signal_logon();
    break;
  case SIGNAL_LOGOFF:
    emit signal_logoff();
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
  switch (e->Command())
  {
  // Event commands for a user
  case ICQ_CMDxTCP_START:
  case ICQ_CMDxSND_THRUxSERVER:
  case ICQ_CMDxSND_USERxGETINFO:
  case ICQ_CMDxSND_USERxGETDETAILS:
  case ICQ_CMDxSND_UPDATExDETAIL:
  case ICQ_CMDxSND_UPDATExBASIC:
  case ICQ_CMDxSND_RANDOMxSEARCH:
  case ICQ_CMDxSND_SETxRANDOMxCHAT:
    emit signal_doneUserFcn(e);
    break;

  case ICQ_CMDxSND_META:
    if (e->SubCommand() == ICQ_CMDxMETA_SEARCHxWPxLAST_USER ||
        e->SubCommand() == ICQ_CMDxMETA_SEARCHxWPxFOUND)
      emit signal_searchResult(e);
    else
      emit signal_doneUserFcn(e);

  // Commands related to the basic operation
  case ICQ_CMDxSND_LOGON:
  case ICQ_CMDxSND_SETxSTATUS:
  case ICQ_CMDxSND_AUTHORIZE:
  case ICQ_CMDxSND_USERxLIST:
  case ICQ_CMDxSND_VISIBLExLIST:
  case ICQ_CMDxSND_INVISIBLExLIST:
  case ICQ_CMDxSND_PING:
  case ICQ_CMDxSND_USERxADD:
  case ICQ_CMDxSND_SYSxMSGxREQ:
  case ICQ_CMDxSND_SYSxMSGxDONExACK:
  case ICQ_CMDxSND_REGISTERxUSER:
  case ICQ_CMDxSND_MODIFYxVIEWxLIST:
    emit signal_doneOwnerFcn(e);
    break;

  case ICQ_CMDxSND_SEARCHxINFO:
  case ICQ_CMDxSND_SEARCHxUIN:
    emit signal_searchResult(e);
    break;

  default:
    gLog.Warn("%sInternal error: CSignalManager::ProcessEvent(): Unknown event command received from daemon: %d.\n",
              L_WARNxSTR, e->Command());
    break;
  }

  delete e;
}

#include "sigman.moc"
