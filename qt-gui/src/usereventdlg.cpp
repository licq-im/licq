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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>
#include <qaccel.h>
#include <qcheckbox.h>
#include <qdatetime.h>
#include <qvbox.h>
#include <qvgroupbox.h>
#include <qhgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qsplitter.h>
#include <qapplication.h>
#include <qpopupmenu.h>
#include <qtextcodec.h>
#include <qwhatsthis.h>
#include <qtabwidget.h>

#ifdef USE_KDE
#include <kfiledialog.h>
#include <kcolordialog.h>
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
#include "keyrequestdlg.h"
#include "editfilelistdlg.h"
#include "xpm/chatChangeFg.xpm"
#include "xpm/chatChangeBg.xpm"

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
  m_szId = _szId ? strdup(_szId) : 0;
  m_nPPID = _nPPID;
  m_bOwner = (gUserManager.FindOwner(m_szId, m_nPPID) != NULL);
  m_bDeleteUser = false;

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

  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_W);
  if (u != NULL)
  {
    nfoStatus->setData(u->StatusStr());
    if (u->NewMessages() == 0)
      setIcon(CMainWindow::iconForStatus(u->StatusFull(), u->IdString(), u->PPID()));
    else
      setIcon(CMainWindow::iconForEvent(ICQ_CMDxSUB_MSG));
    SetGeneralInfo(u);

    // restore prefered encoding
    codec = UserCodec::codecForICQUser(u);

    gUserManager.DropUser(u);
  }

  QString codec_name = QString::fromLatin1( codec->name() ).lower();
  popupEncoding->setCheckable(true);

  // populate the popup menu
  UserCodec::encoding_t *it = &UserCodec::m_encodings[0];
  while(it->encoding != NULL) {

    if (QString::fromLatin1(it->encoding).lower() == codec_name) {
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
}

void UserEventCommon::slot_connectsignal()
{
  connect (sigman, SIGNAL(signal_updatedUser(CICQSignal *)),
           this, SLOT(slot_userupdated(CICQSignal *)));
}

UserEventTabDlg::UserEventTabDlg(QWidget *parent, const char *name)
  : QWidget(parent, name, WDestructiveClose)
{
#if QT_VERSION >= 300
  QBoxLayout *lay = new QVBoxLayout(this);
  tabw = new CETabWidget(this);
  lay->addWidget(tabw);
  connect(tabw, SIGNAL(currentChanged(QWidget *)),
          this, SLOT(slot_currentChanged(QWidget *)));
#endif
}

UserEventTabDlg::~UserEventTabDlg()
{
#if QT_VERSION >= 300
  emit signal_done();
#endif
}

void UserEventTabDlg::addTab(UserEventCommon *tab, int index)
{
#if QT_VERSION >= 300
  QString label;
  ICQUser *u = gUserManager.FetchUser(tab->Id(), tab->PPID(), LOCK_W);
  if (u == NULL) return;

  // initalize codec
  QTextCodec *codec = QTextCodec::codecForLocale();
  label = codec->toUnicode(u->GetAlias());
  tabw->insertTab(tab, label, index);
  updateTabLabel(u);
  gUserManager.DropUser(u);
  tabw->showPage(tab);
#endif
}

void UserEventTabDlg::removeTab(QWidget *tab)
{
#if QT_VERSION >= 300
  if (tabw->count() > 1)
  {
    tabw->removePage(tab);
    tab->close();
    tab->setEnabled(false);
    tab->deleteLater();
  }
  else
    close();
#endif
}

void UserEventTabDlg::selectTab(QWidget *tab)
{
#if QT_VERSION >= 300
  tabw->showPage(tab);
  updateTitle(tab);
#endif
}

void UserEventTabDlg::replaceTab(QWidget *oldTab,
				 UserEventCommon *newTab)
{
#if QT_VERSION >= 300
  addTab(newTab, tabw->indexOf(oldTab));
  removeTab(oldTab);
#endif
}

bool UserEventTabDlg::tabIsSelected(QWidget *tab)
{
#if QT_VERSION >= 300
  if (tabw->currentPageIndex() == tabw->indexOf(tab))
    return true;
  else
    return false;
#else
  return false; //should never be used with QT < 3!
#endif
}

bool UserEventTabDlg::tabExists(QWidget *tab)
{
#if QT_VERSION >= 300
  if (tabw->indexOf(tab) != -1)
    return true;
  else return false;
#else
  return false; //should never be used with QT < 3!
#endif
}

void UserEventTabDlg::updateTabLabel(ICQUser *u)
{
#if QT_VERSION >= 300
  for (int index = 0; index < tabw->count(); index++)
  {
    UserEventCommon *tab = static_cast<UserEventCommon*>(tabw->page(index));
    if (strcmp(tab->Id(), u->IdString()) == 0 && tab->PPID() == u->PPID())
    {
      if (u->NewMessages() > 0)
      {
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
        tabw->setTabIconSet(tab, CMainWindow::iconForStatus(u->StatusFull(), u->IdString(), u->PPID()));
        if (u->GetTyping() == ICQ_TYPING_ACTIVE)
          tabw->setTabColor(tab, QColor("green"));
        else
          tabw->setTabColor(tab, QColor("black"));
      }
      return;
    }
  }
#endif
}

void UserEventTabDlg::gotTyping(ICQUser *u)
{
#if QT_VERSION >= 300
  for (int index = 0; index < tabw->count(); index++)
  {
    UserEventCommon *tab = static_cast<UserEventCommon*>(tabw->page(index));
    if (strcmp(tab->Id(), u->IdString()) == 0 && tab->PPID() == u->PPID())
    {
      tab->gotTyping(u->GetTyping());
    }
  }
#endif
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
#if QT_VERSION >= 300
  if (tab->caption())
    setCaption(tab->caption());
#endif
}

void UserEventTabDlg::clearEvents(QWidget *tab)
{
#if QT_VERSION >= 300
  if (!isActiveWindow()) return;
  UserSendCommon *e = static_cast<UserSendCommon*>(tab);
  QTimer::singleShot(e->clearDelay, e, SLOT(slot_ClearNewEvents()));
#endif
}

void UserEventTabDlg::moveLeft()
{
#if QT_VERSION >= 300
  int index = tabw->currentPageIndex();
  if (index > 0)
    tabw->setCurrentPage(index - 1);
  else // leftmost tab is selected, rotate!
    tabw->setCurrentPage(tabw->count() - 1);
#endif
}

void UserEventTabDlg::moveRight()
{
#if QT_VERSION >= 300
  int index = tabw->currentPageIndex();
  if (index < tabw->count() - 1)
    tabw->setCurrentPage(index + 1);
  else // rightmost tab is selected, rotate!
    tabw->setCurrentPage(0);
#endif
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
    ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_W);
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
  m_sBaseTitle = codec->toUnicode(u->GetAlias()) + tmp;
#if QT_VERSION >= 300
  if (mainwin->userEventTabDlg &&
      mainwin->userEventTabDlg->tabIsSelected(this))
    mainwin->userEventTabDlg->setCaption(m_sBaseTitle);
#endif
  setCaption(m_sBaseTitle);
  setIconText(codec->toUnicode(u->GetAlias()));
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
  emit finished(m_szId, m_nPPID);

  if (m_bDeleteUser && !m_bOwner)
    mainwin->RemoveUserFromList(strdup(m_szId), m_nPPID, this);

  free(m_szId);
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
  if (m_nPPID == LICQ_PPID) return;

  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_W);
  u->SetTyping(ICQ_TYPING_INACTIVEx0);
  nfoStatus->unsetPalette();
  if (mainwin->m_bTabbedChatting && mainwin->userEventTabDlg)
    mainwin->userEventTabDlg->updateTabLabel(u);
  gUserManager.DropUser(u);  
}

//-----UserEventCommon::slot_userupdated-------------------------------------
void UserEventCommon::slot_userupdated(CICQSignal *sig)
{
  if (m_nPPID != sig->PPID() || strcmp(m_szId, sig->Id()) != 0) return;

  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
  if (u == NULL) return;

  switch (sig->SubSignal())
  {
    case USER_STATUS:
    {
      nfoStatus->setData(u->StatusStr());
      if (u->NewMessages() == 0)
        setIcon(CMainWindow::iconForStatus(u->StatusFull(), u->IdString(), u->PPID()));
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
      break;
    }
  }
  // Call the event specific function now
  UserUpdated(sig, u);

  gUserManager.DropUser(u);
}


void UserEventCommon::showHistory()
{
  mainwin->callInfoTab(mnuUserHistory, m_szId, m_nPPID, true);
}


void UserEventCommon::showUserInfo()
{
  mainwin->callInfoTab(mnuUserGeneral, m_szId, m_nPPID, true);
}

void UserEventCommon::slot_security()
{
  (void) new KeyRequestDlg(sigman, m_szId, m_nPPID);
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
#if QT_VERSION < 300
  mlvRead->setFormatQuoted(true);
#else
  connect(mlvRead, SIGNAL(viewurl(QWidget*, QString)), mainwin, SLOT(slot_viewurl(QWidget *, QString)));
#endif
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

  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
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

  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
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
#if QT_VERSION < 300
  mlvRead->setText(m_messageText);
#else
  // Looks like there's no other way to set color in Qt 3's RichText control
  QString colorAttr;
  colorAttr.sprintf(QString::fromLatin1("#%02x%02x%02x"), m->Color()->ForeRed(), m->Color()->ForeGreen(), m->Color()->ForeBlue());
  mlvRead->setText("<font color=\"" + colorAttr + "\">" + MLView::toRichText(m_messageText, true) + "</font>");
#endif
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
  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_W);
  u->EventClearId(m_xCurrentReadEvent->Id());
  gUserManager.DropUser(u);
}

void UserViewEvent::generateReply()
{
  QString s;

  if (mlvRead->hasMarkedText())
    s = QString("> ") + mlvRead->markedText();
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
  UserSendMsgEvent *e = new UserSendMsgEvent(server, sigman, mainwin, m_szId, m_nPPID);
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
      ChatDlg *chatDlg = new ChatDlg(m_szId, m_nPPID, server, mainwin);
      if (c->Port() != 0)  // Joining a multiparty chat (we connect to them)
      {
        if (chatDlg->StartAsClient(c->Port()))
        {
          //TODO in CICQDaemon
          server->icqChatRequestAccept(strtoul(m_szId, (char **)NULL, 10), 0,
            c->Clients(), c->Sequence(), c->MessageID(), c->IsDirect());
        }
      }
      else  // single party (other side connects to us)
      {
        if (chatDlg->StartAsServer())
        {
          //TODO in CICQDaemon
          server->icqChatRequestAccept(strtoul(m_szId, (char **)NULL, 10),
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
      CFileDlg *fileDlg = new CFileDlg(m_szId, m_nPPID, server);

      if (fileDlg->ReceiveFiles())
      {
        //TODO in CICQDaemon
        server->icqFileTransferAccept(strtoul(m_szId, (char **)NULL, 10),
          fileDlg->LocalPort(), f->Sequence(), f->MessageID(), f->IsDirect());
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
      CRefuseDlg *r = new CRefuseDlg(m_szId, m_nPPID, tr("Chat"), this);

      if (r->exec())
      {
        m_xCurrentReadEvent->SetPending(false);
        CEventChat *c = (CEventChat *)m_xCurrentReadEvent;
        btnRead2->setEnabled(false);
        btnRead3->setEnabled(false);

        //TODO in CICQDaemon
        server->icqChatRequestRefuse(strtoul(m_szId, (char **)NULL, 10),
          codec->fromUnicode(r->RefuseMessage()),
          m_xCurrentReadEvent->Sequence(), c->MessageID(), c->IsDirect());
      }
      delete r;
      break;
    }

    case ICQ_CMDxSUB_FILE:  // refuse a file transfer
    {
      CRefuseDlg *r = new CRefuseDlg(m_szId, m_nPPID, tr("File Transfer"), this);

      if (r->exec())
      {
        m_xCurrentReadEvent->SetPending(false);
        CEventFile *f = (CEventFile *)m_xCurrentReadEvent;
        btnRead2->setEnabled(false);
        btnRead3->setEnabled(false);

        //TODO
        server->icqFileTransferRefuse(strtoul(m_szId, (char **)NULL, 10),
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
      mainwin->callFunction(mnuUserSendChat, m_szId, m_nPPID);
      break;
    case ICQ_CMDxSUB_CHAT:  // join to current chat
    {
      CEventChat *c = (CEventChat *)m_xCurrentReadEvent;
      if (c->Port() != 0)  // Joining a multiparty chat (we connect to them)
      {
        ChatDlg *chatDlg = new ChatDlg(m_szId, m_nPPID, server, mainwin);
        //TODO
        if (chatDlg->StartAsClient(c->Port()))
          server->icqChatRequestAccept(strtoul(m_szId, (char **)NULL, 10), 0, c->Clients(), c->Sequence(), c->MessageID(), c->IsDirect());
      }
      else  // single party (other side connects to us)
      {
        ChatDlg *chatDlg = NULL;
        CJoinChatDlg *j = new CJoinChatDlg(this);
        if (j->exec() && (chatDlg = j->JoinedChat()) != NULL)
          //TODO
          server->icqChatRequestAccept(strtoul(m_szId, (char **)NULL, 10), chatDlg->LocalPort(), c->Clients(), c->Sequence(), c->MessageID(), c->IsDirect());
        delete j;
      }
      break;
    }
    case ICQ_CMDxSUB_URL:   // view a url
      emit viewurl(this, ((CEventUrl *)m_xCurrentReadEvent)->Url());
      break;
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


void UserViewEvent::UserUpdated(CICQSignal *sig, ICQUser *u)
{
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
}


void UserViewEvent::slot_sentevent(ICQEvent *e)
{
  if (e->PPID() != m_nPPID || (strcmp(e->Id(), m_szId) != 0)) return;

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
  a->connectItem(a->insertItem(Key_Escape), this, SLOT(cancelSend()));
#if QT_VERSION >= 300
  if (mainwin->userEventTabDlg &&
      parent == mainwin->userEventTabDlg)
  {
    a->connectItem(a->insertItem(ALT + Key_Left),
		   mainwin->userEventTabDlg, SLOT(moveLeft()));
    a->connectItem(a->insertItem(ALT + Key_Right),
		   mainwin->userEventTabDlg, SLOT(moveRight()));
  }
#endif

  QGroupBox *box = new QGroupBox(this);
  top_lay->addWidget(box);
  QBoxLayout *vlay = new QVBoxLayout(box, 10, 5);
  QBoxLayout *hlay = new QHBoxLayout(vlay);
  chkSendServer = new QCheckBox(tr("Se&nd through server"), box);
  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
  chkSendServer->setChecked(u->SendServer() || (u->StatusOffline() && u->SocketDesc(ICQ_CHNxNONE) == -1));

  if( (u->GetInGroup(GROUPS_SYSTEM, GROUP_INVISIBLE_LIST)) ||
      (u->Port() == 0 && u->SocketDesc(ICQ_CHNxNONE) == -1))
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
  int w = QMAX(btnSend->sizeHint().width(), 75);
  // add a wrapper around the send button that
  // tries to establish a secure connection first.
  connect( btnSend, SIGNAL( clicked() ), this, SLOT( trySecure() ) );

  btnCancel = new QPushButton(tr("&Close"), this);
  w = QMAX(btnCancel->sizeHint().width(), w);
  btnSend->setFixedWidth(w);
  btnCancel->setFixedWidth(w);
  h_lay->addWidget(btnSend);
  h_lay->addWidget(btnCancel);
  connect(btnCancel, SIGNAL(clicked()), this, SLOT(cancelSend()));
  splView = new QSplitter(Vertical, mainWidget);
  //splView->setOpaqueResize();
  mleHistory=0;
  if (mainwin->m_bMsgChatView) {
    mleHistory = new CMessageViewWidget(m_szId, m_nPPID, mainwin, splView);
    // add all unread messages.
    ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
    if (u != NULL && u->NewMessages() > 0)
    {
      for (unsigned short i = 0; i < u->NewMessages(); i++)
      {
        CUserEvent *e = u->EventPeek(i);
        if (e->Id() > m_highestEventId)
          m_highestEventId = e->Id();

        mleHistory->addMsg(e);
      }
    }
    gUserManager.DropUser(u);

#if QT_VERSION >= 300
    connect(mleHistory, SIGNAL(viewurl(QWidget*, QString)), mainwin, SLOT(slot_viewurl(QWidget *, QString)));
#endif
    connect (mainwin, SIGNAL(signal_sentevent(ICQEvent *)), mleHistory, SLOT(addMsg(ICQEvent *)));
    //splView->setResizeMode(mleHistory, QSplitter::FollowSizeHint);
  }

  {
    tmrSendTyping = new QTimer(this);
    connect(tmrSendTyping, SIGNAL(timeout()), SLOT(slot_textChanged_timeout()));
  }

  mleSend = new MLEditWrap(true, splView, true);
  if (mainwin->m_bMsgChatView)
  {
    splView->setResizeMode(mleSend, QSplitter::KeepSize);
    mleSend->resize(mleSend->width(), 70);
  }
  setFocusProxy(mleSend);
  setTabOrder(mleSend, btnSend);
  setTabOrder(btnSend, btnCancel);
  icqColor.SetToDefault();
  mleSend->setBackground(QColor(icqColor.BackRed(), icqColor.BackGreen(), icqColor.BackBlue()));
  mleSend->setForeground(QColor(icqColor.ForeRed(), icqColor.ForeGreen(), icqColor.ForeBlue()));
  connect (mleSend, SIGNAL(signal_CtrlEnterPressed()), btnSend, SIGNAL(clicked()));
  connect(mleSend, SIGNAL(textChanged()), this, SLOT(slot_textChanged()));
  connect(this, SIGNAL(updateUser(CICQSignal*)), mainwin, SLOT(slot_updatedUser(CICQSignal*)));
}

UserSendCommon::~UserSendCommon()
{
}

#if QT_VERSION >= 300
//-----UserSendCommon::windowActivationChange--------------------------------
void UserSendCommon::windowActivationChange(bool oldActive)
{
  if (isActiveWindow())
    QTimer::singleShot(clearDelay, this, SLOT(slot_ClearNewEvents()));
  QWidget::windowActivationChange(oldActive);
}
#endif

//-----UserSendCommon::slot_resettitle---------------------------------------
void UserSendCommon::slot_resettitle()
{
#if QT_VERSION >= 300
  if (mainwin->userEventTabDlg &&
      mainwin->userEventTabDlg->tabIsSelected(this))
    mainwin->userEventTabDlg->setCaption(m_sBaseTitle);
#endif
  setCaption(m_sBaseTitle);
}

//-----UserSendCommon::slot_SetForegroundColor-------------------------------
void UserSendCommon::slot_SetForegroundICQColor()
{
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
  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);

  bool autoSecure = ( u->AutoSecure() && gLicqDaemon->CryptoEnabled() &&
                      (u->SecureChannelSupport() == SECURE_CHANNEL_SUPPORTED ) &&
                      !chkSendServer->isChecked() && !u->Secure());
  gUserManager.DropUser( u );
  disconnect( btnSend, SIGNAL( clicked() ), this, SLOT( trySecure() ) );
  connect(btnSend, SIGNAL(clicked()), this, SLOT(sendButton()));
  if ( autoSecure ) {
    QWidget *w = new KeyRequestDlg(sigman, m_szId, m_nPPID);
    connect(w, SIGNAL( destroyed() ), this, SLOT( sendButton() ) );
  }
  else
    sendButton();
}

void UserSendCommon::slot_textChanged()
{
  if (mleSend->text().isEmpty()) return;

  strTempMsg = mleSend->text();
  server->ProtoTypingNotification(m_szId, m_nPPID, true);
  disconnect(mleSend, SIGNAL(textChanged()), this, SLOT(slot_textChanged()));
  tmrSendTyping->start(5000);
}

void UserSendCommon::slot_textChanged_timeout()
{
  QString str = mleSend->text();

  if (str != strTempMsg)
  {
    strTempMsg = str;
    // Hack to not keep sending the typing notification to ICQ
    if (m_nPPID != LICQ_PPID)
      server->ProtoTypingNotification(m_szId, m_nPPID, true);
  }
  else
  {
    if (tmrSendTyping->isActive())
      tmrSendTyping->stop();
    connect(mleSend, SIGNAL(textChanged()), this, SLOT(slot_textChanged()));
    server->ProtoTypingNotification(m_szId, m_nPPID, false);
  }
}

//-----UserSendCommon::slot_SetBackgroundColor-------------------------------
void UserSendCommon::slot_SetBackgroundICQColor()
{
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
  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_W);

  if (mainwin->m_bMsgChatView
#if QT_VERSION >= 300
      && isActiveWindow() && (!mainwin->userEventTabDlg ||
       (mainwin->userEventTabDlg &&
         (!mainwin->userEventTabDlg->tabExists(this) ||
          mainwin->userEventTabDlg->tabIsSelected(this))))
#endif
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
}


//-----UserSendCommon::changeEventType---------------------------------------
void UserSendCommon::changeEventType(int id)
{
  if (isType(id)) return;
  UserSendCommon* e = NULL;
  QWidget *parent = NULL;
#if QT_VERSION >= 300
  if (mainwin->userEventTabDlg && mainwin->userEventTabDlg->tabExists(this))
    parent = mainwin->userEventTabDlg;
#endif
  switch(id)
  {

  case UC_MESSAGE:
    e = new UserSendMsgEvent(server, sigman, mainwin, m_szId, m_nPPID, parent);
    break;
  case UC_URL:
    e = new UserSendUrlEvent(server, sigman, mainwin, m_szId, m_nPPID, parent);
    break;
  case UC_CHAT:
    e = new UserSendChatEvent(server, sigman, mainwin, m_szId, m_nPPID, parent);
    break;
  case UC_FILE:
    e = new UserSendFileEvent(server, sigman, mainwin, m_szId, m_nPPID, parent);
    break;
  case UC_CONTACT:
    e = new UserSendContactEvent(server, sigman, mainwin, m_szId, m_nPPID, parent);
    break;
  case UC_SMS:
    e = new UserSendSmsEvent(server, sigman, mainwin, m_szId, m_nPPID, parent);
    break;
  default:
    assert(0);
  }

  if (e != NULL)
  {
    if (e->mleSend && mleSend)
    {
      e->mleSend->setText(mleSend->text());
      e->mleSend->setEdited(e->mleSend->length());
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
    mainwin->slot_sendfinished(m_szId, m_nPPID);
    connect(e, SIGNAL(finished(const char *, unsigned long)), mainwin,
            SLOT(slot_sendfinished(const char *, unsigned long)));
    mainwin->licqUserSend.append(e);

    emit signal_msgtypechanged(this, e);

    if (!parent)
    {
      QTimer::singleShot(10, e, SLOT(show()));
      QTimer::singleShot(100, this, SLOT(close()));
    }
#if QT_VERSION >= 300
    else
      mainwin->userEventTabDlg->replaceTab(this, e);
#endif
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
#if QT_VERSION >= 300
    if (mainwin->userEventTabDlg &&
        mainwin->userEventTabDlg->tabIsSelected(this))
      tmpWidgetWidth = mainwin->userEventTabDlg->width();
    else
#endif
      tmpWidgetWidth = width();
    if (grpMR == NULL)
    {
      grpMR = new QVGroupBox(this);
      top_hlay->addWidget(grpMR);

      (void) new QLabel(tr("Drag Users Here\nRight Click for Options"), grpMR);
      //TODO in CMMUserView
      lstMultipleRecipients = new CMMUserView(mainwin->colInfo, mainwin->m_bShowHeader,
                                  strtoul(m_szId, (char **)NULL, 10), mainwin, grpMR);
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
#if QT_VERSION >= 300
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
#endif
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
    ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_W);

    if(u->NewUser())
    {
      u->SetNewUser(false);
      gUserManager.DropUser(u);
      CICQSignal s(SIGNAL_UPDATExUSER, USER_BASIC, m_szId, m_nPPID);
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
#if QT_VERSION >= 300
    if (mainwin->userEventTabDlg &&
        mainwin->userEventTabDlg->tabIsSelected(this))
      mainwin->userEventTabDlg->setCaption(title);
#endif
    setCaption(title);
    setCursor(waitCursor);
    btnSend->setEnabled(false);
    btnCancel->setText(tr("&Cancel"));
    connect (sigman, SIGNAL(signal_doneUserFcn(ICQEvent *)), this, SLOT(sendDone_common(ICQEvent *)));
  }
}


//-----UserSendCommon::setText-----------------------------------------------
void UserSendCommon::setText(const QString& txt)
{
  if(!mleSend) return;
  mleSend->setText(txt);
  mleSend->GotoEnd();
  mleSend->setEdited(false);
}


//-----UserSendCommon::sendDone_common---------------------------------------
void UserSendCommon::sendDone_common(ICQEvent *e)
{
  if (e == NULL)
  {
    QString title = m_sBaseTitle + " [" + m_sProgressMsg + tr("error") + "]";
#if QT_VERSION >= 300
    if (mainwin->userEventTabDlg &&
        mainwin->userEventTabDlg->tabIsSelected(this))
      mainwin->userEventTabDlg->setCaption(title);
#endif
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
#if QT_VERSION >= 300
  if (mainwin->userEventTabDlg &&
      mainwin->userEventTabDlg->tabIsSelected(this))
    mainwin->userEventTabDlg->setCaption(title);
#endif
  setCaption(title);

  setCursor(arrowCursor);
  btnSend->setEnabled(true);
  btnCancel->setText(tr("&Close"));
  // If cancelled automatically check "Send through Server"
  if (mainwin->m_bAutoSendThroughServer && e->Result() == EVENT_CANCELLED)
    chkSendServer->setChecked(true);

  if (m_lnEventTag.size() == 0)
    disconnect (sigman, SIGNAL(signal_doneUserFcn(ICQEvent *)), this, SLOT(sendDone_common(ICQEvent *)));

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
    u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_W);

    msg = tr("%1 is in %2 mode:\n%3\nSend...")
             .arg(codec->toUnicode(u->GetAlias())).arg(u->StatusStr())
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
    u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
    msg = tr("%1 refused %2, send through server")
          .arg(codec->toUnicode(u->GetAlias())).arg(EventDescription(ue));
    InformUser(this, msg);
    gUserManager.DropUser(u);
    return;
  }*/
  else
  {
    emit autoCloseNotify();
    if (sendDone(e))
    {
      emit mainwin->signal_sentevent(e);
      if (mainwin->m_bMsgChatView && mleHistory != NULL)
      {
        mleHistory->GotoEnd();
        resetSettings();
      }
      else
        close();
    }
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
      CEventMsg *ue = (CEventMsg *)e->UserEvent();
      // create initial strings (implicit copying, no allocation impact :)
      char *tmp = gTranslator.NToRN(ue->Message());
      QCString wholeMessageRaw(tmp);
      delete [] tmp;
      unsigned int wholeMessagePos = 0;

      bool needsSplitting = false;
      // If we send through server (= have message limit), and we've crossed the limit
      if (!bOnline && ((wholeMessageRaw.length() - wholeMessagePos) > MAX_MESSAGE_SIZE))
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
          messageRaw = wholeMessageRaw.mid(wholeMessagePos, MAX_MESSAGE_SIZE);
          tmp = gTranslator.RNToN(messageRaw);
          messageRaw = tmp;
          delete [] tmp;
          message = codec->toUnicode(messageRaw);

          if ((wholeMessageRaw.length() - wholeMessagePos) > MAX_MESSAGE_SIZE)
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
              message.truncate(foundIndex);
              messageRaw = codec->fromUnicode(message);
            }
          }
        }
        else
        {
          messageRaw = ue->Message();
        }

        icqEventTag = server->icqSendMessage(m_szId, messageRaw.data(),
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

      icqEventTag = server->ProtoSendUrl(m_szId, m_nPPID, ue->Url(),
        ue->Description(), bOnline, nLevel, false, &icqColor);

      break;
    }
    case ICQ_CMDxSUB_CONTACTxLIST:
    {
      CEventContactList* ue = (CEventContactList *) e->UserEvent();
      const ContactList& clist = ue->Contacts();
      UinList uins;

      for(ContactList::const_iterator i = clist.begin(); i != clist.end(); i++)
        uins.push_back((*i)->Uin());

      if(uins.size() == 0)
        break;

      icqEventTag = server->icqSendContactList(strtoul(m_szId, (char **)NULL, 10),
        uins, bOnline, nLevel, false, &icqColor);

      break;
    }
    case ICQ_CMDxSUB_CHAT:
    {
      CEventChat *ue = (CEventChat *)e->UserEvent();

      if (ue->Clients() == NULL)
        //TODO in the daemon
        icqEventTag = server->icqChatRequest(strtoul(m_szId, (char **)NULL, 10),
          ue->Reason(), nLevel, !bOnline);
      else
        //TODO in the daemon
        icqEventTag = server->icqMultiPartyChatRequest(
              strtoul(m_szId, (char **)NULL, 10), ue->Reason(), ue->Clients(),
              ue->Port(), nLevel, !bOnline);

      break;
    }
    case ICQ_CMDxSUB_FILE:
    {
      CEventFile *ue = (CEventFile *)e->UserEvent();
      ConstFileList filelist(ue->FileList());
      //TODO in the daemon
      icqEventTag = server->icqFileTransfer(strtoul(m_szId, (char **)NULL, 10),
        ue->Filename(), ue->FileDescription(), filelist, nLevel, !bOnline);

      break;
    }
    case ICQ_CMDxSUB_SMS:
    {
      CEventSms *ue = (CEventSms *)e->UserEvent();
      //TODO in the daemon
      icqEventTag = server->icqSendSms(ue->Number(), ue->Message(),
        strtoul(m_szId, (char **)NULL, 0));

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


//-----UserSendCommon::cancelSend--------------------------------------------
void UserSendCommon::cancelSend()
{
  unsigned long icqEventTag = 0;
  if (m_lnEventTag.size())
    icqEventTag = m_lnEventTag.front();

  if (!icqEventTag)
  {
    if (mainwin->m_bMsgChatView)
    {
      // the window is at the front, if the timer has not expired and we close
      // the window, then the new events will stay there
      slot_ClearNewEvents();
    }
#if QT_VERSION >= 300
    if (mainwin->userEventTabDlg &&
        mainwin->userEventTabDlg->tabExists(this))
      mainwin->userEventTabDlg->removeTab(this);
    else
#endif
      close();
    return;
  }

#if QT_VERSION >= 300
  if (mainwin->userEventTabDlg &&
      mainwin->userEventTabDlg->tabIsSelected(this))
    mainwin->userEventTabDlg->setCaption(m_sBaseTitle);
#endif
  setCaption(m_sBaseTitle);
  server->CancelEvent(icqEventTag);
  icqEventTag = 0;
  btnSend->setEnabled(true);
  btnCancel->setText(tr("&Close"));
  setCursor(arrowCursor);
}


//-----UserSendCommon::UserUpdated-------------------------------------------
void UserSendCommon::UserUpdated(CICQSignal *sig, ICQUser *u)
{
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
        if (e != NULL)
        {
          mleHistory->addMsg(e);
        }
      }
    }
  }
}


//-----UserSendCommon::checkSecure-------------------------------------------
bool UserSendCommon::checkSecure()
{
  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
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
      ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
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
#if QT_VERSION >= 300
  if (mainwin->userEventTabDlg &&
      mainwin->userEventTabDlg->tabIsSelected(this))
    mainwin->userEventTabDlg->setCaption(m_sBaseTitle);
#endif
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
  server->ProtoTypingNotification(m_szId, m_nPPID, false);
  
  // do nothing if a command is already being processed
  unsigned long icqEventTag = 0;
  if (m_lnEventTag.size())
    icqEventTag = m_lnEventTag.front();

  if (icqEventTag != 0) return;

  if(!mleSend->edited() &&
     !QueryUser(this, tr("You didn't edit the message.\n"
                         "Do you really want to send it?"), tr("&Yes"), tr("&No")))
    return;

  // don't let the user send empty messages
  if (mleSend->text().stripWhiteSpace().isEmpty()) return;

  if (!UserSendCommon::checkSecure()) return;

  // create initial strings (implicit copying, no allocation impact :)
  char *tmp = gTranslator.NToRN(codec->fromUnicode(mleSend->text()));
  QCString wholeMessageRaw(tmp);
  delete [] tmp;
  unsigned int wholeMessagePos = 0;

  bool needsSplitting = false;
  // If we send through server (= have message limit), and we've crossed the limit
  if (chkSendServer->isChecked() && ((wholeMessageRaw.length() - wholeMessagePos) > MAX_MESSAGE_SIZE))
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
        messageRaw = wholeMessageRaw.mid(wholeMessagePos, MAX_MESSAGE_SIZE);
        tmp = gTranslator.RNToN(messageRaw);
        messageRaw = tmp;
        delete [] tmp;
        message = codec->toUnicode(messageRaw);

        if ((wholeMessageRaw.length() - wholeMessagePos) > MAX_MESSAGE_SIZE)
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
              message.truncate(foundIndex);
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

     icqEventTag = server->ProtoSendMessage(m_szId, m_nPPID, messageRaw.data(),
      chkSendServer->isChecked() ? false : true,
      chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL,
      chkMass->isChecked(), &icqColor);
     if (m_nPPID == LICQ_PPID)
       m_lnEventTag.push_back(icqEventTag);

     tmp = gTranslator.NToRN(messageRaw);
     wholeMessagePos += strlen(tmp);
     delete [] tmp;
  }

  UserSendCommon::sendButton();
}


//-----UserSendMsgEvent::sendDone--------------------------------------------
bool UserSendMsgEvent::sendDone(ICQEvent *e)
{
  mleSend->setText(QString::null);

  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
  if (u->Away() && u->ShowAwayMsg())
  {
    gUserManager.DropUser(u);
    (void) new ShowAwayMsgDlg(NULL, NULL, m_szId, m_nPPID);
  }
  else
    gUserManager.DropUser(u);

  return true;

}

void UserSendMsgEvent::resetSettings()
{
  mleSend->clear();
  mleSend->setFocus();
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

  m_sBaseTitle += tr(" - URL");
#if QT_VERSION >= 300
  if (mainwin->userEventTabDlg &&
      mainwin->userEventTabDlg->tabIsSelected(this))
    mainwin->userEventTabDlg->setCaption(m_sBaseTitle);
#endif
  setCaption(m_sBaseTitle);
  cmbSendType->setCurrentItem(UC_URL);
}

UserSendUrlEvent::~UserSendUrlEvent()
{
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
  server->ProtoTypingNotification(m_szId, m_nPPID, false);
  
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
  icqEventTag = server->ProtoSendUrl(m_szId, m_nPPID, edtItem->text().latin1(), codec->fromUnicode(mleSend->text()),
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

  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
  if (u->Away() && u->ShowAwayMsg())
  {
    gUserManager.DropUser(u);
    (void) new ShowAwayMsgDlg(NULL, NULL, m_szId, m_nPPID);
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
#if QT_VERSION >= 300
  if (mainwin->userEventTabDlg &&
      mainwin->userEventTabDlg->tabIsSelected(this))
    mainwin->userEventTabDlg->setCaption(m_sBaseTitle);
#endif
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
    m_lFileList.push_back(strdup((*it).latin1()));

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
  server->ProtoTypingNotification(m_szId, m_nPPID, false);
  
  if (edtItem->text().stripWhiteSpace().isEmpty())
  {
    WarnUser(this, tr("You must specify a file to transfer!"));
    return;
  }

  unsigned long icqEventTag;
  //TODO in daemon
  icqEventTag = server->icqFileTransfer(strtoul(m_szId, (char **)NULL, 10),
     codec->fromUnicode(edtItem->text()),
     codec->fromUnicode(mleSend->text()), m_lFileList,
     chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL,
     chkSendServer->isChecked());

  m_lnEventTag.push_back(icqEventTag);

  UserSendCommon::sendButton();
}

void UserSendFileEvent::setFile(const QString& file, const QString& description)
{
  edtItem->setText(file);
  setText(description);
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
    ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
    QString s(!e->ExtendedAck() ? tr("No reason provided") :
                               codec->toUnicode(e->ExtendedAck()->Response()));
    QString result = tr("File transfer with %2 refused:\n%3")
                       .arg(codec->toUnicode(u->GetAlias()))
                       .arg(s);
    gUserManager.DropUser(u);
    InformUser(this, result);
  }
  else
  {
    CEventFile *f = (CEventFile *)e->UserEvent();
    CFileDlg *fileDlg = new CFileDlg(m_szId, m_nPPID, server);
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
#if QT_VERSION >= 300
  if (mainwin->userEventTabDlg &&
      mainwin->userEventTabDlg->tabIsSelected(this))
    mainwin->userEventTabDlg->setCaption(m_sBaseTitle);
#endif
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
  server->ProtoTypingNotification(m_szId, m_nPPID, false);
  
  unsigned long icqEventTag;

  if (m_nMPChatPort == 0)
    //TODO in daemon
    icqEventTag = server->icqChatRequest(strtoul(m_szId, (char **)NULL, 10),
                                         codec->fromUnicode(mleSend->text()),
                                         chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL,
                                         chkSendServer->isChecked());
  else
    icqEventTag = server->icqMultiPartyChatRequest(strtoul(m_szId, (char **)NULL, 10),
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
    ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
    QString s(!e->ExtendedAck() ? tr("No reason provided") :
                               codec->toUnicode(e->ExtendedAck()->Response()));
    QString result = tr("Chat with %2 refused:\n%3")
                       .arg(codec->toUnicode(u->GetAlias()))
                       .arg(s);
    gUserManager.DropUser(u);
    InformUser(this, result);
  }
  else
  {
    CEventChat *c = (CEventChat *)e->UserEvent();
    if (c->Port() == 0)  // If we requested a join, no need to do anything
    {
      ChatDlg *chatDlg = new ChatDlg(m_szId, m_nPPID, server, mainwin);
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
  delete mleSend; mleSend = NULL;

  QBoxLayout* lay = new QVBoxLayout(mainWidget);
  lay->addWidget(splView);
  QLabel* lblContact =  new QLabel(tr("Drag Users Here - Right Click for Options"), mainWidget);
  lay->addWidget(lblContact);

  lstContacts = new CMMUserView(mainwin->colInfo, mainwin->m_bShowHeader,
                                m_szId, nPPID, mainwin, mainWidget);
  lay->addWidget(lstContacts);

  m_sBaseTitle += tr(" - Contact List");
#if QT_VERSION >= 300
  if (mainwin->userEventTabDlg &&
      mainwin->userEventTabDlg->tabIsSelected(this))
    mainwin->userEventTabDlg->setCaption(m_sBaseTitle);
#endif
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
  connect(mleSend, SIGNAL(textChanged()), this, SLOT(slot_textChanged()));
  server->ProtoTypingNotification(m_szId, m_nPPID, false);
  
  CMMUserViewItem *i = static_cast<CMMUserViewItem*>(lstContacts->firstChild());
  UserStringList users;

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
  icqEventTag = server->icqSendContactList(m_szId, users,
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

  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
  if (u->Away() && u->ShowAwayMsg())
  {
    gUserManager.DropUser(u);
    (void) new ShowAwayMsgDlg(NULL, NULL, m_szId, m_nPPID);
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

  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_W);
  if (u != NULL)
  {
    nfoNumber->setData(codec->toUnicode(u->GetCellularNumber()));
    gUserManager.DropUser(u);
  }

  m_sBaseTitle += tr(" - SMS");
#if QT_VERSION >= 300
  if (mainwin->userEventTabDlg &&
      mainwin->userEventTabDlg->tabIsSelected(this))
    mainwin->userEventTabDlg->setCaption(m_sBaseTitle);
#endif
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
  server->ProtoTypingNotification(m_szId, m_nPPID, false);
  
  unsigned long icqEventTag = 0;
  if (m_lnEventTag.size())
    icqEventTag = m_lnEventTag.front();

  // do nothing if a command is already being processed
  if (icqEventTag != 0)
    return;

  if(!mleSend->edited() &&
     !QueryUser(this, tr("You didn't edit the SMS.\n"
                         "Do you really want to send it?"), tr("&Yes"), tr("&No")))
    return;

  // don't let the user send empty messages
  if (mleSend->text().stripWhiteSpace().isEmpty())
    return;

  //TODO in daemon
  icqEventTag = server->icqSendSms(nfoNumber->text().latin1(), mleSend->text().utf8().data(),
    strtoul(m_szId, (char **)NULL, 10));
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
bool UserSendSmsEvent::sendDone(ICQEvent *e)
{
  return true;
}

//-----UserSendSmsEvent::slot_count--------------------------------------------
void UserSendSmsEvent::slot_count()
{
  int len = 160 - strlen(mleSend->text().utf8().data());
  nfoCount->setData((len >= 0) ? len : 0);
}

// ----------------------------------------------------------------------------

#include "usereventdlg.moc"
