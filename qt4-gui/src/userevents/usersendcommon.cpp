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

#include "usersendcommon.h"

#include "config.h"

#include <assert.h>
#include <boost/foreach.hpp>

#include <QAction>
#include <QApplication>
#include <QColorDialog>
#include <QDateTime>
#include <QDesktopWidget>
#include <QDialogButtonBox>
#include <QDropEvent>
#include <QGroupBox>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QMovie>
#include <QPushButton>
#include <QShortcut>
#include <QSplitter>
#include <QTextCodec>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

#ifdef USE_KDE
#include <KDE/KColorDialog>
#endif

#include <licq/logging/log.h>
#include <licq/contactlist/owner.h>
#include <licq/contactlist/usermanager.h>
#include <licq/conversation.h>
#include <licq/daemon.h>
#include <licq/event.h>
#include <licq/icq.h>
#include <licq/icqdefines.h>
#include <licq/plugin.h>
#include <licq/pluginsignal.h>
#include <licq/protocolmanager.h>
#include <licq/translator.h>
#include <licq/userevents.h>

#include "config/chat.h"
#include "config/emoticons.h"
#include "config/iconmanager.h"
#include "config/shortcuts.h"

#include "core/gui-defines.h"
#include "core/licqgui.h"
#include "core/mainwin.h"
#include "core/messagebox.h"
#include "core/signalmanager.h"

#include "dialogs/keyrequestdlg.h"

#include "helpers/eventdesc.h"
#include "helpers/usercodec.h"

#include "views/mmuserview.h"
#include "views/userview.h"

#include "widgets/historyview.h"
#include "widgets/infofield.h"
#include "widgets/mledit.h"

#include "selectemoticon.h"
#include "usereventtabdlg.h"
#include "usersendchatevent.h"
#include "usersendcontactevent.h"
#include "usersendfileevent.h"
#include "usersendmsgevent.h"
#include "usersendsmsevent.h"
#include "usersendurlevent.h"

using namespace std;
using Licq::StringList;
using Licq::gProtocolManager;
using Licq::gConvoManager;
using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::UserSendCommon */

const size_t SHOW_RECENT_NUM = 5;

typedef pair<const Licq::UserEvent*, Licq::UserId> messagePair;

bool orderMessagePairs(const messagePair& mp1, const messagePair& mp2)
{
  return (mp1.first->Time() < mp2.first->Time());
}

UserSendCommon::UserSendCommon(int type, const Licq::UserId& userId, QWidget* parent, const char* name)
  : UserEventCommon(userId, parent, name),
    myType(type)
{
  myMassMessageBox = NULL;
  myPictureLabel = NULL;
  clearDelay = 250;

  QShortcut* a = new QShortcut(Qt::Key_Escape, this);
  connect(a, SIGNAL(activated()), SLOT(cancelSend()));

  UserEventTabDlg* tabDlg = gLicqGui->userEventTabDlg();
  if (tabDlg != NULL && parent == tabDlg)
  {
    a = new QShortcut(Qt::ALT + Qt::Key_Left, this);
    connect(a, SIGNAL(activated()), tabDlg, SLOT(moveLeft()));

    a = new QShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Tab, this);
    connect(a, SIGNAL(activated()), tabDlg, SLOT(moveLeft()));

    a = new QShortcut(Qt::ALT + Qt::Key_Right, this);
    connect(a, SIGNAL(activated()), tabDlg, SLOT(moveRight()));

    a = new QShortcut(Qt::CTRL + Qt::Key_Tab, this);
    connect(a, SIGNAL(activated()), tabDlg, SLOT(moveRight()));
  }

  myEventTypeGroup = new QActionGroup(this);
  connect(myEventTypeGroup, SIGNAL(triggered(QAction*)), SLOT(changeEventType(QAction*)));

  QAction* action;
  int eventTypesCount = 0;

#define ADD_SENDTYPE(eventFlag, eventType, caption) \
    if (mySendFuncs & eventFlag) { \
      action = new QAction(caption, myEventTypeGroup); \
      action->setData(eventType); \
      action->setCheckable(true); \
      eventTypesCount++; \
    }

  // Populated menu for switching event type
  ADD_SENDTYPE(Licq::ProtocolPlugin::CanSendMsg, MessageEvent, tr("Message"));
  ADD_SENDTYPE(Licq::ProtocolPlugin::CanSendUrl, UrlEvent, tr("URL"));
  ADD_SENDTYPE(Licq::ProtocolPlugin::CanSendChat, ChatEvent, tr("Chat Request"));
  ADD_SENDTYPE(Licq::ProtocolPlugin::CanSendFile, FileEvent, tr("File Transfer"));
  ADD_SENDTYPE(Licq::ProtocolPlugin::CanSendContact, ContactEvent, tr("Contact List"));
  ADD_SENDTYPE(Licq::ProtocolPlugin::CanSendSms, SmsEvent, tr("SMS"));

#undef ADD_SENDTYPE

  QMenu* mnuSendType = new QMenu(this);
  mnuSendType->addActions(myEventTypeGroup->actions());

  myEventTypeMenu = myToolBar->addAction(tr("Message type"), this, SLOT(showSendTypeMenu()));
  myEventTypeMenu->setMenu(mnuSendType);
  if (eventTypesCount <= 1)
    myEventTypeMenu->setEnabled(false);

  mySendServerCheck = myToolBar->addAction(tr("Send through server"));
  mySendServerCheck->setCheckable(true);

  bool canSendDirect = (mySendFuncs & Licq::ProtocolPlugin::CanSendDirect);

  {
    Licq::UserReadGuard u(myUsers.front());
    if (u.isLocked())
    {
      mySendServerCheck->setChecked(u->SendServer() ||
          (!u->isOnline() && u->normalSocketDesc() == -1));

      if (u->InvisibleList() || (u->Port() == 0 && u->normalSocketDesc() == -1))
        canSendDirect = false;
    }
  }
  if (!canSendDirect)
  {
    mySendServerCheck->setChecked(true);
    mySendServerCheck->setEnabled(false);
  }

  myUrgentCheck = myToolBar->addAction(tr("Urgent"));
  myUrgentCheck->setCheckable(true);

  myMassMessageCheck = myToolBar->addAction(tr("Multiple Recipients"));
  myMassMessageCheck->setCheckable(true);
  connect(myMassMessageCheck, SIGNAL(toggled(bool)), SLOT(massMessageToggled(bool)));

  myToolBar->addSeparator();

  myEmoticon = myToolBar->addAction(tr("Smileys"), this, SLOT(showEmoticonsMenu()));
  myForeColor = myToolBar->addAction(tr("Text Color..."), this, SLOT(setForegroundICQColor()));
  myBackColor = myToolBar->addAction(tr("Background Color..."), this, SLOT(setBackgroundICQColor()));

  QDialogButtonBox* buttons = new QDialogButtonBox();
  myTopLayout->addWidget(buttons);

  mySendButton = buttons->addButton(tr("&Send"), QDialogButtonBox::ActionRole);
  mySendButton->setDefault(true);
  // add a wrapper around the send button that
  // tries to establish a secure connection first.
  connect(mySendButton, SIGNAL(clicked()), SLOT(sendTrySecure()));

  myCloseButton = buttons->addButton(QDialogButtonBox::Close);
  myCloseButton->setAutoDefault(true);
  connect(myCloseButton, SIGNAL(clicked()), SLOT(closeDialog()));

  buttons->setVisible(Config::Chat::instance()->showDlgButtons());

  myViewSplitter = new QSplitter(Qt::Vertical);
  myTopLayout->addWidget(myViewSplitter);

  myHistoryView = 0;
  if (Config::Chat::instance()->msgChatView())
  {
    myHistoryView = new HistoryView(false, myUsers.front(), myViewSplitter);
    connect(myHistoryView, SIGNAL(messageAdded()), SLOT(messageAdded()));

    Licq::UserReadGuard u(myUsers.front());
    int userSocketDesc = (u.isLocked() ? u->normalSocketDesc() : -1);
    if (u.isLocked() && Config::Chat::instance()->showHistory())
    {
      // Show the last SHOW_RECENT_NUM messages in the history
      Licq::HistoryList lHistoryList;
      if (u->GetHistory(lHistoryList))
      {
        // Rewind to the starting point. This will be the first message shown in the dialog.
        // Make sure we don't show the new messages waiting.
        unsigned short nNewMessages = u->NewMessages();
        Licq::HistoryList::iterator lHistoryIter = lHistoryList.end();
        for (size_t i = 0; i < (SHOW_RECENT_NUM + nNewMessages) && lHistoryIter != lHistoryList.begin(); i++)
          lHistoryIter--;

        bool bUseHTML = !isdigit(u->accountId()[1]);
        const QTextCodec* myCodec = UserCodec::codecForUser(*u);
        QString contactName = QString::fromUtf8(u->GetAlias());
        QString ownerName;
        {
          Licq::OwnerReadGuard o(u->protocolId());
          if (o.isLocked())
            ownerName = QString::fromUtf8(o->GetAlias());
          else
            ownerName = QString(tr("Error! no owner set"));
        }

        // Iterate through each message to add
        // Only show old messages as recent ones. Don't show duplicates.
        int nMaxNumToShow;
        if (lHistoryList.size() <= SHOW_RECENT_NUM)
          nMaxNumToShow = lHistoryList.size() - nNewMessages;
        else
          nMaxNumToShow = SHOW_RECENT_NUM;

        // Safety net
        if (nMaxNumToShow < 0)
          nMaxNumToShow = 0;

        QDateTime date;

        for (int i = 0; i < nMaxNumToShow && lHistoryIter != lHistoryList.end(); i++)
        {
          QString str;
          date.setTime_t((*lHistoryIter)->Time());
          QString messageText;
          if ((*lHistoryIter)->SubCommand() == ICQ_CMDxSUB_SMS) // SMSs are always in UTF-8
            messageText = QString::fromUtf8((*lHistoryIter)->text().c_str());
          else
            messageText = myCodec->toUnicode((*lHistoryIter)->text().c_str());

          myHistoryView->addMsg(
              (*lHistoryIter)->isReceiver(),
              true,
              (*lHistoryIter)->SubCommand() == ICQ_CMDxSUB_MSG ? "" : EventDescription(*lHistoryIter) + " ",
              date,
              (*lHistoryIter)->IsDirect(),
              (*lHistoryIter)->IsMultiRec(),
              (*lHistoryIter)->IsUrgent(),
              (*lHistoryIter)->IsEncrypted(),
              (*lHistoryIter)->isReceiver() ? contactName : ownerName,
              MLView::toRichText(messageText, true, bUseHTML));
          lHistoryIter++;
        }

        myHistoryView->GotoEnd();

        Licq::User::ClearHistory(lHistoryList);
      }
    }

    // Collect all messages to put them in the correct time order
    vector<messagePair> messages;

    // add all unread messages.
    if (u.isLocked() && u->NewMessages() > 0)
    {
      for (unsigned short i = 0; i < u->NewMessages(); i++)
      {
        const Licq::UserEvent* e = u->EventPeek(i);
        // Get the convo id now
        unsigned long convoId = e->ConvoId();
        if (myConvoId == 0)
          myConvoId = convoId;

        if (convoId == myConvoId)
        {
          if (e->Id() > myHighestEventId)
            myHighestEventId = e->Id();

          messages.push_back(make_pair(e, u->id()));
        }
      }
      u.unlock();

      // Now add messages that are a part of this convo
      if (myPpid != LICQ_PPID)
      {
        Licq::UserListGuard userList(myPpid);
        BOOST_FOREACH(const Licq::User* user, **userList)
        {
          Licq::UserReadGuard pUser(user);
          if (pUser->NewMessages() && myUsers.front() != pUser->id())
          {
            for (unsigned short i = 0; i < pUser->NewMessages(); i++)
            {
              const Licq::UserEvent* e = pUser->EventPeek(i);

              if (e->ConvoId() == myConvoId)
              {
                if (e->Id() > myHighestEventId)
                  myHighestEventId = e->Id();

                // add to the convo list (but what if they left by the time we open this?)
                myUsers.push_back(pUser->id());
                messages.push_back(make_pair(e, pUser->id()));
              }
            }
          }
        }
      }

      // Sort the messages by time
      stable_sort(messages.begin(), messages.end(), orderMessagePairs);

      // Now, finally add them
      vector<messagePair>::iterator messageIter;
      for (messageIter = messages.begin(); messageIter != messages.end(); messageIter++)
        myHistoryView->addMsg((*messageIter).first, (*messageIter).second);
      messages.clear();

      // If the user closed the chat window, we have to make sure we aren't
      // using the old nConvoId
      if (gConvoManager.get(myConvoId) == NULL)
        myConvoId = 0;
    }

    // Do we already have an open socket?
    if (myConvoId == 0 && userSocketDesc != 1)
    {
      Licq::Conversation* convo = gConvoManager.getFromSocket(userSocketDesc);
      if (convo != NULL)
        myConvoId = convo->id();
    }

    connect(gLicqGui, SIGNAL(eventSent(const Licq::Event*)),
        myHistoryView, SLOT(addMsg(const Licq::Event*)));
    //myViewSplitter->setResizeMode(myHistoryView, QSplitter::FollowSizeHint);
  }

  {
    mySendTypingTimer = new QTimer(this);
    connect(mySendTypingTimer, SIGNAL(timeout()), SLOT(textChangedTimeout()));
  }

  myPictureSplitter = new QSplitter(myViewSplitter);

  myMessageEdit = new MLEdit(true, myPictureSplitter);
  myMessageEdit->setSizeHintLines(3);
  if (Config::Chat::instance()->checkSpelling())
  {
#ifdef HAVE_HUNSPELL
    myMessageEdit->setSpellingDictionary(Config::Chat::instance()->spellingDictionary());
#endif
    myMessageEdit->setCheckSpellingEnabled(true);
  }
  myMessageEdit->installEventFilter(this); // Enables send with enter

  // Disable drops for edit box so our own handler gets them
  myMessageEdit->setAcceptDrops(false);

  if (Config::Chat::instance()->msgChatView())
  {
    myViewSplitter->setStretchFactor(myViewSplitter->indexOf(myHistoryView), 1);
    myViewSplitter->setStretchFactor(myViewSplitter->indexOf(myPictureSplitter), 0);

    connect(myHistoryView, SIGNAL(quote(const QString&)),
        myMessageEdit, SLOT(insertPlainText(const QString&)));

    // Connect scroll up/down shortcuts from edit to scroll the history
    connect(myMessageEdit, SIGNAL(scrollDownPressed()),
        myHistoryView, SLOT(scrollPageDown()));
    connect(myMessageEdit, SIGNAL(scrollUpPressed()),
        myHistoryView, SLOT(scrollPageUp()));
  }

  setFocusProxy(myMessageEdit);
  if (Config::Chat::instance()->showDlgButtons())
  {
    setTabOrder(myMessageEdit, mySendButton);
    setTabOrder(mySendButton, myCloseButton);
  }

  myIcqColor.setToDefault();
  myMessageEdit->setBackground(QColor(myIcqColor.backRed(), myIcqColor.backGreen(), myIcqColor.backBlue()));
  myMessageEdit->setForeground(QColor(myIcqColor.foreRed(), myIcqColor.foreGreen(), myIcqColor.foreBlue()));

  updateIcons();
  {
    Licq::UserReadGuard u(myUsers.front());
    updatePicture(*u);
  }
  updateShortcuts();

  connect(myMessageEdit, SIGNAL(ctrlEnterPressed()), mySendButton, SIGNAL(clicked()));
  connect(myMessageEdit, SIGNAL(textChanged()), SLOT(messageTextChanged()));
  connect(mySendServerCheck, SIGNAL(triggered(bool)), SLOT(sendServerToggled(bool)));

  QSize dialogSize = Config::Chat::instance()->sendDialogSize();
  if (dialogSize.isValid())
    resize(dialogSize);

  setAcceptDrops(true);
}

UserSendCommon::~UserSendCommon()
{
  // Empty
}

void UserSendCommon::closeEvent(QCloseEvent* event)
{
  UserEventCommon::closeEvent(event);

  if (event->isAccepted())
  {
    // This widget is about to be destroyed so remove us from the tab dialog
    UserEventTabDlg* tabDlg = gLicqGui->userEventTabDlg();
    if (tabDlg != NULL && tabDlg->tabExists(this))
      tabDlg->removeTab(this);
  }
}

bool UserSendCommon::eventFilter(QObject* watched, QEvent* e)
{
  if (watched == myMessageEdit)
  {
    // If we're in single line chat mode we send messages with Enter and
    // insert new lines with Ctrl+Enter.
    if (Config::Chat::instance()->singleLineChatMode() && e->type() == QEvent::KeyPress)
    {
      QKeyEvent* key = dynamic_cast<QKeyEvent*>(e);
      const bool isEnter = (key->key() == Qt::Key_Enter || key->key() == Qt::Key_Return);
      if (isEnter)
      {
        if (key->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier))
        {
          myMessageEdit->insertPlainText("\n");
          myMessageEdit->ensureCursorVisible();
        }
        else
          mySendButton->animateClick();
        return true; // filter the event out
      }
    }
    return false;
  }
  else
    return UserEventCommon::eventFilter(watched, e);
}

void UserSendCommon::updateIcons()
{
  UserEventCommon::updateIcons();

  IconManager* iconman = IconManager::instance();

  // Toolbar buttons
  myEventTypeMenu->setIcon(iconForType(myType));
  mySendServerCheck->setIcon(iconman->getIcon(IconManager::ThroughServerIcon));
  myUrgentCheck->setIcon(iconman->getIcon(IconManager::UrgentIcon));
  myMassMessageCheck->setIcon(iconman->getIcon(IconManager::MultipleRecIcon));
  myEmoticon->setIcon(iconman->getIcon(IconManager::SmileIcon));
  myForeColor->setIcon(iconman->getIcon(IconManager::TextColorIcon));
  myBackColor->setIcon(iconman->getIcon(IconManager::BackColorIcon));

  // Update message type icons in menu
  foreach (QAction* a, myEventTypeGroup->actions())
    a->setIcon(iconForType(a->data().toInt()));
}

void UserSendCommon::updateShortcuts()
{
  UserEventCommon::updateShortcuts();

  Config::Shortcuts* shortcuts = Config::Shortcuts::instance();

  myEventTypeMenu->setShortcut(shortcuts->getShortcut(Config::Shortcuts::ChatEventMenu));
  mySendServerCheck->setShortcut(shortcuts->getShortcut(Config::Shortcuts::ChatToggleSendServer));
  myUrgentCheck->setShortcut(shortcuts->getShortcut(Config::Shortcuts::ChatToggleUrgent));
  myMassMessageCheck->setShortcut(shortcuts->getShortcut(Config::Shortcuts::ChatToggleMassMessage));
  myEmoticon->setShortcut(shortcuts->getShortcut(Config::Shortcuts::ChatEmoticonMenu));
  myForeColor->setShortcut(shortcuts->getShortcut(Config::Shortcuts::ChatColorFore));
  myBackColor->setShortcut(shortcuts->getShortcut(Config::Shortcuts::ChatColorBack));

  // Tooltips include shortcut so update them here as well
  pushToolTip(myEventTypeMenu, tr("Select type of message to send"));
  pushToolTip(mySendServerCheck, tr("Send through server"));
  pushToolTip(myUrgentCheck, tr("Urgent"));
  pushToolTip(myMassMessageCheck, tr("Multiple recipients"));
  pushToolTip(myEmoticon, tr("Insert smileys"));
  pushToolTip(myForeColor, tr("Change text color"));
  pushToolTip(myBackColor, tr("Change background color"));
}

void UserSendCommon::updatePicture(const Licq::User* u)
{
  if (u == NULL)
    return;

  if (myPictureLabel != NULL)
  {
    delete myPictureLabel;
    myPictureLabel = NULL;
  }

  if (Config::Chat::instance()->showUserPic() &&
      u->GetPicturePresent())
  {
    QString picPath = QString::fromLocal8Bit(u->pictureFileName().c_str());
    QMovie* picMovie = new QMovie(picPath, QByteArray(), this);
    if (picMovie->isValid())
    {
      myPictureLabel = new QLabel();
      myPictureSplitter->insertWidget(1, myPictureLabel);
      myPictureLabel->setMovie(picMovie);
      if (picMovie->frameCount() > 1)
        picMovie->start();
      else
        picMovie->jumpToNextFrame();
      myPictureLabel->setFixedWidth(myPictureLabel->sizeHint().width());
      if (Config::Chat::instance()->showUserPicHidden())
        myPictureSplitter->setSizes(QList<int>() << 1 << 0);
    }
    else
      delete picMovie;
  }
}

const QPixmap& UserSendCommon::iconForType(int type) const
{
  switch (type)
  {
    case UrlEvent:
      return IconManager::instance()->getIcon(IconManager::UrlMessageIcon);

    case ChatEvent:
      return IconManager::instance()->getIcon(IconManager::ChatMessageIcon);

    case FileEvent:
      return IconManager::instance()->getIcon(IconManager::FileMessageIcon);

    case ContactEvent:
      return IconManager::instance()->getIcon(IconManager::ContactMessageIcon);

    case SmsEvent:
      return IconManager::instance()->getIcon(IconManager::SmsMessageIcon);

    case MessageEvent:
    default:
      return IconManager::instance()->getIcon(IconManager::StandardMessageIcon);
  }
}

void UserSendCommon::setText(const QString& text)
{
  if (myMessageEdit == 0)
    return;

  myMessageEdit->setText(text);
  myMessageEdit->GotoEnd();
  myMessageEdit->document()->setModified(false);
}

void UserSendCommon::convoJoin(const Licq::UserId& userId)
{
  if (!userId.isValid())
    return;

  if (Config::Chat::instance()->msgChatView())
  {
    Licq::UserReadGuard u(userId);
    QString userName;
    if (u.isLocked())
      userName = QString::fromUtf8(u->GetAlias());
    else
      userName = "";

    myHistoryView->addNotice(QDateTime::currentDateTime(),
        tr("%1 has joined the conversation.").arg(userName));
  }

  if (!isUserInConvo(userId))
    myUsers.push_back(userId);

  // Now update the tab label
  UserEventTabDlg* tabDlg = gLicqGui->userEventTabDlg();
  if (tabDlg != NULL)
    tabDlg->updateConvoLabel(this);
}

void UserSendCommon::convoLeave(const Licq::UserId& userId)
{
  if (!userId.isValid())
    return;

  if (Config::Chat::instance()->msgChatView())
  {
    Licq::UserWriteGuard u(userId);
    QString userName;
    if (u.isLocked())
      userName = QString::fromUtf8(u->GetAlias());
    else
      userName = "";

    myHistoryView->addNotice(QDateTime::currentDateTime(),
        tr("%1 has left the conversation.").arg(userName));

    // Remove the typing notification if active
    if (u.isLocked())
    {
      if (u->isTyping())
      {
        u->setIsTyping(false);
        myTimezone->setPalette(QPalette());
        UserEventTabDlg* tabDlg = gLicqGui->userEventTabDlg();
        if (Config::Chat::instance()->tabbedChatting() && tabDlg != NULL)
          tabDlg->updateTabLabel(*u);
      }
    }
  }

  if (myUsers.size() > 1)
  {
    list<Licq::UserId>::iterator it;
    for (it = myUsers.begin(); it != myUsers.end(); it++)
    {
      if (*it == userId)
      {
        myUsers.remove(*it);
        break;
      }
    }
    myHistoryView->setOwner(myUsers.front());
  }
  else
    myConvoId = 0;

  if (Config::Chat::instance()->msgChatView())
  {
    // Now update the tab label
    UserEventTabDlg* tabDlg = gLicqGui->userEventTabDlg();
    if (tabDlg != NULL)
      tabDlg->updateConvoLabel(this);
  }
}

void UserSendCommon::windowActivationChange(bool oldActive)
{
  if (isActiveWindow())
    QTimer::singleShot(clearDelay, this, SLOT(clearNewEvents()));
  QWidget::windowActivationChange(oldActive);
}

UserSendCommon* UserSendCommon::changeEventType(int type)
{
  if (myType == type)
    return this;

  UserSendCommon* e = 0;
  QWidget* parent = 0;

  UserEventTabDlg* tabDlg = gLicqGui->userEventTabDlg();
  if (tabDlg != NULL && tabDlg->tabExists(this))
    parent = tabDlg;

  Licq::UserId userId = myUsers.front();

  switch (type)
  {
    case MessageEvent:
      if (mySendFuncs & Licq::ProtocolPlugin::CanSendMsg)
        e = new UserSendMsgEvent(userId, parent);
      break;
    case UrlEvent:
      if (mySendFuncs & Licq::ProtocolPlugin::CanSendUrl)
        e = new UserSendUrlEvent(userId, parent);
      break;
    case ChatEvent:
      if (mySendFuncs & Licq::ProtocolPlugin::CanSendChat)
        e = new UserSendChatEvent(userId, parent);
      break;
    case FileEvent:
      if (mySendFuncs & Licq::ProtocolPlugin::CanSendFile)
        e = new UserSendFileEvent(userId, parent);
      break;
    case ContactEvent:
      if (mySendFuncs & Licq::ProtocolPlugin::CanSendContact)
        e = new UserSendContactEvent(userId, parent);
      break;
    case SmsEvent:
      if (mySendFuncs & Licq::ProtocolPlugin::CanSendSms)
        e = new UserSendSmsEvent(userId, parent);
      break;
    default:
      assert(false);
  }

  if (e != NULL)
  {
    if (e->myMessageEdit != 0 && myMessageEdit != 0)
    {
      e->myMessageEdit->setText(myMessageEdit->toPlainText());
      e->myMessageEdit->document()->setModified(myMessageEdit->document()->isModified());
    }
    if (e->myHistoryView != 0 && myHistoryView != 0)
    {
      e->myHistoryView->setHtml(myHistoryView->toHtml());
      e->myHistoryView->GotoEnd();
    }

    if (parent == 0)
    {
      QPoint p = topLevelWidget()->pos();
      e->move(p);
    }

    gLicqGui->replaceEventDialog(this, e, userId);
    emit msgTypeChanged(this, e);

    if (parent == 0)
    {
      QTimer::singleShot(10, e, SLOT(show()));
      QTimer::singleShot(100, this, SLOT(close()));
    }
    else
      tabDlg->replaceTab(this, e);
  }
  return e;
}

void UserSendCommon::retrySend(const Licq::Event* e, bool online, unsigned short level)
{
  QString accountId = myUsers.front().accountId().c_str();

  unsigned long icqEventTag = 0;
  mySendServerCheck->setChecked(!online);
  myUrgentCheck->setChecked(level == ICQ_TCPxMSG_URGENT);

  switch (e->userEvent()->SubCommand() & ~ICQ_CMDxSUB_FxMULTIREC)
  {
    case ICQ_CMDxSUB_MSG:
    {
      bool userOffline = true;
      {
        Licq::UserReadGuard u(myUsers.front());
        if (u.isLocked())
          userOffline = !u->isOnline();
      }
      const Licq::EventMsg* ue = dynamic_cast<const Licq::EventMsg*>(e->userEvent());
      // create initial strings (implicit copying, no allocation impact :)
      QByteArray wholeMessageRaw(Licq::gTranslator.returnToDos(ue->message()).c_str());
      int wholeMessagePos = 0;

      bool needsSplitting = false;
      // If we send through myServer (= have message limit), and we've crossed the limit
      unsigned short maxSize = userOffline ? CICQDaemon::MaxOfflineMessageSize : CICQDaemon::MaxMessageSize;
      if ((wholeMessageRaw.length() - wholeMessagePos) > maxSize)
        needsSplitting = true;

      QString message;
      QByteArray messageRaw;

      while (wholeMessageRaw.length() > wholeMessagePos)
      {
        if (needsSplitting)
        {
          // This is a bit ugly but adds safety. We don't simply search
          // for a whitespace to cut at in the encoded text (since we don't
          // really know how spaces are represented in its encoding), so
          // we take the maximum length, then convert back to a Unicode string
          // and then search for Unicode whitespaces.
          messageRaw = Licq::gTranslator.returnToUnix(wholeMessageRaw.mid(wholeMessagePos, maxSize).data()).c_str();
          message = myCodec->toUnicode(messageRaw);

          if ((wholeMessageRaw.length() - wholeMessagePos) > maxSize)
          {
            // We try to find the optimal place to cut
            // (according to our narrow-minded Latin1 idea of optimal :)
            // prefer keeping sentences intact 1st
            int foundIndex = message.lastIndexOf(QRegExp("[\\.\\n]"));
            // slicing at 0 position would be useless
            if (foundIndex <= 0)
              foundIndex = message.lastIndexOf(QRegExp("\\s"));

            if (foundIndex > 0)
            {
              message.truncate(foundIndex + 1);
              messageRaw = myCodec->fromUnicode(message);
            }
          }
        }
        else
        {
          messageRaw = ue->message().c_str();
        }

        icqEventTag = gProtocolManager.sendMessage(myUsers.front(), messageRaw.data(),
            !online, level, false, &myIcqColor);

        myEventTag.push_back(icqEventTag);

        wholeMessagePos += Licq::gTranslator.returnToDos(messageRaw.data()).size();
      }

      icqEventTag = 0;

      break;
    }

    case ICQ_CMDxSUB_URL:
    {
      const Licq::EventUrl* ue = dynamic_cast<const Licq::EventUrl*>(e->userEvent());

      icqEventTag = gProtocolManager.sendUrl(myUsers.front(), ue->url(),
          ue->description(), !online, level, false, &myIcqColor);

      break;
    }

    case ICQ_CMDxSUB_CONTACTxLIST:
    {
      const Licq::EventContactList* ue = dynamic_cast<const Licq::EventContactList*>(e->userEvent());
      const Licq::EventContactList::ContactList& clist = ue->Contacts();
      StringList users;

      // ContactList is const but string list holds "char*" so we have to copy each string
      for (Licq::EventContactList::ContactList::const_iterator i = clist.begin(); i != clist.end(); i++)
        users.push_back((*i)->userId().accountId());

      if (users.empty())
        break;

      icqEventTag = gLicqDaemon->icqSendContactList(myUsers.front(),
          users, online, level, false, &myIcqColor);

      break;
    }

    case ICQ_CMDxSUB_CHAT:
    {
      const Licq::EventChat* ue = dynamic_cast<const Licq::EventChat*>(e->userEvent());

      if (ue->clients().empty())
        //TODO in the daemon
        icqEventTag = gLicqDaemon->icqChatRequest(myUsers.front(),
            ue->reason(), level, !online);
      else
        //TODO in the daemon
        icqEventTag = gLicqDaemon->icqMultiPartyChatRequest(myUsers.front(),
            ue->reason(), ue->clients(), ue->Port(), level, !online);

      break;
    }

    case ICQ_CMDxSUB_FILE:
    {
      const Licq::EventFile* ue = dynamic_cast<const Licq::EventFile*>(e->userEvent());
      list<string> filelist(ue->FileList());

      //TODO in the daemon
      icqEventTag = gProtocolManager.fileTransferPropose(myUsers.front(),
          ue->filename(), ue->fileDescription(), filelist, level, !online);

      break;
    }

    case ICQ_CMDxSUB_SMS:
    {
      const Licq::EventSms* ue = dynamic_cast<const Licq::EventSms*>(e->userEvent());

      //TODO in the daemon
      icqEventTag = gLicqDaemon->icqSendSms(myUsers.front(),
          ue->number().c_str(), ue->message().c_str());
      break;
    }

    default:
    {
      Licq::gLog.warning("Internal error: UserSendCommon::RetrySend()\n"
                      "Unknown sub-command %d", e->SubCommand());
      break;
    }
  }

  if (icqEventTag)
    myEventTag.push_back(icqEventTag);

  UserSendCommon::send();
}

void UserSendCommon::userUpdated(const Licq::UserId& userId, unsigned long subSignal, int argument, unsigned long cid)
{
  Licq::UserWriteGuard u(userId);

  if (!u.isLocked())
    return;

  switch (subSignal)
  {
    case Licq::PluginSignal::UserStatus:
    {
      if (u->Port() == 0)
      {
        mySendServerCheck->setChecked(true);
        mySendServerCheck->setEnabled(false);
      }
      else
        mySendServerCheck->setEnabled(true);

      if (!u->isOnline())
        mySendServerCheck->setChecked(true);

      break;
    }

    case Licq::PluginSignal::UserEvents:
    {
      const Licq::UserEvent* e = u->EventPeekId(argument);

      if (e != NULL && myHighestEventId < argument &&
          myHistoryView && argument > 0)
      {
        myHighestEventId = argument;
        e = u->EventPeekId(argument);

        if (e != NULL)
          if (u->ppid() != MSN_PPID || cid == myConvoId)
          {
            u.unlock();
            myHistoryView->addMsg(e, userId);
            return;
          }
      }
      break;
    }

    case Licq::PluginSignal::UserSecurity:
      // Automatically unset 'send through server' upon
      // establishing secure channel
      if (u->Secure())
      {
        u->SetSendServer(false);
        mySendServerCheck->setChecked(false);
      }
      break;

    case Licq::PluginSignal::UserPicture:
      updatePicture(*u);
  }
}

bool UserSendCommon::checkSecure()
{
  bool secure;
  {
    Licq::UserReadGuard u(myUsers.front());
    if (!u.isLocked())
      return false;
    secure = u->Secure() || u->AutoSecure();
  }

  bool send_ok = true;

  if (mySendServerCheck->isChecked() && secure)
  {
    if (!QueryYesNo(this, tr("Message can't be sent securely through the server!\n"
            "Send anyway?")))
      send_ok = false;
    else
    {
      Licq::UserWriteGuard u(myUsers.front());
      if (u.isLocked())
        u->SetAutoSecure(false);
    }
  }
  return send_ok;
}

void UserSendCommon::send()
{
  if (!Config::Chat::instance()->manualNewUser())
  {
    bool newUser = false;
    {
      Licq::UserWriteGuard u(myUsers.front());
      if (u.isLocked() && u->NewUser())
      {
        u->SetNewUser(false);
        newUser = true;
      }
    }
    if (newUser)
      Licq::gUserManager.notifyUserUpdated(myUsers.front(), Licq::PluginSignal::UserSettings);
  }

  unsigned long icqEventTag = 0;

  if (myEventTag.size() != 0)
    icqEventTag = myEventTag.front();

  unsigned long myPpid = myUsers.front().protocolId();

  if (icqEventTag != 0 || myPpid != LICQ_PPID)
  {
    bool via_server = mySendServerCheck->isChecked();
    myProgressMsg = tr("Sending ");
    myProgressMsg += via_server ? tr("via server") : tr("direct");
    myProgressMsg += "...";
    QString title = myBaseTitle + " [" + myProgressMsg + "]";

    UserEventTabDlg* tabDlg = gLicqGui->userEventTabDlg();
    if (tabDlg != NULL && tabDlg->tabIsSelected(this))
      tabDlg->setWindowTitle(title);

    setWindowTitle(title);
    setCursor(Qt::WaitCursor);
    mySendButton->setText(tr("&Cancel"));
    myCloseButton->setEnabled(false);

    if (myMessageEdit != NULL)
      myMessageEdit->setEnabled(false);

    disconnect(mySendButton, SIGNAL(clicked()), this, SLOT(send()));
    connect(mySendButton, SIGNAL(clicked()), SLOT(cancelSend()));

    connect(gGuiSignalManager, SIGNAL(doneUserFcn(const Licq::Event*)),
        SLOT(eventDoneReceived(const Licq::Event*)));
  }
}

void UserSendCommon::eventDoneReceived(const Licq::Event* e)
{
  if (e == NULL)
  {
    QString title = myBaseTitle + " [" + myProgressMsg + tr("error") + "]";

    UserEventTabDlg* tabDlg = gLicqGui->userEventTabDlg();
    if (tabDlg != NULL && tabDlg->tabIsSelected(this))
      tabDlg->setWindowTitle(title);

    setWindowTitle(title);

    return;
  }

  unsigned long icqEventTag = 0;
  list<unsigned long>::iterator iter;

  for (iter = myEventTag.begin(); iter != myEventTag.end(); iter++)
  {
    if (e->Equals(*iter))
    {
      icqEventTag = *iter;
      myEventTag.erase(iter);
      break;
    }
  }

  if (icqEventTag == 0)
    return;

  QString title, result;
  switch (e->Result())
  {
    case Licq::Event::ResultAcked:
    case Licq::Event::ResultSuccess:
      result = tr("done");
      QTimer::singleShot(5000, this, SLOT(resetTitle()));
      break;
    case Licq::Event::ResultCancelled:
      result = tr("cancelled");
      break;
    case Licq::Event::ResultFailed:
      result = tr("failed");
      break;
    case Licq::Event::ResultTimedout:
      result = tr("timed out");
      break;
    case Licq::Event::ResultError:
      result = tr("error");
      break;
    default:
      break;
  }
  title = myBaseTitle + " [" + myProgressMsg + result + "]";

  UserEventTabDlg* tabDlg = gLicqGui->userEventTabDlg();
  if (tabDlg != NULL && tabDlg->tabIsSelected(this))
    tabDlg->setWindowTitle(title);

  setWindowTitle(title);

  setCursor(Qt::ArrowCursor);
  mySendButton->setText(tr("&Send"));
  myCloseButton->setEnabled(true);

  if (myMessageEdit != NULL)
    myMessageEdit->setEnabled(true);

  disconnect(mySendButton, SIGNAL(clicked()), this, SLOT(cancelSend()));
  connect(mySendButton, SIGNAL(clicked()), SLOT(send()));

  // If cancelled automatically, check "Send through Server"
  if (Config::Chat::instance()->autoSendThroughServer() && e->Result() == Licq::Event::ResultCancelled)
    mySendServerCheck->setChecked(true);

  if (myEventTag.empty())
  {
    disconnect(gGuiSignalManager, SIGNAL(doneUserFcn(const Licq::Event*)),
        this, SLOT(eventDoneReceived(const Licq::Event*)));
  }

  if (myMessageEdit != NULL)
    if(tabDlg == NULL || !tabDlg->tabExists(this) || tabDlg->tabIsSelected(this))
      myMessageEdit->setFocus();

  if (e->Result() != Licq::Event::ResultAcked)
  {
    if (e->Command() == ICQ_CMDxTCP_START && e->Result() != Licq::Event::ResultCancelled &&
       (Config::Chat::instance()->autoSendThroughServer() ||
         QueryYesNo(this, tr("Direct send failed,\nsend through server?"))) )
    {
      // Remember that we want to send through server
      mySendServerCheck->setChecked(true);

      retrySend(e, false, ICQ_TCPxMSG_NORMAL);
    }
    return;
  }

  QString msg;

  if (e->SubResult() == ICQ_TCPxACK_RETURN)
  {
    {
      Licq::UserWriteGuard u(myUsers.front());

      msg = tr("%1 is in %2 mode:\n%3\nSend...")
          .arg(QString::fromUtf8(u->GetAlias()))
          .arg(u->statusString().c_str())
          .arg(myCodec->toUnicode(u->autoResponse().c_str()));

      u->SetShowAwayMsg(false);
    }

    // if the original message was through server, send this one through server
    bool throughServer = false;
    switch (e->Channel())
    {
      case ICQ_CHNxNONE: // Fall through
      case ICQ_CHNxINFO:
      case ICQ_CHNxSTATUS:
        throughServer = true;
    }

    switch (QueryUser(this, msg, tr("Urgent"), tr(" to Contact List"), tr("Cancel")))
    {
      case 0:
        retrySend(e, throughServer, ICQ_TCPxMSG_URGENT);
        break;
      case 1:
        retrySend(e, throughServer, ICQ_TCPxMSG_LIST);
        break;
      case 2:
        break;
    }
    return;
  }

  emit autoCloseNotify();
  if (sendDone(e))
  {
    emit eventSent(e);
    if (Config::Chat::instance()->msgChatView() && myHistoryView != NULL)
    {
      myHistoryView->GotoEnd();
      resetSettings();

      // After sending URI/File/Contact/ChatRequest switch back to text message
      if (myType != MessageEvent)
        changeEventType(MessageEvent);
    }
    else
      close();
  }
}

void UserSendCommon::cancelSend()
{
  unsigned long icqEventTag = 0;

  if (myEventTag.size())
    icqEventTag = myEventTag.front();

  if (icqEventTag == 0)
    return closeDialog(); // if we're not sending atm, let ESC close the window

  UserEventTabDlg* tabDlg = gLicqGui->userEventTabDlg();
  if (tabDlg != NULL && tabDlg->tabIsSelected(this))
    tabDlg->setWindowTitle(myBaseTitle);

  Licq::gDaemon.cancelEvent(icqEventTag);
}

void UserSendCommon::changeEventType(QAction* action)
{
  changeEventType(action->data().toInt());
}

void UserSendCommon::clearNewEvents()
{
  // Iterate all users in the conversation
  for (list<Licq::UserId>::iterator it = myUsers.begin(); it != myUsers.end(); ++it)
  {
    Licq::UserWriteGuard u(*it);
    if (!u.isLocked())
      continue;

    UserEventTabDlg* tabDlg = gLicqGui->userEventTabDlg();
    if (Config::Chat::instance()->msgChatView() &&
        isActiveWindow() &&
        (tabDlg == NULL || (!tabDlg->tabExists(this) || tabDlg->tabIsSelected(this))))
    {
      if (u->NewMessages() > 0)
      {
        std::vector<int> idList;
        for (unsigned short i = 0; i < u->NewMessages(); i++)
        {
          const Licq::UserEvent* e = u->EventPeek(i);
          if (e->Id() <= myHighestEventId && e->isReceiver() &&
              (e->SubCommand() == ICQ_CMDxSUB_MSG ||
               e->SubCommand() == ICQ_CMDxSUB_URL))
            idList.push_back(e->Id());
        }

        for (std::vector<int>::size_type i = 0; i < idList.size(); i++)
          u->EventClearId(idList[i]);
      }
    }
  }
}

void UserSendCommon::closeDialog()
{
  gProtocolManager.sendTypingNotification(myUsers.front(), false, myConvoId);

  if (Config::Chat::instance()->msgChatView())
  {
    // the window is at the front, if the timer has not expired and we close
    // the window, then the new events will stay there
    clearNewEvents();
  }

  if (myMessageEdit)
    Config::Chat::instance()->setCheckSpelling(myMessageEdit->checkSpellingEnabled());
  close();
}

void UserSendCommon::showEmoticonsMenu()
{
  // If no emoticons are available, don't display an empty window
  if (Emoticons::self()->emoticonsKeys().size() <= 0)
    return;

  SelectEmoticon* p = new SelectEmoticon(this);

  QWidget* desktop = qApp->desktop();
  QSize s = p->sizeHint();
  QWidget* button = myToolBar->widgetForAction(myEmoticon);
  QPoint pos = QPoint(0, button->height());
  pos = button->mapToGlobal(pos);
  if (pos.x() + s.width() > desktop->width())
  {
    pos.setX(desktop->width() - s.width());
    if (pos.x() < 0)
      pos.setX(0);
  }
  if (pos.y() + s.height() > desktop->height())
  {
    pos.setY(pos.y() - button->height() - s.height());
    if (pos.y() < 0)
      pos.setY(0);
  }

  connect(p, SIGNAL(selected(const QString&)), SLOT(insertEmoticon(const QString&)));
  p->move(pos);
  p->show();
}

void UserSendCommon::insertEmoticon(const QString& value)
{
  if (myMessageEdit)
    myMessageEdit->insertPlainText(value);
}

/*! This slot creates/removes a little widget into the usereventdlg
 *  which enables the user to collect users for mass messaging.
 */
void UserSendCommon::massMessageToggled(bool b)
{
  if (myMassMessageBox == NULL)
  {
    // If the box doesn't exist and we're not supposed to show it, there is no point in continuing
    if (!b)
      return;

    myMassMessageBox = new QGroupBox();
    myTophLayout->addWidget(myMassMessageBox);
    QVBoxLayout* layMR = new QVBoxLayout(myMassMessageBox);

    layMR->addWidget(new QLabel(tr("Drag Users Here\nRight Click for Options")));

    myMassMessageList = new MMUserView(myUsers.front(), gGuiContactList);
    myMassMessageList->setFixedWidth(gMainWindow->getUserView()->width());
    layMR->addWidget(myMassMessageList);
  }

  myMassMessageCheck->setChecked(b);
  myMassMessageBox->setVisible(b);
}

void UserSendCommon::messageAdded()
{
  UserEventTabDlg* tabDlg = gLicqGui->userEventTabDlg();
  if (isActiveWindow() &&
      (!Config::Chat::instance()->tabbedChatting() ||
       (tabDlg != NULL && tabDlg->tabIsSelected(this))))
    QTimer::singleShot(clearDelay, this, SLOT(clearNewEvents()));
}

void UserSendCommon::resetTitle()
{
  UserEventTabDlg* tabDlg = gLicqGui->userEventTabDlg();
  if (tabDlg != NULL && tabDlg->tabIsSelected(this))
    tabDlg->setWindowTitle(myBaseTitle);

  setWindowTitle(myBaseTitle);
}

void UserSendCommon::sendServerToggled(bool sendServer)
{
  // When the "Send through server" checkbox is toggled by the user,
  // we save the setting to disk, so it is persistent.

  Licq::UserWriteGuard u(myUsers.front());
  if (u.isLocked())
    u->SetSendServer(sendServer);
}

void UserSendCommon::setBackgroundICQColor()
{
  if (myMessageEdit == NULL)
    return;

  QColor c = myMessageEdit->palette().color(QPalette::Base);
#ifdef USE_KDE
  if (KColorDialog::getColor(c, this) != KColorDialog::Accepted)
    return;
#else
  c = QColorDialog::getColor(c, this);
  if (!c.isValid())
    return;
#endif

  myIcqColor.setBackground(c.red(), c.green(), c.blue());
  myMessageEdit->setBackground(c);
}

void UserSendCommon::setForegroundICQColor()
{
  if (myMessageEdit == NULL)
    return;

  QColor c = myMessageEdit->palette().color(QPalette::Text);
#ifdef USE_KDE
  if (KColorDialog::getColor(c, this) != KColorDialog::Accepted)
    return;
#else
  c = QColorDialog::getColor(c, this);
  if (!c.isValid())
    return;
#endif

  myIcqColor.setForeground(c.red(), c.green(), c.blue());
  myMessageEdit->setForeground(c);
}

void UserSendCommon::showSendTypeMenu()
{
  // Menu is normally delayed but if we use InstantPopup mode shortcut won't work
  dynamic_cast<QToolButton*>(myToolBar->widgetForAction(myEventTypeMenu))->showMenu();
}

void UserSendCommon::messageTextChanged()
{
  if (myMessageEdit == NULL || myMessageEdit->toPlainText().isEmpty())
    return;

  myTempMessage = myMessageEdit->toPlainText();
  gProtocolManager.sendTypingNotification(myUsers.front(), true, myConvoId);
  disconnect(myMessageEdit, SIGNAL(textChanged()), this, SLOT(messageTextChanged()));
  mySendTypingTimer->start(5000);
}

void UserSendCommon::textChangedTimeout()
{
  if (myMessageEdit == NULL)
  {
    mySendTypingTimer->stop();
    return;
  }

  QString str = myMessageEdit->toPlainText();

  if (str != myTempMessage)
  {
    myTempMessage = str;
    // Hack to not keep sending the typing notification to ICQ
    if (myPpid != LICQ_PPID)
      gProtocolManager.sendTypingNotification(myUsers.front(), true, myConvoId);
  }
  else
  {
    if (mySendTypingTimer->isActive())
      mySendTypingTimer->stop();
    connect(myMessageEdit, SIGNAL(textChanged()), SLOT(messageTextChanged()));
    gProtocolManager.sendTypingNotification(myUsers.front(), false, myConvoId);
  }
}

void UserSendCommon::sendTrySecure()
{
  bool autoSecure = false;
  {
    Licq::UserReadGuard u(myUsers.front());
    if (u.isLocked())
    {
      autoSecure = (u->AutoSecure() && Licq::gDaemon.haveCryptoSupport() &&
          u->SecureChannelSupport() == Licq::SECURE_CHANNEL_SUPPORTED &&
          !mySendServerCheck->isChecked() && !u->Secure());
    }
  }

  disconnect(mySendButton, SIGNAL(clicked()), this, SLOT(sendTrySecure()));
  connect(mySendButton, SIGNAL(clicked()), SLOT(send()));

  if (autoSecure)
  {
    QWidget* w = new KeyRequestDlg(myUsers.front());
    connect(w, SIGNAL(destroyed()), SLOT(send()));
  }
  else
    send();
}

void UserSendCommon::resizeEvent(QResizeEvent* event)
{
  Config::Chat::instance()->setSendDialogSize(size());
  UserEventCommon::resizeEvent(event);
}

void UserSendCommon::dragEnterEvent(QDragEnterEvent* event)
{
  if (event->mimeData()->hasText() ||
      event->mimeData()->hasUrls())
    event->acceptProposedAction();
}

void UserSendCommon::dropEvent(QDropEvent* event)
{
  event->ignore();

  if (gLicqGui->userDropEvent(myUsers.front(), *event->mimeData()))
    event->acceptProposedAction();
}
