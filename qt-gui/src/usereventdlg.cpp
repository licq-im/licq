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

#include <qdatetime.h>
#include <qvbox.h>
#include <qvgroupbox.h>
#include <qhgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include "licq_message.h"
#include "licq_icqd.h"

#include "authuserdlg.h"
#include "ewidgets.h"
#include "messagebox.h"
#include "chatdlg.h"
#include "chatjoin.h"
#include "filedlg.h"
#include "forwarddlg.h"
#include "usereventdlg.h"
#include "refusedlg.h"


// -----------------------------------------------------------------------------

UserEventCommon::UserEventCommon(CICQDaemon *s, CSignalManager *theSigMan,
                                 CMainWindow *m, unsigned long _nUin,
                                 QWidget* parent, const char* name)
  : QDialog(parent, name)
{
  server = s;
  mainwin = m;
  sigman = theSigMan;
  icqEventTag = NULL;
  m_nUin = _nUin;
  m_bOwner = (m_nUin == gUserManager.OwnerUin());

  QBoxLayout* top_lay = new QVBoxLayout(this, 8);

  QBoxLayout *layt = new QHBoxLayout(top_lay, 8);
  layt->addWidget(new QLabel(tr("Status:"), this));
  nfoStatus = new CInfoField(this, true);
  layt->addWidget(nfoStatus);
  layt->addWidget(new QLabel(tr("Time:"), this));
  nfoTimezone = new CInfoField(this, true);
  layt->addWidget(nfoTimezone);

  ICQUser* u = gUserManager.FetchUser(m_nUin, LOCK_R);

  if(u) {
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

  mainWidget = new QVGroupBox(this);
  top_lay->addWidget(mainWidget);
}

void UserEventCommon::slot_updatetime()
{
  QDateTime t;
  t.setTime_t(time(NULL) + m_nRemoteTimeOffset);
  nfoTimezone->setText(nfoTimezone->text().replace(0, t.time().toString().length(), t.time().toString()));
}


UserEventCommon::~UserEventCommon()
{
  emit finished(m_nUin);
}


// -----------------------------------------------------------------------------

UserViewEvent::UserViewEvent(CICQDaemon *s, CSignalManager *theSigMan,
                             CMainWindow *m, unsigned long _nUin, QWidget* parent)
  : UserEventCommon(s, theSigMan, m, _nUin, parent, "UserViewEvent")
{
  QHGroupBox *h_top = new QHGroupBox(mainWidget);
  msgView = new MsgView(h_top);
  btnReadNext = new QPushButton(tr("Nex&t"), h_top);
  btnReadNext->setEnabled(false);
//  btnReadNext->setFixedHeight(msgView->height());
  //btnReadNext->setFixedWidth(btnReadNext->width());
  connect(btnReadNext, SIGNAL(clicked()), this, SLOT(slot_btnReadNext()));

  QHGroupBox *h_msg = new QHGroupBox(mainWidget);
  mleRead = new MLEditWrap(true, h_msg, true);
  mleRead->setReadOnly(true);
//  mainWidget->setStretchFactor(h_msg, 1);

#if QT_VERSION >= 210
  connect (msgView, SIGNAL(clicked(QListViewItem *)), this, SLOT(slot_printMessage(QListViewItem *)));
#else
  connect (msgView, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(slot_printMessage(QListViewItem *)));
#endif

  QHGroupBox *h_action = new QHGroupBox(mainWidget);
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
  btnRead4->setText("Info");
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
#if 0
  QString s;
  for (int i = 0; i < mleRead->numLines(); i++)
     s += QString("> ") + mleRead->textLine(i) + "\n";
  mleSend->setText(s);
  mleSend->GotoEnd();
  mleSend->setEdited(false);
  tabs->showPage(tabList[TAB_SEND].tab);
#endif
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
      CForwardDlg *f = new CForwardDlg(mainwin, m_xCurrentReadEvent);
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


// -----------------------------------------------------------------------------

UserSendMsgEvent::UserSendMsgEvent(CICQDaemon *s, CSignalManager *theSigMan,
                                   CMainWindow *m, unsigned long _nUin, QWidget* parent)
  : UserEventCommon(s, theSigMan, m, _nUin, parent, "UserSendMsgEvent")
{
}

UserSendMsgEvent::~UserSendMsgEvent()
{
}


// -----------------------------------------------------------------------------

UserSendUrlEvent::UserSendUrlEvent(CICQDaemon *s, CSignalManager *theSigMan,
                                   CMainWindow *m, unsigned long _nUin, QWidget* parent)
  : UserEventCommon(s, theSigMan, m, _nUin, parent, "UserSendUrlEvent")
{
}

UserSendUrlEvent::~UserSendUrlEvent()
{
}


// -----------------------------------------------------------------------------

UserSendFileEvent::UserSendFileEvent(CICQDaemon *s, CSignalManager *theSigMan,
                                     CMainWindow *m, unsigned long _nUin, QWidget* parent)
  : UserEventCommon(s, theSigMan, m, _nUin, parent, "UserSendFileEvent")
{
}

UserSendFileEvent::~UserSendFileEvent()
{
}


// -----------------------------------------------------------------------------

UserSendChatEvent::UserSendChatEvent(CICQDaemon *s, CSignalManager *theSigMan,
                                     CMainWindow *m, unsigned long _nUin, QWidget* parent)
  : UserEventCommon(s, theSigMan, m, _nUin, parent, "UserSendChatEvent")
{
}

UserSendChatEvent::~UserSendChatEvent()
{
}


// -----------------------------------------------------------------------------

UserSendContactEvent::UserSendContactEvent(CICQDaemon *s, CSignalManager *theSigMan,
                                           CMainWindow *m, unsigned long _nUin, QWidget* parent)
  : UserEventCommon(s, theSigMan, m, _nUin, parent, "UserSendContactEvent")
{
}

UserSendContactEvent::~UserSendContactEvent()
{
}

// -----------------------------------------------------------------------------

#include "usereventdlg.moc"
