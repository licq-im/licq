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

#include <QAction>
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
#include "core/messagebox.h"
#include "core/signalmanager.h"

#include "dialogs/adduserdlg.h"
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
  myReadSplitter = new QSplitter(Qt::Vertical);
  myReadSplitter->setOpaqueResize();
  myMainWidget->addWidget(myReadSplitter);

  QShortcut* a = new QShortcut(Qt::Key_Escape, this);
  connect(a, SIGNAL(activated()), SLOT(closeDialog()));

  myMessageList = new MessageList();
  myReadSplitter->addWidget(myMessageList);

  myMessageView = new MLView();
  myMessageView->setSizeHintLines(8);
  myReadSplitter->addWidget(myMessageView);

  myReadSplitter->setStretchFactor(0, 0);
  myReadSplitter->setStretchFactor(1, 1);

  connect(myMessageList, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
      SLOT(printMessage(QTreeWidgetItem*)));
  connect(LicqGui::instance(), SIGNAL(eventSent(const ICQEvent*)),
      SLOT(sentEvent(const ICQEvent*)));

  myActionsBox = new QGroupBox();
  myMainWidget->addSpacing(10);
  myMainWidget->addWidget(myActionsBox);

  QHBoxLayout* h_action_lay = new QHBoxLayout(myActionsBox);

  myRead1Button = new QPushButton();
  myRead2Button = new QPushButton();
  myRead3Button = new QPushButton();
  myRead4Button = new QPushButton();

  myRead1Button->setEnabled(false);
  myRead2Button->setEnabled(false);
  myRead3Button->setEnabled(false);
  myRead4Button->setEnabled(false);

  connect(myRead1Button, SIGNAL(clicked()), SLOT(read1()));
  connect(myRead2Button, SIGNAL(clicked()), SLOT(read2()));
  connect(myRead3Button, SIGNAL(clicked()), SLOT(read3()));
  connect(myRead4Button, SIGNAL(clicked()), SLOT(read4()));

  h_action_lay->addWidget(myRead1Button);
  h_action_lay->addWidget(myRead2Button);
  h_action_lay->addWidget(myRead3Button);
  h_action_lay->addWidget(myRead4Button);

  QHBoxLayout* h_lay = new QHBoxLayout();
  myTopLayout->addLayout(h_lay);

  if (!myIsOwner)
  {
    myAutoCloseCheck = new QCheckBox(tr("Aut&o Close"));
    myAutoCloseCheck->setChecked(Config::Chat::instance()->autoClose());
    h_lay->addWidget(myAutoCloseCheck);
  }

  h_lay->addStretch(1);

  myReadNextButton = new QPushButton(tr("Nex&t"));
  myReadNextButton->setEnabled(false);
  connect(myReadNextButton, SIGNAL(clicked()), SLOT(readNext()));
  h_lay->addWidget(myReadNextButton);
  setTabOrder(myRead4Button, myReadNextButton);

  myCloseButton = new SkinnableButton(tr("&Close"));
  myCloseButton->setToolTip(tr("Normal Click - Close Window\n<CTRL>+Click - also delete User"));
  connect(myCloseButton, SIGNAL(clicked()), SLOT(closeDialog()));
  h_lay->addWidget(myCloseButton);
  setTabOrder(myReadNextButton, myCloseButton);

  const ICQUser* u = gUserManager.FetchUser(myUsers.front().c_str(), myPpid, LOCK_R);
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

    MessageListItem* e = new MessageListItem(u->EventPeek(i), myCodec, myMessageList);
    myHighestEventId = u->EventPeek(i)->Id();

    /* Create items for all the messages which already await
     * in the queue. We cannot rely on getting CICQSignals for them
     * since they might've arrived before the dialog appeared,
     * possibly being undisplayed messages from previous licq session.
     */
    for (i++; i < u->NewMessages(); i++)
    {
      const CUserEvent* event = u->EventPeek(i);
      if (!Config::Chat::instance()->msgChatView() ||
          (event->SubCommand() != ICQ_CMDxSUB_MSG &&
           event->SubCommand() != ICQ_CMDxSUB_URL))
      {
        new MessageListItem(event, myCodec, myMessageList);
        // Make sure we don't add this message again,
        // even if we receive an userUpdated signal for it.
        if (myHighestEventId < event->Id())
          myHighestEventId = event->Id();
      }
    }

    gUserManager.DropUser(u);
    for (unsigned short i = 0; i < myMessageList->columnCount(); i++)
      myMessageList->resizeColumnToContents(i);
    myMessageList->setCurrentItem(e, 0);
    myMessageList->scrollToItem(e);
    printMessage(e);
  }
  else
    if (u != NULL)
      gUserManager.DropUser(u);

  connect(this, SIGNAL(encodingChanged()), SLOT(setEncoding()));
}

UserViewEvent::~UserViewEvent()
{
  // Empty
}

void UserViewEvent::generateReply()
{
  QString s = QString("> ");

  if (!myMessageView->markedText().trimmed().isEmpty())
    s += myMessageView->markedText().trimmed();
  else
    if (!myMessageView->toPlainText().trimmed().isEmpty())
      s += myMessageView->toPlainText().trimmed();
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
    int yp = myRead1Button->parentWidget()->mapToGlobal(QPoint(0, 0)).y();
    if (yp + e->height() + 8 > QApplication::desktop()->height())
      yp = QApplication::desktop()->height() - e->height() - 8;
    e->move(x(), yp);
  }

  QTimer::singleShot(10, e, SLOT(show()));

  connect(e, SIGNAL(autoCloseNotify()), SLOT(autoClose()));
  connect(e, SIGNAL(msgTypeChanged(UserSendCommon*, UserSendCommon*)),
      SLOT(msgTypeChanged(UserSendCommon*, UserSendCommon*)));
}

void UserViewEvent::updateNextButton()
{
  int num = myMessageList->getNumUnread();
  MessageListItem* e = myMessageList->getNextUnread();

  myReadNextButton->setEnabled(num > 0);

  if (num > 1)
    myReadNextButton->setText(tr("Nex&t (%1)").arg(num));
  else if (num == 1)
    myReadNextButton->setText(tr("Nex&t"));

  if (e != NULL && e->msg() != NULL)
    myReadNextButton->setIcon(IconManager::instance()->iconForEvent(e->msg()->SubCommand()));
}

void UserViewEvent::userUpdated(CICQSignal* sig, QString id, unsigned long ppid)
{
  const ICQUser* u = gUserManager.FetchUser(id.toLatin1(), ppid, LOCK_R);

  if (u == 0)
    return;

  if (sig->SubSignal() == USER_EVENTS)
  {
    if (sig->Argument() > 0)
    {
      int eventId = sig->Argument();
      const CUserEvent* e = u->EventPeekId(eventId);
      // Making sure we didn't handle this message already.
      if (e != NULL && myHighestEventId < eventId &&
          (!Config::Chat::instance()->msgChatView() ||
           (e->SubCommand() != ICQ_CMDxSUB_MSG &&
            e->SubCommand() != ICQ_CMDxSUB_URL)))
      {
         myHighestEventId = eventId;
         MessageListItem* m = new MessageListItem(e, myCodec, myMessageList);
         myMessageList->scrollToItem(m);
      }
    }

    if (sig->Argument() != 0)
      updateNextButton();
  }

  gUserManager.DropUser(u);
}

void UserViewEvent::autoClose()
{
  if (!myAutoCloseCheck->isChecked())
    return;

  const ICQUser* u = gUserManager.FetchUser(myUsers.front().c_str(), myPpid, LOCK_R);

  bool doclose = false;

  if (u != NULL)
  {
    doclose = (u->NewMessages() == 0);
    gUserManager.DropUser(u);
  }

  if (doclose)
    closeDialog();
}

void UserViewEvent::read1()
{
  if (myCurrentEvent == 0)
    return;

  switch (myCurrentEvent->SubCommand())
  {
    case ICQ_CMDxSUB_MSG:  // reply/quote
    case ICQ_CMDxSUB_URL:
    case ICQ_CMDxSUB_CHAT:
    case ICQ_CMDxSUB_FILE:
      sendMsg("");
      break;

    case ICQ_CMDxSUB_AUTHxREQUEST:
    {
      CEventAuthRequest* p = dynamic_cast<CEventAuthRequest*>(myCurrentEvent);
      new AuthUserDlg(p->IdString(), p->PPID(), true);
      break;
    }

    case ICQ_CMDxSUB_AUTHxGRANTED:
    {
      CEventAuthGranted* p = dynamic_cast<CEventAuthGranted*>(myCurrentEvent);
      new AddUserDlg(p->IdString(), p->PPID(), this);
      break;
    }

    case ICQ_CMDxSUB_ADDEDxTOxLIST:
    {
      CEventAdded* p = dynamic_cast<CEventAdded*>(myCurrentEvent);
      new AddUserDlg(p->IdString(), p->PPID(), this);
      break;
    }

    case ICQ_CMDxSUB_CONTACTxLIST:
    {
      const ContactList& cl = dynamic_cast<CEventContactList*>(myCurrentEvent)->Contacts();
      ContactList::const_iterator it;

      for (it = cl.begin(); it != cl.end(); ++it)
      {
        new AddUserDlg((*it)->IdString(), (*it)->PPID(), this);
      }

      myRead1Button->setEnabled(false);
      break;
    }

    case ICQ_CMDxSUB_EMAILxALERT:
    {
      // FIXME: For now assume MSN protocol, will need to be fixed soon.
      CEventEmailAlert* p = dynamic_cast<CEventEmailAlert*>(myCurrentEvent);

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

      LicqGui::instance()->viewUrl(url);

      break;
    }
  } // switch
}

void UserViewEvent::read2()
{
  if (myCurrentEvent == NULL)
    return;

  switch (myCurrentEvent->SubCommand())
  {
    case ICQ_CMDxSUB_MSG:  // quote
    case ICQ_CMDxSUB_URL:
      generateReply();
      break;

    case ICQ_CMDxSUB_CHAT:  // accept a chat request
    {
      myCurrentEvent->SetPending(false);
      myRead2Button->setEnabled(false);
      myRead3Button->setEnabled(false);
      CEventChat* c = dynamic_cast<CEventChat*>(myCurrentEvent);
      ChatDlg* chatDlg = new ChatDlg(myUsers.front().c_str(), myPpid);
      if (c->Port() != 0)  // Joining a multiparty chat (we connect to them)
      {
        // FIXME: must have been done in CICQDaemon
        if (chatDlg->StartAsClient(c->Port()))
          gLicqDaemon->icqChatRequestAccept(
              myUsers.front().c_str(),
              0, c->Clients(), c->Sequence(),
              c->MessageID(), c->IsDirect());
      }
      else  // single party (other side connects to us)
      {
        // FIXME: must have been done in CICQDaemon
        if (chatDlg->StartAsServer())
          gLicqDaemon->icqChatRequestAccept(
              myUsers.front().c_str(),
              chatDlg->LocalPort(), c->Clients(), c->Sequence(),
              c->MessageID(), c->IsDirect());
      }
      break;
    }

    case ICQ_CMDxSUB_FILE:  // accept a file transfer
    {
      myCurrentEvent->SetPending(false);
      myRead2Button->setEnabled(false);
      myRead3Button->setEnabled(false);
      CEventFile* f = dynamic_cast<CEventFile*>(myCurrentEvent);
      FileDlg* fileDlg = new FileDlg(myUsers.front().c_str(), myPpid);

      if (fileDlg->ReceiveFiles())
        // FIXME: must have been done in CICQDaemon
        gLicqDaemon->icqFileTransferAccept(
            myUsers.front().c_str(),
            fileDlg->LocalPort(), f->Sequence(), f->MessageID(), f->IsDirect(),
            f->FileDescription(), f->Filename(), f->FileSize());
      break;
    }

    case ICQ_CMDxSUB_AUTHxREQUEST:
    {
      CEventAuthRequest* p = dynamic_cast<CEventAuthRequest*>(myCurrentEvent);
      new AuthUserDlg(p->IdString(), p->PPID(), false);
      break;
    }
  } // switch
}

void UserViewEvent::read3()
{
  if (myCurrentEvent == NULL)
    return;

  switch (myCurrentEvent->SubCommand())
  {
    case ICQ_CMDxSUB_MSG:  // Forward
    case ICQ_CMDxSUB_URL:
    {
      ForwardDlg* f = new ForwardDlg(myCurrentEvent, this);
      f->show();
      break;
    }

    case ICQ_CMDxSUB_CHAT:  // refuse a chat request
    {
      RefuseDlg* r = new RefuseDlg(myUsers.front().c_str(), myPpid, tr("Chat"), this);

      if (r->exec())
      {
        myCurrentEvent->SetPending(false);
        CEventChat* c = dynamic_cast<CEventChat*>(myCurrentEvent);
        myRead2Button->setEnabled(false);
        myRead3Button->setEnabled(false);

        // FIXME: must have been done in CICQDaemon
        gLicqDaemon->icqChatRequestRefuse(
            myUsers.front().c_str(),
            myCodec->fromUnicode(r->RefuseMessage()), myCurrentEvent->Sequence(),
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
        myCurrentEvent->SetPending(false);
        CEventFile* f = dynamic_cast<CEventFile*>(myCurrentEvent);
        myRead2Button->setEnabled(false);
        myRead3Button->setEnabled(false);

        // FIXME: must have been done in CICQDaemon
        gLicqDaemon->icqFileTransferRefuse(
            myUsers.front().c_str(),
            myCodec->fromUnicode(r->RefuseMessage()), myCurrentEvent->Sequence(),
            f->MessageID(), f->IsDirect());
      }
      delete r;
      break;
    }

    case ICQ_CMDxSUB_AUTHxREQUEST:
    {
      CEventAuthRequest* p = dynamic_cast<CEventAuthRequest*>(myCurrentEvent);
      new AddUserDlg(p->IdString(), p->PPID(), this);
      break;
    }
  } // switch
}

void UserViewEvent::read4()
{
  if (myCurrentEvent == NULL)
    return;

  switch (myCurrentEvent->SubCommand())
  {
    case ICQ_CMDxSUB_MSG:
      LicqGui::instance()->showEventDialog(ChatEvent, myUsers.front().c_str(), myPpid);
      break;

    case ICQ_CMDxSUB_CHAT:  // join to current chat
    {
      CEventChat* c = dynamic_cast<CEventChat*>(myCurrentEvent);
      if (c->Port() != 0)  // Joining a multiparty chat (we connect to them)
      {
        ChatDlg* chatDlg = new ChatDlg(myUsers.front().c_str(), myPpid);
        // FIXME: must have been done in CICQDaemon
        if (chatDlg->StartAsClient(c->Port()))
          gLicqDaemon->icqChatRequestAccept(
              myUsers.front().c_str(),
              0, c->Clients(), c->Sequence(), c->MessageID(), c->IsDirect());
      }
      else  // single party (other side connects to us)
      {
        ChatDlg* chatDlg = NULL;
        JoinChatDlg* j = new JoinChatDlg(this);
        // FIXME: must have been done in CICQDaemon
        if (j->exec() && (chatDlg = j->JoinedChat()) != NULL)
          gLicqDaemon->icqChatRequestAccept(
              myUsers.front().c_str(),
              chatDlg->LocalPort(), c->Clients(), c->Sequence(), c->MessageID(), c->IsDirect());
        delete j;
      }
      break;
    }

    case ICQ_CMDxSUB_URL:   // view a url
      LicqGui::instance()->viewUrl(dynamic_cast<CEventUrl*>(myCurrentEvent)->Url());
      break;

    case ICQ_CMDxSUB_AUTHxREQUEST: // Fall through
    case ICQ_CMDxSUB_AUTHxGRANTED:
    case ICQ_CMDxSUB_ADDEDxTOxLIST:
    {
      const char* id;
      unsigned long ppid;
#define GETINFO(sub, type) \
      if (myCurrentEvent->SubCommand() == sub) \
      { \
        type* p = dynamic_cast<type*>(myCurrentEvent); \
        id = p->IdString(); \
        ppid = p->PPID(); \
      }

      GETINFO(ICQ_CMDxSUB_AUTHxREQUEST, CEventAuthRequest);
      GETINFO(ICQ_CMDxSUB_AUTHxGRANTED, CEventAuthGranted);
      GETINFO(ICQ_CMDxSUB_ADDEDxTOxLIST, CEventAdded);
#undef GETINFO

      const ICQUser* u = gUserManager.FetchUser(id, ppid, LOCK_R);
      if (u == NULL)
        gLicqDaemon->AddUserToList(id, ppid, false, true);
      else
        gUserManager.DropUser(u);

      LicqGui::instance()->showInfoDialog(mnuUserGeneral, id, ppid, false, true);
      break;
    }
  } // switch
}

void UserViewEvent::readNext()
{
  MessageListItem* e = myMessageList->getNextUnread();

  updateNextButton();

  if (e != NULL)
  {
    myMessageList->setCurrentItem(e, 0);
    myMessageList->scrollToItem(e);
    printMessage(e);
  }
}

void UserViewEvent::clearEvent()
{
  ICQUser* u = gUserManager.FetchUser(myUsers.front().c_str(), myPpid, LOCK_W);

  if (u == NULL)
    return;

  u->EventClearId(myCurrentEvent->Id());
  gUserManager.DropUser(u);
}

void UserViewEvent::closeDialog()
{
  myDeleteUser = myCloseButton->modifiersWhenPressed() & Qt::ControlModifier;
  close();
}

void UserViewEvent::msgTypeChanged(UserSendCommon* from, UserSendCommon* to)
{
  disconnect(from, SIGNAL(autoCloseNotify()), this, SLOT(autoClose()));
  disconnect(from, SIGNAL(msgTypeChanged(UserSendCommon*, UserSendCommon*)),
      this, SLOT(msgTypeChanged(UserSendCommon*, UserSendCommon*)));

  connect(to, SIGNAL(autoCloseNotify()), SLOT(autoClose()));
  connect(to, SIGNAL(msgTypeChanged(UserSendCommon*, UserSendCommon*)),
      SLOT(msgTypeChanged(UserSendCommon*, UserSendCommon*)));
}

void UserViewEvent::printMessage(QTreeWidgetItem* item)
{
  if (item == NULL)
    return;

  MessageListItem* e = dynamic_cast<MessageListItem*>(item);

  myRead1Button->setText("");
  myRead2Button->setText("");
  myRead3Button->setText("");
  myRead4Button->setText("");
  myEncoding->setEnabled(true);

  CUserEvent* m = e->msg();
  myCurrentEvent = m;

  // Set the color for the message
  myMessageView->setBackground(QColor(m->Color()->BackRed(), m->Color()->BackGreen(), m->Color()->BackBlue()));
  myMessageView->setForeground(QColor(m->Color()->ForeRed(), m->Color()->ForeGreen(), m->Color()->ForeBlue()));

  // Set the text
  if (m->SubCommand() == ICQ_CMDxSUB_SMS)
     myMessageText = QString::fromUtf8(m->Text());
  else
     myMessageText = myCodec->toUnicode(m->Text());

  QString colorAttr;
  colorAttr.sprintf("#%02lx%02lx%02lx", m->Color()->ForeRed(), m->Color()->ForeGreen(), m->Color()->ForeBlue());
  myMessageView->setText("<font color=\"" + colorAttr + "\">" + MLView::toRichText(myMessageText, true) + "</font>");

  myMessageView->GotoHome();

  if (m->Direction() == D_RECEIVER &&
      (m->Command() == ICQ_CMDxTCP_START ||
       m->Command() == ICQ_CMDxRCV_SYSxMSGxONLINE ||
       m->Command() == ICQ_CMDxRCV_SYSxMSGxOFFLINE))
  {
    switch (m->SubCommand())
    {
      case ICQ_CMDxSUB_CHAT:  // accept or refuse a chat request
      case ICQ_CMDxSUB_FILE:  // accept or refuse a file transfer
        myRead1Button->setText(tr("&Reply"));
        if (m->IsCancelled())
          myMessageView->append(tr("\n--------------------\nRequest was cancelled."));
        else
        {
          if (m->Pending())
          {
            myRead2Button->setText(tr("A&ccept"));
            myRead3Button->setText(tr("&Refuse"));
          }
          // If this is a chat, and we already have chats going, and this is
          // not a join request, then we can join
          if (m->SubCommand() == ICQ_CMDxSUB_CHAT &&
              ChatDlg::chatDlgs.size() > 0 &&
              dynamic_cast<CEventChat*>(m)->Port() == 0)
            myRead4Button->setText(tr("&Join"));
        }
        break;

      case ICQ_CMDxSUB_MSG:
        myRead1Button->setText(tr("&Reply"));
        myRead2Button->setText(tr("&Quote"));
        myRead3Button->setText(tr("&Forward"));
        myRead4Button->setText(tr("Start Chat"));
        break;

      case ICQ_CMDxSUB_SMS:
        myEncoding->setEnabled(false);
        break;

      case ICQ_CMDxSUB_URL:   // view a url
        myRead1Button->setText(tr("&Reply"));
        myRead2Button->setText(tr("&Quote"));
        myRead3Button->setText(tr("&Forward"));
#ifndef USE_KDE
        if (gLicqDaemon->getUrlViewer() != NULL)
#endif
          myRead4Button->setText(tr("&View"));
        break;

      case ICQ_CMDxSUB_AUTHxREQUEST:
      {
        myRead1Button->setText(tr("A&uthorize"));
        myRead2Button->setText(tr("&Refuse"));
        CEventAuthRequest* pAuthReq = dynamic_cast<CEventAuthRequest*>(m);
        const ICQUser* u = gUserManager.FetchUser(pAuthReq->IdString(), pAuthReq->PPID(), LOCK_R);
        if (u == NULL)
          myRead3Button->setText(tr("A&dd User"));
        else
          gUserManager.DropUser(u);
        myRead4Button->setText(tr("&View Info"));
        break;
      }

      case ICQ_CMDxSUB_AUTHxGRANTED:
      {
        CEventAuthGranted* pAuth = dynamic_cast<CEventAuthGranted*>(m);
        const ICQUser* u = gUserManager.FetchUser(pAuth->IdString(), pAuth->PPID(), LOCK_R);
        if (u == NULL)
          myRead1Button->setText(tr("A&dd User"));
        else
          gUserManager.DropUser(u);
        myRead4Button->setText(tr("&View Info"));
        break;
      }

      case ICQ_CMDxSUB_ADDEDxTOxLIST:
      {
        CEventAdded* pAdd = dynamic_cast<CEventAdded*>(m);
        const ICQUser* u = gUserManager.FetchUser(pAdd->IdString(), pAdd->PPID(), LOCK_R);
        if (u == NULL)
          myRead1Button->setText(tr("A&dd User"));
        else
          gUserManager.DropUser(u);
        myRead4Button->setText(tr("&View Info"));
        break;
      }

      case ICQ_CMDxSUB_CONTACTxLIST:
      {
        int s = dynamic_cast<CEventContactList*>(m)->Contacts().size();
        if (s > 1)
          myRead1Button->setText(tr("A&dd %1 Users").arg(s));
        else
          if (s == 1)
            myRead1Button->setText(tr("A&dd User"));
        break;
      }

      case ICQ_CMDxSUB_EMAILxALERT:
        myRead1Button->setText(tr("&View Email"));
        break;
    } // switch
  }  // if

  myRead1Button->setEnabled(!myRead1Button->text().isEmpty());
  myRead2Button->setEnabled(!myRead2Button->text().isEmpty());
  myRead3Button->setEnabled(!myRead3Button->text().isEmpty());
  myRead4Button->setEnabled(!myRead4Button->text().isEmpty());

  myActionsBox->setVisible(
      myRead1Button->isEnabled() || myRead2Button->isEnabled() ||
      myRead3Button->isEnabled() || myRead4Button->isEnabled());

  myRead1Button->setFocus();

  if (e->isUnread())
  {
    // clear the event only after all the slots have been invoked
    QTimer::singleShot(20, this, SLOT(clearEvent()));
    e->MarkRead();
  }
}

void UserViewEvent::sentEvent(const ICQEvent* e)
{
  if (e->PPID() != myPpid || strcmp(myUsers.front().c_str(), e->Id()) != 0)
    return;

  if (!Config::Chat::instance()->msgChatView())
    new MessageListItem(e->UserEvent(), myCodec, myMessageList);
}

void UserViewEvent::setEncoding()
{
  // if we have an open view, just refresh it
  if (myMessageList != NULL)
    printMessage(myMessageList->currentItem());
}
