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

#include <time.h>

#ifdef USE_KDE
#warning !!!! FIXME !!!!
#include <qfiledialog.h>
#include <kfiledialog.h>
#else
#include <qfiledialog.h>
#endif

#include <qhbox.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qsplitter.h>
#include <qtabbar.h>
#include <qtabwidget.h>
#include <qwidgetstack.h>
#include <qstylesheet.h>
#include <qlayout.h>
#include <qdatetime.h>
#include <qtimer.h>
#include <qspinbox.h>

#include "icqfunctions.h"
#include "chatdlg.h"
#include "ewidgets.h"
#include "messagebox.h"
#include "filedlg.h"
#include "showawaymsgdlg.h"
#include "countrycodes.h"
#include "languagecodes.h"
#include "log.h"
#include "sigman.h"
#include "editfile.h"
#include "eventdesc.h"
#include "gui-defines.h"
#include "refusedlg.h"
#include "forwarddlg.h"

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
                           CMainWindow *m,
                           unsigned long _nUin,
                           bool isAutoClose, QWidget *parent, const char *name)
  : QWidget(parent, name)
{
  server = s;
  mainwin = m;
  sigman = theSigMan;
  icqEventTag = NULL;
  m_nUin = _nUin;
  m_bOwner = (m_nUin == gUserManager.OwnerUin());
  m_xCurrentReadEvent = NULL;

  for (unsigned short i = 0; i < 8; i++)
  {
    tabList[i].loaded = false;
    tabList[i].tab = NULL;
  }

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

  tabs->addTab(tabList[TAB_READ].tab, tabList[TAB_READ].label);
  tabs->addTab(tabList[TAB_SEND].tab, tabList[TAB_SEND].label);
  tabs->addTab(tabList[TAB_GENERALINFO].tab, tabList[TAB_GENERALINFO].label);
  tabs->addTab(tabList[TAB_MOREINFO].tab, tabList[TAB_MOREINFO].label);
  tabs->addTab(tabList[TAB_WORKINFO].tab, tabList[TAB_WORKINFO].label);
  tabs->addTab(tabList[TAB_ABOUT].tab, tabList[TAB_ABOUT].label);
  tabs->addTab(tabList[TAB_HISTORY].tab, tabList[TAB_HISTORY].label);

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
#ifdef USE_SPOOFING
  connect (chkSpoof, SIGNAL(clicked()), this, SLOT(setSpoofed()));
#endif
  connect (tabs, SIGNAL(selected(const QString &)), this, SLOT(tabSelected(const QString &)));
  connect (sigman, SIGNAL(signal_doneUserFcn(ICQEvent *)), this, SLOT(doneFcn(ICQEvent *)));
  connect (sigman, SIGNAL(signal_updatedUser(unsigned long, unsigned long)),
           this, SLOT(slot_updatedUser(unsigned long, unsigned long)));
  connect (btnCancel, SIGNAL(clicked()), this, SLOT(close()));
  connect (btnOk, SIGNAL(clicked()), this, SLOT(callFcn()));
  connect (btnSave, SIGNAL(clicked()), this, SLOT(save()));

  QWidgetStack *stack = (QWidgetStack *)tabs->child("tab pages");
  if (stack == NULL)
    WarnUser(this, "Unable to find widget stack");
  else
    connect(stack, SIGNAL(aboutToShow(QWidget *)), this, SLOT(slot_aboutToShow(QWidget *)));
}


void ICQFunctions::CreateReadEventTab()
{
  tabList[TAB_READ].label = tr("&View");
  QVBox *p = new QVBox(this, tabList[TAB_READ].label);
  p->setMargin(8);
  tabList[TAB_READ].tab = p;
  tabList[TAB_READ].loaded = true;

  splRead = new QSplitter(QSplitter::Vertical, p);
  msgView = new MsgView(splRead);
  mleRead = new MLEditWrap(true, splRead, true);
  mleRead->setReadOnly(true);
  splRead->setOpaqueResize(true);
  splRead->setResizeMode(msgView, QSplitter::KeepSize);
  connect (msgView, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(slot_printMessage(QListViewItem *)));

  QWidget *dummy = new QWidget(p);
  dummy->setMinimumHeight(5);

  QHBox *h = new QHBox(p);
  h->setSpacing(10);
  btnRead1 = new QPushButton(h);
  btnRead2 = new QPushButton(h);
  btnRead3 = new QPushButton(h);
  btnRead4 = new QPushButton(tr("Next"), h);

  btnRead1->setEnabled(false);
  btnRead2->setEnabled(false);
  btnRead3->setEnabled(false);
  btnRead4->setEnabled(false);

  connect(btnRead1, SIGNAL(clicked()), this, SLOT(slot_readbtn1()));
  connect(btnRead2, SIGNAL(clicked()), this, SLOT(slot_readbtn2()));
  connect(btnRead3, SIGNAL(clicked()), this, SLOT(slot_readbtn3()));
  connect(btnRead4, SIGNAL(clicked()), this, SLOT(slot_nextMessage()));
}


void ICQFunctions::CreateSendEventTab()
{
  tabList[TAB_SEND].label = tr("S&end");
  tabList[TAB_SEND].tab = new QWidget(this, tabList[TAB_SEND].label);
  QBoxLayout* selay = new QVBoxLayout(tabList[TAB_SEND].tab, 8);
  tabList[TAB_SEND].loaded = true;

  grpCmd = new QButtonGroup(1, Vertical, tr("Select Function"), tabList[TAB_SEND].tab);
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

  mleSend = new MLEditWrap(true, tabList[TAB_SEND].tab, true);
  mleSend->setMinimumHeight(150);
  selay->addWidget(mleSend);
  selay->setStretchFactor(mleSend, 1);

  grpOpt = new QGroupBox(2, Horizontal, tabList[TAB_SEND].tab);
  selay->addWidget(grpOpt);
  lblItem = new QLabel(grpOpt);
  edtItem = new QLineEdit(grpOpt);
#if QT_VERSION < 210
  QWidget* dummy_w2 = new QWidget(grpOpt);
  dummy_w2->setMinimumHeight(2);
#endif
  grpOpt->hide();

  QGroupBox *box = new QGroupBox(tabList[TAB_SEND].tab);
  selay->addWidget(box);

  QBoxLayout *vlay = new QVBoxLayout(box, 10, 5);

  QBoxLayout *hlay = new QHBoxLayout(vlay);

  chkSendServer = new QCheckBox(tr("Send through server"), box);//tabList[TAB_SEND].tab);
  hlay->addWidget(chkSendServer);
  chkUrgent = new QCheckBox(tr("Urgent"), box);//tabList[TAB_SEND].tab);
  hlay->addWidget(chkUrgent);

#ifdef USE_SPOOFING
  hlay = new QHBoxLayout(vlay);//selay);
  chkSpoof = new QCheckBox(tr("Spoof UIN:"), box);//tabList[TAB_SEND].tab);
  hlay->addWidget(chkSpoof);
  edtSpoof = new QLineEdit(box);//tabList[TAB_SEND].tab);
  hlay->addWidget(edtSpoof);
  edtSpoof->setEnabled(false);
  edtSpoof->setValidator(new QIntValidator(1, 100000000, edtSpoof));
  connect(chkSpoof, SIGNAL(toggled(bool)), edtSpoof, SLOT(setEnabled(bool)));
#else
  edtSpoof = 0;
  chkSpoof = 0;
#endif
  selay->activate();
}


void ICQFunctions::CreateGeneralInfoTab()
{
  tabList[TAB_GENERALINFO].label = tr("General");
  tabList[TAB_GENERALINFO].tab = new QWidget(this, tabList[TAB_GENERALINFO].label);
  tabList[TAB_GENERALINFO].loaded = false;
}


void ICQFunctions::InitGeneralInfoTab()
{
  tabList[TAB_GENERALINFO].loaded = true;

  unsigned short CR = 0;
  QWidget *p = tabList[TAB_GENERALINFO].tab;

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
  nfoFirstName = new CInfoField(p, false);
  lay->addWidget(nfoFirstName, CR, 1);
  nfoLastName = new CInfoField(p, false);
  lay->addMultiCellWidget(nfoLastName, CR, CR, 2, 4);

  lay->addWidget(new QLabel(tr("EMail 1:"), p), ++CR, 0);
  nfoEmail1 = new CInfoField(p, false);
  lay->addMultiCellWidget(nfoEmail1, CR, CR, 1, 4);

  lay->addWidget(new QLabel(tr("EMail 2:"), p), ++CR, 0);
  nfoEmail2 = new CInfoField(p, false);
  lay->addMultiCellWidget(nfoEmail2, CR, CR, 1, 4);

  lay->addWidget(new QLabel(tr("State:"), p), ++CR, 0);
  nfoState = new CInfoField(p, !m_bOwner);
  nfoState->setMaxLength(5);
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
    cmbCountry = new CEComboBox(true, tabList[TAB_GENERALINFO].tab);
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
  tabList[TAB_MOREINFO].label = tr("More");
  tabList[TAB_MOREINFO].tab = new QWidget(this, tabList[TAB_MOREINFO].label);
  tabList[TAB_MOREINFO].loaded = false;
}


void ICQFunctions::InitMoreInfoTab()
{
  tabList[TAB_MOREINFO].loaded = true;

  unsigned short CR = 0;
  QWidget *p = tabList[TAB_MOREINFO].tab;
  QGridLayout *lay = new QGridLayout(p, 8, 5, 10, 5);
  lay->addColSpacing(2, 10);
  lay->addRowSpacing(5, 5);
  lay->setRowStretch(7, 1);

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

  lblAuth = new QLabel(p);
  CR += 2;
  lay->addMultiCellWidget(lblAuth, CR, CR, 0, 4);
}


void ICQFunctions::CreateWorkInfoTab()
{
  tabList[TAB_WORKINFO].label = tr("Work");
  tabList[TAB_WORKINFO].tab = new QWidget(this, tabList[TAB_WORKINFO].label);
  tabList[TAB_WORKINFO].loaded = false;
}


void ICQFunctions::InitWorkInfoTab()
{
  tabList[TAB_WORKINFO].loaded = true;

  unsigned short CR = 0;
  QWidget *p = tabList[TAB_WORKINFO].tab;

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
  tabList[TAB_ABOUT].label = tr("About");
  tabList[TAB_ABOUT].tab = new QVBox(this, tabList[TAB_ABOUT].label);
  tabList[TAB_ABOUT].loaded = false;
}


void ICQFunctions::InitAboutTab()
{
  tabList[TAB_ABOUT].loaded = true;

  QVBox *p = (QVBox *)tabList[TAB_ABOUT].tab;

  p->setMargin(8);
  p->setSpacing(8);

  lblAbout = new QLabel(tr("About:"), p);
  mleAbout = new MLEditWrap(true, p);
}


void ICQFunctions::CreateHistoryTab()
{
  tabList[TAB_HISTORY].label = tr("History");
  tabList[TAB_HISTORY].tab = new QWidget(this, tabList[TAB_HISTORY].label);
  tabList[TAB_HISTORY].loaded = false;
}


void ICQFunctions::InitHistoryTab()
{
  tabList[TAB_HISTORY].loaded = true;
  QWidget *p = tabList[TAB_HISTORY].tab;

  /*QVBoxLayout *lay = new QVBoxLayout(p, 8, 8);

  lblHistory = new QLabel(p);
  lblHistory->setAutoResize(true);
  lblHistory->setAlignment(AlignLeft | AlignVCenter);
  lay->addWidget(lblHistory);

  mleHistory = new HistoryWidget(p);
  lay->addWidget(mleHistory, 2);

  QGroupBox *box = new QGroupBox(1, Vertical, p);
  lay->addWidget(box);

  btnHistoryReload = new QPushButton(tr("Reload"), box);
  connect(btnHistoryReload, SIGNAL(clicked()), SLOT(slot_historyReload()));
  btnHistoryEdit = new QPushButton(tr("Edit"), box);
  connect(btnHistoryEdit, SIGNAL(clicked()), SLOT(slot_historyEdit()));
  chkHistoryReverse = new QCheckBox(tr("Reverse"), box);
  connect(chkHistoryReverse, SIGNAL(toggled(bool)), SLOT(slot_historyReverse(bool)));
  chkHistoryReverse->setChecked(true);*/

  QBoxLayout* lay = new QVBoxLayout(p, 8, 8);

  QBoxLayout* l = new QHBoxLayout(lay);

  lblHistory = new QLabel(p);
  lblHistory->setAutoResize(true);
  lblHistory->setAlignment(AlignLeft | AlignVCenter);
  l->addWidget(lblHistory);

  chkHistoryReverse = new QCheckBox(tr("Reverse"), p);
  connect(chkHistoryReverse, SIGNAL(toggled(bool)), SLOT(slot_historyReverse(bool)));
  chkHistoryReverse->setChecked(true);
  chkHistoryReverse->setFixedSize(chkHistoryReverse->sizeHint());
  l->addWidget(chkHistoryReverse);

  mleHistory = new CHistoryWidget(p);
  lay->addWidget(mleHistory, 1);
}


//-----ICQFunctions::keyPressEvent----------------------------------------------
void ICQFunctions::keyPressEvent(QKeyEvent *e)
{
  if (e->key() == Key_Escape)
  {
    close();
    return;
  }
  else if (tabs->currentPage() == tabList[TAB_READ].tab && !e->text().isEmpty())
  {
    tabs->showPage(tabList[TAB_SEND].tab);
    return;
  }
  QWidget::keyPressEvent(e);
}

//-----ICQFunctions::setupTabs--------------------------------------------------
void ICQFunctions::setupTabs(int index)
{
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);

  // read tab
  if (index == mnuUserView)
  {
    if (u->NewMessages() > 0)
    {
      gUserManager.DropUser(u);
      slot_nextMessage();
      u = gUserManager.FetchUser(m_nUin, LOCK_R);
    }
  }
  if (u->NewMessages() > 0) btnRead4->setEnabled(true);

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
  tabs->setTabEnabled(tabList[TAB_SEND].tab, !m_bOwner);

  // Info tabs
  SetInfo(u);

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

  switch(index)
  {
    case mnuUserView:
      tabs->showPage(tabList[TAB_READ].tab);
      break;
    case mnuUserSendMsg:
      tabs->showPage(tabList[TAB_SEND].tab);
      rdbMsg->setChecked(true);
      specialFcn(0);
      break;
    case mnuUserSendUrl:
      tabs->showPage(tabList[TAB_SEND].tab);
      rdbUrl->setChecked(true);
      specialFcn(1);
      break;
    case mnuUserSendChat:
      tabs->showPage(tabList[TAB_SEND].tab);
      rdbChat->setChecked(true);
      specialFcn(2);
      break;
    case mnuUserSendFile:
      tabs->showPage(tabList[TAB_SEND].tab);
      rdbFile->setChecked(true);
      specialFcn(3);
      break;
    case mnuUserGeneral:
      tabs->showPage(tabList[TAB_GENERALINFO].tab);
      break;
    case mnuUserMore:
      tabs->showPage(tabList[TAB_MOREINFO].tab);
      break;
    case mnuUserWork:
      tabs->showPage(tabList[TAB_WORKINFO].tab);
      break;
    case mnuUserAbout:
      tabs->showPage(tabList[TAB_ABOUT].tab);
      break;
    case mnuUserHistory:
      tabs->showPage(tabList[TAB_HISTORY].tab);
      break;
    default:
      gLog.Warn("%sInternal Error: ICQFunctions::setupTabs(): Invalid index (%d).\n",
                L_WARNxSTR, index);
  }

}

void ICQFunctions::SetInfo(ICQUser *u)
{
  bool bDropUser = false;

  if (u == NULL)
  {
    u = gUserManager.FetchUser(m_nUin, LOCK_R);
    bDropUser = true;
  }

  nfoStatus->setData(u->StatusStr());
  time_t te = time(NULL);
#ifndef __FreeBSD__
  localtime(&te);
  m_nRemoteTimeOffset = timezone - u->GetTimezone() * 1800;
#else
  struct tm *tzone = localtime(&te);
  m_nRemoteTimeOffset = -(tzone->tm_gmtoff) - u->GetTimezone() * 1800;
#endif
  QDateTime t;
  t.setTime_t(te + m_nRemoteTimeOffset);
  nfoTimezone->setData(tr("%1 (GMT%1%1%1)")
                       .arg(t.time().toString())
                       .arg(u->GetTimezone() > 0 ? "-" : "+")
                       .arg(abs(u->GetTimezone() / 2))
                       .arg(u->GetTimezone() % 2 ? "30" : "00") );
  tmrTime = new QTimer(this);
  connect(tmrTime, SIGNAL(timeout()), this, SLOT(slot_updatetime()));
  tmrTime->start(3000);

  m_sBaseTitle = QString::fromLocal8Bit(u->GetAlias()) + " (" +
                 QString::fromLocal8Bit(u->GetFirstName()) + " " +
                 QString::fromLocal8Bit(u->GetLastName())+ ")";
  setCaption(m_sBaseTitle);
  setIconText(u->GetAlias());

  if (bDropUser) gUserManager.DropUser(u);
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

  if (!u->StatusOffline())
    nfoLastOnline->setData(tr("Now"));
  else if (u->LastOnline() == 0)
    nfoLastOnline->setData(tr("Unknown"));
  else
  {
    QDateTime t;
    t.setTime_t(u->LastOnline());
    QString ds = t.toString();
    ds.truncate(ds.length() - 8);
    nfoLastOnline->setData(ds);
  }

  SetInfo(u);

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

  if (u->GetAuthorization())
    lblAuth->setText(tr("Authorization Required"));
  else
    lblAuth->setText(tr("Authorization Not Required"));

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



//-----ICQFunctions::SendMsg---------------------------------------------------
void ICQFunctions::SendMsg(QString msg)
{
  tabs->showPage(tabList[TAB_SEND].tab);
  rdbMsg->setChecked(true);
  specialFcn(0);
  mleSend->setText(msg);
}

//-----ICQFunctions::SendUrl---------------------------------------------------
void ICQFunctions::SendUrl(QString url, QString desc)
{
  tabs->showPage(tabList[TAB_SEND].tab);
  rdbUrl->setChecked(true);
  specialFcn(1);
  edtItem->setText(url);
  mleSend->setText(desc);
}

//-----ICQFunctions::SendFile--------------------------------------------------
void ICQFunctions::SendFile(QString file, QString desc)
{
  tabs->showPage(tabList[TAB_SEND].tab);
  rdbFile->setChecked(true);
  specialFcn(3);
  edtItem->setText(file);
  mleSend->setText(desc);
}


void ICQFunctions::slot_aboutToShow(QWidget *p)
{
  if (p == tabList[TAB_GENERALINFO].tab)
  {
     if (!tabList[TAB_GENERALINFO].loaded)
     {
       InitGeneralInfoTab();
       SetGeneralInfo(NULL);
     }
  }
  else if (p == tabList[TAB_MOREINFO].tab)
  {
     if (!tabList[TAB_MOREINFO].loaded)
     {
       InitMoreInfoTab();
       SetMoreInfo(NULL);
     }
  }
  else if (p == tabList[TAB_WORKINFO].tab)
  {
     if (!tabList[TAB_WORKINFO].loaded)
     {
       InitWorkInfoTab();
       SetWorkInfo(NULL);
     }
  }
  else if (p == tabList[TAB_ABOUT].tab)
  {
     if (!tabList[TAB_ABOUT].loaded)
     {
       InitAboutTab();
       SetAbout(NULL);
     }
  }
  else if (p == tabList[TAB_HISTORY].tab)
  {
     if (!tabList[TAB_HISTORY].loaded)
     {
       InitHistoryTab();
       SetupHistory();
     }
  }
}


//-----ICQFunctions::tabSelected-----------------------------------------------
void ICQFunctions::tabSelected(const QString &tab)
{
  if (tab == tabList[TAB_SEND].label)
  {
     mleSend->setFocus();
     btnOk->setText(tr("&Send"));
     btnSave->hide();
     currentTab = TAB_SEND;
  }
  else if (tab == tabList[TAB_GENERALINFO].label)
  {
     btnOk->setText(tr("Update"));
     btnSave->setText(tr("Save"));
     btnSave->show();
     currentTab = TAB_GENERALINFO;
  }
  else if (tab == tabList[TAB_READ].label)
  {
     btnOk->setText(tr("Ok"));
     btnSave->hide();
     msgView->triggerUpdate();
     currentTab = TAB_READ;
  }
  else if (tab == tabList[TAB_MOREINFO].label)
  {
     btnOk->setText(tr("Update"));
     btnSave->setText(tr("Save"));
     btnSave->show();
     currentTab = TAB_MOREINFO;
  }
  else if (tab == tabList[TAB_WORKINFO].label)
  {
     btnOk->setText(tr("Update"));
     btnSave->setText(tr("Save"));
     btnSave->show();
     currentTab = TAB_WORKINFO;
  }
  else if (tab == tabList[TAB_ABOUT].label)
  {
     btnOk->setText(tr("Update"));
     btnSave->setText(tr("Save"));
     btnSave->show();
     currentTab = TAB_ABOUT;
  }
  else if (tab == tabList[TAB_HISTORY].label)
  {
     btnOk->setText(tr("Next"));
     btnSave->setText(tr("Prev"));
     btnSave->show();
     currentTab = TAB_HISTORY;
  }
  else
  {
     btnOk->setText(tr("Ok"));
  }
}


//-----slot_updatedUser--------------------------------------------------------
void ICQFunctions::slot_updatedUser(unsigned long _nUpdateType, unsigned long _nUin)
{
  if (m_nUin != _nUin) return;
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
  if(u == NULL) return;
  switch (_nUpdateType)
  {
  case USER_STATUS:
  {
    nfoStatus->setData(u->StatusStr());
    if (u->Ip() == 0)
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
    if (u->NewMessages() > 1)
    {
      btnRead4->setEnabled(true);
      btnRead4->setText(tr("Next (%1)").arg(u->NewMessages()));
    }
    else if (u->NewMessages() == 1)
    {
      btnRead4->setEnabled(true);
      btnRead4->setText(tr("Next"));
    }
    else
    {
      btnRead4->setEnabled(false);
      btnRead4->setText(tr("Next"));
    }
    break;
  }
  case USER_GENERAL:
  case USER_BASIC:
  case USER_EXT:
    if (tabList[TAB_GENERALINFO].loaded) SetGeneralInfo(u);
    break;
  case USER_MORE:
    if (tabList[TAB_MOREINFO].loaded) SetMoreInfo(u);
    break;
  case USER_WORK:
    if (tabList[TAB_WORKINFO].loaded) SetWorkInfo(u);
    break;
  case USER_ABOUT:
    if (tabList[TAB_ABOUT].loaded) SetAbout(u);
    break;
  }
  gUserManager.DropUser(u);
}


void ICQFunctions::slot_nextMessage()
{
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_W);
  if (u->NewMessages() == 0)
  {
    gUserManager.DropUser(u);
    return;
  }
  MsgViewItem *e = new MsgViewItem(u->EventPop(), msgView);
  btnRead4->setEnabled(u->NewMessages() > 0);
  if (u->NewMessages() > 1)
  {
    btnRead4->setEnabled(true);
    btnRead4->setText(tr("Next (%1)").arg(u->NewMessages()));
  }
  else if (u->NewMessages() == 1)
  {
    btnRead4->setEnabled(true);
    btnRead4->setText(tr("Next"));
  }
  else
  {
    btnRead4->setEnabled(false);
    btnRead4->setText(tr("Next"));
  }

  gUserManager.DropUser(u);

  msgView->setSelected(e, true);
  slot_printMessage(e);
  emit signal_updatedUser(USER_EVENTS, m_nUin);
}


//-----ICQFunctions::printMessage----------------------------------------------
void ICQFunctions::slot_printMessage(QListViewItem *e)
{
  if (e == NULL)
    return;

  btnRead1->setText("");
  btnRead2->setText("");
  btnRead3->setText("");
  //btnRead4->setText("");
  btnRead1->setEnabled(false);
  btnRead2->setEnabled(false);
  btnRead3->setEnabled(false);
  //btnRead4->setEnabled(false);

  CUserEvent *m = ((MsgViewItem *)e)->msg;
  m_xCurrentReadEvent = m;
  mleRead->setText(QString::fromLocal8Bit(m->Text()));
  if (m->Command() == ICQ_CMDxTCP_START || m->Command() == ICQ_CMDxRCV_SYSxMSGxONLINE)
  {
    switch (m->SubCommand())
    {
      case ICQ_CMDxSUB_CHAT:  // accept or refuse a chat request
      case ICQ_CMDxSUB_FILE:  // accept or refuse a file transfer
        if (m->IsCancelled())
        {
          mleRead->append(tr("\n--------------------\nCANCELLED"));
        }
        else
        {
          btnRead1->setText(tr("&Accept"));
          btnRead2->setText(tr("&Refuse"));
        }
        break;

      case ICQ_CMDxSUB_MSG:
        btnRead1->setText(tr("&Quote"));
        btnRead2->setText(tr("&Forward"));
        break;

      case ICQ_CMDxSUB_URL:   // view a url
        btnRead1->setText(tr("&Quote"));
        btnRead2->setText(tr("&Forward"));
        if (server->getUrlViewer() != NULL)
          btnRead3->setText(tr("&View"));
        break;

      case ICQ_CMDxSUB_AUTHxREQUEST:
      {
        btnRead1->setText(tr("A&uthorize"));
        ICQUser *u = gUserManager.FetchUser( ((CEventAuthRequest *)m)->Uin(), LOCK_R);
        if (u == NULL)
          btnRead2->setText(tr("&Add User"));
        else
          gUserManager.DropUser(u);
        break;
      }
      case ICQ_CMDxSUB_AUTHxGRANTED:
      {
        ICQUser *u = gUserManager.FetchUser( ((CEventAuthGranted *)m)->Uin(), LOCK_R);
        if (u == NULL)
          btnRead1->setText(tr("&Add User"));
        else
          gUserManager.DropUser(u);
        break;
      }
      case ICQ_CMDxSUB_ADDEDxTOxLIST:
      {
        ICQUser *u = gUserManager.FetchUser( ((CEventAdded *)m)->Uin(), LOCK_R);
        if (u == NULL)
          btnRead1->setText(tr("&Add User"));
        else
          gUserManager.DropUser(u);
        break;
      }
    } // switch
  }  // if

  if (!btnRead1->text().isEmpty()) btnRead1->setEnabled(true);
  if (!btnRead2->text().isEmpty()) btnRead2->setEnabled(true);
  if (!btnRead3->text().isEmpty()) btnRead3->setEnabled(true);
  //if (!btnRead4->text().isEmpty()) btnRead4->setEnabled(true);
}


void ICQFunctions::slot_readbtn1()
{
  if (m_xCurrentReadEvent == NULL) return;

  switch (m_xCurrentReadEvent->SubCommand())
  {
    case ICQ_CMDxSUB_CHAT:  // accept a chat request
    {
      CEventChat *c = (CEventChat *)m_xCurrentReadEvent;
      ChatDlg *chatDlg = new ChatDlg(m_nUin, server);
      if (chatDlg->StartAsServer())
        server->icqChatRequestAccept(m_nUin, chatDlg->LocalPort(), c->Sequence());
      break;
    }

    case ICQ_CMDxSUB_FILE:  // accept a file transfer
    {
      CEventFile *f = (CEventFile *)m_xCurrentReadEvent;
      CFileDlg *fileDlg = new CFileDlg(m_nUin, f->Filename(), f->FileSize(), server);
      if (fileDlg->StartAsServer())
        server->icqFileTransferAccept(m_nUin, fileDlg->LocalPort(), f->Sequence());
      break;
    }

    case ICQ_CMDxSUB_MSG:
      generateReply();
      break;

    case ICQ_CMDxSUB_URL:
      generateReply();
      break;

    case ICQ_CMDxSUB_AUTHxREQUEST:
      server->icqAuthorize( ((CEventAuthRequest *)m_xCurrentReadEvent)->Uin() );
      break;

    case ICQ_CMDxSUB_AUTHxGRANTED:
      server->AddUserToList( ((CEventAuthGranted *)m_xCurrentReadEvent)->Uin());
      break;

    case ICQ_CMDxSUB_ADDEDxTOxLIST:
      server->AddUserToList( ((CEventAdded *)m_xCurrentReadEvent)->Uin());
      break;
  } // switch

}


void ICQFunctions::slot_readbtn2()
{
  if (m_xCurrentReadEvent == NULL) return;

  switch (m_xCurrentReadEvent->SubCommand())
  {
    case ICQ_CMDxSUB_CHAT:  // refuse a chat request
    {
      CRefuseDlg *r = new CRefuseDlg(m_nUin, tr("Chat"), this);
      if (r->exec())
      {
        server->icqChatRequestRefuse(m_nUin, (const char *)r->RefuseMessage(),
           m_xCurrentReadEvent->Sequence());
      }
      delete r;
      break;
    }

    case ICQ_CMDxSUB_FILE:  // refuse a file transfer
    {
      CRefuseDlg *r = new CRefuseDlg(m_nUin, tr("File Transfer"), this);
      if (r->exec())
      {
        server->icqFileTransferRefuse(m_nUin, (const char *)r->RefuseMessage(),
           m_xCurrentReadEvent->Sequence());
      }
      delete r;
      break;
    }

    case ICQ_CMDxSUB_MSG:  // Forward
    case ICQ_CMDxSUB_URL:
    {
      CForwardDlg *f = new CForwardDlg(mainwin, m_xCurrentReadEvent);
      // Move should go here to avoid flicker, but qt is stupid and
      // will ignore any move()s until after a show().
      //f->move(x() + width() / 2 - f->width() / 2, y() + height() / 2 - f->height() / 2);
      f->show();
      f->move(x() + width() / 2 - f->width() / 2, y() + height() / 2 - f->height() / 2);
      break;
    }

    case ICQ_CMDxSUB_AUTHxREQUEST:
      server->AddUserToList( ((CEventAuthRequest *)m_xCurrentReadEvent)->Uin());
      break;

  } // switch

}


void ICQFunctions::slot_readbtn3()
{
  if (m_xCurrentReadEvent == NULL) return;

  switch (m_xCurrentReadEvent->SubCommand())
  {
    case ICQ_CMDxSUB_URL:   // view a url
      if (server->getUrlViewer() != NULL)
      {
        char* szCmd = new char[strlen(server->getUrlViewer()) + strlen(((CEventUrl *)m_xCurrentReadEvent)->Url()) + 8];
        sprintf(szCmd, "%s '%s' &", server->getUrlViewer(),
                ((CEventUrl *)m_xCurrentReadEvent)->Url());
        if (system(szCmd) != 0) WarnUser(this, tr("View URL failed."));
        delete szCmd;
      }
      break;
  }
}


void ICQFunctions::slot_readbtn4()
{
}


//-----ICQFunctions::save------------------------------------------------------
void ICQFunctions::save()
{
  switch (currentTab)
  {
  case TAB_READ:
    //generateReply();
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
  if (!u->GetHistory(m_lHistoryList))
  {
    mleHistory->setText(tr("Error loading history"));
  }
  else
  {
    m_bHistoryReverse = chkHistoryReverse->isChecked();
    m_iHistoryEIter = m_lHistoryList.end();
    m_iHistorySIter = m_iHistoryEIter;
    for (unsigned short i = 0;
         (i < NUM_MSG_PER_HISTORY) && (m_iHistorySIter != m_lHistoryList.begin());
         i++)
    {
      m_iHistorySIter--;
    }
    m_nHistoryIndex = m_lHistoryList.size();
    ShowHistory();
  }
  gUserManager.DropUser(u);
}


void ICQFunctions::slot_historyReload()
{
  ICQUser::ClearHistory(m_lHistoryList);
  SetupHistory();
}

void ICQFunctions::slot_historyEdit()
{
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
  if (u == NULL) return;

  (void) new EditFileDlg(u->HistoryFile());

  gUserManager.DropUser(u);
}


void ICQFunctions::slot_historyReverse(bool newVal)
{
  if (chkHistoryReverse->isChecked() != newVal)
    chkHistoryReverse->setChecked(newVal);
  else if(m_bHistoryReverse != newVal)
  {
    m_bHistoryReverse = newVal;
    ShowHistory();
  }
}

void ICQFunctions::ShowHistoryPrev()
{
  if (m_iHistorySIter != m_lHistoryList.begin())
  {
    m_iHistoryEIter = m_iHistorySIter;
    m_nHistoryIndex -= NUM_MSG_PER_HISTORY;
    for (unsigned short i = 0;
         (i < NUM_MSG_PER_HISTORY) && (m_iHistorySIter != m_lHistoryList.begin());
         i++)
    {
	    m_iHistorySIter--;
    }
    ShowHistory();
  }
}

void ICQFunctions::ShowHistoryNext()
{
  if (m_iHistoryEIter != m_lHistoryList.end())
  {
    m_iHistorySIter = m_iHistoryEIter;
    for (unsigned short i = 0;
         (i < NUM_MSG_PER_HISTORY) && (m_iHistoryEIter != m_lHistoryList.end());
         i++)
    {
	    m_iHistoryEIter++;
	    m_nHistoryIndex++;
    }
    ShowHistory();
  }
}

//-----ICQFunctions::ShowHistory--------------------------------------------
void ICQFunctions::ShowHistory()
{
  // Last check (should never be true)
  if (m_lHistoryList.size() == 0) return;
  HistoryListIter tempIter;

  if(m_bHistoryReverse)
  {
    tempIter = m_iHistoryEIter;
    tempIter--;
  }
  else
  {
    tempIter = m_iHistorySIter;
  }
  QString s, st, n;
  QDateTime d;
  m_nHistoryShowing = 0;
  if (m_bOwner)
    n = tr("server");
  else
  {
    ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
    if (u != NULL)
    {
      n = QString::fromLocal8Bit(u->GetAlias());
      gUserManager.DropUser(u);
    }
  }
  while (m_nHistoryShowing < (NUM_MSG_PER_HISTORY))
  {
    d.setTime_t((*tempIter)->Time());
    if ((*tempIter)->Direction() == D_RECEIVER)
      s.sprintf("%c%s %s %s\n%c%s [%c%c%c]\n\n%s\n\n",
                '\001', (const char *)EventDescription(*tempIter),
                (const char *)tr("from"), (const char *)n, '\001',
                (const char *)d.toString(),
                (*tempIter)->IsDirect() ? 'D' : '-',
                (*tempIter)->IsMultiRec() ? 'M' : '-',
                (*tempIter)->IsUrgent() ? 'U' : '-',
                (QString::fromLocal8Bit((*tempIter)->Text())).utf8().data());
    else
      s.sprintf("%c%s %s %s\n%c%s [%c%c%c]\n\n%s\n\n",
                '\002', (const char *)EventDescription(*tempIter),
                (const char *)tr("to"), (const char *)n, '\002',
                (const char *)d.toString(),
                (*tempIter)->IsDirect() ? 'D' : '-',
                (*tempIter)->IsMultiRec() ? 'M' : '-',
                (*tempIter)->IsUrgent() ? 'U' : '-',
                (QString::fromLocal8Bit((*tempIter)->Text())).utf8().data());
    st.append(s);
    m_nHistoryShowing++;
    if(m_bHistoryReverse)
    {
      if (tempIter == m_iHistorySIter)
        break;
      tempIter--;
    }
    else
    {
       tempIter++;
       if (tempIter == m_iHistoryEIter)
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
  if(!m_bHistoryReverse)
    mleHistory->GotoEnd();
  else
    mleHistory->setCursorPosition(0, 0);
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
  mleSend->GotoEnd();
  tabs->showPage(tabList[TAB_SEND].tab);
}

//---------------------------------------------------------------------------
void ICQFunctions::setSpoofed()
{
  if (chkSpoof && chkSpoof->isChecked())
  {
    if (!QueryUser(this, tr("Spoofing messages is immoral and possibly illegal\n"
                            "In clicking OK you absolve the author from any \n"
                            "responsibility for your actions.\n"
                            "Do you want to continue?"),
                   tr("Ok"), tr("Cancel")))
    {
      chkSpoof->setChecked(false);
    }
  }
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
    if (icqEventTag != NULL) break;
#ifdef USE_KDE
    // !!! FIXME !!!!
    //QStringList fl = KFileDialog::getOpenFileNames(NULL, NULL, this);
    QStringList fl = QFileDialog::getOpenFileNames(NULL, NULL, this);
#else
    QStringList fl = QFileDialog::getOpenFileNames(NULL, NULL, this);
#endif
    QStringList::ConstIterator it;
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
    if (nMsgLen > MAX_MESSAGE_SIZE && chkSendServer->isChecked()
        && !QueryUser(this, tr("Message is %1 characters, over the ICQ server limit of %2.\n"
                               "The message will be truncated if sent through the server.").arg(nMsgLen).arg(MAX_MESSAGE_SIZE),
                      tr("Continue"), tr("Cancel")))
        break;

    unsigned long uin = (chkSpoof && chkSpoof->isChecked() ?
                         edtSpoof->text().toULong() : 0);
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
         chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL, uin);
    }
    else if (rdbUrl->isChecked()) // send URL
    {
      m_sProgressMsg = tr("Sending URL ");
      m_sProgressMsg += chkSendServer->isChecked() ? tr("through server") : tr("direct");
      m_sProgressMsg += "...";
      icqEventTag = server->icqSendUrl(m_nUin, edtItem->text(), mleSend->text().local8Bit(),
         chkSendServer->isChecked() ? false : true,
         chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL, uin);
    }
    else if (rdbChat->isChecked())   // send chat request
    {
      m_sProgressMsg = tr("Sending chat request...");
      icqEventTag = server->icqChatRequest(m_nUin, mleSend->text().local8Bit(),
         chkSendServer->isChecked() ? false : true,
         chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL, uin);
    }
    else if (rdbFile->isChecked())   // send file transfer
    {
      if (edtItem->text().isEmpty())
      {
        WarnUser(this, tr("You must specify a file to transfer!"));
        break;
      }
      m_sProgressMsg = tr("Sending file transfer...");
      icqEventTag = server->icqFileTransfer(m_nUin, edtItem->text(),
         mleSend->text().local8Bit(),
         chkSendServer->isChecked() ? false : true,
         chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL, uin);
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


//-----RetrySend-------------------------------------------------------------
void ICQFunctions::RetrySend(ICQEvent *e, bool bOnline, unsigned short nLevel)
{
  btnOk->setEnabled(false);
  btnCancel->setText(tr("Cancel"));
  if (!bOnline)
  {
    chkSendServer->setChecked(true);
    ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_W);
    u->SetSendServer(true);
    gUserManager.DropUser(u);
  }

  chkUrgent->setChecked(nLevel == ICQ_TCPxMSG_URGENT);

  switch(e->m_nSubCommand)
  {
    case ICQ_CMDxSUB_MSG:
    {
      CEventMsg *ue = (CEventMsg *)e->m_xUserEvent;
      m_sProgressMsg = tr("Sending msg ");
      m_sProgressMsg += bOnline ? tr("direct") : tr("through server");
      m_sProgressMsg += "...";
      icqEventTag = server->icqSendMessage(m_nUin, ue->Message(), bOnline,
         nLevel, 0);
      break;
    }
    case ICQ_CMDxSUB_URL:
    {
      CEventUrl *ue = (CEventUrl *)e->m_xUserEvent;
      m_sProgressMsg = tr("Sending URL ");
      m_sProgressMsg += bOnline ? tr("direct") : tr("through server");
      m_sProgressMsg += "...";
      icqEventTag = server->icqSendUrl(m_nUin, ue->Url(), ue->Description(),
         bOnline, nLevel, 0);
      break;
    }
    case ICQ_CMDxSUB_CHAT:
    {
      CEventChat *ue = (CEventChat *)e->m_xUserEvent;
      m_sProgressMsg = tr("Sending chat request...");
      icqEventTag = server->icqChatRequest(m_nUin, ue->Reason(),
         bOnline, nLevel, 0);
      break;
    }
    case ICQ_CMDxSUB_FILE:
    {
      CEventFile *ue = (CEventFile *)e->m_xUserEvent;
      m_sProgressMsg = tr("Sending file transfer...");
      icqEventTag = server->icqFileTransfer(m_nUin, ue->Filename(),
         ue->FileDescription(), bOnline, nLevel, 0);
      break;
    }
  }

  QString title = m_sBaseTitle + " [" + m_sProgressMsg + "]";
  setCaption(title);
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
      RetrySend(e, false, ICQ_TCPxMSG_NORMAL);
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
        u = gUserManager.FetchUser(m_nUin, LOCK_R);
        msg = tr("%1 is in %2 mode:\n%3\n")
                 .arg(u->GetAlias()).arg(u->StatusStr()).arg(u->AutoResponse());
        gUserManager.DropUser(u);
        switch (QueryUser(this, msg, tr("Send\nUrgent"), tr("Send to\nContact List"), tr("Cancel")))
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
           QString result = tr("%1 with %2 refused:\n%3")
              .arg(EventDescription(ue)).arg(u->GetAlias()).arg(ea->szResponse);
           gUserManager.DropUser(u);
           InformUser(this, result);
        }
        else
        {
          switch (e->m_nSubCommand)
          {
          case ICQ_CMDxSUB_CHAT:
          {
            ChatDlg *chatDlg = new ChatDlg(m_nUin, server);
            chatDlg->StartAsClient(ea->nPort);
            break;
          }
          case ICQ_CMDxSUB_FILE:
          {
            CEventFile *f = (CEventFile *)ue;
            CFileDlg *fileDlg = new CFileDlg(m_nUin, f->Filename(), f->FileSize(), server);
            fileDlg->StartAsClient(ea->nPort);
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
    emit signal_finished(m_nUin);
    e->accept();
    delete this;
  }
}


ICQFunctions::~ICQFunctions()
{
  if (icqEventTag != NULL) server->CancelEvent(icqEventTag);
  delete icqEventTag;
  ICQUser::ClearHistory(m_lHistoryList);
}

#include "icqfunctions.moc"
