#include <qapplication.h>

#include "sigman.h"
#include "log.h"
#include "icqevent.h"
#include "icqd.h"
#include "user.h"
#include "icq-defines.h"

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

  default:
    gLog.Warn("%sUnknown notification type from daemon: %c.\n", L_WARNxSTR, buf[0]);
  }
}


void CSignalManager::ProcessSignal(CICQSignal *s)
{
  switch (s->Signal())
  {
  case SIGNAL_UPDATExLIST:
    emit signal_updatedList(s->SubSignal(), s->Uin());
    break;
  case SIGNAL_UPDATExUSER:
    emit signal_updatedUser(s->SubSignal(), s->Uin());
    if (s->Uin() == gUserManager.OwnerUin() && s->SubSignal() == USER_STATUS)
    {
      emit signal_updatedStatus();
    }
    break;
  case SIGNAL_LOGON:
    emit signal_logon();
    break;
  default:
    gLog.Warn("%sInternal error: CSignalManager::ProcessSignal(): Unknown signal command received from daemon: %d.\n", 
              L_WARNxSTR, s->Signal());
    break;
  }

  delete s;
}

void CSignalManager::ProcessEvent(ICQEvent *e)
{
  switch (e->m_nCommand)
  {
  // Event commands for a user
  case ICQ_CMDxTCP_START:
  case ICQ_CMDxSND_THRUxSERVER:
  case ICQ_CMDxSND_USERxGETINFO:
  case ICQ_CMDxSND_USERxGETDETAILS:
  case ICQ_CMDxSND_UPDATExDETAIL:
  case ICQ_CMDxSND_UPDATExBASIC:
    emit signal_doneUserFcn(e);
    break;

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
    emit signal_doneOwnerFcn(e);
    break;

  case ICQ_CMDxSND_SEARCHxSTART:
    emit signal_searchResult(e);
    break;

  default:
    gLog.Warn("%sInternal error: CSignalManager::ProcessEvent(): Unknown event command received from daemon: %d.\n", 
              L_WARNxSTR, e->m_nCommand);
    break;
  }

  delete e;
}

#include "moc/moc_sigman.h"
