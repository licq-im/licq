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
#ifdef QT_PROTOCOL_PLUGIN
UserEventCommon::UserEventCommon(CICQDaemon *s, CSignalManager *theSigMan,
                                 CMainWindow *m, const char *_szId,
                                 unsigned long _nPPID, QWidget *parent,
                                 const char *name)
  : QWidget(parent, name, WDestructiveClose)
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

  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_W);
  if (u != NULL)
  {
    nfoStatus->setData(u->StatusStr());
    if (u->NewMessages() == 0)
      setIcon(CMainWindow::iconForStatus(u->StatusFull()));
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

  connect (sigman, SIGNAL(signal_updatedUser(CICQSignal *)),
           this, SLOT(slot_userupdated(CICQSignal *)));

  mainWidget = new QWidget(this);
  top_lay->addWidget(mainWidget);
}
#endif

UserEventTabDlg::UserEventTabDlg(QWidget *parent, const char *name)
  : QWidget(parent, name, WDestructiveClose)
{
#if QT_VERSION >= 300
  QBoxLayout *lay = new QVBoxLayout(this);
  tabw = new QTabWidget(this);
  lay->addWidget(tabw);
  connect(tabw, SIGNAL(currentChanged(QWidget *)),
	  this, SLOT(updateTitle(QWidget *)));
  connect(tabw, SIGNAL(currentChanged(QWidget *)),
          this, SLOT(clearEvents(QWidget *)));
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
  ICQUser *u = gUserManager.FetchUser(tab->Uin(), LOCK_W);
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
    if (tab->Uin() == u->Uin())
    {
      if (u->NewMessages() > 0)
      {
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
      }
      // use status icon
      else tabw->setTabIconSet(tab, CMainWindow::iconForStatus(u->StatusFull()));
      return;
    }
  }
#endif
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
  UserEventCommon *e = static_cast<UserEventCommon*>(tab);
  ICQUser *u = gUserManager.FetchUser(e->Uin(), LOCK_R);
  if (u != NULL && u->NewMessages() > 0)
  {
    std::vector<int> idList;
    for (unsigned short i = 0; i < u->NewMessages(); i++)
    {
      CUserEvent *e = u->EventPeek(i);
      if (e->Direction() == D_RECEIVER && e->SubCommand() == ICQ_CMDxSUB_MSG)
	idList.push_back(e->Id());
    }

    for (unsigned short i = 0; i < idList.size(); i++)
      u->EventClearId(idList[i]);
  }
  gUserManager.DropUser(u);
#endif
}

void UserEventTabDlg::moveLeft()
{
#if QT_VERSION >= 300
  int index = tabw->currentPageIndex();
  if (index > 0)
    tabw->setCurrentPage(index - 1);
#endif
}

void UserEventTabDlg::moveRight()
{
#if QT_VERSION >= 300
  int index = tabw->currentPageIndex();
  if (index < tabw->count() - 1)
    tabw->setCurrentPage(index + 1);
#endif
}

// -----------------------------------------------------------------------------

UserEventCommon::UserEventCommon(CICQDaemon *s, CSignalManager *theSigMan,
                                 CMainWindow *m, unsigned long _nUin,
                                 QWidget* parent, const char* name)
  : QWidget(parent, name, WDestructiveClose)
{
  server = s;
  mainwin = m;
  sigman = theSigMan;
  m_nUin = _nUin;
  m_bOwner = (m_nUin == gUserManager.OwnerUin());
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

  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_W);
  if (u != NULL)
  {
    nfoStatus->setData(u->StatusStr());
    if (u->NewMessages() == 0)
      setIcon(CMainWindow::iconForStatus(u->StatusFull()));
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

  connect (sigman, SIGNAL(signal_updatedUser(CICQSignal *)),
           this, SLOT(slot_userupdated(CICQSignal *)));

  mainWidget = new QWidget(this);
  top_lay->addWidget(mainWidget);
}

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
#ifdef QT_PROTOCOL_PLUGIN
    ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_W);
#else
    ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_W);
#endif
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

  if (u->Secure())
    btnSecure->setPixmap(mainwin->pmSecureOn);
  else
    btnSecure->setPixmap(mainwin->pmSecureOff);

  m_sBaseTitle = codec->toUnicode(u->GetAlias()) + " (" +
             codec->toUnicode(u->GetFirstName()) + " " +
             codec->toUnicode(u->GetLastName())+ ")";
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
#ifdef QT_PROTOCOL_PLUGIN
  //TODO
  emit finished(strtoul(m_szId, (char **)NULL, 10));

  //TODO for CMainWindow
  if (m_bDeleteUser && !m_bOwner)
    mainwin->RemoveUserFromList(strdup(m_szId), m_nPPID, this);
    
  free(m_szId);
#else
  emit finished(m_nUin);

  if (m_bDeleteUser && !m_bOwner)
    mainwin->RemoveUserFromList(m_nUin, this);
#endif
}


//-----UserEventCommon::slot_userupdated-------------------------------------
void UserEventCommon::slot_userupdated(CICQSignal *sig)
{
#ifdef QT_PROTOCOL_PLUGIN
  if (m_nPPID != sig->PPID() || strcmp(m_szId, sig->Id()) == 0) return;

  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
#else
  if (m_nUin != sig->Uin()) return;

  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
#endif
  if (u == NULL) return;

  switch (sig->SubSignal())
  {
    case USER_STATUS:
    {
      nfoStatus->setData(u->StatusStr());
      if (u->NewMessages() == 0)
        setIcon(CMainWindow::iconForStatus(u->StatusFull()));
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
        setIcon(CMainWindow::iconForStatus(u->StatusFull()));
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
#ifdef QT_PROTOCOL_PLUGIN
  //TODO in CMainWindow
  mainwin->callInfoTab(mnuUserHistory, m_szId, m_nPPID, true);
#else
  mainwin->callInfoTab(mnuUserHistory, m_nUin, true);
#endif
}


void UserEventCommon::showUserInfo()
{
#ifdef QT_PROTOCOL_PLUGIN
  //TODO in CMainWindow
  mainwin->callInfoTab(mnuUserGeneral, m_szId, m_nPPID, true);
#else
 mainwin->callInfoTab(mnuUserGeneral, m_nUin, true);
#endif
}

void UserEventCommon::slot_security()
{
#ifdef QT_PROTOCOL_PLUGIN
  //TODO in KeyRequestDlg
  (void) new KeyRequestDlg(sigman, m_szId, m_nPPID);
#else
  (void) new KeyRequestDlg(sigman, m_nUin);
#endif
}


//=====UserViewEvent=========================================================
#ifdef QT_PROTOCOL_PLUGIN
UserViewEvent::UserViewEvent(CICQDaemon *s, CSignalManager *theSigMan,
                             CMainWindow *m, const char *_szId,
                             unsigned long _nPPID, QWidget* parent)
  : UserEventCommon(s, theSigMan, m, _szId, _nPPID, parent, "UserViewEvent"),
    m_highestEventId(-1)
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
    /*
     Create an item for the message we're currently viewing.
    */
    MsgViewItem *e = new MsgViewItem(u->EventPeek(0), codec, msgView);
    /*
     Create items for all the messages which already await
     in the queue. We cannot rely on getting CICQSignals for them
     since they might've arrived before the dialog appeared,
     possibly being undisplayed messages from previous licq session.
    */
    for (unsigned short i = 1; i < u->NewMessages(); i++)
    {
      CUserEvent* event = u->EventPeek(i);
      (void) new MsgViewItem(event, codec, msgView);
      // Make sure we don't add this message again, even if we'll
      // receive an userUpdated signal for it.
      if (m_highestEventId < event->Id())
         m_highestEventId = event->Id();
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
#endif

UserViewEvent::UserViewEvent(CICQDaemon *s, CSignalManager *theSigMan,
                             CMainWindow *m, unsigned long _nUin, QWidget* parent)
  : UserEventCommon(s, theSigMan, m, _nUin, parent, "UserViewEvent"),
    m_highestEventId(-1)
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

  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
  if (u != NULL && u->NewMessages() > 0)
  {
    /*
     Create an item for the message we're currently viewing.
    */
    MsgViewItem *e = new MsgViewItem(u->EventPeek(0), codec, msgView);
    /*
     Create items for all the messages which already await
     in the queue. We cannot rely on getting CICQSignals for them
     since they might've arrived before the dialog appeared,
     possibly being undisplayed messages from previous licq session.
    */
    for (unsigned short i = 1; i < u->NewMessages(); i++)
    {
      CUserEvent* event = u->EventPeek(i);
      (void) new MsgViewItem(event, codec, msgView);
      // Make sure we don't add this message again, even if we'll
      // receive an userUpdated signal for it.
      if (m_highestEventId < event->Id())
         m_highestEventId = event->Id();
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

#ifdef QT_PROTOCOL_PLUGIN
  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
#else
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
#endif
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
#ifdef QT_PROTOCOL_PLUGIN
        CEventAuthRequest *pAuthReq = (CEventAuthRequest *)m;;
        ICQUser *u = gUserManager.FetchUser(pAuthReq->IdString(), pAuthReq->PPID(), LOCK_R);
#else
        ICQUser *u = gUserManager.FetchUser( ((CEventAuthRequest *)m)->Uin(), LOCK_R);
#endif
        if (u == NULL)
          btnRead3->setText(tr("A&dd User"));
        else
          gUserManager.DropUser(u);
        break;
      }
      case ICQ_CMDxSUB_AUTHxGRANTED:
      {
#ifdef QT_PROTOCOL_PLUGIN
        CEventAuthGranted *pAuth = (CEventAuthGranted *)m;
        ICQUser *u = gUserManager.FetchUser(pAuth->IdString(), pAuth->PPID(), LOCK_R);
#else
        ICQUser *u = gUserManager.FetchUser( ((CEventAuthGranted *)m)->Uin(), LOCK_R);
#endif
        if (u == NULL)
          btnRead1->setText(tr("A&dd User"));
        else
          gUserManager.DropUser(u);
        break;
      }
      case ICQ_CMDxSUB_ADDEDxTOxLIST:
      {
#ifdef QT_PROTOCOL_PLUGIN
        CEventAdded *pAdd = (CEventAdded *)m;
        ICQUser *u = gUserManager.FetchUser(pAdd->IdString(), pAdd->PPID(), LOCK_R);
#else
        ICQUser *u = gUserManager.FetchUser( ((CEventAdded *)m)->Uin(), LOCK_R);
#endif
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
#ifdef QT_PROTOCOL_PLUGIN
    ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_W);
#else
    ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_W);
#endif
    u->EventClearId(e->m_nEventId);
    gUserManager.DropUser(u);
    e->MarkRead();
  }
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
#ifdef QT_PROTOCOL_PLUGIN
  UserSendMsgEvent *e = new UserSendMsgEvent(server, sigman, mainwin, m_szId, m_nPPID);
#else
  UserSendMsgEvent *e = new UserSendMsgEvent(server, sigman, mainwin, m_nUin);
#endif
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
#ifdef QT_PROTOCOL_PLUGIN
      CEventAuthRequest *p = (CEventAuthRequest *)m_xCurrentReadEvent;
      (void) new AuthUserDlg(server, p->IdString(), p->PPID(), true);
#else
      (void) new AuthUserDlg(server, ((CEventAuthRequest *)m_xCurrentReadEvent)->Uin(), true);
#endif
      break;
    }

    case ICQ_CMDxSUB_AUTHxGRANTED:
    {
#ifdef QT_PROTOCOL_PLUGIN
      CEventAuthGranted *p = (CEventAuthGranted *)m_xCurrentReadEvent;
      server->AddUserToList(p->IdString(), p->PPID()); //TODO  maybe?
#else
      server->AddUserToList( ((CEventAuthGranted *)m_xCurrentReadEvent)->Uin());
#endif
      break;
    }

    case ICQ_CMDxSUB_ADDEDxTOxLIST:
    {
#ifdef QT_PROTOCOL_PLUGIN
      CEventAdded *p = (CEventAdded *)m_xCurrentReadEvent;
      server->AddUserToList(p->IdString(), p->PPID()); //TODO maybe?
#else
      server->AddUserToList( ((CEventAdded *)m_xCurrentReadEvent)->Uin());
#endif
      break;
    }

    case ICQ_CMDxSUB_CONTACTxLIST:
    {
      const ContactList& cl = static_cast<CEventContactList*>(m_xCurrentReadEvent)->Contacts();

      ContactList::const_iterator it;
      for(it = cl.begin(); it != cl.end(); ++it) {
#ifdef QT_PROTOCOL_PLUGIN
        ICQUser *u = gUserManager.FetchUser((*it)->IdString(), (*it)->PPID(), LOCK_R);
#else
        ICQUser* u = gUserManager.FetchUser((*it)->Uin(), LOCK_R);
#endif
        if(u == NULL)
          server->AddUserToList((*it)->Uin());
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
#ifdef QT_PROTOCOL_PLUGIN
      //TODO in ChatDlg
      ChatDlg *chatDlg = new ChatDlg(m_szId, m_nPPID, server, mainwin);
#else
      ChatDlg *chatDlg = new ChatDlg(m_nUin, server, mainwin);
#endif
      if (c->Port() != 0)  // Joining a multiparty chat (we connect to them)
      {
        if (chatDlg->StartAsClient(c->Port()))
#ifdef QT_PROTOCOL_PLUGIN
        {
          //TODO in CICQDaemon
          server->icqChatRequestAccept(strtoul(m_szId, (char **)NULL, 10), 0,
            c->Clients(), c->Sequence(), c->MessageID(), c->IsDirect());
        }
#else
          server->icqChatRequestAccept(m_nUin, 0, c->Clients(), c->Sequence(), c->MessageID(), c->IsDirect());
#endif
      }
      else  // single party (other side connects to us)
      {
        if (chatDlg->StartAsServer())
#ifdef QT_PROTOCOL_PLUGIN
        {
          //TODO in CICQDaemon
          server->icqChatRequestAccept(strtoul(m_szId, (char **)NULL, 10),
            chatDlg->LocalPort(), c->Clients(), c->Sequence(), c->MessageID(), c->IsDirect());
        }
#else
          server->icqChatRequestAccept(m_nUin, chatDlg->LocalPort(), c->Clients(), c->Sequence(), c->MessageID(), c->IsDirect());
#endif
      }
      break;
    }

    case ICQ_CMDxSUB_FILE:  // accept a file transfer
    {
      m_xCurrentReadEvent->SetPending(false);
      btnRead2->setEnabled(false);
      btnRead3->setEnabled(false);
      CEventFile *f = (CEventFile *)m_xCurrentReadEvent;
#ifdef QT_PROTOCOL_PLUGIN
      // TODO in CFileDlg
      CFileDlg *fileDlg = new CFileDlg(m_szId, m_nPPID, server);
      if (fileDlg->ReceiveFiles())
      {
        //TODO in CICQDaemon
        server->icqFileTransferAccept(strtoul(m_szId, (char **)NULL, 10),
          fileDlg->LocalPort(), f->Sequence(), f->MessageID(), f->IsDirect());
      }
#else
      CFileDlg *fileDlg = new CFileDlg(m_nUin, server);
      if (fileDlg->ReceiveFiles())
        server->icqFileTransferAccept(m_nUin, fileDlg->LocalPort(), f->Sequence(), f->MessageID(), f->IsDirect());
#endif
      break;
    }

    case ICQ_CMDxSUB_AUTHxREQUEST:
    {
#ifdef QT_PROTOCOL_PLUGIN
      CEventAuthRequest *p = (CEventAuthRequest *)m_xCurrentReadEvent;
      //TODO in AuthUserDlg
      (void) new AuthUserDlg(server, p->IdString(), p->PPID(), false);
#else
      (void) new AuthUserDlg(server, ((CEventAuthRequest *)m_xCurrentReadEvent)->Uin(), false);
#endif
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
#ifdef QT_PROTOCOL_PLUGIN
      //TODO in CRefuseDlg
      CRefuseDlg *r = new CRefuseDlg(m_szId, m_nPPID, tr("Chat"), this);
#else
      CRefuseDlg *r = new CRefuseDlg(m_nUin, tr("Chat"), this);
#endif
      if (r->exec())
      {
        m_xCurrentReadEvent->SetPending(false);
        CEventChat *c = (CEventChat *)m_xCurrentReadEvent;
        btnRead2->setEnabled(false);
        btnRead3->setEnabled(false);
#ifdef QT_PROTOCOL_PLUGIN
        //TODO in CICQDaemon
        server->icqChatRequestRefuse(strtoul(m_szId, (char **)NULL, 10),
          codec->fromUnicode(r->RefuseMessage()),
          m_xCurrentReadEvent->Sequence(), c->MessageID(), c->IsDirect());
#else
        server->icqChatRequestRefuse(m_nUin, codec->fromUnicode(r->RefuseMessage()),
           m_xCurrentReadEvent->Sequence(), c->MessageID(), c->IsDirect());
#endif
      }
      delete r;
      break;
    }

    case ICQ_CMDxSUB_FILE:  // refuse a file transfer
    {
#ifdef QT_PROTOCOL_PLUGIN
      CRefuseDlg *r = new CRefuseDlg(m_szId, m_nPPID, tr("File Transfer"), this);
#else
      CRefuseDlg *r = new CRefuseDlg(m_nUin, tr("File Transfer"), this);
#endif
      if (r->exec())
      {
        m_xCurrentReadEvent->SetPending(false);
        CEventFile *f = (CEventFile *)m_xCurrentReadEvent;
        btnRead2->setEnabled(false);
        btnRead3->setEnabled(false);
#ifdef QT_PROTOCOL_PLUGIN
        //TODO
        server->icqFileTransferRefuse(strtoul(m_szId, (char **)NULL, 10),
          codec->fromUnicode(r->RefuseMessage()),
          m_xCurrentReadEvent->Sequence(), f->MessageID(), f->IsDirect());
#else
        server->icqFileTransferRefuse(m_nUin, codec->fromUnicode(r->RefuseMessage()),
           m_xCurrentReadEvent->Sequence(), f->MessageID(), f->IsDirect());
#endif
      }
      delete r;
      break;
    }

    case ICQ_CMDxSUB_AUTHxREQUEST:
#ifdef QT_PROTOCOL_PLUGIN
    {
      CEventAuthRequest *p = (CEventAuthRequest *)m_xCurrentReadEvent;
      server->AddUserToList(p->IdString(), p->PPID());
      break;
    }
#else
      server->AddUserToList( ((CEventAuthRequest *)m_xCurrentReadEvent)->Uin());
      break;
#endif
  }
}


void UserViewEvent::slot_btnRead4()
{
  if (m_xCurrentReadEvent == NULL) return;

  switch (m_xCurrentReadEvent->SubCommand())
  {
    case ICQ_CMDxSUB_MSG:
#ifdef QT_PROTOCOL_PLUGIN
      mainwin->callFunction(mnuUserSendChat, m_szId, m_nPPID);
#else
      mainwin->callFunction(mnuUserSendChat, Uin());
#endif
      break;
    case ICQ_CMDxSUB_CHAT:  // join to current chat
    {
      CEventChat *c = (CEventChat *)m_xCurrentReadEvent;
      if (c->Port() != 0)  // Joining a multiparty chat (we connect to them)
      {
#ifdef QT_PROTOCOL_PLUGIN
        ChatDlg *chatDlg = new ChatDlg(m_szId, m_nPPID, server, mainwin);
        //TODO
        if (chatDlg->StartAsClient(c->Port()))
          server->icqChatRequestAccept(strtoul(m_szId, (char **)NULL, 10), 0, c->Clients(), c->Sequence(), c->MessageID(), c->IsDirect());
#else
        ChatDlg *chatDlg = new ChatDlg(m_nUin, server, mainwin);
        if (chatDlg->StartAsClient(c->Port()))
          server->icqChatRequestAccept(m_nUin, 0, c->Clients(), c->Sequence(), c->MessageID(), c->IsDirect());
#endif
      }
      else  // single party (other side connects to us)
      {
        ChatDlg *chatDlg = NULL;
        CJoinChatDlg *j = new CJoinChatDlg(this);
        if (j->exec() && (chatDlg = j->JoinedChat()) != NULL)
#ifdef QT_PROTOCOL_PLUGIN
          //TODO
          server->icqChatRequestAccept(strtoul(m_szId, (char **)NULL, 10), chatDlg->LocalPort(), c->Clients(), c->Sequence(), c->MessageID(), c->IsDirect());
#else
          server->icqChatRequestAccept(m_nUin, chatDlg->LocalPort(), c->Clients(), c->Sequence(), c->MessageID(), c->IsDirect());
#endif
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
    CUserEvent* e = NULL;

    if (sig->Argument() > 0)
    {
      int eventId = sig->Argument();
      // Making sure we didn't handle this message already.
      if (m_highestEventId < eventId)
      {
         m_highestEventId = eventId;
         e = u->EventPeekId(eventId);
         if (e != NULL)
         {
           MsgViewItem *m = new MsgViewItem(e, codec, msgView);
           msgView->ensureItemVisible(m);
         }
      }
      else
      {
         qDebug("Ignoring double message");
      }
    }

    if (sig->Argument() != 0) updateNextButton();
  }
}


void UserViewEvent::slot_sentevent(ICQEvent *e)
{
#ifdef QT_PROTOCOL_PLUGIN
  if (e->PPID() != m_nPPID || (strcmp(e->Id(), m_szId) != 0)) return;
#else
  if (e->Uin() != m_nUin) return;
#endif
  if (!mainwin->m_bMsgChatView)
    (void) new MsgViewItem(e->GrabUserEvent(), codec, msgView);
}


//=====UserSendCommon========================================================
#ifdef QT_PROTOCOL_PLUGIN
UserSendCommon::UserSendCommon(CICQDaemon *s, CSignalManager *theSigMan,
                               CMainWindow *m, const char *szId,
                               unsigned long nPPID, QWidget* parent, const char* name)
  : UserEventCommon(s, theSigMan, m, szId, nPPID, parent, name)
{
  grpMR = NULL;
  tmpWidgetWidth = 0;

  QAccel *a = new QAccel( this );
  a->connectItem(a->insertItem(Key_Escape), this, SLOT(cancelSend()));

  QGroupBox *box = new QGroupBox(this);
  top_lay->addWidget(box);
  QBoxLayout *vlay = new QVBoxLayout(box, 10, 5);
  QBoxLayout *hlay = new QHBoxLayout(vlay);
  chkSendServer = new QCheckBox(tr("Se&nd through server"), box);
  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
  chkSendServer->setChecked(u->SendServer() || (u->StatusOffline() && u->SocketDesc() == -1));

  if( (u->GetInGroup(GROUPS_SYSTEM, GROUP_INVISIBLE_LIST)) ||
      (u->Port() == 0 && u->SocketDesc() == -1))
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
  cmbSendType->insertItem(tr("Message"));
  cmbSendType->insertItem(tr("URL"));
  cmbSendType->insertItem(tr("Chat Request"));
  cmbSendType->insertItem(tr("File Transfer"));
  cmbSendType->insertItem(tr("Contact List"));
  cmbSendType->insertItem(tr("SMS"));
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
    //TODO in CMessageViewWidget
    mleHistory = new CMessageViewWidget(strtoul(m_szId, (char **)NULL, 10), mainwin, splView);
#if QT_VERSION >= 300
    connect(mleHistory, SIGNAL(viewurl(QWidget*, QString)), mainwin, SLOT(slot_viewurl(QWidget *, QString)));
#endif
    connect (mainwin, SIGNAL(signal_sentevent(ICQEvent *)), mleHistory, SLOT(addMsg(ICQEvent *)));
    //splView->setResizeMode(mleHistory, QSplitter::FollowSizeHint);
  }
  mleSend = new MLEditWrap(true, splView, true);
  if (mainwin->m_bMsgChatView)
  {
    splView->setResizeMode(mleSend, QSplitter::KeepSize);
    mleSend->resize(mleSend->width(), 90);
  }
  setTabOrder(mleSend, btnSend);
  setTabOrder(btnSend, btnCancel);
  icqColor.SetToDefault();
  mleSend->setBackground(QColor(icqColor.BackRed(), icqColor.BackGreen(), icqColor.BackBlue()));
  mleSend->setForeground(QColor(icqColor.ForeRed(), icqColor.ForeGreen(), icqColor.ForeBlue()));
  connect (mleSend, SIGNAL(signal_CtrlEnterPressed()), btnSend, SIGNAL(clicked()));
  connect(this, SIGNAL(updateUser(CICQSignal*)), mainwin, SLOT(slot_updatedUser(CICQSignal*)));
}
#endif

UserSendCommon::UserSendCommon(CICQDaemon *s, CSignalManager *theSigMan,
                               CMainWindow *m, unsigned long _nUin, QWidget* parent, const char* name)
  : UserEventCommon(s, theSigMan, m, _nUin, parent, name)
{
  grpMR = NULL;
  tmpWidgetWidth = 0;

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
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
  chkSendServer->setChecked(u->SendServer() || (u->StatusOffline() && u->SocketDesc() == -1));

  if( (u->GetInGroup(GROUPS_SYSTEM, GROUP_INVISIBLE_LIST)) ||
      (u->Port() == 0 && u->SocketDesc() == -1))
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
  cmbSendType->insertItem(tr("Message"));
  cmbSendType->insertItem(tr("URL"));
  cmbSendType->insertItem(tr("Chat Request"));
  cmbSendType->insertItem(tr("File Transfer"));
  cmbSendType->insertItem(tr("Contact List"));
  cmbSendType->insertItem(tr("SMS"));
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
    mleHistory = new CMessageViewWidget(_nUin, mainwin, splView);
#if QT_VERSION >= 300
    connect(mleHistory, SIGNAL(viewurl(QWidget*, QString)), mainwin, SLOT(slot_viewurl(QWidget *, QString)));
#endif
    connect (mainwin, SIGNAL(signal_sentevent(ICQEvent *)), mleHistory, SLOT(addMsg(ICQEvent *)));
    //splView->setResizeMode(mleHistory, QSplitter::FollowSizeHint);
  }
  mleSend = new MLEditWrap(true, splView, true);
  if (mainwin->m_bMsgChatView)
  {
    splView->setResizeMode(mleSend, QSplitter::KeepSize);
    mleSend->resize(mleSend->width(), 90);
  }
  setTabOrder(mleSend, btnSend);
  setTabOrder(btnSend, btnCancel);
  icqColor.SetToDefault();
  mleSend->setBackground(QColor(icqColor.BackRed(), icqColor.BackGreen(), icqColor.BackBlue()));
  mleSend->setForeground(QColor(icqColor.ForeRed(), icqColor.ForeGreen(), icqColor.ForeBlue()));
  connect (mleSend, SIGNAL(signal_CtrlEnterPressed()), btnSend, SIGNAL(clicked()));
  connect(this, SIGNAL(updateUser(CICQSignal*)), mainwin, SLOT(slot_updatedUser(CICQSignal*)));
}


UserSendCommon::~UserSendCommon()
{
}

#if QT_VERSION >= 300
//-----UserSendCommon::windowActivationChange--------------------------------
void UserSendCommon::windowActivationChange(bool oldActive)
{
#ifdef QT_PROTOCOL_PLUGIN
  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
#else
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
#endif

  if (isActiveWindow() && mainwin->m_bMsgChatView &&
      (!mainwin->userEventTabDlg ||
       (mainwin->userEventTabDlg &&
	(!mainwin->userEventTabDlg->tabExists(this) ||
	 mainwin->userEventTabDlg->tabIsSelected(this)))))
    {
    if (u != NULL && u->NewMessages() > 0)
    {
      std::vector<int> idList;
      for (unsigned short i = 0; i < u->NewMessages(); i++)
      {
        CUserEvent *e = u->EventPeek(i);
        if (e->Direction() == D_RECEIVER && e->SubCommand() == ICQ_CMDxSUB_MSG)
          idList.push_back(e->Id());
      }

      for (unsigned short i = 0; i < idList.size(); i++)
        u->EventClearId(idList[i]);
    }
  }
  gUserManager.DropUser(u);
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
#ifdef QT_PROTOCOL_PLUGIN
  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
#else
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
#endif
  bool autoSecure = ( u->AutoSecure() && gLicqDaemon->CryptoEnabled() &&
                      (u->SecureChannelSupport() == SECURE_CHANNEL_SUPPORTED ) &&
                      !chkSendServer->isChecked() && !u->Secure());
  gUserManager.DropUser( u );
  disconnect( btnSend, SIGNAL( clicked() ), this, SLOT( trySecure() ) );
  connect(btnSend, SIGNAL(clicked()), this, SLOT(sendButton()));
  if ( autoSecure ) {
#ifdef QT_PROTOCOL_PLUGIN
    QWidget *w = new KeyRequestDlg(sigman, m_szId, m_nPPID);
#else
    QWidget* w = new KeyRequestDlg(sigman, m_nUin);
#endif
    connect(w, SIGNAL( destroyed() ), this, SLOT( sendButton() ) );
  }
  else
    sendButton();
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


//-----UserSendCommon::changeEventType---------------------------------------
void UserSendCommon::changeEventType(int id)
{
  UserSendCommon* e = NULL;
  QWidget *parent = NULL;
#if QT_VERSION >= 300
  if (mainwin->userEventTabDlg && mainwin->userEventTabDlg->tabExists(this))
      parent = mainwin->userEventTabDlg;
#endif
  switch(id)
  {
#ifdef QT_PROTOCOL_PLUGIN
  case 0:
    e = new UserSendMsgEvent(server, sigman, mainwin, m_szId, m_nPPID);
    break;
  case 1:
    e = new UserSendUrlEvent(server, sigman, mainwin, m_szId, m_nPPID);
    break;
  case 2:
    e = new UserSendChatEvent(server, sigman, mainwin, m_szId, m_nPPID);
    break;
  case 3:
    e = new UserSendFileEvent(server, sigman, mainwin, m_szId, m_nPPID);
    break;
  case 4:
    e = new UserSendContactEvent(server, sigman, mainwin, m_szId, m_nPPID);
    break;
  case 5:
    e = new UserSendSmsEvent(server, sigman, mainwin, m_szId, m_nPPID);
    break;
#else
  case 0:
    e = new UserSendMsgEvent(server, sigman, mainwin, m_nUin, parent);
    break;
  case 1:
    e = new UserSendUrlEvent(server, sigman, mainwin, m_nUin, parent);
    break;
  case 2:
    e = new UserSendChatEvent(server, sigman, mainwin, m_nUin, parent);
    break;
  case 3:
    e = new UserSendFileEvent(server, sigman, mainwin, m_nUin, parent);
    break;
  case 4:
    e = new UserSendContactEvent(server, sigman, mainwin, m_nUin, parent);
    break;
  case 5:
    e = new UserSendSmsEvent(server, sigman, mainwin, m_nUin, parent);
    break;
#endif
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

    disconnect(this, SIGNAL(finished(unsigned long)), mainwin, SLOT(slot_sendfinished(unsigned long)));
    mainwin->slot_sendfinished(m_nUin);
    connect(e, SIGNAL(finished(unsigned long)), mainwin, SLOT(slot_sendfinished(unsigned long)));
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
 *	which enables the user to collect users for mass messaging.
 */
void UserSendCommon::massMessageToggled(bool b)
{
	if (b)
	{
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
#ifdef QT_PROTOCOL_PLUGIN
      //TODO in CMMUserView
      lstMultipleRecipients = new CMMUserView(mainwin->colInfo, mainwin->m_bShowHeader,
        strtoul(m_szId, (char **)NULL, 10), mainwin, grpMR);
#else
			lstMultipleRecipients = new CMMUserView(mainwin->colInfo, mainwin->m_bShowHeader,
																	m_nUin, mainwin, grpMR);
#endif
			lstMultipleRecipients->setFixedWidth(mainwin->UserView()->width());
		}
		grpMR->show();
	}
	else
	{
		if (grpMR != NULL)
		{
			int grpMRWidth = grpMR->width();

			grpMR->hide();
			grpMR->close(true);
			grpMR = NULL;

			// resize the widget to it's origin width.
			// This is a workaroung and not perfect, but resize() does not
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
  if(!mainwin->m_bManualNewUser) {
#ifdef QT_PROTOCOL_PLUGIN
    ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_W);
#else
    ICQUser* u = gUserManager.FetchUser(m_nUin, LOCK_W);
#endif

    if(u->NewUser())
    {
      u->SetNewUser(false);
      gUserManager.DropUser(u);
#ifdef QT_PROTOCOL_PLUGIN
      CICQSignal s(SIGNAL_UPDATExUSER, USER_BASIC, m_szId, m_nPPID);
#else
      CICQSignal s(SIGNAL_UPDATExUSER, USER_BASIC, m_nUin);
#endif
      emit updateUser(&s);
    }
    else
      gUserManager.DropUser(u);
  }

  unsigned long icqEventTag = 0;
  if (m_lnEventTag.size())
    icqEventTag = m_lnEventTag.front();

  if (icqEventTag != 0)
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
    {
      RetrySend(e, false, ICQ_TCPxMSG_NORMAL);
    }
    return;
  }

  ICQUser *u = NULL;
  CUserEvent *ue = e->UserEvent();
  QString msg;
  if (e->SubResult() == ICQ_TCPxACK_RETURN)
  {
#ifdef QT_PROTOCOL_PLUGIN
    u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_W);
#else
    u = gUserManager.FetchUser(m_nUin, LOCK_W);
#endif
    msg = tr("%1 is in %2 mode:\n%3\nSend...")
             .arg(codec->toUnicode(u->GetAlias())).arg(u->StatusStr())
             .arg(codec->toUnicode(u->AutoResponse()));

    u->SetShowAwayMsg( false );
    gUserManager.DropUser(u);
    switch (QueryUser(this, msg, tr("Urgent"), tr(" to Contact List"), tr("Cancel")))
    {
      case 0:
        RetrySend(e, true, ICQ_TCPxMSG_URGENT);
        break;
      case 1:
        RetrySend(e, true, ICQ_TCPxMSG_LIST);
        break;
      case 2:
        break;
    }
    return;
  }
  else if (e->SubResult() == ICQ_TCPxACK_REFUSE)
  {
#ifdef QT_PROTOCOL_PLUGIN
    u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
#else
    u = gUserManager.FetchUser(m_nUin, LOCK_R);
#endif
    msg = tr("%1 refused %2, send through server")
          .arg(codec->toUnicode(u->GetAlias())).arg(EventDescription(ue));
    InformUser(this, msg);
    gUserManager.DropUser(u);
    return;
  }
  else
  {
    emit autoCloseNotify();
    if (sendDone(e))
    {
      emit mainwin->signal_sentevent(e);

      if (mainwin->m_bMsgChatView && mleHistory != NULL) {
        mleHistory->GotoEnd();
        resetSettings();
      } else
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

  switch(e->SubCommand() & ~ICQ_CMDxSUB_FxMULTIREC)
  {
    case ICQ_CMDxSUB_MSG:
    {
      CEventMsg *ue = (CEventMsg *)e->UserEvent();
      // create initial strings (implicit copying, no allocation impact :)
      char *tmp = gTranslator.NToRN(codec->fromUnicode(ue->Message()));
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
          message = ue->Message();
          messageRaw = codec->fromUnicode(message);
        }

#ifdef QT_PROTOCOL_PLUGIN
        icqEventTag = server->ProtoSendMessage(m_szId, m_nPPID, messageRaw.data(),
          bOnline, nLevel, false, &icqColor);
#else
        icqEventTag = server->icqSendMessage(m_nUin, messageRaw.data(),
           bOnline, nLevel, false, &icqColor);
#endif
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
#ifdef QT_PROTOCOL_PLUGIN
      icqEventTag = server->ProtoSendUrl(m_szId, m_nPPID, ue->Url(),
        ue->Description(), bOnline, nLevel, false, &icqColor);
#else
      icqEventTag = server->icqSendUrl(m_nUin, ue->Url(), ue->Description(),
         bOnline, nLevel, false, &icqColor);
#endif
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

#ifdef QT_PROTOCOL_PLUGIN
      //TODO in the daemon
      icqEventTag = server->icqSendContactList(strtoul(m_szId, (char **)NULL, 10),
        uins, bOnline, nLevel, false, &icqColor);
#else
      icqEventTag = server->icqSendContactList(m_nUin, uins, bOnline,
       nLevel, false, &icqColor);
#endif
      break;
    }
    case ICQ_CMDxSUB_CHAT:
    {
      CEventChat *ue = (CEventChat *)e->UserEvent();
#ifdef QT_PROTOCOL_PLUGIN
      //TODO in the daemon
      icqEventTag = server->icqChatRequest(strtoul(m_szId, (char **)NULL, 10),
        ue->Reason(), nLevel, !bOnline);
#else
      icqEventTag = server->icqChatRequest(m_nUin, ue->Reason(), nLevel, !bOnline);
#endif
      break;
    }
    case ICQ_CMDxSUB_FILE:
    {
      CEventFile *ue = (CEventFile *)e->UserEvent();
      ConstFileList filelist(ue->FileList());
#ifdef QT_PROTOCOL_PLUGIN
      //TODO in the daemon
      icqEventTag = server->icqFileTransfer(strtoul(m_szId, (char **)NULL, 10),
        ue->Filename(), ue->FileDescription(), filelist, nLevel, !bOnline);
#else
      icqEventTag = server->icqFileTransfer(m_nUin, ue->Filename(),
         ue->FileDescription(), filelist, nLevel, !bOnline); // try through server
#endif
      break;
    }
    case ICQ_CMDxSUB_SMS:
    {
      CEventSms *ue = (CEventSms *)e->UserEvent();
#ifdef QT_PROTOCOL_PLUGIN
      //TODO in the daemon
      icqEventTag = server->icqSendSms(ue->Number(), ue->Message(),
        strtoul(m_szId, (char **)NULL, 0));
#else
      icqEventTag = server->icqSendSms(ue->Number(), ue->Message(), m_nUin);
#endif
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
      CUserEvent *e = 0;
      if (sig->Argument() > 0 && mleHistory)
      {
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
#ifdef QT_PROTOCOL_PLUGIN
  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
#else
  ICQUser* u = gUserManager.FetchUser(m_nUin, LOCK_R);
#endif
  bool send_ok = true;
  if (chkSendServer->isChecked() && ( u->Secure() || u->AutoSecure()) )
  {
    if (!QueryUser(this, tr("Warning: Message can't be sent securely\n"
                            "through the server!"),
                   tr("Send anyway"), tr("Cancel")))
    {
      send_ok = false;
    }
    else
      u->SetAutoSecure( false );
  }
  gUserManager.DropUser(u);
  return send_ok;
}

//=====UserSendMsgEvent======================================================
#ifdef QT_PROTOCOL_PLUGIN
UserSendMsgEvent::UserSendMsgEvent(CICQDaemon *s, CSignalManager *theSigMan,
  CMainWindow *m, const char *szId, unsigned long nPPID, QWidget *parent)
  : UserSendCommon(s, theSigMan, m, szId, nPPID, parent, "UserSendMsgEvent")
{
  QBoxLayout* lay = new QVBoxLayout(mainWidget);
  lay->addWidget(splView);
  if (!m->m_bMsgChatView) mleSend->setMinimumHeight(150);
  mleSend->setFocus ();

  m_sBaseTitle += tr(" - Message");
  setCaption(m_sBaseTitle);
  cmbSendType->setCurrentItem(0);
}
#endif

UserSendMsgEvent::UserSendMsgEvent(CICQDaemon *s, CSignalManager *theSigMan,
  CMainWindow *m, unsigned long nUin, QWidget *parent)
  : UserSendCommon(s, theSigMan, m, nUin, parent, "UserSendMsgEvent")
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
  cmbSendType->setCurrentItem(0);
}


UserSendMsgEvent::~UserSendMsgEvent()
{
}


//-----UserSendMsgEvent::sendButton------------------------------------------
void UserSendMsgEvent::sendButton()
{
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
        message = mleSend->text();
        messageRaw = codec->fromUnicode(message);
     }

     if (chkMass->isChecked())
     {
        CMMSendDlg *m = new CMMSendDlg(server, sigman, lstMultipleRecipients, this);
        m->go_message(message);
     }

#ifdef QT_PROTOCOL_PLUGIN
     icqEventTag = server->ProtoSendMessage(m_szId, m_nPPID, messageRaw.data(),
      chkSendServer->isChecked() ? false : true,
      chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL,
      chkMass->isChecked(), &icqColor);
#else
     icqEventTag = server->icqSendMessage(m_nUin, messageRaw.data(),
        chkSendServer->isChecked() ? false : true,
        chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL,
        chkMass->isChecked(), &icqColor);
#endif
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

#ifdef QT_PROTOCOL_PLUGIN
  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
#else
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
#endif
  if (u->Away() && u->ShowAwayMsg())
  {
    gUserManager.DropUser(u);
#ifdef QT_PROTOCOL_PLUGIN
    (void) new ShowAwayMsgDlg(NULL, NULL, m_szId, m_nPPID);
#else
    (void) new ShowAwayMsgDlg(NULL, NULL, m_nUin);
#endif
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

//=====UserSendUrlEvent======================================================
#ifdef QT_PROTOCOL_PLUGIN
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
  setCaption(m_sBaseTitle);
  cmbSendType->setCurrentItem(1);
}
#endif

UserSendUrlEvent::UserSendUrlEvent(CICQDaemon *s, CSignalManager *theSigMan,
                                   CMainWindow *m, unsigned long _nUin, QWidget* parent)
  : UserSendCommon(s, theSigMan, m, _nUin, parent, "UserSendUrlEvent")
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
  cmbSendType->setCurrentItem(1);
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
  if (edtItem->text().stripWhiteSpace().isEmpty())
  {
    InformUser(this, tr("No URL specified"));
    return;
  }

  if (!UserSendCommon::checkSecure()) return;

  if (chkMass->isChecked())
  {
    CMMSendDlg *m = new CMMSendDlg(server, sigman, lstMultipleRecipients, this);
    int r = m->go_url(edtItem->text(), mleSend->text());
    delete m;
    if (r != QDialog::Accepted) return;
  }

  unsigned long icqEventTag;
#ifdef QT_PROTOCOL_PLUGIN
  icqEventTag = server->ProtoSendUrl(m_szId, m_nPPID, edtItem->text().latin1(), codec->fromUnicode(mleSend->text()),
#else
  icqEventTag = server->icqSendUrl(m_nUin, edtItem->text().latin1(), codec->fromUnicode(mleSend->text()),
#endif
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

#ifdef QT_PROTOCOL_PLUGIN
  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
#else
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
#endif
  if (u->Away() && u->ShowAwayMsg())
  {
    gUserManager.DropUser(u);
#ifdef QT_PROTOCOL_PLUGIN
    (void) new ShowAwayMsgDlg(NULL, NULL, m_szId, m_nPPID);
#else
    (void) new ShowAwayMsgDlg(NULL, NULL, m_nUin);
#endif
  }
  else
    gUserManager.DropUser(u);

  return true;
}


//=====UserSendFileEvent=====================================================
#ifdef QT_PROTOCOL_PLUGIN
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
  setCaption(m_sBaseTitle);
  cmbSendType->setCurrentItem(3);
}
#endif

UserSendFileEvent::UserSendFileEvent(CICQDaemon *s, CSignalManager *theSigMan,
                                     CMainWindow *m, unsigned long _nUin, QWidget* parent)
  : UserSendCommon(s, theSigMan, m, _nUin, parent, "UserSendFileEvent")
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
  cmbSendType->setCurrentItem(3);
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
  if (edtItem->text().stripWhiteSpace().isEmpty())
  {
    WarnUser(this, tr("You must specify a file to transfer!"));
    return;
  }

  unsigned long icqEventTag;
#ifdef QT_PROTOCOL_PLUGIN
  //TODO in daemon
  icqEventTag = server->icqFileTransfer(strtoul(m_szId, (char **)NULL, 10),
     codec->fromUnicode(edtItem->text()),
     codec->fromUnicode(mleSend->text()), m_lFileList,
     chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL,
     chkSendServer->isChecked());
#else
  icqEventTag = server->icqFileTransfer(m_nUin, codec->fromUnicode(edtItem->text()),
     codec->fromUnicode(mleSend->text()), m_lFileList,
     chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL,
     chkSendServer->isChecked());
#endif
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
  if (!e->ExtendedAck()->Accepted())
  {
#ifdef QT_PROTOCOL_PLUGIN
    ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
#else
    ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
#endif
    QString result = tr("File transfer with %2 refused:\n%3").arg(codec->toUnicode(u->GetAlias())).arg(e->ExtendedAck()->Response());
    gUserManager.DropUser(u);
    InformUser(this, result);
  }
  else
  {
    CEventFile *f = (CEventFile *)e->UserEvent();
#ifdef QT_PROTOCOL_PLUGIN
    CFileDlg *fileDlg = new CFileDlg(m_szId, m_nPPID, server);
#else
    CFileDlg *fileDlg = new CFileDlg(m_nUin, server);
#endif
    fileDlg->SendFiles(f->FileList(), e->ExtendedAck()->Port());
  }

  return true;
}


//=====UserSendChatEvent=====================================================
#ifdef QT_PROTOCOL_PLUGIN
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
  setCaption(m_sBaseTitle);
  cmbSendType->setCurrentItem(2);
}
#endif

UserSendChatEvent::UserSendChatEvent(CICQDaemon *s, CSignalManager *theSigMan,
                                     CMainWindow *m, unsigned long _nUin, QWidget* parent)
  : UserSendCommon(s, theSigMan, m, _nUin, parent, "UserSendChatEvent")
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
  cmbSendType->setCurrentItem(2);
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
  unsigned long icqEventTag;

  if (m_nMPChatPort == 0)
#ifdef QT_PROTOCOL_PLUGIN
  {
    //TODO in daemon
    icqEventTag = server->icqChatRequest(strtoul(m_szId, (char **)NULL, 10),
                                         codec->fromUnicode(mleSend->text()),
                                         chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL,
					 chkSendServer->isChecked());
  }
  else
    icqEventTag = server->icqMultiPartyChatRequest(strtoul(m_szId, (char **)NULL, 10),
                                                   codec->fromUnicode(mleSend->text()), codec->fromUnicode(m_szMPChatClients),
                                                   m_nMPChatPort,
                                                   chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL,
						   chkSendServer->isChecked() );
#else
    icqEventTag = server->icqChatRequest(m_nUin,
                                         codec->fromUnicode(mleSend->text()),
                                         chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL,
					 chkSendServer->isChecked());
  else
    icqEventTag = server->icqMultiPartyChatRequest(m_nUin,
                                                   codec->fromUnicode(mleSend->text()), codec->fromUnicode(m_szMPChatClients),
                                                   m_nMPChatPort,
                                                   chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL,
						   chkSendServer->isChecked() );
#endif

  m_lnEventTag.push_back(icqEventTag);

  UserSendCommon::sendButton();
}


//-----UserSendChatEvent::sendDone-------------------------------------------
bool UserSendChatEvent::sendDone(ICQEvent *e)
{
  if (!e->ExtendedAck()->Accepted())
  {
#ifdef QT_PROTOCOL_PLUGIN
    ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
#else
    ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
#endif
    QString result = tr("Chat with %2 refused:\n%3").arg(codec->toUnicode(u->GetAlias()))
                     .arg(e->ExtendedAck()->Response());
    gUserManager.DropUser(u);
    InformUser(this, result);

  }
  else
  {
    CEventChat *c = (CEventChat *)e->UserEvent();
    if (c->Port() == 0)  // If we requested a join, no need to do anything
    {
#ifdef QT_PROTOCOL_PLUGIN
      ChatDlg *chatDlg = new ChatDlg(m_szId, m_nPPID, server, mainwin);
#else
      ChatDlg *chatDlg = new ChatDlg(m_nUin, server, mainwin);
#endif
      chatDlg->StartAsClient(e->ExtendedAck()->Port());
    }
  }

  return true;
}


//=====UserSendContactEvent==================================================
#ifdef QT_PROTOCOL_PLUGIN
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
                                m_nUin, mainwin, mainWidget);
  lay->addWidget(lstContacts);

  m_sBaseTitle += tr(" - Contact List");
  setCaption(m_sBaseTitle);
  cmbSendType->setCurrentItem(4);
}
#endif

UserSendContactEvent::UserSendContactEvent(CICQDaemon *s, CSignalManager *theSigMan,
                                           CMainWindow *m, unsigned long _nUin, QWidget* parent)
  : UserSendCommon(s, theSigMan, m, _nUin, parent, "UserSendContactEvent")
{
  delete mleSend; mleSend = NULL;

  QBoxLayout* lay = new QVBoxLayout(mainWidget);
  lay->addWidget(splView);
  QLabel* lblContact =  new QLabel(tr("Drag Users Here - Right Click for Options"), mainWidget);
  lay->addWidget(lblContact);

  lstContacts = new CMMUserView(mainwin->colInfo, mainwin->m_bShowHeader,
                                m_nUin, mainwin, mainWidget);
  lay->addWidget(lstContacts);

  m_sBaseTitle += tr(" - Contact List");
#if QT_VERSION >= 300
  if (mainwin->userEventTabDlg &&
      mainwin->userEventTabDlg->tabIsSelected(this))
    mainwin->userEventTabDlg->setCaption(m_sBaseTitle);
#endif
  setCaption(m_sBaseTitle);
  cmbSendType->setCurrentItem(4);
}


UserSendContactEvent::~UserSendContactEvent()
{
}


//TODO Fix this for new protocol plugin
void UserSendContactEvent::sendButton()
{
  CMMUserViewItem *i = static_cast<CMMUserViewItem*>(lstContacts->firstChild());
  UinList uins;

  while (i)
  {
    uins.push_back(i->Uin());
    i = static_cast<CMMUserViewItem *>(i->nextSibling());
  }

  if (uins.size() == 0)
    return;

  if (!UserSendCommon::checkSecure()) return;

  if (chkMass->isChecked())
  {
    CMMSendDlg *m = new CMMSendDlg(server, sigman, lstMultipleRecipients, this);
    int r = m->go_contact(uins);
    delete m;
    if (r != QDialog::Accepted) return;
  }

  unsigned long icqEventTag;
  icqEventTag = server->icqSendContactList(m_nUin, uins,
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

#ifdef QT_PROTOCOL_PLUGIN
  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
#else
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
#endif
  if (u->Away() && u->ShowAwayMsg())
  {
    gUserManager.DropUser(u);
#ifdef QT_PROTOCOL_PLUGIN
    (void) new ShowAwayMsgDlg(NULL, NULL, m_szId, m_nPPID);
#else
    (void) new ShowAwayMsgDlg(NULL, NULL, m_nUin);
#endif
  }
  else
    gUserManager.DropUser(u);

  return true;
}


//-----UserSendContactEvent::setContact--------------------------------------
//TODO for new protocol plugin
void UserSendContactEvent::setContact(unsigned long Uin, const QString&)
{
  ICQUser* u = gUserManager.FetchUser(Uin, LOCK_R);

  if(u != NULL)
  {
    (void) new CMMUserViewItem(u, lstContacts);

    gUserManager.DropUser(u);
  }
}


//=====UserSendSmsEvent======================================================
#ifdef QT_PROTOCOL_PLUGIN
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
  setCaption(m_sBaseTitle);
  cmbSendType->setCurrentItem(5);
}
#endif

UserSendSmsEvent::UserSendSmsEvent(CICQDaemon *s, CSignalManager *theSigMan,
  CMainWindow *m, unsigned long nUin, QWidget *parent)
  : UserSendCommon(s, theSigMan, m, nUin, parent, "UserSendSmsEvent")
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

  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_W);
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
  cmbSendType->setCurrentItem(5);
}

//-----UserSendSmsEvent::~UserSendSmsEvent-------------------------------------
UserSendSmsEvent::~UserSendSmsEvent()
{
}

//-----UserSendSmsEvent::sendButton--------------------------------------------
void UserSendSmsEvent::sendButton()
{
  unsigned long icqEventTag = 0;
  if (m_lnEventTag.size())
    icqEventTag = m_lnEventTag.front();


  // do nothing if a command is already being processed
  if (icqEventTag != 0) return;

  if(!mleSend->edited() &&
     !QueryUser(this, tr("You didn't edit the SMS.\n"
                         "Do you really want to send it?"), tr("&Yes"), tr("&No")))
    return;

  // don't let the user send empty messages
  if (mleSend->text().stripWhiteSpace().isEmpty()) return;

#ifdef QT_PROTOCOL_PLUGIN
  //TODO in daemon
  icqEventTag = server->icqSendSms(nfoNumber->text().latin1(), mleSend->text().utf8().data(),
    strtoul(m_szId, (char **)NULL, 10));
#else
  icqEventTag = server->icqSendSms(nfoNumber->text().latin1(), mleSend->text().utf8().data(), m_nUin);
#endif
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
