// -*- c-basic-offset: 2 -*-
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

// written by Graham Roff <graham@licq.org>
// contributions by Dirk A. Mueller <dirk@licq.org>

#include <qcheckbox.h>
#include <qdatetime.h>
#include <qvbox.h>
#include <qvgroupbox.h>
#include <qhgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qsplitter.h>

#include "licq_message.h"
#include "licq_icqd.h"

#include "authuserdlg.h"
#include "ewidgets.h"
#include "mainwin.h"
#include "messagebox.h"
#include "mmlistview.h"
#include "mmsenddlg.h"
#include "chatdlg.h"
#include "chatjoin.h"
#include "eventdesc.h"
#include "filedlg.h"
#include "forwarddlg.h"
#include "usereventdlg.h"
#include "refusedlg.h"
#include "sigman.h"
#include "showawaymsgdlg.h"


// -----------------------------------------------------------------------------

UserEventCommon::UserEventCommon(CICQDaemon *s, CSignalManager *theSigMan,
                                 CMainWindow *m, unsigned long _nUin,
                                 QWidget* parent, const char* name)
  : QDialog(parent, name, false, WDestructiveClose)
{
  server = s;
  mainwin = m;
  sigman = theSigMan;
  icqEventTag = NULL;
  m_nUin = _nUin;
  m_bOwner = (m_nUin == gUserManager.OwnerUin());

  top_lay = new QVBoxLayout(this, 8);

  QBoxLayout *layt = new QHBoxLayout(top_lay, 8);
  layt->addWidget(new QLabel(tr("Status:"), this));
  nfoStatus = new CInfoField(this, true);
  layt->addWidget(nfoStatus);
  layt->addWidget(new QLabel(tr("Time:"), this));
  nfoTimezone = new CInfoField(this, true);
  layt->addWidget(nfoTimezone);
  // these strings are not translated because they're replaced with an
  // icon anyway
  btnHistory = new QPushButton("Hist", this);
  connect(btnHistory, SIGNAL(clicked()), this, SLOT(showHistory()));
  layt->addWidget(btnHistory);
  btnInfo = new QPushButton("Info", this);
  connect(btnInfo, SIGNAL(clicked()), this, SLOT(showUserInfo()));
  layt->addWidget(btnInfo);

  ICQUser* u = gUserManager.FetchUser(m_nUin, LOCK_R);

  if (u != NULL)
  {
    nfoStatus->setData(u->StatusStr());
    if (u->GetTimezone() == TIMEZONE_UNKNOWN)
      nfoTimezone->setText(tr("Unknown"));
    else
    {
      m_nRemoteTimeOffset = u->LocalTimeOffset();
      QDateTime t;
      t.setTime_t(u->LocalTime());
      nfoTimezone->setText(tr("%1 (GMT%1%1%1)")
                           .arg(t.time().toString())
                           .arg(u->GetTimezone() > 0 ? "-" : "+")
                           .arg(abs(u->GetTimezone() / 2))
                           .arg(u->GetTimezone() % 2 ? "30" : "00") );
      tmrTime = new QTimer(this);
      connect(tmrTime, SIGNAL(timeout()), this, SLOT(slot_updatetime()));
      tmrTime->start(3000);
    }

    setCaption(QString::fromLocal8Bit(u->GetAlias()) + " (" +
               QString::fromLocal8Bit(u->GetFirstName()) + " " +
               QString::fromLocal8Bit(u->GetLastName())+ ")");
    setIconText(u->GetAlias());
    gUserManager.DropUser(u);
  }

  connect (sigman, SIGNAL(signal_updatedUser(CICQSignal *)), this, SLOT(userUpdated(CICQSignal *)));

  mainWidget = new QGroupBox(this);
  top_lay->addWidget(mainWidget);
}

void UserEventCommon::slot_updatetime()
{
  QDateTime t;
  t.setTime_t(time(NULL) + m_nRemoteTimeOffset);
  nfoTimezone->setText(nfoTimezone->text().replace(0, t.time().toString().length(), t.time().toString()));
}

void UserEventCommon::userUpdated(CICQSignal *sig)
{
  if (m_nUin != sig->Uin() || sig->SubSignal() != USER_STATUS) return;

  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
  if(u != NULL)
  {
    nfoStatus->setData(u->StatusStr());

    gUserManager.DropUser(u);
  }
}


void UserEventCommon::showHistory()
{
  mainwin->callInfoTab(mnuUserHistory, m_nUin);
}

void UserEventCommon::showUserInfo()
{
  mainwin->callInfoTab(mnuUserGeneral, m_nUin);
}

UserEventCommon::~UserEventCommon()
{
  qDebug("UserEventCommon::~UserEventCommon()");
  emit finished(m_nUin);
}


// -----------------------------------------------------------------------------

UserViewEvent::UserViewEvent(CICQDaemon *s, CSignalManager *theSigMan,
                             CMainWindow *m, unsigned long _nUin, QWidget* parent)
  : UserEventCommon(s, theSigMan, m, _nUin, parent, "UserViewEvent")
{
  // HACK, to be removed
  QBoxLayout* lay = new QVBoxLayout(mainWidget);
  splRead = new QSplitter(Vertical, mainWidget);
  lay->addWidget(splRead);
  splRead->setOpaqueResize();
  QHBox *h_top = new QHBox(splRead);
  msgView = new MsgView(h_top);
  btnReadNext = new QPushButton(tr("Nex&t"), h_top);
  btnReadNext->setEnabled(false);
//  btnReadNext->setFixedHeight(msgView->height());
  //btnReadNext->setFixedWidth(btnReadNext->width());
  connect(btnReadNext, SIGNAL(clicked()), this, SLOT(slot_btnReadNext()));

  QHGroupBox *h_msg = new QHGroupBox(splRead);
  mleRead = new MLEditWrap(true, h_msg, true);
  mleRead->setReadOnly(true);
//  mainWidget->setStretchFactor(h_msg, 1);

  connect (sigman, SIGNAL(signal_updatedUser(CICQSignal *)), this, SLOT(userUpdated(CICQSignal *)));
  connect (msgView, SIGNAL(clicked(QListViewItem *)), this, SLOT(slot_printMessage(QListViewItem *)));

  QHGroupBox *h_action = new QHGroupBox(mainWidget);
  lay->addWidget(h_action);
  btnRead1 = new QPushButton(h_action);
  btnRead2 = new QPushButton(h_action);
  btnRead3 = new QPushButton(h_action);
  btnRead4 = new QPushButton(h_action);

  btnRead1->setEnabled(false);
  btnRead2->setEnabled(false);
  btnRead3->setEnabled(false);
  btnRead4->setEnabled(false);

  connect(btnRead1, SIGNAL(clicked()), this, SLOT(slot_btnRead1()));
  connect(btnRead2, SIGNAL(clicked()), this, SLOT(slot_btnRead2()));
  connect(btnRead3, SIGNAL(clicked()), this, SLOT(slot_btnRead3()));
  connect(btnRead4, SIGNAL(clicked()), this, SLOT(slot_btnRead4()));

  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
  if (u && u->NewMessages() > 0)
  {
    MsgViewItem *e = new MsgViewItem(u->EventPeek(0), msgView);
    for (unsigned short i = 1; i < u->NewMessages(); i++)
    {
      (void) new MsgViewItem(u->EventPeek(i), msgView);
    }
    gUserManager.DropUser(u);
    slot_printMessage(e);
    u = gUserManager.FetchUser(m_nUin, LOCK_R);
    msgView->setSelected(e, true);
    msgView->ensureItemVisible(e);
    gUserManager.DropUser(u);
  }
}


UserViewEvent::~UserViewEvent()
{
}


void UserViewEvent::slot_printMessage(QListViewItem *eq)
{
  if (eq == NULL)
    return;

  MsgViewItem *e = (MsgViewItem *)eq;

  btnRead1->setText("");
  btnRead2->setText("");
  btnRead3->setText("");
  btnRead4->setText("");
  btnRead1->setEnabled(false);
  btnRead2->setEnabled(false);
  btnRead3->setEnabled(false);
  btnRead4->setEnabled(false);

  CUserEvent *m = e->msg;
  m_xCurrentReadEvent = m;
  mleRead->setText(QString::fromLocal8Bit(m->Text()));
  if (m->Direction() == D_RECEIVER && (m->Command() == ICQ_CMDxTCP_START || m->Command() == ICQ_CMDxRCV_SYSxMSGxONLINE))
  {
    switch (m->SubCommand())
    {
      case ICQ_CMDxSUB_CHAT:  // accept or refuse a chat request
      case ICQ_CMDxSUB_FILE:  // accept or refuse a file transfer
        if (m->IsCancelled())
        {
          mleRead->append(tr("\n--------------------\nRequest was cancelled."));
        }
        else
        {
          btnRead1->setText(tr("A&ccept"));
          btnRead2->setText(tr("&Refuse"));
          // If this is a chat, and we already have chats going, and this is
          // not a join request, then we can join
          if (m->SubCommand() == ICQ_CMDxSUB_CHAT &&
              ChatDlg::chatDlgs.size() > 0 &&
              ((CEventChat *)m)->Port() == 0)
            btnRead3->setText(tr("&Join"));
        }
        break;

      case ICQ_CMDxSUB_MSG:
        btnRead1->setText(tr("&Quote"));
        btnRead2->setText(tr("&Forward"));
        break;

      case ICQ_CMDxSUB_URL:   // view a url
        btnRead1->setText(tr("&Quote"));
        btnRead2->setText(tr("&Forward"));
        if (server->getUrlViewer() != NULL)
          btnRead3->setText(tr("&View"));
        break;

      case ICQ_CMDxSUB_AUTHxREQUEST:
      {
        btnRead1->setText(tr("A&uthorize"));
        btnRead2->setText(tr("&Refuse"));
        ICQUser *u = gUserManager.FetchUser( ((CEventAuthRequest *)m)->Uin(), LOCK_R);
        if (u == NULL)
          btnRead3->setText(tr("A&dd User"));
        else
          gUserManager.DropUser(u);
        break;
      }
      case ICQ_CMDxSUB_AUTHxGRANTED:
      {
        ICQUser *u = gUserManager.FetchUser( ((CEventAuthGranted *)m)->Uin(), LOCK_R);
        if (u == NULL)
          btnRead1->setText(tr("A&dd User"));
        else
          gUserManager.DropUser(u);
        break;
      }
      case ICQ_CMDxSUB_ADDEDxTOxLIST:
      {
        ICQUser *u = gUserManager.FetchUser( ((CEventAdded *)m)->Uin(), LOCK_R);
        if (u == NULL)
          btnRead1->setText(tr("A&dd User"));
        else
          gUserManager.DropUser(u);
        break;
      }
    } // switch
  }  // if

  if (!btnRead1->text().isEmpty()) btnRead1->setEnabled(true);
  if (!btnRead2->text().isEmpty()) btnRead2->setEnabled(true);
  if (!btnRead3->text().isEmpty()) btnRead3->setEnabled(true);
  if (!btnRead4->text().isEmpty()) btnRead4->setEnabled(true);

  if (e->m_nEventId != -1 && e->msg->Direction() == D_RECEIVER)
  {
    ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_W);
    u->EventClearId(e->m_nEventId);
    gUserManager.DropUser(u);
    e->MarkRead();
  }
}


void UserViewEvent::generateReply()
{
  QString s;
  for (int i = 0; i < mleRead->numLines(); i++)
     s += QString("> ") + mleRead->textLine(i) + " \n";

  sendMsg(s);
}

void UserViewEvent::sendMsg(QString txt)
{
  UserSendMsgEvent* e = new UserSendMsgEvent(server, sigman, mainwin, m_nUin);
  e->show();
  e->setText(txt);
}


void UserViewEvent::slot_btnRead1()
{
  if (m_xCurrentReadEvent == NULL) return;

  switch (m_xCurrentReadEvent->SubCommand())
  {
    case ICQ_CMDxSUB_CHAT:  // accept a chat request
    {
      btnRead1->setEnabled(false);
      btnRead2->setEnabled(false);
      CEventChat *c = (CEventChat *)m_xCurrentReadEvent;
      ChatDlg *chatDlg = new ChatDlg(m_nUin, server);
      if (c->Port() != 0)  // Joining a multiparty chat (we connect to them)
      {
        if (chatDlg->StartAsClient(c->Port()))
          server->icqChatRequestAccept(m_nUin, chatDlg->LocalPort(), c->Sequence());
      }
      else  // single party (other side connects to us)
      {
        if (chatDlg->StartAsServer())
          server->icqChatRequestAccept(m_nUin, chatDlg->LocalPort(), c->Sequence());
      }
      break;
    }

    case ICQ_CMDxSUB_FILE:  // accept a file transfer
    {
      btnRead1->setEnabled(false);
      btnRead2->setEnabled(false);
      CEventFile *f = (CEventFile *)m_xCurrentReadEvent;
      CFileDlg *fileDlg = new CFileDlg(m_nUin, server);
      if (fileDlg->ReceiveFiles())
        server->icqFileTransferAccept(m_nUin, fileDlg->LocalPort(), f->Sequence());
      break;
    }

    case ICQ_CMDxSUB_MSG:
      generateReply();
      break;

    case ICQ_CMDxSUB_URL:
      generateReply();
      break;

    case ICQ_CMDxSUB_AUTHxREQUEST:
      //server->icqAuthorizeGrant( ((CEventAuthRequest *)m_xCurrentReadEvent)->Uin(), "" );
      (void) new AuthUserDlg(server, ((CEventAuthRequest *)m_xCurrentReadEvent)->Uin(), true);
      break;

    case ICQ_CMDxSUB_AUTHxGRANTED:
      server->AddUserToList( ((CEventAuthGranted *)m_xCurrentReadEvent)->Uin());
      break;

    case ICQ_CMDxSUB_ADDEDxTOxLIST:
      server->AddUserToList( ((CEventAdded *)m_xCurrentReadEvent)->Uin());
      break;
  } // switch

}

void UserViewEvent::slot_btnRead2()
{
  if (m_xCurrentReadEvent == NULL) return;

  switch (m_xCurrentReadEvent->SubCommand())
  {
    case ICQ_CMDxSUB_CHAT:  // refuse a chat request
    {
      CRefuseDlg *r = new CRefuseDlg(m_nUin, tr("Chat"), this);
      if (r->exec())
      {
        btnRead1->setEnabled(false);
        btnRead2->setEnabled(false);
        server->icqChatRequestRefuse(m_nUin, r->RefuseMessage().local8Bit(),
           m_xCurrentReadEvent->Sequence());
      }
      delete r;
      break;
    }

    case ICQ_CMDxSUB_FILE:  // refuse a file transfer
    {
      CRefuseDlg *r = new CRefuseDlg(m_nUin, tr("File Transfer"), this);
      if (r->exec())
      {
        btnRead1->setEnabled(false);
        btnRead2->setEnabled(false);
        server->icqFileTransferRefuse(m_nUin, r->RefuseMessage().local8Bit(),
           m_xCurrentReadEvent->Sequence());
      }
      delete r;
      break;
    }

    case ICQ_CMDxSUB_MSG:  // Forward
    case ICQ_CMDxSUB_URL:
    {
      CForwardDlg *f = new CForwardDlg(server, sigman, mainwin, m_xCurrentReadEvent);
      // Move should go here to avoid flicker, but qt is stupid and
      // will ignore any move()s until after a show().
      //f->move(x() + width() / 2 - f->width() / 2, y() + height() / 2 - f->height() / 2);
      f->show();
      f->move(x() + width() / 2 - f->width() / 2, y() + height() / 2 - f->height() / 2);
      break;
    }

    case ICQ_CMDxSUB_AUTHxREQUEST:
    {
      (void) new AuthUserDlg(server, ((CEventAuthRequest *)m_xCurrentReadEvent)->Uin(), false);
      break;
    }
  } // switch

}


void UserViewEvent::slot_btnRead3()
{
  if (m_xCurrentReadEvent == NULL) return;

  switch (m_xCurrentReadEvent->SubCommand())
  {
    case ICQ_CMDxSUB_URL:   // view a url
      if (!server->ViewUrl(((CEventUrl *)m_xCurrentReadEvent)->Url()))
        WarnUser(this, tr("View URL failed"));
      break;

    case ICQ_CMDxSUB_CHAT:  // join to current chat
    {
      CEventChat *c = (CEventChat *)m_xCurrentReadEvent;
      if (c->Port() != 0)  // Joining a multiparty chat (we connect to them)
      {
        ChatDlg *chatDlg = new ChatDlg(m_nUin, server);
        if (chatDlg->StartAsClient(c->Port()))
          server->icqChatRequestAccept(m_nUin, chatDlg->LocalPort(), c->Sequence());
      }
      else  // single party (other side connects to us)
      {
        ChatDlg *chatDlg = NULL;
        CJoinChatDlg *j = new CJoinChatDlg(this);
        if (j->exec() && (chatDlg = j->JoinedChat()) != NULL)
          server->icqChatRequestAccept(m_nUin, chatDlg->LocalPort(), c->Sequence());
        delete j;
      }
      break;
    }

    case ICQ_CMDxSUB_AUTHxREQUEST:
      server->AddUserToList( ((CEventAuthRequest *)m_xCurrentReadEvent)->Uin());
      break;

  }
}


void UserViewEvent::slot_btnRead4()
{
}


void UserViewEvent::slot_btnReadNext()
{
  // todo
}


void UserViewEvent::userUpdated(CICQSignal *sig)
{
  if (m_nUin != sig->Uin() || sig->SubSignal() != USER_EVENTS) return;

  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
  if(u != NULL)
  {
    if (u->NewMessages() > 1)
    {
      btnReadNext->setEnabled(true);
      btnReadNext->setText(tr("Nex&t\n(%1)").arg(u->NewMessages()));
    }
    else if (u->NewMessages() == 1)
    {
      btnReadNext->setEnabled(true);
      btnReadNext->setText(tr("Nex&t"));
    }
    else
    {
      btnReadNext->setEnabled(false);
      btnReadNext->setText(tr("Nex&t"));
    }
    if (sig->Argument() > 0)
    {
      CUserEvent *e = u->EventPeekId(sig->Argument());
      if (e != NULL)
      {
        MsgViewItem *m = new MsgViewItem(e, msgView);
        msgView->ensureItemVisible(m);
      }
    }
    gUserManager.DropUser(u);
  }
}


//=====UserSendCommon========================================================
UserSendCommon::UserSendCommon(CICQDaemon *s, CSignalManager *theSigMan,
                               CMainWindow *m, unsigned long _nUin, QWidget* parent, const char* name)
  : UserEventCommon(s, theSigMan, m, _nUin, parent, name)
{
  grpMR = NULL;

  QGroupBox *box = new QGroupBox(this);
  top_lay->addWidget(box);
  QBoxLayout *vlay = new QVBoxLayout(box, 10, 5);
  QBoxLayout *hlay = new QHBoxLayout(vlay);
  chkSendServer = new QCheckBox(tr("Se&nd through server"), box);
  ICQUser* u = gUserManager.FetchUser(m_nUin, LOCK_R);
  if (chkSendServer->isEnabled())
  {
    chkSendServer->setChecked(u->SendServer() || (u->StatusOffline() && u->SocketDesc() == -1));
  }
  if (u->Ip() == 0)
  {
    chkSendServer->setChecked(true);
    chkSendServer->setEnabled(false);
  }
  gUserManager.DropUser(u);
  hlay->addWidget(chkSendServer);
  chkUrgent = new QCheckBox(tr("U&rgent"), box);
  hlay->addWidget(chkUrgent);
  chkMass = new QCheckBox(tr("&Multiple recipients"), box);
  hlay->addWidget(chkMass);
  connect(chkMass, SIGNAL(toggled(bool)), this, SLOT(massMessageToggled(bool)));

#ifdef USE_SPOOFING
  hlay = new QHBoxLayout(vlay);
  chkSpoof = new QCheckBox(tr("S&poof UIN:"), box);
  hlay->addWidget(chkSpoof);
  edtSpoof = new QLineEdit(box);
  hlay->addWidget(edtSpoof);
  edtSpoof->setEnabled(false);
  edtSpoof->setValidator(new QIntValidator(10000, 2000000000, edtSpoof));
  connect(chkSpoof, SIGNAL(toggled(bool)), edtSpoof, SLOT(setEnabled(bool)));
#else
  edtSpoof = NULL;
  chkSpoof = NULL;
#endif

  QBoxLayout* h_lay = new QHBoxLayout(top_lay);
  h_lay->addStretch(1);
  btnSend = new QPushButton(tr("&Send"), this);
  h_lay->addWidget(btnSend);
  connect(btnSend, SIGNAL(clicked()), this, SLOT(sendButton()));
  btnCancel = new QPushButton(tr("&Close"), this);
  h_lay->addWidget(btnCancel);
  connect(btnCancel, SIGNAL(clicked()), this, SLOT(cancelSend()));

  connect (sigman, SIGNAL(signal_updatedUser(CICQSignal *)), this, SLOT(userUpdated(CICQSignal *)));
}


UserSendCommon::~UserSendCommon()
{
}

void UserSendCommon::userUpdated(CICQSignal *sig)
{
  if (m_nUin != sig->Uin() || sig->SubSignal() != USER_STATUS) return;

  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
  if(u != NULL)
  {
    if (u->Ip() == 0)
    {
      chkSendServer->setChecked(true);
      chkSendServer->setEnabled(false);
    }
    else
    {
      chkSendServer->setEnabled(true);
    }
    if (u->StatusOffline())
      chkSendServer->setChecked(true);

    gUserManager.DropUser(u);
  }
}

void UserSendCommon::massMessageToggled(bool b)
{
  if(grpMR == NULL) {
    grpMR = new QVGroupBox(mainWidget);
    (void) new QLabel(tr("Drag Users Here\nRight Click for Options"), grpMR);
    lstMultipleRecipients = new CMMUserView(mainwin->UserView()->ColInfo(),
                                            mainwin->showHeader, m_nUin, mainwin, grpMR);
    lstMultipleRecipients->setFixedWidth(mainwin->UserView()->width());
    //laySend->addWidget(grpMR, 1, 1);
    //laySend->addMultiCellWidget(grpMR, 1, 2, 1, 1);
  }

  if(b)
    grpMR->show();
  else
    grpMR->close();
}


//-----UserSendCommon::sendButton--------------------------------------------
void UserSendCommon::sendButton()
{
  if (icqEventTag != NULL)
  {
    QString title = m_sBaseTitle + " [" + m_sProgressMsg + "]";
    setCaption(title);
    setCursor(waitCursor);
    btnSend->setEnabled(false);
    btnCancel->setText(tr("&Cancel"));
    connect (sigman, SIGNAL(signal_doneUserFcn(ICQEvent *)), this, SLOT(sendDone(ICQEvent *)));
  }
}


//-----UserSendCommon::sendDone----------------------------------------------
bool UserSendCommon::sendDone(ICQEvent *e)
{
  qDebug("UserSendCommon::sendDone()");

  if ( !icqEventTag->Equals(e) )
    return false;

  QString title, result;
  if (e == NULL)
  {
    result = tr("error");
  }
  else
  {
    switch (e->Result())
    {
    case EVENT_ACKED:
    case EVENT_SUCCESS:
      result = tr("done");
//      QTimer::singleShot(5000, this, SLOT(slot_resettitle()));
      break;
    case EVENT_FAILED:
      result = tr("failed");
      break;
    case EVENT_TIMEDOUT:
      result = tr("timed out");
      break;
    case EVENT_ERROR:
      result = tr("error");
      break;
    default:
      break;
    }
  }
  title = m_sBaseTitle + " [" + m_sProgressMsg + result + "]";
  setCaption(title);

  setCursor(arrowCursor);
  btnSend->setEnabled(true);
  btnCancel->setText(tr("&Close"));
  delete icqEventTag;
  icqEventTag = NULL;
  disconnect (sigman, SIGNAL(signal_doneUserFcn(ICQEvent *)), this, SLOT(sendDone(ICQEvent *)));

  return true;
}

//-----UserSendCommon::RetrySend---------------------------------------------
void UserSendCommon::RetrySend(ICQEvent *e, bool bOnline, unsigned short nLevel)
{
  qDebug("retrySend");

  //btnOk->setEnabled(false);
  //btnCancel->setText(tr("&Cancel"));

  chkSendServer->setChecked(!bOnline);
  chkUrgent->setChecked(nLevel == ICQ_TCPxMSG_URGENT);

  switch(e->SubCommand())
  {
    case ICQ_CMDxSUB_MSG:
    {
      CEventMsg *ue = (CEventMsg *)e->UserEvent();
      m_sProgressMsg = tr("Sending msg ");
      m_sProgressMsg += bOnline ? tr("direct") : tr("through server");
      m_sProgressMsg += "...";
      icqEventTag = server->icqSendMessage(m_nUin, ue->Message(), bOnline,
         nLevel, 0);
      break;
    }
    case ICQ_CMDxSUB_URL:
    {
      CEventUrl *ue = (CEventUrl *)e->UserEvent();
      m_sProgressMsg = tr("Sending URL ");
      m_sProgressMsg += bOnline ? tr("direct") : tr("through server");
      m_sProgressMsg += "...";
      icqEventTag = server->icqSendUrl(m_nUin, ue->Url(), ue->Description(),
         bOnline, nLevel, 0);
      break;
    }
    case ICQ_CMDxSUB_CHAT:
    {
      CEventChat *ue = (CEventChat *)e->UserEvent();
      m_sProgressMsg = tr("Sending chat request...");
      icqEventTag = server->icqChatRequest(m_nUin, ue->Reason(), nLevel);
      break;
    }
    case ICQ_CMDxSUB_FILE:
    {
      CEventFile *ue = (CEventFile *)e->UserEvent();
      m_sProgressMsg = tr("Sending file transfer...");
      icqEventTag = server->icqFileTransfer(m_nUin, ue->Filename(),
         ue->FileDescription(), nLevel);
      break;
    }
  }

  sendButton();

  /*if (icqEventTag != NULL)
  {
    connect (sigman, SIGNAL(signal_doneUserFcn(ICQEvent *)), this, SLOT(doneFcn(ICQEvent *)));
    QString title = m_sBaseTitle + " [" + m_sProgressMsg + "]";
    setCaption(title);
  }*/
}


void UserSendCommon::cancelSend()
{
  if(!icqEventTag) {
    close();
    return;
  }

  setCaption(m_sBaseTitle);
  server->CancelEvent(icqEventTag);
  delete icqEventTag;
  icqEventTag = NULL;

  setCursor(arrowCursor);
}



//=====UserSendMsgEvent======================================================

UserSendMsgEvent::UserSendMsgEvent(CICQDaemon *s, CSignalManager *theSigMan,
                                   CMainWindow *m, unsigned long _nUin, QWidget* parent)
  : UserSendCommon(s, theSigMan, m, _nUin, parent, "UserSendMsgEvent")
{
  QBoxLayout* lay = new QVBoxLayout(mainWidget);
  mleSend = new MLEditWrap(true, mainWidget, true);
  lay->addWidget(mleSend);
  mleSend->setMinimumHeight(150);
  mleSend->setFocus();
  setTabOrder(mleSend, btnSend);
}


UserSendMsgEvent::~UserSendMsgEvent()
{
}


//-----UserSendMsgEvent::setText---------------------------------------------
void UserSendMsgEvent::setText(const QString& txt)
{
  mleSend->setText(txt);
  mleSend->GotoEnd();
  mleSend->setEdited(false);
}


//-----UserSendMsgEvent::sendButton------------------------------------------
void UserSendMsgEvent::sendButton()
{
  qDebug("UserSendMsgEvent::sendButton() called!");

  // do nothing if a command is already being processed
  if (icqEventTag != NULL) return;

  if(!mleSend->edited() &&
     !QueryUser(this, tr("You didn't edit the message.\n"
                         "Do you really want to send it?"), tr("&Yes"), tr("&No")))
    return;

  unsigned short nMsgLen = mleSend->text().length();
  if (nMsgLen > MAX_MESSAGE_SIZE && chkSendServer->isChecked()
      && !QueryUser(this, tr("Message is %1 characters, over the ICQ server limit of %2.\n"
                             "The message will be truncated if sent through the server.")
                    .arg(nMsgLen).arg(MAX_MESSAGE_SIZE),
                    tr("C&ontinue"), tr("&Cancel")))
    return;

  unsigned long uin = (chkSpoof && chkSpoof->isChecked() ?
                       edtSpoof->text().toULong() : 0);
  // don't let the user send empty messages
  if (mleSend->text().stripWhiteSpace().isEmpty()) return;

  if (chkMass->isChecked())
  {
    CMMSendDlg *m = new CMMSendDlg(server, sigman, lstMultipleRecipients, this);
    int r = m->go_message(mleSend->text());
    delete m;
    if (r != QDialog::Accepted) return;
  }

  m_sProgressMsg = tr("Sending msg ");
  m_sProgressMsg += chkSendServer->isChecked() ? tr("through server") : tr("direct");
  m_sProgressMsg += "...";
  icqEventTag = server->icqSendMessage(m_nUin, mleSend->text().local8Bit(),
     chkSendServer->isChecked() ? false : true,
     chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL, uin);

  UserSendCommon::sendButton();
}


//-----UserSendMsgEvent::sendDone--------------------------------------------
bool UserSendMsgEvent::sendDone(ICQEvent* e)
{
  if (!UserSendCommon::sendDone(e)) return false;

  bool isOk = (e != NULL && (e->Result() == EVENT_ACKED || e->Result() == EVENT_SUCCESS));

  if (!isOk)
  {
    if (e->Command() == ICQ_CMDxTCP_START &&
        (e->SubCommand() == ICQ_CMDxSUB_MSG ||
         e->SubCommand() == ICQ_CMDxSUB_URL) &&
        QueryUser(this, tr("Direct send failed,\nsend through server?"), tr("Yes"), tr("No")) )
    {
      RetrySend(e, false, ICQ_TCPxMSG_NORMAL);
    }
  }
  else if(e->Command() == ICQ_CMDxTCP_START)
  {
    ICQUser *u = NULL;
    CUserEvent *ue = e->UserEvent();
    QString msg;
    if (e->SubResult() == ICQ_TCPxACK_RETURN)
    {
      u = gUserManager.FetchUser(m_nUin, LOCK_R);
      msg = tr("%1 is in %2 mode:\n%3\n")
        .arg(u->GetAlias()).arg(u->StatusStr())
        .arg(QString::fromLocal8Bit(u->AutoResponse()));
      gUserManager.DropUser(u);
      switch (QueryUser(this, msg, tr("Send\nUrgent"), tr("Send to\nContact List"), tr("Cancel")))
      {
      case 0:
        RetrySend(e, true, ICQ_TCPxMSG_URGENT);
        break;
      case 1:
        RetrySend(e, true, ICQ_TCPxMSG_LIST);
        break;
      case 2:
        break;
      }
    }
    else if (e->SubResult() == ICQ_TCPxACK_REFUSE)
    {
      u = gUserManager.FetchUser(m_nUin, LOCK_R);
      msg = tr("%1 refused %2, send through server.")
        .arg(u->GetAlias()).arg(EventDescription(ue));
      InformUser(this, msg);
      gUserManager.DropUser(u);
    }
    else
    {
      u = gUserManager.FetchUser(m_nUin, LOCK_R);
      if (u->Away() && u->ShowAwayMsg()) {
        gUserManager.DropUser(u);
        (void) new ShowAwayMsgDlg(NULL, NULL, m_nUin);
      }
      else
        gUserManager.DropUser(u);

      close();
    }
  }

  return true;
}


//=====UserSendUrlEvent======================================================
UserSendUrlEvent::UserSendUrlEvent(CICQDaemon *s, CSignalManager *theSigMan,
                                   CMainWindow *m, unsigned long _nUin, QWidget* parent)
  : UserSendCommon(s, theSigMan, m, _nUin, parent, "UserSendUrlEvent")
{
  QBoxLayout* lay = new QVBoxLayout(mainWidget);
  mleSend = new MLEditWrap(true, mainWidget, true);
  lay->addWidget(mleSend);
  setTabOrder(mleSend, btnSend);

  QGroupBox* grpOpt = new QGroupBox(1, Vertical, mainWidget);
  lay->addWidget(grpOpt);
  lblItem = new QLabel(tr("URL : "), grpOpt);
  edtItem = new CInfoField(grpOpt, false);
}


UserSendUrlEvent::~UserSendUrlEvent()
{
}

void UserSendUrlEvent::setUrl(const QString& url, const QString& description)
{
  edtItem->setText(url);
  mleSend->setText(description);
  mleSend->GotoEnd();
  mleSend->setEdited(false);
}

//-----UserSendUrlEvent::sendButton------------------------------------------
void UserSendUrlEvent::sendButton()
{
  unsigned long uin = (chkSpoof && chkSpoof->isChecked() ?
                       edtSpoof->text().toULong() : 0);

  if (edtItem->text().stripWhiteSpace().isEmpty())
    return;

  if (chkMass->isChecked())
  {
    CMMSendDlg *m = new CMMSendDlg(server, sigman, lstMultipleRecipients, this);
    int r = m->go_url(edtItem->text(), mleSend->text());
    delete m;
    if (r != QDialog::Accepted) return;
  }

  m_sProgressMsg = tr("Sending URL ");
  m_sProgressMsg += chkSendServer->isChecked() ? tr("through server") : tr("direct");
  m_sProgressMsg += "...";
  icqEventTag = server->icqSendUrl(m_nUin, edtItem->text().latin1(), mleSend->text().local8Bit(),
                                   chkSendServer->isChecked() ? false : true,
                                   chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL, uin);

  UserSendCommon::sendButton();
}


//=====UserSendFileEvent=====================================================
UserSendFileEvent::UserSendFileEvent(CICQDaemon *s, CSignalManager *theSigMan,
                                     CMainWindow *m, unsigned long _nUin, QWidget* parent)
  : UserSendCommon(s, theSigMan, m, _nUin, parent, "UserSendFileEvent")
{
  chkSendServer->setChecked(false);
  chkSendServer->setEnabled(false);
  chkMass->setChecked(false);
  chkMass->setEnabled(false);

  QBoxLayout* lay = new QVBoxLayout(mainWidget);
  mleSend = new MLEditWrap(true, mainWidget, true);
  lay->addWidget(mleSend);
  mleSend->setMinimumHeight(150);
  setTabOrder(mleSend, btnSend);

  QGroupBox* grpOpt = new QGroupBox(1, Vertical, mainWidget);
  lblItem = new QLabel(tr("File(s)"), grpOpt);
  edtItem = new CInfoField(grpOpt, false);
  lay->addWidget(grpOpt);
}


UserSendFileEvent::~UserSendFileEvent()
{
}


void UserSendFileEvent::sendButton()
{
  if (edtItem->text().stripWhiteSpace().isEmpty())
  {
    WarnUser(this, tr("You must specify a file to transfer!"));
    return;
  }
  m_sProgressMsg = tr("Sending file transfer...");
  icqEventTag = server->icqFileTransfer(m_nUin, edtItem->text().local8Bit(),
                                        mleSend->text().local8Bit(),
                                        chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL);

  UserSendCommon::sendButton();
}


bool UserSendFileEvent::sendDone(ICQEvent *e)
{
  if (!UserSendCommon::sendDone(e)) return false;

  if (!e->ExtendedAck()->Accepted())
  {
    ICQUser* u = gUserManager.FetchUser(m_nUin, LOCK_R);
    QString result = tr("File transfer with %2 refused:\n%3").arg(u->GetAlias()).arg(e->ExtendedAck()->Response());
    gUserManager.DropUser(u);
    InformUser(this, result);

  }
  else
  {
    CEventFile *f = (CEventFile *)e->UserEvent();
    CFileDlg *fileDlg = new CFileDlg(m_nUin, server);
    fileDlg->SendFiles(f->Filename(), e->ExtendedAck()->Port());
  }

  return true;
}


//=====UserSendChatEvent=====================================================
UserSendChatEvent::UserSendChatEvent(CICQDaemon *s, CSignalManager *theSigMan,
                                     CMainWindow *m, unsigned long _nUin, QWidget* parent)
  : UserSendCommon(s, theSigMan, m, _nUin, parent, "UserSendChatEvent")
{
  chkSendServer->setChecked(false);
  chkSendServer->setEnabled(false);
  chkMass->setChecked(false);
  chkMass->setEnabled(false);

  QBoxLayout* lay = new QVBoxLayout(mainWidget);
  mleSend = new MLEditWrap(true, mainWidget, true);
  lay->addWidget(mleSend);
  mleSend->setMinimumHeight(150);
  setTabOrder(mleSend, btnSend);
}


UserSendChatEvent::~UserSendChatEvent()
{
}


void UserSendChatEvent::sendButton()
{
  m_sProgressMsg = tr("Sending chat request...");
  if (m_nMPChatPort == 0)
    icqEventTag = server->icqChatRequest(m_nUin,
                                         mleSend->text().local8Bit(),
                                         chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL);
  else
    icqEventTag = server->icqMultiPartyChatRequest(m_nUin,
                                                   mleSend->text().local8Bit(), m_szMPChatClients.local8Bit(),
                                                   m_nMPChatPort,
                                                   chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL);
  UserSendCommon::sendButton();
}


bool UserSendChatEvent::sendDone(ICQEvent *e)
{
  if (!UserSendCommon::sendDone(e)) return false;

  if (!e->ExtendedAck()->Accepted())
  {
    ICQUser* u = gUserManager.FetchUser(m_nUin, LOCK_R);
    QString result = tr("Chat with %2 refused:\n%3").arg(u->GetAlias()).arg(e->ExtendedAck()->Response());
    gUserManager.DropUser(u);
    InformUser(this, result);

  }
  else
  {
    CEventChat *c = (CEventChat *)e->UserEvent();
    if (c->Port() == 0)  // If we requested a join, no need to do anything
    {
      ChatDlg *chatDlg = new ChatDlg(m_nUin, server);
      chatDlg->StartAsClient(e->ExtendedAck()->Port());
    }
  }

  return true;
}


// -----------------------------------------------------------------------------

UserSendContactEvent::UserSendContactEvent(CICQDaemon *s, CSignalManager *theSigMan,
                                           CMainWindow *m, unsigned long _nUin, QWidget* parent)
  : UserSendCommon(s, theSigMan, m, _nUin, parent, "UserSendContactEvent")
{
  QBoxLayout* lay = new QVBoxLayout(mainWidget);
  // this sucks but is okay just for now FIXME
  QGroupBox* grpOpt = new QGroupBox(1, Vertical, mainWidget);
  lay->addWidget(grpOpt);
  lblItem = new QLabel(tr("Uin: "), grpOpt);
  edtItem = new CInfoField(grpOpt, false);
}

UserSendContactEvent::~UserSendContactEvent()
{
}

void UserSendContactEvent::sendButton()
{
  unsigned long nContactUin = edtItem->text().toULong();

  if(nContactUin >= 10000) {
    UinList uins;

    uins.push_back(nContactUin);

    icqEventTag = server->icqSendContactList(m_nUin, uins,
                                             chkSendServer->isChecked() ? false : true,
                                             chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL);

    UserSendCommon::sendButton();
  }
}

// -----------------------------------------------------------------------------

#include "usereventdlg.moc"
