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
#include <qtimer.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qtextcodec.h>
#include <qaccel.h>
#include <qtimer.h>
#include <qwhatsthis.h>
#include <ctype.h>

#include "licq_countrycodes.h"
#include "licq_events.h"
#include "licq_languagecodes.h"
#include "licq_user.h"
#include "licq_icqd.h"
#include "licq_socket.h"
#include "usercodec.h"

#include "editfile.h"
#include "ewidgets.h"
#include "eventdesc.h"
#include "gui-defines.h"
#include "userinfodlg.h"
#include "sigman.h"
#include "mledit.h"
#include "mlview.h"

// -----------------------------------------------------------------------------
UserInfoDlg::UserInfoDlg(CICQDaemon *s, CSignalManager *theSigMan, CMainWindow *m,
                         const char *szId, unsigned long nPPID, QWidget* parent)
  : QWidget(parent, "UserInfoDialog", WDestructiveClose)
{
  server = s;
  mainwin = m;
  sigman = theSigMan;
  icqEventTag = 0;
  m_szId = szId ? strdup(szId) : 0;
  m_nPPID = nPPID;
  m_bOwner = (gUserManager.FindOwner(szId, nPPID) != NULL);

  CreateGeneralInfo();
  CreateMoreInfo();
  CreateWorkInfo();
  CreateAbout();
  CreateHistory();
  CreateLastCountersInfo();

  QBoxLayout *lay = new QVBoxLayout(this, 8);

  tabs = new QTabWidget(this);
  lay->addWidget(tabs, 2);

  tabs->addTab(tabList[GeneralInfo].tab, tabList[GeneralInfo].label);
  tabs->addTab(tabList[MoreInfo].tab, tabList[MoreInfo].label);
  tabs->addTab(tabList[WorkInfo].tab, tabList[WorkInfo].label);
  tabs->addTab(tabList[AboutInfo].tab, tabList[AboutInfo].label);
  tabs->addTab(tabList[HistoryInfo].tab, tabList[HistoryInfo].label);
  tabs->addTab(tabList[LastCountersInfo].tab, tabList[LastCountersInfo].label);

  connect (tabs, SIGNAL(selected(const QString &)), this, SLOT(updateTab(const QString &)));
  connect (sigman, SIGNAL(signal_updatedUser(CICQSignal *)),
           this, SLOT(updatedUser(CICQSignal *)));

  btnMain3 = new QPushButton(tr("&Update"), this);
  btnMain4 = new QPushButton(tr("&Close"), this);
  connect(btnMain4, SIGNAL(clicked()), this, SLOT(close()));

  if (m_bOwner)
  {
    btnMain1 = new QPushButton(tr("&Save"), this);
    btnMain2 = new QPushButton(tr("Retrieve"), this);
    connect(btnMain1, SIGNAL(clicked()), this, SLOT(SaveSettings()));
    connect(btnMain2, SIGNAL(clicked()), this, SLOT(slotRetrieve()));
    connect(btnMain3, SIGNAL(clicked()), this, SLOT(slotUpdate()));
  }
  else
  {
    btnMain1 = new QPushButton(tr("&Menu"), this);
    btnMain2 = new QPushButton(tr("&Save"), this);
    connect(btnMain1, SIGNAL(pressed()), this, SLOT(ShowUsermenu()));
    btnMain1->setPopup(gMainWindow->UserMenu());
    connect(btnMain2, SIGNAL(clicked()), this, SLOT(SaveSettings()));
    connect(btnMain3, SIGNAL(clicked()), this, SLOT(slotRetrieve()));
  }

  int bw = 80;
  bw = QMAX(bw, btnMain1->sizeHint().width());
  bw = QMAX(bw, btnMain2->sizeHint().width());
  bw = QMAX(bw, btnMain3->sizeHint().width());
  bw = QMAX(bw, btnMain4->sizeHint().width());
  btnMain1->setFixedWidth(bw);
  btnMain2->setFixedWidth(bw);
  btnMain3->setFixedWidth(bw);
  btnMain4->setFixedWidth(bw);

  QBoxLayout* l = new QHBoxLayout(lay);

  l->addWidget(btnMain1);
  l->addStretch(2);
  l->addWidget(btnMain2);
  l->addWidget(btnMain3);
  l->addSpacing(35);
  l->addWidget(btnMain4);
  btnMain4->setDefault(true);

  // prepare the timer for the history filter
  timer = new QTimer(this, "history_filterTimer");
  connect(timer, SIGNAL(timeout()), this, SLOT(ShowHistory()));

  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
  if (u == NULL)
  {
    m_sBasic = tr("Licq - Info ") + tr("INVALID USER");
    resetCaption();
    setIconText(tr("INVALID USER"));
  }
  else
  {
    QTextCodec * codec = UserCodec::codecForICQUser(u);
    m_sBasic = tr("Licq - Info ") + codec->toUnicode(u->GetAlias()) + " (" + codec->toUnicode(u->GetFirstName()) + " " + codec->toUnicode(u->GetLastName()) + ")";
    resetCaption();
    setIconText(codec->toUnicode(u->GetAlias()));
    gUserManager.DropUser(u);
  }

  // Set Tab Order
  setTabOrder (tabs, btnMain1);
  setTabOrder (btnMain1, btnMain2);
  setTabOrder (btnMain2, btnMain3);
  setTabOrder (btnMain3, btnMain4);
}

// -----------------------------------------------------------------------------
UserInfoDlg::~UserInfoDlg()
{
  if (icqEventTag != 0)
  {
    server->CancelEvent(icqEventTag);
    icqEventTag = 0;
  }

  emit finished(m_szId, m_nPPID);

  ICQUser::ClearHistory(m_lHistoryList);
}


// -----------------------------------------------------------------------------

void UserInfoDlg::keyPressEvent(QKeyEvent* e)
{
  if ( e->state() == 0 )
  {
    switch ( e->key() ) {
    case Key_Enter:
    case Key_Return:
    case Key_Escape:
      e->accept();
      close();
      break;
    default:
      e->ignore();
      return;
    }
  }
  else
    e->ignore();
}


// -----------------------------------------------------------------------------
void UserInfoDlg::showTab(int tab)
{
  tabs->showPage(tabList[tab].tab);
}

bool UserInfoDlg::isTabShown(int tab)
{
  return (tabs->currentPage() == tabList[tab].tab);
}


// -----------------------------------------------------------------------------

void UserInfoDlg::CreateGeneralInfo()
{
  tabList[GeneralInfo].label = tr("&General");
  tabList[GeneralInfo].tab = new QWidget(this, tabList[GeneralInfo].label.latin1());
  tabList[GeneralInfo].loaded = false;

  unsigned short CR = 0;
  QWidget *p = tabList[GeneralInfo].tab;

  QGridLayout *lay = new QGridLayout(p, 11, 5, 10, 5);
  lay->addColSpacing(2, 10);
  lay->setRowStretch(9, 1);

  lay->addWidget(new QLabel(tr("Alias:"), p), CR, 0);
  nfoAlias = new CInfoField(p, false);
  lay->addWidget(nfoAlias, CR, 1);
  chkKeepAliasOnUpdate = new QCheckBox(tr("Keep Alias on Update"), p);
  QWhatsThis::add(chkKeepAliasOnUpdate, tr("Normally Licq overwrites the Alias when updating user details.\n"
                                           "Check this if you want to keep your changes to the Alias."));
  lay->addMultiCellWidget(chkKeepAliasOnUpdate, CR, CR, 3,4);

  lay->addWidget(new QLabel(tr("ID:"), p), ++CR, 0);
  nfoUin = new CInfoField(p, true);
  lay->addWidget(nfoUin, CR, 1);
  lay->addWidget(new QLabel(tr("IP:"), p), CR, 3);
  nfoIp = new CInfoField(p, true);
  lay->addWidget(nfoIp, CR, 4);

  lay->addWidget(new QLabel(tr("Status:"), p), ++CR, 0);
  nfoStatus = new CInfoField(p, true);
  lay->addWidget(nfoStatus, CR, 1);
  lay->addWidget(new QLabel(tr("Timezone:"), p), CR, 3);
  nfoTime = new CInfoField(p, true);
  lay->addWidget(nfoTime, CR, 4);

  lay->addWidget(new QLabel(tr("Name:"), p), ++CR, 0);
  nfoFirstName = new CInfoField(p, false);
  lay->addWidget(nfoFirstName, CR, 1);
  nfoLastName = new CInfoField(p, false);
  lay->addMultiCellWidget(nfoLastName, CR, CR, 2, 4);

  lay->addWidget(new QLabel(tr("EMail 1:"), p), ++CR, 0);
  nfoEmailPrimary = new CInfoField(p, false);
  lay->addMultiCellWidget(nfoEmailPrimary, CR, CR, 1, 4);

  lay->addWidget(new QLabel(tr("EMail 2:"), p), ++CR, 0);
  nfoEmailSecondary = new CInfoField(p, false);
  lay->addMultiCellWidget(nfoEmailSecondary, CR, CR, 1, 4);

  lay->addWidget(new QLabel(tr("Old Email:"), p), ++CR, 0);
  nfoEmailOld = new CInfoField(p, false);
  lay->addMultiCellWidget(nfoEmailOld, CR, CR, 1, 4);

  lay->addWidget(new QLabel(tr("Address:"), p), ++CR, 0);
  nfoAddress = new CInfoField(p, !m_bOwner);
  lay->addWidget(nfoAddress, CR, 1);
  lay->addWidget(new QLabel(tr("Phone:"), p), CR, 3);
  nfoPhone = new CInfoField(p, false);//!m_bOwner);
  lay->addWidget(nfoPhone, CR, 4);

  lay->addWidget(new QLabel(tr("State:"), p), ++CR, 0);
  nfoState = new CInfoField(p, !m_bOwner);
  nfoState->setMaxLength(3);
  setTabOrder(nfoAddress, nfoState);
  lay->addWidget(nfoState, CR, 1);
  lay->addWidget(new QLabel(tr("Fax:"), p), CR, 3);
  nfoFax = new CInfoField(p, false);//!m_bOwner);
  setTabOrder(nfoPhone, nfoFax);
  lay->addWidget(nfoFax, CR, 4);

  lay->addWidget(new QLabel(tr("City:"), p), ++CR, 0);
  nfoCity = new CInfoField(p, !m_bOwner);
  setTabOrder(nfoState, nfoCity);
  lay->addWidget(nfoCity, CR, 1);
  lay->addWidget(new QLabel(tr("Cellular:"), p), CR, 3);
  nfoCellular = new CInfoField(p, false);//!m_bOwner);
  setTabOrder(nfoFax, nfoCellular);
  lay->addWidget(nfoCellular, CR, 4);

  lay->addWidget(new QLabel(tr("Zip:"), p), ++CR, 0);
  nfoZipCode = new CInfoField(p, !m_bOwner);
  setTabOrder(nfoCity, nfoZipCode);
  lay->addWidget(nfoZipCode, CR, 1);
  lay->addWidget(new QLabel(tr("Country:"), p), CR, 3);
  if (m_bOwner)
  {
    cmbCountry = new CEComboBox(true, tabList[GeneralInfo].tab);
    //cmbCountry->insertItem(tr("Unspecified"));
    cmbCountry->setMaximumWidth(cmbCountry->sizeHint().width()+20);
    for (unsigned short i = 0; i < NUM_COUNTRIES; i++)
      cmbCountry->insertItem(GetCountryByIndex(i)->szName);
    lay->addWidget(cmbCountry, CR, 4);
  }
  else
  {
    nfoCountry = new CInfoField(p, !m_bOwner);
    lay->addWidget(nfoCountry, CR, 4);
  }

  lay->setRowStretch(++CR, 5);
}

void UserInfoDlg::SetGeneralInfo(ICQUser *u)
{
  tabList[GeneralInfo].loaded = true;
  char buf[32];
  bool bDropUser = false;

  if (u == NULL)
  {
    u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
    if (u == NULL) return;
    bDropUser = true;
  }

  QTextCodec * codec = UserCodec::codecForICQUser(u);

  if(m_bOwner)
    chkKeepAliasOnUpdate->hide();
  chkKeepAliasOnUpdate->setChecked(u->KeepAliasOnUpdate());
  nfoAlias->setData(codec->toUnicode(u->GetAlias()));
  connect(nfoAlias, SIGNAL(textChanged(const QString &)), this, SLOT(slot_aliasChanged(const QString &)));
  nfoFirstName->setData(codec->toUnicode(u->GetFirstName()));
  nfoLastName->setData(codec->toUnicode(u->GetLastName()));
  nfoEmailPrimary->setData(codec->toUnicode(u->GetEmailPrimary()));
  nfoEmailSecondary->setData(codec->toUnicode(u->GetEmailSecondary()));
  nfoEmailOld->setData(codec->toUnicode(u->GetEmailOld()));
  nfoUin->setData(u->IdString());
  QString ip = QString(u->IpStr(buf));
  if (u->Ip() != u->IntIp() && u->IntIp() != 0)
  {
    ip.append(QString(" / %1").arg(u->IntIpStr(buf)));
  }
  if (u->Port() != 0)
  {
    ip.append(QString(":%1").arg(u->PortStr(buf)));
  }
  nfoIp->setData(ip);
  if (u->GetTimezone() == TIMEZONE_UNKNOWN)
    nfoTime->setText(tr("Unknown"));
  else
  {
    nfoTime->setText(tr("GMT%1%1%1")
       .arg(u->GetTimezone() > 0 ? "-" : "+")
       .arg(abs(u->GetTimezone() / 2)).arg(u->GetTimezone() % 2 ? "30" : "00") );
  }
  nfoStatus->setData(u->StatusStr());
  if (m_bOwner)
  {
    const SCountry *c = GetCountryByCode(u->GetCountryCode());
    if (c == NULL)
      cmbCountry->setCurrentItem(0);
    else
      cmbCountry->setCurrentItem(c->nIndex);
  }
  else
  {
    const SCountry *c = GetCountryByCode(u->GetCountryCode());
    if (c == NULL)
      nfoCountry->setData(tr("Unknown (%1)").arg(u->GetCountryCode()));
    else  // known
      nfoCountry->setData(c->szName);
  }
  nfoAddress->setData(codec->toUnicode(u->GetAddress()));
  nfoCity->setData(codec->toUnicode(u->GetCity()));
  nfoState->setData(codec->toUnicode(u->GetState()));
  nfoPhone->setData(codec->toUnicode(u->GetPhoneNumber()));
  nfoFax->setData(codec->toUnicode(u->GetFaxNumber()));
  nfoCellular->setData(codec->toUnicode(u->GetCellularNumber()));
  nfoZipCode->setData(codec->toUnicode(u->GetZipCode()));

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
  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_W);
  if (u == NULL) return;

  QTextCodec * codec = UserCodec::codecForICQUser(u);

  u->SetEnableSave(false);

  u->SetAlias(codec->fromUnicode(nfoAlias->text()));
  u->SetKeepAliasOnUpdate(chkKeepAliasOnUpdate->isChecked());
  u->SetFirstName(codec->fromUnicode(nfoFirstName->text()));
  u->SetLastName(codec->fromUnicode(nfoLastName->text()));
  u->SetEmailPrimary(codec->fromUnicode(nfoEmailPrimary->text()));
  u->SetEmailSecondary(codec->fromUnicode(nfoEmailSecondary->text()));
  u->SetEmailOld(codec->fromUnicode(nfoEmailOld->text()));
  u->SetCity(codec->fromUnicode(nfoCity->text()));
  u->SetState(codec->fromUnicode(nfoState->text()));
  u->SetAddress(codec->fromUnicode(nfoAddress->text()));
  u->SetPhoneNumber(codec->fromUnicode(nfoPhone->text()));
  u->SetFaxNumber(codec->fromUnicode(nfoFax->text()));
  u->SetCellularNumber(codec->fromUnicode(nfoCellular->text()));
  u->SetZipCode(codec->fromUnicode(nfoZipCode->text()));
  if (m_bOwner)
  {
    unsigned short i = cmbCountry->currentItem();
    u->SetCountryCode(GetCountryByIndex(i)->nCode);
  }

  u->SetEnableSave(true);
  u->SaveGeneralInfo();

  gUserManager.DropUser(u);

  if (!m_bOwner)
    server->icqRenameUser(m_szId, m_nPPID);
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
    lay->addWidget(new QLabel(tr("Language 2:"), p), ++CR, 0);
    cmbLanguage[1] = new CEComboBox(true, p);
    lay->addWidget(cmbLanguage[1], CR, 1);

    lay->addWidget(new QLabel(tr("Language 3:"), p), ++CR, 0);
    cmbLanguage[2] = new CEComboBox(true, p);
    lay->addWidget(cmbLanguage[2], CR, 1);

    for (unsigned short i = 0; i < 3; i++)
    {
      for (unsigned short j = 0; j < NUM_LANGUAGES; j++)
        if (GetLanguageByIndex(j))
          cmbLanguage[i]->insertItem(GetLanguageByIndex(j)->szName);
    }
  }
  else
  {
    lay->addWidget(new QLabel(tr("Language 1:"), p), ++CR, 0);
    nfoLanguage[0] = new CInfoField(p, !m_bOwner);
    lay->addWidget(nfoLanguage[0], CR, 1);
    lay->addWidget(new QLabel(tr("Language 2:"), p), ++CR, 0);
    nfoLanguage[1] = new CInfoField(p, !m_bOwner);
    lay->addWidget(nfoLanguage[1], CR, 1);

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
    u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
    if (u == NULL) return;
    bDropUser = true;
  }

  QTextCodec * codec = UserCodec::codecForICQUser(u);

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
    spnBirthYear->setValue(u->GetBirthYear());
  }
  else
  {
    if (u->GetBirthMonth() == 0 || u->GetBirthDay() == 0)
    {
      nfoBirthday->setData(tr("Unspecified"));
    }
    else
    {
      QDate d(u->GetBirthYear(), u->GetBirthMonth(), u->GetBirthDay());
      nfoBirthday->setData(d.toString());
    }
  }
  nfoHomepage->setData(codec->toUnicode(u->GetHomepage()));

  for (unsigned short i = 0; i < 3; i++)
  {
    const SLanguage *l = GetLanguageByCode(u->GetLanguage(i));
    if (m_bOwner)
    {
      if (l == NULL)
        cmbLanguage[i]->setCurrentItem(0);
      else
        cmbLanguage[i]->setCurrentItem(l->nIndex);
    }
    else
    {
      if (l == NULL)
        nfoLanguage[i]->setData(tr("Unknown (%1)").arg((unsigned short)u->GetLanguage(i)));
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
  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_W);
  if (u == NULL) return;
  u->SetEnableSave(false);

  u->SetAge(nfoAge->text().toULong());
  u->SetHomepage(nfoHomepage->text().local8Bit().data());
  if (m_bOwner)
  {
    u->SetGender(cmbGender->currentItem());
    u->SetBirthYear(spnBirthYear->value());
    u->SetBirthMonth(spnBirthMonth->value());
    u->SetBirthDay(spnBirthDay->value());
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

  QGridLayout *lay = new QGridLayout(p, 9, 5, 10, 5);
  lay->addColSpacing(2, 10);
  lay->setRowStretch(8, 1);

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
  nfoCompanyState->setMaxLength(5);
  lay->addWidget(nfoCompanyState, CR, 4);

  lay->addWidget(new QLabel(tr("Address:"), p), ++CR, 0);
  nfoCompanyAddress = new CInfoField(p, !m_bOwner);
  lay->addMultiCellWidget(nfoCompanyAddress, CR, CR, 1, 4);

  lay->addWidget(new QLabel(tr("Zip:"), p), ++CR, 0);
  nfoCompanyZip = new CInfoField(p, !m_bOwner);
  lay->addWidget(nfoCompanyZip, CR, 1);
  lay->addWidget(new QLabel(tr("Country:"), p), CR, 3);
  if (m_bOwner)
  {
    cmbCompanyCountry = new CEComboBox(true, tabList[WorkInfo].tab);
    cmbCompanyCountry->setMaximumWidth(cmbCompanyCountry->sizeHint().width()+20);
    for (unsigned short i = 0; i < NUM_COUNTRIES; i++)
      cmbCompanyCountry->insertItem(GetCountryByIndex(i)->szName);
    lay->addWidget(cmbCompanyCountry, CR, 4);
  }
  else
  {
    nfoCompanyCountry = new CInfoField(p, !m_bOwner);
    lay->addWidget(nfoCompanyCountry, CR, 4);
  }

  lay->addWidget(new QLabel(tr("Phone:"), p), ++CR, 0);
  nfoCompanyPhone = new CInfoField(p, !m_bOwner);
  lay->addWidget(nfoCompanyPhone, CR, 1);
  lay->addWidget(new QLabel(tr("Fax:"), p), CR, 3);
  nfoCompanyFax = new CInfoField(p, !m_bOwner);
  lay->addWidget(nfoCompanyFax, CR, 4);

  lay->addWidget(new QLabel(tr("Homepage:"), p), ++CR, 0);
  nfoCompanyHomepage = new CInfoField(p, !m_bOwner);
  lay->addMultiCellWidget(nfoCompanyHomepage, CR, CR, 1, 4);
}

void UserInfoDlg::SetWorkInfo(ICQUser *u)
{
  tabList[WorkInfo].loaded = true;
  bool bDropUser = false;

  if (u == NULL)
  {
    u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
    if (u == NULL) return;
    bDropUser = true;
  }

  QTextCodec * codec = UserCodec::codecForICQUser(u);

  nfoCompanyName->setData(codec->toUnicode(u->GetCompanyName()));
  nfoCompanyDepartment->setData(codec->toUnicode(u->GetCompanyDepartment()));
  nfoCompanyPosition->setData(codec->toUnicode(u->GetCompanyPosition()));
  nfoCompanyCity->setData(codec->toUnicode(u->GetCompanyCity()));
  nfoCompanyState->setData(codec->toUnicode(u->GetCompanyState()));
  nfoCompanyAddress->setData(codec->toUnicode(u->GetCompanyAddress()));
  nfoCompanyZip->setData(codec->toUnicode(u->GetCompanyZip()));
  if (m_bOwner)
  {
    const SCountry *c = GetCountryByCode(u->GetCompanyCountry());
    if (c == NULL)
      cmbCompanyCountry->setCurrentItem(0);
    else
      cmbCompanyCountry->setCurrentItem(c->nIndex);
  }
  else
  {
    const SCountry *c = GetCountryByCode(u->GetCompanyCountry());
    if (c == NULL)
      nfoCompanyCountry->setData(tr("Unknown (%1)").arg(u->GetCompanyCountry()));
    else  // known
      nfoCompanyCountry->setData(c->szName);
  }
  nfoCompanyPhone->setData(codec->toUnicode(u->GetCompanyPhoneNumber()));
  nfoCompanyFax->setData(codec->toUnicode(u->GetCompanyFaxNumber()));
  nfoCompanyHomepage->setData(codec->toUnicode(u->GetCompanyHomepage()));

  if (bDropUser) gUserManager.DropUser(u);
}

void UserInfoDlg::SaveWorkInfo()
{
  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_W);
  if (u == NULL) return;

  QTextCodec * codec = UserCodec::codecForICQUser(u);

  u->SetEnableSave(false);

  u->SetCompanyCity(codec->fromUnicode(nfoCompanyCity->text()));
  u->SetCompanyState(codec->fromUnicode(nfoCompanyState->text()));
  u->SetCompanyPhoneNumber(codec->fromUnicode(nfoCompanyPhone->text()));
  u->SetCompanyFaxNumber(codec->fromUnicode(nfoCompanyFax->text()));
  u->SetCompanyAddress(codec->fromUnicode(nfoCompanyAddress->text()));
  u->SetCompanyZip(codec->fromUnicode(nfoCompanyZip->text()));
  if (m_bOwner)
  {
    unsigned short i = cmbCompanyCountry->currentItem();
    u->SetCompanyCountry(GetCountryByIndex(i)->nCode);
  }
  u->SetCompanyName(codec->fromUnicode(nfoCompanyName->text()));
  u->SetCompanyDepartment(codec->fromUnicode(nfoCompanyDepartment->text()));
  u->SetCompanyPosition(codec->fromUnicode(nfoCompanyPosition->text()));
  u->SetCompanyHomepage(codec->fromUnicode(nfoCompanyHomepage->text()));

  u->SetEnableSave(true);
  u->SaveWorkInfo();

  gUserManager.DropUser(u);
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
#if QT_VERSION < 300
  // Windows ICQ seems to limit the about info to 450 chars
  // so we do the same
  mleAbout->setMaxLength(450);
#endif
}

void UserInfoDlg::SetAbout(ICQUser *u)
{
  tabList[AboutInfo].loaded = true;
  bool bDropUser = false;

  if (u == NULL)
  {
    u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
    if (u == NULL) return;
    bDropUser = true;
  }

  QTextCodec * codec = UserCodec::codecForICQUser(u);

  QString aboutstr = codec->toUnicode(u->GetAbout());
  aboutstr.replace(QRegExp("\r"), "");
  mleAbout->setText(aboutstr);

  if (bDropUser) gUserManager.DropUser(u);
}

void UserInfoDlg::SaveAbout()
{
  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_W);
  if (u == NULL) return;

  QTextCodec * codec = UserCodec::codecForICQUser(u);
  QString str = mleAbout->text();

  u->SetAbout(codec->fromUnicode(str.left(450)));
  gUserManager.DropUser(u);
}

//-----LastCounters--------------------------------------------------------
void UserInfoDlg::CreateLastCountersInfo()
{
  tabList[LastCountersInfo].label = tr("&Last");
  tabList[LastCountersInfo].tab = new QWidget(this, tabList[LastCountersInfo].label.latin1());
  tabList[LastCountersInfo].loaded = false;

  unsigned short CR = 0;
  QWidget *p = tabList[LastCountersInfo].tab;

  QGridLayout *lay = new QGridLayout(p, 6, 2, 10, 5);
  //lay->setRowStretch(9, 1);

  lay->addWidget(new QLabel(tr("Last Online:"), p), CR, 0);
  nfoLastOnline = new CInfoField(p, true);
  lay->addWidget(nfoLastOnline, CR, 1);

  lay->addWidget(new QLabel(tr("Last Sent Event:"), p), ++CR, 0);
  nfoLastSent = new CInfoField(p, true);
  lay->addWidget(nfoLastSent, CR, 1);

  lay->addWidget(new QLabel(tr("Last Received Event:"), p), ++CR, 0);
  nfoLastRecv = new CInfoField(p, true);
  lay->addWidget(nfoLastRecv, CR, 1);

  lay->addWidget(new QLabel(tr("Last Checked Auto Response:"), p), ++CR, 0);
  nfoLastCheckedAR = new CInfoField(p, true);
  lay->addWidget(nfoLastCheckedAR, CR, 1);

  lay->addWidget(new QLabel(tr("Online Since:"), p), ++CR, 0);
  nfoOnlineSince = new CInfoField(p, true);
  lay->addWidget(nfoOnlineSince, CR, 1);

  lay->setRowStretch(++CR, 5);
}

void UserInfoDlg::SetLastCountersInfo(ICQUser *u)
{
  tabList[LastCountersInfo].loaded = true;
  bool bDropUser = false;

  if (u == NULL)
  {
    u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
    if (u == NULL) return;
    bDropUser = true;
  }

  QDateTime t;
  QString ds;

  if (!u->StatusOffline())
    nfoLastOnline->setData(tr("Now"));
  else if (u->LastOnline() == 0)
    nfoLastOnline->setData(tr("Unknown"));
  else
  {
    t.setTime_t(u->LastOnline());
    ds = t.toString();
    ds.truncate(ds.length() - 8);
    nfoLastOnline->setData(ds);
  }

  if (u->LastSentEvent() == 0)
    nfoLastSent->setData(tr("Unknown"));
  else
  {
    t.setTime_t(u->LastSentEvent());
    ds = t.toString();
    ds.truncate(ds.length() - 8);
    nfoLastSent->setData(ds);
  }

  if (u->LastReceivedEvent() == 0)
    nfoLastRecv->setData(tr("Unknown"));
  else
  {
    t.setTime_t(u->LastReceivedEvent());
    ds = t.toString();
    ds.truncate(ds.length() - 8);
    nfoLastRecv->setData(ds);
  }

  if (u->LastCheckedAutoResponse() == 0)
    nfoLastCheckedAR->setData(tr("Unknown"));
  else
  {
    t.setTime_t(u->LastCheckedAutoResponse());
    ds = t.toString();
    ds.truncate(ds.length() - 8);
    nfoLastCheckedAR->setData(ds);
  }

  if (u->StatusOffline())
    nfoOnlineSince->setData(tr("Offline"));
  else if (u->OnlineSince() == 0)
    nfoOnlineSince->setData(tr("Unknown"));
  else
  {
    t.setTime_t(u->OnlineSince());
    ds = t.toString();
    ds.truncate(ds.length() - 8);
    nfoOnlineSince->setData(ds);
  }

  if (bDropUser) gUserManager.DropUser(u);
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
  chkHistoryReverse->setChecked(m_bHistoryReverse = true);
  chkHistoryReverse->setFixedSize(chkHistoryReverse->sizeHint());
  l->addWidget(chkHistoryReverse);

  mlvHistory = new CHistoryWidget(p, "history");
#if QT_VERSION >= 300
  connect(mlvHistory, SIGNAL(viewurl(QWidget*, QString)), mainwin, SLOT(slot_viewurl(QWidget *, QString)));
#endif
  lay->addWidget(mlvHistory, 1);

  l = new QHBoxLayout(lay);

  lneFilter = new QLineEdit(p);
  lblFilter = new QLabel(lneFilter, tr("&Filter: "), p);
  l->addWidget(lblFilter);
  l->addWidget(lneFilter, 1);
  l->addSpacing(50);
  connect(lneFilter, SIGNAL(textChanged(const QString&)), this, SLOT(slot_showHistoryTimer()));
  barFiltering = new QProgressBar(p);
  l->addWidget(barFiltering, 1);

  QAccel *a = new QAccel(p);
  a->connectItem(a->insertItem(Key_U + CTRL), this, SLOT(HistoryReload()));
  a->connectItem(a->insertItem(Key_F5), this, SLOT(HistoryReload()));
}

/*!  \brief Resets and restarts the timeout for the history filter
 *
 *   Everytime this slot is called, the timer for the history filter
 *   is reset.
 *   The timeout is set to 1000 ms.
 *   This slot is called everytime the user changes the filter
 *   string in the history tab.  After 1 second of being idle, the
 *   filter is invoked because SIGNAL(timeout()) is connected to
 *   ShowHistory().
 */
void UserInfoDlg::slot_showHistoryTimer()
{
  timer->stop();
  timer->start(1000, true);
}

void UserInfoDlg::SetupHistory()
{
  tabList[HistoryInfo].loaded = true;

  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
  if (u == NULL) return;

  if (!u->GetHistory(m_lHistoryList))
  {
    if(u->HistoryFile())
      mlvHistory->setText(tr("Error loading history file: %1\nDescription: %2")
        .arg(u->HistoryFile()).arg(u->HistoryName()));
    else
      mlvHistory->setText(tr("Sorry, history is disabled for this person."));

    btnMain2->setEnabled(false);
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

    btnMain2->setEnabled((m_nHistoryIndex - m_nHistoryShowing) != 0);
  }
  gUserManager.DropUser(u);

  btnMain3->setEnabled(false);
}

// -----------------------------------------------------------------------------


void UserInfoDlg::HistoryReload()
{
  ICQUser::ClearHistory(m_lHistoryList);
  SetupHistory();
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
    btnMain2->setEnabled(m_iHistorySIter != m_lHistoryList.begin());
    btnMain3->setEnabled(true);
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
    btnMain3->setEnabled(m_iHistoryEIter != m_lHistoryList.end());
    btnMain2->setEnabled(true);
  }
}

bool UserInfoDlg::chkContains(const char* d, const char* filter, int len)
{
  if ( !d ) return false;
  while ( *d ) {
    if ( strncasecmp(d, filter, len) == 0 )
      return true;
    d++;
  }
  return false;
}

void UserInfoDlg::ShowHistory()
{
  m_nHistoryShowing = 0;

  // Last check (will be true if history is empty)
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
  QString s;
  QString tmp;
  QDateTime date;
  QString contactName = tr("server");
  QTextCodec * codec = QTextCodec::codecForLocale();
  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
  bool bUseHTML = false;
  if (u != NULL)
  {
      codec = UserCodec::codecForICQUser(u);
      if (!m_bOwner)
         contactName = codec->toUnicode(u->GetAlias());
      for (unsigned int x = 0; x < strlen(m_szId); x++)
      {
        if (!isdigit(m_szId[x]))
        {
          bUseHTML = true;
          break;
        }
      }
      gUserManager.DropUser(u);
  }
  barFiltering->setTotalSteps(NUM_MSG_PER_HISTORY);
  char* ftxt = qstrdup(codec->fromUnicode(lneFilter->text()));
  int flen = strlen(ftxt);
  while (m_nHistoryShowing < (NUM_MSG_PER_HISTORY))
  {
    if(UserInfoDlg::chkContains((*tempIter)->Text(), ftxt, flen))
    {
      date.setTime_t((*tempIter)->Time());
      QString messageText;
      if ((*tempIter)->SubCommand() == ICQ_CMDxSUB_SMS) // SMSs are always in UTF-8
        messageText = QString::fromUtf8((*tempIter)->Text());
      else
        messageText = codec->toUnicode((*tempIter)->Text());

#if QT_VERSION >= 300
      const char *color = (*tempIter)->Direction() == D_RECEIVER ? "red" : "blue";
      s.sprintf("<font color=\"%s\"><b>%s<br>%s [%c%c%c%c]</b></font><br><br>",
                color,
                ((*tempIter)->Direction() == D_RECEIVER ? tr("%1 from %2") : tr("%1 to %2"))
                  .arg(EventDescription(*tempIter)).arg(QStyleSheet::escape(contactName)).utf8().data(),
                date.toString().utf8().data(),
                (*tempIter)->IsDirect() ? 'D' : '-',
                (*tempIter)->IsMultiRec() ? 'M' : '-',
                (*tempIter)->IsUrgent() ? 'U' : '-',
                (*tempIter)->IsEncrypted() ? 'E' : '-'
               );
      tmp.append(s);
      // We break the paragraph here, since the history text
      // could be in a different BiDi directionality than the
      // header and timestamp text.
      s.sprintf("<font color=\"%s\">%s</font><br><br>",
                color,
                MLView::toRichText(messageText, true, bUseHTML).utf8().data()
               );
      tmp.append(s);
#else
      // See CHistoryWidget::paintCell for reference on those Qt 2-only
      // formatting escape codes.
      s.sprintf("%c%s\n%c%s [%c%c%c%c]\n\n%s\n\n",
                ((*tempIter)->Direction() == D_RECEIVER ? '\001' : '\002'),
                ((*tempIter)->Direction() == D_RECEIVER ? tr("%1 from %2") : tr("%1 to %2"))
                  .arg(EventDescription(*tempIter)).arg(contactName).utf8().data(),
                ((*tempIter)->Direction() == D_RECEIVER ? '\001' : '\002'),
                date.toString().utf8().data(),
                (*tempIter)->IsDirect() ? 'D' : '-',
                (*tempIter)->IsMultiRec() ? 'M' : '-',
                (*tempIter)->IsUrgent() ? 'U' : '-',
                (*tempIter)->IsEncrypted() ? 'E' : '-',
                messageText.utf8().data()
      );
      tmp.append(s);
#endif
      m_nHistoryShowing++;
      barFiltering->setProgress(m_nHistoryShowing);
    }
    if(m_bHistoryReverse)
    {
      if (tempIter == (lneFilter->text().isEmpty() ?
                       m_iHistorySIter : m_lHistoryList.begin()))
        break;
      tempIter--;
    }
    else
    {
       tempIter++;
       if (tempIter == (lneFilter->text().isEmpty() ?
                        m_iHistoryEIter : m_lHistoryList.end()))
         break;
    }
  }
  delete [] ftxt;
  if(lneFilter->text().isEmpty())
    lblHistory->setText(tr("[<font color=\"%1\">Received</font>] "
                           "[<font color=\"%2\">Sent</font>] "
                           "%3 to %4 of %5")
                        .arg(COLOR_RECEIVED).arg(COLOR_SENT)
                        .arg(m_nHistoryIndex - m_nHistoryShowing + 1)
                        .arg(m_nHistoryIndex)
                        .arg(m_lHistoryList.size()));
  else
    lblHistory->setText(tr("[<font color=\"%1\">Received</font>] "
                           "[<font color=\"%2\">Sent</font>] "
                           "%3 out of %4 matches")
                        .arg(COLOR_RECEIVED).arg(COLOR_SENT)
                        .arg(m_nHistoryShowing)
                        .arg(m_lHistoryList.size()));
  mlvHistory->clear();
  mlvHistory->append(tmp.left(tmp.length()-4));
  if(!m_bHistoryReverse)
    mlvHistory->GotoEnd();
  else
    mlvHistory->setCursorPosition(0, 0);
  barFiltering->reset();
}


void UserInfoDlg::SaveHistory()
{
  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
  if (u == NULL) return;

  QTextCodec * codec = UserCodec::codecForICQUser(u);

  u->SaveHistory(codec->fromUnicode(mlvHistory->text()));
  gUserManager.DropUser(u);
}


// -----------------------------------------------------------------------------

void UserInfoDlg::updateTab(const QString& txt)
{
  if (txt == tabList[GeneralInfo].label)
  {
    currentTab = GeneralInfo;
    btnMain1->setText(m_bOwner ? tr("&Save") : tr("&Menu"));
    btnMain2->setText(m_bOwner ? tr("&Retrieve") : tr("&Save"));
    btnMain3->setText(m_bOwner ? tr("S&end") : tr("&Update"));
    btnMain3->setEnabled(true);
    btnMain2->setEnabled(true);
    btnMain1->setEnabled(true);
    if (!tabList[GeneralInfo].loaded)
      SetGeneralInfo(NULL);
  }
  else if (txt == tabList[MoreInfo].label)
  {
    btnMain1->setText(m_bOwner ? tr("&Save") : tr("&Menu"));
    btnMain2->setText(m_bOwner ? tr("&Retrieve") : tr("&Save"));
    btnMain3->setText(m_bOwner ? tr("S&end") : tr("&Update"));
    btnMain3->setEnabled(true);
    btnMain2->setEnabled(true);
    btnMain1->setEnabled(true);
    currentTab = MoreInfo;
    if (!tabList[MoreInfo].loaded)
      SetMoreInfo(NULL);
  }
  else if (txt == tabList[WorkInfo].label)
  {
    btnMain1->setText(m_bOwner ? tr("&Save") : tr("&Menu"));
    btnMain2->setText(m_bOwner ? tr("&Retrieve") : tr("&Save"));
    btnMain3->setText(m_bOwner ? tr("S&end") : tr("&Update"));
    btnMain3->setEnabled(true);
    btnMain2->setEnabled(true);
    btnMain1->setEnabled(true);
    currentTab = WorkInfo;
    if (!tabList[WorkInfo].loaded)
      SetWorkInfo(NULL);
  }
  else if (txt == tabList[AboutInfo].label)
  {
    btnMain1->setText(m_bOwner ? tr("&Save") : tr("&Menu"));
    btnMain2->setText(m_bOwner ? tr("&Retrieve") : tr("&Save"));
    btnMain3->setText(m_bOwner ? tr("S&end") : tr("&Update"));
    btnMain3->setEnabled(true);
    btnMain2->setEnabled(true);
    btnMain1->setEnabled(true);
    currentTab = AboutInfo;
    if (!tabList[AboutInfo].loaded)
      SetAbout(NULL);
  }
  else if (txt == tabList[HistoryInfo].label)
  {
    btnMain3->setText(tr("Nex&t"));
    btnMain2->setText(tr("P&rev"));
    btnMain1->setText(m_bOwner ? (QString)"" : tr("&Menu"));
    if (tabList[HistoryInfo].loaded)
    {
      btnMain3->setEnabled(m_iHistoryEIter != m_lHistoryList.end());
      btnMain2->setEnabled((m_nHistoryIndex - m_nHistoryShowing) != 0);
    }
    btnMain1->setEnabled(!m_bOwner);
    currentTab = HistoryInfo;
    if (!tabList[HistoryInfo].loaded)
      SetupHistory();
    mlvHistory->setFocus();
  }
  else if (txt == tabList[LastCountersInfo].label)
  {
    currentTab = LastCountersInfo;
    btnMain3->setText("");
    btnMain2->setText("");
    btnMain1->setText(m_bOwner ? tr("&Save") : tr("&Menu"));
    btnMain3->setEnabled(false);
    btnMain2->setEnabled(false);
    btnMain1->setEnabled(true);
    if (!tabList[LastCountersInfo].loaded)
      SetLastCountersInfo(NULL);
  }
}

void UserInfoDlg::SaveSettings()
{
  switch(currentTab) {
  case GeneralInfo:
  {
    SaveGeneralInfo();
    CICQSignal s(SIGNAL_UPDATExUSER, USER_GENERAL, m_szId, m_nPPID);
    gMainWindow->slot_updatedUser(&s);
    break;
  }
  case MoreInfo:
    SaveMoreInfo();
    break;
  case WorkInfo:
    SaveWorkInfo();
    break;
  case AboutInfo:
    SaveAbout();
    break;
  case HistoryInfo:
    if (!m_bOwner)
      ShowHistoryPrev();
    break;
  case LastCountersInfo:
    break;
  }
}


void UserInfoDlg::slotRetrieve()
{
  if (currentTab == LastCountersInfo) return;

  if (currentTab != HistoryInfo)
  {
    ICQOwner *o = gUserManager.FetchOwner(m_nPPID, LOCK_R);
    if(o == NULL)  return;
    unsigned short status = o->Status();
    gUserManager.DropOwner();

    if(status == ICQ_STATUS_OFFLINE) {
      InformUser(this, tr("You need to be connected to the\n"
                          "ICQ Network to retrieve your settings."));
      return;
    }
  }
  else
  {
    if (m_bOwner)
      ShowHistoryPrev();
    else
      ShowHistoryNext();
    return;
  }

  switch(currentTab)
  {
    //TODO change in the daemon
    case GeneralInfo:
      icqEventTag = server->icqRequestMetaInfo(strtoul(m_szId, (char **)NULL, 10));
      break;
    case MoreInfo:
      icqEventTag = server->icqRequestMetaInfo(strtoul(m_szId, (char **)NULL, 10));
      break;
    case WorkInfo:
      icqEventTag = server->icqRequestMetaInfo(strtoul(m_szId, (char **)NULL, 10));
      break;
    case AboutInfo:
      icqEventTag = server->icqRequestMetaInfo(strtoul(m_szId, (char **)NULL, 10));
      break;
  }

  if (icqEventTag != 0)
  {
    setCursor(waitCursor);
    m_sProgressMsg = tr("Updating...");
    connect (sigman, SIGNAL(signal_doneUserFcn(ICQEvent *)), this, SLOT(doneFunction(ICQEvent *)));
    setCaption(m_sBasic + " [" + m_sProgressMsg +"]");
  }
}


void UserInfoDlg::slotUpdate()
{
  if (currentTab == LastCountersInfo) return;

  QTextCodec * codec = QTextCodec::codecForLocale();

  if (currentTab != HistoryInfo)
  {
    ICQOwner *o = gUserManager.FetchOwner(m_nPPID, LOCK_R);
    if(o == NULL)  return;
    unsigned short status = o->Status();
    codec = UserCodec::codecForICQUser(o);
    gUserManager.DropOwner();

    if(status == ICQ_STATUS_OFFLINE) {
      InformUser(this, tr("You need to be connected to the\n"
                          "ICQ Network to change your settings."));
      return;
    }
  }

  unsigned short i, cc;

  switch(currentTab) {
  case GeneralInfo:
  {
    i = cmbCountry->currentItem();
    cc = GetCountryByIndex(i)->nCode;
    server->icqSetEmailInfo(codec->fromUnicode(nfoEmailSecondary->text()),
			    codec->fromUnicode(nfoEmailOld->text()));
    //TODO in the daemon. along with the rest of the icqSet... family
    icqEventTag = server->icqSetGeneralInfo(codec->fromUnicode(nfoAlias->text()),
                                            codec->fromUnicode(nfoFirstName->text()),
                                            codec->fromUnicode(nfoLastName->text()),
                                            codec->fromUnicode(nfoEmailPrimary->text()),
                                            codec->fromUnicode(nfoCity->text()),
                                            codec->fromUnicode(nfoState->text()),
                                            codec->fromUnicode(nfoPhone->text()),
                                            codec->fromUnicode(nfoFax->text()),
                                            codec->fromUnicode(nfoAddress->text()),
                                            codec->fromUnicode(nfoCellular->text()),
                                            codec->fromUnicode(nfoZipCode->text()),
                                            cc, false);
  }
  break;
  case MoreInfo:
    icqEventTag = server->icqSetMoreInfo(nfoAge->text().toUShort(),
                                         cmbGender->currentItem(),
                                         nfoHomepage->text().local8Bit(),
                                         spnBirthYear->value(),
                                         spnBirthMonth->value(),
                                         spnBirthDay->value(),
                                         GetLanguageByIndex(cmbLanguage[0]->currentItem())->nCode,
                                         GetLanguageByIndex(cmbLanguage[1]->currentItem())->nCode,
                                         GetLanguageByIndex(cmbLanguage[2]->currentItem())->nCode);
  break;
  case WorkInfo:
    i = cmbCompanyCountry->currentItem();
    cc = GetCountryByIndex(i)->nCode;
    icqEventTag = server->icqSetWorkInfo(codec->fromUnicode(nfoCompanyCity->text()),
                                         codec->fromUnicode(nfoCompanyState->text()),
                                         codec->fromUnicode(nfoCompanyPhone->text()),
                                         codec->fromUnicode(nfoCompanyFax->text()),
                                         codec->fromUnicode(nfoCompanyAddress->text()),
                                         codec->fromUnicode(nfoCompanyZip->text()),
					 cc,
                                         codec->fromUnicode(nfoCompanyName->text()),
                                         codec->fromUnicode(nfoCompanyDepartment->text()),
                                         codec->fromUnicode(nfoCompanyPosition->text()),
                                         nfoCompanyHomepage->text().local8Bit());
  break;
  case AboutInfo:    icqEventTag = server->icqSetAbout(codec->fromUnicode(mleAbout->text()));  break;
  case HistoryInfo:  ShowHistoryNext();  break;
  }

  if (icqEventTag != 0)
  {
    m_sProgressMsg = tr("Updating server...");
    setCursor(waitCursor);
    connect (sigman, SIGNAL(signal_doneUserFcn(ICQEvent *)), this, SLOT(doneFunction(ICQEvent *)));
    setCaption(m_sBasic + " [" + m_sProgressMsg +"]");
  }
}

void UserInfoDlg::doneFunction(ICQEvent* e)
{
  if ( !e->Equals(icqEventTag) )
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

  setCaption(m_sBasic + " [" + m_sProgressMsg + result + "]");
  QTimer::singleShot(5000, this, SLOT(resetCaption()));
  setCursor(arrowCursor);
  icqEventTag = 0;
  disconnect (sigman, SIGNAL(signal_doneUserFcn(ICQEvent *)), this, SLOT(doneFunction(ICQEvent *)));
}


void UserInfoDlg::updatedUser(CICQSignal *sig)
{
  if (m_nPPID != sig->PPID() || strcmp(m_szId, sig->Id())) return;

  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
  if (u == NULL) return;

  switch (sig->SubSignal())
  {
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

void UserInfoDlg::slot_aliasChanged(const QString &)
{
  chkKeepAliasOnUpdate->setChecked(true);
}

void UserInfoDlg::resetCaption()
{
  setCaption(m_sBasic);
}

// -----------------------------------------------------------------------------

#include "userinfodlg.moc"

