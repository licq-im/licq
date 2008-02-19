// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2006 Licq developers
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

#include "userviewevent.h"

#include "config.h"

#include <QApplication>
#include <QCheckBox>
#include <QDesktopWidget>
#include <QFile>
#include <QGroupBox>
#include <QPushButton>
#include <QShortcut>
#include <QSplitter>
#include <QTextCodec>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

#ifdef USE_KDE
#include <KDE/KToolInvocation>
#endif

#include <licq_events.h>
#include <licq_icqd.h>
#include <licq_message.h>

#include "config/chat.h"
#include "config/iconmanager.h"

#include "core/gui-defines.h"
#include "core/licqgui.h"
#include "core/mainwin.h"
#include "core/messagebox.h"
#include "core/signalmanager.h"

#include "dialogs/authuserdlg.h"
#include "dialogs/chatdlg.h"
#include "dialogs/filedlg.h"
#include "dialogs/forwarddlg.h"
#include "dialogs/joinchatdlg.h"
#include "dialogs/refusedlg.h"

#include "widgets/messagelist.h"
#include "widgets/mlview.h"
#include "widgets/skinnablebutton.h"

#include "usersendmsgevent.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::UserViewEvent */

UserViewEvent::UserViewEvent(QString id, unsigned long ppid, QWidget* parent)
  : UserEventCommon(id, ppid, parent, "UserViewEvent")
{
  splRead = new QSplitter(Qt::Vertical);
  splRead->setOpaqueResize();
  mainWidget->addWidget(splRead);

  QShortcut* a = new QShortcut(Qt::Key_Escape, this);
  connect(a, SIGNAL(activated()), SLOT(close()));

  msgView = new MessageList();
  splRead->addWidget(msgView);

  mlvRead = new MLView();
  splRead->addWidget(mlvRead);

  splRead->setStretchFactor(0, 0);
  splRead->setStretchFactor(1, 1);

  connect(msgView, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
      SLOT(slotPrintMessage(QTreeWidgetItem*)));
  connect(gMainWindow, SIGNAL(signal_sentevent(ICQEvent*)),
      SLOT(slotSentEvent(ICQEvent*)));
  connect(mlvRead, SIGNAL(viewurl(QWidget*, QString)),
      LicqGui::instance(), SLOT(viewUrl(QWidget*, QString)));

  QGroupBox* h_action = new QGroupBox();
  mainWidget->addSpacing(10);
  mainWidget->addWidget(h_action);

  QHBoxLayout* h_action_lay = new QHBoxLayout(h_action);

  btnRead1 = new QPushButton();
  btnRead2 = new QPushButton();
  btnRead3 = new QPushButton();
  btnRead4 = new QPushButton();

  btnRead1->setEnabled(false);
  btnRead2->setEnabled(false);
  btnRead3->setEnabled(false);
  btnRead4->setEnabled(false);

  connect(btnRead1, SIGNAL(clicked()), SLOT(slotBtnRead1()));
  connect(btnRead2, SIGNAL(clicked()), SLOT(slotBtnRead2()));
  connect(btnRead3, SIGNAL(clicked()), SLOT(slotBtnRead3()));
  connect(btnRead4, SIGNAL(clicked()), SLOT(slotBtnRead4()));

  h_action_lay->addWidget(btnRead1);
  h_action_lay->addWidget(btnRead2);
  h_action_lay->addWidget(btnRead3);
  h_action_lay->addWidget(btnRead4);

  QHBoxLayout* h_lay = new QHBoxLayout();
  top_lay->addLayout(h_lay);

  if (!myIsOwner)
  {
    chkAutoClose = new QCheckBox(tr("Aut&o Close"));
    chkAutoClose->setChecked(Config::Chat::instance()->autoClose());
    h_lay->addWidget(chkAutoClose);
  }

  h_lay->addStretch(1);

  btnReadNext = new QPushButton(tr("Nex&t"));
  btnReadNext->setEnabled(false);
  connect(btnReadNext, SIGNAL(clicked()), SLOT(slotBtnReadNext()));
  h_lay->addWidget(btnReadNext);
  setTabOrder(btnRead4, btnReadNext);

  btnClose = new SkinnableButton(tr("&Close"));
  btnClose->setToolTip(tr("Normal Click - Close Window\n<CTRL>+Click - also delete User"));
  connect(btnClose, SIGNAL(clicked()), SLOT(slotClose()));
  h_lay->addWidget(btnClose);
  setTabOrder(btnReadNext, btnClose);

  ICQUser* u = gUserManager.FetchUser(myUsers.front().c_str(), myPpid, LOCK_R);
  if (u != NULL && u->NewMessages() > 0)
  {
    unsigned short i = 0;
    // Create an item for the message we're currently viewing.
    if (Config::Chat::instance()->msgChatView())
    {
      for (i = 0; i < u->NewMessages(); i++)
        if (u->EventPeek(i)->SubCommand() != ICQ_CMDxSUB_MSG &&
            u->EventPeek(i)->SubCommand() != ICQ_CMDxSUB_URL)
          break;
      if (i == u->NewMessages())
        i = 0;
    }

    MessageListItem* e = new MessageListItem(u->EventPeek(i), myCodec, msgView);
    myHighestEventId = u->EventPeek(i)->Id();

    /* Create items for all the messages which already await
     * in the queue. We cannot rely on getting CICQSignals for them
     * since they might've arrived before the dialog appeared,
     * possibly being undisplayed messages from previous licq session.
     */
    for (i++; i < u->NewMessages(); i++)
    {
      CUserEvent* event = u->EventPeek(i);
      if (!Config::Chat::instance()->msgChatView() ||
          (event->SubCommand() != ICQ_CMDxSUB_MSG &&
           event->SubCommand() != ICQ_CMDxSUB_URL))
      {
        new MessageListItem(event, myCodec, msgView);
        // Make sure we don't add this message again,
        // even if we receive an userUpdated signal for it.
        if (myHighestEventId < event->Id())
          myHighestEventId = event->Id();
      }
    }

    gUserManager.DropUser(u);
    for (unsigned short i = 0; i < msgView->columnCount(); i++)
      msgView->resizeColumnToContents(i);
    msgView->setCurrentItem(e, 0);
    msgView->scrollToItem(e);
    slotPrintMessage(e);
  }
  else
    if (u != NULL)
      gUserManager.DropUser(u);

  connect(this, SIGNAL(encodingChanged()), SLOT(slotSetEncoding()));
}

UserViewEvent::~UserViewEvent()
{
  // Empty
}

void UserViewEvent::generateReply()
{
  QString s = QString("> ");

  if (!mlvRead->markedText().trimmed().isEmpty())
    s += mlvRead->markedText().trimmed();
  else
    if (!mlvRead->toPlainText().trimmed().isEmpty())
      s += mlvRead->toPlainText().trimmed();
    else
      s = QString::null;

  s.replace("\n", "\n> ");
  s = s.trimmed();
  if (!s.isEmpty())
    s += "\n\n";

  sendMsg(s);
}

void UserViewEvent::sendMsg(QString text)
{
  UserSendMsgEvent* e = new UserSendMsgEvent(myUsers.front().c_str(), myPpid);

  e->setText(text);

  // Find a good position for the new window
  if (Config::Chat::instance()->autoPosReplyWin())
  {
    int yp = btnRead1->parentWidget()->mapToGlobal(QPoint(0, 0)).y();
    if (yp + e->height() + 8 > QApplication::desktop()->height())
      yp = QApplication::desktop()->height() - e->height() - 8;
    e->move(x(), yp);
  }

  QTimer::singleShot(10, e, SLOT(show()));

  connect(e, SIGNAL(autoCloseNotify()), SLOT(slotAutoClose()));
  connect(e, SIGNAL(msgTypeChanged(UserSendCommon*, UserSendCommon*)),
      SLOT(slotMsgTypeChanged(UserSendCommon*, UserSendCommon*)));
}

void UserViewEvent::updateNextButton()
{
  int num = msgView->getNumUnread();
  MessageListItem* e = msgView->getNextUnread();

  btnReadNext->setEnabled(num > 0);

  if (num > 1)
    btnReadNext->setText(tr("Nex&t (%1)").arg(num));
  else if (num == 1)
    btnReadNext->setText(tr("Nex&t"));

  if (e != NULL && e->msg() != NULL)
    btnReadNext->setIcon(IconManager::instance()->iconForEvent(e->msg()->SubCommand()));
}

void UserViewEvent::userUpdated(CICQSignal* sig, QString id, unsigned long ppid)
{
  ICQUser* u = gUserManager.FetchUser(id.toLatin1(), ppid, LOCK_R);

  if (u == 0)
    return;

  if (sig->SubSignal() == USER_EVENTS)
  {
    if (sig->Argument() > 0)
    {
      int eventId = sig->Argument();
      CUserEvent* e = u->EventPeekId(eventId);
      // Making sure we didn't handle this message already.
      if (e != NULL && myHighestEventId < eventId &&
          (!Config::Chat::instance()->msgChatView() ||
           (e->SubCommand() != ICQ_CMDxSUB_MSG &&
            e->SubCommand() != ICQ_CMDxSUB_URL)))
      {
         myHighestEventId = eventId;
         MessageListItem* m = new MessageListItem(e, myCodec, msgView);
         msgView->scrollToItem(m);
      }
    }

    if (sig->Argument() != 0)
      updateNextButton();
  }

  gUserManager.DropUser(u);
}

void UserViewEvent::slotAutoClose()
{
  if (!chkAutoClose->isChecked())
    return;

  ICQUser* u = gUserManager.FetchUser(myUsers.front().c_str(), myPpid, LOCK_R);

  bool doclose = false;

  if (u != NULL)
  {
    doclose = (u->NewMessages() == 0);
    gUserManager.DropUser(u);
  }

  if (doclose)
    close();
}

void UserViewEvent::slotBtnRead1()
{
  if (currentEvent == 0)
    return;

  switch (currentEvent->SubCommand())
  {
    case ICQ_CMDxSUB_MSG:  // reply/quote
    case ICQ_CMDxSUB_URL:
    case ICQ_CMDxSUB_CHAT:
    case ICQ_CMDxSUB_FILE:
      sendMsg("");
      break;

    case ICQ_CMDxSUB_AUTHxREQUEST:
    {
      CEventAuthRequest* p = dynamic_cast<CEventAuthRequest*>(currentEvent);
      new AuthUserDlg(p->IdString(), p->PPID(), true);
      break;
    }

    case ICQ_CMDxSUB_AUTHxGRANTED:
    {
      CEventAuthGranted* p = dynamic_cast<CEventAuthGranted*>(currentEvent);
      gLicqDaemon->AddUserToList(p->IdString(), p->PPID());
      break;
    }

    case ICQ_CMDxSUB_ADDEDxTOxLIST:
    {
      CEventAdded* p = dynamic_cast<CEventAdded*>(currentEvent);
      gLicqDaemon->AddUserToList(p->IdString(), p->PPID());
      break;
    }

    case ICQ_CMDxSUB_CONTACTxLIST:
    {
      const ContactList& cl = dynamic_cast<CEventContactList*>(currentEvent)->Contacts();
      ContactList::const_iterator it;

      for (it = cl.begin(); it != cl.end(); ++it)
      {
        ICQUser* u = gUserManager.FetchUser((*it)->IdString(), (*it)->PPID(), LOCK_R);
        if (u == NULL)
        {
          gLicqDaemon->AddUserToList((*it)->IdString(), (*it)->PPID());
          continue;
        }
        gUserManager.DropUser(u);
      }

      btnRead1->setEnabled(false);
      break;
    }

    case ICQ_CMDxSUB_EMAILxALERT:
    {
      // FIXME: For now assume MSN protocol, will need to be fixed soon.
      CEventEmailAlert* p = dynamic_cast<CEventEmailAlert*>(currentEvent);

      // Create the HTML
      QString url = BASE_DIR;
      url += "/.msn_email.html";

      QString strUser = p->To();
      QString strHTML = QString(
          "<html><head><noscript><meta http-equiv=Refresh content=\"0; url=http://www.hotmail.com\">"
          "</noscript></head><body onload=\"document.pform.submit(); \"><form name=\"pform\" action=\""
          "%1\" method=\"POST\"><input type=\"hidden\" name=\"mode\" value=\"ttl\">"
          "<input type=\"hidden\" name=\"login\" value=\"%2\"><input type=\"hidden\" name=\"username\""
          "value=\"%3\"><input type=\"hidden\" name=\"sid\" value=\"%4\"><input type=\"hidden\" name=\"kv\" value=\""
          "%5\"><input type=\"hidden\" name=\"id\" value=\"%6\"><input type=\"hidden\" name=\"sl\" value=\"9\"><input "
          "type=\"hidden\" name=\"rru\" value=\"%7\"><input type=\"hidden\" name=\"auth\" value=\"%8\""
          "><input type=\"hidden\" name=\"creds\" value=\"%9\"><input type=\"hidden\" name=\"svc\" value=\"mail\">"
          "<input type=\"hidden\" name=\"js\"value=\"yes\"></form></body></html>")
        .arg(p->PostURL())
        .arg(strUser.left(strUser.indexOf("@")))
        .arg(strUser)
        .arg(p->SID())
        .arg(p->KV())
        .arg(p->Id())
        .arg(p->MsgURL())
        .arg(p->MSPAuth())
        .arg(p->Creds());

      QFile fileHTML(url);
      fileHTML.open(QIODevice::WriteOnly);
      fileHTML.write(strHTML.toAscii(), strHTML.length());
      fileHTML.close();

      // Now we have to add the file:// after it was created,
      // but before it is executed.
      url.prepend("file://");

#ifdef USE_KDE
      // If no URL viewer is set, use KDE default
      if (!gLicqDaemon->getUrlViewer())
        KToolInvocation::invokeBrowser(url);
      else
#endif
      {
        if (!gLicqDaemon->ViewUrl(url.toLocal8Bit().data()))
          WarnUser(this, tr("Licq is unable to start your browser and open the URL.\n"
                "You will need to start the browser and open the URL manually."));
      }
      break;
    }
  } // switch
}

void UserViewEvent::slotBtnRead2()
{
  if (currentEvent == NULL)
    return;

  switch (currentEvent->SubCommand())
  {
    case ICQ_CMDxSUB_MSG:  // quote
    case ICQ_CMDxSUB_URL:
      generateReply();
      break;

    case ICQ_CMDxSUB_CHAT:  // accept a chat request
    {
      currentEvent->SetPending(false);
      btnRead2->setEnabled(false);
      btnRead3->setEnabled(false);
      CEventChat* c = dynamic_cast<CEventChat*>(currentEvent);
      ChatDlg* chatDlg = new ChatDlg(myUsers.front().c_str(), myPpid);
      if (c->Port() != 0)  // Joining a multiparty chat (we connect to them)
      {
        // FIXME: must have been done in CICQDaemon
        if (chatDlg->StartAsClient(c->Port()))
          gLicqDaemon->icqChatRequestAccept(
              strtoul(myUsers.front().c_str(), NULL, 10),
              0, c->Clients(), c->Sequence(),
              c->MessageID(), c->IsDirect());
      }
      else  // single party (other side connects to us)
      {
        // FIXME: must have been done in CICQDaemon
        if (chatDlg->StartAsServer())
          gLicqDaemon->icqChatRequestAccept(
              strtoul(myUsers.front().c_str(), NULL, 10),
              chatDlg->LocalPort(), c->Clients(), c->Sequence(),
              c->MessageID(), c->IsDirect());
      }
      break;
    }

    case ICQ_CMDxSUB_FILE:  // accept a file transfer
    {
      currentEvent->SetPending(false);
      btnRead2->setEnabled(false);
      btnRead3->setEnabled(false);
      CEventFile* f = dynamic_cast<CEventFile*>(currentEvent);
      FileDlg* fileDlg = new FileDlg(myUsers.front().c_str(), myPpid);

      if (fileDlg->ReceiveFiles())
        // FIXME: must have been done in CICQDaemon
        gLicqDaemon->icqFileTransferAccept(
            strtoul(myUsers.front().c_str(), NULL, 10),
            fileDlg->LocalPort(), f->Sequence(), f->MessageID(), f->IsDirect(),
            f->FileDescription(), f->Filename(), f->FileSize());
      break;
    }

    case ICQ_CMDxSUB_AUTHxREQUEST:
    {
      CEventAuthRequest* p = dynamic_cast<CEventAuthRequest*>(currentEvent);
      new AuthUserDlg(p->IdString(), p->PPID(), false);
      break;
    }
  } // switch
}

void UserViewEvent::slotBtnRead3()
{
  if (currentEvent == NULL)
    return;

  switch (currentEvent->SubCommand())
  {
    case ICQ_CMDxSUB_MSG:  // Forward
    case ICQ_CMDxSUB_URL:
    {
      ForwardDlg* f = new ForwardDlg(currentEvent, this);
      f->show();
      break;
    }

    case ICQ_CMDxSUB_CHAT:  // refuse a chat request
    {
      RefuseDlg* r = new RefuseDlg(myUsers.front().c_str(), myPpid, tr("Chat"), this);

      if (r->exec())
      {
        currentEvent->SetPending(false);
        CEventChat* c = dynamic_cast<CEventChat*>(currentEvent);
        btnRead2->setEnabled(false);
        btnRead3->setEnabled(false);

        // FIXME: must have been done in CICQDaemon
        gLicqDaemon->icqChatRequestRefuse(
            strtoul(myUsers.front().c_str(), NULL, 10),
            myCodec->fromUnicode(r->RefuseMessage()), currentEvent->Sequence(),
            c->MessageID(), c->IsDirect());
      }
      delete r;
      break;
    }

    case ICQ_CMDxSUB_FILE:  // refuse a file transfer
    {
      RefuseDlg* r = new RefuseDlg(myUsers.front().c_str(), myPpid, tr("File Transfer"), this);

      if (r->exec())
      {
        currentEvent->SetPending(false);
        CEventFile* f = dynamic_cast<CEventFile*>(currentEvent);
        btnRead2->setEnabled(false);
        btnRead3->setEnabled(false);

        // FIXME: must have been done in CICQDaemon
        gLicqDaemon->icqFileTransferRefuse(
            strtoul(myUsers.front().c_str(), NULL, 10),
            myCodec->fromUnicode(r->RefuseMessage()), currentEvent->Sequence(),
            f->MessageID(), f->IsDirect());
      }
      delete r;
      break;
    }

    case ICQ_CMDxSUB_AUTHxREQUEST:
    {
      CEventAuthRequest* p = dynamic_cast<CEventAuthRequest*>(currentEvent);
      gLicqDaemon->AddUserToList(p->IdString(), p->PPID());
      break;
    }
  } // switch
}

void UserViewEvent::slotBtnRead4()
{
  if (currentEvent == NULL)
    return;

  switch (currentEvent->SubCommand())
  {
    case ICQ_CMDxSUB_MSG:
      LicqGui::instance()->showEventDialog(mnuUserSendChat, myUsers.front().c_str(), myPpid);
      break;

    case ICQ_CMDxSUB_CHAT:  // join to current chat
    {
      CEventChat* c = dynamic_cast<CEventChat*>(currentEvent);
      if (c->Port() != 0)  // Joining a multiparty chat (we connect to them)
      {
        ChatDlg* chatDlg = new ChatDlg(myUsers.front().c_str(), myPpid);
        // FIXME: must have been done in CICQDaemon
        if (chatDlg->StartAsClient(c->Port()))
          gLicqDaemon->icqChatRequestAccept(
              strtoul(myUsers.front().c_str(), NULL, 10),
              0, c->Clients(), c->Sequence(), c->MessageID(), c->IsDirect());
      }
      else  // single party (other side connects to us)
      {
        ChatDlg* chatDlg = NULL;
        JoinChatDlg* j = new JoinChatDlg(this);
        // FIXME: must have been done in CICQDaemon
        if (j->exec() && (chatDlg = j->JoinedChat()) != NULL)
          gLicqDaemon->icqChatRequestAccept(
              strtoul(myUsers.front().c_str(), NULL, 10),
              chatDlg->LocalPort(), c->Clients(), c->Sequence(), c->MessageID(), c->IsDirect());
        delete j;
      }
      break;
    }

    case ICQ_CMDxSUB_URL:   // view a url
      emit viewUrl(this, dynamic_cast<CEventUrl*>(currentEvent)->Url());
      break;
  } // switch
}

void UserViewEvent::slotBtnReadNext()
{
  MessageListItem* e = msgView->getNextUnread();

  updateNextButton();

  if (e != NULL)
  {
    msgView->setCurrentItem(e, 0);
    msgView->scrollToItem(e);
    slotPrintMessage(e);
  }
}

void UserViewEvent::slotClearEvent()
{
  ICQUser* u = gUserManager.FetchUser(myUsers.front().c_str(), myPpid, LOCK_W);

  if (u == NULL)
    return;

  u->EventClearId(currentEvent->Id());
  gUserManager.DropUser(u);
}

void UserViewEvent::slotClose()
{
  myDeleteUser = btnClose->modifiersWhenPressed() & Qt::ControlModifier;
  close();
}

void UserViewEvent::slotMsgTypeChanged(UserSendCommon* from, UserSendCommon* to)
{
  disconnect(from, SIGNAL(autoCloseNotify()), this, SLOT(slotAutoClose()));
  disconnect(from, SIGNAL(msgTypeChanged(UserSendCommon*, UserSendCommon*)),
      this, SLOT(slotMsgTypeChanged(UserSendCommon*, UserSendCommon*)));

  connect(to, SIGNAL(autoCloseNotify()), SLOT(slotAutoClose()));
  connect(to, SIGNAL(msgTypeChanged(UserSendCommon*, UserSendCommon*)),
      SLOT(slotMsgTypeChanged(UserSendCommon*, UserSendCommon*)));
}

void UserViewEvent::slotPrintMessage(QTreeWidgetItem* item)
{
  if (item == NULL)
    return;

  MessageListItem* e = dynamic_cast<MessageListItem*>(item);

  btnRead1->setText("");
  btnRead2->setText("");
  btnRead3->setText("");
  btnRead4->setText("");
  btnRead1->setEnabled(false);
  btnRead2->setEnabled(false);
  btnRead3->setEnabled(false);
  btnRead4->setEnabled(false);
  myEncoding->setEnabled(true);

  CUserEvent* m = e->msg();
  currentEvent = m;

  // Set the color for the message
  mlvRead->setBackground(QColor(m->Color()->BackRed(), m->Color()->BackGreen(), m->Color()->BackBlue()));
  mlvRead->setForeground(QColor(m->Color()->ForeRed(), m->Color()->ForeGreen(), m->Color()->ForeBlue()));

  // Set the text
  if (m->SubCommand() == ICQ_CMDxSUB_SMS)
     myMessageText = QString::fromUtf8(m->Text());
  else
     myMessageText = myCodec->toUnicode(m->Text());

  QString colorAttr;
  colorAttr.sprintf("#%02lx%02lx%02lx", m->Color()->ForeRed(), m->Color()->ForeGreen(), m->Color()->ForeBlue());
  mlvRead->setText("<font color=\"" + colorAttr + "\">" + MLView::toRichText(myMessageText, true) + "</font>");

  mlvRead->GotoHome();

  if (m->Direction() == D_RECEIVER &&
      (m->Command() == ICQ_CMDxTCP_START ||
       m->Command() == ICQ_CMDxRCV_SYSxMSGxONLINE ||
       m->Command() == ICQ_CMDxRCV_SYSxMSGxOFFLINE))
  {
    switch (m->SubCommand())
    {
      case ICQ_CMDxSUB_CHAT:  // accept or refuse a chat request
      case ICQ_CMDxSUB_FILE:  // accept or refuse a file transfer
        btnRead1->setText(tr("&Reply"));
        if (m->IsCancelled())
          mlvRead->append(tr("\n--------------------\nRequest was cancelled."));
        else
        {
          if (m->Pending())
          {
            btnRead2->setText(tr("A&ccept"));
            btnRead3->setText(tr("&Refuse"));
          }
          // If this is a chat, and we already have chats going, and this is
          // not a join request, then we can join
          if (m->SubCommand() == ICQ_CMDxSUB_CHAT &&
              ChatDlg::chatDlgs.size() > 0 &&
              dynamic_cast<CEventChat*>(m)->Port() == 0)
            btnRead4->setText(tr("&Join"));
        }
        break;

      case ICQ_CMDxSUB_MSG:
        btnRead1->setText(tr("&Reply"));
        btnRead2->setText(tr("&Quote"));
        btnRead3->setText(tr("&Forward"));
        btnRead4->setText(tr("Start Chat"));
        break;

      case ICQ_CMDxSUB_SMS:
        myEncoding->setEnabled(false);
        break;

      case ICQ_CMDxSUB_URL:   // view a url
        btnRead1->setText(tr("&Reply"));
        btnRead2->setText(tr("&Quote"));
        btnRead3->setText(tr("&Forward"));
#ifndef USE_KDE
        if (gLicqDaemon->getUrlViewer() != NULL)
#endif
          btnRead4->setText(tr("&View"));
        break;

      case ICQ_CMDxSUB_AUTHxREQUEST:
      {
        btnRead1->setText(tr("A&uthorize"));
        btnRead2->setText(tr("&Refuse"));
        CEventAuthRequest* pAuthReq = dynamic_cast<CEventAuthRequest*>(m);
        ICQUser* u = gUserManager.FetchUser(pAuthReq->IdString(), pAuthReq->PPID(), LOCK_R);
        if (u == NULL)
          btnRead3->setText(tr("A&dd User"));
        else
          gUserManager.DropUser(u);
        break;
      }

      case ICQ_CMDxSUB_AUTHxGRANTED:
      {
        CEventAuthGranted* pAuth = dynamic_cast<CEventAuthGranted*>(m);
        ICQUser* u = gUserManager.FetchUser(pAuth->IdString(), pAuth->PPID(), LOCK_R);
        if (u == NULL)
          btnRead1->setText(tr("A&dd User"));
        else
          gUserManager.DropUser(u);
        break;
      }

      case ICQ_CMDxSUB_ADDEDxTOxLIST:
      {
        CEventAdded* pAdd = dynamic_cast<CEventAdded*>(m);
        ICQUser* u = gUserManager.FetchUser(pAdd->IdString(), pAdd->PPID(), LOCK_R);
        if (u == NULL)
          btnRead1->setText(tr("A&dd User"));
        else
          gUserManager.DropUser(u);
        break;
      }

      case ICQ_CMDxSUB_CONTACTxLIST:
      {
        int s = dynamic_cast<CEventContactList*>(m)->Contacts().size();
        if (s > 1)
          btnRead1->setText(tr("A&dd %1 Users").arg(s));
        else
          if (s == 1)
            btnRead1->setText(tr("A&dd User"));
        break;
      }

      case ICQ_CMDxSUB_EMAILxALERT:
        btnRead1->setText(tr("&View Email"));
        break;
    } // switch
  }  // if

  if (!btnRead1->text().isEmpty())
    btnRead1->setEnabled(true);
  if (!btnRead2->text().isEmpty())
    btnRead2->setEnabled(true);
  if (!btnRead3->text().isEmpty())
    btnRead3->setEnabled(true);
  if (!btnRead4->text().isEmpty())
    btnRead4->setEnabled(true);

  btnRead1->setFocus();

  if (e->isUnread())
  {
    // clear the event only after all the slots have been invoked
    QTimer::singleShot(20, this, SLOT(slotClearEvent()));
    e->MarkRead();
  }
}

void UserViewEvent::slotSentEvent(ICQEvent* e)
{
  if (e->PPID() != myPpid || strcmp(myUsers.front().c_str(), e->Id()) != 0)
    return;

  if (!Config::Chat::instance()->msgChatView())
    new MessageListItem(e->GrabUserEvent(), myCodec, msgView);
}

void UserViewEvent::slotSetEncoding()
{
  // if we have an open view, just refresh it
  if (msgView != NULL)
    slotPrintMessage(msgView->currentItem());
}
