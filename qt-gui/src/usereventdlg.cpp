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

// written by Graham Roff <graham@licq.org>
// contributions by Dirk A. Mueller <dirk@licq.org>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <algorithm>
#include <utility>
#include <cmath>

#include <assert.h>
#include <qaccel.h>
#include <qcheckbox.h>
#include <qdatetime.h>
#include <qevent.h>
#include <qfileinfo.h>
#include <qvbox.h>
#include <qvgroupbox.h>
#include <qhgroupbox.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmap.h>
#include <qpushbutton.h>
#include <qsplitter.h>
#include <qapplication.h>
#include <qpopupmenu.h>
#include <qtextcodec.h>
#include <qwhatsthis.h>
#include <qtabwidget.h>
#include <qsize.h>
#include <ctype.h>

#ifdef USE_KDE
#include <kapp.h>
#include <kfiledialog.h>
#include <kcolordialog.h>
#if KDE_IS_VERSION(3, 2, 0)
#include <kwin.h>
#endif // KDE_IS_VERSION
#else
#include <qfiledialog.h>
#include <qcolordialog.h>
#endif

#include "licq_message.h"
#include "licq_translate.h"
#include "licq_icqd.h"
#include "licq_log.h"
#include "usercodec.h"

#include "authuserdlg.h"
#include "emoticon.h"
#include "ewidgets.h"
#include "gui-defines.h"
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
#include "keyrequestdlg.h"
#include "editfilelistdlg.h"
#include "usercodec.h"
#include "xpm/chatChangeFg.xpm"
#include "xpm/chatChangeBg.xpm"
#include "xpm/smile.xpm"
#include "support.h"
#include "mledit.h"
#include "gui-strings.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

using namespace std;

const size_t SHOW_RECENT_NUM = 5;

typedef pair<CUserEvent *, char *> MessageIter;

bool OrderMessages(const MessageIter& m1, const MessageIter& m2)
{
  return (m1.first->Time() < m2.first->Time());
}

// -----------------------------------------------------------------------------
UserEventCommon::UserEventCommon(CICQDaemon *s, CSignalManager *theSigMan,
                                 CMainWindow *m, const char *_szId,
                                 unsigned long _nPPID, QWidget *parent,
                                 const char *name)
  : QWidget(parent, name, WDestructiveClose), m_highestEventId(-1)
{
  server = s;
  mainwin = m;
  sigman = theSigMan;
  if (_szId)
  {
    ICQUser::MakeRealId(_szId, _nPPID, m_szId);
    m_lUsers.push_back(m_szId);
  }
  else
    m_szId = 0;
  m_nPPID = _nPPID;
  m_bOwner = (gUserManager.FindOwner(m_lUsers.front().c_str(), m_nPPID) != NULL);
  m_bDeleteUser = false;
  m_nConvoId = 0;
    
  top_hlay = new QHBoxLayout(this, 6);
  top_lay = new QVBoxLayout(top_hlay);
  top_hlay->setStretchFactor(top_lay, 1);

  // initalize codec
  codec = QTextCodec::codecForLocale();

  QBoxLayout *layt = new QHBoxLayout(top_lay, 8);
  layt->addWidget(new QLabel(tr("Status:"), this));
  nfoStatus = new CInfoField(this, true);
  nfoStatus->setMinimumWidth(nfoStatus->sizeHint().width()+30);
  layt->addWidget(nfoStatus);
  layt->addWidget(new QLabel(tr("Time:"), this));
  nfoTimezone = new CInfoField(this, true);
  nfoTimezone->setMinimumWidth(nfoTimezone->sizeHint().width()/2+10);
  layt->addWidget(nfoTimezone);

  popupEncoding = new QPopupMenu(this);
  btnSecure = new QPushButton(this);
  QToolTip::add(btnSecure, tr("Open / Close secure channel"));
  layt->addWidget(btnSecure);
  connect(btnSecure, SIGNAL(clicked()), this, SLOT(slot_security()));
  btnHistory = new QPushButton(this);
  btnHistory->setPixmap(mainwin->pmHistory);
  QToolTip::add(btnHistory, tr("Show User History"));
  connect(btnHistory, SIGNAL(clicked()), this, SLOT(showHistory()));
  layt->addWidget(btnHistory);
  btnInfo = new QPushButton(this);
  btnInfo->setPixmap(mainwin->pmInfo);
  QToolTip::add(btnInfo, tr("Show User Info"));
  connect(btnInfo, SIGNAL(clicked()), this, SLOT(showUserInfo()));
  layt->addWidget(btnInfo);
  btnEncoding = new QPushButton(this);
  btnEncoding->setPixmap(mainwin->pmEncoding);
  QToolTip::add(btnEncoding, tr("Change user text encoding"));
  QWhatsThis::add(btnEncoding, tr("This button selects the text encoding used when communicating with this user. You might need to change the encoding to communicate in a different language."));
  btnEncoding->setPopup(popupEncoding);

  layt->addWidget(btnEncoding);

  tmrTime = NULL;
  tmrTyping = NULL;

  ICQUser *u = gUserManager.FetchUser(m_lUsers.front().c_str(), m_nPPID, LOCK_W);
  if (u != NULL)
  {
    nfoStatus->setData(Strings::getStatus(u));
    if (u->NewMessages() == 0)
      setIcon(CMainWindow::iconForStatus(u->StatusFull(), u->IdString(), u->PPID()));
    else
      setIcon(CMainWindow::iconForEvent(ICQ_CMDxSUB_MSG));

    if (mainwin->m_bFlashTaskbar)
      FlashTaskbar(u->NewMessages() != 0);

    SetGeneralInfo(u);

    // restore prefered encoding
    codec = UserCodec::codecForICQUser(u);

    gotTyping(u->GetTyping());
    gUserManager.DropUser(u);
  }

  QString codec_name = QString::fromLatin1( codec->name() ).lower();
  popupEncoding->setCheckable(true);

  // populate the popup menu
  UserCodec::encoding_t *it = &UserCodec::m_encodings[0];
  while(it->encoding != NULL) {
  // Use check_codec since the QTextCodec name will be different from the
  // user codec. But QTextCodec will recognize both, so let's make it standard
  // for the purpose of checking for the same string.
  QTextCodec *check_codec = QTextCodec::codecForName(it->encoding);
    if (QString::fromLatin1(check_codec->name()).lower() == codec_name) {
      if (mainwin->m_bShowAllEncodings || it->isMinimal) {
        popupEncoding->insertItem(UserCodec::nameForEncoding(it->encoding), this, SLOT(slot_setEncoding(int)), 0, it->mib);
      } else {
        // if the current encoding does not appear in the minimal list
        popupEncoding->insertSeparator(0);
        popupEncoding->insertItem(UserCodec::nameForEncoding(it->encoding), this, SLOT(slot_setEncoding(int)), 0, it->mib, 0);
      }
      popupEncoding->setItemChecked(it->mib, true);
    } else {
      if (mainwin->m_bShowAllEncodings || it->isMinimal) {
        popupEncoding->insertItem(UserCodec::nameForEncoding(it->encoding), this, SLOT(slot_setEncoding(int)), 0, it->mib);
      }
    }

    ++it;
  }

  /* We might be called from a slot so connect the signal only after all the
     existing signals are handled */
  QTimer::singleShot(0, this, SLOT(slot_connectsignal()));

  mainWidget = new QWidget(this);
  top_lay->addWidget(mainWidget);

  // Check if we want the window sticky
  if (!m->m_bTabbedChatting && m->m_bMsgWinSticky)
  {
    QTimer *timer = new QTimer( this );
    connect( timer, SIGNAL(timeout()), this, SLOT(setMsgWinSticky()) );
    timer->start(100, true); // 100 milliseconds single-shot timer
  }
}

void UserEventCommon::slot_connectsignal()
{
  connect (sigman, SIGNAL(signal_updatedUser(CICQSignal *)),
           this, SLOT(slot_userupdated(CICQSignal *)));
}

UserEventTabDlg::UserEventTabDlg(CMainWindow *mainwin, QWidget *parent, const char *name)
  : QWidget(parent, name, WDestructiveClose)
{
  this->mainwin = mainwin;
  QBoxLayout *lay = new QVBoxLayout(this);
  tabw = new CETabWidget(this);
  lay->addWidget(tabw);
  connect(tabw, SIGNAL(currentChanged(QWidget *)),
          this, SLOT(slot_currentChanged(QWidget *)));
  connect(tabw, SIGNAL(middleClick(QWidget*)), this, SLOT(removeTab(QWidget*)));
}

UserEventTabDlg::~UserEventTabDlg()
{
  emit signal_done();
}

void UserEventTabDlg::addTab(UserEventCommon *tab, int index)
{
  QString label;
  ICQUser *u = gUserManager.FetchUser(tab->Id(), tab->PPID(), LOCK_W);
  if (u == NULL) return;

  // initalize codec
  label = QString::fromUtf8(u->GetAlias());
  tabw->insertTab(tab, label, index);
  updateTabLabel(u);
  gUserManager.DropUser(u);
  tabw->showPage(tab);
}

void UserEventTabDlg::removeTab(QWidget *tab)
{
  if (tabw->count() > 1)
  {
    tabw->removePage(tab);
    tab->close();
    tab->setEnabled(false);
    tab->deleteLater();
  }
  else
    close();
}

void UserEventTabDlg::selectTab(QWidget *tab)
{
  tabw->showPage(tab);
  updateTitle(tab);
}

void UserEventTabDlg::replaceTab(QWidget *oldTab,
				 UserEventCommon *newTab)
{
  addTab(newTab, tabw->indexOf(oldTab));
  removeTab(oldTab);
}

bool UserEventTabDlg::tabIsSelected(QWidget *tab)
{
  if (tabw->currentPageIndex() == tabw->indexOf(tab))
    return true;
  else
    return false;
}

bool UserEventTabDlg::tabExists(QWidget *tab)
{
  if (tabw->indexOf(tab) != -1)
    return true;
  else return false;
}

void UserEventTabDlg::updateConvoLabel(UserEventCommon *tab)
{
  // Show the list of users in the conversation
  list<string> lUsers = tab->ConvoUsers();
  list<string>::iterator it;
  string newLabel = "";
  for (it = lUsers.begin(); it != lUsers.end(); ++it)
  {
    ICQUser *u = gUserManager.FetchUser((*it).c_str(), tab->PPID(), LOCK_R);
    if (newLabel.size() == 0)
      newLabel = u->GetAlias();
    else
    {
      newLabel += ", ";
      newLabel += u->GetAlias();
    }
    gUserManager.DropUser(u);
  }
  tabw->changeTab(tab, QString::fromUtf8(newLabel.c_str()));
}

void UserEventTabDlg::updateTabLabel(ICQUser *u)
{
  for (int index = 0; index < tabw->count(); index++)
  {
    UserEventCommon *tab = static_cast<UserEventCommon*>(tabw->page(index));
      
    if (tab->FindUserInConvo(u->IdString()) && tab->PPID() == u->PPID())
    {        
      if (u->NewMessages() > 0)
      {        
        if (tabw->currentPageIndex() == index)
          setIcon(CMainWindow::iconForEvent(ICQ_CMDxSUB_MSG));
	//if (mainwin->m_bFlashTaskbar)
          //flashTaskbar(true);

        // to clear it..
        tab->gotTyping(u->GetTyping());

        // use an event icon
        unsigned short SubCommand = 0;
        for (unsigned short i = 0; i < u->NewMessages(); i++)
        {
          switch(u->EventPeek(i)->SubCommand())
          {
          case ICQ_CMDxSUB_FILE:
            SubCommand = ICQ_CMDxSUB_FILE;
            break;
          case ICQ_CMDxSUB_CHAT:
            if (SubCommand != ICQ_CMDxSUB_FILE) SubCommand = ICQ_CMDxSUB_CHAT;
            break;
          case ICQ_CMDxSUB_URL:
            if (SubCommand != ICQ_CMDxSUB_FILE && SubCommand != ICQ_CMDxSUB_CHAT)
              SubCommand = ICQ_CMDxSUB_URL;
            break;
          case ICQ_CMDxSUB_CONTACTxLIST:
            if(SubCommand != ICQ_CMDxSUB_FILE && SubCommand != ICQ_CMDxSUB_CHAT
              && SubCommand != ICQ_CMDxSUB_URL)
              SubCommand = ICQ_CMDxSUB_CONTACTxLIST;
          case ICQ_CMDxSUB_MSG:
          default:
            if (SubCommand == 0) SubCommand = ICQ_CMDxSUB_MSG;
            break;
          }
        }
        if(SubCommand)
          tabw->setTabIconSet(tab, CMainWindow::iconForEvent(SubCommand));
        tabw->setTabColor(tab, QColor("blue"));
      }
      // use status icon
      else
      {
        if (tabw->currentPageIndex() == index)
          setIcon(CMainWindow::iconForStatus(u->StatusFull(), u->IdString(), u->PPID()));
        flashTaskbar(false);

        tabw->setTabIconSet(tab, CMainWindow::iconForStatus(u->StatusFull(), u->IdString(), u->PPID()));
        if (u->GetTyping() == ICQ_TYPING_ACTIVE)
          tabw->setTabColor(tab, mainwin->m_colorTabTyping);
        else
          tabw->setTabColor(tab, QColor("black"));
      }
      return;
    }
  }
}

void UserEventTabDlg::gotTyping(ICQUser *u, int nConvoId)
{
  for (int index = 0; index < tabw->count(); index++)
  {
    UserEventCommon *tab = static_cast<UserEventCommon*>(tabw->page(index));
    if ( ((u->PPID() == MSN_PPID && tab->PPID() == u->PPID()) &&
          tab->FindUserInConvo(u->IdString()) && tab->ConvoId() == (unsigned long)(nConvoId)) ||
         (tab->FindUserInConvo(u->IdString()) && tab->PPID() == u->PPID()))
    {
      tab->gotTyping(u->GetTyping());
    }
  }
}

/*! This slot should get called when the current tab has 
 *  changed.
 */
void UserEventTabDlg::slot_currentChanged(QWidget *tab)
{
  tab->setFocus();  // prevent users from accidently typing in the wrong widget
  updateTitle(tab);
  clearEvents(tab);
}

void UserEventTabDlg::updateTitle(QWidget *tab)
{
  if (tab->caption())
    setCaption(tab->caption());

  if (!tabw->tabIconSet(tab).isNull() &&
      !tabw->tabIconSet(tab).pixmap().isNull())
    setIcon(tabw->tabIconSet(tab).pixmap());
}

void UserEventTabDlg::clearEvents(QWidget *tab)
{
  if (!isActiveWindow()) return;
  UserSendCommon *e = static_cast<UserSendCommon*>(tab);
  QTimer::singleShot(e->clearDelay, e, SLOT(slot_ClearNewEvents()));
}

void UserEventTabDlg::flashTaskbar(bool _bFlash)
{
  Display *dsp = x11Display();  // get the display
  WId win = winId(); // get the window
  XWMHints *hints; // hints
  hints = XGetWMHints(dsp, win); // init hints
  if (_bFlash)
    hints->flags |= XUrgencyHint;
  else
    hints->flags &= ~XUrgencyHint;
  XSetWMHints(dsp, win, hints); // set hints
  XFree(hints);
}

void UserEventTabDlg::moveLeft()
{
  tabw->setPreviousPage();
}

void UserEventTabDlg::moveRight()
{
  tabw->setNextPage();
}

#if defined(USE_KDE) && KDE_IS_VERSION(3, 2, 0)
/* KDE 3.2 handles app-icon updates differently, since KDE 3.2 a simple setIcon() call
   does no longer update the icon in kicker anymore :(
   So we do it the "kde-way" here */
void UserEventTabDlg::setIcon(const QPixmap &icon)
{
  KWin::setIcons(winId(), icon, icon);
}
#endif

void UserEventTabDlg::setMsgWinSticky()
{
  CSupport::changeWinSticky(winId(), true);
}

void UserEventTabDlg::changeMsgWinSticky(bool _bStick)
{
  CSupport::changeWinSticky(winId(), _bStick);
}

// -----------------------------------------------------------------------------

void UserEventCommon::slot_setEncoding(int encodingMib) {
  /* initialize a codec according to the encoding menu item id */
  QString encoding( UserCodec::encodingForMib(encodingMib) );

  if (!encoding.isNull()) {
    QTextCodec * _codec = QTextCodec::codecForName(encoding.latin1());
    if (_codec == NULL)
    {
      WarnUser(this, QString(tr("Unable to load encoding <b>%1</b>. Message contents may appear garbled.")).arg(encoding));
      return;
    }
    codec = _codec;

    /* uncheck all encodings */
    for (unsigned int i=0; i<popupEncoding->count(); ++i) {
      popupEncoding->setItemChecked(popupEncoding->idAt(i), false);
    }

    /* make the chosen encoding checked */
    popupEncoding->setItemChecked(encodingMib, true);

    /* save prefered character set */
    ICQUser *u = gUserManager.FetchUser(m_lUsers.front().c_str(), m_nPPID, LOCK_W);
    if (u != NULL) {
      u->SetEnableSave(false);
      u->SetUserEncoding(encoding.latin1());
      u->SetEnableSave(true);
      u->SaveLicqInfo();
      gUserManager.DropUser(u);
    }

    emit encodingChanged();
  }
}

//-----UserEventCommon::SetGeneralInfo---------------------------------------
void UserEventCommon::SetGeneralInfo(ICQUser *u)
{
  QTextCodec * codec = UserCodec::codecForICQUser(u);

  if (u->GetTimezone() == TIMEZONE_UNKNOWN)
    nfoTimezone->setText(tr("Unknown"));
  else
  {
    m_nRemoteTimeOffset = u->LocalTimeOffset();
    QDateTime t;
    t.setTime_t(u->LocalTime());
    nfoTimezone->setText(t.time().toString());
    /*nfoTimezone->setText(tr("%1 (GMT%1%1%1)")
                         .arg(t.time().toString())
                         .arg(u->GetTimezone() > 0 ? "-" : "+")
                         .arg(abs(u->GetTimezone() / 2))
                         .arg(u->GetTimezone() % 2 ? "30" : "00") );*/
    if (tmrTime == NULL)
    {
      tmrTime = new QTimer(this);
      connect(tmrTime, SIGNAL(timeout()), this, SLOT(slot_updatetime()));
      tmrTime->start(3000);
    }
  }

  if (tmrTyping == NULL)
  {
    tmrTyping = new QTimer(this);
    connect(tmrTyping, SIGNAL(timeout()), this, SLOT(slot_updatetyping()));
  }

  if (u->Secure())
    btnSecure->setPixmap(mainwin->pmSecureOn);
  else
    btnSecure->setPixmap(mainwin->pmSecureOff);

  QString tmp = codec->toUnicode(u->GetFirstName());
  QString lastname = codec->toUnicode(u->GetLastName());
  if ((!tmp.isEmpty()) && (!lastname.isEmpty()))
    tmp = tmp + " " + lastname;
  else
    tmp = tmp + lastname;
  if (!tmp.isEmpty()) tmp = " (" + tmp + ")";
  m_sBaseTitle = QString::fromUtf8(u->GetAlias()) + tmp;

  if (mainwin->userEventTabDlg &&
      mainwin->userEventTabDlg->tabIsSelected(this))
    mainwin->userEventTabDlg->setCaption(m_sBaseTitle);

  setCaption(m_sBaseTitle);
  setIconText(u->GetAlias());
}

void UserEventCommon::FlashTaskbar(bool _bFlash)
{
  Display *dsp = x11Display();  // get the display
  WId win = winId(); // get the window
  XWMHints *hints; // hints
  hints = XGetWMHints(dsp, win); // init hints
  if (_bFlash)
    hints->flags |= XUrgencyHint;
  else
    hints->flags &= ~XUrgencyHint;
  XSetWMHints(dsp, win, hints); // set hints
  XFree(hints);
}

void UserEventCommon::setMsgWinSticky()
{
  CSupport::changeWinSticky(winId(), true);
}

void UserEventCommon::changeMsgWinSticky(bool _bStick)
{
  CSupport::changeWinSticky(winId(), _bStick);
}

void UserEventCommon::slot_updatetime()
{
  QDateTime t;
  t.setTime_t(time(NULL) + m_nRemoteTimeOffset);
  nfoTimezone->setText(t.time().toString());
  //nfoTimezone->setText(nfoTimezone->text().replace(0, t.time().toString().length(), t.time().toString()));
}


UserEventCommon::~UserEventCommon()
{
  emit finished(m_lUsers.front().c_str(), m_nPPID);

  if (m_bDeleteUser && !m_bOwner)
    mainwin->RemoveUserFromList(strdup(m_lUsers.front().c_str()), m_nPPID, this);

  if (m_szId)
    delete [] m_szId;
  m_lUsers.clear();
}

bool UserEventCommon::FindUserInConvo(const char *szId)
{
  char *szRealId;
  ICQUser::MakeRealId(szId, m_nPPID, szRealId);
  bool bFound = (std::find(m_lUsers.begin(), m_lUsers.end(), szRealId) != m_lUsers.end());
  delete [] szRealId;
  return bFound;
}

void UserEventCommon::gotTyping(unsigned short nTyping)
{
  if (nTyping == ICQ_TYPING_ACTIVE)
  {
    if (tmrTyping->isActive())
      tmrTyping->stop();
    tmrTyping->start(10000, true);
    
    nfoStatus->setPaletteBackgroundColor(QColor("yellow"));
  }
  else
    nfoStatus->unsetPalette();
}

void UserEventCommon::slot_updatetyping()
{
  // MSN needs this, ICQ/AIM don't send additional packets
  // This does need to be verified with the official AIM client, there is a
  // packet for it, but ICQ isn't using it apparently.
  if (m_nPPID == LICQ_PPID || m_lUsers.empty()) return;

  //FIXME Which user?
  ICQUser *u = gUserManager.FetchUser(m_lUsers.front().c_str(), m_nPPID, LOCK_W);
  u->SetTyping(ICQ_TYPING_INACTIVEx0);
  nfoStatus->unsetPalette();
  if (mainwin->m_bTabbedChatting && mainwin->userEventTabDlg)
    mainwin->userEventTabDlg->updateTabLabel(u);
  gUserManager.DropUser(u);  
}

//-----UserEventCommon::slot_userupdated-------------------------------------
void UserEventCommon::slot_userupdated(CICQSignal *sig)
{
  if (m_nPPID != sig->PPID() || !FindUserInConvo(sig->Id()))
       //std::find(m_lUsers.begin(), m_lUsers.end(), sig->Id()) == m_lUsers.end())
  {
    if (sig->CID() == m_nConvoId && m_nConvoId != 0)
    { 
      char *szRealId;
      ICQUser::MakeRealId(sig->Id(), sig->PPID(), szRealId);
      m_lUsers.push_back(szRealId);
      delete [] szRealId;

      // Now update the tab label
      if (mainwin->userEventTabDlg)
        mainwin->userEventTabDlg->updateConvoLabel(this);
    }
    else
      return;
  }

  ICQUser *u = gUserManager.FetchUser(sig->Id(), m_nPPID, LOCK_R);
  if (u == NULL) return;

  switch (sig->SubSignal())
  {
    case USER_STATUS:
    {
      nfoStatus->setData(Strings::getStatus(u));
      if (u->NewMessages() == 0)
      {
        setIcon(CMainWindow::iconForStatus(u->StatusFull(), u->IdString(), u->PPID()));
        if (mainwin->m_bFlashTaskbar)
          FlashTaskbar(false);
      }
      break;
    }
    case USER_GENERAL:
    case USER_SECURITY:
    case USER_BASIC:
    {
      SetGeneralInfo(u);
      break;
    }
    case USER_EVENTS:
    {
      if (u->NewMessages() == 0)
        setIcon(CMainWindow::iconForStatus(u->StatusFull(), u->IdString(), u->PPID()));
      else
        setIcon(CMainWindow::iconForEvent(ICQ_CMDxSUB_MSG));

      if (mainwin->m_bFlashTaskbar)
        FlashTaskbar(u->NewMessages() != 0);
      break;
    }
  }
    gUserManager.DropUser(u);
    
  // Call the event specific function now
  UserUpdated(sig, sig->Id(), m_nPPID);
}


void UserEventCommon::showHistory()
{
  mainwin->callInfoTab(mnuUserHistory, m_lUsers.front().c_str(), m_nPPID, true);
}


void UserEventCommon::showUserInfo()
{
  mainwin->callInfoTab(mnuUserGeneral, m_lUsers.front().c_str(), m_nPPID, true);
}

void UserEventCommon::slot_security()
{
  (void) new KeyRequestDlg(sigman, m_lUsers.front().c_str(), m_nPPID);
}


//=====UserViewEvent=========================================================
UserViewEvent::UserViewEvent(CICQDaemon *s, CSignalManager *theSigMan,
                             CMainWindow *m, const char *_szId,
                             unsigned long _nPPID, QWidget* parent)
  : UserEventCommon(s, theSigMan, m, _szId, _nPPID, parent, "UserViewEvent")
{
  QBoxLayout* lay = new QVBoxLayout(mainWidget);
  splRead = new QSplitter(Vertical, mainWidget);
  lay->addWidget(splRead);
  splRead->setOpaqueResize();

  QAccel *a = new QAccel( this );
  a->connectItem(a->insertItem(Key_Escape), this, SLOT(close()));

  msgView = new MsgView(splRead);
  mlvRead = new MLView(splRead, "mlvRead");

  connect(mlvRead, SIGNAL(viewurl(QWidget*, QString)), mainwin, SLOT(slot_viewurl(QWidget *, QString)));

  splRead->setResizeMode(msgView, QSplitter::FollowSizeHint);
  splRead->setResizeMode(mlvRead, QSplitter::Stretch);

  connect (msgView, SIGNAL(currentChanged(QListViewItem *)), this, SLOT(slot_printMessage(QListViewItem *)));
  connect (mainwin, SIGNAL(signal_sentevent(ICQEvent *)), this, SLOT(slot_sentevent(ICQEvent *)));

  QHGroupBox *h_action = new QHGroupBox(mainWidget);
  lay->addSpacing(10);
  lay->addWidget(h_action);
  btnRead1 = new CEButton(h_action);
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

  QBoxLayout *h_lay = new QHBoxLayout(top_lay, 4);
  if (!m_bOwner)
  {
    QPushButton *btnMenu = new QPushButton(tr("&Menu"), this);
    h_lay->addWidget(btnMenu);
    connect(btnMenu, SIGNAL(pressed()), this, SLOT(slot_usermenu()));
    btnMenu->setPopup(mainwin->UserMenu());
    chkAutoClose = new QCheckBox(tr("Aut&o Close"), this);
    chkAutoClose->setChecked(mainwin->m_bAutoClose);
    h_lay->addWidget(chkAutoClose);
  }
  h_lay->addStretch(1);
  int bw = 75;
  btnReadNext = new QPushButton(tr("Nex&t"), this);
  setTabOrder(btnRead4, btnReadNext);
  btnClose = new CEButton(tr("&Close"), this);
  QToolTip::add(btnClose, tr("Normal Click - Close Window\n<CTRL>+Click - also delete User"));
  setTabOrder(btnReadNext, btnClose);
  bw = QMAX(bw, btnReadNext->sizeHint().width());
  bw = QMAX(bw, btnClose->sizeHint().width());
  btnReadNext->setFixedWidth(bw);
  btnClose->setFixedWidth(bw);
  h_lay->addWidget(btnReadNext);
  btnReadNext->setEnabled(false);
  connect(btnReadNext, SIGNAL(clicked()), this, SLOT(slot_btnReadNext()));
  connect(btnClose, SIGNAL(clicked()), SLOT(slot_close()));
  h_lay->addWidget(btnClose);

  ICQUser *u = gUserManager.FetchUser(m_lUsers.front().c_str(), m_nPPID, LOCK_R);
  if (u != NULL && u->NewMessages() > 0)
  {
    unsigned short i;
    /*
     Create an item for the message we're currently viewing.
    */
    if (mainwin->m_bMsgChatView)
    {
      for (i = 0; i < u->NewMessages(); i++)
        if (u->EventPeek(i)->SubCommand() != ICQ_CMDxSUB_MSG)
          break;
      if (i == u->NewMessages())
        i = 0;
    }
    else
      i = 0;

    MsgViewItem *e = new MsgViewItem(u->EventPeek(i), codec, msgView);
    m_highestEventId = u->EventPeek(i)->Id();
    /*
     Create items for all the messages which already await
     in the queue. We cannot rely on getting CICQSignals for them
     since they might've arrived before the dialog appeared,
     possibly being undisplayed messages from previous licq session.
    */
    for (i++; i < u->NewMessages(); i++)
    {
      CUserEvent* event = u->EventPeek(i);
      if (!mainwin->m_bMsgChatView ||
          event->SubCommand() != ICQ_CMDxSUB_MSG)
      {
        new MsgViewItem(event, codec, msgView);
        // Make sure we don't add this message again, even if we'll
        // receive an userUpdated signal for it.
        if (m_highestEventId < event->Id())
           m_highestEventId = event->Id();
      }
    }
    gUserManager.DropUser(u);
    slot_printMessage(e);
    msgView->setSelected(e, true);
    msgView->ensureItemVisible(e);
  }
  else
    gUserManager.DropUser(u);

  connect(this, SIGNAL(encodingChanged()), this, SLOT(slot_setEncoding()));
}

void UserViewEvent::slot_setEncoding() {
  /* if we have an open view, refresh it */
  if (this->msgView) {
    slot_printMessage(this->msgView->selectedItem());
  }
}

UserViewEvent::~UserViewEvent()
{
}


void UserViewEvent::slot_close()
{
  m_bDeleteUser = btnClose->stateWhenPressed() & ControlButton;
  close();
}


//-----UserViewEvent::slot_autoClose-----------------------------------------
void UserViewEvent::slot_autoClose()
{
  if(!chkAutoClose->isChecked()) return;

  ICQUser *u = gUserManager.FetchUser(m_lUsers.front().c_str(), m_nPPID, LOCK_R);
  bool doclose = (u->NewMessages() == 0);
  gUserManager.DropUser(u);

  if(doclose)
    close();
}

//-----UserViewEvent::updateNextButton---------------------------------------
void UserViewEvent::updateNextButton()
{
  int num = 0;

  MsgViewItem *it = static_cast<MsgViewItem*>(msgView->firstChild());
  MsgViewItem *e = NULL;

  while (it)
  {
    if (it->m_nEventId != -1 && it->msg->Direction() == D_RECEIVER)
    {
      e = it;
      num++;
    }
    it = static_cast<MsgViewItem*>(it->nextSibling());
  }

  btnReadNext->setEnabled(num > 0);

  if (num > 1)
    btnReadNext->setText(tr("Nex&t (%1)").arg(num));
  else if (num == 1)
    btnReadNext->setText(tr("Nex&t"));

  if(e && e->msg)
    btnReadNext->setIconSet(CMainWindow::iconForEvent(e->msg->SubCommand()));
}


//-----UserViewEvent::slot_printMessage--------------------------------------
void UserViewEvent::slot_printMessage(QListViewItem *eq)
{
  if (eq == NULL) return;

  MsgViewItem *e = (MsgViewItem *)eq;

  btnRead1->setText("");
  btnRead2->setText("");
  btnRead3->setText("");
  btnRead4->setText("");
  btnRead1->setEnabled(false);
  btnRead2->setEnabled(false);
  btnRead3->setEnabled(false);
  btnRead4->setEnabled(false);
  btnEncoding->setEnabled(true);

  CUserEvent *m = e->msg;
  m_xCurrentReadEvent = m;
  // Set the color for the message
  mlvRead->setBackground(QColor(m->Color()->BackRed(), m->Color()->BackGreen(), m->Color()->BackBlue()));
  mlvRead->setForeground(QColor(m->Color()->ForeRed(), m->Color()->ForeGreen(), m->Color()->ForeBlue()));
  // Set the text
  if (m->SubCommand() == ICQ_CMDxSUB_SMS)
     m_messageText = QString::fromUtf8(m->Text());
  else
     m_messageText = codec->toUnicode(m->Text());

  // Looks like there's no other way to set color in Qt 3's RichText control
  QString colorAttr;
  colorAttr.sprintf(QString::fromLatin1("#%02x%02x%02x"), m->Color()->ForeRed(), m->Color()->ForeGreen(), m->Color()->ForeBlue());
  mlvRead->setText("<font color=\"" + colorAttr + "\">" + MLView::toRichText(m_messageText, true) + "</font>");

  mlvRead->setCursorPosition(0, 0);

  if (m->Direction() == D_RECEIVER && (m->Command() == ICQ_CMDxTCP_START || m->Command() == ICQ_CMDxRCV_SYSxMSGxONLINE || m->Command() == ICQ_CMDxRCV_SYSxMSGxOFFLINE))
  {
    switch (m->SubCommand())
    {
      case ICQ_CMDxSUB_CHAT:  // accept or refuse a chat request
      case ICQ_CMDxSUB_FILE:  // accept or refuse a file transfer
        btnRead1->setText(tr("&Reply"));
        if (m->IsCancelled())
        {
          mlvRead->append(tr("\n--------------------\nRequest was cancelled."));
        }
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
              ((CEventChat *)m)->Port() == 0)
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
        btnEncoding->setEnabled(false);
        break;

      case ICQ_CMDxSUB_URL:   // view a url
        btnRead1->setText(tr("&Reply"));
        btnRead2->setText(tr("&Quote"));
        btnRead3->setText(tr("&Forward"));
#ifndef USE_KDE
        if (server->getUrlViewer() != NULL)
#endif
          btnRead4->setText(tr("&View"));
        break;

      case ICQ_CMDxSUB_AUTHxREQUEST:
      {
        btnRead1->setText(tr("A&uthorize"));
        btnRead2->setText(tr("&Refuse"));
        CEventAuthRequest *pAuthReq = (CEventAuthRequest *)m;;
        ICQUser *u = gUserManager.FetchUser(pAuthReq->IdString(), pAuthReq->PPID(), LOCK_R);
        if (u == NULL)
          btnRead3->setText(tr("A&dd User"));
        else
          gUserManager.DropUser(u);
        btnRead4->setText(tr("&View Info"));
        break;
      }
      case ICQ_CMDxSUB_AUTHxGRANTED:
      {
        CEventAuthGranted *pAuth = (CEventAuthGranted *)m;
        ICQUser *u = gUserManager.FetchUser(pAuth->IdString(), pAuth->PPID(), LOCK_R);
        if (u == NULL)
          btnRead1->setText(tr("A&dd User"));
        else
          gUserManager.DropUser(u);
        btnRead4->setText(tr("&View Info"));
        break;
      }
      case ICQ_CMDxSUB_ADDEDxTOxLIST:
      {
        CEventAdded *pAdd = (CEventAdded *)m;
        ICQUser *u = gUserManager.FetchUser(pAdd->IdString(), pAdd->PPID(), LOCK_R);
        if (u == NULL)
          btnRead1->setText(tr("A&dd User"));
        else
          gUserManager.DropUser(u);
        btnRead4->setText(tr("&View Info"));
        break;
      }
      case ICQ_CMDxSUB_CONTACTxLIST:
      {
        int s = static_cast<CEventContactList*>(m)->Contacts().size();
        if(s > 1)
          btnRead1->setText(tr("A&dd %1 Users").arg(s));
        else if(s == 1)
          btnRead1->setText(tr("A&dd User"));
        break;
      }
      case ICQ_CMDxSUB_EMAILxALERT:
      {
        btnRead1->setText(tr("&View Email"));
        break;
      }
    } // switch
  }  // if

  if (!btnRead1->text().isEmpty()) btnRead1->setEnabled(true);
  if (!btnRead2->text().isEmpty()) btnRead2->setEnabled(true);
  if (!btnRead3->text().isEmpty()) btnRead3->setEnabled(true);
  if (!btnRead4->text().isEmpty()) btnRead4->setEnabled(true);

  btnRead1->setFocus();

  if (e->m_nEventId != -1 && e->msg->Direction() == D_RECEIVER)
  {
    // clear the event only after all the slots have been invoked
    QTimer::singleShot(0, this, SLOT(slot_clearEvent()));
    e->MarkRead();
  }
}

void UserViewEvent::slot_clearEvent()
{
  ICQUser *u = gUserManager.FetchUser(m_lUsers.front().c_str(), m_nPPID, LOCK_W);
  u->EventClearId(m_xCurrentReadEvent->Id());
  gUserManager.DropUser(u);
}

void UserViewEvent::generateReply()
{
  QString s;

  if (mlvRead->hasMarkedText())
  {
    mlvRead->setTextFormat(PlainText);
    s = QString("> ") + mlvRead->markedText();
    mlvRead->setTextFormat(RichText);
  }
  else
    // we don't use mlvRead->text() since on Qt3 it returns rich text
    if (!m_messageText.stripWhiteSpace().isEmpty())
      s = QString("> ") + m_messageText;

  s.replace(QRegExp("\n"), QString::fromLatin1("\n> "));
  s = s.stripWhiteSpace();
  if (!s.isEmpty())
    s += "\n\n";

  sendMsg(s);
}


void UserViewEvent::sendMsg(QString txt)
{
  UserSendMsgEvent *e = new UserSendMsgEvent(server, sigman, mainwin, m_lUsers.front().c_str(), m_nPPID);
  e->setText(txt);

  // Find a good position for the new window
  if (mainwin->m_bAutoPosReplyWin)
  {
    int yp = btnRead1->parentWidget()->mapToGlobal(QPoint(0, 0)).y();
    if (yp + e->height() + 8 > QApplication::desktop()->height())
      yp = QApplication::desktop()->height() - e->height() - 8;
    e->move(x(), yp);
  }
  QTimer::singleShot( 10, e, SLOT( show() ) );

  connect(e, SIGNAL(autoCloseNotify()), this, SLOT(slot_autoClose()));
  connect(e, SIGNAL(signal_msgtypechanged(UserSendCommon *, UserSendCommon *)),
          this, SLOT(slot_msgtypechanged(UserSendCommon *, UserSendCommon *)));
}

void UserViewEvent::slot_msgtypechanged(UserSendCommon *old_t, UserSendCommon *new_t)
{
  disconnect(old_t, SIGNAL(autoCloseNotify()), this, SLOT(slot_autoClose()));
  disconnect(old_t, SIGNAL(signal_msgtypechanged(UserSendCommon *, UserSendCommon *)),
          this, SLOT(slot_msgtypechanged(UserSendCommon *, UserSendCommon *)));

  connect(new_t, SIGNAL(autoCloseNotify()), this, SLOT(slot_autoClose()));
  connect(new_t, SIGNAL(signal_msgtypechanged(UserSendCommon *, UserSendCommon *)),
          this, SLOT(slot_msgtypechanged(UserSendCommon *, UserSendCommon *)));
}


void UserViewEvent::slot_btnRead1()
{
  if (m_xCurrentReadEvent == NULL) return;

  switch (m_xCurrentReadEvent->SubCommand())
  {
    case ICQ_CMDxSUB_MSG:  // reply/quote
    case ICQ_CMDxSUB_URL:
    case ICQ_CMDxSUB_CHAT:
    case ICQ_CMDxSUB_FILE:
      sendMsg("");
      break;

    case ICQ_CMDxSUB_AUTHxREQUEST:
    {
      CEventAuthRequest *p = (CEventAuthRequest *)m_xCurrentReadEvent;
      (void) new AuthUserDlg(server, p->IdString(), p->PPID(), true);
      break;
    }

    case ICQ_CMDxSUB_AUTHxGRANTED:
    {
      CEventAuthGranted *p = (CEventAuthGranted *)m_xCurrentReadEvent;
      server->AddUserToList(p->IdString(), p->PPID());
      break;
    }

    case ICQ_CMDxSUB_ADDEDxTOxLIST:
    {
      CEventAdded *p = (CEventAdded *)m_xCurrentReadEvent;
      server->AddUserToList(p->IdString(), p->PPID());
      break;
    }

    case ICQ_CMDxSUB_CONTACTxLIST:
    {
      const ContactList& cl = static_cast<CEventContactList*>(m_xCurrentReadEvent)->Contacts();

      ContactList::const_iterator it;
      for(it = cl.begin(); it != cl.end(); ++it) {
        ICQUser *u = gUserManager.FetchUser((*it)->IdString(), (*it)->PPID(), LOCK_R);
        if(u == NULL)
          server->AddUserToList((*it)->IdString(), (*it)->PPID());
        gUserManager.DropUser(u);
      }
      btnRead1->setEnabled(false);
      break;
    }

    case ICQ_CMDxSUB_EMAILxALERT:
    {
      //XXX For now assume MSN protocol, will need to be fixed soon.
      CEventEmailAlert *p = (CEventEmailAlert *)m_xCurrentReadEvent;
      
      // Create the HTML 
      QString url = BASE_DIR;
      url += "/.msn_email.html";
      
      QString strUser = p->To();
      QString strHTML = QString("<html><head><noscript><meta http-equiv=Refresh content=\"0; url=http://www.hotmail.com\">"
                     "</noscript></head><body onload=\"document.pform.submit(); \"><form name=\"pform\" action=\""
                     "%1\" method=\"POST\"><input type=\"hidden\" name=\"mode\" value=\"ttl\">"
                     "<input type=\"hidden\" name=\"login\" value=\"%2\"><input type=\"hidden\" name=\"username\""
                     "value=\"%3\"><input type=\"hidden\" name=\"sid\" value=\"%4\"><input type=\"hidden\" name=\"kv\" value=\""
                     "%5\"><input type=\"hidden\" name=\"id\" value=\"%6\"><input type=\"hidden\" name=\"sl\" value=\"9\"><input "
                     "type=\"hidden\" name=\"rru\" value=\"%7\"><input type=\"hidden\" name=\"auth\" value=\"%8\""
                     "><input type=\"hidden\" name=\"creds\" value=\"%9\"><input type=\"hidden\" name=\"svc\" value=\"mail\">"
                      "<input type=\"hidden\" name=\"js\"value=\"yes\"></form></body></html>")
                        .arg(p->PostURL())
                        .arg(strUser.left(strUser.find("@")))
                        .arg(strUser)
                        .arg(p->SID())
                        .arg(p->KV())
                        .arg(p->Id())
                        .arg(p->MsgURL())
                        .arg(p->MSPAuth())
                        .arg((char *)p->Creds());
      QFile fileHTML(url);
      fileHTML.open(IO_WriteOnly);
      fileHTML.writeBlock(strHTML, strHTML.length());
      fileHTML.close();
      
      // Now we have to add the file:// after it was created, but before
      // it is executed.
      url = "file://" + url;
#ifdef USE_KDE
      KApplication* app = static_cast<KApplication*>(qApp);
      // If no URL viewer is set, use KDE default
      if (mainwin->licqDaemon && (!mainwin->licqDaemon->getUrlViewer()))
        app->invokeBrowser(url);
      else
#endif
      {
        if (mainwin->licqDaemon == NULL)
          WarnUser(this, tr("Licq is unable to find a browser application due to an internal error."));
        else if (!mainwin->licqDaemon->ViewUrl(url.local8Bit().data()))
          WarnUser(this, tr("Licq is unable to start your browser and open the URL.\nYou will need to start the browser and open the URL manually."));
      }
    }
  } // switch
}

void UserViewEvent::slot_btnRead2()
{
  if (m_xCurrentReadEvent == NULL) return;

  switch (m_xCurrentReadEvent->SubCommand())
  {
    case ICQ_CMDxSUB_MSG:  // quote
    case ICQ_CMDxSUB_URL:
      generateReply();
      break;

    case ICQ_CMDxSUB_CHAT:  // accept a chat request
    {
      m_xCurrentReadEvent->SetPending(false);
      btnRead2->setEnabled(false);
      btnRead3->setEnabled(false);
      CEventChat *c = (CEventChat *)m_xCurrentReadEvent;
      ChatDlg *chatDlg = new ChatDlg(m_lUsers.front().c_str(), m_nPPID, server, mainwin);
      if (c->Port() != 0)  // Joining a multiparty chat (we connect to them)
      {
        if (chatDlg->StartAsClient(c->Port()))
        {
          //TODO in CICQDaemon
          server->icqChatRequestAccept(m_lUsers.front().c_str(), 0,
            c->Clients(), c->Sequence(), c->MessageID(), c->IsDirect());
        }
      }
      else  // single party (other side connects to us)
      {
        if (chatDlg->StartAsServer())
        {
          //TODO in CICQDaemon
          server->icqChatRequestAccept(m_lUsers.front().c_str(),
            chatDlg->LocalPort(), c->Clients(), c->Sequence(), c->MessageID(), c->IsDirect());
        }
      }
      break;
    }

    case ICQ_CMDxSUB_FILE:  // accept a file transfer
    {
      m_xCurrentReadEvent->SetPending(false);
      btnRead2->setEnabled(false);
      btnRead3->setEnabled(false);
      CEventFile *f = (CEventFile *)m_xCurrentReadEvent;
      CFileDlg *fileDlg = new CFileDlg(m_lUsers.front().c_str(), m_nPPID, server);

      if (fileDlg->ReceiveFiles())
      {
        //TODO in CICQDaemon
        server->icqFileTransferAccept(m_lUsers.front().c_str(),
          fileDlg->LocalPort(), f->Sequence(), f->MessageID(), f->IsDirect(),
          f->FileDescription(), f->Filename(), f->FileSize());
      }
      break;
    }

    case ICQ_CMDxSUB_AUTHxREQUEST:
    {
      CEventAuthRequest *p = (CEventAuthRequest *)m_xCurrentReadEvent;
      (void) new AuthUserDlg(server, p->IdString(), p->PPID(), false);
      break;
    }
  } // switch

}


void UserViewEvent::slot_btnRead3()
{
  if (m_xCurrentReadEvent == NULL) return;

  switch (m_xCurrentReadEvent->SubCommand())
  {
    case ICQ_CMDxSUB_MSG:  // Forward
    case ICQ_CMDxSUB_URL:
    {
      CForwardDlg *f = new CForwardDlg(sigman, m_xCurrentReadEvent, this);
      f->show();
      break;
    }

    case ICQ_CMDxSUB_CHAT:  // refuse a chat request
    {
      CRefuseDlg *r = new CRefuseDlg(m_lUsers.front().c_str(), m_nPPID, tr("Chat"), this);

      if (r->exec())
      {
        m_xCurrentReadEvent->SetPending(false);
        CEventChat *c = (CEventChat *)m_xCurrentReadEvent;
        btnRead2->setEnabled(false);
        btnRead3->setEnabled(false);

        //TODO in CICQDaemon
        server->icqChatRequestRefuse(m_lUsers.front().c_str(),
          codec->fromUnicode(r->RefuseMessage()),
          m_xCurrentReadEvent->Sequence(), c->MessageID(), c->IsDirect());
      }
      delete r;
      break;
    }

    case ICQ_CMDxSUB_FILE:  // refuse a file transfer
    {
      CRefuseDlg *r = new CRefuseDlg(m_lUsers.front().c_str(), m_nPPID, tr("File Transfer"), this);

      if (r->exec())
      {
        m_xCurrentReadEvent->SetPending(false);
        CEventFile *f = (CEventFile *)m_xCurrentReadEvent;
        btnRead2->setEnabled(false);
        btnRead3->setEnabled(false);

        //TODO
        server->icqFileTransferRefuse(m_lUsers.front().c_str(),
          codec->fromUnicode(r->RefuseMessage()),
          m_xCurrentReadEvent->Sequence(), f->MessageID(), f->IsDirect());
      }
      delete r;
      break;
    }

    case ICQ_CMDxSUB_AUTHxREQUEST:
    {
      CEventAuthRequest *p = (CEventAuthRequest *)m_xCurrentReadEvent;
      server->AddUserToList(p->IdString(), p->PPID());
      break;
    }
  }
}


void UserViewEvent::slot_btnRead4()
{
  if (m_xCurrentReadEvent == NULL) return;

  switch (m_xCurrentReadEvent->SubCommand())
  {
    case ICQ_CMDxSUB_MSG:
      mainwin->callFunction(mnuUserSendChat, m_lUsers.front().c_str(), m_nPPID);
      break;
    case ICQ_CMDxSUB_CHAT:  // join to current chat
    {
      CEventChat *c = (CEventChat *)m_xCurrentReadEvent;
      if (c->Port() != 0)  // Joining a multiparty chat (we connect to them)
      {
        ChatDlg *chatDlg = new ChatDlg(m_lUsers.front().c_str(), m_nPPID, server, mainwin);
        //TODO
        if (chatDlg->StartAsClient(c->Port()))
          server->icqChatRequestAccept(m_lUsers.front().c_str(), 0, c->Clients(), c->Sequence(), c->MessageID(), c->IsDirect());
      }
      else  // single party (other side connects to us)
      {
        ChatDlg *chatDlg = NULL;
        CJoinChatDlg *j = new CJoinChatDlg(this);
        if (j->exec() && (chatDlg = j->JoinedChat()) != NULL)
          //TODO
          server->icqChatRequestAccept(m_lUsers.front().c_str(), chatDlg->LocalPort(), c->Clients(), c->Sequence(), c->MessageID(), c->IsDirect());
        delete j;
      }
      break;
    }
    case ICQ_CMDxSUB_URL:   // view a url
      emit viewurl(this, ((CEventUrl *)m_xCurrentReadEvent)->Url());
      break;

    case ICQ_CMDxSUB_AUTHxREQUEST: // Fall through
    case ICQ_CMDxSUB_AUTHxGRANTED:
    case ICQ_CMDxSUB_ADDEDxTOxLIST:
    {
      const char* id = NULL;
      unsigned long ppid = 0;
#define GETINFO(sub, type) \
      if (m_xCurrentReadEvent->SubCommand() == sub) \
      { \
        type* p = dynamic_cast<type*>(m_xCurrentReadEvent); \
        id = p->IdString(); \
        ppid = p->PPID(); \
      }

      GETINFO(ICQ_CMDxSUB_AUTHxREQUEST, CEventAuthRequest);
      GETINFO(ICQ_CMDxSUB_AUTHxGRANTED, CEventAuthGranted);
      GETINFO(ICQ_CMDxSUB_ADDEDxTOxLIST, CEventAdded);
#undef GETINFO

      if (id == NULL || ppid == 0)
        break;

      ICQUser* u = gUserManager.FetchUser(id, ppid, LOCK_R);
      if (u == NULL)
        gLicqDaemon->AddUserToList(id, ppid, false, true);
      else
        gUserManager.DropUser(u);

      mainwin->callInfoTab(mnuUserGeneral, id, ppid, false, true);
      break;
    }
  }
}


void UserViewEvent::slot_btnReadNext()
{
  MsgViewItem *it = static_cast<MsgViewItem*>(msgView->firstChild());
  MsgViewItem *e = NULL;

  while (it)
  {
    if (it->m_nEventId != -1 && it->msg->Direction() == D_RECEIVER)
    {
      e = it;
    }
    it = static_cast<MsgViewItem*>(it->nextSibling());
  }

  updateNextButton();

  if (e != NULL)
  {
    msgView->setSelected(e, true);
    msgView->ensureItemVisible(e);
    slot_printMessage(e);
  }
}


void UserViewEvent::UserUpdated(CICQSignal* sig, const char* szId, unsigned long nPPID)
{
  ICQUser *u = gUserManager.FetchUser(szId, nPPID, LOCK_R);
  if (!u) return;
  if (sig->SubSignal() == USER_EVENTS)
  {
    if (sig->Argument() > 0)
    {
      int eventId = sig->Argument();
      CUserEvent *e = u->EventPeekId(eventId);
      // Making sure we didn't handle this message already.
      if (e != NULL && m_highestEventId < eventId &&
          (!mainwin->m_bMsgChatView || e->SubCommand() != ICQ_CMDxSUB_MSG))
      {
         m_highestEventId = eventId;
         MsgViewItem *m = new MsgViewItem(e, codec, msgView);
         msgView->ensureItemVisible(m);
      }
    }

    if (sig->Argument() != 0) updateNextButton();
  }
  gUserManager.DropUser(u);
}


void UserViewEvent::slot_sentevent(ICQEvent *e)
{
  if (e->PPID() != m_nPPID || strcmp(m_lUsers.front().c_str(), e->Id()) != 0) return;

  if (!mainwin->m_bMsgChatView)
    (void) new MsgViewItem(e->GrabUserEvent(), codec, msgView);
}


//=====UserSendCommon========================================================
UserSendCommon::UserSendCommon(CICQDaemon *s, CSignalManager *theSigMan,
                               CMainWindow *m, const char *szId,
                               unsigned long nPPID, QWidget* parent, const char* name)
  : UserEventCommon(s, theSigMan, m, szId, nPPID, parent, name)
{
  grpMR = NULL;
  tmpWidgetWidth = 0;
  m_bGrpMRIsVisible = false;
  clearDelay = 250;

  QAccel *a = new QAccel( this );
  a->connectItem(a->insertItem(Key_Escape), this, SLOT(slot_cancelSend()));

  if (mainwin->userEventTabDlg &&
      parent == mainwin->userEventTabDlg)
  {
    a->connectItem(a->insertItem(ALT + Key_Left),
		   mainwin->userEventTabDlg, SLOT(moveLeft()));
    a->connectItem(a->insertItem(CTRL + SHIFT + Key_Tab),
		   mainwin->userEventTabDlg, SLOT(moveLeft()));
    a->connectItem(a->insertItem(ALT + Key_Right),
		   mainwin->userEventTabDlg, SLOT(moveRight()));
    a->connectItem(a->insertItem(CTRL + Key_Tab),
		   mainwin->userEventTabDlg, SLOT(moveRight()));
  }

  QGroupBox *box = new QGroupBox(this);
  top_lay->addWidget(box);
  QBoxLayout *vlay = new QVBoxLayout(box, 10, 5);
  QBoxLayout *hlay = new QHBoxLayout(vlay);
  chkSendServer = new QCheckBox(tr("Se&nd through server"), box);
  ICQUser *u = gUserManager.FetchUser(m_lUsers.front().c_str(), m_nPPID, LOCK_R);
  chkSendServer->setChecked(u->SendServer() ||
    (u->StatusOffline() && u->SocketDesc(ICQ_CHNxNONE) == -1));

  if( (u->GetInGroup(GROUPS_SYSTEM, GROUP_INVISIBLE_LIST)) ||
      (u->Port() == 0 && u->SocketDesc(ICQ_CHNxNONE) == -1))
  {
    chkSendServer->setChecked(true);
    chkSendServer->setEnabled(false);
  }
    
  // With MSN contacts always mark as SendServer and disable the checkbox
  if (u->PPID() == MSN_PPID)
  {
    chkSendServer->setChecked(true);
    chkSendServer->setEnabled(false);
  }

  gUserManager.DropUser(u);
  hlay->addWidget(chkSendServer);
  chkUrgent = new QCheckBox(tr("U&rgent"), box);
  hlay->addWidget(chkUrgent);
  chkMass = new QCheckBox(tr("M&ultiple recipients"), box);
  hlay->addWidget(chkMass);
  connect(chkMass, SIGNAL(toggled(bool)), this, SLOT(massMessageToggled(bool)));
  btnEmoticon = new QPushButton(box);
  btnEmoticon->setPixmap(smile_xpm);
  connect(btnEmoticon, SIGNAL(clicked()), this, SLOT(slot_Emoticon()));
  hlay->addWidget(btnEmoticon);
  btnForeColor = new QPushButton(box);
  btnForeColor->setPixmap(chatChangeFg_xpm);
  connect(btnForeColor, SIGNAL(clicked()), this, SLOT(slot_SetForegroundICQColor()));
  hlay->addWidget(btnForeColor);
  btnBackColor = new QPushButton(box);
  btnBackColor->setPixmap(chatChangeBg_xpm);
  connect(btnBackColor, SIGNAL(clicked()), this, SLOT(slot_SetBackgroundICQColor()));
  hlay->addWidget(btnBackColor);

  QBoxLayout *h_lay = new QHBoxLayout(top_lay);
  if (!m_bOwner)
  {
    QPushButton *btnMenu = new QPushButton(tr("&Menu"), this);
    h_lay->addWidget(btnMenu);
    connect(btnMenu, SIGNAL(pressed()), this, SLOT(slot_usermenu()));
    btnMenu->setPopup(mainwin->UserMenu());
  }
  cmbSendType = new QComboBox(this);
  cmbSendType->insertItem(tr("Message"), UC_MESSAGE);
  cmbSendType->insertItem(tr("URL"), UC_URL);
  cmbSendType->insertItem(tr("Chat Request"), UC_CHAT);
  cmbSendType->insertItem(tr("File Transfer"), UC_FILE);
  cmbSendType->insertItem(tr("Contact List"), UC_CONTACT);
  cmbSendType->insertItem(tr("SMS"), UC_SMS);
  connect(cmbSendType, SIGNAL(activated(int)), this, SLOT(changeEventType(int)));
  h_lay->addWidget(cmbSendType);
  h_lay->addStretch(1);
  btnSend = new QPushButton(tr("&Send"), this);
  btnSend->setDefault(true);
  int w = QMAX(btnSend->sizeHint().width(), 75);
  // add a wrapper around the send button that
  // tries to establish a secure connection first.
  connect( btnSend, SIGNAL( clicked() ), this, SLOT( trySecure() ) );

  btnClose = new QPushButton(tr("&Close"), this);
  btnClose->setAutoDefault(true);
  w = QMAX(btnClose->sizeHint().width(), w);
  btnSend->setFixedWidth(w);
  btnClose->setFixedWidth(w);
  h_lay->addWidget(btnSend);
  h_lay->addWidget(btnClose);
  connect(btnClose, SIGNAL(clicked()), this, SLOT(slot_close()));
  splView = new QSplitter(Vertical, mainWidget);
  //splView->setOpaqueResize();
  
  mleHistory = 0;
  if (mainwin->m_bMsgChatView)
  {
    mleHistory = new CMessageViewWidget(m_lUsers.front().c_str(), m_nPPID, mainwin, splView);
    
    u = gUserManager.FetchUser(m_lUsers.front().c_str(), m_nPPID, LOCK_R);
    if (u && mainwin->m_bShowHistory)
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
          
        bool bUseHTML = !isdigit((m_lUsers.front().c_str())[1]); 
        QTextCodec *codec = UserCodec::codecForICQUser(u);
        QString tmp = "";
        QString contactName = QString::fromUtf8(u->GetAlias());
        ICQOwner *o = gUserManager.FetchOwner(m_nPPID, LOCK_R);
        QString ownerName;
        if (o)
        {
          ownerName = QString::fromUtf8(o->GetAlias());
          gUserManager.DropOwner(o);
        }
        else
          ownerName = QString(tr("Error! no owner set"));
        QDateTime date;

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
        for (int i = 0; i < nMaxNumToShow && lHistoryIter != lHistoryList.end(); i++)
        {
          QString str;
          date.setTime_t((*lHistoryIter)->Time());
          QString messageText;
          if ((*lHistoryIter)->SubCommand() == ICQ_CMDxSUB_SMS) // SMSs are always in UTF-8
            messageText = QString::fromUtf8((*lHistoryIter)->Text());
          else
            messageText = codec->toUnicode((*lHistoryIter)->Text());
  
          mleHistory->addMsg((*lHistoryIter)->Direction(), true,
              ((*lHistoryIter)->SubCommand() == ICQ_CMDxSUB_MSG ? 
                  QString("") : (EventDescription((*lHistoryIter)) + " ")),
              date,
              (*lHistoryIter)->IsDirect(),
              (*lHistoryIter)->IsMultiRec(),
              (*lHistoryIter)->IsUrgent(),
              (*lHistoryIter)->IsEncrypted(),
              (*lHistoryIter)->Direction() == D_RECEIVER ? contactName : ownerName,
              MLView::toRichText(messageText, true, bUseHTML));
          lHistoryIter++;
        }
        if (tmp != "")
          mleHistory->append(tmp);

        mleHistory->GotoEnd();

        ICQUser::ClearHistory(lHistoryList);
      }
    }
    
    // Collect all messages to put them in the correct time order
    vector< pair<CUserEvent *, char *> > m_vMsgs;
    
    // add all unread messages.
    if (u && u->NewMessages() > 0)
    {
      for (unsigned short i = 0; i < u->NewMessages(); i++)
      {
        CUserEvent *e = u->EventPeek(i);
        // Get the convo id now
        unsigned long nConvoId = e->ConvoId();
        if (m_nConvoId == 0)
          m_nConvoId = nConvoId;
        
        if (e->Id() > m_highestEventId && nConvoId == m_nConvoId)
          m_highestEventId = e->Id();
        
        if (nConvoId == m_nConvoId)
          m_vMsgs.push_back(make_pair(e, strdup(u->IdString())));
      }
      gUserManager.DropUser(u);
    
      // Now add messages that are a part of this convo
      if (m_nPPID != LICQ_PPID)
      {
        FOR_EACH_PROTO_USER_START(m_nPPID, LOCK_R)
        {
          if (pUser->NewMessages() && strcmp(m_lUsers.front().c_str(), pUser->IdString()) != 0)
          {
            for (unsigned short i = 0; i < pUser->NewMessages(); i++)
            {            
              CUserEvent *e = pUser->EventPeek(i);
              if (e->Id() > m_highestEventId && e->ConvoId() == m_nConvoId)
                m_highestEventId = e->Id();
                
              if (e->ConvoId() == m_nConvoId)
              {
                // add to the convo list (but what if they left by the time we open this?)
                char *szRealId;
                ICQUser::MakeRealId(pUser->IdString(), pUser->PPID(), szRealId);
                m_lUsers.push_back(szRealId);
                delete [] szRealId;
                m_vMsgs.push_back(make_pair(e, strdup(pUser->IdString())));
              }
            }
          }
        }
        FOR_EACH_PROTO_USER_END
      }

      // Sort the messages by time
      stable_sort(m_vMsgs.begin(), m_vMsgs.end(), OrderMessages);
    
      // Now, finally add them
      vector< pair<CUserEvent *, char *> >::iterator MsgIter;
      for (MsgIter = m_vMsgs.begin(); MsgIter != m_vMsgs.end(); ++MsgIter)
      {
        mleHistory->addMsg((*MsgIter).first, (*MsgIter).second, m_nPPID);
        free((*MsgIter).second);
      }
      m_vMsgs.clear();
      
      // If the user closed the chat window, we have to make sure we aren't
      // using the old nConvoId
      if (s->FindConversation(m_nConvoId) == 0)
        m_nConvoId = 0;

      // Fetch the user again since we dropped it above
      u = gUserManager.FetchUser(m_lUsers.front().c_str(), m_nPPID, LOCK_R);
    }
    
    // Do we already have an open socket?
    if (m_nConvoId == 0 && u)
    {
//       bool bHasOpen = false;
//       QPtrListIterator<UserSendCommon> it(licqUserSend);
//       for (; it.current(); ++it)
//       {
//         if (strcmp((*it)->Id(), m_szId) == 0 && (*it)->PPID() == m_nPPID)
//         {
//           bHasOpen = true;
//           break;
//         }
//       }
      
      if (u->SocketDesc(ICQ_CHNxNONE) != 1)
      {
        CConversation *pConv = s->FindConversation(u->SocketDesc(ICQ_CHNxNONE));
        if (pConv)
          m_nConvoId = pConv->CID();
      }
    }
    
    gUserManager.DropUser(u);

    connect(mleHistory, SIGNAL(viewurl(QWidget*, QString)), mainwin, SLOT(slot_viewurl(QWidget *, QString)));
    connect (mainwin, SIGNAL(signal_sentevent(ICQEvent *)), mleHistory, SLOT(addMsg(ICQEvent *)));
    //splView->setResizeMode(mleHistory, QSplitter::FollowSizeHint);
  }

  {
    tmrSendTyping = new QTimer(this);
    connect(tmrSendTyping, SIGNAL(timeout()), SLOT(slot_textChanged_timeout()));
  }

  mleSend = new MLEditWrap(true, splView, true);
  mleSend->setCheckSpellingEnabled(mainwin->m_bCheckSpellingEnabled);
  mleSend->installEventFilter(this); // Enables send with enter

  if (mainwin->m_bMsgChatView)
  {
    splView->setResizeMode(mleSend, QSplitter::KeepSize);
    mleSend->resize(mleSend->width(), 70);
  }
  setFocusProxy(mleSend);
  setTabOrder(mleSend, btnSend);
  setTabOrder(btnSend, btnClose);
  icqColor.SetToDefault();
  mleSend->setBackground(QColor(icqColor.BackRed(), icqColor.BackGreen(), icqColor.BackBlue()));
  mleSend->setForeground(QColor(icqColor.ForeRed(), icqColor.ForeGreen(), icqColor.ForeBlue()));
  connect (mleSend, SIGNAL(signal_CtrlEnterPressed()), btnSend, SIGNAL(clicked()));
  connect(mleSend, SIGNAL(textChanged()), this, SLOT(slot_textChanged()));
  connect(this, SIGNAL(updateUser(CICQSignal*)), mainwin, SLOT(slot_updatedUser(CICQSignal*)));
  // Do not connect this before the check box gets set automatically. If the user is offline
  // the send through server flag gets set, and then that setting gets saved to disk.
  connect(chkSendServer, SIGNAL(clicked()), this, SLOT(slot_sendServerToggled()));
}

UserSendCommon::~UserSendCommon()
{
}

#undef KeyPress
bool UserSendCommon::eventFilter(QObject *watched, QEvent *e)
{
  if (watched == mleSend)
  {
    // If we're in single line chat mode we send messages with Enter and
    // insert new lines with Ctrl+Enter.
    if (mainwin->m_bSingleLineChatMode && e->type() == QEvent::KeyPress)
    {
      QKeyEvent *key = static_cast<QKeyEvent*>(e);
      const bool isEnter = (key->key() == Key_Enter || key->key() == Key_Return);
      if (isEnter)
      {
       if (key->state() & ControlButton)
          mleSend->insert("\n");
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

void UserSendCommon::convoJoin(const char *szId, unsigned long _nConvoId)
{
  if (szId == 0) return;
  
  if (mainwin->m_bMsgChatView)
  {
    ICQUser *u = gUserManager.FetchUser(szId, m_nPPID, LOCK_R);
    QString userName;
    if (u)
      userName = QString::fromUtf8(u->GetAlias());
    else
      userName = szId;
    gUserManager.DropUser(u);

    QString strMsg = QString(tr("%1 has joined the conversation."))
      .arg(userName);
    mleHistory->addNotice(QDateTime::currentDateTime(), strMsg);
  }

  if (!FindUserInConvo(const_cast<char *>(szId)))
  {
    char *szRealId;
    ICQUser::MakeRealId(szId, m_nPPID, szRealId);
    m_lUsers.push_back(szRealId);
    delete [] szRealId;
  }
  
  m_nConvoId = _nConvoId;

  // Now update the tab label
  if (mainwin->userEventTabDlg)
    mainwin->userEventTabDlg->updateConvoLabel(this);
}

void UserSendCommon::convoLeave(const char *szId, unsigned long /* _nConvoId */)
{
  if (szId == 0) return;
  
  if (mainwin->m_bMsgChatView)
  {
    ICQUser *u = gUserManager.FetchUser(szId, m_nPPID, LOCK_R);
    QString userName;
    if (u)
      userName = QString::fromUtf8(u->GetAlias());
    else
      userName = szId;

    QString strMsg = QString(tr("%1 has left the conversation."))
      .arg(userName);
    mleHistory->addNotice(QDateTime::currentDateTime(), strMsg);
    
    // Remove the typing notification if active
    if (u != 0)
    {
      if (u->GetTyping() == ICQ_TYPING_ACTIVE)
      {
        u->SetTyping(ICQ_TYPING_INACTIVEx0);
        nfoStatus->unsetPalette();
        if (mainwin->m_bTabbedChatting && mainwin->userEventTabDlg)
          mainwin->userEventTabDlg->updateTabLabel(u);  
      }
    }
    gUserManager.DropUser(u);
  }
  
  if (m_lUsers.size() > 1)
  {
    list<string>::iterator it;
    for (it = m_lUsers.begin(); it != m_lUsers.end(); it++)
    {
      if (strcasecmp(szId, it->c_str()) == 0)
      {
        m_lUsers.remove(*it);
        break;
      }
    }
    mleHistory->setOwner(m_lUsers.front().c_str());
  }
  else
    m_nConvoId = 0;

  if (mainwin->m_bMsgChatView)
  {
    // Now update the tab label
    if (mainwin->userEventTabDlg)
      mainwin->userEventTabDlg->updateConvoLabel(this);
  }
}

//-----UserSendCommon::windowActivationChange--------------------------------
void UserSendCommon::windowActivationChange(bool oldActive)
{
  if (isActiveWindow())
    QTimer::singleShot(clearDelay, this, SLOT(slot_ClearNewEvents()));
  QWidget::windowActivationChange(oldActive);
}

//-----UserSendCommon::slot_resettitle---------------------------------------
void UserSendCommon::slot_resettitle()
{
  if (mainwin->userEventTabDlg &&
      mainwin->userEventTabDlg->tabIsSelected(this))
    mainwin->userEventTabDlg->setCaption(m_sBaseTitle);

  setCaption(m_sBaseTitle);
}

//-----UserSendCommon::slot_Emoticon-----------------------------------------
void UserSendCommon::slot_Emoticon()
{
  SelectEmoticon *p = new SelectEmoticon(this);

  QWidget *desktop = qApp->desktop();
  QSize s = p->sizeHint();
  QPoint pos = QPoint(0, btnEmoticon->height());
  pos = btnEmoticon->mapToGlobal(pos);
  if (pos.x() + s.width() > desktop->width())
  {
    pos.setX(desktop->width() - s.width());
    if (pos.x() < 0)
      pos.setX(0);
  }
  if (pos.y() + s.height() > desktop->height())
  {
    pos.setY(pos.y() - btnEmoticon->height() - s.height());
    if (pos.y() < 0)
      pos.setY(0);
  }
  
  connect(p, SIGNAL(selected(const QString &)), this,
    SLOT(slot_insertEmoticon(const QString &)));
  p->move(pos);
  p->show();
}

//-----UserSecondCommon::slot_insertEmoticon-----------------------------------
void UserSendCommon::slot_insertEmoticon(const QString &sKey)
{
  if (mleSend)
    mleSend->insert(sKey);
}

//-----UserSendCommon::slot_SetForegroundColor-------------------------------
void UserSendCommon::slot_SetForegroundICQColor()
{
  if (!mleSend)
    return;

#ifdef USE_KDE
  QColor c = mleSend->foregroundColor();
  if (KColorDialog::getColor(c, this) != KColorDialog::Accepted) return;
#else
  QColor c = QColorDialog::getColor(mleSend->foregroundColor(), this);
  if (!c.isValid()) return;
#endif

  icqColor.SetForeground(c.red(), c.green(), c.blue());
  mleSend->setForeground(c);
}

void UserSendCommon::trySecure()
{
  ICQUser *u = gUserManager.FetchUser(m_lUsers.front().c_str(), m_nPPID, LOCK_R);

  bool autoSecure = ( u->AutoSecure() && gLicqDaemon->CryptoEnabled() &&
                      (u->SecureChannelSupport() == SECURE_CHANNEL_SUPPORTED ) &&
                      !chkSendServer->isChecked() && !u->Secure());
  gUserManager.DropUser( u );
  disconnect( btnSend, SIGNAL( clicked() ), this, SLOT( trySecure() ) );
  connect(btnSend, SIGNAL(clicked()), this, SLOT(sendButton()));
  if ( autoSecure ) {
    QWidget *w = new KeyRequestDlg(sigman, m_lUsers.front().c_str(), m_nPPID);
    connect(w, SIGNAL( destroyed() ), this, SLOT( sendButton() ) );
  }
  else
    sendButton();
}

void UserSendCommon::slot_textChanged()
{
  if (mleSend == NULL || mleSend->text().isEmpty())
    return;

  strTempMsg = mleSend->text();
  server->ProtoTypingNotification(m_lUsers.front().c_str(), m_nPPID, true, m_nConvoId);
  disconnect(mleSend, SIGNAL(textChanged()), this, SLOT(slot_textChanged()));
  tmrSendTyping->start(5000);
}

void UserSendCommon::slot_textChanged_timeout()
{
  if (mleSend == NULL)
  {
    tmrSendTyping->stop();
    return;
  }

  QString str = mleSend->text();

  if (str != strTempMsg)
  {
    strTempMsg = str;
    // Hack to not keep sending the typing notification to ICQ
    if (m_nPPID != LICQ_PPID)
      server->ProtoTypingNotification(m_lUsers.front().c_str(), m_nPPID, true, m_nConvoId);
  }
  else
  {
    if (tmrSendTyping->isActive())
      tmrSendTyping->stop();
    connect(mleSend, SIGNAL(textChanged()), this, SLOT(slot_textChanged()));
    server->ProtoTypingNotification(m_lUsers.front().c_str(), m_nPPID, false, m_nConvoId);
  }
}

void UserSendCommon::slot_sendServerToggled()
{
  // When the "Send through server" checkbox is toggled, we will save the setting
  // to disk, so it will be persistent.

  ICQUser* u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_W);
  if (u != NULL)
  {
    u->SetSendServer(chkSendServer->isChecked());
    gUserManager.DropUser(u);
  }
}

//-----UserSendCommon::slot_SetBackgroundColor-------------------------------
void UserSendCommon::slot_SetBackgroundICQColor()
{
  if (!mleSend)
    return;

#ifdef USE_KDE
  QColor c = mleSend->backgroundColor();
  if (KColorDialog::getColor(c, this) != KColorDialog::Accepted) return;
#else
  QColor c = QColorDialog::getColor(mleSend->backgroundColor(), this);
  if (!c.isValid()) return;
#endif

  icqColor.SetBackground(c.red(), c.green(), c.blue());
  mleSend->setBackground(c);
}


//-----UserSendCommon::slot_ClearNewEvents-----------------------------------
void UserSendCommon::slot_ClearNewEvents()
{
  ICQUser *u = 0;

  // Iterate all users in the conversation
  for (list<string>::iterator it = m_lUsers.begin(); it != m_lUsers.end(); ++it)
  {
    u = gUserManager.FetchUser((*it).c_str(), m_nPPID, LOCK_W);
    if (mainwin->m_bMsgChatView
        && isActiveWindow() && (!mainwin->userEventTabDlg ||
        (mainwin->userEventTabDlg &&
          (!mainwin->userEventTabDlg->tabExists(this) ||
            mainwin->userEventTabDlg->tabIsSelected(this))))
        )
    {
      if (u != NULL && u->NewMessages() > 0)
      {
        std::vector<int> idList;
        for (unsigned short i = 0; i < u->NewMessages(); i++)
        {
          CUserEvent *e = u->EventPeek(i);
          if (e->Id() <= m_highestEventId && e->Direction() == D_RECEIVER &&
              e->SubCommand() == ICQ_CMDxSUB_MSG)
            idList.push_back(e->Id());
        }
  
        for (unsigned short i = 0; i < idList.size(); i++)
          u->EventClearId(idList[i]);
      }
    }
    gUserManager.DropUser(u);
    u = 0;
  }
}


//-----UserSendCommon::changeEventType---------------------------------------
void UserSendCommon::changeEventType(int id)
{
  if (isType(id)) return;
  UserSendCommon* e = NULL;
  QWidget *parent = NULL;

  if (mainwin->userEventTabDlg && mainwin->userEventTabDlg->tabExists(this))
    parent = mainwin->userEventTabDlg;

  switch(id)
  {

  case UC_MESSAGE:
    e = new UserSendMsgEvent(server, sigman, mainwin, m_lUsers.front().c_str(), m_nPPID, parent);
    break;
  case UC_URL:
    e = new UserSendUrlEvent(server, sigman, mainwin, m_lUsers.front().c_str(), m_nPPID, parent);
    break;
  case UC_CHAT:
    e = new UserSendChatEvent(server, sigman, mainwin, m_lUsers.front().c_str(), m_nPPID, parent);
    break;
  case UC_FILE:
    e = new UserSendFileEvent(server, sigman, mainwin, m_lUsers.front().c_str(), m_nPPID, parent);
    break;
  case UC_CONTACT:
    e = new UserSendContactEvent(server, sigman, mainwin, m_lUsers.front().c_str(), m_nPPID, parent);
    break;
  case UC_SMS:
    e = new UserSendSmsEvent(server, sigman, mainwin, m_lUsers.front().c_str(), m_nPPID, parent);
    break;
  default:
    assert(0);
  }

  if (e != NULL)
  {
    if (e->mleSend && mleSend)
    {
      e->mleSend->setText(mleSend->text());
      e->mleSend->setModified(e->mleSend->length());
    }
    if (e->mleHistory && mleHistory){
      e->mleHistory->setText(mleHistory->text());
      e->mleHistory->GotoEnd();
    }

    if (!parent)
    {
      QPoint p = topLevelWidget()->pos();
      e->move(p);
    }

    disconnect(this, SIGNAL(finished(const char *, unsigned long)), 
               mainwin, SLOT(slot_sendfinished(const char *,unsigned long)));
    mainwin->slot_sendfinished(m_lUsers.front().c_str(), m_nPPID);
    connect(e, SIGNAL(finished(const char *, unsigned long)), mainwin,
            SLOT(slot_sendfinished(const char *, unsigned long)));
    mainwin->licqUserSend.append(e);

    emit signal_msgtypechanged(this, e);

    if (!parent)
    {
      QTimer::singleShot(10, e, SLOT(show()));
      QTimer::singleShot(100, this, SLOT(close()));
    }
    else
      mainwin->userEventTabDlg->replaceTab(this, e);
  }
}


//-----UserSendCommon::massMessageToggled------------------------------------
/*! This slot creates/removes a little widget into the usereventdlg
 *  which enables the user to collect users for mass messaging.
 */
void UserSendCommon::massMessageToggled(bool b)
{
  if (b)
  {
    chkMass->setChecked(true);

    if (mainwin->userEventTabDlg &&
        mainwin->userEventTabDlg->tabIsSelected(this))
      tmpWidgetWidth = mainwin->userEventTabDlg->width();
    else
      tmpWidgetWidth = width();
    if (grpMR == NULL)
    {
      grpMR = new QVGroupBox(this);
      top_hlay->addWidget(grpMR);

      (void) new QLabel(tr("Drag Users Here\nRight Click for Options"), grpMR);
      //TODO in CMMUserView
      lstMultipleRecipients = new CMMUserView(mainwin->colInfo, mainwin->m_bShowHeader,
          m_lUsers.front().c_str(), LICQ_PPID, mainwin, grpMR);
      lstMultipleRecipients->setFixedWidth(mainwin->UserView()->width());
    }
    grpMR->show();
    m_bGrpMRIsVisible = true;
  }
  else
  {
    chkMass->setChecked(false);
    if (grpMR != NULL && m_bGrpMRIsVisible ) /* only resize if really necessary! */
    {
      int grpMRWidth = grpMR->width();

      grpMR->hide();
      m_bGrpMRIsVisible = false;

      // resize the widget to it's origin width.
      // This is a workaround and not perfect, but resize() does not
      // work as expected. Maybe we find a better solution for this in future.
      if (mainwin->userEventTabDlg &&
          mainwin->userEventTabDlg->tabIsSelected(this))
      {
        QSize tmpMaxSize = mainwin->userEventTabDlg->maximumSize();
        if (tmpWidgetWidth == 0)
          mainwin->userEventTabDlg->setFixedWidth(mainwin->userEventTabDlg->width() - grpMRWidth);
        else
        {
          mainwin->userEventTabDlg->setFixedWidth(tmpWidgetWidth);
          tmpWidgetWidth = 0;
        }
        mainwin->userEventTabDlg->setMaximumSize(tmpMaxSize);
      }
      else
      {
        QSize tmpMaxSize = maximumSize();
        if (tmpWidgetWidth == 0)
          setFixedWidth(width() - grpMRWidth);
        else
        {
          setFixedWidth(tmpWidgetWidth);
          tmpWidgetWidth = 0;
        }
        setMaximumSize(tmpMaxSize);
      }
    }
  }
}


//-----UserSendCommon::sendButton--------------------------------------------
void UserSendCommon::sendButton()
{
  if(!mainwin->m_bManualNewUser)
  {
    ICQUser *u = gUserManager.FetchUser(m_lUsers.front().c_str(), m_nPPID, LOCK_W);

    if(u->NewUser())
    {
      u->SetNewUser(false);
      gUserManager.DropUser(u);
      CICQSignal s(SIGNAL_UPDATExUSER, USER_BASIC, m_lUsers.front().c_str(), m_nPPID);
      emit updateUser(&s);
    }
    else
      gUserManager.DropUser(u);
  }

  unsigned long icqEventTag = 0;
  if (m_lnEventTag.size())
    icqEventTag = m_lnEventTag.front();

  if (icqEventTag != 0 || m_nPPID != LICQ_PPID)
  {
    m_sProgressMsg = tr("Sending ");
    bool via_server = chkSendServer->isChecked();
    m_sProgressMsg += via_server ? tr("via server") : tr("direct");
    m_sProgressMsg += "...";
    QString title = m_sBaseTitle + " [" + m_sProgressMsg + "]";

    if (mainwin->userEventTabDlg &&
        mainwin->userEventTabDlg->tabIsSelected(this))
      mainwin->userEventTabDlg->setCaption(title);

    setCaption(title);
    setCursor(waitCursor);
    btnSend->setText(tr("&Cancel"));
    btnClose->setEnabled(false);

    if (mleSend != NULL)
      mleSend->setEnabled(false);

    disconnect(btnSend, SIGNAL(clicked()), this, SLOT(sendButton()));
    connect(btnSend, SIGNAL(clicked()), this, SLOT(slot_cancelSend()));

    connect (sigman, SIGNAL(signal_doneUserFcn(ICQEvent *)), this, SLOT(sendDone_common(ICQEvent *)));
  }
}


//-----UserSendCommon::setText-----------------------------------------------
void UserSendCommon::setText(const QString& txt)
{
  if(!mleSend) return;
  mleSend->setText(txt);
  mleSend->GotoEnd();
  mleSend->setModified(false);
}


//-----UserSendCommon::sendDone_common---------------------------------------
void UserSendCommon::sendDone_common(ICQEvent *e)
{
  if (e == NULL)
  {
    QString title = m_sBaseTitle + " [" + m_sProgressMsg + tr("error") + "]";

    if (mainwin->userEventTabDlg &&
        mainwin->userEventTabDlg->tabIsSelected(this))
      mainwin->userEventTabDlg->setCaption(title);

    setCaption(title);

    return;
  }

  unsigned long icqEventTag = 0;
  std::list<unsigned long>::iterator iter;
  for (iter = m_lnEventTag.begin(); iter != m_lnEventTag.end(); iter++)
  {
    if (e->Equals(*iter))
    {
      icqEventTag = *iter;
      m_lnEventTag.erase(iter);
      break;
    }
  }

  if (icqEventTag == 0)
    return;

  QString title, result;
  switch (e->Result())
  {
  case EVENT_ACKED:
  case EVENT_SUCCESS:
    result = tr("done");
    QTimer::singleShot(5000, this, SLOT(slot_resettitle()));
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
  title = m_sBaseTitle + " [" + m_sProgressMsg + result + "]";

  if (mainwin->userEventTabDlg &&
      mainwin->userEventTabDlg->tabIsSelected(this))
    mainwin->userEventTabDlg->setCaption(title);

  setCaption(title);

  setCursor(arrowCursor);
  btnSend->setText(tr("&Send"));
  btnClose->setEnabled(true);

  if (mleSend != NULL)
    mleSend->setEnabled(true);

  disconnect(btnSend, SIGNAL(clicked()), this, SLOT(slot_cancelSend()));
  connect(btnSend, SIGNAL(clicked()), this, SLOT(sendButton()));

  // If cancelled automatically check "Send through Server"
  if (mainwin->m_bAutoSendThroughServer && e->Result() == EVENT_CANCELLED)
    chkSendServer->setChecked(true);

  if (m_lnEventTag.size() == 0)
    disconnect (sigman, SIGNAL(signal_doneUserFcn(ICQEvent *)), this, SLOT(sendDone_common(ICQEvent *)));

  if (mleSend != NULL)
    mleSend->setFocus();

  if (e->Result() != EVENT_ACKED)
  {
    if (e->Command() == ICQ_CMDxTCP_START && e->Result() != EVENT_CANCELLED &&
       (mainwin->m_bAutoSendThroughServer ||
         QueryUser(this, tr("Direct send failed,\nsend through server?"), tr("Yes"), tr("No"))) )
      RetrySend(e, false, ICQ_TCPxMSG_NORMAL);
    return;
  }

  ICQUser *u = NULL;
  //CUserEvent *ue = e->UserEvent();
  QString msg;
  if (e->SubResult() == ICQ_TCPxACK_RETURN)
  {
    u = gUserManager.FetchUser(m_lUsers.front().c_str(), m_nPPID, LOCK_W);

    msg = tr("%1 is in %2 mode:\n%3\nSend...")
        .arg(QString::fromUtf8(u->GetAlias()))
        .arg(Strings::getStatus(u))
        .arg(codec->toUnicode(u->AutoResponse()));

    u->SetShowAwayMsg( false );
    gUserManager.DropUser(u);
    // if the original message was through server, send this one through server
    bool bServer = e->Channel() == ICQ_CHNxNONE ||
                   e->Channel() == ICQ_CHNxINFO ||
                   e->Channel() == ICQ_CHNxSTATUS;
    switch (QueryUser(this, msg, tr("Urgent"), tr(" to Contact List"), tr("Cancel")))
    {
      case 0:
        RetrySend(e, bServer, ICQ_TCPxMSG_URGENT);
        break;
      case 1:
        RetrySend(e, bServer, ICQ_TCPxMSG_LIST);
        break;
      case 2:
        break;
    }
    return;
  }
/*  else if (e->SubResult() == ICQ_TCPxACK_REFUSE)
  {
    u = gUserManager.FetchUser(m_lUsers.front().c_str(), m_nPPID, LOCK_R);
    msg = tr("%1 refused %2, send through server")
          .arg(QString::fromUtf8(u->GetAlias())).arg(EventDescription(ue));
    InformUser(this, msg);
    gUserManager.DropUser(u);
    return;
  }*/

  emit autoCloseNotify();
  if (sendDone(e))
  {
    emit mainwin->signal_sentevent(e);
    if (mainwin->m_bMsgChatView && mleHistory != NULL)
    {
      mleHistory->GotoEnd();
      resetSettings();

      // After sending URI/File/Contact/ChatRequest switch back to text message
      if (!isType(UC_MESSAGE))
        changeEventType(UC_MESSAGE);
    }
    else
      close();
  }
}

//-----UserSendCommon::RetrySend---------------------------------------------
void UserSendCommon::RetrySend(ICQEvent *e, bool bOnline, unsigned short nLevel)
{
  unsigned long icqEventTag = 0;
  chkSendServer->setChecked(!bOnline);
  chkUrgent->setChecked(nLevel == ICQ_TCPxMSG_URGENT);

  switch(e->UserEvent()->SubCommand() & ~ICQ_CMDxSUB_FxMULTIREC)
  {
    case ICQ_CMDxSUB_MSG:
    {
      ICQUser *u = gUserManager.FetchUser(m_lUsers.front().c_str(), m_nPPID, LOCK_R);
      bool bUserOffline = u->StatusOffline();
      gUserManager.DropUser(u);
      CEventMsg *ue = (CEventMsg *)e->UserEvent();
      // create initial strings (implicit copying, no allocation impact :)
      char *tmp = gTranslator.NToRN(ue->Message());
      QCString wholeMessageRaw(tmp);
      delete [] tmp;
      unsigned int wholeMessagePos = 0;


      bool needsSplitting = false;
      // If we send through server (= have message limit), and we've crossed the limit
      unsigned short nMaxSize = bUserOffline ? MAX_OFFLINE_MESSAGE_SIZE : MAX_MESSAGE_SIZE;
      if ((wholeMessageRaw.length() - wholeMessagePos) > nMaxSize)
      {
        needsSplitting = true;
      }

      QString message;
      QCString messageRaw;

      while (wholeMessageRaw.length() > wholeMessagePos)
      {
        if (needsSplitting)
        {
          // This is a bit ugly but adds safety. We don't simply search
          // for a whitespace to cut at in the encoded text (since we don't
          // really know how spaces are represented in its encoding), so
          // we take the maximum length, then convert back to a Unicode string
          // and then search for Unicode whitespaces.
          messageRaw = wholeMessageRaw.mid(wholeMessagePos, nMaxSize);
          tmp = gTranslator.RNToN(messageRaw);
          messageRaw = tmp;
          delete [] tmp;
          message = codec->toUnicode(messageRaw);

          if ((wholeMessageRaw.length() - wholeMessagePos) > nMaxSize)
          {
            // We try to find the optimal place to cut
            // (according to our narrow-minded Latin1 idea of optimal :)
            // prefer keeping sentences intact 1st
            int foundIndex = message.findRev(QRegExp("[\\.\\n]"));
            // slicing at 0 position would be useless
            if (foundIndex <= 0)
              foundIndex = message.findRev(QRegExp("\\s"));

            if (foundIndex > 0)
            {
              message.truncate(foundIndex + 1);
              messageRaw = codec->fromUnicode(message);
            }
          }
        }
        else
        {
          messageRaw = ue->Message();
        }

        icqEventTag = server->icqSendMessage(m_lUsers.front().c_str(), messageRaw.data(),
          bOnline, nLevel, false, &icqColor);

        m_lnEventTag.push_back(icqEventTag);

        tmp = gTranslator.NToRN(messageRaw);
        wholeMessagePos += strlen(tmp);
        delete [] tmp;
      }

      icqEventTag = 0;

      break;
    }
    case ICQ_CMDxSUB_URL:
    {
      CEventUrl *ue = (CEventUrl *)e->UserEvent();

      icqEventTag = server->ProtoSendUrl(m_lUsers.front().c_str(), m_nPPID, ue->Url(),
        ue->Description(), bOnline, nLevel, false, &icqColor);

      break;
    }
    case ICQ_CMDxSUB_CONTACTxLIST:
    {
      CEventContactList* ue = (CEventContactList *) e->UserEvent();
      const ContactList& clist = ue->Contacts();
      StringList users;

      // ContactList is const but string list holds "char*" so we have to copy each string
      for(ContactList::const_iterator i = clist.begin(); i != clist.end(); i++)
        users.push_back((*i)->IdString());

      if(users.size() == 0)
        break;

      icqEventTag = server->icqSendContactList(m_lUsers.front().c_str(),
          users, bOnline, nLevel, false, &icqColor);

      break;
    }
    case ICQ_CMDxSUB_CHAT:
    {
      CEventChat *ue = (CEventChat *)e->UserEvent();

      if (ue->Clients() == NULL)
        //TODO in the daemon
        icqEventTag = server->icqChatRequest(m_lUsers.front().c_str(),
          ue->Reason(), nLevel, !bOnline);
      else
        //TODO in the daemon
        icqEventTag = server->icqMultiPartyChatRequest(
            m_lUsers.front().c_str(), ue->Reason(), ue->Clients(),
              ue->Port(), nLevel, !bOnline);

      break;
    }
    case ICQ_CMDxSUB_FILE:
    {
      CEventFile *ue = (CEventFile *)e->UserEvent();
      ConstFileList filelist(ue->FileList());
      //TODO in the daemon
      icqEventTag = server->icqFileTransfer(m_lUsers.front().c_str(),
        ue->Filename(), ue->FileDescription(), filelist, nLevel, !bOnline);

      break;
    }
    case ICQ_CMDxSUB_SMS:
    {
      CEventSms *ue = (CEventSms *)e->UserEvent();
      //TODO in the daemon
      icqEventTag = server->icqSendSms(m_lUsers.front().c_str(), LICQ_PPID,
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
    m_lnEventTag.push_back(icqEventTag);

  UserSendCommon::sendButton();
}


//-----UserSendCommon::slot_close--------------------------------------------
void UserSendCommon::slot_close()
{
  server->ProtoTypingNotification(m_lUsers.front().c_str(), m_nPPID, false, m_nConvoId);
  
  if (mainwin->m_bMsgChatView)
  {
    // the window is at the front, if the timer has not expired and we close
    // the window, then the new events will stay there
    slot_ClearNewEvents();
  }

  if (mleSend)
    mainwin->m_bCheckSpellingEnabled = mleSend->checkSpellingEnabled();

  if (mainwin->userEventTabDlg &&
      mainwin->userEventTabDlg->tabExists(this))
    mainwin->userEventTabDlg->removeTab(this);
  else
    close();
}

//-----UserSendCommon::slot_cancelSend--------------------------------------------
void UserSendCommon::slot_cancelSend()
{
  unsigned long icqEventTag = 0;
  if (m_lnEventTag.size())
    icqEventTag = m_lnEventTag.front();

  if (!icqEventTag)
    return slot_close(); // if we're not sending atm, let ESC close the window

  if (mainwin->userEventTabDlg &&
      mainwin->userEventTabDlg->tabIsSelected(this))
    mainwin->userEventTabDlg->setCaption(m_sBaseTitle);

  server->CancelEvent(icqEventTag);
}


//-----UserSendCommon::UserUpdated-------------------------------------------
void UserSendCommon::UserUpdated(CICQSignal* sig, const char* szId, unsigned long nPPID)
{
  ICQUser *u = gUserManager.FetchUser(szId, nPPID, LOCK_R);
  if (!u) return;
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
      {
        chkSendServer->setEnabled(true);
      }
      if (u->StatusOffline())
        chkSendServer->setChecked(true);
      break;
    }
    case USER_EVENTS:
    {
      CUserEvent *e = u->EventPeekId(sig->Argument());
      if (e != NULL && m_highestEventId < sig->Argument() && mleHistory &&
          sig->Argument() > 0)
      {
        m_highestEventId = sig->Argument();
        e = u->EventPeekId(sig->Argument());
        if (e)
        {
          if (sig->PPID() != MSN_PPID || (sig->PPID() == MSN_PPID && sig->CID() == m_nConvoId))
          {
            gUserManager.DropUser(u);
            mleHistory->addMsg(e, szId, nPPID);
            return;
          }
        }
      }
      break;
    }
    case USER_SECURITY:
    {
      if (u->Secure())
      {
        u->SetSendServer(false);
        chkSendServer->setChecked(false);
      }
      break;
    }
  }
  gUserManager.DropUser(u);
}


//-----UserSendCommon::checkSecure-------------------------------------------
bool UserSendCommon::checkSecure()
{
  ICQUser *u = gUserManager.FetchUser(m_lUsers.front().c_str(), m_nPPID, LOCK_R);
  if (u == 0)
    return false;
 
  bool secure = u->Secure() || u->AutoSecure();
  bool send_ok = true;

  gUserManager.DropUser(u);

  if (chkSendServer->isChecked() && secure)
  {
    if (!QueryUser(this, tr("Warning: Message can't be sent securely\n"
                             "through the server!"),
                   tr("Send anyway"), tr("Cancel")))
    {
      send_ok = false;
    }
    else
    {
      ICQUser *u = gUserManager.FetchUser(m_lUsers.front().c_str(), m_nPPID, LOCK_R);
      if (u)
        u->SetAutoSecure(false);
      gUserManager.DropUser(u);
    }
  }
  return send_ok;
}

//=====UserSendMsgEvent======================================================
UserSendMsgEvent::UserSendMsgEvent(CICQDaemon *s, CSignalManager *theSigMan,
  CMainWindow *m, const char *szId, unsigned long nPPID, QWidget *parent)
  : UserSendCommon(s, theSigMan, m, szId, nPPID, parent, "UserSendMsgEvent")
{
  QBoxLayout* lay = new QVBoxLayout(mainWidget);
  lay->addWidget(splView);
  if (!m->m_bMsgChatView) mleSend->setMinimumHeight(150);
  mleSend->setFocus ();

  m_sBaseTitle += tr(" - Message");

  if (mainwin->userEventTabDlg &&
      mainwin->userEventTabDlg->tabIsSelected(this))
    mainwin->userEventTabDlg->setCaption(m_sBaseTitle);

  setCaption(m_sBaseTitle);
  cmbSendType->setCurrentItem(UC_MESSAGE);
}

UserSendMsgEvent::~UserSendMsgEvent()
{
}


//-----UserSendMsgEvent::sendButton------------------------------------------
void UserSendMsgEvent::sendButton()
{
  // Take care of typing notification now
  if (tmrSendTyping->isActive())
    tmrSendTyping->stop();
  connect(mleSend, SIGNAL(textChanged()), this, SLOT(slot_textChanged()));
  server->ProtoTypingNotification(m_lUsers.front().c_str(), m_nPPID, false, m_nConvoId);
  
  // do nothing if a command is already being processed
  unsigned long icqEventTag = 0;
  if (m_lnEventTag.size())
    icqEventTag = m_lnEventTag.front();

  if (icqEventTag != 0) return;

  if(!mleSend->isModified() &&
     !QueryUser(this, tr("You didn't edit the message.\n"
                         "Do you really want to send it?"), tr("&Yes"), tr("&No")))
    return;

  // don't let the user send empty messages
  if (mleSend->text().stripWhiteSpace().isEmpty()) return;

  if (!UserSendCommon::checkSecure()) return;

  ICQUser *u = gUserManager.FetchUser(m_lUsers.front().c_str(), m_nPPID, LOCK_R);
  bool bUserOffline = u->StatusOffline();
  gUserManager.DropUser(u);

  // create initial strings (implicit copying, no allocation impact :)
  char *tmp = gTranslator.NToRN(codec->fromUnicode(mleSend->text()));
  QCString wholeMessageRaw(tmp);
  delete [] tmp;
  unsigned int wholeMessagePos = 0;

  bool needsSplitting = false;
  // If we send through server (= have message limit), and we've crossed the limit
  unsigned short nMaxSize = bUserOffline ? MAX_OFFLINE_MESSAGE_SIZE : MAX_MESSAGE_SIZE;
  if (chkSendServer->isChecked() && ((wholeMessageRaw.length() - wholeMessagePos) > nMaxSize))
    needsSplitting = true;

  QString message;
  QCString messageRaw;

  while (wholeMessageRaw.length() > wholeMessagePos)
  {
     if (needsSplitting)
     {
        // This is a bit ugly but adds safety. We don't simply search
        // for a whitespace to cut at in the encoded text (since we don't
        // really know how spaces are represented in its encoding), so
        // we take the maximum length, then convert back to a Unicode string
        // and then search for Unicode whitespaces.
        messageRaw = wholeMessageRaw.mid(wholeMessagePos, nMaxSize);
        tmp = gTranslator.RNToN(messageRaw);
        messageRaw = tmp;
        delete [] tmp;
        message = codec->toUnicode(messageRaw);

        if ((wholeMessageRaw.length() - wholeMessagePos) > nMaxSize)
        {
           // We try to find the optimal place to cut
           // (according to our narrow-minded Latin1 idea of optimal :)
           // prefer keeping sentences intact 1st
           int foundIndex = message.findRev(QRegExp("[\\.\\n]"));
           // slicing at 0 position would be useless
           if (foundIndex <= 0)
             foundIndex = message.findRev(QRegExp("\\s"));

           if (foundIndex > 0)
           {
              message.truncate(foundIndex + 1);
              messageRaw = codec->fromUnicode(message);
           }
        }
     }
     else
     {
        message = mleSend->text();
        messageRaw = codec->fromUnicode(message);
     }

     if (chkMass->isChecked())
     {
        CMMSendDlg *m = new CMMSendDlg(server, sigman, lstMultipleRecipients, this);
        m->go_message(message);
     }

     icqEventTag = server->ProtoSendMessage(m_lUsers.front().c_str(), m_nPPID, messageRaw.data(),
      chkSendServer->isChecked() ? false : true,
      chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL,
      chkMass->isChecked(), &icqColor, m_nConvoId);
     if (m_nPPID == LICQ_PPID)
       m_lnEventTag.push_back(icqEventTag);

     tmp = gTranslator.NToRN(messageRaw);
     wholeMessagePos += strlen(tmp);
     delete [] tmp;
  }

  UserSendCommon::sendButton();
}


//-----UserSendMsgEvent::sendDone--------------------------------------------
bool UserSendMsgEvent::sendDone(ICQEvent* /* e */)
{
  mleSend->setText(QString::null);

  ICQUser *u = gUserManager.FetchUser(m_lUsers.front().c_str(), m_nPPID, LOCK_R);
  if (u->Away() && u->ShowAwayMsg())
  {
    gUserManager.DropUser(u);
    (void) new ShowAwayMsgDlg(NULL, NULL, m_lUsers.front().c_str(), m_nPPID);
  }
  else
    gUserManager.DropUser(u);

  return true;

}

void UserSendMsgEvent::resetSettings()
{
  mleSend->clear();
  mleSend->setFocus();

  // Makes the cursor blink so that the user sees that the text edit has focus.
  mleSend->moveCursor(QTextEdit::MoveHome, false);

  massMessageToggled( false );
}

UserSendUrlEvent::UserSendUrlEvent(CICQDaemon *s, CSignalManager *theSigMan,
                                   CMainWindow *m, const char *szId,
                                   unsigned long nPPID, QWidget* parent)
  : UserSendCommon(s, theSigMan, m, szId, nPPID, parent, "UserSendUrlEvent")
{
  QBoxLayout* lay = new QVBoxLayout(mainWidget, 4);
  lay->addWidget(splView);
  mleSend->setFocus ();

  QBoxLayout* h_lay = new QHBoxLayout(lay);
  lblItem = new QLabel(tr("URL : "), mainWidget);
  h_lay->addWidget(lblItem);
  edtItem = new CInfoField(mainWidget, false);
  h_lay->addWidget(edtItem);
  edtItem->installEventFilter(this);

  m_sBaseTitle += tr(" - URL");

  if (mainwin->userEventTabDlg &&
      mainwin->userEventTabDlg->tabIsSelected(this))
    mainwin->userEventTabDlg->setCaption(m_sBaseTitle);

  setCaption(m_sBaseTitle);
  cmbSendType->setCurrentItem(UC_URL);
}

UserSendUrlEvent::~UserSendUrlEvent()
{
}

bool UserSendUrlEvent::eventFilter(QObject *watched, QEvent *e)
{
  if (watched == edtItem)
  {
    if (e->type() == QEvent::KeyPress)
    {
      QKeyEvent *key = static_cast<QKeyEvent*>(e);
      const bool isEnter = (key->key() == Key_Enter || key->key() == Key_Return);
      if (isEnter && (mainwin->m_bSingleLineChatMode || key->state() & ControlButton))
      {
        btnSend->animateClick();
        return true; // filter the event out
      }
    }
    return false;
  }
  else
    return UserSendCommon::eventFilter(watched, e);
}

void UserSendUrlEvent::setUrl(const QString& url, const QString& description)
{
  edtItem->setText(url);
  setText(description);
}

void UserSendUrlEvent::resetSettings()
{
  mleSend->clear();
  edtItem->clear();
  mleSend->setFocus();
  massMessageToggled( false );
}


//-----UserSendUrlEvent::sendButton------------------------------------------
void UserSendUrlEvent::sendButton()
{
  // Take care of typing notification now
  tmrSendTyping->stop();
  connect(mleSend, SIGNAL(textChanged()), this, SLOT(slot_textChanged()));
  server->ProtoTypingNotification(m_lUsers.front().c_str(), m_nPPID, false, m_nConvoId);
  
  if (edtItem->text().stripWhiteSpace().isEmpty())
  {
    InformUser(this, tr("No URL specified"));
    return;
  }

  if (!UserSendCommon::checkSecure())
    return;

  if (chkMass->isChecked())
  {
    CMMSendDlg *m = new CMMSendDlg(server, sigman, lstMultipleRecipients, this);
    int r = m->go_url(edtItem->text(), mleSend->text());
    delete m;
    if (r != QDialog::Accepted) return;
  }

  unsigned long icqEventTag;
  icqEventTag = server->ProtoSendUrl(m_lUsers.front().c_str(), m_nPPID, edtItem->text().latin1(), codec->fromUnicode(mleSend->text()),
     chkSendServer->isChecked() ? false : true,
     chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL,
     chkMass->isChecked(), &icqColor);

  m_lnEventTag.push_back(icqEventTag);

  UserSendCommon::sendButton();
}


//-----UserSendUrlEvent::sendDone--------------------------------------------
bool UserSendUrlEvent::sendDone(ICQEvent *e)
{
  if (e->Command() != ICQ_CMDxTCP_START) return true;

  ICQUser *u = gUserManager.FetchUser(m_lUsers.front().c_str(), m_nPPID, LOCK_R);
  if (u->Away() && u->ShowAwayMsg())
  {
    gUserManager.DropUser(u);
    (void) new ShowAwayMsgDlg(NULL, NULL, m_lUsers.front().c_str(), m_nPPID);
  }
  else
    gUserManager.DropUser(u);

  return true;
}

UserSendFileEvent::UserSendFileEvent(CICQDaemon *s, CSignalManager *theSigMan,
                                     CMainWindow *m, const char *szId,
                                     unsigned long nPPID, QWidget* parent)
  : UserSendCommon(s, theSigMan, m, szId, nPPID, parent, "UserSendFileEvent")
{
  chkMass->setChecked(false);
  chkMass->setEnabled(false);
  btnForeColor->setEnabled(false);
  btnBackColor->setEnabled(false);

  QBoxLayout* lay = new QVBoxLayout(mainWidget, 4);
  lay->addWidget(splView);

  QBoxLayout* h_lay = new QHBoxLayout(lay);
  lblItem = new QLabel(tr("File(s): "), mainWidget);
  h_lay->addWidget(lblItem);

  edtItem = new CInfoField(mainWidget, false);
  edtItem->SetReadOnly(true);
  h_lay->addWidget(edtItem);

  btnBrowse = new QPushButton(tr("Browse"), mainWidget);
  connect(btnBrowse, SIGNAL(clicked()), this, SLOT(browseFile()));
  h_lay->addWidget(btnBrowse);

  btnEdit = new QPushButton(tr("Edit"), mainWidget);
  btnEdit->setEnabled(false);
  connect(btnEdit,  SIGNAL(clicked()), this, SLOT(editFileList()));
  h_lay->addWidget(btnEdit);

  m_sBaseTitle += tr(" - File Transfer");

  if (mainwin->userEventTabDlg &&
      mainwin->userEventTabDlg->tabIsSelected(this))
    mainwin->userEventTabDlg->setCaption(m_sBaseTitle);

  setCaption(m_sBaseTitle);
  cmbSendType->setCurrentItem(UC_FILE);
}

void UserSendFileEvent::browseFile()
{
#ifdef USE_KDE
  QStringList fl = KFileDialog::getOpenFileNames(NULL, NULL, this);
#else
  QStringList fl = QFileDialog::getOpenFileNames(QString::null, QString::null, this,
                                                 "SendFileBrowser", tr("Select files to send"));
#endif

  if (fl.isEmpty()) return;
  QStringList::ConstIterator it = fl.begin();
  QString f;
  unsigned n = fl.count() + m_lFileList.size();

  if ( n == 0 )
  {
    btnEdit->setEnabled(false);
    f = QString("");
  }
  else if( n == 1 )
  {
    btnEdit->setEnabled(true);
    f = (*it);
  }
  else
  {
    f = QString("%1 Files").arg(fl.count() + m_lFileList.size() );
    btnEdit->setEnabled(true);
  }

  for(; it != fl.end(); it++)
    m_lFileList.push_back(strdup((*it).local8Bit()));

  edtItem->setText(f);
}

void UserSendFileEvent::addFile(const QString &file)
{
  if (m_lFileList.size() == 0) return;
  
  m_lFileList.push_back(strdup(file.local8Bit()));
  
  btnEdit->setEnabled(true);
  QString f = QString("%1 Files").arg(m_lFileList.size());
  edtItem->setText(f);
}

void UserSendFileEvent::editFileList()
{
  CEditFileListDlg *dlg;

  dlg = new CEditFileListDlg(&m_lFileList);

  connect(dlg,SIGNAL(file_deleted(unsigned)), this, SLOT(slot_filedel(unsigned)));
}

void UserSendFileEvent::slot_filedel(unsigned n)
{
  QString f;

  if (n == 0)
  {
    f = QString("");
    btnEdit->setEnabled(false);
  }
  else if (n == 1)
  {
    f = *(m_lFileList.begin());
    btnEdit->setEnabled(true);
  }
  else
  {
    f = QString("%1 Files").arg(m_lFileList.size());
    btnEdit->setEnabled(true);
  }

  edtItem->setText(f);
}

UserSendFileEvent::~UserSendFileEvent()
{
}


void UserSendFileEvent::sendButton()
{
  // Take care of typing notification now
  tmrSendTyping->stop();
  connect(mleSend, SIGNAL(textChanged()), this, SLOT(slot_textChanged()));
  server->ProtoTypingNotification(m_lUsers.front().c_str(), m_nPPID, false, m_nConvoId);
  
  if (edtItem->text().stripWhiteSpace().isEmpty())
  {
    WarnUser(this, tr("You must specify a file to transfer!"));
    return;
  }

  unsigned long icqEventTag;
  //TODO in daemon
  icqEventTag = server->icqFileTransfer(m_lUsers.front().c_str(),
     codec->fromUnicode(edtItem->text()),
     codec->fromUnicode(mleSend->text()), m_lFileList,
     chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL,
     chkSendServer->isChecked());

  m_lnEventTag.push_back(icqEventTag);

  UserSendCommon::sendButton();
}

void UserSendFileEvent::setFile(const QString& file, const QString& description)
{
  QFileInfo fileinfo(file);
  if (fileinfo.exists() && fileinfo.isFile() && fileinfo.isReadable())
  {
    edtItem->setText(file);
    setText(description);
    m_lFileList.push_back(strdup(file.local8Bit()));
    btnEdit->setEnabled(true);
  }
}

void UserSendFileEvent::resetSettings()
{
  mleSend->clear();
  edtItem->clear();
  mleSend->setFocus();
  m_lFileList.clear();
  btnEdit->setEnabled(false);
  massMessageToggled( false );
}

//-----UserSendFileEvent::sendDone-------------------------------------------
bool UserSendFileEvent::sendDone(ICQEvent *e)
{
  if (!e->ExtendedAck() || !e->ExtendedAck()->Accepted())
  {
    ICQUser *u = gUserManager.FetchUser(m_lUsers.front().c_str(), m_nPPID, LOCK_R);
    QString s(!e->ExtendedAck() ? tr("No reason provided") :
                               codec->toUnicode(e->ExtendedAck()->Response()));
    QString result = tr("File transfer with %2 refused:\n%3")
                       .arg(QString::fromUtf8(u->GetAlias()))
                       .arg(s);
    gUserManager.DropUser(u);
    InformUser(this, result);
  }
  else
  {
    CEventFile *f = (CEventFile *)e->UserEvent();
    CFileDlg *fileDlg = new CFileDlg(m_lUsers.front().c_str(), m_nPPID, server);
    fileDlg->SendFiles(f->FileList(), e->ExtendedAck()->Port());
  }

  return true;
}


//=====UserSendChatEvent=====================================================
UserSendChatEvent::UserSendChatEvent(CICQDaemon *s, CSignalManager *theSigMan,
                                     CMainWindow *m, const char *szId,
                                     unsigned long nPPID, QWidget* parent)
  : UserSendCommon(s, theSigMan, m, szId, nPPID, parent, "UserSendChatEvent")
{
  m_nMPChatPort = 0;
  chkMass->setChecked(false);
  chkMass->setEnabled(false);
  btnForeColor->setEnabled(false);
  btnBackColor->setEnabled(false);

  QBoxLayout *lay = new QVBoxLayout(mainWidget, 9);
  lay->addWidget(splView);

  if (!m->m_bMsgChatView) mleSend->setMinimumHeight(150);

  QBoxLayout* h_lay = new QHBoxLayout(lay);
  lblItem = new QLabel(tr("Multiparty: "), mainWidget);
  h_lay->addWidget(lblItem);

  edtItem = new CInfoField(mainWidget, false);
  h_lay->addWidget(edtItem);

  btnBrowse = new QPushButton(tr("Invite"), mainWidget);
  connect(btnBrowse, SIGNAL(clicked()), this, SLOT(InviteUser()));
  h_lay->addWidget(btnBrowse);

  m_sBaseTitle += tr(" - Chat Request");

  if (mainwin->userEventTabDlg &&
      mainwin->userEventTabDlg->tabIsSelected(this))
    mainwin->userEventTabDlg->setCaption(m_sBaseTitle);

  setCaption(m_sBaseTitle);
  cmbSendType->setCurrentItem(UC_CHAT);
}

UserSendChatEvent::~UserSendChatEvent()
{
}

void UserSendChatEvent::InviteUser()
{
  if (m_nMPChatPort == 0)
  {
    if (ChatDlg::chatDlgs.size() > 0)
    {
      ChatDlg *chatDlg = NULL;
      CJoinChatDlg *j = new CJoinChatDlg(true, this);
      if (j->exec() && (chatDlg = j->JoinedChat()) != NULL)
      {
        edtItem->setText(j->ChatClients());
        m_nMPChatPort = chatDlg->LocalPort();
        m_szMPChatClients = chatDlg->ChatName() + ", " + chatDlg->ChatClients();
      }
      delete j;
      btnBrowse->setText(tr("Clear"));
    }
  }
  else
  {
    m_nMPChatPort = 0;
    m_szMPChatClients = "";
    edtItem->setText("");
    btnBrowse->setText(tr("Invite"));
  }
}

void UserSendChatEvent::resetSettings()
{
  mleSend->clear();
  edtItem->clear();
  mleSend->setFocus();
  massMessageToggled( false );
}

//-----UserSendChatEvent::sendButton-----------------------------------------
void UserSendChatEvent::sendButton()
{
  // Take care of typing notification now
  tmrSendTyping->stop();
  connect(mleSend, SIGNAL(textChanged()), this, SLOT(slot_textChanged()));
  server->ProtoTypingNotification(m_lUsers.front().c_str(), m_nPPID, false, m_nConvoId);
  
  unsigned long icqEventTag;

  if (m_nMPChatPort == 0)
    //TODO in daemon
    icqEventTag = server->icqChatRequest(m_lUsers.front().c_str(),
                                         codec->fromUnicode(mleSend->text()),
                                         chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL,
                                         chkSendServer->isChecked());
  else
    icqEventTag = server->icqMultiPartyChatRequest(m_lUsers.front().c_str(),
                                                   codec->fromUnicode(mleSend->text()),
                                                   codec->fromUnicode(m_szMPChatClients),
                                                   m_nMPChatPort,
                                                   chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL,
                                                   chkSendServer->isChecked() );

  m_lnEventTag.push_back(icqEventTag);

  UserSendCommon::sendButton();
}


//-----UserSendChatEvent::sendDone-------------------------------------------
bool UserSendChatEvent::sendDone(ICQEvent *e)
{
  if (!e->ExtendedAck() || !e->ExtendedAck()->Accepted())
  {
    ICQUser *u = gUserManager.FetchUser(m_lUsers.front().c_str(), m_nPPID, LOCK_R);
    QString s(!e->ExtendedAck() ? tr("No reason provided") :
                               codec->toUnicode(e->ExtendedAck()->Response()));
    QString result = tr("Chat with %2 refused:\n%3")
                       .arg(QString::fromUtf8(u->GetAlias()))
                       .arg(s);
    gUserManager.DropUser(u);
    InformUser(this, result);
  }
  else
  {
    CEventChat *c = (CEventChat *)e->UserEvent();
    if (c->Port() == 0)  // If we requested a join, no need to do anything
    {
      ChatDlg *chatDlg = new ChatDlg(m_lUsers.front().c_str(), m_nPPID, server, mainwin);
      chatDlg->StartAsClient(e->ExtendedAck()->Port());
    }
  }

  return true;
}


//=====UserSendContactEvent==================================================
UserSendContactEvent::UserSendContactEvent(CICQDaemon *s, CSignalManager *theSigMan,
                                           CMainWindow *m, const char *szId,
                                           unsigned long nPPID, QWidget* parent)
  : UserSendCommon(s, theSigMan, m, szId, nPPID, parent, "UserSendContactEvent")
{
  chkMass->setChecked(false);
  chkMass->setEnabled(false);
  btnForeColor->setEnabled(false);
  btnBackColor->setEnabled(false);
  btnEmoticon->setEnabled(false);

  delete mleSend; mleSend = NULL;

  QBoxLayout* lay = new QVBoxLayout(mainWidget);
  lay->addWidget(splView);
  QLabel* lblContact =  new QLabel(tr("Drag Users Here - Right Click for Options"), mainWidget);
  lay->addWidget(lblContact);

  lstContacts = new CMMUserView(mainwin->colInfo, mainwin->m_bShowHeader,
                                const_cast<char *>(m_lUsers.front().c_str()), nPPID, mainwin, mainWidget);
  lay->addWidget(lstContacts);

  m_sBaseTitle += tr(" - Contact List");

  if (mainwin->userEventTabDlg &&
      mainwin->userEventTabDlg->tabIsSelected(this))
    mainwin->userEventTabDlg->setCaption(m_sBaseTitle);

  setCaption(m_sBaseTitle);
  cmbSendType->setCurrentItem(UC_CONTACT);
}

UserSendContactEvent::~UserSendContactEvent()
{
}


//TODO Fix this for new protocol plugin
void UserSendContactEvent::sendButton()
{
  // Take care of typing notification now
  tmrSendTyping->stop();
  server->ProtoTypingNotification(m_lUsers.front().c_str(), m_nPPID, false, m_nConvoId);

  CMMUserViewItem *i = static_cast<CMMUserViewItem*>(lstContacts->firstChild());
  StringList users;

  while (i)
  {
    users.push_back(i->Id());
    i = static_cast<CMMUserViewItem *>(i->nextSibling());
  }

  if (users.size() == 0)
    return;

  if (!UserSendCommon::checkSecure()) return;

  if (chkMass->isChecked())
  {
    CMMSendDlg *m = new CMMSendDlg(server, sigman, lstMultipleRecipients, this);
    int r = m->go_contact(users);
    delete m;
    if (r != QDialog::Accepted) return;
  }

  unsigned long icqEventTag;
  icqEventTag = server->icqSendContactList(m_lUsers.front().c_str(), users,
    chkSendServer->isChecked() ? false : true,
    chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL,
    chkMass->isChecked(), &icqColor);
  m_lnEventTag.push_back(icqEventTag);

  UserSendCommon::sendButton();
}

void UserSendContactEvent::resetSettings()
{
  lstContacts->clear();
  massMessageToggled( false );
}

//-----UserSendContactEvent::sendDone------------------------------------------
bool UserSendContactEvent::sendDone(ICQEvent *e)
{
  if (e->Command() != ICQ_CMDxTCP_START) return true;

  ICQUser *u = gUserManager.FetchUser(m_lUsers.front().c_str(), m_nPPID, LOCK_R);
  if (u->Away() && u->ShowAwayMsg())
  {
    gUserManager.DropUser(u);
    (void) new ShowAwayMsgDlg(NULL, NULL, m_lUsers.front().c_str(), m_nPPID);
  }
  else
    gUserManager.DropUser(u);

  return true;
}


//-----UserSendContactEvent::setContact--------------------------------------
void UserSendContactEvent::setContact(const char *szId, unsigned long nPPID,
  const QString&)
{
  ICQUser* u = gUserManager.FetchUser(szId, nPPID, LOCK_R);

  if(u != NULL)
  {
    (void) new CMMUserViewItem(u, lstContacts);
    gUserManager.DropUser(u);
  }
}


//=====UserSendSmsEvent======================================================
UserSendSmsEvent::UserSendSmsEvent(CICQDaemon *s, CSignalManager *theSigMan,
  CMainWindow *m, const char *szId, unsigned long nPPID, QWidget *parent)
  : UserSendCommon(s, theSigMan, m, szId, nPPID, parent, "UserSendSmsEvent")
{
  chkSendServer->setChecked(true);
  chkSendServer->setEnabled(false);
  chkUrgent->setChecked(false);
  chkUrgent->setEnabled(false);
  chkMass->setChecked(false);
  chkMass->setEnabled(false);
  btnForeColor->setEnabled(false);
  btnBackColor->setEnabled(false);
  btnEncoding->setEnabled(false); // SMSs are always UTF-8

  QBoxLayout* lay = new QVBoxLayout(mainWidget, 4);
  lay->addWidget(splView);
  mleSend->setFocus();

  QBoxLayout* h_lay = new QHBoxLayout(lay);
  lblNumber = new QLabel(tr("Phone : "), mainWidget);
  h_lay->addWidget(lblNumber);
  nfoNumber = new CInfoField(mainWidget, false);
  h_lay->addWidget(nfoNumber);
  nfoNumber->setFixedWidth(QMAX(140, nfoNumber->sizeHint().width()));
  h_lay->addStretch(1);
  lblCount = new QLabel(tr("Chars left : "), mainWidget);
  h_lay->addWidget(lblCount);
  nfoCount = new CInfoField(mainWidget, false);
  h_lay->addWidget(nfoCount);
  nfoCount->setFixedWidth(40);
  nfoCount->setAlignment(AlignCenter);
  slot_count();
  connect(mleSend, SIGNAL(textChanged()), this, SLOT(slot_count()));

  ICQUser *u = gUserManager.FetchUser(m_lUsers.front().c_str(), m_nPPID, LOCK_W);
  if (u != NULL)
  {
    nfoNumber->setData(codec->toUnicode(u->GetCellularNumber()));
    gUserManager.DropUser(u);
  }

  m_sBaseTitle += tr(" - SMS");

  if (mainwin->userEventTabDlg &&
      mainwin->userEventTabDlg->tabIsSelected(this))
    mainwin->userEventTabDlg->setCaption(m_sBaseTitle);

  setCaption(m_sBaseTitle);
  cmbSendType->setCurrentItem(UC_SMS);
}

//-----UserSendSmsEvent::~UserSendSmsEvent-------------------------------------
UserSendSmsEvent::~UserSendSmsEvent()
{
}

//-----UserSendSmsEvent::sendButton--------------------------------------------
void UserSendSmsEvent::sendButton()
{
  // Take care of typing notification now
  tmrSendTyping->stop();
  connect(mleSend, SIGNAL(textChanged()), this, SLOT(slot_textChanged()));
  server->ProtoTypingNotification(m_lUsers.front().c_str(), m_nPPID, false, m_nConvoId);
  
  unsigned long icqEventTag = 0;
  if (m_lnEventTag.size())
    icqEventTag = m_lnEventTag.front();

  // do nothing if a command is already being processed
  if (icqEventTag != 0)
    return;

  if(!mleSend->isModified() &&
     !QueryUser(this, tr("You didn't edit the SMS.\n"
                         "Do you really want to send it?"), tr("&Yes"), tr("&No")))
    return;

  // don't let the user send empty messages
  if (mleSend->text().stripWhiteSpace().isEmpty())
    return;

  //TODO in daemon
  icqEventTag = server->icqSendSms(m_lUsers.front().c_str(), LICQ_PPID,
      nfoNumber->text().latin1(), mleSend->text().utf8().data());
  m_lnEventTag.push_back(icqEventTag);

  UserSendCommon::sendButton();
}

//-----UserSendSmsEvent::resetSettings-----------------------------------------
void UserSendSmsEvent::resetSettings()
{
  mleSend->clear();
  mleSend->setFocus();
  massMessageToggled(false);
}

//-----UserSendSmsEvent::sendDone----------------------------------------------
bool UserSendSmsEvent::sendDone(ICQEvent* /* e */)
{
  return true;
}

//-----UserSendSmsEvent::slot_count--------------------------------------------
void UserSendSmsEvent::slot_count()
{
  int len = 160 - strlen(mleSend->text().utf8().data());
  nfoCount->setData((len >= 0) ? len : 0);
}

//=====EmoticonLabel===========================================================
EmoticonLabel::EmoticonLabel(const QString &file, const QString &key,
                             QWidget *parent)
  : QLabel(parent)
{
  m_sKey = key;
  
  QImage img = QImage(file);
  QPixmap pm(img);  
  setPixmap(pm);
}

//-----EmoticonLabel::mouseReleaseEvent----------------------------------------
void EmoticonLabel::mouseReleaseEvent(QMouseEvent *)
{
  emit clicked(m_sKey);
}

//=====SelectEmoticon==========================================================
SelectEmoticon::SelectEmoticon(QWidget *parent)
  : QFrame(parent, "SelectEmoticon", WType_Popup | WStyle_Tool | 
      WStyle_Customize | WDestructiveClose)
{
  QMap<QString, QString> map = CEmoticons::self()->emoticonsKeys();
  QMapIterator<QString, QString> iter;
  
  int nRows = static_cast<int>(::sqrt(map.size()));
  double d = ::sqrt(map.size());
  if (d - (double)nRows != 0)
    nRows++;
  int nCols = nRows;
  
  QGridLayout *grid = new QGridLayout(this, nRows, nCols);
  grid->setSpacing(3);
  grid->setMargin(4);
  int x = 0, y = 0;
  for (iter = map.begin(); iter != map.end(); ++iter)
  {
    EmoticonLabel *lbl = new EmoticonLabel(iter.key(), iter.data(), this);
    connect(lbl, SIGNAL(clicked(const QString &)), this,
      SLOT(emoticonClicked(const QString &)));
    
    grid->addWidget(lbl, x++, y);
    if (x == nCols)
    {
      x = 0;
      y++;
    }  
  }

  setFrameShadow(Sunken);
  setFrameShape(PopupPanel);
}

//-----SelectEmoticon::emoticonClicked-----------------------------------------
void SelectEmoticon::emoticonClicked(const QString &sKey)
{
  selected(sKey);
  close();
}

// ----------------------------------------------------------------------------

#include "usereventdlg.moc"
