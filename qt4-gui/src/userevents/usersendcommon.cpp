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

#include "usersendcommon.h"

#include "config.h"

#include <assert.h>

#include <QAction>
#include <QApplication>
#include <QColorDialog>
#include <QDateTime>
#include <QDesktopWidget>
#include <QDialogButtonBox>
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

#include <licq_icqd.h>
#include <licq_log.h>
#include <licq_translate.h>

#include "config/chat.h"
#include "config/emoticons.h"
#include "config/iconmanager.h"

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

using std::pair;
using std::make_pair;
using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::UserSendCommon */

const size_t SHOW_RECENT_NUM = 5;

typedef pair<CUserEvent*, char*> messagePair;

bool orderMessagePairs(const messagePair& mp1, const messagePair& mp2)
{
  return (mp1.first->Time() < mp2.first->Time());
}

UserSendCommon::UserSendCommon(int type, QString id, unsigned long ppid, QWidget* parent, const char* name)
  : UserEventCommon(id, ppid, parent, name),
    myType(type)
{
  grpMR = NULL;
  clearDelay = 250;

  QShortcut* a = new QShortcut(Qt::Key_Escape, this);
  connect(a, SIGNAL(activated()), SLOT(slotCancelSend()));

  UserEventTabDlg* tabDlg = LicqGui::instance()->userEventTabDlg();
  if (tabDlg != NULL && parent == tabDlg)
  {
    a = new QShortcut(Qt::ALT + Qt::Key_Left, this);
    connect(a, SIGNAL(activated()), tabDlg, SLOT(slotMoveLeft()));

    a = new QShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Tab, this);
    connect(a, SIGNAL(activated()), tabDlg, SLOT(slotMoveLeft()));

    a = new QShortcut(Qt::ALT + Qt::Key_Right, this);
    connect(a, SIGNAL(activated()), tabDlg, SLOT(slotMoveRight()));

    a = new QShortcut(Qt::CTRL + Qt::Key_Tab, this);
    connect(a, SIGNAL(activated()), tabDlg, SLOT(slotMoveRight()));
  }

  grpSendType = new QActionGroup(this);
  connect(grpSendType, SIGNAL(triggered(QAction*)), SLOT(slotChangeEventType(QAction*)));

  QAction* action;

  action = new QAction(tr("Message"), grpSendType);
  action->setData(ET_MESSAGE);
  action->setCheckable(true);

  action = new QAction(tr("URL"), grpSendType);
  action->setData(ET_URL);
  action->setCheckable(true);

  action = new QAction(tr("Chat Request"), grpSendType);
  action->setData(ET_CHAT);
  action->setCheckable(true);

  action = new QAction(tr("File Transfer"), grpSendType);
  action->setData(ET_FILE);
  action->setCheckable(true);

  action = new QAction(tr("Contact List"), grpSendType);
  action->setData(ET_CONTACT);
  action->setCheckable(true);

  action = new QAction(tr("SMS"), grpSendType);
  action->setData(ET_SMS);
  action->setCheckable(true);

  QMenu* mnuSendType = new QMenu(this);
  mnuSendType->addActions(grpSendType->actions());

  cmbSendType = myToolBar->addAction(tr("Message type"), this, SLOT(slotShowSendTypeMenu()));
  cmbSendType->setShortcut(Qt::ALT + Qt::Key_P);
  pushToolTip(cmbSendType, tr("Select type of message to send"));
  cmbSendType->setMenu(mnuSendType);
  if (ppid == MSN_PPID)
    cmbSendType->setEnabled(false);

  chkSendServer = myToolBar->addAction(tr("Send through server"));
  chkSendServer->setShortcut(Qt::ALT + Qt::Key_N);
  pushToolTip(chkSendServer, tr("Send through server"));
  chkSendServer->setCheckable(true);

  ICQUser* u = gUserManager.FetchUser(myUsers.front().c_str(), myPpid, LOCK_R);

  if (u != 0)
  {
    chkSendServer->setChecked(u->SendServer() ||
        (u->StatusOffline() && u->SocketDesc(ICQ_CHNxNONE) == -1));

    if (u->GetInGroup(GROUPS_SYSTEM, GROUP_INVISIBLE_LIST) ||
        u->PPID() == MSN_PPID ||
        (u->Port() == 0 && u->SocketDesc(ICQ_CHNxNONE) == -1))
    {
      chkSendServer->setChecked(true);
      chkSendServer->setEnabled(false);
    }

    gUserManager.DropUser(u);
  }
  else
  {
    chkSendServer->setChecked(true);
    chkSendServer->setEnabled(false);
  }

  chkUrgent = myToolBar->addAction(tr("Urgent"));
  chkUrgent->setShortcut(Qt::ALT + Qt::Key_R);
  pushToolTip(chkUrgent, tr("Urgent"));
  chkUrgent->setCheckable(true);

  chkMass = myToolBar->addAction(tr("Multiple Recipients"));
  chkMass->setShortcut(Qt::ALT + Qt::Key_U);
  pushToolTip(chkMass, tr("Multiple recipients"));
  chkMass->setCheckable(true);
  connect(chkMass, SIGNAL(toggled(bool)), SLOT(slotMassMessageToggled(bool)));

  myToolBar->addSeparator();

  myEmoticon = myToolBar->addAction(tr("Smileys"), this, SLOT(slotEmoticon()));
  myEmoticon->setShortcut(Qt::ALT + Qt::Key_L);
  pushToolTip(myEmoticon, tr("Insert smileys"));

  myForeColor = myToolBar->addAction(tr("Text Color..."), this, SLOT(slotSetForegroundICQColor()));
  myForeColor->setShortcut(Qt::ALT + Qt::Key_T);
  pushToolTip(myForeColor, tr("Change text color"));

  myBackColor = myToolBar->addAction(tr("Background Color..."), this, SLOT(slotSetBackgroundICQColor()));
  myBackColor->setShortcut(Qt::ALT + Qt::Key_B);
  pushToolTip(myBackColor, tr("Change background color"));

  QDialogButtonBox* buttons = new QDialogButtonBox();
  top_lay->addWidget(buttons);

  btnSend = buttons->addButton(tr("&Send"), QDialogButtonBox::ActionRole);
  btnSend->setDefault(true);
  // add a wrapper around the send button that
  // tries to establish a secure connection first.
  connect(btnSend, SIGNAL(clicked()), SLOT(sendTrySecure()));

  btnClose = buttons->addButton(QDialogButtonBox::Close);
  btnClose->setAutoDefault(true);
  connect(btnClose, SIGNAL(clicked()), SLOT(slotClose()));

  buttons->setVisible(Config::Chat::instance()->showSendClose());

  splView = new QSplitter(Qt::Vertical);
  top_lay->addWidget(splView);

  mleHistory = 0;
  if (Config::Chat::instance()->msgChatView())
  {
    mleHistory = new HistoryView(false, myUsers.front().c_str(), myPpid, splView);
    connect(mleHistory, SIGNAL(messageAdded()), SLOT(slotMessageAdded()));

    u = gUserManager.FetchUser(myUsers.front().c_str(), myPpid, LOCK_R);
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

        bool bUseHTML = !isdigit((myUsers.front().c_str())[1]);
        QTextCodec* myCodec = UserCodec::codecForICQUser(u);
        QString contactName = QString::fromUtf8(u->GetAlias());
        ICQOwner* o = gUserManager.FetchOwner(myPpid, LOCK_R);
        QString ownerName;
        if (o)
        {
          ownerName = QString::fromUtf8(o->GetAlias());
          gUserManager.DropOwner(myPpid);
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

          mleHistory->addMsg(
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

        mleHistory->GotoEnd();

        ICQUser::ClearHistory(lHistoryList);
      }
    }

    // Collect all messages to put them in the correct time order
    vector<messagePair> messages;

    // add all unread messages.
    if (u != 0 && u->NewMessages() > 0)
    {
      for (unsigned short i = 0; i < u->NewMessages(); i++)
      {
        CUserEvent* e = u->EventPeek(i);
        // Get the convo id now
        unsigned long convoId = e->ConvoId();
        if (myConvoId == 0)
          myConvoId = convoId;

        if (convoId == myConvoId)
        {
          if (e->Id() > myHighestEventId)
            myHighestEventId = e->Id();

          messages.push_back(make_pair(e, strdup(u->IdString())));
        }
      }
      gUserManager.DropUser(u);

      // Now add messages that are a part of this convo
      if (myPpid != LICQ_PPID)
      {
        FOR_EACH_PROTO_USER_START(myPpid, LOCK_R)
        {
          if (pUser->NewMessages() && strcmp(myUsers.front().c_str(), pUser->IdString()) != 0)
          {
            for (unsigned short i = 0; i < pUser->NewMessages(); i++)
            {
              CUserEvent* e = pUser->EventPeek(i);

              if (e->ConvoId() == myConvoId)
              {
                if (e->Id() > myHighestEventId)
                  myHighestEventId = e->Id();

                // add to the convo list (but what if they left by the time we open this?)
                char* realId;
                ICQUser::MakeRealId(pUser->IdString(), pUser->PPID(), realId);
                myUsers.push_back(realId);
                delete [] realId;
                messages.push_back(make_pair(e, strdup(pUser->IdString())));
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
      {
        mleHistory->addMsg((*messageIter).first, (*messageIter).second, myPpid);
        free((*messageIter).second);
      }
      messages.clear();

      // If the user closed the chat window, we have to make sure we aren't
      // using the old nConvoId
      if (gLicqDaemon->FindConversation(myConvoId) == 0)
        myConvoId = 0;

      // Fetch the user again since we dropped it above
      u = gUserManager.FetchUser(myUsers.front().c_str(), myPpid, LOCK_R);
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

    connect(mleHistory, SIGNAL(viewurl(QWidget*, QString)),
        LicqGui::instance(), SLOT(viewUrl(QWidget*, QString)));
    connect(gMainWindow, SIGNAL(signal_sentevent(ICQEvent*)),
            mleHistory, SLOT(addMsg(ICQEvent*)));
    //splView->setResizeMode(mleHistory, QSplitter::FollowSizeHint);
  }

  {
    tmrSendTyping = new QTimer(this);
    connect(tmrSendTyping, SIGNAL(timeout()), SLOT(slotTextChangedTimeout()));
  }

  QSplitter* bottom = new QSplitter(splView);

  mleSend = new MLEdit(true, bottom, true);
  mleSend->setSizeHintLines(3);
  mleSend->setCheckSpellingEnabled(Config::Chat::instance()->checkSpelling());
  mleSend->installEventFilter(this); // Enables send with enter

  if (Config::Chat::instance()->showUserPic())
  {
    u = gUserManager.FetchUser(myUsers.front().c_str(), myPpid, LOCK_R);
    if (u != NULL)
    {
      if (u->GetPicturePresent())
      {
        QString picPath = QString(BASE_DIR) + USER_DIR + "/" + u->IdString() + ".pic";
        QMovie* picMovie = new QMovie(picPath);
        if (picMovie->isValid())
        {
          QLabel* picLabel = new QLabel(bottom);
          picLabel->setMovie(picMovie);
          picMovie->start();
          picLabel->setFixedWidth(picLabel->sizeHint().width());
          if (Config::Chat::instance()->showUserPicHidden())
            bottom->setSizes(QList<int>() << 1 << 0);
        }
      }
      gUserManager.DropUser(u);
    }
  }

  if (Config::Chat::instance()->msgChatView())
  {
    splView->setStretchFactor(splView->indexOf(mleHistory), 1);
    splView->setStretchFactor(splView->indexOf(bottom), 0);

    connect(mleHistory, SIGNAL(quote(const QString&)),
        mleSend, SLOT(insertPlainText(const QString&)));
  }

  setFocusProxy(mleSend);
  if (Config::Chat::instance()->showSendClose())
  {
    setTabOrder(mleSend, btnSend);
    setTabOrder(btnSend, btnClose);
  }

  icqColor.SetToDefault();
  mleSend->setBackground(QColor(icqColor.BackRed(), icqColor.BackGreen(), icqColor.BackBlue()));
  mleSend->setForeground(QColor(icqColor.ForeRed(), icqColor.ForeGreen(), icqColor.ForeBlue()));

  updateIcons();

  connect(mleSend, SIGNAL(ctrlEnterPressed()), btnSend, SIGNAL(clicked()));
  connect(mleSend, SIGNAL(textChanged()), SLOT(messageTextChanged()));
  connect(this, SIGNAL(updateUser(CICQSignal*)), gMainWindow, SLOT(slot_updatedUser(CICQSignal*)));
  connect(chkSendServer, SIGNAL(triggered(bool)), SLOT(slotSendServerToggled(bool)));
}

UserSendCommon::~UserSendCommon()
{
  // Empty
}

bool UserSendCommon::eventFilter(QObject* watched, QEvent* e)
{
  if (watched == mleSend)
  {
    // If we're in single line chat mode we send messages with Enter and
    // insert new lines with Ctrl+Enter.
    if (Config::Chat::instance()->singleLineChatMode() && e->type() == QEvent::KeyPress)
    {
      QKeyEvent* key = dynamic_cast<QKeyEvent*>(e);
      const bool isEnter = (key->key() == Qt::Key_Enter || key->key() == Qt::Key_Return);
      if (isEnter)
      {
        if (key->modifiers() & Qt::ControlModifier)
          mleSend->insertPlainText("\n");
        else
          btnSend->animateClick();
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
  cmbSendType->setIcon(iconForType(myType));
  chkSendServer->setIcon(iconman->getIcon(IconManager::ThroughServerIcon));
  chkUrgent->setIcon(iconman->getIcon(IconManager::UrgentIcon));
  chkMass->setIcon(iconman->getIcon(IconManager::MultipleRecIcon));
  myEmoticon->setIcon(iconman->getIcon(IconManager::SmileIcon));
  myForeColor->setIcon(iconman->getIcon(IconManager::TextColorIcon));
  myBackColor->setIcon(iconman->getIcon(IconManager::BackColorIcon));

  // Update message type icons in menu
  foreach (QAction* a, grpSendType->actions())
    a->setIcon(iconForType(a->data().toInt()));
}

const QPixmap& UserSendCommon::iconForType(int type) const
{
  switch (type)
  {
    case ET_URL:
      return IconManager::instance()->getIcon(IconManager::UrlMessageIcon);

    case ET_CHAT:
      return IconManager::instance()->getIcon(IconManager::ChatMessageIcon);

    case ET_FILE:
      return IconManager::instance()->getIcon(IconManager::FileMessageIcon);

    case ET_CONTACT:
      return IconManager::instance()->getIcon(IconManager::ContactMessageIcon);

    case ET_SMS:
      return IconManager::instance()->getIcon(IconManager::SmsMessageIcon);

    case ET_MESSAGE:
    default:
      return IconManager::instance()->getIcon(IconManager::StandardMessageIcon);
  }
}

void UserSendCommon::setText(const QString& text)
{
  if (mleSend == 0)
    return;

  mleSend->setText(text);
  mleSend->GotoEnd();
  mleSend->document()->setModified(false);
}

void UserSendCommon::convoJoin(QString id, unsigned long convoId)
{
  if (id.isEmpty())
    return;

  if (Config::Chat::instance()->msgChatView())
  {
    ICQUser* u = gUserManager.FetchUser(id.toLatin1(), myPpid, LOCK_R);
    QString userName;
    if (u != 0)
    {
      userName = QString::fromUtf8(u->GetAlias());
      gUserManager.DropUser(u);
    }
    else
      userName = id;

    mleHistory->addNotice(QDateTime::currentDateTime(),
        tr("%1 has joined the conversation.").arg(userName));
  }

  if (!isUserInConvo(id))
  {
    char* realId;
    ICQUser::MakeRealId(id.toLatin1(), myPpid, realId);
    myUsers.push_back(realId);
    delete [] realId;
  }

  myConvoId = convoId;

  // Now update the tab label
  UserEventTabDlg* tabDlg = LicqGui::instance()->userEventTabDlg();
  if (tabDlg != NULL)
    tabDlg->updateConvoLabel(this);
}

void UserSendCommon::convoLeave(QString id, unsigned long /* convoId */)
{
  if (id.isEmpty())
    return;

  if (Config::Chat::instance()->msgChatView())
  {
    ICQUser* u = gUserManager.FetchUser(id.toLatin1(), myPpid, LOCK_R);
    QString userName;
    if (u != 0)
      userName = QString::fromUtf8(u->GetAlias());
    else
      userName = id;

    mleHistory->addNotice(QDateTime::currentDateTime(),
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
    list<string>::iterator it;
    for (it = myUsers.begin(); it != myUsers.end(); it++)
    {
      if (id.compare(it->c_str(), Qt::CaseInsensitive) == 0)
      {
        myUsers.remove(*it);
        break;
      }
    }
    mleHistory->setOwner(myUsers.front().c_str());
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
    QTimer::singleShot(clearDelay, this, SLOT(slotClearNewEvents()));
  QWidget::windowActivationChange(oldActive);
}

void UserSendCommon::changeEventType(int type)
{
  if (myType == type)
    return;

  UserSendCommon* e = 0;
  QWidget* parent = 0;

  UserEventTabDlg* tabDlg = LicqGui::instance()->userEventTabDlg();
  if (tabDlg != NULL && tabDlg->tabExists(this))
    parent = tabDlg;

  switch (type)
  {
    case ET_MESSAGE:
      e = new UserSendMsgEvent(myUsers.front().c_str(), myPpid, parent);
      break;
    case ET_URL:
      e = new UserSendUrlEvent(myUsers.front().c_str(), myPpid, parent);
      break;
    case ET_CHAT:
      e = new UserSendChatEvent(myUsers.front().c_str(), myPpid, parent);
      break;
    case ET_FILE:
      e = new UserSendFileEvent(myUsers.front().c_str(), myPpid, parent);
      break;
    case ET_CONTACT:
      e = new UserSendContactEvent(myUsers.front().c_str(), myPpid, parent);
      break;
    case ET_SMS:
      e = new UserSendSmsEvent(myUsers.front().c_str(), myPpid, parent);
      break;
    default:
      assert(false);
  }

  if (e != 0)
  {
    if (e->mleSend != 0 && mleSend != 0)
    {
      e->mleSend->setText(mleSend->toPlainText());
      e->mleSend->document()->setModified(mleSend->document()->isModified());
    }
    if (e->mleHistory != 0 && mleHistory != 0)
    {
      e->mleHistory->setHtml(mleHistory->toHtml());
      e->mleHistory->GotoEnd();
    }

    if (parent == 0)
    {
      QPoint p = topLevelWidget()->pos();
      e->move(p);
    }

    LicqGui::instance()->replaceEventDialog(this, e, myUsers.front().c_str(), myPpid);

    emit msgTypeChanged(this, e);

    if (parent == 0)
    {
      QTimer::singleShot(10, e, SLOT(show()));
      QTimer::singleShot(100, this, SLOT(close()));
    }
    else
      LicqGui::instance()->userEventTabDlg()->replaceTab(this, e);
  }
}

void UserSendCommon::retrySend(ICQEvent* e, bool online, unsigned short level)
{
  unsigned long icqEventTag = 0;
  chkSendServer->setChecked(!online);
  chkUrgent->setChecked(level == ICQ_TCPxMSG_URGENT);

  switch (e->UserEvent()->SubCommand() & ~ICQ_CMDxSUB_FxMULTIREC)
  {
    case ICQ_CMDxSUB_MSG:
    {
      ICQUser* u = gUserManager.FetchUser(myUsers.front().c_str(), myPpid, LOCK_R);
      bool userOffline = true;
      if (u != 0)
      {
        userOffline = u->StatusOffline();
        gUserManager.DropUser(u);
      }
      CEventMsg* ue = dynamic_cast<CEventMsg*>(e->UserEvent());
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

        icqEventTag = gLicqDaemon->icqSendMessage(myUsers.front().c_str(), messageRaw.data(),
            online, level, false, &icqColor);

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
      CEventUrl* ue = dynamic_cast<CEventUrl*>(e->UserEvent());

      icqEventTag = gLicqDaemon->ProtoSendUrl(myUsers.front().c_str(), myPpid, ue->Url(),
          ue->Description(), online, level, false, &icqColor);

      break;
    }

    case ICQ_CMDxSUB_CONTACTxLIST:
    {
      CEventContactList* ue = dynamic_cast<CEventContactList*>(e->UserEvent());
      const ContactList& clist = ue->Contacts();
      UinList uins;

      for (ContactList::const_iterator i = clist.begin(); i != clist.end(); i++)
        uins.push_back((*i)->Uin());

      if (uins.size() == 0)
        break;

      icqEventTag = gLicqDaemon->icqSendContactList(strtoul(myUsers.front().c_str(), NULL, 10),
          uins, online, level, false, &icqColor);

      break;
    }

    case ICQ_CMDxSUB_CHAT:
    {
      CEventChat* ue = dynamic_cast<CEventChat*>(e->UserEvent());

      if (ue->Clients() == NULL)
        //TODO in the daemon
        icqEventTag = gLicqDaemon->icqChatRequest(strtoul(myUsers.front().c_str(), NULL, 10),
            ue->Reason(), level, !online);
      else
        //TODO in the daemon
        icqEventTag = gLicqDaemon->icqMultiPartyChatRequest(strtoul(myUsers.front().c_str(), NULL, 10),
            ue->Reason(), ue->Clients(), ue->Port(), level, !online);

      break;
    }

    case ICQ_CMDxSUB_FILE:
    {
      CEventFile* ue = dynamic_cast<CEventFile*>(e->UserEvent());
      ConstFileList filelist(ue->FileList());

      //TODO in the daemon
      icqEventTag = gLicqDaemon->icqFileTransfer(strtoul(myUsers.front().c_str(), NULL, 10),
          ue->Filename(), ue->FileDescription(), filelist, level, !online);

      break;
    }

    case ICQ_CMDxSUB_SMS:
    {
      CEventSms* ue = dynamic_cast<CEventSms*>(e->UserEvent());

      //TODO in the daemon
      icqEventTag = gLicqDaemon->icqSendSms(ue->Number(), ue->Message(),
          strtoul(myUsers.front().c_str(), NULL, 0));

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

void UserSendCommon::userUpdated(CICQSignal* sig, QString id, unsigned long ppid)
{
  ICQUser* u = gUserManager.FetchUser(id.toLatin1(), ppid, LOCK_R);

  if (u == NULL)
    return;

  switch (sig->SubSignal())
  {
    case USER_STATUS:
    {
      if (u->Port() == 0)
      {
        chkSendServer->setChecked(true);
        chkSendServer->setEnabled(false);
      }
      else
        chkSendServer->setEnabled(true);

      if (u->StatusOffline())
        chkSendServer->setChecked(true);

      break;
    }

    case USER_EVENTS:
    {
      CUserEvent* e = u->EventPeekId(sig->Argument());

      if (e != NULL && myHighestEventId < sig->Argument() &&
          mleHistory && sig->Argument() > 0)
      {
        myHighestEventId = sig->Argument();
        e = u->EventPeekId(sig->Argument());

        if (e != NULL)
          if (sig->PPID() != MSN_PPID || (sig->PPID() == MSN_PPID && sig->CID() == myConvoId))
          {
            gUserManager.DropUser(u);
            mleHistory->addMsg(e, id, ppid);
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
        chkSendServer->setChecked(false);
      }
      break;
  }

  gUserManager.DropUser(u);
}

bool UserSendCommon::checkSecure()
{
  ICQUser* u = gUserManager.FetchUser(myUsers.front().c_str(), myPpid, LOCK_R);

  if (u == NULL)
    return false;

  bool secure = u->Secure() || u->AutoSecure();
  bool send_ok = true;

  gUserManager.DropUser(u);

  if (chkSendServer->isChecked() && secure)
  {
    if (!QueryYesNo(this, tr("Message can't be sent securely through the server!\n"
            "Send anyway?")))
      send_ok = false;
    else
    {
      ICQUser* u = gUserManager.FetchUser(myUsers.front().c_str(), myPpid, LOCK_R);
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
    ICQUser* u = gUserManager.FetchUser(myUsers.front().c_str(), myPpid, LOCK_W);

    if (u != NULL)
    {
      if (u->NewUser())
      {
        u->SetNewUser(false);
        gUserManager.DropUser(u);
        CICQSignal s(SIGNAL_UPDATExUSER, USER_BASIC, myUsers.front().c_str(), myPpid);
        emit updateUser(&s);
      }
      else
        gUserManager.DropUser(u);
    }
  }

  unsigned long icqEventTag = 0;

  if (myEventTag.size() != 0)
    icqEventTag = myEventTag.front();

  if (icqEventTag != 0 || myPpid != LICQ_PPID)
  {
    bool via_server = chkSendServer->isChecked();
    myProgressMsg = tr("Sending ");
    myProgressMsg += via_server ? tr("via server") : tr("direct");
    myProgressMsg += "...";
    QString title = myBaseTitle + " [" + myProgressMsg + "]";

    UserEventTabDlg* tabDlg = LicqGui::instance()->userEventTabDlg();
    if (tabDlg != NULL && tabDlg->tabIsSelected(this))
      tabDlg->setWindowTitle(title);

    setWindowTitle(title);
    setCursor(Qt::WaitCursor);
    btnSend->setText(tr("&Cancel"));
    btnClose->setEnabled(false);

    if (mleSend != NULL)
      mleSend->setEnabled(false);

    disconnect(btnSend, SIGNAL(clicked()), this, SLOT(send()));
    connect(btnSend, SIGNAL(clicked()), SLOT(slotCancelSend()));

    connect(LicqGui::instance()->signalManager(),
        SIGNAL(doneUserFcn(ICQEvent*)), SLOT(slotSendDone(ICQEvent*)));
  }
}

void UserSendCommon::slotSendDone(ICQEvent* e)
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
  std::list<unsigned long>::iterator iter;

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
      QTimer::singleShot(5000, this, SLOT(slotResetTitle()));
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
  btnSend->setText(tr("&Send"));
  btnClose->setEnabled(true);

  if (mleSend != NULL)
    mleSend->setEnabled(true);

  disconnect(btnSend, SIGNAL(clicked()), this, SLOT(slotCancelSend()));
  connect(btnSend, SIGNAL(clicked()), SLOT(send()));

  // If cancelled automatically, check "Send through Server"
  if (Config::Chat::instance()->autoSendThroughServer() && e->Result() == EVENT_CANCELLED)
    chkSendServer->setChecked(true);

  if (myEventTag.size() == 0)
    disconnect(LicqGui::instance()->signalManager(),
        SIGNAL(doneUserFcn(ICQEvent*)), this, SLOT(slotSendDone(ICQEvent*)));

  if (mleSend != NULL)
    mleSend->setFocus();

  if (e->Result() != EVENT_ACKED)
  {
    if (e->Command() == ICQ_CMDxTCP_START && e->Result() != EVENT_CANCELLED &&
       (Config::Chat::instance()->autoSendThroughServer() ||
         QueryYesNo(this, tr("Direct send failed,\nsend through server?"))) )
      retrySend(e, false, ICQ_TCPxMSG_NORMAL);
    return;
  }

  ICQUser* u = NULL;
  QString msg;

  if (e->SubResult() == ICQ_TCPxACK_RETURN)
  {
    u = gUserManager.FetchUser(myUsers.front().c_str(), myPpid, LOCK_W);

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
  else
  {
    emit autoCloseNotify();
    if (sendDone(e))
    {
      emit gMainWindow->signal_sentevent(e);
      if (Config::Chat::instance()->msgChatView() && mleHistory != NULL)
      {
        mleHistory->GotoEnd();
        resetSettings();
      }
      else
        close();
    }
  }
}

void UserSendCommon::slotCancelSend()
{
  unsigned long icqEventTag = 0;

  if (myEventTag.size())
    icqEventTag = myEventTag.front();

  if (icqEventTag == 0)
    return slotClose(); // if we're not sending atm, let ESC close the window

  UserEventTabDlg* tabDlg = LicqGui::instance()->userEventTabDlg();
  if (tabDlg != NULL && tabDlg->tabIsSelected(this))
    tabDlg->setWindowTitle(myBaseTitle);

  gLicqDaemon->CancelEvent(icqEventTag);
}

void UserSendCommon::slotChangeEventType(QAction* action)
{
  changeEventType(action->data().toInt());
}

void UserSendCommon::slotClearNewEvents()
{
  ICQUser* u = NULL;

  // Iterate all users in the conversation
  for (list<string>::iterator it = myUsers.begin(); it != myUsers.end(); ++it)
  {
    u = gUserManager.FetchUser((*it).c_str(), myPpid, LOCK_W);
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
            CUserEvent* e = u->EventPeek(i);
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

void UserSendCommon::slotClose()
{
  gLicqDaemon->ProtoTypingNotification(myUsers.front().c_str(), myPpid, false, myConvoId);

  if (Config::Chat::instance()->msgChatView())
  {
    // the window is at the front, if the timer has not expired and we close
    // the window, then the new events will stay there
    slotClearNewEvents();
  }

  if (mleSend)
    Config::Chat::instance()->setCheckSpelling(mleSend->checkSpellingEnabled());

  UserEventTabDlg* tabDlg = LicqGui::instance()->userEventTabDlg();
  if (tabDlg != NULL && tabDlg->tabExists(this))
    tabDlg->slotRemoveTab(this);
  else
    close();
}

void UserSendCommon::slotEmoticon()
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

  connect(p, SIGNAL(selected(const QString&)), SLOT(slotInsertEmoticon(const QString&)));
  p->move(pos);
  p->show();
}

void UserSendCommon::slotInsertEmoticon(const QString& value)
{
  if (mleSend)
    mleSend->insertPlainText(value);
}

/*! This slot creates/removes a little widget into the usereventdlg
 *  which enables the user to collect users for mass messaging.
 */
void UserSendCommon::slotMassMessageToggled(bool b)
{
  if (grpMR == NULL)
  {
    grpMR = new QGroupBox();
    top_hlay->addWidget(grpMR);
    QVBoxLayout* layMR = new QVBoxLayout(grpMR);

    layMR->addWidget(new QLabel(tr("Drag Users Here\nRight Click for Options")));

    lstMultipleRecipients = new MMUserView(myUsers.front().c_str(), myPpid,
        LicqGui::instance()->contactList());
    lstMultipleRecipients->setFixedWidth(gMainWindow->getUserView()->width());
    layMR->addWidget(lstMultipleRecipients);
  }

  chkMass->setChecked(b);
  grpMR->setVisible(b);
}

void UserSendCommon::slotMessageAdded()
{
  UserEventTabDlg* tabDlg = LicqGui::instance()->userEventTabDlg();
  if (isActiveWindow() &&
      (!Config::Chat::instance()->tabbedChatting() ||
       (tabDlg != NULL && tabDlg->tabIsSelected(this))))
    QTimer::singleShot(clearDelay, this, SLOT(slotClearNewEvents()));
}

void UserSendCommon::slotResetTitle()
{
  UserEventTabDlg* tabDlg = LicqGui::instance()->userEventTabDlg();
  if (tabDlg != NULL && tabDlg->tabIsSelected(this))
    tabDlg->setWindowTitle(myBaseTitle);

  setWindowTitle(myBaseTitle);
}

void UserSendCommon::slotSendServerToggled(bool sendServer)
{
  // When the "Send through server" checkbox is toggled by the user,
  // we save the setting to disk, so it is persistent.

  ICQUser* u = gUserManager.FetchUser(myId.toLatin1(), myPpid, LOCK_W);
  if (u != NULL)
  {
    u->SetSendServer(sendServer);
    gUserManager.DropUser(u);
  }
}

void UserSendCommon::slotSetBackgroundICQColor()
{
  if (mleSend == NULL)
    return;

#ifdef USE_KDE
  QColor c = mleSend->palette().color(mleSend->backgroundRole());
  if (KColorDialog::getColor(c, this) != KColorDialog::Accepted)
    return;
#else
  QColor c = QColorDialog::getColor(mleSend->palette().color(mleSend->backgroundRole()), this);
  if (!c.isValid())
    return;
#endif

  icqColor.SetBackground(c.red(), c.green(), c.blue());
  mleSend->setBackground(c);
}

void UserSendCommon::slotSetForegroundICQColor()
{
  if (mleSend == NULL)
    return;

#ifdef USE_KDE
  QColor c = mleSend->palette().color(mleSend->foregroundRole());
  if (KColorDialog::getColor(c, this) != KColorDialog::Accepted)
    return;
#else
  QColor c = QColorDialog::getColor(mleSend->palette().color(mleSend->foregroundRole()), this);
  if (!c.isValid())
    return;
#endif

  icqColor.SetForeground(c.red(), c.green(), c.blue());
  mleSend->setForeground(c);
}

void UserSendCommon::slotShowSendTypeMenu()
{
  // Menu is normally delayed but if we use InstantPopup mode shortcut won't work
  dynamic_cast<QToolButton*>(myToolBar->widgetForAction(cmbSendType))->showMenu();
}

void UserSendCommon::messageTextChanged()
{
  if (mleSend == NULL || mleSend->toPlainText().isEmpty())
    return;

  strTempMsg = mleSend->toPlainText();
  gLicqDaemon->ProtoTypingNotification(myUsers.front().c_str(), myPpid, true, myConvoId);
  disconnect(mleSend, SIGNAL(textChanged()), this, SLOT(messageTextChanged()));
  tmrSendTyping->start(5000);
}

void UserSendCommon::slotTextChangedTimeout()
{
  if (mleSend == NULL)
  {
    tmrSendTyping->stop();
    return;
  }

  QString str = mleSend->toPlainText();

  if (str != strTempMsg)
  {
    strTempMsg = str;
    // Hack to not keep sending the typing notification to ICQ
    if (myPpid != LICQ_PPID)
      gLicqDaemon->ProtoTypingNotification(myUsers.front().c_str(), myPpid, true, myConvoId);
  }
  else
  {
    if (tmrSendTyping->isActive())
      tmrSendTyping->stop();
    connect(mleSend, SIGNAL(textChanged()), SLOT(messageTextChanged()));
    gLicqDaemon->ProtoTypingNotification(myUsers.front().c_str(), myPpid, false, myConvoId);
  }
}

void UserSendCommon::sendTrySecure()
{
  ICQUser* u = gUserManager.FetchUser(myUsers.front().c_str(), myPpid, LOCK_R);

  bool autoSecure = false;
  if (u != NULL)
  {
    autoSecure = (u->AutoSecure() && gLicqDaemon->CryptoEnabled() &&
        u->SecureChannelSupport() == SECURE_CHANNEL_SUPPORTED &&
        !chkSendServer->isChecked() && !u->Secure());
    gUserManager.DropUser(u);
  }

  disconnect(btnSend, SIGNAL(clicked()), this, SLOT(sendTrySecure()));
  connect(btnSend, SIGNAL(clicked()), SLOT(send()));

  if (autoSecure)
  {
    QWidget* w = new KeyRequestDlg(QString(myUsers.front().c_str()), myPpid);
    connect(w, SIGNAL(destroyed()), SLOT(send()));
  }
  else
    send();
}
