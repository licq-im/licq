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


// -----------------------------------------------------------------------------

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qvbox.h>
#include <qcheckbox.h>
#include <qdatetime.h>
#include <qlayout.h>
#include <qspinbox.h>
#include <qtabwidget.h>

#include "licq_countrycodes.h"
#include "licq_events.h"
#include "licq_languagecodes.h"
#include "licq_user.h"
#include "licq_icqd.h"

#include "editfile.h"
#include "ewidgets.h"
#include "eventdesc.h"
#include "gui-defines.h"
#include "userinfodlg.h"
#include "sigman.h"

// -----------------------------------------------------------------------------

UserInfoDlg::UserInfoDlg(CICQDaemon *s, CSignalManager *theSigMan, CMainWindow *m,
                         unsigned long _nUin, QWidget* parent)
  : QDialog(parent, "UserInfoDialog", false, WDestructiveClose)
{
  server = s;
  mainwin = m;
  sigman = theSigMan;
  icqEventTag = NULL;
  m_nUin = _nUin;
  m_bOwner = (m_nUin == gUserManager.OwnerUin());

  CreateGeneralInfo();
  CreateMoreInfo();
  CreateWorkInfo();
  CreateAbout();
  CreateHistory();

  QBoxLayout* lay = new QVBoxLayout(this, 8);

  lblStatus = new QLabel(this);
  lblStatus->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
  lay->addWidget(lblStatus);

  tabs = new QTabWidget(this);
  lay->addWidget(tabs, 2);

  tabs->addTab(tabList[GeneralInfo].tab, tabList[GeneralInfo].label);
  tabs->addTab(tabList[MoreInfo].tab, tabList[MoreInfo].label);
  tabs->addTab(tabList[WorkInfo].tab, tabList[WorkInfo].label);
  tabs->addTab(tabList[AboutInfo].tab, tabList[AboutInfo].label);
  tabs->addTab(tabList[HistoryInfo].tab, tabList[HistoryInfo].label);

  connect (tabs, SIGNAL(selected(const QString &)), this, SLOT(updateTab(const QString &)));

  QBoxLayout* l = new QHBoxLayout(lay);
  l->addStretch(2);

  btnSave = new QPushButton(tr("&Save"), this);
  l->addWidget(btnSave);
  connect(btnSave, SIGNAL(clicked()), this, SLOT(SaveSettings()));

  btnOk = new QPushButton(tr("&OK"), this);
  l->addWidget(btnOk);
  connect(btnOk, SIGNAL(clicked()), this, SLOT(slotOk()));

  btnClose = new QPushButton(tr("&Close"), this);
  l->addWidget(btnClose);
  connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));

  ICQUser* u = gUserManager.FetchUser(m_nUin, LOCK_R);

  setCaption(tr("Licq - User Info: ") + QString::fromLocal8Bit(u->GetAlias()) + " (" +
             QString::fromLocal8Bit(u->GetFirstName()) + " " +
             QString::fromLocal8Bit(u->GetLastName())+ ")");
  setIconText(u->GetAlias());
  gUserManager.DropUser(u);
}


// -----------------------------------------------------------------------------

UserInfoDlg::~UserInfoDlg()
{
  if (icqEventTag != NULL)
  {
    server->CancelEvent(icqEventTag);
    delete icqEventTag;
    icqEventTag = NULL;
  }
  emit finished(m_nUin);
  ICQUser::ClearHistory(m_lHistoryList);
}


// -----------------------------------------------------------------------------

void UserInfoDlg::showTab(int tab)
{
  tabs->showPage(tabList[tab].tab);
}


// -----------------------------------------------------------------------------

void UserInfoDlg::CreateGeneralInfo()
{
  tabList[GeneralInfo].label = tr("&General");
  tabList[GeneralInfo].tab = new QWidget(this, tabList[GeneralInfo].label.latin1());
  tabList[GeneralInfo].loaded = false;

  unsigned short CR = 0;
  QWidget *p = tabList[GeneralInfo].tab;

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
    cmbCountry = new CEComboBox(true, tabList[GeneralInfo].tab);
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

void UserInfoDlg::SetGeneralInfo(ICQUser *u)
{
  tabList[GeneralInfo].loaded = true;
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

  if (bDropUser) gUserManager.DropUser(u);
}

void UserInfoDlg::SaveGeneralInfo()
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

// -----------------------------------------------------------------------------

void UserInfoDlg::CreateMoreInfo()
{
  tabList[MoreInfo].label = tr("&More");
  tabList[MoreInfo].tab = new QWidget(this, tabList[MoreInfo].label.latin1());
  tabList[MoreInfo].loaded = false;

  unsigned short CR = 0;
  QWidget *p = tabList[MoreInfo].tab;
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

void UserInfoDlg::SetMoreInfo(ICQUser *u)
{
  tabList[MoreInfo].loaded = true;
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

void UserInfoDlg::SaveMoreInfo()
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

// -----------------------------------------------------------------------------

void UserInfoDlg::CreateWorkInfo()
{
  tabList[WorkInfo].label = tr("&Work");
  tabList[WorkInfo].tab = new QWidget(this, tabList[WorkInfo].label.latin1());
  tabList[WorkInfo].loaded = false;

  unsigned short CR = 0;
  QWidget *p = tabList[WorkInfo].tab;

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

void UserInfoDlg::SetWorkInfo(ICQUser *u)
{
  tabList[WorkInfo].loaded = true;
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

// -----------------------------------------------------------------------------

void UserInfoDlg::CreateAbout()
{
  tabList[AboutInfo].label = tr("&About");
  tabList[AboutInfo].tab = new QVBox(this, tabList[AboutInfo].label.latin1());
  tabList[AboutInfo].loaded = false;

  QVBox *p = (QVBox *)tabList[AboutInfo].tab;

  p->setMargin(8);
  p->setSpacing(8);

  lblAbout = new QLabel(tr("About:"), p);
  mleAbout = new MLEditWrap(true, p);
  mleAbout->setReadOnly(!m_bOwner);
  // Windows ICQ seems to limit the about info to 450 chars
  // so we do the same
  mleAbout->setMaxLength(450);
}

void UserInfoDlg::SetAbout(ICQUser *u)
{
  tabList[AboutInfo].loaded = true;
  bool bDropUser = false;

  if (u == NULL)
  {
    u = gUserManager.FetchUser(m_nUin, LOCK_R);
    bDropUser = true;
  }

  mleAbout->setText(QString::fromLocal8Bit(u->GetAbout()));

  if (bDropUser) gUserManager.DropUser(u);
}

void UserInfoDlg::SaveAbout()
{
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_W);
  u->SetAbout(mleAbout->text().local8Bit());
  gUserManager.DropUser(u);
}

// -----------------------------------------------------------------------------

void UserInfoDlg::CreateHistory()
{
  tabList[HistoryInfo].label = tr("&History");
  tabList[HistoryInfo].tab = new QWidget(this, tabList[HistoryInfo].label.latin1());
  tabList[HistoryInfo].loaded = false;

  QWidget *p = tabList[HistoryInfo].tab;

  QBoxLayout* lay = new QVBoxLayout(p, 8, 8);

  QBoxLayout* l = new QHBoxLayout(lay);

  lblHistory = new QLabel(p);
  lblHistory->setAlignment(AlignLeft | AlignVCenter | SingleLine);
  l->addWidget(lblHistory, 1);

  chkHistoryReverse = new QCheckBox(tr("Rever&se"), p);
  connect(chkHistoryReverse, SIGNAL(toggled(bool)), SLOT(HistoryReverse(bool)));
  chkHistoryReverse->setChecked(true);
  chkHistoryReverse->setFixedSize(chkHistoryReverse->sizeHint());
  l->addWidget(chkHistoryReverse);

  mleHistory = new CHistoryWidget(p);
  lay->addWidget(mleHistory, 1);
}

void UserInfoDlg::SetupHistory()
{
  tabList[HistoryInfo].loaded = true;
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
  if (!u->GetHistory(m_lHistoryList))
  {
    if(u->HistoryFile())
      mleHistory->setText(tr("Error loading history file: %1\nDescription: %2")
        .arg(u->HistoryFile()).arg(u->HistoryName()));
    else
      mleHistory->setText(tr("Sorry, history is disabled for this person."));
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

// -----------------------------------------------------------------------------


void UserInfoDlg::HistoryReload()
{
  ICQUser::ClearHistory(m_lHistoryList);
  SetupHistory();
}

void UserInfoDlg::HistoryEdit()
{
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
  if (u == NULL) return;

  (void) new EditFileDlg(u->HistoryFile());

  gUserManager.DropUser(u);
}


void UserInfoDlg::HistoryReverse(bool newVal)
{
  if (chkHistoryReverse->isChecked() != newVal)
    chkHistoryReverse->setChecked(newVal);
  else if(m_bHistoryReverse != newVal)
  {
    m_bHistoryReverse = newVal;
    ShowHistory();
  }
}

void UserInfoDlg::ShowHistoryPrev()
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

void UserInfoDlg::ShowHistoryNext()
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

void UserInfoDlg::ShowHistory()
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


void UserInfoDlg::SaveHistory()
{
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
  u->SaveHistory(mleHistory->text().local8Bit());
  gUserManager.DropUser(u);
}


// -----------------------------------------------------------------------------

void UserInfoDlg::updateTab(const QString& txt)
{
  if (txt == tabList[GeneralInfo].label)
  {
    currentTab = GeneralInfo;
    btnOk->setText(tr("&Update"));
    btnSave->setText(tr("&Save"));
    if (!tabList[GeneralInfo].loaded)
      SetGeneralInfo(NULL);
  }
  else if (txt == tabList[MoreInfo].label)
  {
    btnOk->setText(tr("&Update"));
    btnSave->setText(tr("&Save"));
    currentTab = MoreInfo;
    if (!tabList[MoreInfo].loaded)
      SetMoreInfo(NULL);
  }
  else if (txt == tabList[WorkInfo].label)
  {
    btnOk->setText(tr("&Update"));
    btnSave->setText(tr("&Save"));
    currentTab = WorkInfo;
    if (!tabList[WorkInfo].loaded)
      SetWorkInfo(NULL);
  }
  else if (txt == tabList[AboutInfo].label)
  {
    btnOk->setText(tr("&Update"));
    btnSave->setText(tr("&Save"));
    currentTab = AboutInfo;
    if (!tabList[AboutInfo].loaded)
      SetAbout(NULL);
  }
  else if (txt == tabList[HistoryInfo].label)
  {
    btnOk->setText(tr("Nex&t"));
    btnSave->setText(tr("P&rev"));
    currentTab = HistoryInfo;
     if (!tabList[HistoryInfo].loaded)
       SetupHistory();
  }
}

void UserInfoDlg::SaveSettings()
{
  switch(currentTab) {
  case GeneralInfo:
  {
    SaveGeneralInfo();
    CICQSignal s(SIGNAL_UPDATExUSER, USER_GENERAL, m_nUin);
    emit signal_updatedUser(&s);
    break;
  }
  case MoreInfo:
    SaveMoreInfo();
    break;
  case WorkInfo:
    SaveMoreInfo();
    break;
  case AboutInfo:
    SaveAbout();
    break;
  case HistoryInfo:
    ShowHistoryPrev();
    break;
  }
}

void UserInfoDlg::slotOk()
{
  switch(currentTab) {
  case GeneralInfo:
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
  case MoreInfo:
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
  case WorkInfo:
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
  case AboutInfo:
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
  case HistoryInfo:
    ShowHistoryNext();
    break;
  }

  if (icqEventTag != NULL)
  {
    setCursor(waitCursor);
    connect (sigman, SIGNAL(signal_doneUserFcn(ICQEvent *)), this, SLOT(doneFunction(ICQEvent *)));
    lblStatus->setText(m_sProgressMsg);
  }
}

void UserInfoDlg::doneFunction(ICQEvent* e)
{
  if ( !icqEventTag->Equals(e) )
    return;

  QString title, result;
  if (e == NULL)
    result = tr("error");
  else
  {
    switch (e->Result())
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

  lblStatus->setText(m_sProgressMsg + " [" + result + "]");
  setCursor(arrowCursor);
  delete icqEventTag;
  icqEventTag = NULL;
  disconnect (sigman, SIGNAL(signal_doneUserFcn(ICQEvent *)), this, SLOT(doneFunction(ICQEvent *)));
}



// -----------------------------------------------------------------------------

#include "userinfodlg.moc"

