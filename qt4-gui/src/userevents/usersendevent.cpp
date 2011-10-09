/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2011 Licq developers
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

#include "usersendevent.h"

#include "config.h"

#include <cassert>
#include <ctime>
#include <boost/foreach.hpp>

#include <QAction>
#include <QApplication>
#include <QColorDialog>
#include <QDateTime>
#include <QDesktopWidget>
#include <QDialogButtonBox>
#include <QDropEvent>
#include <QFileInfo>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
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
#include <KDE/KFileDialog>
#endif

#include <licq/logging/log.h>
#include <licq/contactlist/owner.h>
#include <licq/contactlist/usermanager.h>
#include <licq/conversation.h>
#include <licq/daemon.h>
#include <licq/event.h>
#include <licq/icq/icq.h>
#include <licq/plugin/protocolplugin.h>
#include <licq/pluginsignal.h>
#include <licq/protocolmanager.h>
#include <licq/protocolsignal.h>
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

#include "dialogs/chatdlg.h"
#include "dialogs/filedlg.h"
#include "dialogs/editfilelistdlg.h"
#include "dialogs/joinchatdlg.h"
#include "dialogs/keyrequestdlg.h"
#include "dialogs/mmsenddlg.h"
#include "dialogs/showawaymsgdlg.h"

#include "helpers/usercodec.h"

#include "views/mmuserview.h"
#include "views/userview.h"

#include "widgets/historyview.h"
#include "widgets/infofield.h"
#include "widgets/mledit.h"

#include "selectemoticon.h"
#include "usereventtabdlg.h"

using namespace std;
using Licq::StringList;
using Licq::gProtocolManager;
using Licq::gConvoManager;
using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::UserSendEvent */

typedef pair<const Licq::UserEvent*, Licq::UserId> messagePair;

bool orderMessagePairs(const messagePair& mp1, const messagePair& mp2)
{
  return (mp1.first->Time() < mp2.first->Time());
}

UserSendEvent::UserSendEvent(int type, const Licq::UserId& userId, QWidget* parent)
  : UserEventCommon(userId, parent, "UserSendEvent"),
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
  myMainWidget->addWidget(myViewSplitter);

  myHistoryView = 0;
  if (Config::Chat::instance()->msgChatView())
  {
    myHistoryView = new HistoryView(false, myUsers.front(), myViewSplitter);
    connect(myHistoryView, SIGNAL(messageAdded()), SLOT(messageAdded()));

    Licq::UserReadGuard u(myUsers.front());
    int userSocketDesc = (u.isLocked() ? u->normalSocketDesc() : -1);
    int historyCount = Config::Chat::instance()->showHistoryCount();
    int historyTime = Config::Chat::instance()->showHistoryTime();
    if (u.isLocked() && (historyCount > 0 || historyTime > 0))
    {
      // Show recent messages in the history
      Licq::HistoryList lHistoryList;
      if (u->GetHistory(lHistoryList))
      {
        // Rewind to the starting point. This will be the first message shown in the dialog.
        // Make sure we don't show the new messages waiting.
        unsigned short nNewMessages = u->NewMessages();
        Licq::HistoryList::iterator lHistoryIter = lHistoryList.end();
        for (int i = 0; i < historyCount + nNewMessages && lHistoryIter != lHistoryList.begin(); i++)
          lHistoryIter--;

        time_t timeLimit = time(NULL) - historyTime;
        while (lHistoryIter != lHistoryList.begin())
        {
          lHistoryIter--;

          if ((*lHistoryIter)->Time() < timeLimit)
          {
            // Found a message that is older than we want, go back one step and stop looking
            lHistoryIter++;
            break;
          }

          // One more message from history to show
          historyCount++;
        }

        bool bUseHTML = !isdigit(u->accountId()[1]);
        const QTextCodec* myCodec = UserCodec::codecForUser(*u);
        QString contactName = QString::fromUtf8(u->getAlias().c_str());
        QString ownerName;
        {
          Licq::OwnerReadGuard o(u->protocolId());
          if (o.isLocked())
            ownerName = QString::fromUtf8(o->getAlias().c_str());
          else
            ownerName = QString(tr("Error! no owner set"));
        }

        // Iterate through each message to add
        // Only show old messages as recent ones. Don't show duplicates.
        int nMaxNumToShow;
        if (lHistoryList.size() <= static_cast<size_t>(historyCount))
          nMaxNumToShow = lHistoryList.size() - nNewMessages;
        else
          nMaxNumToShow = historyCount;

        // Safety net
        if (nMaxNumToShow < 0)
          nMaxNumToShow = 0;

        QDateTime date;

        for (int i = 0; i < nMaxNumToShow && lHistoryIter != lHistoryList.end(); i++)
        {
          QString str;
          date.setTime_t((*lHistoryIter)->Time());
          QString messageText;
          if ((*lHistoryIter)->eventType() == Licq::UserEvent::TypeSms) // SMSs are always in UTF-8
            messageText = QString::fromUtf8((*lHistoryIter)->text().c_str());
          else
            messageText = myCodec->toUnicode((*lHistoryIter)->text().c_str());

          myHistoryView->addMsg(
              (*lHistoryIter)->isReceiver(),
              true,
              (*lHistoryIter)->eventType() == Licq::UserEvent::TypeMessage ? "" : ((*lHistoryIter)->description() + " ").c_str(),
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
      if (myConvoId != 0)
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


  // Extra controls for URL
  myUrlControls = new QWidget();
  QHBoxLayout* urlLayout = new QHBoxLayout(myUrlControls);
  QLabel* urlLabel = new QLabel(tr("URL:"));
  urlLayout->addWidget(urlLabel);
  myUrlEdit = new InfoField(false);
  urlLayout->addWidget(myUrlEdit);
  urlLabel->setBuddy(myUrlEdit);
  myUrlEdit->installEventFilter(this);
  myUrlControls->setVisible(false);
  myMainWidget->addWidget(myUrlControls);

  // Extra controls for Chat Request
  myChatControls = new QWidget();
  QHBoxLayout* chatLayout = new QHBoxLayout(myChatControls);
  QLabel* chatLabel = new QLabel(tr("Multiparty:"));
  chatLayout->addWidget(chatLabel);
  myChatItemEdit = new InfoField(false);
  chatLayout->addWidget(myChatItemEdit);
  myChatInviteButton = new QPushButton(tr("Invite"));
  chatLayout->addWidget(myChatInviteButton);
  myChatControls->setVisible(false);
  myMainWidget->addWidget(myChatControls);
  myChatPort = 0;

  // Extra controls for File Transfer
  myFileControls = new QWidget();
  QHBoxLayout* fileLayout = new QHBoxLayout(myFileControls);
  QLabel* fileLabel = new QLabel(tr("File(s):"));
  fileLayout->addWidget(fileLabel);
  myFileEdit = new InfoField(true);
  fileLayout->addWidget(myFileEdit);
  myFileBrowseButton = new QPushButton(tr("Browse"));
  fileLayout->addWidget(myFileBrowseButton);
  myFileEditButton = new QPushButton(tr("Edit"));
  myFileEditButton->setEnabled(false);
  fileLayout->addWidget(myFileEditButton);
  myFileControls->setVisible(false);
  myMainWidget->addWidget(myFileControls);

  // Extra controls for Contacts
  myContactsControls = new QWidget();
  QVBoxLayout* contactsLayout = new QVBoxLayout(myContactsControls);
  contactsLayout->setContentsMargins(0, 0, 0, 0);
  myContactsControls->setToolTip(tr("Drag Users Here - Right Click for Options"));
  myContactsList = new MMUserView(myUsers.front(), gGuiContactList);
  myContactsList->installEventFilter(this);
  contactsLayout->addWidget(myContactsList);
  myContactsControls->setVisible(false);
  myMainWidget->addWidget(myContactsControls);

  // Extra controls for SMS
  mySmsControls = new QWidget();
  QHBoxLayout* smsLayout = new QHBoxLayout(mySmsControls);
  QLabel* smsPhoneLabel = new QLabel(tr("Phone:"));
  smsLayout->addWidget(smsPhoneLabel);
  mySmsPhoneEdit = new InfoField(false);
  smsLayout->addWidget(mySmsPhoneEdit);
  mySmsPhoneEdit->setFixedWidth(qMax(140, mySmsPhoneEdit->sizeHint().width()));
  mySmsPhoneEdit->installEventFilter(this);
  smsPhoneLabel->setBuddy(mySmsPhoneEdit);
  smsLayout->addStretch(1);
  QLabel* smsCountLabel = new QLabel(tr("Chars left:"));
  smsLayout->addWidget(smsCountLabel);
  mySmsCountEdit = new InfoField(true);
  mySmsCountEdit->setFixedWidth(40);
  mySmsCountEdit->setAlignment(Qt::AlignCenter);
  smsLayout->addWidget(mySmsCountEdit);
  smsCountLabel->setBuddy(mySmsCountEdit);
  mySmsControls->setVisible(false);
  myMainWidget->addWidget(mySmsControls);


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
    mySmsPhoneEdit->setText(myCodec->toUnicode(u->getCellularNumber().c_str()));
  }
  updateShortcuts();

  updateEmoticons();
  connect(Emoticons::self(), SIGNAL(themeChanged()), SLOT(updateEmoticons()));

  connect(myMessageEdit, SIGNAL(ctrlEnterPressed()), mySendButton, SIGNAL(clicked()));
  connect(myMessageEdit, SIGNAL(textChanged()), SLOT(messageTextChanged()));
  connect(mySendServerCheck, SIGNAL(triggered(bool)), SLOT(sendServerToggled(bool)));

  connect(myChatInviteButton, SIGNAL(clicked()), SLOT(chatInviteUser()));
  connect(myFileBrowseButton, SIGNAL(clicked()), SLOT(fileBrowse()));
  connect(myFileEditButton, SIGNAL(clicked()), SLOT(fileEditList()));
  connect(myMessageEdit, SIGNAL(textChanged()), SLOT(smsCount()));

  QSize dialogSize = Config::Chat::instance()->sendDialogSize();
  if (dialogSize.isValid())
    resize(dialogSize);

  setAcceptDrops(true);

  setEventType();
  smsCount();
}

UserSendEvent::~UserSendEvent()
{
  // Empty
}

void UserSendEvent::closeEvent(QCloseEvent* event)
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

bool UserSendEvent::eventFilter(QObject* watched, QEvent* e)
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
  else if (watched == myUrlEdit || watched == myContactsList || watched == mySmsPhoneEdit)
  {
    if (e->type() == QEvent::KeyPress)
    {
      QKeyEvent* key = dynamic_cast<QKeyEvent*>(e);
      const bool isEnter = (key->key() == Qt::Key_Enter || key->key() == Qt::Key_Return);
      if (isEnter && (Config::Chat::instance()->singleLineChatMode() || key->modifiers() & Qt::ControlModifier))
      {
        mySendButton->animateClick();
        return true; // filter the event out
      }
    }
    return false;
  }
  else
    return UserEventCommon::eventFilter(watched, e);
}

void UserSendEvent::setEventType()
{
  myMassMessageCheck->setEnabled(myType == MessageEvent || myType == UrlEvent);
  myForeColor->setEnabled(myType == MessageEvent || myType == UrlEvent);
  myBackColor->setEnabled(myType == MessageEvent || myType == UrlEvent);
  myEmoticon->setEnabled(myType != ContactEvent);
  mySendServerCheck->setEnabled(myType != SmsEvent);
  myUrgentCheck->setEnabled(myType != SmsEvent);
  myEncoding->setEnabled(myType != SmsEvent); // SMSs are always UTF-8

  myMessageEdit->setVisible(myType != ContactEvent);

  switch (myType)
  {
    case UrlEvent:
      myTitle = myBaseTitle + tr(" - URL");
      break;
    case ChatEvent:
      myTitle = myBaseTitle + tr(" - Chat Request");
      myMassMessageCheck->setChecked(false);
      break;
    case FileEvent:
      myTitle = myBaseTitle + tr(" - File Transfer");
      myMassMessageCheck->setChecked(false);
      break;
    case ContactEvent:
      myTitle = myBaseTitle + tr(" - Contact List");
      myMassMessageCheck->setChecked(false);
      break;
    case SmsEvent:
      myTitle = myBaseTitle + tr(" - SMS");
      myMassMessageCheck->setChecked(false);
      mySendServerCheck->setChecked(true);
      myUrgentCheck->setChecked(false);
      break;
    case MessageEvent:
    default:
      myTitle = myBaseTitle + tr(" - Message");
      break;
  }
  setWindowTitle(myTitle);

  myUrlControls->setVisible(myType == UrlEvent);
  myChatControls->setVisible(myType == ChatEvent);
  myFileControls->setVisible(myType == FileEvent);
  myContactsControls->setVisible(myType == ContactEvent);
  mySmsControls->setVisible(myType == SmsEvent);

  myEventTypeGroup->actions().at(myType)->setChecked(true);

  if (myType != ContactEvent)
    myMessageEdit->setFocus();
}

void UserSendEvent::updateIcons()
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

void UserSendEvent::updateEmoticons()
{
  // Don't show tool button for emoticons if there are no emoticons to select
  myEmoticon->setVisible(Emoticons::self()->emoticonsKeys().size() > 0);
}

void UserSendEvent::updateShortcuts()
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

void UserSendEvent::updatePicture(const Licq::User* u)
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

const QPixmap& UserSendEvent::iconForType(int type) const
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

void UserSendEvent::setText(const QString& text)
{
  myMessageEdit->setText(text);
  myMessageEdit->GotoEnd();
  myMessageEdit->document()->setModified(false);
}

void UserSendEvent::setUrl(const QString& url, const QString& description)
{
  myUrlEdit->setText(url);
  setText(description);
}

void UserSendEvent::setContact(const Licq::UserId& userId)
{
  Licq::UserReadGuard u(userId);
  if (u.isLocked())
    myContactsList->add(u->id());
}

void UserSendEvent::setFile(const QString& file, const QString& description)
{
  QFileInfo fileinfo(file);
  if (fileinfo.exists() && fileinfo.isFile() && fileinfo.isReadable())
  {
    myFileEdit->setText(file);
    setText(description);
    myFileList.push_back(strdup(file.toLocal8Bit()));
    myFileEditButton->setEnabled(true);
  }
}

void UserSendEvent::addFile(const QString& file)
{
  if (myFileList.empty())
    return;

  myFileList.push_back(strdup(file.toLocal8Bit()));

  myFileEditButton->setEnabled(true);
  fileUpdateLabel(myFileList.size());
}

void UserSendEvent::convoJoin(const Licq::UserId& userId)
{
  if (!userId.isValid())
    return;

  if (Config::Chat::instance()->msgChatView())
  {
    Licq::UserReadGuard u(userId);
    QString userName;
    if (u.isLocked())
      userName = QString::fromUtf8(u->getAlias().c_str());
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

void UserSendEvent::convoLeave(const Licq::UserId& userId)
{
  if (!userId.isValid())
    return;

  if (Config::Chat::instance()->msgChatView())
  {
    Licq::UserWriteGuard u(userId);
    QString userName;
    if (u.isLocked())
      userName = QString::fromUtf8(u->getAlias().c_str());
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

void UserSendEvent::windowActivationChange(bool oldActive)
{
  if (isActiveWindow())
    QTimer::singleShot(clearDelay, this, SLOT(clearNewEvents()));
  QWidget::windowActivationChange(oldActive);
}

void UserSendEvent::changeEventType(int type)
{
  if (myType == type)
    return;

  switch (type)
  {
    case MessageEvent:
      if ((mySendFuncs & Licq::ProtocolPlugin::CanSendMsg) == 0)
        return;
      break;
    case UrlEvent:
      if ((mySendFuncs & Licq::ProtocolPlugin::CanSendUrl) == 0)
        return;
      break;
    case ChatEvent:
      if ((mySendFuncs & Licq::ProtocolPlugin::CanSendChat) == 0)
        return;
      break;
    case FileEvent:
      if ((mySendFuncs & Licq::ProtocolPlugin::CanSendFile) == 0)
        return;
      break;
    case ContactEvent:
      if ((mySendFuncs & Licq::ProtocolPlugin::CanSendContact) == 0)
        return;
      break;
    case SmsEvent:
      if ((mySendFuncs & Licq::ProtocolPlugin::CanSendSms) == 0)
        return;
      break;
    default:
      assert(false);
  }

  myType = type;
  setEventType();
}

void UserSendEvent::retrySend(const Licq::Event* e, unsigned flags)
{
  QString accountId = myUsers.front().accountId().c_str();

  unsigned long icqEventTag = 0;
  mySendServerCheck->setChecked((flags & Licq::ProtocolSignal::SendDirect) == 0);
  myUrgentCheck->setChecked(flags & Licq::ProtocolSignal::SendUrgent);

  switch (e->userEvent()->eventType())
  {
    case Licq::UserEvent::TypeMessage:
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
            flags, &myIcqColor);

        myEventTag.push_back(icqEventTag);

        wholeMessagePos += Licq::gTranslator.returnToDos(messageRaw.data()).size();
      }

      icqEventTag = 0;

      break;
    }

    case Licq::UserEvent::TypeUrl:
    {
      const Licq::EventUrl* ue = dynamic_cast<const Licq::EventUrl*>(e->userEvent());

      icqEventTag = gProtocolManager.sendUrl(myUsers.front(), ue->url(),
          ue->description(), flags, &myIcqColor);

      break;
    }

    case Licq::UserEvent::TypeContactList:
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
          users, flags, &myIcqColor);

      break;
    }

    case Licq::UserEvent::TypeChat:
    {
      const Licq::EventChat* ue = dynamic_cast<const Licq::EventChat*>(e->userEvent());

      if (ue->clients().empty())
        //TODO in the daemon
        icqEventTag = gLicqDaemon->icqChatRequest(myUsers.front(),
            ue->reason(), flags);
      else
        //TODO in the daemon
        icqEventTag = gLicqDaemon->icqMultiPartyChatRequest(myUsers.front(),
            ue->reason(), ue->clients(), ue->Port(), flags);

      break;
    }

    case Licq::UserEvent::TypeFile:
    {
      const Licq::EventFile* ue = dynamic_cast<const Licq::EventFile*>(e->userEvent());
      list<string> filelist(ue->FileList());

      //TODO in the daemon
      icqEventTag = gProtocolManager.fileTransferPropose(myUsers.front(),
          ue->filename(), ue->fileDescription(), filelist, flags);

      break;
    }

    case Licq::UserEvent::TypeSms:
    {
      const Licq::EventSms* ue = dynamic_cast<const Licq::EventSms*>(e->userEvent());

      //TODO in the daemon
      icqEventTag = gLicqDaemon->icqSendSms(myUsers.front(),
          ue->number().c_str(), ue->message().c_str());
      break;
    }

    default:
      break;
  }

  if (icqEventTag)
    myEventTag.push_back(icqEventTag);

  sendBase();
}

void UserSendEvent::userUpdated(const Licq::UserId& userId, unsigned long subSignal, int argument, unsigned long cid)
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
          if (u->protocolId() != MSN_PPID || cid == myConvoId)
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

void UserSendEvent::send()
{
  if (myType == MessageEvent || myType == SmsEvent)
  {
    // don't let the user send empty messages
    if (myMessageEdit->toPlainText().trimmed().isEmpty())
      return;

    // do nothing if a command is already being processed
    if (myEventTag.size() > 0 && myEventTag.front() != 0)
      return;

    if (!myMessageEdit->document()->isModified() &&
        !QueryYesNo(this, (myType == SmsEvent ?
            tr("You didn't edit the SMS.\nDo you really want to send it?") :
            tr("You didn't edit the message.\nDo you really want to send it?"))))
      return;
  }

  if (myType == UrlEvent && myUrlEdit->text().trimmed().isEmpty())
  {
    InformUser(this, tr("No URL specified"));
    return;
  }
  if (myType == FileEvent && myFileEdit->text().trimmed().isEmpty())
  {
    WarnUser(this, tr("You must specify a file to transfer!"));
    return;
  }
  if (myType == ContactEvent && myContactsList->contacts().empty())
    return;

  bool secure = false;;
  bool offline = true;
  {
    Licq::UserReadGuard u(myUsers.front());
    if (u.isLocked())
    {
      secure = u->Secure() || u->AutoSecure();
      offline = !u->isOnline();
    }
  }

  if ((myType == MessageEvent || myType == UrlEvent || myType == ContactEvent) &&
      secure && mySendServerCheck->isChecked())
  {
    if (!QueryYesNo(this, tr("Message can't be sent securely through the server!\n"
            "Send anyway?")))
      return;

    Licq::UserWriteGuard u(myUsers.front());
    if (u.isLocked())
      u->SetAutoSecure(false);
  }


  // Take care of typing notification now`
  if (mySendTypingTimer->isActive())
    mySendTypingTimer->stop();

  if (myType != ContactEvent)
    connect(myMessageEdit, SIGNAL(textChanged()), SLOT(messageTextChanged()));
  gProtocolManager.sendTypingNotification(myUsers.front(), false, myConvoId);

  StringList contacts;
  Licq::UserId userId;
  foreach (userId, myContactsList->contacts())
  {
    contacts.push_back(userId.accountId());
  }

  if (myMassMessageCheck->isChecked())
  {
    MMSendDlg* m = new MMSendDlg(myMassMessageList, this);
    connect(m, SIGNAL(eventSent(const Licq::Event*)), SIGNAL(eventSent(const Licq::Event*)));
    int r = QDialog::Accepted;
    if (myType == UrlEvent)
      r = m->go_url(myUrlEdit->text(), myMessageEdit->toPlainText());
    else if (myType == ContactEvent)
      r = m->go_contact(contacts);
    else
      m->go_message(myMessageEdit->toPlainText());
    if (r != QDialog::Accepted)
      return;
  }

  unsigned flags = 0;
  if (!mySendServerCheck->isChecked())
    flags |= Licq::ProtocolSignal::SendDirect;
  if (myUrgentCheck->isChecked())
    flags |= Licq::ProtocolSignal::SendUrgent;
  if (myMassMessageCheck->isChecked())
    flags |= Licq::ProtocolSignal::SendToMultiple;

  if (myType == MessageEvent)
  {
    QByteArray wholeMessageRaw(Licq::gTranslator.returnToDos(myCodec->fromUnicode(myMessageEdit->toPlainText()).data()).c_str());
    int wholeMessagePos = 0;

    bool needsSplitting = false;
    // If we send through server (= have message limit), and we've crossed the limit
    unsigned short maxSize = offline ? CICQDaemon::MaxOfflineMessageSize : CICQDaemon::MaxMessageSize;
    if (mySendServerCheck->isChecked() && ((wholeMessageRaw.length() - wholeMessagePos) > maxSize))
      needsSplitting = true;

    while (wholeMessageRaw.length() > wholeMessagePos)
    {
      QByteArray messageRaw;

      if (needsSplitting)
      {
        // This is a bit ugly but adds safety. We don't simply search
        // for a whitespace to cut at in the encoded text (since we don't
        // really know how spaces are represented in its encoding), so
        // we take the maximum length, then convert back to a Unicode string
        // and then search for Unicode whitespaces.
        messageRaw = Licq::gTranslator.returnToUnix(wholeMessageRaw.mid(wholeMessagePos, maxSize).data()).c_str();
        QString message = myCodec->toUnicode(messageRaw);

        if (wholeMessageRaw.length() - wholeMessagePos > maxSize)
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
        messageRaw = myCodec->fromUnicode(myMessageEdit->toPlainText());
      }

      unsigned long icqEventTag = gProtocolManager.sendMessage(
          myUsers.front(),
          messageRaw.data(),
          flags,
          &myIcqColor,
          myConvoId);
      if (icqEventTag != 0)
        myEventTag.push_back(icqEventTag);

      wholeMessagePos += Licq::gTranslator.returnToDos(messageRaw.data()).size();
    }
  }
  else
  {
    unsigned long icqEventTag = 0;

    switch (myType)
    {
      case UrlEvent:
        icqEventTag = gProtocolManager.sendUrl(
            myUsers.front(),
            myUrlEdit->text().toLatin1().constData(),
            myCodec->fromUnicode(myMessageEdit->toPlainText()).data(),
            flags,
            &myIcqColor);
        break;

      case ContactEvent:
        //TODO Fix this for new protocol plugin
        icqEventTag = gLicqDaemon->icqSendContactList(
            myUsers.front(),
            contacts,
            flags,
            &myIcqColor);
        break;

      case ChatEvent:
        if (myChatPort == 0)
          //TODO in daemon
          icqEventTag = gLicqDaemon->icqChatRequest(
              myUsers.front(),
              myCodec->fromUnicode(myMessageEdit->toPlainText()).data(),
              flags);
        else
          icqEventTag = gLicqDaemon->icqMultiPartyChatRequest(
              myUsers.front(),
              myCodec->fromUnicode(myMessageEdit->toPlainText()).data(),
              myCodec->fromUnicode(myChatClients).data(),
              myChatPort,
              flags);
        break;

      case FileEvent:
        //TODO in daemon
        icqEventTag = gProtocolManager.fileTransferPropose(
            myUsers.front(),
            myCodec->fromUnicode(myFileEdit->text()).data(),
            myCodec->fromUnicode(myMessageEdit->toPlainText()).data(),
            myFileList,
            flags);
        break;

      case SmsEvent:
        icqEventTag = gLicqDaemon->icqSendSms(myUsers.front(),
            mySmsPhoneEdit->text().toLatin1().constData(),
            myMessageEdit->toPlainText().toUtf8().data());
        break;
    }

    myEventTag.push_back(icqEventTag);
  }

  sendBase();
}

void UserSendEvent::sendBase()
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
    QString title = myTitle + " [" + myProgressMsg + "]";

    UserEventTabDlg* tabDlg = gLicqGui->userEventTabDlg();
    if (tabDlg != NULL && tabDlg->tabIsSelected(this))
      tabDlg->setWindowTitle(title);

    setWindowTitle(title);
    setCursor(Qt::WaitCursor);
    mySendButton->setText(tr("&Cancel"));
    myCloseButton->setEnabled(false);
    myMessageEdit->setEnabled(false);

    disconnect(mySendButton, SIGNAL(clicked()), this, SLOT(send()));
    connect(mySendButton, SIGNAL(clicked()), SLOT(cancelSend()));

    connect(gGuiSignalManager, SIGNAL(doneUserFcn(const Licq::Event*)),
        SLOT(eventDoneReceived(const Licq::Event*)));
  }
}

void UserSendEvent::eventDoneReceived(const Licq::Event* e)
{
  if (e == NULL)
  {
    QString title = myTitle + " [" + myProgressMsg + tr("error") + "]";

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
  title = myTitle + " [" + myProgressMsg + result + "]";

  UserEventTabDlg* tabDlg = gLicqGui->userEventTabDlg();
  if (tabDlg != NULL && tabDlg->tabIsSelected(this))
    tabDlg->setWindowTitle(title);

  setWindowTitle(title);

  setCursor(Qt::ArrowCursor);
  mySendButton->setText(tr("&Send"));
  myCloseButton->setEnabled(true);
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

  if (myType != ContactEvent)
    if(tabDlg == NULL || !tabDlg->tabExists(this) || tabDlg->tabIsSelected(this))
      myMessageEdit->setFocus();

  if (e->Result() != Licq::Event::ResultAcked)
  {
    if ((e->flags() & Licq::Event::FlagDirect) &&
        e->Result() != Licq::Event::ResultCancelled &&
       (Config::Chat::instance()->autoSendThroughServer() ||
         QueryYesNo(this, tr("Direct send failed,\nsend through server?"))) )
    {
      // Remember that we want to send through server
      mySendServerCheck->setChecked(true);

      retrySend(e, 0);
    }
    return;
  }

  QString msg;

  if (e->subResult() == Licq::Event::SubResultReturn)
  {
    {
      Licq::UserWriteGuard u(myUsers.front());

      msg = tr("%1 is in %2 mode:\n%3\nSend...")
          .arg(QString::fromUtf8(u->getAlias().c_str()))
          .arg(u->statusString().c_str())
          .arg(myCodec->toUnicode(u->autoResponse().c_str()));

      u->SetShowAwayMsg(false);
    }

    // if the original message was through server, send this one through server
    unsigned flags = 0;
    if (e->userEvent()->IsDirect())
      flags |= Licq::ProtocolSignal::SendDirect;

    switch (QueryUser(this, msg, tr("Urgent"), tr(" to Contact List"), tr("Cancel")))
    {
      case 0:
        retrySend(e, flags | Licq::ProtocolSignal::SendUrgent);
        break;
      case 1:
        retrySend(e, flags | Licq::ProtocolSignal::SendToList);
        break;
      case 2:
        break;
    }
    return;
  }

  emit autoCloseNotify();


  switch (myType)
  {
    case UrlEvent:
    case ContactEvent:
    case MessageEvent:
    {
      if ((e->flags() & Licq::Event::FlagDirect) == 0)
        break;

      myMessageEdit->setText(QString::null);

      bool showAwayDlg = false;
      {
        Licq::UserReadGuard u(myUsers.front());
        if (u.isLocked())
          showAwayDlg = u->isAway() && u->ShowAwayMsg();
      }

      if (showAwayDlg && Config::Chat::instance()->popupAutoResponse())
        new ShowAwayMsgDlg(myUsers.front());

      break;
    }

    case FileEvent:
      if (!e->ExtendedAck() || !e->ExtendedAck()->accepted())
      {
        Licq::UserReadGuard u(myUsers.front());
        if (!u.isLocked())
          break;
        QString s = !e->ExtendedAck() ?
          tr("No reason provided") :
          myCodec->toUnicode(e->ExtendedAck()->response().c_str());
        QString result = tr("File transfer with %1 refused:\n%2")
          .arg(QString::fromUtf8(u->getAlias().c_str()))
          .arg(s);
        u.unlock();
        InformUser(this, result);
      }
      else
      {
        const Licq::EventFile* f = dynamic_cast<const Licq::EventFile*>(e->userEvent());
        FileDlg* fileDlg = new FileDlg(myUsers.front());
        fileDlg->SendFiles(f->FileList(), e->ExtendedAck()->port());
      }
      break;

    case ChatEvent:
      if (!e->ExtendedAck() || !e->ExtendedAck()->accepted())
      {
        Licq::UserReadGuard u(myUsers.front());
        QString s = !e->ExtendedAck() ?
          tr("No reason provided") :
          myCodec->toUnicode(e->ExtendedAck()->response().c_str());
        QString result = tr("Chat with %1 refused:\n%2")
          .arg(!u.isLocked() ? u->accountId().c_str() : QString::fromUtf8(u->getAlias().c_str()))
          .arg(s);
        u.unlock();
        InformUser(this, result);
      }
      else
      {
        const Licq::EventChat* c = dynamic_cast<const Licq::EventChat*>(e->userEvent());
        if (c->Port() == 0)  // If we requested a join, no need to do anything
        {
          ChatDlg* chatDlg = new ChatDlg(myUsers.front());
          chatDlg->StartAsClient(e->ExtendedAck()->port());
        }
      }
      break;
  }

  emit eventSent(e);
  if (Config::Chat::instance()->msgChatView() && myHistoryView != NULL)
  {
    myHistoryView->GotoEnd();

    myMessageEdit->clear();
    myMessageEdit->setFocus();

    // Makes the cursor blink so that the user sees that the text edit has focus.
    myMessageEdit->moveCursor(QTextCursor::Start);

    myUrlEdit->clear();
    myChatItemEdit->clear();
    myFileEdit->clear();
    myFileList.clear();
    myFileEditButton->setEnabled(false);
    myContactsList->clear();

    massMessageToggled(false);

    // After sending URI/File/Contact/ChatRequest switch back to text message
    if (myType != MessageEvent)
      changeEventType(MessageEvent);
  }
  else
    close();
}

void UserSendEvent::cancelSend()
{
  unsigned long icqEventTag = 0;

  if (myEventTag.size())
    icqEventTag = myEventTag.front();

  if (icqEventTag == 0)
    return closeDialog(); // if we're not sending atm, let ESC close the window

  UserEventTabDlg* tabDlg = gLicqGui->userEventTabDlg();
  if (tabDlg != NULL && tabDlg->tabIsSelected(this))
    tabDlg->setWindowTitle(myTitle);

  Licq::gDaemon.cancelEvent(icqEventTag);
}

void UserSendEvent::changeEventType(QAction* action)
{
  changeEventType(action->data().toInt());
}

void UserSendEvent::clearNewEvents()
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
              (e->eventType() == Licq::UserEvent::TypeMessage ||
              e->eventType() == Licq::UserEvent::TypeUrl))
            idList.push_back(e->Id());
        }

        for (std::vector<int>::size_type i = 0; i < idList.size(); i++)
          u->EventClearId(idList[i]);
      }
    }
  }
}

void UserSendEvent::closeDialog()
{
  gProtocolManager.sendTypingNotification(myUsers.front(), false, myConvoId);

  if (Config::Chat::instance()->msgChatView())
  {
    // the window is at the front, if the timer has not expired and we close
    // the window, then the new events will stay there
    clearNewEvents();
  }

  Config::Chat::instance()->setCheckSpelling(myMessageEdit->checkSpellingEnabled());
  close();
}

void UserSendEvent::showEmoticonsMenu()
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

void UserSendEvent::insertEmoticon(const QString& value)
{
  myMessageEdit->insertPlainText(value);
}

/*! This slot creates/removes a little widget into the usereventdlg
 *  which enables the user to collect users for mass messaging.
 */
void UserSendEvent::massMessageToggled(bool b)
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

void UserSendEvent::messageAdded()
{
  UserEventTabDlg* tabDlg = gLicqGui->userEventTabDlg();
  if (isActiveWindow() &&
      (!Config::Chat::instance()->tabbedChatting() ||
       (tabDlg != NULL && tabDlg->tabIsSelected(this))))
    QTimer::singleShot(clearDelay, this, SLOT(clearNewEvents()));
}

void UserSendEvent::resetTitle()
{
  UserEventTabDlg* tabDlg = gLicqGui->userEventTabDlg();
  if (tabDlg != NULL && tabDlg->tabIsSelected(this))
    tabDlg->setWindowTitle(myTitle);

  setWindowTitle(myTitle);
}

void UserSendEvent::sendServerToggled(bool sendServer)
{
  // When the "Send through server" checkbox is toggled by the user,
  // we save the setting to disk, so it is persistent.

  Licq::UserWriteGuard u(myUsers.front());
  if (u.isLocked())
    u->SetSendServer(sendServer);
}

void UserSendEvent::setBackgroundICQColor()
{
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

void UserSendEvent::setForegroundICQColor()
{
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

void UserSendEvent::showSendTypeMenu()
{
  // Menu is normally delayed but if we use InstantPopup mode shortcut won't work
  dynamic_cast<QToolButton*>(myToolBar->widgetForAction(myEventTypeMenu))->showMenu();
}

void UserSendEvent::messageTextChanged()
{
  if (myMessageEdit->toPlainText().isEmpty())
    return;

  myTempMessage = myMessageEdit->toPlainText();
  gProtocolManager.sendTypingNotification(myUsers.front(), true, myConvoId);
  disconnect(myMessageEdit, SIGNAL(textChanged()), this, SLOT(messageTextChanged()));
  mySendTypingTimer->start(5000);
}

void UserSendEvent::textChangedTimeout()
{
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

void UserSendEvent::sendTrySecure()
{
  bool autoSecure = false;
  {
    Licq::UserReadGuard u(myUsers.front());
    if (u.isLocked())
    {
      autoSecure = (u->AutoSecure() && Licq::gDaemon.haveCryptoSupport() &&
          u->secureChannelSupport() == Licq::User::SecureChannelSupported &&
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

void UserSendEvent::resizeEvent(QResizeEvent* event)
{
  Config::Chat::instance()->setSendDialogSize(size());
  UserEventCommon::resizeEvent(event);
}

void UserSendEvent::dragEnterEvent(QDragEnterEvent* event)
{
  if (event->mimeData()->hasText() ||
      event->mimeData()->hasUrls())
    event->acceptProposedAction();
}

void UserSendEvent::dropEvent(QDropEvent* event)
{
  event->ignore();

  if (gLicqGui->userDropEvent(myUsers.front(), *event->mimeData()))
    event->acceptProposedAction();
}

void UserSendEvent::chatInviteUser()
{
  if (myChatPort == 0)
  {
    if (ChatDlg::chatDlgs.size() > 0)
    {
      ChatDlg* chatDlg = NULL;
      JoinChatDlg* j = new JoinChatDlg(true, this);
      if (j->exec() && (chatDlg = j->JoinedChat()) != NULL)
      {
        myChatItemEdit->setText(j->ChatClients());
        myChatPort = chatDlg->LocalPort();
        myChatClients = chatDlg->ChatName() + ", " + chatDlg->ChatClients();
      }
      delete j;
      myChatInviteButton->setText(tr("Clear"));
    }
  }
  else
  {
    myChatPort = 0;
    myChatClients = "";
    myChatItemEdit->setText("");
    myChatInviteButton->setText(tr("Invite"));
  }
}

void UserSendEvent::fileBrowse()
{
#ifdef USE_KDE
  QStringList fl = KFileDialog::getOpenFileNames(KUrl(), QString(), this, tr("Select files to send"));
#else
  QStringList fl = QFileDialog::getOpenFileNames(this, tr("Select files to send"));
#endif

  if (fl.isEmpty())
    return;

  QStringList::ConstIterator it = fl.begin();

  for(; it != fl.end(); it++)
    myFileList.push_back(strdup((*it).toLocal8Bit()));

  fileUpdateLabel(myFileList.size());
}

void UserSendEvent::fileEditList()
{
  EditFileListDlg* dlg = new EditFileListDlg(&myFileList);
  connect(dlg, SIGNAL(fileDeleted(unsigned)), SLOT(fileUpdateLabel(unsigned)));
}

void UserSendEvent::fileUpdateLabel(unsigned count)
{
  myFileEditButton->setEnabled(count > 0);

  QString f;

  switch (count)
  {
    case 0:
      f = QString::null;
      break;

    case 1:
      f = myFileList.front().c_str();
      break;

    default:
      f = QString(tr("%1 Files")).arg(count);
      break;
  }

  myFileEdit->setText(f);
}

void UserSendEvent::smsCount()
{
  int len = 160 - strlen(myMessageEdit->toPlainText().toUtf8().data());
  mySmsCountEdit->setText((len >= 0) ? len : 0);
}
