// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2009 Licq developers
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

#include <licq_icq.h>
#include <licq_icqd.h>
#include <licq_log.h>
#include <licq_translate.h>

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
#include "helpers/licqstrings.h"
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
using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::UserSendCommon */

const size_t SHOW_RECENT_NUM = 5;

typedef pair<const CUserEvent*, UserId> messagePair;

bool orderMessagePairs(const messagePair& mp1, const messagePair& mp2)
{
  return (mp1.first->Time() < mp2.first->Time());
}

UserSendCommon::UserSendCommon(int type, const UserId& userId, QWidget* parent, const char* name)
  : UserEventCommon(userId, parent, name),
    myType(type)
{
  myMassMessageBox = NULL;
  myPictureLabel = NULL;
  clearDelay = 250;

  QShortcut* a = new QShortcut(Qt::Key_Escape, this);
  connect(a, SIGNAL(activated()), SLOT(cancelSend()));

  UserEventTabDlg* tabDlg = LicqGui::instance()->userEventTabDlg();
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
  ADD_SENDTYPE(PP_SEND_MSG, MessageEvent, tr("Message"));
  ADD_SENDTYPE(PP_SEND_URL, UrlEvent, tr("URL"));
  ADD_SENDTYPE(PP_SEND_CHAT, ChatEvent, tr("Chat Request"));
  ADD_SENDTYPE(PP_SEND_FILE, FileEvent, tr("File Transfer"));
  ADD_SENDTYPE(PP_SEND_CONTACT, ContactEvent, tr("Contact List"));
  ADD_SENDTYPE(PP_SEND_SMS, SmsEvent, tr("SMS"));

#undef ADD_SENDTYPE

  QMenu* mnuSendType = new QMenu(this);
  mnuSendType->addActions(myEventTypeGroup->actions());

  myEventTypeMenu = myToolBar->addAction(tr("Message type"), this, SLOT(showSendTypeMenu()));
  myEventTypeMenu->setMenu(mnuSendType);
  if (eventTypesCount <= 1)
    myEventTypeMenu->setEnabled(false);

  mySendServerCheck = myToolBar->addAction(tr("Send through server"));
  mySendServerCheck->setCheckable(true);

  bool canSendDirect = (mySendFuncs & PP_SEND_DIRECT);

  const LicqUser* u = gUserManager.fetchUser(myUsers.front());

  if (u != NULL)
  {
    mySendServerCheck->setChecked(u->SendServer() ||
        (u->StatusOffline() && u->SocketDesc(ICQ_CHNxNONE) == -1));

    if (u->GetInGroup(GROUPS_SYSTEM, GROUP_INVISIBLE_LIST) ||
        (u->Port() == 0 && u->SocketDesc(ICQ_CHNxNONE) == -1))
      canSendDirect = false;

    gUserManager.DropUser(u);
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

    u = gUserManager.fetchUser(myUsers.front());
    if (u != NULL && Config::Chat::instance()->showHistory())
    {
      // Show the last SHOW_RECENT_NUM messages in the history
      HistoryList lHistoryList;
      if (u->GetHistory(lHistoryList))
      {
        // Rewind to the starting point. This will be the first message shown in the dialog.
        // Make sure we don't show the new messages waiting.
        unsigned short nNewMessages = u->NewMessages();
        HistoryListIter lHistoryIter = lHistoryList.end();
        for (size_t i = 0; i < (SHOW_RECENT_NUM + nNewMessages) && lHistoryIter != lHistoryList.begin(); i++)
          lHistoryIter--;

        bool bUseHTML = !isdigit(u->accountId()[1]);
        const QTextCodec* myCodec = UserCodec::codecForUser(u);
        QString contactName = QString::fromUtf8(u->GetAlias());
        const ICQOwner* o = gUserManager.FetchOwner(u->ppid(), LOCK_R);
        QString ownerName;
        if (o)
        {
          ownerName = QString::fromUtf8(o->GetAlias());
          gUserManager.DropOwner(o);
        }
        else
          ownerName = QString(tr("Error! no owner set"));

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
            messageText = QString::fromUtf8((*lHistoryIter)->Text());
          else
            messageText = myCodec->toUnicode((*lHistoryIter)->Text());

          myHistoryView->addMsg(
              (*lHistoryIter)->Direction(),
              true,
              (*lHistoryIter)->SubCommand() == ICQ_CMDxSUB_MSG ? "" : EventDescription(*lHistoryIter) + " ",
              date,
              (*lHistoryIter)->IsDirect(),
              (*lHistoryIter)->IsMultiRec(),
              (*lHistoryIter)->IsUrgent(),
              (*lHistoryIter)->IsEncrypted(),
              (*lHistoryIter)->Direction() == D_RECEIVER ? contactName : ownerName,
              MLView::toRichText(messageText, true, bUseHTML));
          lHistoryIter++;
        }

        myHistoryView->GotoEnd();

        LicqUser::ClearHistory(lHistoryList);
      }
    }

    // Collect all messages to put them in the correct time order
    vector<messagePair> messages;

    // add all unread messages.
    if (u != 0 && u->NewMessages() > 0)
    {
      for (unsigned short i = 0; i < u->NewMessages(); i++)
      {
        const CUserEvent* e = u->EventPeek(i);
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
      gUserManager.DropUser(u);

      // Now add messages that are a part of this convo
      if (myPpid != LICQ_PPID)
      {
        FOR_EACH_PROTO_USER_START(myPpid, LOCK_R)
        {
          if (pUser->NewMessages() && myUsers.front() != pUser->id())
          {
            for (unsigned short i = 0; i < pUser->NewMessages(); i++)
            {
              const CUserEvent* e = pUser->EventPeek(i);

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
        FOR_EACH_PROTO_USER_END
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
      if (gLicqDaemon->FindConversation(myConvoId) == 0)
        myConvoId = 0;

      // Fetch the user again since we dropped it above
      u = gUserManager.fetchUser(myUsers.front());
    }

    // Do we already have an open socket?
    if (myConvoId == 0 && u != 0)
    {
//       bool bHasOpen = false;
//       QPtrListIterator<UserSendCommon> it(licqUserSend);
//       for (; it.current(); ++it)
//       {
//         if ((*it)->Id() == myId && (*it)->PPID() == myPpid)
//         {
//           bHasOpen = true;
//           break;
//         }
//       }

      if (u->SocketDesc(ICQ_CHNxNONE) != 1)
      {
        CConversation* pConv = gLicqDaemon->FindConversation(u->SocketDesc(ICQ_CHNxNONE));
        if (pConv != 0)
          myConvoId = pConv->CID();
      }
    }

    gUserManager.DropUser(u);

    connect(LicqGui::instance(), SIGNAL(eventSent(const LicqEvent*)),
        myHistoryView, SLOT(addMsg(const LicqEvent*)));
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

  myIcqColor.SetToDefault();
  myMessageEdit->setBackground(QColor(myIcqColor.BackRed(), myIcqColor.BackGreen(), myIcqColor.BackBlue()));
  myMessageEdit->setForeground(QColor(myIcqColor.ForeRed(), myIcqColor.ForeGreen(), myIcqColor.ForeBlue()));

  updateIcons();
  updatePicture();
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
    UserEventTabDlg* tabDlg = LicqGui::instance()->userEventTabDlg();
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

void UserSendCommon::updatePicture(const LicqUser* u)
{
  bool fetched = false;

  if (u == NULL)
  {
    u = gUserManager.fetchUser(myUsers.front());
    fetched = true;
  }
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
    QString picPath = QString(BASE_DIR) + USER_DIR + "/" + u->IdString() + ".pic";
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

  if (fetched)
    gUserManager.DropUser(u);
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

void UserSendCommon::convoJoin(const UserId& userId)
{
  if (!USERID_ISVALID(userId))
    return;

  if (Config::Chat::instance()->msgChatView())
  {
    const LicqUser* u = gUserManager.fetchUser(userId);
    QString userName;
    if (u != 0)
    {
      userName = QString::fromUtf8(u->GetAlias());
      gUserManager.DropUser(u);
    }
    else
      userName = "";

    myHistoryView->addNotice(QDateTime::currentDateTime(),
        tr("%1 has joined the conversation.").arg(userName));
  }

  if (!isUserInConvo(userId))
    myUsers.push_back(userId);

  // Now update the tab label
  UserEventTabDlg* tabDlg = LicqGui::instance()->userEventTabDlg();
  if (tabDlg != NULL)
    tabDlg->updateConvoLabel(this);
}

void UserSendCommon::convoLeave(const UserId& userId)
{
  if (!USERID_ISVALID(userId))
    return;

  if (Config::Chat::instance()->msgChatView())
  {
    LicqUser* u = gUserManager.fetchUser(userId, LOCK_W);
    QString userName;
    if (u != 0)
      userName = QString::fromUtf8(u->GetAlias());
    else
      userName = "";

    myHistoryView->addNotice(QDateTime::currentDateTime(),
        tr("%1 has left the conversation.").arg(userName));

    // Remove the typing notification if active
    if (u != 0)
    {
      if (u->GetTyping() == ICQ_TYPING_ACTIVE)
      {
        u->SetTyping(ICQ_TYPING_INACTIVEx0);
        myTimezone->setPalette(QPalette());
        UserEventTabDlg* tabDlg = LicqGui::instance()->userEventTabDlg();
        if (Config::Chat::instance()->tabbedChatting() && tabDlg != NULL)
          tabDlg->updateTabLabel(u);
      }
      gUserManager.DropUser(u);
    }
  }

  if (myUsers.size() > 1)
  {
    list<UserId>::iterator it;
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
    UserEventTabDlg* tabDlg = LicqGui::instance()->userEventTabDlg();
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

  UserEventTabDlg* tabDlg = LicqGui::instance()->userEventTabDlg();
  if (tabDlg != NULL && tabDlg->tabExists(this))
    parent = tabDlg;

  UserId userId = myUsers.front();

  switch (type)
  {
    case MessageEvent:
      if (mySendFuncs & PP_SEND_MSG)
        e = new UserSendMsgEvent(userId, parent);
      break;
    case UrlEvent:
      if (mySendFuncs & PP_SEND_URL)
        e = new UserSendUrlEvent(userId, parent);
      break;
    case ChatEvent:
      if (mySendFuncs & PP_SEND_CHAT)
        e = new UserSendChatEvent(userId, parent);
      break;
    case FileEvent:
      if (mySendFuncs & PP_SEND_FILE)
        e = new UserSendFileEvent(userId, parent);
      break;
    case ContactEvent:
      if (mySendFuncs & PP_SEND_CONTACT)
        e = new UserSendContactEvent(userId, parent);
      break;
    case SmsEvent:
      if (mySendFuncs & PP_SEND_SMS)
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

    LicqGui::instance()->replaceEventDialog(this, e, userId);
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

void UserSendCommon::retrySend(const LicqEvent* e, bool online, unsigned short level)
{
  QString accountId = LicqUser::getUserAccountId(myUsers.front()).c_str();

  unsigned long icqEventTag = 0;
  mySendServerCheck->setChecked(!online);
  myUrgentCheck->setChecked(level == ICQ_TCPxMSG_URGENT);

  switch (e->UserEvent()->SubCommand() & ~ICQ_CMDxSUB_FxMULTIREC)
  {
    case ICQ_CMDxSUB_MSG:
    {
      const LicqUser* u = gUserManager.fetchUser(myUsers.front());
      bool userOffline = true;
      if (u != 0)
      {
        userOffline = u->StatusOffline();
        gUserManager.DropUser(u);
      }
      const CEventMsg* ue = dynamic_cast<const CEventMsg*>(e->UserEvent());
      // create initial strings (implicit copying, no allocation impact :)
      char* tmp = gTranslator.NToRN(ue->Message());
      QByteArray wholeMessageRaw(tmp);
      delete [] tmp;
      int wholeMessagePos = 0;

      bool needsSplitting = false;
      // If we send through myServer (= have message limit), and we've crossed the limit
      unsigned short maxSize = userOffline ? MAX_OFFLINE_MESSAGE_SIZE : MAX_MESSAGE_SIZE;
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
          messageRaw = wholeMessageRaw.mid(wholeMessagePos, maxSize);
          tmp = gTranslator.RNToN(messageRaw);
          messageRaw = tmp;
          delete [] tmp;
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
          messageRaw = ue->Message();
        }

        icqEventTag = gLicqDaemon->sendMessage(myUsers.front(), messageRaw.data(),
            !online, level, false, &myIcqColor);

        myEventTag.push_back(icqEventTag);

        tmp = gTranslator.NToRN(messageRaw);
        wholeMessagePos += strlen(tmp);
        delete [] tmp;
      }

      icqEventTag = 0;

      break;
    }

    case ICQ_CMDxSUB_URL:
    {
      const CEventUrl* ue = dynamic_cast<const CEventUrl*>(e->UserEvent());

      icqEventTag = gLicqDaemon->sendUrl(myUsers.front(), ue->Url(),
          ue->Description(), !online, level, false, &myIcqColor);

      break;
    }

    case ICQ_CMDxSUB_CONTACTxLIST:
    {
      const CEventContactList* ue = dynamic_cast<const CEventContactList*>(e->UserEvent());
      const ContactList& clist = ue->Contacts();
      StringList users;

      // ContactList is const but string list holds "char*" so we have to copy each string
      for (ContactList::const_iterator i = clist.begin(); i != clist.end(); i++)
        users.push_back(LicqUser::getUserAccountId((*i)->userId()));

      if (users.size() == 0)
        break;

      icqEventTag = gLicqDaemon->icqSendContactList(accountId.toLatin1(),
          users, online, level, false, &myIcqColor);

      break;
    }

    case ICQ_CMDxSUB_CHAT:
    {
      const CEventChat* ue = dynamic_cast<const CEventChat*>(e->UserEvent());

      if (ue->Clients() == NULL)
        //TODO in the daemon
        icqEventTag = gLicqDaemon->icqChatRequest(accountId.toLatin1(),
            ue->Reason(), level, !online);
      else
        //TODO in the daemon
        icqEventTag = gLicqDaemon->icqMultiPartyChatRequest(accountId.toLatin1(),
            ue->Reason(), ue->Clients(), ue->Port(), level, !online);

      break;
    }

    case ICQ_CMDxSUB_FILE:
    {
      const CEventFile* ue = dynamic_cast<const CEventFile*>(e->UserEvent());
      ConstFileList filelist(ue->FileList());

      //TODO in the daemon
      icqEventTag = gLicqDaemon->fileTransferPropose(myUsers.front(),
          ue->Filename(), ue->FileDescription(), filelist, level, !online);

      break;
    }

    case ICQ_CMDxSUB_SMS:
    {
      const CEventSms* ue = dynamic_cast<const CEventSms*>(e->UserEvent());

      //TODO in the daemon
      icqEventTag = gLicqDaemon->icqSendSms(accountId.toLatin1(), LICQ_PPID,
          ue->Number(), ue->Message());
      break;
    }

    default:
    {
      gLog.Warn("%sInternal error: UserSendCommon::RetrySend()\n"
                "%sUnknown sub-command %d.\n", L_WARNxSTR, L_BLANKxSTR, e->SubCommand());
      break;
    }
  }

  if (icqEventTag)
    myEventTag.push_back(icqEventTag);

  UserSendCommon::send();
}

void UserSendCommon::userUpdated(const UserId& userId, unsigned long subSignal, int argument, unsigned long cid)
{
  LicqUser* u = gUserManager.fetchUser(userId, LOCK_W);

  if (u == NULL)
    return;

  switch (subSignal)
  {
    case USER_STATUS:
    {
      if (u->Port() == 0)
      {
        mySendServerCheck->setChecked(true);
        mySendServerCheck->setEnabled(false);
      }
      else
        mySendServerCheck->setEnabled(true);

      if (u->StatusOffline())
        mySendServerCheck->setChecked(true);

      break;
    }

    case USER_EVENTS:
    {
      const CUserEvent* e = u->EventPeekId(argument);

      if (e != NULL && myHighestEventId < argument &&
          myHistoryView && argument > 0)
      {
        myHighestEventId = argument;
        e = u->EventPeekId(argument);

        if (e != NULL)
          if (u->ppid() != MSN_PPID || cid == myConvoId)
          {
            gUserManager.DropUser(u);
            myHistoryView->addMsg(e, userId);
            return;
          }
      }
      break;
    }

    case USER_SECURITY:
      // Automatically unset 'send through server' upon
      // establishing secure channel
      if (u->Secure())
      {
        u->SetSendServer(false);
        mySendServerCheck->setChecked(false);
      }
      break;

    case USER_PICTURE:
      updatePicture(u);
  }

  gUserManager.DropUser(u);
}

bool UserSendCommon::checkSecure()
{
  const LicqUser* u = gUserManager.fetchUser(myUsers.front());

  if (u == NULL)
    return false;

  bool secure = u->Secure() || u->AutoSecure();
  bool send_ok = true;

  gUserManager.DropUser(u);

  if (mySendServerCheck->isChecked() && secure)
  {
    if (!QueryYesNo(this, tr("Message can't be sent securely through the server!\n"
            "Send anyway?")))
      send_ok = false;
    else
    {
      LicqUser* u = gUserManager.fetchUser(myUsers.front(), LOCK_W);
      if (u != NULL)
      {
        u->SetAutoSecure(false);
        gUserManager.DropUser(u);
      }
    }
  }
  return send_ok;
}

void UserSendCommon::send()
{
  if (!Config::Chat::instance()->manualNewUser())
  {
    bool newUser = false;
    LicqUser* u = gUserManager.fetchUser(myUsers.front(), LOCK_W);
    if (u != NULL)
    {
      if (u->NewUser())
        newUser = true;
      gUserManager.DropUser(u);
    }
    if (newUser)
      gUserManager.setUserInGroup(myUsers.front(), GROUPS_SYSTEM, GROUP_NEW_USERS, false);
  }

  unsigned long icqEventTag = 0;

  if (myEventTag.size() != 0)
    icqEventTag = myEventTag.front();

  LicqUser* user = gUserManager.fetchUser(myUsers.front());
  unsigned long myPpid = user->ppid();
  gUserManager.DropUser(user);

  if (icqEventTag != 0 || myPpid != LICQ_PPID)
  {
    bool via_server = mySendServerCheck->isChecked();
    myProgressMsg = tr("Sending ");
    myProgressMsg += via_server ? tr("via server") : tr("direct");
    myProgressMsg += "...";
    QString title = myBaseTitle + " [" + myProgressMsg + "]";

    UserEventTabDlg* tabDlg = LicqGui::instance()->userEventTabDlg();
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

    connect(LicqGui::instance()->signalManager(), SIGNAL(eventTag(const UserId&, unsigned long)),
        SLOT(addEventTag(const UserId&, unsigned long)));
    connect(LicqGui::instance()->signalManager(),
        SIGNAL(doneUserFcn(const LicqEvent*)), SLOT(eventDoneReceived(const LicqEvent*)));
  }
}

void UserSendCommon::addEventTag(const UserId& userId, unsigned long eventTag)
{
  if (eventTag == 0 || !USERID_ISVALID(userId) || userId != myUsers.front())
    return;

  myEventTag.push_back(eventTag);
}

void UserSendCommon::eventDoneReceived(const LicqEvent* e)
{
  if (e == NULL)
  {
    QString title = myBaseTitle + " [" + myProgressMsg + tr("error") + "]";

    UserEventTabDlg* tabDlg = LicqGui::instance()->userEventTabDlg();
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
    case EVENT_ACKED: // Fall through
    case EVENT_SUCCESS:
      result = tr("done");
      QTimer::singleShot(5000, this, SLOT(resetTitle()));
      break;
    case EVENT_CANCELLED:
      result = tr("cancelled");
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
  title = myBaseTitle + " [" + myProgressMsg + result + "]";

  UserEventTabDlg* tabDlg = LicqGui::instance()->userEventTabDlg();
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
  if (Config::Chat::instance()->autoSendThroughServer() && e->Result() == EVENT_CANCELLED)
    mySendServerCheck->setChecked(true);

  if (myEventTag.size() == 0)
  {
    disconnect(LicqGui::instance()->signalManager(), SIGNAL(eventTag(const UserId&, unsigned long)),
        this, SLOT(addEventTag(const UserId&, unsigned long)));
    disconnect(LicqGui::instance()->signalManager(),
        SIGNAL(doneUserFcn(const LicqEvent*)), this, SLOT(eventDoneReceived(const LicqEvent*)));
  }

  if (myMessageEdit != NULL)
    if(tabDlg == NULL || !tabDlg->tabExists(this) || tabDlg->tabIsSelected(this))
      myMessageEdit->setFocus();

  if (e->Result() != EVENT_ACKED)
  {
    if (e->Command() == ICQ_CMDxTCP_START && e->Result() != EVENT_CANCELLED &&
       (Config::Chat::instance()->autoSendThroughServer() ||
         QueryYesNo(this, tr("Direct send failed,\nsend through server?"))) )
      retrySend(e, false, ICQ_TCPxMSG_NORMAL);
    return;
  }

  LicqUser* u = NULL;
  QString msg;

  if (e->SubResult() == ICQ_TCPxACK_RETURN)
  {
    u = gUserManager.fetchUser(myUsers.front(), LOCK_W);

    msg = tr("%1 is in %2 mode:\n%3\nSend...")
      .arg(QString::fromUtf8(u->GetAlias()))
      .arg(LicqStrings::getStatus(u))
      .arg(myCodec->toUnicode(u->AutoResponse()));

    u->SetShowAwayMsg(false);
    gUserManager.DropUser(u);

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

  UserEventTabDlg* tabDlg = LicqGui::instance()->userEventTabDlg();
  if (tabDlg != NULL && tabDlg->tabIsSelected(this))
    tabDlg->setWindowTitle(myBaseTitle);

  gLicqDaemon->CancelEvent(icqEventTag);
}

void UserSendCommon::changeEventType(QAction* action)
{
  changeEventType(action->data().toInt());
}

void UserSendCommon::clearNewEvents()
{
  LicqUser* u = NULL;

  // Iterate all users in the conversation
  for (list<UserId>::iterator it = myUsers.begin(); it != myUsers.end(); ++it)
  {
    u = gUserManager.fetchUser(*it, LOCK_W);
    if (u != NULL)
    {
      UserEventTabDlg* tabDlg = LicqGui::instance()->userEventTabDlg();
      if (Config::Chat::instance()->msgChatView() &&
          isActiveWindow() &&
          (tabDlg == NULL || (!tabDlg->tabExists(this) || tabDlg->tabIsSelected(this))))
      {
        if (u->NewMessages() > 0)
        {
          std::vector<int> idList;
          for (unsigned short i = 0; i < u->NewMessages(); i++)
          {
            const CUserEvent* e = u->EventPeek(i);
            if (e->Id() <= myHighestEventId &&
                e->Direction() == D_RECEIVER &&
                (e->SubCommand() == ICQ_CMDxSUB_MSG ||
                 e->SubCommand() == ICQ_CMDxSUB_URL))
              idList.push_back(e->Id());
          }

          for (unsigned short i = 0; i < idList.size(); i++)
            u->EventClearId(idList[i]);
        }
      }
      gUserManager.DropUser(u);
      u = NULL;
    }
  }
}

void UserSendCommon::closeDialog()
{
  gLicqDaemon->sendTypingNotification(myUsers.front(), false, myConvoId);

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

    myMassMessageList = new MMUserView(myUsers.front(),
        LicqGui::instance()->contactList());
    myMassMessageList->setFixedWidth(gMainWindow->getUserView()->width());
    layMR->addWidget(myMassMessageList);
  }

  myMassMessageCheck->setChecked(b);
  myMassMessageBox->setVisible(b);
}

void UserSendCommon::messageAdded()
{
  UserEventTabDlg* tabDlg = LicqGui::instance()->userEventTabDlg();
  if (isActiveWindow() &&
      (!Config::Chat::instance()->tabbedChatting() ||
       (tabDlg != NULL && tabDlg->tabIsSelected(this))))
    QTimer::singleShot(clearDelay, this, SLOT(clearNewEvents()));
}

void UserSendCommon::resetTitle()
{
  UserEventTabDlg* tabDlg = LicqGui::instance()->userEventTabDlg();
  if (tabDlg != NULL && tabDlg->tabIsSelected(this))
    tabDlg->setWindowTitle(myBaseTitle);

  setWindowTitle(myBaseTitle);
}

void UserSendCommon::sendServerToggled(bool sendServer)
{
  // When the "Send through server" checkbox is toggled by the user,
  // we save the setting to disk, so it is persistent.

  LicqUser* u = gUserManager.fetchUser(myUsers.front(), LOCK_W);
  if (u != NULL)
  {
    u->SetSendServer(sendServer);
    gUserManager.DropUser(u);
  }
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

  myIcqColor.SetBackground(c.red(), c.green(), c.blue());
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

  myIcqColor.SetForeground(c.red(), c.green(), c.blue());
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
  gLicqDaemon->sendTypingNotification(myUsers.front(), true, myConvoId);
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
      gLicqDaemon->sendTypingNotification(myUsers.front(), true, myConvoId);
  }
  else
  {
    if (mySendTypingTimer->isActive())
      mySendTypingTimer->stop();
    connect(myMessageEdit, SIGNAL(textChanged()), SLOT(messageTextChanged()));
    gLicqDaemon->sendTypingNotification(myUsers.front(), false, myConvoId);
  }
}

void UserSendCommon::sendTrySecure()
{
  const LicqUser* u = gUserManager.fetchUser(myUsers.front());

  bool autoSecure = false;
  if (u != NULL)
  {
    autoSecure = (u->AutoSecure() && gLicqDaemon->CryptoEnabled() &&
        u->SecureChannelSupport() == SECURE_CHANNEL_SUPPORTED &&
        !mySendServerCheck->isChecked() && !u->Secure());
    gUserManager.DropUser(u);
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

  if (LicqGui::instance()->userDropEvent(myUsers.front(), *event->mimeData()))
    event->acceptProposedAction();
}
