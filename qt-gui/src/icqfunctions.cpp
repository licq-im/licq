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
#include <qhgroupbox.h>
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
#include "licq_countrycodes.h"
#include "licq_languagecodes.h"
#include "licq_log.h"
#include "sigman.h"
#include "editfile.h"
#include "eventdesc.h"
#include "gui-defines.h"
#include "refusedlg.h"
#include "forwarddlg.h"
#include "chatjoin.h"
#include "mainwin.h"
#include "mmlistview.h"
#include "mmsenddlg.h"

#include "licq_user.h"
#include "mledit.h"
#include "licq_events.h"
#include "licq_icqd.h"

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
  m_nMPChatPort = 0;
  m_bDeleteUser = false;

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

  chkAutoClose = new QCheckBox(tr("Aut&o Close"), this);
  chkAutoClose->setChecked(isAutoClose);
  l->addWidget(chkAutoClose);
  l->addSpacing(15);
  l->addStretch(1);

  int bw = 75;
  btnSave = new QPushButton(tr("&Save"), this);
  btnOk = new QPushButton(tr("O&k"), this);
  btnCancel = new CEButton(tr("&Close"), this);
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
  connect (sigman, SIGNAL(signal_updatedUser(CICQSignal *)),
           this, SLOT(slot_updatedUser(CICQSignal *)));
  connect (btnCancel, SIGNAL(clicked()), this, SLOT(slot_close()));
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
  QVBox *p = new QVBox(this, tabList[TAB_READ].label.latin1());
  p->setMargin(8);
  p->setSpacing(5);
  tabList[TAB_READ].tab = p;
  tabList[TAB_READ].loaded = true;

  QHGroupBox *h_top = new QHGroupBox(/*tr("Conversation"),*/ p);
  msgView = new MsgView(h_top);
  btnReadNext = new QPushButton(tr("Nex&t"), h_top);
  btnReadNext->setEnabled(false);
  btnReadNext->setFixedHeight(msgView->height());
  //btnReadNext->setFixedWidth(btnReadNext->width());
  connect(btnReadNext, SIGNAL(clicked()), this, SLOT(slot_nextMessage()));

  QHGroupBox *h_msg = new QHGroupBox(p);
  mleRead = new MLEditWrap(true, h_msg, true);
  mleRead->setReadOnly(true);
  p->setStretchFactor(h_msg, 1);

#if QT_VERSION >= 210
  connect (msgView, SIGNAL(clicked(QListViewItem *)), this, SLOT(slot_printMessage(QListViewItem *)));
#else
  connect (msgView, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(slot_printMessage(QListViewItem *)));
#endif

  QHGroupBox *h_action = new QHGroupBox(/*tr("Actions"),*/ p);
  btnRead1 = new QPushButton(h_action);
  btnRead2 = new QPushButton(h_action);
  btnRead3 = new QPushButton(h_action);
  btnRead4 = new QPushButton(h_action);

  btnRead1->setEnabled(false);
  btnRead2->setEnabled(false);
  btnRead3->setEnabled(false);
  btnRead4->setEnabled(false);

  connect(btnRead1, SIGNAL(clicked()), this, SLOT(slot_readbtn1()));
  connect(btnRead2, SIGNAL(clicked()), this, SLOT(slot_readbtn2()));
  connect(btnRead3, SIGNAL(clicked()), this, SLOT(slot_readbtn3()));
  connect(btnRead4, SIGNAL(clicked()), this, SLOT(slot_readbtn4()));
}


void ICQFunctions::CreateSendEventTab()
{
  tabList[TAB_SEND].label = tr("S&end");
  tabList[TAB_SEND].tab = new QWidget(this, tabList[TAB_SEND].label.latin1());
  QBoxLayout* selay = new QVBoxLayout(tabList[TAB_SEND].tab, 8);
  tabList[TAB_SEND].loaded = true;

  grpCmd = new QButtonGroup(1, Vertical, tr("Select Function"), tabList[TAB_SEND].tab);
  rdbMsg = new QRadioButton(tr("&Message"), grpCmd);
  rdbUrl = new QRadioButton(tr("&URL"), grpCmd);
  rdbChat = new QRadioButton(tr("Chat Re&quest"), grpCmd);
  rdbFile = new QRadioButton(tr("&File Transfer"), grpCmd);
  connect(grpCmd, SIGNAL(clicked(int)), this, SLOT(specialFcn(int)));
#if QT_VERSION < 210
  QWidget* dummy_w = new QWidget(grpCmd);
  dummy_w->setMinimumHeight(2);
#endif
  selay->addWidget(grpCmd);

  QHGroupBox *h_mid = new QHGroupBox(tabList[TAB_SEND].tab);
  mleSend = new MLEditWrap(true, h_mid, true);
  mleSend->setMinimumHeight(150);
  lstMultipleRecipients = new CMMUserView(mainwin->UserView()->ColInfo(),
     mainwin->showHeader, m_nUin, mainwin, h_mid);
  lstMultipleRecipients->setFixedWidth(mainwin->UserView()->width());
  lstMultipleRecipients->hide();
  //h_mid->setStretchFactor(mleSend, 1);
  selay->addWidget(h_mid);
  selay->setStretchFactor(h_mid, 1);

  grpOpt = new QGroupBox(3, Horizontal, tabList[TAB_SEND].tab);
  selay->addWidget(grpOpt);
  lblItem = new QLabel(grpOpt);
  edtItem = new CInfoField(grpOpt, false);
  btnItem = new QPushButton(grpOpt);
  connect(btnItem, SIGNAL(clicked()), SLOT(slot_sendbtn()));
#if QT_VERSION < 210
  QWidget* dummy_w2 = new QWidget(grpOpt);
  dummy_w2->setMinimumHeight(2);
#endif
  grpOpt->hide();

  QGroupBox *box = new QGroupBox(tabList[TAB_SEND].tab);
  selay->addWidget(box);

  QBoxLayout *vlay = new QVBoxLayout(box, 10, 5);

  QBoxLayout *hlay = new QHBoxLayout(vlay);

  chkSendServer = new QCheckBox(tr("Se&nd through server"), box);
  hlay->addWidget(chkSendServer);
  chkUrgent = new QCheckBox(tr("U&rgent"), box);
  hlay->addWidget(chkUrgent);
  chkMass = new QCheckBox(tr("&Multiple"), box);
  hlay->addWidget(chkMass);
  connect(chkMass, SIGNAL(toggled(bool)), this, SLOT(slot_masstoggled(bool)));

#ifdef USE_SPOOFING
  hlay = new QHBoxLayout(vlay);
  chkSpoof = new QCheckBox(tr("S&poof UIN:"), box);
  hlay->addWidget(chkSpoof);
  edtSpoof = new QLineEdit(box);
  hlay->addWidget(edtSpoof);
  edtSpoof->setEnabled(false);
  edtSpoof->setValidator(new QIntValidator(10000, 2000000000, edtSpoof));
  connect(chkSpoof, SIGNAL(toggled(bool)), edtSpoof, SLOT(setEnabled(bool)));
#else
  edtSpoof = 0;
  chkSpoof = 0;
#endif
  selay->activate();
}


void ICQFunctions::slot_masstoggled(bool b)
{
  b ? lstMultipleRecipients->show() : lstMultipleRecipients->hide();
}



void ICQFunctions::CreateGeneralInfoTab()
{
  tabList[TAB_GENERALINFO].label = tr("&General");
  tabList[TAB_GENERALINFO].tab = new QWidget(this, tabList[TAB_GENERALINFO].label.latin1());
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
  tabList[TAB_MOREINFO].label = tr("&More");
  tabList[TAB_MOREINFO].tab = new QWidget(this, tabList[TAB_MOREINFO].label.latin1());
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
  tabList[TAB_WORKINFO].label = tr("&Work");
  tabList[TAB_WORKINFO].tab = new QWidget(this, tabList[TAB_WORKINFO].label.latin1());
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
  tabList[TAB_ABOUT].label = tr("&About");
  tabList[TAB_ABOUT].tab = new QVBox(this, tabList[TAB_ABOUT].label.latin1());
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
  tabList[TAB_HISTORY].label = tr("&History");
  tabList[TAB_HISTORY].tab = new QWidget(this, tabList[TAB_HISTORY].label.latin1());
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

  chkHistoryReverse = new QCheckBox(tr("Rever&se"), p);
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
  if (u->NewMessages() > 0) btnReadNext->setEnabled(true);

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
    CICQSignal s(SIGNAL_UPDATExUSER, USER_BASIC, m_nUin);
    emit signal_updatedUser(&s);
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
     btnOk->show();
     currentTab = TAB_SEND;
  }
  else if (tab == tabList[TAB_GENERALINFO].label)
  {
     btnOk->setText(tr("&Update"));
     btnSave->setText(tr("&Save"));
     btnSave->show();
     btnOk->show();
     currentTab = TAB_GENERALINFO;
  }
  else if (tab == tabList[TAB_READ].label)
  {
     btnOk->setText(tr("Ok"));
     btnOk->hide();
     btnSave->hide();
     msgView->triggerUpdate();
     currentTab = TAB_READ;
  }
  else if (tab == tabList[TAB_MOREINFO].label)
  {
     btnOk->setText(tr("&Update"));
     btnOk->show();
     btnSave->setText(tr("&Save"));
     btnSave->show();
     currentTab = TAB_MOREINFO;
  }
  else if (tab == tabList[TAB_WORKINFO].label)
  {
     btnOk->setText(tr("&Update"));
     btnOk->show();
     btnSave->setText(tr("&Save"));
     btnSave->show();
     currentTab = TAB_WORKINFO;
  }
  else if (tab == tabList[TAB_ABOUT].label)
  {
     btnOk->setText(tr("&Update"));
     btnOk->show();
     btnSave->setText(tr("&Save"));
     btnSave->show();
     currentTab = TAB_ABOUT;
  }
  else if (tab == tabList[TAB_HISTORY].label)
  {
     btnOk->setText(tr("Nex&t"));
     btnOk->show();
     btnSave->setText(tr("P&rev"));
     btnSave->show();
     currentTab = TAB_HISTORY;
  }
  else
  {
     btnOk->setText(tr("&Ok"));
     btnOk->show();
  }
}


//-----slot_updatedUser--------------------------------------------------------
void ICQFunctions::slot_updatedUser(CICQSignal *sig)
{
  if (m_nUin != sig->Uin()) return;

  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
  if(u == NULL) return;
  switch (sig->SubSignal())
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
      btnReadNext->setEnabled(true);
      btnReadNext->setText(tr("Nex&t\n(%1)").arg(u->NewMessages()));
    }
    else if (u->NewMessages() == 1)
    {
      btnReadNext->setEnabled(true);
      btnReadNext->setText(tr("Nex&t"));
    }
    else
    {
      btnReadNext->setEnabled(false);
      btnReadNext->setText(tr("Nex&t"));
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
  btnReadNext->setEnabled(u->NewMessages() > 0);
  if (u->NewMessages() > 1)
  {
    btnReadNext->setEnabled(true);
    btnReadNext->setText(tr("Nex&t\n(%1)").arg(u->NewMessages()));
  }
  else if (u->NewMessages() == 1)
  {
    btnReadNext->setEnabled(true);
    btnReadNext->setText(tr("Nex&t"));
  }
  else
  {
    btnReadNext->setEnabled(false);
    btnReadNext->setText(tr("Nex&t"));
  }

  gUserManager.DropUser(u);

  msgView->setSelected(e, true);
  slot_printMessage(e);
}


//-----ICQFunctions::printMessage----------------------------------------------
void ICQFunctions::slot_printMessage(QListViewItem *e)
{
  if (e == NULL)
    return;

  btnRead1->setText("");
  btnRead2->setText("");
  btnRead3->setText("");
  btnRead4->setText("");
  btnRead1->setEnabled(false);
  btnRead2->setEnabled(false);
  btnRead3->setEnabled(false);
  btnRead4->setEnabled(false);

  CUserEvent *m = ((MsgViewItem *)e)->msg;
  m_xCurrentReadEvent = m;
  mleRead->setText(QString::fromLocal8Bit(m->Text()));
  if (m->Direction() == D_RECEIVER && (m->Command() == ICQ_CMDxTCP_START || m->Command() == ICQ_CMDxRCV_SYSxMSGxONLINE))
  {
    switch (m->SubCommand())
    {
      case ICQ_CMDxSUB_CHAT:  // accept or refuse a chat request
      case ICQ_CMDxSUB_FILE:  // accept or refuse a file transfer
        if (m->IsCancelled())
        {
          mleRead->append(tr("\n--------------------\nRequest was cancelled."));
        }
        else
        {
          btnRead1->setText(tr("A&ccept"));
          btnRead2->setText(tr("&Refuse"));
          // If this is a chat, and we already have chats going, and this is
          // not a join request, then we can join
          if (m->SubCommand() == ICQ_CMDxSUB_CHAT &&
              ChatDlg::chatDlgs.size() > 0 &&
              ((CEventChat *)m)->Port() == 0)
            btnRead3->setText(tr("&Join"));
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
        btnRead2->setText(tr("&Refuse"));
        ICQUser *u = gUserManager.FetchUser( ((CEventAuthRequest *)m)->Uin(), LOCK_R);
        if (u == NULL)
          btnRead3->setText(tr("A&dd User"));
        else
          gUserManager.DropUser(u);
        break;
      }
      case ICQ_CMDxSUB_AUTHxGRANTED:
      {
        ICQUser *u = gUserManager.FetchUser( ((CEventAuthGranted *)m)->Uin(), LOCK_R);
        if (u == NULL)
          btnRead1->setText(tr("A&dd User"));
        else
          gUserManager.DropUser(u);
        break;
      }
      case ICQ_CMDxSUB_ADDEDxTOxLIST:
      {
        ICQUser *u = gUserManager.FetchUser( ((CEventAdded *)m)->Uin(), LOCK_R);
        if (u == NULL)
          btnRead1->setText(tr("A&dd User"));
        else
          gUserManager.DropUser(u);
        break;
      }
    } // switch
  }  // if

  if (!btnRead1->text().isEmpty()) btnRead1->setEnabled(true);
  if (!btnRead2->text().isEmpty()) btnRead2->setEnabled(true);
  if (!btnRead3->text().isEmpty()) btnRead3->setEnabled(true);
  if (!btnRead4->text().isEmpty()) btnRead4->setEnabled(true);
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
      if (c->Port() != 0)  // Joining a multiparty chat (we connect to them)
      {
        if (chatDlg->StartAsClient(c->Port()))
          server->icqChatRequestAccept(m_nUin, chatDlg->LocalPort(), c->Sequence());
      }
      else  // single party (other side connects to us)
      {
        if (chatDlg->StartAsServer())
          server->icqChatRequestAccept(m_nUin, chatDlg->LocalPort(), c->Sequence());
      }
      break;
    }

    case ICQ_CMDxSUB_FILE:  // accept a file transfer
    {
      CEventFile *f = (CEventFile *)m_xCurrentReadEvent;
      CFileDlg *fileDlg = new CFileDlg(m_nUin, server);
      if (fileDlg->ReceiveFiles())
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
      server->icqAuthorizeGrant( ((CEventAuthRequest *)m_xCurrentReadEvent)->Uin(), "" );
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
        server->icqChatRequestRefuse(m_nUin, r->RefuseMessage().local8Bit(),
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
        server->icqFileTransferRefuse(m_nUin, r->RefuseMessage().local8Bit(),
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
      server->icqAuthorizeRefuse( ((CEventAuthRequest *)m_xCurrentReadEvent)->Uin(), "" );
      break;
  } // switch

}


void ICQFunctions::slot_readbtn3()
{
  if (m_xCurrentReadEvent == NULL) return;

  switch (m_xCurrentReadEvent->SubCommand())
  {
    case ICQ_CMDxSUB_URL:   // view a url
      if (!server->ViewUrl(((CEventUrl *)m_xCurrentReadEvent)->Url()))
        WarnUser(this, tr("View URL failed"));
      break;

    case ICQ_CMDxSUB_CHAT:  // join to current chat
    {
      CEventChat *c = (CEventChat *)m_xCurrentReadEvent;
      if (c->Port() != 0)  // Joining a multiparty chat (we connect to them)
      {
        ChatDlg *chatDlg = new ChatDlg(m_nUin, server);
        if (chatDlg->StartAsClient(c->Port()))
          server->icqChatRequestAccept(m_nUin, chatDlg->LocalPort(), c->Sequence());
      }
      else  // single party (other side connects to us)
      {
        ChatDlg *chatDlg = NULL;
        CJoinChatDlg *j = new CJoinChatDlg(this);
        if (j->exec() && (chatDlg = j->JoinedChat()) != NULL)
          server->icqChatRequestAccept(m_nUin, chatDlg->LocalPort(), c->Sequence());
        delete j;
      }
      break;
    }

    case ICQ_CMDxSUB_AUTHxREQUEST:
      server->AddUserToList( ((CEventAuthRequest *)m_xCurrentReadEvent)->Uin());
      break;

  }
}


void ICQFunctions::slot_readbtn4()
{
}


void ICQFunctions::slot_sendbtn()
{
  if (icqEventTag != NULL) return;

  if (rdbUrl->isChecked())
  {
    if (!server->ViewUrl(edtItem->text().local8Bit()))
      WarnUser(this, tr("View URL failed"));
  }
  else if (rdbChat->isChecked())
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
          m_szMPChatClients = chatDlg->ChatClients();
        }
        delete j;
        btnItem->setText(tr("Clear"));
      }
    }
    else
    {
      m_nMPChatPort = 0;
      m_szMPChatClients = "";
      edtItem->setText("");
      btnItem->setText(tr("Invite"));
    }
  }
  else if (rdbFile->isChecked())
  {
#ifdef USE_KDE
    // !!! FIXME !!!!
    //QStringList fl = KFileDialog::getOpenFileNames(NULL, NULL, this);
    QStringList fl = QFileDialog::getOpenFileNames(NULL, NULL, this);
#else
    QStringList fl = QFileDialog::getOpenFileNames(NULL, NULL, this);
#endif
    if (fl.isEmpty()) return;
    QStringList::ConstIterator it;
    QString f;
    for( it = fl.begin(); it != fl.end(); it++ )
    {
      if (it != fl.begin())
        f += ", ";
      f += (*it);
    }
    edtItem->setText(f);
  }
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
  u->SetEmail1(nfoEmail1->text().local8Bit());
  u->SetEmail2(nfoEmail2->text().local8Bit());
  u->SetCity(nfoCity->text().local8Bit());
  u->SetState(nfoState->text().local8Bit());
  u->SetAddress(nfoAddress->text().local8Bit());
  u->SetPhoneNumber(nfoPhone->text().local8Bit());
  u->SetFaxNumber(nfoFax->text().local8Bit());
  u->SetCellularNumber(nfoCellular->text().local8Bit());
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

  u->SetAge(nfoAge->text().toULong());
  u->SetHomepage(nfoHomepage->text().local8Bit());
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
                '\001', EventDescription(*tempIter).data(),
                tr("from").utf8().data(), n.utf8().data(), '\001',
                d.toString().utf8().data(),
                (*tempIter)->IsDirect() ? 'D' : '-',
                (*tempIter)->IsMultiRec() ? 'M' : '-',
                (*tempIter)->IsUrgent() ? 'U' : '-',
                (QString::fromLocal8Bit((*tempIter)->Text())).utf8().data());
    else
      s.sprintf("%c%s %s %s\n%c%s [%c%c%c]\n\n%s\n\n",
                '\002', EventDescription(*tempIter).data(),
                tr("to").utf8().data(), n.utf8().data(), '\002',
                d.toString().utf8().data(),
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
  QString s;
  for (int i = 0; i < mleRead->numLines(); i++)
     s += QString("> ") + mleRead->textLine(i) + "\n";
  mleSend->setText(s);
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
                   tr("&Ok"), tr("&Cancel")))
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
    chkSendServer->setEnabled(true);
    chkMass->setEnabled(true);
    break;
  case 1:  // Url
    lblItem->setText(tr("URL:"));
    btnItem->setText(tr("View"));
    edtItem->clear();
    edtItem->SetReadOnly(false);
    //btnItem->setEnabled(true);
    grpOpt->show();
    tabs->updateGeometry();
    chkSendServer->setEnabled(true);
    chkMass->setEnabled(true);
    break;
  case 2: // Chat
    lblItem->setText(tr("Multiparty:"));
    btnItem->setText(tr("Invite"));
    edtItem->SetReadOnly(true);
    //btnItem->setEnabled(ChatDlg::chatDlgs.size() > 0);
    edtItem->clear();
    grpOpt->show();
    tabs->updateGeometry();
    chkSendServer->setChecked(false);
    chkSendServer->setEnabled(false);
    chkMass->setChecked(false);
    chkMass->setEnabled(false);
    break;
  case 3:  // File transfer
    lblItem->setText(tr("Filename:"));
    btnItem->setText(tr("Choose"));
    edtItem->clear();
    edtItem->SetReadOnly(false);
    //btnItem->setEnabled(true);
    grpOpt->show();
    tabs->updateGeometry();
    chkSendServer->setChecked(false);
    chkSendServer->setEnabled(false);
    chkMass->setChecked(false);
    chkMass->setEnabled(false);
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
                      tr("C&ontinue"), tr("&Cancel")))
        break;

    unsigned long uin = (chkSpoof && chkSpoof->isChecked() ?
                         edtSpoof->text().toULong() : 0);
    if (rdbMsg->isChecked())  // send a message
    {
      if (chkMass->isChecked())
      {
        CMMSendDlg *m = new CMMSendDlg(server, sigman, lstMultipleRecipients, this);
        int r = m->go_message(mleSend->text());
        delete m;
        if (r != QDialog::Accepted) break;
      }
      m_sProgressMsg = tr("Sending msg ");
      m_sProgressMsg += chkSendServer->isChecked() ? tr("through server") : tr("direct");
      m_sProgressMsg += "...";
      icqEventTag = server->icqSendMessage(m_nUin, mleSend->text().local8Bit(),
         chkSendServer->isChecked() ? false : true,
         chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL, uin);
    }
    else if (rdbUrl->isChecked()) // send URL
    {
      if (chkMass->isChecked())
      {
        CMMSendDlg *m = new CMMSendDlg(server, sigman, lstMultipleRecipients, this);
        int r = m->go_url(edtItem->text(), mleSend->text());
        delete m;
        if (r != QDialog::Accepted) break;
      }
      m_sProgressMsg = tr("Sending URL ");
      m_sProgressMsg += chkSendServer->isChecked() ? tr("through server") : tr("direct");
      m_sProgressMsg += "...";
      icqEventTag = server->icqSendUrl(m_nUin, edtItem->text().latin1(), mleSend->text().local8Bit(),
         chkSendServer->isChecked() ? false : true,
         chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL, uin);
    }
    else if (rdbChat->isChecked())   // send chat request
    {
      m_sProgressMsg = tr("Sending chat request...");
      if (m_nMPChatPort == 0)
        icqEventTag = server->icqChatRequest(m_nUin,
           mleSend->text().local8Bit(),
           chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL);
      else
        icqEventTag = server->icqMultiPartyChatRequest(m_nUin,
           mleSend->text().local8Bit(), m_szMPChatClients.local8Bit(),
           m_nMPChatPort,
           chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL);
    }
    else if (rdbFile->isChecked())   // send file transfer
    {
      if (edtItem->text().isEmpty())
      {
        WarnUser(this, tr("You must specify a file to transfer!"));
        break;
      }
      m_sProgressMsg = tr("Sending file transfer...");
      icqEventTag = server->icqFileTransfer(m_nUin, edtItem->text().local8Bit(),
         mleSend->text().local8Bit(),
         chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL);
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
                                           nfoHomepage->text().local8Bit(),
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
    btnCancel->setText(tr("&Cancel"));
    connect (sigman, SIGNAL(signal_doneUserFcn(ICQEvent *)), this, SLOT(doneFcn(ICQEvent *)));
  }
}


//-----RetrySend-------------------------------------------------------------
void ICQFunctions::RetrySend(ICQEvent *e, bool bOnline, unsigned short nLevel)
{
  btnOk->setEnabled(false);
  btnCancel->setText(tr("&Cancel"));
  if (!bOnline) chkSendServer->setChecked(true);

  chkUrgent->setChecked(nLevel == ICQ_TCPxMSG_URGENT);

  switch(e->SubCommand())
  {
    case ICQ_CMDxSUB_MSG:
    {
      CEventMsg *ue = (CEventMsg *)e->UserEvent();
      m_sProgressMsg = tr("Sending msg ");
      m_sProgressMsg += bOnline ? tr("direct") : tr("through server");
      m_sProgressMsg += "...";
      icqEventTag = server->icqSendMessage(m_nUin, ue->Message(), bOnline,
         nLevel, 0);
      break;
    }
    case ICQ_CMDxSUB_URL:
    {
      CEventUrl *ue = (CEventUrl *)e->UserEvent();
      m_sProgressMsg = tr("Sending URL ");
      m_sProgressMsg += bOnline ? tr("direct") : tr("through server");
      m_sProgressMsg += "...";
      icqEventTag = server->icqSendUrl(m_nUin, ue->Url(), ue->Description(),
         bOnline, nLevel, 0);
      break;
    }
    case ICQ_CMDxSUB_CHAT:
    {
      CEventChat *ue = (CEventChat *)e->UserEvent();
      m_sProgressMsg = tr("Sending chat request...");
      icqEventTag = server->icqChatRequest(m_nUin, ue->Reason(), nLevel);
      break;
    }
    case ICQ_CMDxSUB_FILE:
    {
      CEventFile *ue = (CEventFile *)e->UserEvent();
      m_sProgressMsg = tr("Sending file transfer...");
      icqEventTag = server->icqFileTransfer(m_nUin, ue->Filename(),
         ue->FileDescription(), nLevel);
      break;
    }
  }

  QString title = m_sBaseTitle + " [" + m_sProgressMsg + "]";
  setCaption(title);
}


//-----ICQFunctions::doneFcn-------------------------------------------------
void ICQFunctions::doneFcn(ICQEvent *e)
{
  if ( !icqEventTag->Equals(e) )
    return;

  bool isOk = (e != NULL && (e->Result() == EVENT_ACKED || e->Result() == EVENT_SUCCESS));
  bool bForceOpen = !chkAutoClose->isChecked();

  QString title, result;
  if (e == NULL)
  {
    result = tr("error");
  }
  else
  {
    switch (e->Result())
    {
    case EVENT_ACKED:
    case EVENT_SUCCESS:
      result = tr("done");
      QTimer::singleShot(5000, this, SLOT(slot_resettitle()));
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
  disconnect (sigman, SIGNAL(signal_doneUserFcn(ICQEvent *)), this, SLOT(doneFcn(ICQEvent *)));

  if (e == NULL) return;

  if (!isOk)
  {
    if (e->Command() == ICQ_CMDxTCP_START &&
        (e->SubCommand() == ICQ_CMDxSUB_MSG ||
         e->SubCommand() == ICQ_CMDxSUB_URL) &&
        QueryUser(this, tr("Direct send failed,\nsend through server?"), tr("Yes"), tr("No")) )
    {
      RetrySend(e, false, ICQ_TCPxMSG_NORMAL);
    }
  }
  else
  {
    switch(e->Command())
    {
      case ICQ_CMDxTCP_START:
      {
        ICQUser *u = NULL;
        CUserEvent *ue = e->UserEvent();
        QString msg;
        if (e->SubResult() == ICQ_TCPxACK_RETURN)
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
        else if (e->SubResult() == ICQ_TCPxACK_REFUSE)
        {
          u = gUserManager.FetchUser(m_nUin, LOCK_R);
          msg = tr("%1 refused %2, send through server.")
                .arg(u->GetAlias()).arg(EventDescription(ue));
          InformUser(this, msg);
          gUserManager.DropUser(u);
          bForceOpen = true;
        }
        else if (e->SubCommand() == ICQ_CMDxSUB_CHAT || e->SubCommand() == ICQ_CMDxSUB_FILE)
        {
          CExtendedAck *ea = e->ExtendedAck();
          if (ea == NULL || ue == NULL)
          {
            gLog.Error("%sInternal error: ICQFunctions::doneFcn(): chat or file request acknowledgement without extended result.\n", L_ERRORxSTR);
            return;
          }
          if (!ea->Accepted())
          {
             u = gUserManager.FetchUser(m_nUin, LOCK_R);
             QString result = tr("%1 with %2 refused:\n%3")
                .arg(EventDescription(ue)).arg(u->GetAlias()).arg(ea->Response());
             gUserManager.DropUser(u);
             InformUser(this, result);
          }
          else
          {
            switch (e->SubCommand())
            {
            case ICQ_CMDxSUB_CHAT:
            {
              CEventChat *c = (CEventChat *)ue;
              if (c->Port() == 0)  // If we requested a join, no need to do anything
              {
                ChatDlg *chatDlg = new ChatDlg(m_nUin, server);
                chatDlg->StartAsClient(ea->Port());
              }
              break;
            }
            case ICQ_CMDxSUB_FILE:
            {
              CEventFile *f = (CEventFile *)ue;
              CFileDlg *fileDlg = new CFileDlg(m_nUin, server);
              fileDlg->SendFiles(f->Filename(), ea->Port());
              break;
            }
            default:
              break;
            } // case
          } // if accepted

          if (bForceOpen)
          {
            (void) new MsgViewItem(e->GrabUserEvent(), msgView);
            mleSend->clear();
          }
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

          if (bForceOpen)
          {
            (void) new MsgViewItem(e->GrabUserEvent(), msgView);
            mleSend->clear();
          }
        }

        break;
      } // case
      case ICQ_CMDxSND_THRUxSERVER:
      {
        if (bForceOpen)
        {
          (void) new MsgViewItem(e->GrabUserEvent(), msgView);
          mleSend->clear();
        }
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

    if (!bForceOpen) close();
  }

}


void ICQFunctions::slot_resettitle()
{
  setCaption(m_sBaseTitle);
}


void ICQFunctions::slot_close()
{
  m_bDeleteUser = btnCancel->MouseEvent()->state() & ControlButton;
  close();
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
    if (m_bDeleteUser && !m_bOwner)
    {
      mainwin->RemoveUserFromList(m_nUin, this);
    }
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
