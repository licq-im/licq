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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef USE_KDE
#include <kfiledialog.h>
#else
#include <qfiledialog.h>
#endif

#include <sys/timeb.h>

#include <qhbox.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qsplitter.h>
#include <qtabbar.h>
#include <qtabwidget.h>
#include <qtextview.h>
#include <qwidgetstack.h>
#include <qstylesheet.h>
#include <qlayout.h>
#include <qdatetime.h>
#include <qtimer.h>
#include <qspinbox.h>

#include "icqfunctions.h"
#include "chatacceptdlg.h"
#include "chatdlg.h"
#include "ewidgets.h"
#include "messagebox.h"
#include "fileacceptdlg.h"
#include "filedlg.h"
#include "showawaymsgdlg.h"
#include "countrycodes.h"
#include "languagecodes.h"
#include "log.h"
#include "sigman.h"
#include "eventdesc.h"
#include "gui-defines.h"

#include "user.h"
#include "mledit.h"
#include "icqevent.h"
#include "icqd.h"

#define NUM_MSG_PER_HISTORY 20
#define COLOR_SENT "blue"
#define COLOR_RECEIVED "red"

//#define TEST_POS

unsigned short ICQFunctions::s_nX = 100;
unsigned short ICQFunctions::s_nY = 100;

//-----ICQFunctions::constructor---------------------------------------------
ICQFunctions::ICQFunctions(CICQDaemon *s, CSignalManager *theSigMan,
                           unsigned long _nUin,
                           bool isAutoClose, QWidget *parent, const char *name)
  : QWidget(parent, name)
{
  server = s;
  sigman = theSigMan;
  icqEventTag = NULL;
  m_nUin = _nUin;
  m_bOwner = (m_nUin == gUserManager.OwnerUin());

  CreateReadEventTab();
  CreateSendEventTab();
  CreateGeneralInfoTab();
  CreateMoreInfoTab();
  CreateWorkInfoTab();
  CreateAboutTab();
  CreateHistoryTab();

  QBoxLayout* lay = new QVBoxLayout(this, 8);

  QBoxLayout *layt = new QHBoxLayout(lay, 8);
  layt->addWidget(new QLabel(tr("Status:"), this));
  nfoStatus = new CInfoField(this, true);
  layt->addWidget(nfoStatus);
  layt->addWidget(new QLabel(tr("Time:"), this));
  nfoTimezone = new CInfoField(this, true);
  layt->addWidget(nfoTimezone);

  tabs = new QTabWidget(this);
  lay->addWidget(tabs);

  tabs->addTab(fcnTab[TAB_READ], tabLabel[TAB_READ]);
  tabs->addTab(fcnTab[TAB_SEND], tabLabel[TAB_SEND]);
  tabs->addTab(fcnTab[TAB_GENERALINFO], tabLabel[TAB_GENERALINFO]);
  tabs->addTab(fcnTab[TAB_MOREINFO], tabLabel[TAB_MOREINFO]);
  tabs->addTab(fcnTab[TAB_WORKINFO], tabLabel[TAB_WORKINFO]);
  tabs->addTab(fcnTab[TAB_ABOUT], tabLabel[TAB_ABOUT]);
  tabs->addTab(fcnTab[TAB_HISTORY], tabLabel[TAB_HISTORY]);

  QBoxLayout* l = new QHBoxLayout(lay, 8);

  chkAutoClose = new QCheckBox(tr("A&uto Close"), this);
  chkAutoClose->setChecked(isAutoClose);
  l->addWidget(chkAutoClose);
  l->addSpacing(15);
  l->addStretch(1);

  int bw = 75;
  btnSave = new QPushButton(tr("&Save"), this);
  btnOk = new QPushButton(tr("&Ok"), this);
  btnCancel = new QPushButton(tr("&Close"), this);
  bw = QMAX(bw, btnSave->sizeHint().width());
  bw = QMAX(bw, btnOk->sizeHint().width());
  bw = QMAX(bw, btnCancel->sizeHint().width());
  btnSave->setFixedWidth(bw);
  btnOk->setFixedWidth(bw);
  btnCancel->setFixedWidth(bw);
  l->addWidget(btnSave);
  l->addSpacing(7);
  l->addWidget(btnOk);
  l->addWidget(btnCancel);

  setTabOrder(mleSend, btnOk);

  connect (mleSend, SIGNAL(signal_CtrlEnterPressed()), btnOk, SIGNAL(clicked()));
  connect (tabs, SIGNAL(selected(const QString &)), this, SLOT(tabSelected(const QString &)));
  connect (sigman, SIGNAL(signal_doneUserFcn(ICQEvent *)), this, SLOT(doneFcn(ICQEvent *)));
  connect (sigman, SIGNAL(signal_updatedUser(unsigned long, unsigned long)),
           this, SLOT(slot_updatedUser(unsigned long, unsigned long)));
  connect (btnCancel, SIGNAL(clicked()), this, SLOT(close()));
  connect (btnOk, SIGNAL(clicked()), this, SLOT(callFcn()));
  connect (btnSave, SIGNAL(clicked()), this, SLOT(save()));


#ifdef TEST_POS
  printf("constructor: %d %d\n", x(), y());
#endif
}


void ICQFunctions::CreateReadEventTab()
{
  tabLabel[TAB_READ] = tr("&View");
  QVBox* p = new QVBox(this, tabLabel[TAB_READ]);
  p->setMargin(8);
  fcnTab[TAB_READ] = p;

  splRead = new QSplitter(QSplitter::Vertical, fcnTab[TAB_READ]);
  msgView = new MsgView(splRead);
  mleRead = new MLEditWrap(true, splRead);
  mleRead->setReadOnly(true);
  splRead->setOpaqueResize(true);
  splRead->setResizeMode(msgView, QSplitter::KeepSize);
  connect (msgView, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(printMessage(QListViewItem *)));
}


void ICQFunctions::CreateSendEventTab()
{
  tabLabel[TAB_SEND] = tr("S&end");
  fcnTab[TAB_SEND] = new QWidget(this, tabLabel[TAB_SEND]);
  QBoxLayout* selay = new QVBoxLayout(fcnTab[TAB_SEND], 8);

  grpCmd = new QButtonGroup(1, Vertical, tr("Select Function"), fcnTab[TAB_SEND]);
  rdbMsg = new QRadioButton(tr("Message"), grpCmd);
  rdbUrl = new QRadioButton(tr("URL"), grpCmd);
  rdbChat = new QRadioButton(tr("Chat Request"), grpCmd);
  rdbFile = new QRadioButton(tr("File Transfer"), grpCmd);
  connect(grpCmd, SIGNAL(clicked(int)), this, SLOT(specialFcn(int)));
#if QT_VERSION < 210
  QWidget* dummy_w = new QWidget(grpCmd);
  dummy_w->setMinimumHeight(2);
#endif
  selay->addWidget(grpCmd);

  mleSend = new MLEditWrap(true, fcnTab[TAB_SEND]);
  mleSend->setMinimumHeight(150);
  selay->addWidget(mleSend);
  selay->setStretchFactor(mleSend, 1);

  grpOpt = new QGroupBox(2, Horizontal, fcnTab[TAB_SEND]);
  selay->addWidget(grpOpt);
  lblItem = new QLabel(grpOpt);
  edtItem = new QLineEdit(grpOpt);
#if QT_VERSION < 210
  QWidget* dummy_w2 = new QWidget(grpOpt);
  dummy_w2->setMinimumHeight(2);
#endif
  grpOpt->hide();

  QGroupBox* boxOptions = new QGroupBox(2, Horizontal, fcnTab[TAB_SEND]);
  selay->addWidget(boxOptions);

  chkSendServer = new QCheckBox(tr("Send through server"), boxOptions);
  chkUrgent = new QCheckBox(tr("Urgent"), boxOptions);
#if QT_VERSION < 210
  QWidget* dummy_w3 = new QWidget(boxOptions);
  dummy_w3->setMinimumHeight(2);
#endif

  selay->activate();
}


void ICQFunctions::CreateGeneralInfoTab()
{
  unsigned short CR = 0;

  tabLabel[TAB_GENERALINFO] = tr("General");
  fcnTab[TAB_GENERALINFO] = new QWidget(this, tabLabel[TAB_GENERALINFO]);
  QWidget *p = fcnTab[TAB_GENERALINFO];

  QGridLayout *lay = new QGridLayout(p, 10, 5, 10, 5);
  lay->addColSpacing(2, 10);
  lay->setRowStretch(9, 1);

  lay->addWidget(new QLabel(tr("Alias:"), p), CR, 0);
  nfoAlias = new CInfoField(p, false);
  lay->addWidget(nfoAlias, CR, 1);
  lay->addWidget(new QLabel(tr("Online:"), p), CR, 3);
  nfoLastOnline = new CInfoField(p, true);
  lay->addWidget(nfoLastOnline, CR, 4);

  lay->addWidget(new QLabel(tr("UIN:"), p), ++CR, 0);
  nfoUin = new CInfoField(p, true);
  lay->addWidget(nfoUin, CR, 1);
  lay->addWidget(new QLabel(tr("IP:"), p), CR, 3);
  nfoIp = new CInfoField(p, true);
  lay->addWidget(nfoIp, CR, 4);

  lay->addWidget(new QLabel(tr("Name:"), p), ++CR, 0);
  nfoFirstName = new CInfoField(p, !m_bOwner);
  lay->addWidget(nfoFirstName, CR, 1);
  nfoLastName = new CInfoField(p, !m_bOwner);
  lay->addMultiCellWidget(nfoLastName, CR, CR, 2, 4);

  lay->addWidget(new QLabel(tr("EMail 1:"), p), ++CR, 0);
  nfoEmail1 = new CInfoField(p, !m_bOwner);
  lay->addMultiCellWidget(nfoEmail1, CR, CR, 1, 4);

  lay->addWidget(new QLabel(tr("EMail 2:"), p), ++CR, 0);
  nfoEmail2 = new CInfoField(p, !m_bOwner);
  lay->addMultiCellWidget(nfoEmail2, CR, CR, 1, 4);

  lay->addWidget(new QLabel(tr("State:"), p), ++CR, 0);
  nfoState = new CInfoField(p, !m_bOwner);
  lay->addWidget(nfoState, CR, 1);
  lay->addWidget(new QLabel(tr("City:"), p), CR, 3);
  nfoCity = new CInfoField(p, !m_bOwner);
  lay->addWidget(nfoCity, CR, 4);

  lay->addWidget(new QLabel(tr("Address:"), p), ++CR, 0);
  nfoAddress = new CInfoField(p, !m_bOwner);
  lay->addWidget(nfoAddress, CR, 1);
  lay->addWidget(new QLabel(tr("Phone:"), p), CR, 3);
  nfoPhone = new CInfoField(p, !m_bOwner);
  lay->addWidget(nfoPhone, CR, 4);

  lay->addWidget(new QLabel(tr("Country:"), p), ++CR, 0);
  if (m_bOwner)
  {
    cmbCountry = new CEComboBox(true, fcnTab[TAB_GENERALINFO]);
    cmbCountry->insertItem(tr("Unspecified"));
    cmbCountry->setMaximumWidth(cmbCountry->sizeHint().width()+20);
    for (unsigned short i = 0; i < NUM_COUNTRIES; i++)
      cmbCountry->insertItem(GetCountryByIndex(i)->szName);
    lay->addWidget(cmbCountry, CR, 1);
  }
  else
  {
    nfoCountry = new CInfoField(p, !m_bOwner);
    lay->addWidget(nfoCountry, CR, 1);
  }
  lay->addWidget(new QLabel(tr("Zip:"), p), CR, 3);
  nfoZipCode = new CInfoField(p, !m_bOwner);
  lay->addWidget(nfoZipCode, CR, 4);

  lay->addWidget(new QLabel(tr("Cellular:"), p), ++CR, 0);
  nfoCellular = new CInfoField(p, !m_bOwner);
  lay->addWidget(nfoCellular, CR, 1);
  lay->addWidget(new QLabel(tr("Fax:"), p), CR, 3);
  nfoFax = new CInfoField(p, !m_bOwner);
  lay->addWidget(nfoFax, CR, 4);

}


void ICQFunctions::CreateMoreInfoTab()
{
  unsigned short CR = 0;
  tabLabel[TAB_MOREINFO] = tr("More");
  fcnTab[TAB_MOREINFO] = new QWidget(this, tabLabel[TAB_MOREINFO]);
  QWidget *p = fcnTab[TAB_MOREINFO];

  QGridLayout *lay = new QGridLayout(p, 6, 5, 10, 5);
  lay->addColSpacing(2, 10);
  lay->setRowStretch(5, 1);

  lay->addWidget(new QLabel(tr("Age:"), p), CR, 0);
  nfoAge = new CInfoField(p, !m_bOwner);
  lay->addWidget(nfoAge, CR, 1);
  lay->addWidget(new QLabel(tr("Gender:"), p), CR, 3);
  if (m_bOwner)
  {
    cmbGender = new CEComboBox(true, p);
    cmbGender->insertItem(tr("Unspecified"), GENDER_UNSPECIFIED);
    cmbGender->insertItem(tr("Female"), GENDER_FEMALE);
    cmbGender->insertItem(tr("Male"), GENDER_MALE);
    lay->addWidget(cmbGender, CR, 4);
  }
  else
  {
    nfoGender = new CInfoField(p, true);
    lay->addWidget(nfoGender, CR, 4);
  }

  lay->addWidget(new QLabel(tr("Homepage:"), p), ++CR, 0);
  nfoHomepage = new CInfoField(p, !m_bOwner);
  lay->addMultiCellWidget(nfoHomepage, CR, CR, 1, 4);

  lay->addWidget(new QLabel(tr("Birthday:"), p), ++CR, 0);
  if (m_bOwner)
  {
    QHBox* w = new QHBox(p);
    w->setSpacing(8);
    QLabel* lblDay = new QLabel(tr(" Day:"), w);
    lblDay->setAlignment(AlignRight | AlignVCenter | SingleLine);
    spnBirthDay = new QSpinBox(1, 31, 1, w);
    QLabel* lblMonth = new QLabel(tr(" Month:"), w);
    lblMonth->setAlignment(AlignRight | AlignVCenter | SingleLine);
    spnBirthMonth = new QSpinBox(1, 12, 1, w);
    QLabel* lblYear = new QLabel(tr(" Year:"), w);
    lblYear->setAlignment(AlignRight | AlignVCenter | SingleLine);
    spnBirthYear = new QSpinBox(1900, 2020, 1, w);
    lay->addMultiCellWidget(w, CR, CR, 1, 4);
  }
  else
  {
    nfoBirthday = new CInfoField(p, !m_bOwner);
    lay->addMultiCellWidget(nfoBirthday, CR, CR, 1, 4);
  }

  if (m_bOwner)
  {
    lay->addWidget(new QLabel(tr("Language 1:"), p), ++CR, 0);
    cmbLanguage[0] = new CEComboBox(true, p);
    lay->addWidget(cmbLanguage[0], CR, 1);
    lay->addWidget(new QLabel(tr("Language 2:"), p), CR, 3);
    cmbLanguage[1] = new CEComboBox(true, p);
    lay->addWidget(cmbLanguage[1], CR, 4);

    lay->addWidget(new QLabel(tr("Language 3:"), p), ++CR, 0);
    cmbLanguage[2] = new CEComboBox(true, p);
    lay->addWidget(cmbLanguage[2], CR, 1);

    for (unsigned short i = 0; i < 3; i++)
    {
      for (unsigned short j = 0; j < NUM_LANGUAGES; j++)
        cmbLanguage[i]->insertItem(GetLanguageByIndex(j)->szName);
    }
  }
  else
  {
    lay->addWidget(new QLabel(tr("Language 1:"), p), ++CR, 0);
    nfoLanguage[0] = new CInfoField(p, !m_bOwner);
    lay->addWidget(nfoLanguage[0], CR, 1);
    lay->addWidget(new QLabel(tr("Language 2:"), p), CR, 3);
    nfoLanguage[1] = new CInfoField(p, !m_bOwner);
    lay->addWidget(nfoLanguage[1], CR, 4);

    lay->addWidget(new QLabel(tr("Language 3:"), p), ++CR, 0);
    nfoLanguage[2] = new CInfoField(p, !m_bOwner);
    lay->addWidget(nfoLanguage[2], CR, 1);
  }
}


void ICQFunctions::CreateWorkInfoTab()
{
  unsigned short CR = 0;
  tabLabel[TAB_WORKINFO] = tr("Work");
  fcnTab[TAB_WORKINFO] = new QWidget(this, tabLabel[TAB_WORKINFO]);
  QWidget *p = fcnTab[TAB_WORKINFO];

  QGridLayout *lay = new QGridLayout(p, 8, 5, 10, 5);
  lay->addColSpacing(2, 10);
  lay->setRowStretch(7, 1);

  lay->addWidget(new QLabel(tr("Name:"), p), CR, 0);
  nfoCompanyName = new CInfoField(p, !m_bOwner);
  lay->addMultiCellWidget(nfoCompanyName, CR, CR, 1, 4);

  lay->addWidget(new QLabel(tr("Department:"), p), ++CR, 0);
  nfoCompanyDepartment = new CInfoField(p, !m_bOwner);
  lay->addMultiCellWidget(nfoCompanyDepartment, CR, CR, 1, 4);

  lay->addWidget(new QLabel(tr("Position:"), p), ++CR, 0);
  nfoCompanyPosition = new CInfoField(p, !m_bOwner);
  lay->addMultiCellWidget(nfoCompanyPosition, CR, CR, 1, 4);

  lay->addWidget(new QLabel(tr("City:"), p), ++CR, 0);
  nfoCompanyCity = new CInfoField(p, !m_bOwner);
  lay->addWidget(nfoCompanyCity, CR, 1);
  lay->addWidget(new QLabel(tr("State:"), p), CR, 3);
  nfoCompanyState = new CInfoField(p, !m_bOwner);
  lay->addWidget(nfoCompanyState, CR, 4);

  lay->addWidget(new QLabel(tr("Address:"), p), ++CR, 0);
  nfoCompanyAddress = new CInfoField(p, !m_bOwner);
  lay->addMultiCellWidget(nfoCompanyAddress, CR, CR, 1, 4);

  lay->addWidget(new QLabel(tr("Phone:"), p), ++CR, 0);
  nfoCompanyPhone = new CInfoField(p, !m_bOwner);
  lay->addWidget(nfoCompanyPhone, CR, 1);
  lay->addWidget(new QLabel(tr("Fax:"), p), CR, 3);
  nfoCompanyFax = new CInfoField(p, !m_bOwner);
  lay->addWidget(nfoCompanyFax, CR, 4);

  lay->addWidget(new QLabel(tr("Homepage:"), p), CR, 0);
  nfoCompanyHomepage = new CInfoField(p, !m_bOwner);
  lay->addMultiCellWidget(nfoCompanyHomepage, CR, CR, 1, 4);
}


void ICQFunctions::CreateAboutTab()
{
  tabLabel[TAB_ABOUT] = tr("About");
  QVBox* p = new QVBox(this, tabLabel[TAB_ABOUT]);
  p->setMargin(8);
  p->setSpacing(8);
  fcnTab[TAB_ABOUT] = p;

  lblAbout = new QLabel(tr("About:"), p);
  mleAbout = new MLEditWrap(true, p);
}


void ICQFunctions::CreateHistoryTab()
{
  unsigned short CR = 0;

  tabLabel[TAB_HISTORY] = tr("History");
  fcnTab[TAB_HISTORY] = new QWidget(this, tabLabel[TAB_HISTORY]);
  QWidget *p = fcnTab[TAB_HISTORY];

  QGridLayout *lay = new QGridLayout(p, 3, 2, 8, 0);

  mleHistory = new QTextView(p);
  lay->addMultiCellWidget(mleHistory, CR, CR, 0, 1);

  lblHistory = new QLabel(p);
  CR++;
  lay->addMultiCellWidget(lblHistory, CR, CR, 0, 1);

  lay->addWidget(new QLabel(tr("History File:"), p), ++CR, 0);
  nfoHistory = new CInfoField(p, true);
  lay->addWidget(nfoHistory, CR, 1);
}

//-----ICQFunctions::keyPressEvent----------------------------------------------
void ICQFunctions::keyPressEvent(QKeyEvent *e)
{
  if (e->key() == Key_Escape) {
    close();
    return;
  } else if (tabs->currentPage() == fcnTab[TAB_READ]) {
    tabs->showPage(fcnTab[TAB_SEND]);
    return;
  }
  QWidget::keyPressEvent(e);
}

//-----ICQFunctions::setupTabs--------------------------------------------------
void ICQFunctions::setupTabs(int index)
{
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);

  // read tab
  for (short i = u->NewMessages() - 1; i >= 0; i--)
     (void) new MsgViewItem(u->GetEvent(i), i, msgView);

  // print the first event if it's a message
  MsgViewItem *e = (MsgViewItem *)msgView->firstChild();
  if (e != NULL && (e->msg->SubCommand() == ICQ_CMDxSUB_MSG || e->msg->Command() == 0))
  {
     msgView->setSelected(e, true);
     gUserManager.DropUser(u);
     printMessage(e);
     u = gUserManager.FetchUser(m_nUin, LOCK_R);
  }

  // Send tab
  rdbFile->setEnabled(!u->StatusOffline());
  rdbChat->setEnabled(!u->StatusOffline());
  if (chkSendServer->isEnabled())
  {
    chkSendServer->setChecked(u->SendServer() || (u->StatusOffline() && u->SocketDesc() == -1));
  }
  if (u->Ip() == 0)
  {
    chkSendServer->setChecked(true);
    chkSendServer->setEnabled(false);
  }
  rdbMsg->setChecked(true);
  specialFcn(0);
  tabs->setTabEnabled(fcnTab[TAB_SEND], !m_bOwner);

  // Info tabs
  SetGeneralInfo(u);
  SetMoreInfo(u);
  SetWorkInfo(u);
  SetAbout(u);

  bool bNewUser = u->NewUser();
  gUserManager.DropUser(u);

  // mark the user as no longer new if they are new
  if (bNewUser)
  {
    u = gUserManager.FetchUser(m_nUin, LOCK_W);
    u->SetNewUser(false);
    gUserManager.DropUser(u);
    emit signal_updatedUser(USER_BASIC, m_nUin);
  }
  move (s_nX, s_nY);
  show();
#ifdef TEST_POS
  printf("just shown: %d %d\n", x(), y());
  move(s_nX, s_nY);
  printf("just shown 2: %d %d\n", x(), y());
#endif

  switch(index) {
  case mnuUserView:
    tabs->showPage(fcnTab[TAB_READ]);
    break;
  case mnuUserSendMsg:
    tabs->showPage(fcnTab[TAB_SEND]);
    rdbMsg->setChecked(true);
    specialFcn(0);
    break;
  case mnuUserSendUrl:
    tabs->showPage(fcnTab[TAB_SEND]);
    rdbUrl->setChecked(true);
    specialFcn(1);
    break;
  case mnuUserSendChat:
    tabs->showPage(fcnTab[TAB_SEND]);
    rdbChat->setChecked(true);
    specialFcn(2);
    break;
  case mnuUserSendFile:
    tabs->showPage(fcnTab[TAB_SEND]);
    rdbFile->setChecked(true);
    specialFcn(3);
    break;
  case mnuUserGeneral:
    tabs->showPage(fcnTab[TAB_GENERALINFO]);
    break;
  case mnuUserMore:
    tabs->showPage(fcnTab[TAB_MOREINFO]);
    break;
  case mnuUserWork:
    tabs->showPage(fcnTab[TAB_WORKINFO]);
    break;
  case mnuUserAbout:
    tabs->showPage(fcnTab[TAB_ABOUT]);
    break;
  case mnuUserHistory:
    tabs->showPage(fcnTab[TAB_HISTORY]);
    break;
  default:
    gLog.Warn("%sInternal Error: ICQFunctions::setupTabs(): Invalid index (%d).\n",
              L_WARNxSTR, index);
  }

}


//-----ICQFunctions::setGeneralInfo-----------------------------------------------
void ICQFunctions::SetGeneralInfo(ICQUser *u)
{
  char buf[32];
  bool bDropUser = false;

  if (u == NULL)
  {
    u = gUserManager.FetchUser(m_nUin, LOCK_R);
    bDropUser = true;
  }

  nfoAlias->setData(u->GetAlias());
  nfoStatus->setData(u->StatusStr(buf));
  nfoFirstName->setData(u->GetFirstName());
  nfoLastName->setData(u->GetLastName());
  nfoEmail1->setData(u->GetEmail1());
  nfoEmail2->setData(u->GetEmail2());
  nfoUin->setData(u->Uin());
  nfoIp->setData(u->IpPortStr(buf));
  if (m_bOwner)
  {
    if (u->GetCountryCode() == COUNTRY_UNSPECIFIED)
      cmbCountry->setCurrentItem(0);
    else
    {
      const SCountry *c = GetCountryByCode(u->GetCountryCode());
      if (c == NULL)
        cmbCountry->setCurrentItem(0);
      else
        cmbCountry->setCurrentItem(c->nIndex + 1);
    }
  }
  else
  {
    if (u->GetCountryCode() == COUNTRY_UNSPECIFIED)
      nfoCountry->setData(tr("Unspecified"));
    else
    {
      const SCountry *c = GetCountryByCode(u->GetCountryCode());
      if (c == NULL)
        nfoCountry->setData(tr("Unknown (%1)").arg(u->GetCountryCode()));
      else  // known
        nfoCountry->setData(c->szName);
    }
  }
  nfoCity->setData(u->GetCity());
  nfoState->setData(u->GetState());
  nfoPhone->setData(u->GetPhoneNumber());
  nfoFax->setData(u->GetFaxNumber());
  nfoCellular->setData(u->GetCellularNumber());
  nfoZipCode->setData(u->GetZipCode());
  struct timeb tb;
  ftime(&tb);
  localtime(&tb.time);
  m_nRemoteTimeOffset = timezone - u->GetTimezone() * 1800;
  QDateTime t;
  t.setTime_t(tb.time + m_nRemoteTimeOffset);
  nfoTimezone->setData(tr("%1 (GMT%1%1%1)")
                       .arg(t.time().toString())
                       .arg(u->GetTimezone() < 0 ? "" : "+")
                       .arg(u->GetTimezone() / 2)
                       .arg(u->GetTimezone() % 2 ? "30" : "00") );
  tmrTime = new QTimer(this);
  connect(tmrTime, SIGNAL(timeout()), this, SLOT(slot_updatetime()));
  tmrTime->start(3000);

  if (!u->StatusOffline())
    nfoLastOnline->setData(tr("Now"));
  else if (u->LastOnline() == 0)
    nfoLastOnline->setData(tr("Unknown"));
  else
  {
    t.setTime_t(u->LastOnline());
    QString ds = t.toString();
    ds.truncate(ds.length() - 8);
    nfoLastOnline->setData(ds);
  }

  m_sBaseTitle = QString::fromLocal8Bit(u->GetAlias()) + " (" +
                 QString::fromLocal8Bit(u->GetFirstName()) + " " +
                 QString::fromLocal8Bit(u->GetLastName())+ ")";
  setCaption(m_sBaseTitle);
  setIconText(u->GetAlias());

  if (bDropUser) gUserManager.DropUser(u);
}


void ICQFunctions::slot_updatetime()
{
  QDateTime t;
  t.setTime_t(time(NULL) + m_nRemoteTimeOffset);
  nfoTimezone->setText(nfoTimezone->text().replace(0, t.time().toString().length(), t.time().toString()));
}


//-----ICQFunctions::SetMoreInfo-------------------------------------------------
void ICQFunctions::SetMoreInfo(ICQUser *u)
{
  bool bDropUser = false;

  if (u == NULL)
  {
    u = gUserManager.FetchUser(m_nUin, LOCK_R);
    bDropUser = true;
  }

  // Gender
  if (m_bOwner)
  {
    cmbGender->setCurrentItem(u->GetGender());
  }
  else
  {
    if (u->GetGender() == 1)
      nfoGender->setData(tr("Female"));
    else if (u->GetGender() == 2)
      nfoGender->setData(tr("Male"));
    else
      nfoGender->setData(tr("Unspecified"));
  }

  // Age
  if (u->GetAge() == AGE_UNSPECIFIED)
    nfoAge->setData(tr("Unspecified"));
  else
    nfoAge->setData(u->GetAge());

  // Birthday
  if (m_bOwner)
  {
    spnBirthDay->setValue((unsigned short)u->GetBirthDay());
    spnBirthMonth->setValue((unsigned short)u->GetBirthMonth());
    spnBirthYear->setValue((unsigned short)u->GetBirthYear() + 1900);
  }
  else
  {
    if (u->GetBirthMonth() == 0 || u->GetBirthDay() == 0)
    {
      nfoBirthday->setData(tr("Unspecified"));
    }
    else
    {
      QDate d(u->GetBirthYear() + 1900, u->GetBirthMonth(), u->GetBirthDay());
      nfoBirthday->setData(d.toString());
    }
  }
  nfoHomepage->setData(u->GetHomepage());
  for (unsigned short i = 0; i < 3; i++)
  {
    const SLanguage *l = GetLanguageByCode(u->GetLanguage(i));
    if (m_bOwner)
    {
      if (l == NULL)
        cmbLanguage[i]->setCurrentItem(NUM_LANGUAGES - 1);
      else
        cmbLanguage[i]->setCurrentItem(l->nIndex);
    }
    else
    {
      if (l == NULL)
        nfoLanguage[i]->setData(tr("Unknown (%1)").arg(u->GetLanguage(i)));
      else  // known
        nfoLanguage[i]->setData(l->szName);
    }
  }

  if (bDropUser) gUserManager.DropUser(u);
}


//-----ICQFunctions::SetWorkInfo-------------------------------------------------
void ICQFunctions::SetWorkInfo(ICQUser *u)
{
  bool bDropUser = false;

  if (u == NULL)
  {
    u = gUserManager.FetchUser(m_nUin, LOCK_R);
    bDropUser = true;
  }

  nfoCompanyName->setData(u->GetCompanyName());
  nfoCompanyDepartment->setData(u->GetCompanyDepartment());
  nfoCompanyPosition->setData(u->GetCompanyPosition());
  nfoCompanyCity->setData(u->GetCompanyCity());
  nfoCompanyState->setData(u->GetCompanyState());
  nfoCompanyAddress->setData(u->GetCompanyAddress());
  nfoCompanyPhone->setData(u->GetCompanyPhoneNumber());
  nfoCompanyFax->setData(u->GetCompanyFaxNumber());
  nfoCompanyHomepage->setData(u->GetCompanyHomepage());

  if (bDropUser) gUserManager.DropUser(u);
}


//-----ICQFunctions::SetAbout-------------------------------------------------
void ICQFunctions::SetAbout(ICQUser *u)
{
  bool bDropUser = false;

  if (u == NULL)
  {
    u = gUserManager.FetchUser(m_nUin, LOCK_R);
    bDropUser = true;
  }

  mleAbout->setText(QString::fromLocal8Bit(u->GetAbout()));

  if (bDropUser) gUserManager.DropUser(u);
}



//-----ICQFunctions::SendUrl---------------------------------------------------
void ICQFunctions::SendUrl(const char *url, const char *desc)
{
  tabs->showPage(fcnTab[1]);
  rdbUrl->setChecked(true);
  specialFcn(1);
  edtItem->setText(url);
  mleSend->setText(desc);
}

//-----ICQFunctions::SendFile--------------------------------------------------
void ICQFunctions::SendFile(const char *file, const char *desc)
{
  tabs->showPage(fcnTab[1]);
  rdbFile->setChecked(true);
  specialFcn(3);
  edtItem->setText(file);
  mleSend->setText(desc);
}

//-----ICQFunctions::tabSelected-----------------------------------------------
void ICQFunctions::tabSelected(const QString &tab)
{
  if (tab == tabLabel[TAB_SEND])
  {
     mleSend->setFocus();
     btnOk->setText(tr("&Send"));
     btnSave->hide();
     currentTab = TAB_SEND;
  }
  else if (tab == tabLabel[TAB_GENERALINFO])
  {
     btnOk->setText(tr("Update"));
     btnSave->setText(tr("Save"));
     btnSave->show();
     currentTab = TAB_GENERALINFO;
  }
  else if (tab == tabLabel[TAB_READ])
  {
     btnOk->setText(tr("Ok"));
     btnSave->setText(tr("&Quote"));
     m_bOwner ? btnSave->hide() : btnSave->show();
     msgView->triggerUpdate();
     currentTab = TAB_READ;
  }
  else if (tab == tabLabel[TAB_MOREINFO])
  {
     btnOk->setText(tr("Update"));
     btnSave->setText(tr("Save"));
     btnSave->show();
     currentTab = TAB_MOREINFO;
  }
  else if (tab == tabLabel[TAB_WORKINFO])
  {
     btnOk->setText(tr("Update"));
     btnSave->setText(tr("Save"));
     btnSave->show();
     currentTab = TAB_WORKINFO;
  }
  else if (tab == tabLabel[TAB_ABOUT])
  {
     btnOk->setText(tr("Update"));
     btnSave->setText(tr("Save"));
     btnSave->show();
     currentTab = TAB_ABOUT;
  }
  else if (tab == tabLabel[TAB_HISTORY])
  {
     if (m_lHistoryList.size() == 0) SetupHistory();  // if no history, then get it
     btnOk->setText(tr("Next ->"));
     btnSave->setText(tr("<- Prev"));
     btnSave->show();
     currentTab = TAB_HISTORY;
  }
  else
  {
     btnOk->setText(tr("Ok"));
  }
#ifdef TEST_POS
  printf("just shown new page: %d %d\n", x(), y());
  move(s_nX, s_nY);
  printf("just shown new page 2: %d %d\n", x(), y());
#endif
}


//-----slot_updatedUser--------------------------------------------------------
void ICQFunctions::slot_updatedUser(unsigned long _nUpdateType, unsigned long _nUin)
{
  if (m_nUin != _nUin) return;
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
  switch (_nUpdateType)
  {
  case USER_STATUS:
  {
    char szStatus[32];
    u->StatusStr(szStatus);
    nfoStatus->setData(szStatus);
    if (u->Ip() == 0)
    {
      chkSendServer->setChecked(true);
      chkSendServer->setEnabled(false);
    }
    else
    {
      chkSendServer->setEnabled(true);
    }
    break;
  }
  case USER_EVENTS:
  {
    MsgViewItem *e = (MsgViewItem *)msgView->firstChild();
    short index = -1;
    if (e != NULL)
    {
      do
      {
        if (e->index > index) index = e->index;
        e = (MsgViewItem *)e->nextSibling();
      } while (e != NULL);
    }
    index++;
    e = new MsgViewItem(u->GetEvent(index), index, msgView);
    msgView->ensureItemVisible(e);
    break;
  }
  case USER_GENERAL:
  case USER_BASIC:
  case USER_EXT:
    SetGeneralInfo(u);
    break;
  case USER_MORE:
    SetMoreInfo(u);
    break;
  case USER_WORK:
    SetWorkInfo(u);
    break;
  case USER_ABOUT:
    SetAbout(u);
    break;
  }
  gUserManager.DropUser(u);
}


//-----ICQFunctions::printMessage----------------------------------------------
void ICQFunctions::printMessage(QListViewItem *e)
{
  if(!e)
    return;

  CUserEvent *m = ((MsgViewItem *)e)->msg;
  mleRead->setText(QString::fromLocal8Bit(m->Text()));
  if (m->Command() == ICQ_CMDxTCP_START || m->Command() == ICQ_CMDxRCV_SYSxMSGxONLINE)
  {
    switch (m->SubCommand())
    {
    case ICQ_CMDxSUB_CHAT:  // accept or refuse a chat request
      (void) new CChatAcceptDlg(server, m_nUin, m->Sequence());
      break;
    case ICQ_CMDxSUB_FILE:  // accept or refuse a file transfer
      (void) new CFileAcceptDlg(server, m_nUin, (CEventFile *)m);
      break;
    case ICQ_CMDxSUB_URL:   // view a url
      if (server->getUrlViewer() != NULL && QueryUser(this, tr("View URL?"), tr("Yes"), tr("No")) )
      {
        char* szCmd = new char[strlen(server->getUrlViewer()) + strlen(((CEventUrl *)m)->Url()) + 8];
        sprintf(szCmd, "%s %s &", server->getUrlViewer(), ((CEventUrl *)m)->Url());
        if (system(szCmd) != 0) gLog.Error("%sView URL failed.\n", L_ERRORxSTR);
        delete szCmd;
      }
      break;
    case ICQ_CMDxSUB_REQxAUTH:
    {
      if (QueryUser(this, tr("Authorize?"), tr("Yes"), tr("No")))
        server->icqAuthorize( ((CEventAuthReq *)m)->Uin() );
      //...only ask if they aren't already there...
      ICQUser *u = gUserManager.FetchUser(((CEventAuthReq *)m)->Uin(), LOCK_R);
      if (u != NULL)
        gUserManager.DropUser(u);
      else
      {
        if (QueryUser(this, tr("Add?"), tr("Yes"), tr("No")))
          server->AddUserToList( ((CEventAuthReq *)m)->Uin());
      }
      break;
    }
    case ICQ_CMDxSUB_AUTHORIZED:
    {
      //...only ask if they aren't already there...
      ICQUser *u = gUserManager.FetchUser(((CEventAuth *)m)->Uin(), LOCK_R);
      if (u != NULL)
        gUserManager.DropUser(u);
      else
      {
        if (QueryUser(this, tr("Add?"), tr("Yes"), tr("No")))
          server->AddUserToList( ((CEventAuth *)m)->Uin());
      }
      break;
    }
    } // switch
  }  // if

  short index = ((MsgViewItem *)e)->index;
  if (index >= 0)   // the message had not been seen yet
  {
    ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_W);
    u->ClearEvent(index);
    gUserManager.DropUser(u);
    msgView->markRead(index);
    emit signal_updatedUser(USER_EVENTS, m_nUin);
  }
}


//-----ICQFunctions::save------------------------------------------------------
void ICQFunctions::save()
{
  switch (currentTab)
  {
  case TAB_READ:
    generateReply();
    break;
  case TAB_GENERALINFO:
    SaveGeneralInfo();
    break;
  case TAB_MOREINFO:
    SaveMoreInfo();
    break;
  case TAB_WORKINFO:
    SaveMoreInfo();
    break;
  case TAB_ABOUT:
    SaveAbout();
    break;
  case TAB_HISTORY:
    ShowHistoryPrev();
    break;
  default:
    gLog.Warn("%sInternal error: ICQFunctions::save(): invalid tab - %d.\n",
              L_WARNxSTR, currentTab);
    break;
  }
}


//-----ICQFunctions::SaveGeneralInfo-------------------------------------------
void ICQFunctions::SaveGeneralInfo()
{
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_W);
  u->SetEnableSave(false);

  u->SetAlias(nfoAlias->text().local8Bit());
  u->SetFirstName(nfoFirstName->text().local8Bit());
  u->SetLastName(nfoLastName->text().local8Bit());
  u->SetEmail1(nfoEmail1->text());
  u->SetEmail2(nfoEmail2->text());
  u->SetCity(nfoCity->text().local8Bit());
  u->SetState(nfoState->text().local8Bit());
  u->SetAddress(nfoAddress->text().local8Bit());
  u->SetPhoneNumber(nfoPhone->text());
  u->SetFaxNumber(nfoFax->text());
  u->SetCellularNumber(nfoCellular->text());
  u->SetZipCode(nfoZipCode->text().toULong());
  if (m_bOwner)
  {
    unsigned short i = cmbCountry->currentItem();
    if (i == 0)
      u->SetCountryCode(COUNTRY_UNSPECIFIED);
    else
      u->SetCountryCode(GetCountryByIndex(i - 1)->nCode);
  }

  u->SetEnableSave(true);
  u->SaveGeneralInfo();
  gUserManager.DropUser(u);
}


//-----ICQFunctions::SaveMoreInfo---------------------------------------------
void ICQFunctions::SaveMoreInfo()
{
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_W);
  u->SetEnableSave(false);

  u->SetAge(atol(nfoAge->text()));
  u->SetHomepage(nfoHomepage->text());
  //u->SetBirthYear();
  //u->SetBirthMonth();
  //u->SetBirthDay();
  if (m_bOwner)
  {
    u->SetGender(cmbGender->currentItem());
    for (unsigned short i = 0; i < 3; i++)
    {
      u->SetLanguage(i, GetLanguageByIndex(cmbLanguage[i]->currentItem())->nCode);
    }
  }

  u->SetEnableSave(true);
  u->SaveMoreInfo();
  gUserManager.DropUser(u);
}


//-----ICQFunctions::SaveAbout---------------------------------------------
void ICQFunctions::SaveAbout()
{
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_W);
  u->SetAbout(mleAbout->text().local8Bit());
  gUserManager.DropUser(u);
}


//-----ICQFunctions::SetupHistory--------------------------------------------
void ICQFunctions::SetupHistory()
{
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
  nfoHistory->setData(u->HistoryName());
  if (!u->GetHistory(m_lHistoryList))
  {
    mleHistory->setText(tr("Error loading history"));
  }
  else
  {
    m_iHistoryIter = m_lHistoryList.end();
    m_nHistoryIndex = m_lHistoryList.size();
    m_nHistoryShowing = 0;
    ShowHistoryPrev();
  }
  gUserManager.DropUser(u);
}


void ICQFunctions::ShowHistoryPrev()
{
  // Iterate back over what's currently showing
  while (m_nHistoryShowing > 0)
  {
    m_iHistoryIter--;
    m_nHistoryIndex--;
    m_nHistoryShowing--;
  }
  // Iterate far enough to show the previous messages
  for (unsigned short i = 0;
       i < NUM_MSG_PER_HISTORY && m_iHistoryIter != m_lHistoryList.begin();
       i++)
  {
    m_iHistoryIter--;
    m_nHistoryIndex--;
  }
  ShowHistory();
}

void ICQFunctions::ShowHistoryNext()
{
  if (m_iHistoryIter != m_lHistoryList.end())
  {
    ShowHistory();
  }
}

void ItalisizeLine(QString &t, QString pre, unsigned short inspos)
{
  int i = 0;
  while ( (i = t.find(pre, i)) != -1)
  {
    t.insert(i + inspos, "<i>");
    i = t.find("<br>", i + inspos);
    if (i == -1)
    {
      t.append("</i>");
      break;
    }
    else
      t.insert(i, "</i>");
  }
}


//-----ICQFunctions::ShowHistory--------------------------------------------
void ICQFunctions::ShowHistory()
{
  // Last check (should never be true)
  if (m_lHistoryList.size() == 0) return;

  QString s, st;
  QDateTime d;
  m_nHistoryShowing = 0;
  while (m_nHistoryShowing < NUM_MSG_PER_HISTORY)
  {
    d.setTime_t((*m_iHistoryIter)->Time());
    s.sprintf("<font color=\"%s\"><b>%s (%s) [%c%c%c]</b><br><br>",
              (*m_iHistoryIter)->Direction() == D_RECEIVER ? COLOR_RECEIVED : COLOR_SENT,
              (const char *)EventDescription(*m_iHistoryIter),
              (const char *)d.toString(),
              (*m_iHistoryIter)->IsDirect() ? 'D' : '-',
              (*m_iHistoryIter)->IsMultiRec() ? 'M' : '-',
              (*m_iHistoryIter)->IsUrgent() ? 'U' : '-');
    QString t = QStyleSheet::convertFromPlainText(QString::fromLocal8Bit( (*m_iHistoryIter)->Text() ));
    ItalisizeLine(t, "<p>&gt;", 3);
    ItalisizeLine(t, "<br>&gt;", 4);
    //s.append(QStyleSheet::convertFromPlainText(QString::fromLocal8Bit( (*m_iHistoryIter)->Text() )));
    s.append(t);
    s.append("</font><br><hr><br>");
    st.append(s);
    m_iHistoryIter++;
    m_nHistoryIndex++;
    m_nHistoryShowing++;
    if (m_iHistoryIter == m_lHistoryList.end())
    {
      break;
    }
  }
  lblHistory->setText(tr("[<font color=\"%1\">Received</font>] "
                         "[<font color=\"%2\">Sent</font>] "
                         "%3 to %4 of %5")
                      .arg(COLOR_RECEIVED).arg(COLOR_SENT)
                      .arg(m_nHistoryIndex - m_nHistoryShowing + 1)
                      .arg(m_nHistoryIndex)
                      .arg(m_lHistoryList.size()));
  mleHistory->setText(st);

}


//-----ICQFunctions::saveHistory---------------------------------------------
void ICQFunctions::saveHistory()
{
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
  u->SaveHistory(mleHistory->text().local8Bit());
  gUserManager.DropUser(u);
}


//-----ICQFunctions::generateReply-------------------------------------------
void ICQFunctions::generateReply()
{
  mleSend->clear();
  for (int i = 0; i < mleRead->numLines(); i++)
    mleSend->insertLine( QString("> ") + mleRead->textLine(i));
  mleSend->append("\n");
  mleSend->goToEnd();
  tabs->showPage(fcnTab[1]);
}


//-----ICQFunctions::specialFcn----------------------------------------------
void ICQFunctions::specialFcn(int theFcn)
{
  switch (theFcn)
  {
  case 0:
    grpOpt->hide();
    tabs->updateGeometry();
    mleSend->setEnabled(true);
    chkSendServer->setEnabled(true);
    break;
  case 1:
    lblItem->setText(tr("URL:"));
    grpOpt->show();
    tabs->updateGeometry();
    mleSend->setEnabled(true);
    chkSendServer->setEnabled(true);
    break;
  case 2:
    grpOpt->hide();
    tabs->updateGeometry();
    mleSend->setEnabled(true);
    chkSendServer->setChecked(false);
    chkSendServer->setEnabled(false);
    break;
  case 3:
    lblItem->setText(tr("Filename:"));
    grpOpt->show();
    tabs->updateGeometry();
    chkSendServer->setChecked(false);
    chkSendServer->setEnabled(false);
    mleSend->setEnabled(true);
#ifdef USE_KDE
      QStringList fl = KFileDialog::getOpenFileNames(NULL, NULL, this);
#else
      QStringList fl = QFileDialog::getOpenFileNames(NULL, NULL, this);
#endif
      QStringList::Iterator it;
      QString f;
      for( it = fl.begin(); it != fl.end(); it++ )
      {
        if (it != fl.begin())
          f += ", ";
        f += (*it);
      }
      edtItem->setText(f);
      break;
   }
}


//-----ICQFunctions::callFcn-------------------------------------------------
void ICQFunctions::callFcn()
{
  // do nothing if a command is already being processed
  if (icqEventTag != NULL) return;

  switch (currentTab)
  {
  case TAB_READ: close(); return;
  case TAB_SEND:
  {
    unsigned short nMsgLen = mleSend->text().length();
    if (nMsgLen > MAX_MESSAGE_SIZE)
     {
        if(!QueryUser(this, tr("Message is %1 characters, over the ICQ98 limit of %2.\n"
                               "Licq, ICQ99, and other clones support longer messages \n"
                               "however ICQ98 will not. Continue?").arg(nMsgLen).arg(MAX_MESSAGE_SIZE),
                      tr("Ok"), tr("Cancel")))
        {
          btnOk->setEnabled(true);
          break;
        }
     }
     if (rdbMsg->isChecked())  // send a message
     {
        ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_W);
        u->SetSendServer(chkSendServer->isChecked());
        gUserManager.DropUser(u);
        m_sProgressMsg = tr("Sending msg ");
        m_sProgressMsg += chkSendServer->isChecked() ? tr("through server") : tr("direct");
        m_sProgressMsg += "...";
        icqEventTag = server->icqSendMessage(m_nUin, mleSend->text().local8Bit(),
                                          chkSendServer->isChecked() ? false : true,
                                          chkUrgent->isChecked() ? true : false);
     }
     else if (rdbUrl->isChecked()) // send URL
     {
        m_sProgressMsg = tr("Sending URL ");
        m_sProgressMsg += chkSendServer->isChecked() ? tr("through server") : tr("direct");
        m_sProgressMsg += "...";
        icqEventTag = server->icqSendUrl(m_nUin, edtItem->text(), mleSend->text().local8Bit(),
                                      chkSendServer->isChecked() ? false : true,
                                      chkUrgent->isChecked() ? true : false);
     }
     else if (rdbChat->isChecked())   // send chat request
     {
        m_sProgressMsg = tr("Sending chat request ");
        m_sProgressMsg += chkSendServer->isChecked() ? tr("through server") : tr("direct");
        m_sProgressMsg += "...";
        icqEventTag = server->icqChatRequest(m_nUin, mleSend->text().local8Bit(),
                                          chkSendServer->isChecked() ? false : true,
                                          chkUrgent->isChecked() ? true : false);
     }
     else if (rdbFile->isChecked())   // send file transfer
     {
       if (edtItem->text().isEmpty())
       {
         WarnUser(this, tr("You must specify a file to transfer!"));
         break;
       }
       m_sProgressMsg = tr("Sending file transfer ");
       m_sProgressMsg += chkSendServer->isChecked() ? tr("through server") : tr("direct");
       m_sProgressMsg += "...";
       icqEventTag = server->icqFileTransfer(m_nUin, edtItem->text(), mleSend->text().local8Bit(),
                         chkSendServer->isChecked() ? false : true,
                         chkUrgent->isChecked() ? true : false);
     }

     if (icqEventTag == NULL)
        doneFcn(NULL);

     break;
  }

  case TAB_GENERALINFO:
     if ( m_bOwner && (!QueryUser(this, tr("Update local or server information?"), tr("Local"), tr("Server"))) )
     {
        m_sProgressMsg = tr("Updating server...");
        unsigned short i = cmbCountry->currentItem();
        unsigned short cc = ( i == 0 ? COUNTRY_UNSPECIFIED : GetCountryByIndex(i - 1)->nCode);
        icqEventTag = server->icqSetGeneralInfo(nfoAlias->text().local8Bit(),
                                             nfoFirstName->text().local8Bit(),
                                             nfoLastName->text().local8Bit(),
                                             nfoEmail1->text().local8Bit(),
                                             nfoEmail2->text().local8Bit(),
                                             nfoCity->text().local8Bit(),
                                             nfoState->text().local8Bit(),
                                             nfoPhone->text().local8Bit(),
                                             nfoFax->text().local8Bit(),
                                             nfoAddress->text().local8Bit(),
                                             nfoCellular->text().local8Bit(),
                                             nfoZipCode->text().toULong(),
                                             cc, false);
     }
     else
     {
        m_sProgressMsg = tr("Updating...");
        icqEventTag = server->icqRequestMetaInfo(m_nUin);
     }
     break;
  case TAB_MOREINFO:
     if ( m_bOwner && (!QueryUser(this, tr("Update local or server information?"), tr("Local"), tr("Server"))) )
     {
        m_sProgressMsg = tr("Updating server...");
        unsigned short i;
        i = cmbLanguage[0]->currentItem();
        unsigned short lc1 = GetLanguageByIndex(i)->nCode;
        i = cmbLanguage[1]->currentItem();
        unsigned short lc2 = GetLanguageByIndex(i)->nCode;
        i = cmbLanguage[2]->currentItem();
        unsigned short lc3 = GetLanguageByIndex(i)->nCode;
        icqEventTag = server->icqSetMoreInfo(nfoAge->text().toUShort(),
                                          cmbGender->currentItem(),
                                          nfoHomepage->text(),
                                          spnBirthYear->value() - 1900,
                                          spnBirthMonth->value(),
                                          spnBirthDay->value(),
                                          lc1, lc2, lc3);
     }
     else
     {
        m_sProgressMsg = tr("Updating...");
        icqEventTag = server->icqRequestMetaInfo(m_nUin);
     }
     break;
  case TAB_WORKINFO:
     if ( m_bOwner && (!QueryUser(this, tr("Update local or server information?"), tr("Local"), tr("Server"))) )
     {
        m_sProgressMsg = tr("Updating server...");
        icqEventTag = server->icqSetWorkInfo(nfoCompanyCity->text().local8Bit(),
                                          nfoCompanyState->text().local8Bit(),
                                          nfoCompanyPhone->text().local8Bit(),
                                          nfoCompanyFax->text().local8Bit(),
                                          nfoCompanyAddress->text().local8Bit(),
                                          nfoCompanyName->text().local8Bit(),
                                          nfoCompanyDepartment->text().local8Bit(),
                                          nfoCompanyPosition->text().local8Bit(),
                                          nfoCompanyHomepage->text().local8Bit());

     }
     else
     {
        m_sProgressMsg = tr("Updating...");
        icqEventTag = server->icqRequestMetaInfo(m_nUin);
     }
     break;
  case TAB_ABOUT:
     if ( m_bOwner && (!QueryUser(this, tr("Update local or server information?"), tr("Local"), tr("Server"))) )
     {
        m_sProgressMsg = tr("Updating server...");
        icqEventTag = server->icqSetAbout(mleAbout->text().local8Bit());
     }
     else
     {
        m_sProgressMsg = tr("Updating...");
        icqEventTag = server->icqRequestMetaInfo(m_nUin);
     }
     break;
  case TAB_HISTORY:
    ShowHistoryNext();
    break;
  }

  if (icqEventTag != NULL)
  {
    QString title = m_sBaseTitle + " [" + m_sProgressMsg + "]";
    setCaption(title);
    setCursor(waitCursor);
    btnOk->setEnabled(false);
    btnCancel->setText(tr("Cancel"));
  }
}


//-----ICQFunctions::doneFcn-------------------------------------------------
void ICQFunctions::doneFcn(ICQEvent *e)
{
  if ( (icqEventTag == NULL && e != NULL) ||
       (icqEventTag != NULL && !icqEventTag->Equals(e)) )
    return;

  bool isOk = (e != NULL && (e->m_eResult == EVENT_ACKED || e->m_eResult == EVENT_SUCCESS));
  bool bForceOpen = false;

  QString title, result;
  if (e == NULL)
  {
    result = tr("error");
  }
  else
  {
    switch (e->m_eResult)
    {
    case EVENT_ACKED:
    case EVENT_SUCCESS:
      result = tr("done");
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
  }
  title = m_sBaseTitle + " [" + m_sProgressMsg + result + "]";
  setCaption(title);
  setCursor(arrowCursor);
  btnOk->setEnabled(true);
  btnCancel->setText(tr("&Close"));
  delete icqEventTag;
  icqEventTag = NULL;

  if (e == NULL) return;

  if (!isOk)
  {
    if (e->m_nCommand == ICQ_CMDxTCP_START &&
        (e->m_nSubCommand == ICQ_CMDxSUB_MSG ||
         e->m_nSubCommand == ICQ_CMDxSUB_URL) &&
        QueryUser(this, tr("Direct send failed,\nsend through server?"), tr("Yes"), tr("No")) )
    {
      btnOk->setEnabled(false);
      btnCancel->setText(tr("Cancel"));
      chkSendServer->setChecked(true);
      ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_W);
      u->SetSendServer(true);
      gUserManager.DropUser(u);
      if (e->m_nSubCommand == ICQ_CMDxSUB_MSG)
      {
        CEventMsg *ue = (CEventMsg *)e->m_xUserEvent;
        m_sProgressMsg = tr("Sending msg through server...");
        icqEventTag = server->icqSendMessage(m_nUin, ue->Message(), false,
                                          false, 0);
      }
      else
      {
        CEventUrl *ue = (CEventUrl *)e->m_xUserEvent;
        m_sProgressMsg = tr("Sending URL through server...");
        icqEventTag = server->icqSendUrl(m_nUin, ue->Url(), ue->Description(), false,
                                      false, 0);
      }
      QString title = m_sBaseTitle + " [" + m_sProgressMsg + "]";
      setCaption(title);
    }
  }
  else
  {
    switch(e->m_nCommand)
    {
    case ICQ_CMDxTCP_START:
    {
      ICQUser *u = NULL;
      CUserEvent *ue = e->m_xUserEvent;
      QString msg;
      if (e->m_nSubResult == ICQ_TCPxACK_RETURN)
      {
        char status[32];
        u = gUserManager.FetchUser(m_nUin, LOCK_R);
        u->StatusStr(status);
        msg = tr("%1 is in %2 mode:\n%3\n[Send \"urgent\" to ignore]")
                 .arg(u->GetAlias()).arg(status).arg(u->AutoResponse());
        InformUser(this, msg);
        gUserManager.DropUser(u);
        bForceOpen = true;
      }
      else if (e->m_nSubResult == ICQ_TCPxACK_REFUSE)
      {
        u = gUserManager.FetchUser(m_nUin, LOCK_R);
        msg = tr("%1 refused %2, send through server.")
              .arg(u->GetAlias()).arg(EventDescription(ue));
        InformUser(this, msg);
        gUserManager.DropUser(u);
        bForceOpen = true;
      }
      else if (e->m_nSubCommand == ICQ_CMDxSUB_CHAT || e->m_nSubCommand == ICQ_CMDxSUB_FILE)
      {
        struct SExtendedAck *ea = e->m_sExtendedAck;
        if (ea == NULL || ue == NULL)
        {
          gLog.Error("%sInternal error: ICQFunctions::doneFcn(): chat or file request acknowledgement without extended result.\n", L_ERRORxSTR);
          return;
        }
        if (!ea->bAccepted)
        {
           u = gUserManager.FetchUser(m_nUin, LOCK_R);
           QString result;
           result.sprintf(tr("%s%1 with %2 refused:\n%s%3"), L_TCPxSTR, L_BLANKxSTR);
           result.arg(EventDescription(ue)).arg(u->GetAlias()).arg(ea->szResponse);
           gUserManager.DropUser(u);
           InformUser(this, result);
        }
        else
        {
          switch (e->m_nSubCommand)
          {
          case ICQ_CMDxSUB_CHAT:
          {
            ChatDlg *chatDlg = new ChatDlg(m_nUin, false, ea->nPort);
            chatDlg->show();
            break;
          }
          case ICQ_CMDxSUB_FILE:
          {
            CFileDlg *fileDlg = new CFileDlg(m_nUin,
                                             ((CEventFile *)ue)->Filename(),
                                             ((CEventFile *)ue)->FileSize(),
                                             false, ea->nPort);
            fileDlg->show();
            break;
          }
          default:
            break;
          } // case
        } // if accepted
      } // if file or chat
      else
      {
        u = gUserManager.FetchUser(m_nUin, LOCK_R);
        if (u->Away() && u->ShowAwayMsg()) {
          gUserManager.DropUser(u);
          (void) new ShowAwayMsgDlg(NULL, NULL, m_nUin);
        }
        else
          gUserManager.DropUser(u);

        mleSend->clear();
      }

      break;
    } // case
    case ICQ_CMDxSND_THRUxSERVER:
    {
      mleSend->clear();
      break;
    }

    case ICQ_CMDxSND_USERxGETINFO:
    case ICQ_CMDxSND_USERxGETDETAILS:
    case ICQ_CMDxSND_UPDATExBASIC:
    case ICQ_CMDxSND_UPDATExDETAIL:
    case ICQ_CMDxSND_META:
    {
      bForceOpen = true;
      break;
    }

    default:
      break;

    }

    if (chkAutoClose->isChecked() && !bForceOpen) close();
  }

}

void ICQFunctions::closeEvent(QCloseEvent *e)
{
#ifdef TEST_POS
  printf("close event: %d %d\n", x(), y());
#endif
  if (icqEventTag != NULL)
  {
    setCaption(m_sBaseTitle);
    server->CancelEvent(icqEventTag);
    delete icqEventTag;
    icqEventTag = NULL;
    btnOk->setEnabled(true);
    btnCancel->setText(tr("&Close"));
    setCursor(arrowCursor);
  }
  else
  {
    s_nX = x();
    s_nY = y();
    e->accept();
    ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_W);
    if (u != NULL)
    {
      u->fcnDlg = NULL;
      gUserManager.DropUser(u);
    }
    delete this;
  }
}


ICQFunctions::~ICQFunctions()
{
  delete icqEventTag;
}

#include "moc/moc_icqfunctions.h"
