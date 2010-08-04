// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2010 Licq developers
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

#include <licq/contactlist/usermanager.h>
#include <licq/daemon.h>
#include <licq/event.h>
#include <licq/icq.h>
#include <licq/icqdefines.h>
#include <licq/pluginsignal.h>
#include <licq/protocolmanager.h>
#include <licq/userevents.h>

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

using Licq::gProtocolManager;
using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::UserViewEvent */

UserViewEvent::UserViewEvent(const Licq::UserId& userId, QWidget* parent)
  : UserEventCommon(userId, parent, "UserViewEvent")
{
  myReadSplitter = new QSplitter(Qt::Vertical);
  myReadSplitter->setOpaqueResize();
  myMainWidget->addWidget(myReadSplitter);

  QShortcut* shortcutEscape = new QShortcut(Qt::Key_Escape, this);

  myMessageList = new MessageList();
  myReadSplitter->addWidget(myMessageList);

  myMessageView = new MLView();
  myMessageView->setSizeHintLines(8);
  myReadSplitter->addWidget(myMessageView);

  myReadSplitter->setStretchFactor(0, 0);
  myReadSplitter->setStretchFactor(1, 1);

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
  h_lay->addWidget(myReadNextButton);
  setTabOrder(myRead4Button, myReadNextButton);

  myCloseButton = new SkinnableButton(tr("&Close"));
  myCloseButton->setToolTip(tr("Normal Click - Close Window\n<CTRL>+Click - also delete User"));
  h_lay->addWidget(myCloseButton);
  setTabOrder(myReadNextButton, myCloseButton);

  Licq::UserReadGuard u(myUsers.front());
  if (u.isLocked() && u->NewMessages() > 0)
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
      const Licq::UserEvent* event = u->EventPeek(i);
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

    u.unlock();
    for (int i = 0; i < myMessageList->columnCount(); i++)
      myMessageList->resizeColumnToContents(i);
    myMessageList->setCurrentItem(e, 0);
    myMessageList->scrollToItem(e);
    printMessage(e);
  }
  u.unlock();

  QSize dialogSize = Config::Chat::instance()->viewDialogSize();
  if (dialogSize.isValid())
    resize(dialogSize);

  connect(gLicqGui, SIGNAL(eventSent(const Licq::Event*)),
      SLOT(sentEvent(const Licq::Event*)));
  connect(myMessageList, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
      SLOT(printMessage(QTreeWidgetItem*)));
  connect(myRead1Button, SIGNAL(clicked()), SLOT(read1()));
  connect(myRead2Button, SIGNAL(clicked()), SLOT(read2()));
  connect(myRead3Button, SIGNAL(clicked()), SLOT(read3()));
  connect(myRead4Button, SIGNAL(clicked()), SLOT(read4()));
  connect(myReadNextButton, SIGNAL(clicked()), SLOT(readNext()));
  connect(myCloseButton, SIGNAL(clicked()), SLOT(closeDialog()));
  connect(shortcutEscape, SIGNAL(activated()), SLOT(closeDialog()));
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
  UserSendMsgEvent* e = new UserSendMsgEvent(myUsers.front());

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
  else
    myReadNextButton->setIcon(QIcon());
}

void UserViewEvent::userUpdated(const Licq::UserId& userId, unsigned long subSignal, int argument, unsigned long /* cid */)
{
  Licq::UserReadGuard u(userId);

  if (!u.isLocked())
    return;

  if (subSignal == Licq::PluginSignal::UserEvents)
  {
    if (argument > 0)
    {
      int eventId = argument;
      const Licq::UserEvent* e = u->EventPeekId(eventId);
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

    if (argument != 0)
      updateNextButton();
  }
}

void UserViewEvent::autoClose()
{
  if (!myAutoCloseCheck->isChecked())
    return;

  bool doclose = false;

  {
    Licq::UserReadGuard u(myUsers.front());
    if (u.isLocked())
      doclose = (u->NewMessages() == 0);
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
      Licq::EventAuthRequest* p = dynamic_cast<Licq::EventAuthRequest*>(myCurrentEvent);
      new AuthUserDlg(p->userId(), true);
      break;
    }

    case ICQ_CMDxSUB_AUTHxGRANTED:
    {
      Licq::EventAuthGranted* p = dynamic_cast<Licq::EventAuthGranted*>(myCurrentEvent);
      new AddUserDlg(p->userId(), this);
      break;
    }

    case ICQ_CMDxSUB_ADDEDxTOxLIST:
    {
      Licq::EventAdded* p = dynamic_cast<Licq::EventAdded*>(myCurrentEvent);
      new AddUserDlg(p->userId(), this);
      break;
    }

    case ICQ_CMDxSUB_CONTACTxLIST:
    {
      const Licq::EventContactList::ContactList& cl = dynamic_cast<Licq::EventContactList*>(myCurrentEvent)->Contacts();
      Licq::EventContactList::ContactList::const_iterator it;

      for (it = cl.begin(); it != cl.end(); ++it)
      {
        new AddUserDlg((*it)->userId(), this);
      }

      myRead1Button->setEnabled(false);
      break;
    }

    case ICQ_CMDxSUB_EMAILxALERT:
    {
      // FIXME: For now assume MSN protocol, will need to be fixed soon.
      Licq::EventEmailAlert* p = dynamic_cast<Licq::EventEmailAlert*>(myCurrentEvent);

      // Create the HTML
      QString url = Licq::gDaemon.baseDir().c_str();
      url += ".msn_email.html";

      QString strUser = p->to().c_str();
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
        .arg(p->postUrl().c_str())
        .arg(strUser.left(strUser.indexOf("@")))
        .arg(strUser)
        .arg(p->sid().c_str())
        .arg(p->kv().c_str())
        .arg(p->id().c_str())
        .arg(p->msgUrl().c_str())
        .arg(p->mspAuth().c_str())
        .arg(p->creds().c_str());

      QFile fileHTML(url);
      fileHTML.open(QIODevice::WriteOnly);
      fileHTML.write(strHTML.toAscii(), strHTML.length());
      fileHTML.close();

      // Now we have to add the file:// after it was created,
      // but before it is executed.
      url.prepend("file://");

      gLicqGui->viewUrl(url);

      break;
    }
  } // switch
}

void UserViewEvent::read2()
{
  if (myCurrentEvent == NULL)
    return;

  QString accountId = myUsers.front().accountId().c_str();

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
      Licq::EventChat* c = dynamic_cast<Licq::EventChat*>(myCurrentEvent);
      ChatDlg* chatDlg = new ChatDlg(myUsers.front());
      if (c->Port() != 0)  // Joining a multiparty chat (we connect to them)
      {
        // FIXME: must have been done in CICQDaemon
        if (chatDlg->StartAsClient(c->Port()))
          gLicqDaemon->icqChatRequestAccept(
              myUsers.front(),
              0, c->clients(), c->Sequence(),
              c->MessageID(), c->IsDirect());
      }
      else  // single party (other side connects to us)
      {
        // FIXME: must have been done in CICQDaemon
        if (chatDlg->StartAsServer())
          gLicqDaemon->icqChatRequestAccept(
              myUsers.front(),
              chatDlg->LocalPort(), c->clients(), c->Sequence(),
              c->MessageID(), c->IsDirect());
      }
      break;
    }

    case ICQ_CMDxSUB_FILE:  // accept a file transfer
    {
      myCurrentEvent->SetPending(false);
      myRead2Button->setEnabled(false);
      myRead3Button->setEnabled(false);
      Licq::EventFile* f = dynamic_cast<Licq::EventFile*>(myCurrentEvent);
      FileDlg* fileDlg = new FileDlg(myUsers.front());

      if (fileDlg->ReceiveFiles())
        // FIXME: must have been done in CICQDaemon
        gProtocolManager.fileTransferAccept(
            myUsers.front(),
            fileDlg->LocalPort(), f->Sequence(), f->MessageID()[0], f->MessageID()[1],
            f->fileDescription(), f->filename(), f->FileSize(), !f->IsDirect());
      break;
    }

    case ICQ_CMDxSUB_AUTHxREQUEST:
    {
      Licq::EventAuthRequest* p = dynamic_cast<Licq::EventAuthRequest*>(myCurrentEvent);
      new AuthUserDlg(p->userId(), false);
      break;
    }
  } // switch
}

void UserViewEvent::read3()
{
  if (myCurrentEvent == NULL)
    return;

  QString accountId = myUsers.front().accountId().c_str();

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
      RefuseDlg* r = new RefuseDlg(myUsers.front(), tr("Chat"), this);

      if (r->exec())
      {
        myCurrentEvent->SetPending(false);
        Licq::EventChat* c = dynamic_cast<Licq::EventChat*>(myCurrentEvent);
        myRead2Button->setEnabled(false);
        myRead3Button->setEnabled(false);

        // FIXME: must have been done in CICQDaemon
        gLicqDaemon->icqChatRequestRefuse(
            myUsers.front(),
            myCodec->fromUnicode(r->RefuseMessage()).data(), myCurrentEvent->Sequence(),
            c->MessageID(), c->IsDirect());
      }
      delete r;
      break;
    }

    case ICQ_CMDxSUB_FILE:  // refuse a file transfer
    {
      RefuseDlg* r = new RefuseDlg(myUsers.front(), tr("File Transfer"), this);

      if (r->exec())
      {
        myCurrentEvent->SetPending(false);
        Licq::EventFile* f = dynamic_cast<Licq::EventFile*>(myCurrentEvent);
        myRead2Button->setEnabled(false);
        myRead3Button->setEnabled(false);

        // FIXME: must have been done in CICQDaemon
        gProtocolManager.fileTransferRefuse(
            myUsers.front(),
            myCodec->fromUnicode(r->RefuseMessage()).data(), myCurrentEvent->Sequence(),
            f->MessageID()[0], f->MessageID()[1], !f->IsDirect());
      }
      delete r;
      break;
    }

    case ICQ_CMDxSUB_AUTHxREQUEST:
    {
      Licq::EventAuthRequest* p = dynamic_cast<Licq::EventAuthRequest*>(myCurrentEvent);
      new AddUserDlg(p->userId(), this);
      break;
    }
  } // switch
}

void UserViewEvent::read4()
{
  if (myCurrentEvent == NULL)
    return;

  QString accountId = myUsers.front().accountId().c_str();

  switch (myCurrentEvent->SubCommand())
  {
    case ICQ_CMDxSUB_MSG:
      gLicqGui->showEventDialog(ChatEvent, myUsers.front());
      break;

    case ICQ_CMDxSUB_CHAT:  // join to current chat
    {
      Licq::EventChat* c = dynamic_cast<Licq::EventChat*>(myCurrentEvent);
      if (c->Port() != 0)  // Joining a multiparty chat (we connect to them)
      {
        ChatDlg* chatDlg = new ChatDlg(myUsers.front());
        // FIXME: must have been done in CICQDaemon
        if (chatDlg->StartAsClient(c->Port()))
          gLicqDaemon->icqChatRequestAccept(
              myUsers.front(),
              0, c->clients(), c->Sequence(), c->MessageID(), c->IsDirect());
      }
      else  // single party (other side connects to us)
      {
        ChatDlg* chatDlg = NULL;
        JoinChatDlg* j = new JoinChatDlg(this);
        // FIXME: must have been done in CICQDaemon
        if (j->exec() && (chatDlg = j->JoinedChat()) != NULL)
          gLicqDaemon->icqChatRequestAccept(
              myUsers.front(),
              chatDlg->LocalPort(), c->clients(), c->Sequence(), c->MessageID(), c->IsDirect());
        delete j;
      }
      break;
    }

    case ICQ_CMDxSUB_URL:   // view a url
      gLicqGui->viewUrl(dynamic_cast<Licq::EventUrl*>(myCurrentEvent)->url().c_str());
      break;

    case ICQ_CMDxSUB_AUTHxREQUEST: // Fall through
    case ICQ_CMDxSUB_AUTHxGRANTED:
    case ICQ_CMDxSUB_ADDEDxTOxLIST:
    {
      Licq::UserId userId;
#define GETINFO(sub, type) \
      if (myCurrentEvent->SubCommand() == sub) \
      { \
        type* p = dynamic_cast<type*>(myCurrentEvent); \
        userId = p->userId(); \
      }

      GETINFO(ICQ_CMDxSUB_AUTHxREQUEST, Licq::EventAuthRequest);
      GETINFO(ICQ_CMDxSUB_AUTHxGRANTED, Licq::EventAuthGranted);
      GETINFO(ICQ_CMDxSUB_ADDEDxTOxLIST, Licq::EventAdded);
#undef GETINFO

      {
        Licq::UserReadGuard u(userId, true);
      }

      gLicqGui->showInfoDialog(mnuUserGeneral, userId, false, true);
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
  Licq::UserWriteGuard u(myUsers.front());

  if (!u.isLocked())
    return;

  u->EventClearId(myCurrentEvent->Id());
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

  Licq::UserEvent* m = e->msg();
  myCurrentEvent = m;

  // Set the color for the message
  myMessageView->setBackground(QColor(m->color()->backRed(), m->color()->backGreen(), m->color()->backBlue()));
  myMessageView->setForeground(QColor(m->color()->foreRed(), m->color()->foreGreen(), m->color()->foreBlue()));

  // Set the text
  if (m->SubCommand() == ICQ_CMDxSUB_SMS)
     myMessageText = QString::fromUtf8(m->text().c_str());
  else
     myMessageText = myCodec->toUnicode(m->text().c_str());

  QString colorAttr;
  colorAttr.sprintf("#%02x%02x%02x", m->color()->foreRed(), m->color()->foreGreen(), m->color()->foreBlue());
  myMessageView->setText("<font color=\"" + colorAttr + "\">" + MLView::toRichText(myMessageText, true) + "</font>");

  myMessageView->GotoHome();

  if (m->isReceiver() &&
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
              dynamic_cast<Licq::EventChat*>(m)->Port() == 0)
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
        myRead4Button->setText(tr("&View"));
        break;

      case ICQ_CMDxSUB_AUTHxREQUEST:
      {
        myRead1Button->setText(tr("A&uthorize"));
        myRead2Button->setText(tr("&Refuse"));
        Licq::EventAuthRequest* pAuthReq = dynamic_cast<Licq::EventAuthRequest*>(m);
        if (!Licq::gUserManager.userExists(pAuthReq->userId()))
          myRead3Button->setText(tr("A&dd User"));
        myRead4Button->setText(tr("&View Info"));
        break;
      }

      case ICQ_CMDxSUB_AUTHxGRANTED:
      {
        Licq::EventAuthGranted* pAuth = dynamic_cast<Licq::EventAuthGranted*>(m);
        if (!Licq::gUserManager.userExists(pAuth->userId()))
          myRead1Button->setText(tr("A&dd User"));
        myRead4Button->setText(tr("&View Info"));
        break;
      }

      case ICQ_CMDxSUB_ADDEDxTOxLIST:
      {
        Licq::EventAdded* pAdd = dynamic_cast<Licq::EventAdded*>(m);
        if (!Licq::gUserManager.userExists(pAdd->userId()))
          myRead1Button->setText(tr("A&dd User"));
        myRead4Button->setText(tr("&View Info"));
        break;
      }

      case ICQ_CMDxSUB_CONTACTxLIST:
      {
        int s = dynamic_cast<Licq::EventContactList*>(m)->Contacts().size();
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

void UserViewEvent::sentEvent(const Licq::Event* e)
{
  if (e->userId() != myUsers.front())
    return;

  if (!Config::Chat::instance()->msgChatView())
    new MessageListItem(e->userEvent(), myCodec, myMessageList);
}

void UserViewEvent::setEncoding()
{
  // if we have an open view, just refresh it
  if (myMessageList != NULL)
    printMessage(myMessageList->currentItem());
}

void UserViewEvent::resizeEvent(QResizeEvent* event)
{
  Config::Chat::instance()->setViewDialogSize(size());
  UserEventCommon::resizeEvent(event);
}
