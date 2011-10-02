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

#include "info.h"

#include "config.h"

#include <cstring>

#include <QCheckBox>
#include <QComboBox>
#include <QDate>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QMovie>
#include <QPixmap>
#include <QPushButton>
#include <QSpinBox>
#include <QTextCodec>
#include <QTreeWidget>
#include <QVBoxLayout>

#ifdef USE_KDE
#include <KDE/KFileDialog>
#include <kdeversion.h>

// TODO
//#include <kabc/stdaddressbook.h>
//#include <kabc/addressee.h>
//#include <kabc/addresseedialog.h>
//#include "core/licqkimiface.h"
//#define USE_KABC
#else
#include <QFileDialog>
#endif

#include <licq/contactlist/owner.h>
#include <licq/contactlist/user.h>
#include <licq/icq/icq.h>
#include <licq/icq/codes.h>
#include <licq/pluginsignal.h>
#include <licq/protocolmanager.h>
#include <licq/logging/log.h>

#include "config/iconmanager.h"
#include "core/messagebox.h"
#include "dialogs/editcategorydlg.h"
#include "dialogs/phonedlg.h"
#include "helpers/usercodec.h"
#include "widgets/infofield.h"
#include "widgets/mledit.h"
#include "widgets/mlview.h"
#include "widgets/timezoneedit.h"
#include "userdlg.h"

#ifdef USE_KABC
#include "core/mainwin.h"
#endif

using Licq::OwnerWriteGuard;
using Licq::gProtocolManager;
using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::UserPages::Info */

UserPages::Info::Info(bool isOwner, unsigned long protocolId, UserDlg* parent)
  : QObject(parent),
    myPpid(protocolId),
    m_bOwner(isOwner),
    myAliasHasChanged(false)
{
  m_PhoneBook = NULL;

  parent->addPage(UserDlg::GeneralPage, createPageGeneral(parent),
      tr("Info"));
  if (myPpid == LICQ_PPID)
  {
    parent->addPage(UserDlg::MorePage, createPageMore(parent),
        tr("More"), UserDlg::GeneralPage);
    parent->addPage(UserDlg::More2Page, createPageMore2(parent),
        tr("More II"), UserDlg::GeneralPage);
    parent->addPage(UserDlg::WorkPage, createPageWork(parent),
        tr("Work"), UserDlg::GeneralPage);
    parent->addPage(UserDlg::AboutPage, createPageAbout(parent),
        tr("About"), UserDlg::GeneralPage);
    parent->addPage(UserDlg::PhonePage, createPagePhoneBook(parent),
        tr("Phone Book"), UserDlg::GeneralPage);
  }
  parent->addPage(UserDlg::PicturePage, createPagePicture(parent),
      tr("Picture"), UserDlg::GeneralPage);
  parent->addPage(UserDlg::CountersPage, createPageCounters(parent),
      tr("Last"));
#ifdef USE_KABC
  parent->addPage(UserDlg::KabcPage, createPageKabc(parent),
      tr("KDE Adressbook"));
#endif
}

void UserPages::Info::load(const Licq::User* user)
{
  myUserId = user->id();
  myId = user->accountId().c_str();
  codec = UserCodec::codecForUser(user);

  loadPageGeneral(user);
  if (myPpid == LICQ_PPID)
  {
    loadPageMore(user);
    loadPageMore2(user);
    loadPageWork(user);
    loadPageAbout(user);
    loadPagePhoneBook(user);
  }
  loadPagePicture(user);
  loadPageCounters(user);
#ifdef USE_KABC
  loadPageKabc(user);
#endif
}

void UserPages::Info::apply(Licq::User* user)
{
  savePageGeneral(user);
  if (myPpid == LICQ_PPID)
  {
    savePageMore(user);
    savePageMore2(user);
    savePageWork(user);
    savePageAbout(user);
    savePagePhoneBook(user);
  }
  savePagePicture(user);
}

void UserPages::Info::apply2(const Licq::UserId& /* userId */)
{
  if (myAliasHasChanged)
    gProtocolManager.updateUserAlias(myUserId);
  myAliasHasChanged = false;

#ifdef USE_KABC
  savePageKabc();
#endif
}

QWidget* UserPages::Info::createPageGeneral(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  myPageGeneralLayout = new QVBoxLayout(w);
  myPageGeneralLayout->setContentsMargins(0, 0, 0, 0);

  unsigned short CR = 0;

  myGeneralBox = new QGroupBox(tr("General Information"));
  QGridLayout* lay = new QGridLayout(myGeneralBox);
  lay->setColumnMinimumWidth(2, 10);

  lay->addWidget(new QLabel(tr("Alias:")), CR, 0);
  nfoAlias = new InfoField(false);
  lay->addWidget(nfoAlias, CR, 1);

  if (!m_bOwner)
  {
    chkKeepAliasOnUpdate = new QCheckBox(tr("Keep Alias on Update"));
    chkKeepAliasOnUpdate->setToolTip(tr(
        "Normally Licq overwrites the Alias when updating user details.\n"
        "Check this if you want to keep your changes to the Alias."));
    lay->addWidget(chkKeepAliasOnUpdate, CR, 3, 1, 2);
    connect(nfoAlias, SIGNAL(textEdited(const QString&)), SLOT(aliasChanged()));
  }

  lay->addWidget(new QLabel(tr("ID:")), ++CR, 0);
  nfoUin = new InfoField(true);
  lay->addWidget(nfoUin, CR, 1);
  lay->addWidget(new QLabel(tr("IP:")), CR, 3);
  nfoIp = new InfoField(true);
  lay->addWidget(nfoIp, CR, 4);

  lay->addWidget(new QLabel(tr("Status:")), ++CR, 0);
  nfoStatus = new InfoField(true);
  lay->addWidget(nfoStatus, CR, 1);
  lay->addWidget(new QLabel(tr("Timezone:")), CR, 3);
  tznZone = new TimeZoneEdit();
  lay->addWidget(tznZone, CR, 4);

  lay->addWidget(new QLabel(tr("Name:")), ++CR, 0);
  nfoFirstName = new InfoField(false);
  lay->addWidget(nfoFirstName, CR, 1);
  nfoLastName = new InfoField(false);
  lay->addWidget(nfoLastName, CR, 2, 1, 3);

  lay->addWidget(new QLabel(tr("EMail 1:")), ++CR, 0);
  nfoEmailPrimary = new InfoField(false);
  lay->addWidget(nfoEmailPrimary, CR, 1, 1, 4);

  if (myPpid == LICQ_PPID)
  {
    lay->addWidget(new QLabel(tr("EMail 2:")), ++CR, 0);
    nfoEmailSecondary = new InfoField(false);
    lay->addWidget(nfoEmailSecondary, CR, 1, 1, 4);

    lay->addWidget(new QLabel(tr("Old Email:")), ++CR, 0);
    nfoEmailOld = new InfoField(false);
    lay->addWidget(nfoEmailOld, CR, 1, 1, 4);

    lay->addWidget(new QLabel(tr("Address:")), ++CR, 0);
    nfoAddress = new InfoField(!m_bOwner);
    lay->addWidget(nfoAddress, CR, 1);
    lay->addWidget(new QLabel(tr("Phone:")), CR, 3);
    nfoPhone = new InfoField(false);//!m_bOwner);
    lay->addWidget(nfoPhone, CR, 4);

    lay->addWidget(new QLabel(tr("State:")), ++CR, 0);
    nfoState = new InfoField(!m_bOwner);
    nfoState->setMaxLength(3);
    lay->addWidget(nfoState, CR, 1);
    w->setTabOrder(nfoAddress, nfoState);
    lay->addWidget(new QLabel(tr("Fax:")), CR, 3);
    nfoFax = new InfoField(false);//!m_bOwner);
    lay->addWidget(nfoFax, CR, 4);
    w->setTabOrder(nfoPhone, nfoFax);

    lay->addWidget(new QLabel(tr("City:")), ++CR, 0);
    nfoCity = new InfoField(!m_bOwner);
    lay->addWidget(nfoCity, CR, 1);
    w->setTabOrder(nfoState, nfoCity);
    lay->addWidget(new QLabel(tr("Cellular:")), CR, 3);
    nfoCellular = new InfoField(false);//!m_bOwner);
    lay->addWidget(nfoCellular, CR, 4);
    w->setTabOrder(nfoFax, nfoCellular);

    lay->addWidget(new QLabel(tr("Zip:")), ++CR, 0);
    nfoZipCode = new InfoField(!m_bOwner);
    lay->addWidget(nfoZipCode, CR, 1);
    w->setTabOrder(nfoCity, nfoZipCode);
    lay->addWidget(new QLabel(tr("Country:")), CR, 3);
    if (m_bOwner)
    {
      cmbCountry = new QComboBox();
      //cmbCountry->addItem(tr("Unspecified"));
      cmbCountry->setMaximumWidth(cmbCountry->sizeHint().width()+20);
      for (unsigned short i = 0; i < NUM_COUNTRIES; i++)
        cmbCountry->addItem(GetCountryByIndex(i)->szName);
      lay->addWidget(cmbCountry, CR, 4);
    }
    else
    {
      nfoCountry = new InfoField(!m_bOwner);
      lay->addWidget(nfoCountry, CR, 4);
    }
  }

  lay->setRowStretch(++CR, 5);

  myPageGeneralLayout->addWidget(myGeneralBox);
  myPageGeneralLayout->addStretch(1);

  return w;
}

void UserPages::Info::loadPageGeneral(const Licq::User* u)
{
  if (!m_bOwner)
    chkKeepAliasOnUpdate->setChecked(u->KeepAliasOnUpdate());
  nfoUin->setText(myId);
  nfoAlias->setText(QString::fromUtf8(u->getAlias().c_str()));
  nfoFirstName->setText(codec->toUnicode(u->getFirstName().c_str()));
  nfoLastName->setText(codec->toUnicode(u->getLastName().c_str()));
  QString ip(u->ipToString().c_str());
  if (u->Ip() != u->IntIp() && u->IntIp() != 0)
  {
    ip.append(QString(" / %1").arg(u->internalIpToString().c_str()));
  }
  if (u->Port() != 0)
  {
    ip.append(QString(":%1").arg(u->portToString().c_str()));
  }
  nfoIp->setText(ip);
  tznZone->setData(u->GetTimezone());
  // Owner timezone is not editable, it is taken from system timezone instead
  if (m_bOwner)
    tznZone->setEnabled(false);
  nfoStatus->setText(u->statusString().c_str());
  nfoEmailPrimary->setText(codec->toUnicode(u->getUserInfoString("Email1").c_str()));

  if (myPpid != LICQ_PPID)
    return;

  nfoEmailSecondary->setText(codec->toUnicode(u->getUserInfoString("Email2").c_str()));
  nfoEmailOld->setText(codec->toUnicode(u->getUserInfoString("Email0").c_str()));
  unsigned int countryCode = u->getUserInfoUint("Country");
  if (m_bOwner)
  {
    const SCountry* c = GetCountryByCode(countryCode);
    if (c == NULL)
      cmbCountry->setCurrentIndex(0);
    else
      cmbCountry->setCurrentIndex(c->nIndex);
  }
  else
  {
    const SCountry* c = GetCountryByCode(countryCode);
    if (c == NULL)
      nfoCountry->setText(tr("Unknown (%1)").arg(countryCode));
    else  // known
      nfoCountry->setText(c->szName);
  }
  nfoAddress->setText(codec->toUnicode(u->getUserInfoString("Address").c_str()));
  nfoCity->setText(codec->toUnicode(u->getUserInfoString("City").c_str()));
  nfoState->setText(codec->toUnicode(u->getUserInfoString("State").c_str()));
  nfoPhone->setText(codec->toUnicode(u->getUserInfoString("PhoneNumber").c_str()));
  nfoFax->setText(codec->toUnicode(u->getUserInfoString("FaxNumber").c_str()));
  nfoCellular->setText(codec->toUnicode(u->getCellularNumber().c_str()));
  nfoZipCode->setText(codec->toUnicode(u->getUserInfoString("Zipcode").c_str()));
}

void UserPages::Info::savePageGeneral(Licq::User* u)
{
  myAliasHasChanged = (u->getAlias() != nfoAlias->text().toUtf8().data());
  u->setAlias(nfoAlias->text().toUtf8().data());
  if (!m_bOwner)
    u->SetKeepAliasOnUpdate(chkKeepAliasOnUpdate->isChecked());
  u->SetTimezone(tznZone->data());
  u->setUserInfoString("FirstName", codec->fromUnicode(nfoFirstName->text()).data());
  u->setUserInfoString("LastName", codec->fromUnicode(nfoLastName->text()).data());
  u->setUserInfoString("Email1", codec->fromUnicode(nfoEmailPrimary->text()).data());

  if (myPpid != LICQ_PPID)
    return;

  u->setUserInfoString("Email2", codec->fromUnicode(nfoEmailSecondary->text()).data());
  u->setUserInfoString("Email0", codec->fromUnicode(nfoEmailOld->text()).data());
  u->setUserInfoString("City", codec->fromUnicode(nfoCity->text()).data());
  u->setUserInfoString("State", codec->fromUnicode(nfoState->text()).data());
  u->setUserInfoString("Address", codec->fromUnicode(nfoAddress->text()).data());
  u->setUserInfoString("PhoneNumber", codec->fromUnicode(nfoPhone->text()).data());
  u->setUserInfoString("FaxNumber", codec->fromUnicode(nfoFax->text()).data());
  u->setUserInfoString("CellularNumber", codec->fromUnicode(nfoCellular->text()).data());
  u->setUserInfoString("Zipcode", codec->fromUnicode(nfoZipCode->text()).data());
  if (m_bOwner)
  {
    unsigned short i = cmbCountry->currentIndex();
    u->setUserInfoUint("Country", GetCountryByIndex(i)->nCode);
  }
}

QWidget* UserPages::Info::createPageMore(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  myPageMoreLayout = new QVBoxLayout(w);
  myPageMoreLayout->setContentsMargins(0, 0, 0, 0);

  unsigned short CR = 0;
  myMoreBox = new QGroupBox(tr("More"));
  QGridLayout* lay = new QGridLayout(myMoreBox);
  lay->setRowMinimumHeight(6, 5);

  lay->addWidget(new QLabel(tr("Age:")), CR, 0);
  nfoAge = new InfoField(!m_bOwner);
  lay->addWidget(nfoAge, CR, 1);
  lay->addWidget(new QLabel(tr("Gender:")), CR, 3);
  if (m_bOwner)
  {
    cmbGender = new QComboBox();
    cmbGender->insertItem(Licq::User::GenderUnspecified, tr("Unspecified"));
    cmbGender->insertItem(Licq::User::GenderFemale, tr("Female"));
    cmbGender->insertItem(Licq::User::GenderMale, tr("Male"));
    lay->addWidget(cmbGender, CR, 4);
  }
  else
  {
    nfoGender = new InfoField(true);
    lay->addWidget(nfoGender, CR, 4);
  }

  lay->addWidget(new QLabel(tr("Homepage:")), ++CR, 0);
  nfoHomepage = new InfoField(!m_bOwner);
  lay->addWidget(nfoHomepage, CR, 1, 1, 4);

  lay->addWidget(new QLabel(tr("Category:")), ++CR, 0);
  lvHomepageCategory = new QTreeWidget();
  lvHomepageCategory->setColumnCount(1);
  lvHomepageCategory->header()->hide();
  lvHomepageCategory->setRootIsDecorated(true);
  lvHomepageCategory->setMaximumHeight(50);
  lay->addWidget(lvHomepageCategory, CR, 1, 1, 4);

  lay->addWidget(new QLabel(tr("Description:")), ++CR, 0);
  mleHomepageDesc = new MLEdit(true);
  mleHomepageDesc->setReadOnly(true);
  lay->addWidget(mleHomepageDesc, CR, 1, 1, 4);

  lay->addWidget(new QLabel(tr("Birthday:")), ++CR, 0);
  if (m_bOwner)
  {
    QHBoxLayout* w = new QHBoxLayout();
    w->setSpacing(8);
    QLabel* lblDay = new QLabel(tr(" Day:"));
    lblDay->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    spnBirthDay = new QSpinBox();
    spnBirthDay->setRange(1, 31);
    QLabel* lblMonth = new QLabel(tr(" Month:"));
    lblMonth->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    spnBirthMonth = new QSpinBox();
    spnBirthMonth->setRange(1, 12);
    QLabel* lblYear = new QLabel(tr(" Year:"));
    lblYear->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    spnBirthYear = new QSpinBox();
    spnBirthYear->setRange(1900, 2020);
    lay->addLayout(w, CR, 1, 1, 4);
  }
  else
  {
    nfoBirthday = new InfoField(!m_bOwner);
    lay->addWidget(nfoBirthday, CR, 1, 1, 4);
  }

  if (m_bOwner)
  {
    lay->addWidget(new QLabel(tr("Language 1:")), ++CR, 0);
    cmbLanguage[0] = new QComboBox();
    lay->addWidget(cmbLanguage[0], CR, 1);
    lay->addWidget(new QLabel(tr("Language 2:")), CR, 3);
    cmbLanguage[1] = new QComboBox();
    lay->addWidget(cmbLanguage[1], CR, 4);

    lay->addWidget(new QLabel(tr("Language 3:")), ++CR, 0);
    cmbLanguage[2] = new QComboBox();
    lay->addWidget(cmbLanguage[2], CR, 1);

    for (unsigned short i = 0; i < 3; i++)
    {
      for (unsigned short j = 0; j < NUM_LANGUAGES; j++)
        if (GetLanguageByIndex(j))
          cmbLanguage[i]->addItem(GetLanguageByIndex(j)->szName);
    }
  }
  else
  {
    lay->addWidget(new QLabel(tr("Language 1:")), ++CR, 0);
    nfoLanguage[0] = new InfoField(!m_bOwner);
    lay->addWidget(nfoLanguage[0], CR, 1);
    lay->addWidget(new QLabel(tr("Language 2:")), CR, 3);
    nfoLanguage[1] = new InfoField(!m_bOwner);
    lay->addWidget(nfoLanguage[1], CR, 4);

    lay->addWidget(new QLabel(tr("Language 3:")), ++CR, 0);
    nfoLanguage[2] = new InfoField(!m_bOwner);
    lay->addWidget(nfoLanguage[2], CR, 1);
  }

  lblAuth = new QLabel();
  CR++;
  lay->addWidget(lblAuth, CR, 0, 1, 5);

  lblICQHomepage = new QLabel();
  CR++;
  lay->addWidget(lblICQHomepage, CR, 0, 1, 5);

  myPageMoreLayout->addWidget(myMoreBox);
  myPageMoreLayout->addStretch(1);

  return w;
}

void UserPages::Info::loadPageMore(const Licq::User* u)
{
  // Gender
  unsigned int gender = u->getUserInfoUint("Gender");
  if (m_bOwner)
  {
    cmbGender->setCurrentIndex(gender);
  }
  else
  {
    if (gender == Licq::User::GenderFemale)
      nfoGender->setText(tr("Female"));
    else if (gender == Licq::User::GenderMale)
      nfoGender->setText(tr("Male"));
    else
      nfoGender->setText(tr("Unspecified"));
  }

  // Age
  unsigned int age = u->getUserInfoUint("Age");
  if (age == Licq::User::AgeUnspecified)
    nfoAge->setText(tr("Unspecified"));
  else
    nfoAge->setText(age);

  // Birthday
  unsigned int birthDay = u->getUserInfoUint("BirthDay");
  unsigned int birthMonth = u->getUserInfoUint("BirthMonth");
  unsigned int birthYear = u->getUserInfoUint("BirthYear");
  if (m_bOwner)
  {
    spnBirthDay->setValue(birthDay);
    spnBirthMonth->setValue(birthMonth);
    spnBirthYear->setValue(birthYear);
  }
  else
  {
    if (birthMonth == 0 || birthDay == 0)
    {
      nfoBirthday->setText(tr("Unspecified"));
    }
    else
    {
      QDate d(birthYear, birthMonth, birthDay);
      nfoBirthday->setText(d.toString());
    }
  }

  nfoHomepage->setText(codec->toUnicode(u->getUserInfoString("Homepage").c_str()));

  lvHomepageCategory->clear();
  mleHomepageDesc->clear();
  if (u->getUserInfoBool("HomepageCatPresent"))
  {
    if (m_bOwner)
      mleHomepageDesc->setReadOnly(false);

    const SHomepageCat* c = GetHomepageCatByCode(u->getUserInfoUint("HomepageCatCode"));
    if (c != NULL)
    {
      QTreeWidgetItem* lvi = new QTreeWidgetItem(lvHomepageCategory);
      int rowCount = 1;
      char* sTmp = strdup(c->szName);
      char* front = sTmp;
      char* last = NULL;
      char* end;
      while (true)
      {
        lvi->setExpanded(true);
        end = strchr(front, '/');
        if (end == NULL)
        {
          lvi->setText(0, front);
          break;
        }

        *end = '\0';

        if (last == NULL || strcmp(last, front))
        {
          lvi->setText(0, front);
          last = front;
          lvi = new QTreeWidgetItem(lvi);
          rowCount++;
        }

        front = end + 1;
      }
      lvHomepageCategory->setMaximumHeight(lvHomepageCategory->sizeHintForRow(0) * rowCount + 5);
      free(sTmp);
    }
    QString descstr = codec->toUnicode(u->getUserInfoString("HomepageDesc").c_str());
    descstr.replace(QRegExp("\r"), "");
    mleHomepageDesc->setText(descstr);
  }

  for (unsigned short i = 0; i < 3; i++)
  {
    unsigned int language = u->getUserInfoUint(QString("Language%1").arg(i).toLatin1().constData());
    const SLanguage* l = GetLanguageByCode(language);
    if (m_bOwner)
    {
      if (l == NULL)
        cmbLanguage[i]->setCurrentIndex(0);
      else
        cmbLanguage[i]->setCurrentIndex(l->nIndex);
    }
    else
    {
      if (l == NULL)
        nfoLanguage[i]->setText(tr("Unknown (%1)").arg(language));
      else  // known
        nfoLanguage[i]->setText(l->szName);
    }
  }

  if (u->GetAuthorization())
    lblAuth->setText(tr("Authorization Required"));
  else
    lblAuth->setText(tr("Authorization Not Required"));

  if (u->getUserInfoBool("ICQHomepagePresent"))
  {
    QString url;
    url.sprintf("(http://%s.homepage.icq.com/)", myId.toLatin1().constData());
    lblICQHomepage->setText(tr("User has an ICQ Homepage ") + url);
  }
  else
    lblICQHomepage->setText(tr("User has no ICQ Homepage"));
}

void UserPages::Info::savePageMore(Licq::User* u)
{
  u->setUserInfoUint("Age", nfoAge->text().toULong());
  u->setUserInfoString("Homepage", nfoHomepage->text().toLocal8Bit().constData());
  if (m_bOwner)
  {
    u->setUserInfoUint("Gender", cmbGender->currentIndex());
    u->setUserInfoUint("BirthYear", spnBirthYear->value());
    u->setUserInfoUint("BirthMonth", spnBirthMonth->value());
    u->setUserInfoUint("BirthDay", spnBirthDay->value());
    u->setUserInfoUint("Language0", GetLanguageByIndex(cmbLanguage[0]->currentIndex())->nCode);
    u->setUserInfoUint("Language1", GetLanguageByIndex(cmbLanguage[1]->currentIndex())->nCode);
    u->setUserInfoUint("Language2", GetLanguageByIndex(cmbLanguage[2]->currentIndex())->nCode);
  }
}

QWidget* UserPages::Info::createPageMore2(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  myPageMore2Layout = new QVBoxLayout(w);
  myPageMore2Layout->setContentsMargins(0, 0, 0, 0);

  myMore2Box = new QGroupBox(tr("More II"));
  QVBoxLayout* lay = new QVBoxLayout(myMore2Box);

  lsvMore2 = new QTreeWidget();
  lsvMore2->setColumnCount(1);
  lsvMore2->header()->hide();
  lsvMore2->setEnabled(true);
  lsvMore2->setAllColumnsShowFocus(true);
  if (!m_bOwner)
    lsvMore2->setSelectionMode(QTreeWidget::NoSelection);
  lay->addWidget(lsvMore2);

  lviMore2Top[Licq::CAT_BACKGROUND] = new QTreeWidgetItem(lsvMore2);
  lviMore2Top[Licq::CAT_BACKGROUND]->setText(0, "Past Background");
  lviMore2Top[Licq::CAT_BACKGROUND]->setExpanded(true);

  lviMore2Top[Licq::CAT_ORGANIZATION] = new QTreeWidgetItem(lsvMore2);
  lviMore2Top[Licq::CAT_ORGANIZATION]->setText(0, "Organization, Affiliation, Group");
  lviMore2Top[Licq::CAT_ORGANIZATION]->setExpanded(true);

  lviMore2Top[Licq::CAT_INTERESTS] = new QTreeWidgetItem(lsvMore2);
  lviMore2Top[Licq::CAT_INTERESTS]->setText(0, "Personal Interests");
  lviMore2Top[Licq::CAT_INTERESTS]->setExpanded(true);

  if (m_bOwner)
    connect(lsvMore2, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
        SLOT(editCategory(QTreeWidgetItem*)));

  myPageMore2Layout->addWidget(myMore2Box);
  myPageMore2Layout->addStretch(1);

  return w;
}

int UserPages::Info::splitCategory(QTreeWidgetItem* parent, const char* descr)
{
  char* p;
  char* q;
  char* s;
  QTreeWidgetItem* lvi = NULL;

  if (descr == NULL || !*descr)
    return -1;

  s = p = strdup(descr);
  if (p == NULL)
    return -1;

  while ((q = strchr(s,',')))
  {
    if (q)
    {
      *q = '\0';

      if (*s)
      {
        QString qs = codec->toUnicode(s);
        if (lvi == NULL)
        {
          lvi = new QTreeWidgetItem(parent);
          lvi->setText(0, qs);
        }
        else
        {
          lvi = new QTreeWidgetItem(parent, lvi);
          lvi->setText(0, qs);
        }
      }
      s = q + 1;
    }
  }
  if (*s)
  {
    QString qs = codec->toUnicode(s);
    if (lvi == NULL)
    {
      lvi = new QTreeWidgetItem(parent);
      lvi->setText(0, qs);
    }
    else
    {
      lvi = new QTreeWidgetItem(parent, lvi);
      lvi->setText(0, qs);
    }
  }

  parent->setExpanded(true);

  free(p);
  return 0;
}

void UserPages::Info::loadPageMore2(const Licq::User* u)
{
  myInterests = u->getInterests();
  updateMore2Info(Licq::CAT_INTERESTS, myInterests);

  myOrganizations = u->getOrganizations();
  updateMore2Info(Licq::CAT_ORGANIZATION, myOrganizations);

  myBackgrounds = u->getBackgrounds();
  updateMore2Info(Licq::CAT_BACKGROUND, myBackgrounds);
}

void UserPages::Info::updateMore2Info(Licq::UserCat cat, const Licq::UserCategoryMap& category)
{
  QTreeWidgetItem* lvi = NULL;

  while (QTreeWidgetItem* lvChild = lviMore2Top[cat]->takeChild(0))
    delete lvChild;

  const struct SCategory* (*cat2str)(unsigned short);
  switch (cat)
  {
    case Licq::CAT_INTERESTS:
    cat2str = GetInterestByCode;
    break;
    case Licq::CAT_ORGANIZATION:
    cat2str = GetOrganizationByCode;
    break;
    case Licq::CAT_BACKGROUND:
    cat2str = GetBackgroundByCode;
    break;
  default:
    return;
  }

  Licq::UserCategoryMap::const_iterator i;
  for (i = category.begin(); i != category.end(); ++i)
  {
    const struct SCategory* sCat = cat2str(i->first);
    QString name;
    if (sCat == NULL)
      name = tr("Unknown");
    else
      name = sCat->szName;

    if (lvi == NULL)
    {
      lvi = new QTreeWidgetItem(lviMore2Top[cat]);
      lvi->setText(0, name);
    }
    else
    {
      lvi = new QTreeWidgetItem(lviMore2Top[cat], lvi);
      lvi->setText(0, name);
    }
    splitCategory(lvi, i->second.c_str());
  }

  if (category.empty())
  {
    lvi = new QTreeWidgetItem(lviMore2Top[cat]);
    lvi->setText(0, tr("(none)"));
  }
}

void UserPages::Info::savePageMore2(Licq::User* u)
{
  u->getInterests() = myInterests;
  u->getOrganizations() = myOrganizations;
  u->getBackgrounds() = myBackgrounds;
}

QWidget* UserPages::Info::createPageWork(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  myPageWorkLayout = new QVBoxLayout(w);
  myPageWorkLayout->setContentsMargins(0, 0, 0, 0);

  unsigned short CR = 0;

  myWorkBox = new QGroupBox(tr("Work"));
  QGridLayout* lay = new QGridLayout(myWorkBox);
  lay->setColumnMinimumWidth(2, 10);
  lay->setRowStretch(9, 1);

  lay->addWidget(new QLabel(tr("Name:")), CR, 0);
  nfoCompanyName = new InfoField(!m_bOwner);
  lay->addWidget(nfoCompanyName, CR, 1, 1, 4);

  lay->addWidget(new QLabel(tr("Department:")), ++CR, 0);
  nfoCompanyDepartment = new InfoField(!m_bOwner);
  lay->addWidget(nfoCompanyDepartment, CR, 1, 1, 4);

  lay->addWidget(new QLabel(tr("Position:")), ++CR, 0);
  nfoCompanyPosition = new InfoField(!m_bOwner);
  lay->addWidget(nfoCompanyPosition, CR, 1, 1, 4);

  lay->addWidget(new QLabel(tr("Occupation:")), ++CR, 0);
  if (m_bOwner)
  {
    cmbCompanyOccupation = new QComboBox();
    cmbCompanyOccupation->setMaximumWidth(cmbCompanyOccupation->sizeHint().width()+20);

    for (unsigned short i = 0; i < NUM_OCCUPATIONS; i++)
      cmbCompanyOccupation->addItem(GetOccupationByIndex(i)->szName);
    lay->addWidget(cmbCompanyOccupation, CR, 1);
  }
  else
  {
    nfoCompanyOccupation = new InfoField(!m_bOwner);
    lay->addWidget(nfoCompanyOccupation, CR, 1);
  }

  lay->addWidget(new QLabel(tr("City:")), ++CR, 0);
  nfoCompanyCity = new InfoField(!m_bOwner);
  lay->addWidget(nfoCompanyCity, CR, 1);
  lay->addWidget(new QLabel(tr("State:")), CR, 3);
  nfoCompanyState = new InfoField(!m_bOwner);
  nfoCompanyState->setMaxLength(5);
  lay->addWidget(nfoCompanyState, CR, 4);

  lay->addWidget(new QLabel(tr("Address:")), ++CR, 0);
  nfoCompanyAddress = new InfoField(!m_bOwner);
  lay->addWidget(nfoCompanyAddress, CR, 1, 1, 4);

  lay->addWidget(new QLabel(tr("Zip:")), ++CR, 0);
  nfoCompanyZip = new InfoField(!m_bOwner);
  lay->addWidget(nfoCompanyZip, CR, 1);
  lay->addWidget(new QLabel(tr("Country:")), CR, 3);
  if (m_bOwner)
  {
    cmbCompanyCountry = new QComboBox();
    cmbCompanyCountry->setMaximumWidth(cmbCompanyCountry->sizeHint().width()+20);
    for (unsigned short i = 0; i < NUM_COUNTRIES; i++)
      cmbCompanyCountry->addItem(GetCountryByIndex(i)->szName);
    lay->addWidget(cmbCompanyCountry, CR, 4);
  }
  else
  {
    nfoCompanyCountry = new InfoField(!m_bOwner);
    lay->addWidget(nfoCompanyCountry, CR, 4);
  }

  lay->addWidget(new QLabel(tr("Phone:")), ++CR, 0);
  nfoCompanyPhone = new InfoField(!m_bOwner);
  lay->addWidget(nfoCompanyPhone, CR, 1);
  lay->addWidget(new QLabel(tr("Fax:")), CR, 3);
  nfoCompanyFax = new InfoField(!m_bOwner);
  lay->addWidget(nfoCompanyFax, CR, 4);

  lay->addWidget(new QLabel(tr("Homepage:")), ++CR, 0);
  nfoCompanyHomepage = new InfoField(!m_bOwner);
  lay->addWidget(nfoCompanyHomepage, CR, 1, 1, 4);

  myPageWorkLayout->addWidget(myWorkBox);
  myPageWorkLayout->addStretch(1);

  return w;
}

void UserPages::Info::loadPageWork(const Licq::User* u)
{
  nfoCompanyName->setText(codec->toUnicode(u->getUserInfoString("CompanyName").c_str()));
  nfoCompanyDepartment->setText(codec->toUnicode(u->getUserInfoString("CompanyDepartment").c_str()));
  nfoCompanyPosition->setText(codec->toUnicode(u->getUserInfoString("CompanyPosition").c_str()));
  nfoCompanyCity->setText(codec->toUnicode(u->getUserInfoString("CompanyCity").c_str()));
  nfoCompanyState->setText(codec->toUnicode(u->getUserInfoString("CompanyState").c_str()));
  nfoCompanyAddress->setText(codec->toUnicode(u->getUserInfoString("CompanyAddress").c_str()));
  nfoCompanyZip->setText(codec->toUnicode(u->getUserInfoString("CompanyZip").c_str()));
  unsigned int companyCountry = u->getUserInfoUint("CompanyCountry");
  unsigned int companyOccupation = u->getUserInfoUint("CompanyOccupation");
  if (m_bOwner)
  {
    const SCountry* c = GetCountryByCode(companyCountry);
    if (c == NULL)
      cmbCompanyCountry->setCurrentIndex(0);
    else
      cmbCompanyCountry->setCurrentIndex(c->nIndex);

    const SOccupation* o = GetOccupationByCode(companyOccupation);
    if (o == NULL)
      cmbCompanyOccupation->setCurrentIndex(0);
    else
      cmbCompanyOccupation->setCurrentIndex(o->nIndex);
  }
  else
  {
    const SCountry* c = GetCountryByCode(companyCountry);
    if (c == NULL)
      nfoCompanyCountry->setText(tr("Unknown (%1)").arg(companyCountry));
    else  // known
      nfoCompanyCountry->setText(c->szName);

    const SOccupation* o = GetOccupationByCode(companyOccupation);
    if (o == NULL)
      nfoCompanyOccupation->setText(tr("Unknown (%1)").arg(companyOccupation));
    else
      nfoCompanyOccupation->setText(o->szName);
  }
  nfoCompanyPhone->setText(codec->toUnicode(u->getUserInfoString("CompanyPhoneNumber").c_str()));
  nfoCompanyFax->setText(codec->toUnicode(u->getUserInfoString("CompanyFaxNumber").c_str()));
  nfoCompanyHomepage->setText(codec->toUnicode(u->getUserInfoString("CompanyHomepage").c_str()));
}

void UserPages::Info::savePageWork(Licq::User* u)
{
  u->setUserInfoString("CompanyCity", codec->fromUnicode(nfoCompanyCity->text()).data());
  u->setUserInfoString("CompanyState", codec->fromUnicode(nfoCompanyState->text()).data());
  u->setUserInfoString("CompanyPhoneNumber", codec->fromUnicode(nfoCompanyPhone->text()).data());
  u->setUserInfoString("CompanyFaxNumber", codec->fromUnicode(nfoCompanyFax->text()).data());
  u->setUserInfoString("CompanyAddress", codec->fromUnicode(nfoCompanyAddress->text()).data());
  u->setUserInfoString("CompanyZip", codec->fromUnicode(nfoCompanyZip->text()).data());
  if (m_bOwner)
  {
    unsigned short i = cmbCompanyCountry->currentIndex();
    u->setUserInfoUint("CompanyCountry", GetCountryByIndex(i)->nCode);

    i = cmbCompanyOccupation->currentIndex();
    u->setUserInfoUint("CompanyOccupation", GetOccupationByIndex(i)->nCode);
  }
  u->setUserInfoString("CompanyName", codec->fromUnicode(nfoCompanyName->text()).data());
  u->setUserInfoString("CompanyDepartment", codec->fromUnicode(nfoCompanyDepartment->text()).data());
  u->setUserInfoString("CompanyPosition", codec->fromUnicode(nfoCompanyPosition->text()).data());
  u->setUserInfoString("CompanyHomepage", codec->fromUnicode(nfoCompanyHomepage->text()).data());
}

QWidget* UserPages::Info::createPageAbout(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  myPageAboutLayout = new QVBoxLayout(w);
  myPageAboutLayout->setContentsMargins(0, 0, 0, 0);

  myAboutBox = new QGroupBox(tr("About"));
  QVBoxLayout* lay = new QVBoxLayout(myAboutBox);

  mlvAbout = new MLView();//EditWrap(true, p);
  mlvAbout->setReadOnly(!m_bOwner);
  lay->addWidget(mlvAbout);

  myPageAboutLayout->addWidget(myAboutBox);
  myPageAboutLayout->addStretch(1);

  return w;
}

void UserPages::Info::loadPageAbout(const Licq::User* u)
{
  bool bUseHTML = myId[0].isLetter();

  QString aboutstr = codec->toUnicode(u->getUserInfoString("About").c_str());
  aboutstr.replace(QRegExp("\r"), "");
  mlvAbout->clear();
  mlvAbout->append(MLView::toRichText(aboutstr, true, bUseHTML));
}

void UserPages::Info::savePageAbout(Licq::User* u)
{
  QString str = mlvAbout->toPlainText();

  u->setUserInfoString("About", codec->fromUnicode(str.left(450)).data());
}

QWidget* UserPages::Info::createPagePhoneBook(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  myPagePhoneBookLayout = new QVBoxLayout(w);
  myPagePhoneBookLayout->setContentsMargins(0, 0, 0, 0);

  myPhoneBookBox = new QGroupBox(tr("PhoneBook"));
  QVBoxLayout* lay = new QVBoxLayout(myPhoneBookBox);

  lsvPhoneBook = new QTreeWidget();
  lsvPhoneBook->setColumnCount(3);
  QStringList labels;
  labels << tr("Type");
  labels << tr("Number/Gateway");
  labels << tr("Country/Provider");
  lsvPhoneBook->setHeaderLabels(labels);
  lsvPhoneBook->setEnabled(true);
  lsvPhoneBook->setAllColumnsShowFocus(true);
  lay->addWidget(lsvPhoneBook);

  QHBoxLayout* hlay = new QHBoxLayout();
  lay->addLayout(hlay);

  hlay->addWidget(new QLabel(tr("Currently at:")));

  if (m_bOwner)
  {
    cmbActive = new QComboBox();
    hlay->addWidget(cmbActive);

    connect(lsvPhoneBook, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
        SLOT(editPhoneEntry(QTreeWidgetItem*)));
    connect(cmbActive, SIGNAL(activated(int)), SLOT(changeActivePhone(int)));
  }
  else
  {
    nfoActive = new InfoField(true);
    hlay->addWidget(nfoActive);

    lsvPhoneBook->setSelectionMode(QTreeWidget::NoSelection);
  }

  if (m_bOwner)
  {
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch(1);

    myPhoneAddButton = new QPushButton(tr("Add..."));
    connect(myPhoneAddButton, SIGNAL(clicked()), SLOT(addPhone()));
    buttonLayout->addWidget(myPhoneAddButton);

    myPhoneClearButton = new QPushButton(tr("Clear"));
    connect(myPhoneClearButton, SIGNAL(clicked()), SLOT(clearPhone()));
    buttonLayout->addWidget(myPhoneClearButton);

    lay->addLayout(buttonLayout);
  }

  myPagePhoneBookLayout->addWidget(myPhoneBookBox);
  myPagePhoneBookLayout->addStretch(1);

  return w;
}

void UserPages::Info::loadPagePhoneBook(const Licq::User* u)
{
  if (m_PhoneBook != NULL)
    delete m_PhoneBook;

  m_PhoneBook = new Licq::ICQUserPhoneBook();
  const struct Licq::PhoneBookEntry* entry;
  for (unsigned long i = 0; u->GetPhoneBook()->Get(i, &entry); i++)
    m_PhoneBook->AddEntry(entry);

  updatePhoneBook();
}

void UserPages::Info::updatePhoneBook()
{
  lsvPhoneBook->clear();

  if (m_bOwner)
  {
    cmbActive->clear();
    cmbActive->addItem("");
  }
  else
    nfoActive->clear();

  QTreeWidgetItem* lsv = NULL;
  const struct Licq::PhoneBookEntry* entry;
  for (unsigned long i = 0; m_PhoneBook->Get(i, &entry); i++)
  {
    QString description = codec->toUnicode(entry->description.c_str());
    QString number;
    QString country;
    if (entry->nType == Licq::TYPE_PAGER)
    {
      //Windows icq uses extension, try it first
      if (!entry->extension.empty())
        number = codec->toUnicode(entry->extension.c_str());
      else
        number = codec->toUnicode(entry->phoneNumber.c_str());

      QString gateway;
      if (entry->nGatewayType == Licq::GATEWAY_BUILTIN)
      {
        country = codec->toUnicode(entry->gateway.c_str());

        const struct SProvider* sProvider = GetProviderByName(entry->gateway.c_str());
        if (sProvider != NULL)
          gateway = sProvider->szGateway;
        else
          gateway = tr("Unknown");
      }
      else
      {
        country = tr("Unknown");
        gateway = codec->toUnicode(entry->gateway.c_str());
      }

      number += gateway;
    }
    else
    {
      const struct SCountry* sCountry = GetCountryByName(entry->country.c_str());
      if (sCountry != NULL)
        number.sprintf("+%u ", sCountry->nPhone);
      const char* szAreaCode = entry->areaCode.c_str();
      if (entry->nRemoveLeading0s)
        szAreaCode += strspn(szAreaCode, "0");
      if (szAreaCode[0] != '\0')
        number += tr("(") + codec->toUnicode(szAreaCode) + tr(") ");
      else if (!entry->areaCode.empty())
        number += tr("(") + codec->toUnicode(entry->areaCode.c_str()) + tr(") ");
      number += codec->toUnicode(entry->phoneNumber.c_str());
      if (!entry->extension.empty())
        number += tr("-") + codec->toUnicode(entry->extension.c_str());

      country = codec->toUnicode(entry->country.c_str());
    }

    if (m_bOwner)
    {
      cmbActive->addItem(number);
      if (entry->nActive)
        cmbActive->setCurrentIndex(i + 1);
    }
    else if (entry->nActive)
    {
      nfoActive->setText(number);
    }

    if (lsv == NULL)
    {
      lsv = new QTreeWidgetItem(lsvPhoneBook);
      lsv->setText(0, description);
      lsv->setText(1, number);
      lsv->setText(2, country);
    }
    else
    {
      lsv = new QTreeWidgetItem(lsvPhoneBook, lsv);
      lsv->setText(0, description);
      lsv->setText(1, number);
      lsv->setText(2, country);
    }

    switch (entry->nType)
    {
      case Licq::TYPE_PHONE:
        lsv->setIcon(0, IconManager::instance()->getIcon(IconManager::PSTNIcon));
        break;
      case Licq::TYPE_CELLULAR:
        lsv->setIcon(0, IconManager::instance()->getIcon(IconManager::MobileIcon));
        break;
      case Licq::TYPE_CELLULARxSMS:
        lsv->setIcon(0, IconManager::instance()->getIcon(IconManager::SMSIcon));
        break;
      case Licq::TYPE_FAX:
        lsv->setIcon(0, IconManager::instance()->getIcon(IconManager::FaxIcon));
        break;
      case Licq::TYPE_PAGER:
        lsv->setIcon(0, IconManager::instance()->getIcon(IconManager::PagerIcon));
      break;
    default:
      break;
    }
  }

  for (int i = 0; i < lsvPhoneBook->columnCount(); i++)
    lsvPhoneBook->resizeColumnToContents(i);
}

void UserPages::Info::savePagePhoneBook(Licq::User* u)
{
  u->GetPhoneBook()->Clean();
  const struct Licq::PhoneBookEntry* entry;
  for (unsigned long i = 0; m_PhoneBook->Get(i, &entry); i++)
    u->GetPhoneBook()->AddEntry(entry);
}

void UserPages::Info::clearPhone()
{
  unsigned long nSelection = lsvPhoneBook->indexOfTopLevelItem(lsvPhoneBook->currentItem());

  m_PhoneBook->ClearEntry(nSelection);
  updatePhoneBook();
}


QWidget* UserPages::Info::createPagePicture(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  myPagePictureLayout = new QVBoxLayout(w);
  myPagePictureLayout->setContentsMargins(0, 0, 0, 0);

  myPictureBox = new QGroupBox(tr("Picture"));
  QVBoxLayout* lay = new QVBoxLayout(myPictureBox);
  lblPicture = new QLabel();
  lblPicture->setAlignment(lblPicture->alignment() | Qt::AlignHCenter);
  lay->addWidget(lblPicture);

  if (m_bOwner)
  {
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch(1);

    myPictureBrowseButton = new QPushButton(tr("Browse..."));
    connect(myPictureBrowseButton, SIGNAL(clicked()), SLOT(browsePicture()));
    buttonLayout->addWidget(myPictureBrowseButton);

    myPictureClearButton = new QPushButton(tr("Clear"));
    connect(myPictureClearButton, SIGNAL(clicked()), SLOT(clearPicture()));
    buttonLayout->addWidget(myPictureClearButton);

    lay->addLayout(buttonLayout);
  }

  myPagePictureLayout->addWidget(myPictureBox);
  myPagePictureLayout->addStretch(1);

  return w;
}

void UserPages::Info::loadPagePicture(const Licq::User* u)
{
  //FIXME: other protocols
  if (u != NULL && u->GetPicturePresent())
    m_sFilename = QString::fromLocal8Bit(u->pictureFileName().c_str());
  else
    m_sFilename = QString::null;

  QMovie* m = NULL;
  QString s = tr("Not Available");
  if (!m_sFilename.isNull())
  {
    m = new QMovie(m_sFilename, QByteArray(), this);
    if (!m->isValid())
    {
      delete m;
      m = NULL;
      s = tr("Failed to Load");
    }
  }

  if (m_bOwner)
    myPictureClearButton->setEnabled(!m_sFilename.isNull());

  if (m == NULL)
    lblPicture->setText(s);
  else
  {
    lblPicture->setMovie(m);
    if (m->frameCount() > 1)
      m->start();
    else
      m->jumpToNextFrame();
  }
}

void UserPages::Info::clearPicture()
{
  loadPagePicture(NULL);
}

void UserPages::Info::savePagePicture(Licq::User* u)
{
  // Only owner can set his picture
  if (!m_bOwner) return;

  Licq::Owner* o = dynamic_cast<Licq::Owner*>(u);
  if (m_sFilename.isEmpty())
    o->SetPicture(NULL);
  else
    o->SetPicture(m_sFilename.toLatin1());
}

QWidget* UserPages::Info::createPageCounters(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  myPageCountersLayout = new QVBoxLayout(w);
  myPageCountersLayout->setContentsMargins(0, 0, 0, 0);

  unsigned short CR = 0;

  myCountersBox = new QGroupBox(tr("Last"));
  QGridLayout* lay = new QGridLayout(myCountersBox);

  lay->addWidget(new QLabel(tr("Last Online:")), CR, 0);
  nfoLastOnline = new InfoField(true);
  lay->addWidget(nfoLastOnline, CR, 1);

  lay->addWidget(new QLabel(tr("Last Sent Event:")), ++CR, 0);
  nfoLastSent = new InfoField(true);
  lay->addWidget(nfoLastSent, CR, 1);

  lay->addWidget(new QLabel(tr("Last Received Event:")), ++CR, 0);
  nfoLastRecv = new InfoField(true);
  lay->addWidget(nfoLastRecv, CR, 1);

  lay->addWidget(new QLabel(tr("Last Checked Auto Response:")), ++CR, 0);
  nfoLastCheckedAR = new InfoField(true);
  lay->addWidget(nfoLastCheckedAR, CR, 1);

  lay->addWidget(new QLabel(tr("Online Since:")), ++CR, 0);
  nfoOnlineSince = new InfoField(true);
  lay->addWidget(nfoOnlineSince, CR, 1);

  lay->addWidget(new QLabel(tr("Registration Date:")), ++CR, 0);
  nfoRegDate = new InfoField(true);
  lay->addWidget(nfoRegDate, CR, 1);

  lay->setRowStretch(++CR, 5);

  myPageCountersLayout->addWidget(myCountersBox);
  myPageCountersLayout->addStretch(1);

  return w;
}

void UserPages::Info::loadPageCounters(const Licq::User* u)
{
  if (u->isOnline())
    nfoLastOnline->setText(tr("Now"));
  else
    nfoLastOnline->setDateTime(u->LastOnline());

  nfoLastSent->setDateTime(u->LastSentEvent());
  nfoLastRecv->setDateTime(u->LastReceivedEvent());
  nfoLastCheckedAR->setDateTime(u->LastCheckedAutoResponse());
  nfoRegDate->setDateTime(u->RegisteredTime());

  if (u->isOnline())
    nfoOnlineSince->setDateTime(u->OnlineSince());
  else
    nfoOnlineSince->setText(tr("Offline"));
}

#ifdef USE_KABC
QWidget* UserPages::Info::createPageKabc(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  myPageKabcLayout = new QVBoxLayout(w);
  myPageKabcLayout->setContentsMargins(0, 0, 0, 0);

  myKabcBox = new QGroupBox(tr("KDE Adress Book"));
  QGridLayout* lay = new QGridLayout(myKabcBox);

  lay->addWidget(new QLabel(tr("Name:")), 0, 0);
  nfoKABCName = new InfoField(true);
  lay->addWidget(nfoKABCName, 0, 1);

  lay->addWidget(new QLabel(tr("Email:")), 1, 0);
  nfoKABCEmail = new InfoField(true);
  lay->addWidget(nfoKABCEmail, 1, 1);

  myKabcBrowseButton = new QPushButton(tr("Browse..."));
  connect(myKabcBrowseButton, SIGNAL(clicked()), SLOT(browseKabc()));
  lay->addWidget(myKabcBrowseButton, 2, 0, 1, 2);

  myPageKabcLayout->addWidget(myKabcBox);
  myPageKabcLayout->addStretch(1);

  return w;
}

void UserPages::Info::loadPageKabc(const Licq::User* u)
{
  if (m_kabcID.isEmpty())
    m_kabcID = mainwin->kdeIMInterface->kabcIDForUser(myId.toLatin1(), myPpid);

  if (!m_kabcID.isEmpty())
  {
    KABC::AddressBook* adrBook = KABC::StdAddressBook::self();
    if (adrBook == 0)
      return;

    KABC::Addressee contact = adrBook->findByUid(m_kabcID);
    if (!contact.isEmpty())
    {
      nfoKABCName->setText(contact.assembledName());
      QString email = contact.preferredEmail();
      nfoKABCEmail->setText(email);
    }
  }
}

void UserPages::Info::browseKabc()
{
    KABC::Addressee contact = KABC::AddresseeDialog::getAddressee(this);
    if (!contact.isEmpty())
    {
      nfoKABCName->setText(contact.assembledName());
      QString email = contact.preferredEmail();
      nfoKABCEmail->setText(email);
      m_kabcID = contact.uid();
    }
}

void UserPages::Info::savePageKabc()
{
  mainwin->kdeIMInterface->setKABCIDForUser(myId, myPpid, m_kabcID);
}
#endif

void UserPages::Info::editCategory(QTreeWidgetItem* selected)
{
  //undo the effect of double click
  selected->setExpanded(!selected->isExpanded());
  //at the end of this, selected points at an item at the top level of the list
  while (selected->parent() != NULL)
    selected = selected->parent();

  EditCategoryDlg* ecd;

  if (selected == lviMore2Top[Licq::CAT_INTERESTS])
    ecd = new EditCategoryDlg(Licq::CAT_INTERESTS, myInterests, dynamic_cast<UserDlg*>(parent()));
  else if (selected == lviMore2Top[Licq::CAT_ORGANIZATION])
    ecd = new EditCategoryDlg(Licq::CAT_ORGANIZATION, myOrganizations, dynamic_cast<UserDlg*>(parent()));
  else if (selected == lviMore2Top[Licq::CAT_BACKGROUND])
    ecd = new EditCategoryDlg(Licq::CAT_BACKGROUND, myBackgrounds, dynamic_cast<UserDlg*>(parent()));
  else
    return;

  connect(ecd, SIGNAL(updated(Licq::UserCat, const Licq::UserCategoryMap&)),
      SLOT(setCategory(Licq::UserCat, const Licq::UserCategoryMap&)));
}

void UserPages::Info::setCategory(Licq::UserCat cat, const Licq::UserCategoryMap& category)
{
  switch (cat)
  {
    case Licq::CAT_INTERESTS:
      myInterests = category;
      break;
    case Licq::CAT_ORGANIZATION:
      myOrganizations = category;
      break;
    case Licq::CAT_BACKGROUND:
      myBackgrounds = category;
    break;
  default:
    return;
  }

  updateMore2Info(cat, category);
}

void UserPages::Info::phoneBookUpdated(struct Licq::PhoneBookEntry pbe, int entryNum)
{
  // FIXME implement this
  pbe.nActive = 0;
  pbe.nPublish = Licq::PUBLISH_DISABLE;

  if (entryNum == -1)
    m_PhoneBook->AddEntry(&pbe);
  else
    m_PhoneBook->SetEntry(&pbe, entryNum);

  updatePhoneBook();
}

void UserPages::Info::editPhoneEntry(QTreeWidgetItem* selected)
{
  unsigned long nSelection = lsvPhoneBook->indexOfTopLevelItem(selected);

  const struct Licq::PhoneBookEntry* entry;
  m_PhoneBook->Get(nSelection, &entry);

  EditPhoneDlg* epd = new EditPhoneDlg(dynamic_cast<UserDlg*>(parent()), entry, nSelection);
  connect(epd, SIGNAL(updated(struct Licq::PhoneBookEntry, int)),
      SLOT(phoneBookUpdated(struct Licq::PhoneBookEntry, int)));
  epd->show();
}

void UserPages::Info::changeActivePhone(int index)
{
  m_PhoneBook->SetActive(index - 1);

  updatePhoneBook();
}

unsigned long UserPages::Info::retrieve(UserDlg::UserPage page)
{
  if (page == UserDlg::CountersPage || page == UserDlg::KabcPage)
    return 0;

  unsigned status;
  {
    Licq::OwnerReadGuard o(myPpid);
    if (!o.isLocked())
      return 0;
    status = o->status();
  }

  if(status == Licq::User::OfflineStatus)
  {
    InformUser(dynamic_cast<UserDlg*>(parent()),
        tr("You need to be connected to the\nICQ Network to retrieve your settings."));
    return 0;
  }

    //TODO change in the daemon to support other protocols
  if (page == UserDlg::GeneralPage)
  {
      // Before retrieving the meta data we have to
      // save current status of "chkKeepAliasOnUpdate"
      // and the alias
    Licq::UserWriteGuard u(myUserId);
    if (!u.isLocked())
      return 0;
    u->SetEnableSave(false);
    u->setAlias(nfoAlias->text().toUtf8().data());
    if (!m_bOwner)
      u->SetKeepAliasOnUpdate(chkKeepAliasOnUpdate->isChecked());
    u->SetEnableSave(true);
    u->save(Licq::User::SaveUserInfo);
  }

  unsigned long icqEventTag;
  if (page == UserDlg::PhonePage)
  {
    bool bSendServer;
    {
      Licq::UserReadGuard u(myUserId);
      if (!u.isLocked())
        return 0;
      bSendServer = (u->infoSocketDesc() < 0);
    }
    icqEventTag = gLicqDaemon->icqRequestPhoneBook(myUserId, bSendServer);
  }
  else if (page == UserDlg::PicturePage)
  {
    icqEventTag = gProtocolManager.requestUserPicture(myUserId);
  }
  else
  {
    icqEventTag = gProtocolManager.requestUserInfo(myUserId);
  }

  return icqEventTag;
}

unsigned long UserPages::Info::send(UserDlg::UserPage page)
{
  unsigned status;
  Licq::UserId ownerId;

  {
    OwnerWriteGuard owner(myPpid);
    if (!owner.isLocked())
      return 0;
    status = owner->status();
    ownerId = owner->id();

    // Owner info is read from owner so make sure it's updated
    if (page == UserDlg::GeneralPage)
      savePageGeneral(*owner);
  }

    if (status == Licq::User::OfflineStatus)
    {
    InformUser(dynamic_cast<UserDlg*>(parent()),
        tr("You need to be connected to the\nICQ Network to change your settings."));
    return 0;
  }

  unsigned short cc, i, occupation;
  unsigned long icqEventTag = 0;

  switch (page)
  {
    case UserDlg::GeneralPage:
      if (myPpid == LICQ_PPID)
        gLicqDaemon->icqSetEmailInfo(
            codec->fromUnicode(nfoEmailSecondary->text()).data(),
            codec->fromUnicode(nfoEmailOld->text()).data());

      icqEventTag = gProtocolManager.updateOwnerInfo(ownerId);
      break;

    case UserDlg::MorePage:
    icqEventTag = gLicqDaemon->icqSetMoreInfo(
        nfoAge->text().toUShort(),
        cmbGender->currentIndex(),
          nfoHomepage->text().toLocal8Bit().constData(),
        spnBirthYear->value(),
        spnBirthMonth->value(),
        spnBirthDay->value(),
        GetLanguageByIndex(cmbLanguage[0]->currentIndex())->nCode,
        GetLanguageByIndex(cmbLanguage[1]->currentIndex())->nCode,
        GetLanguageByIndex(cmbLanguage[2]->currentIndex())->nCode);
  break;

    case UserDlg::More2Page:
      gLicqDaemon->icqSetInterestsInfo(myInterests);
      icqEventTag = gLicqDaemon->icqSetOrgBackInfo(myOrganizations, myBackgrounds);
      break;

    case UserDlg::WorkPage:
    i = cmbCompanyCountry->currentIndex();
    cc = GetCountryByIndex(i)->nCode;
    i = cmbCompanyOccupation->currentIndex();
    occupation = GetOccupationByIndex(i)->nCode;
    icqEventTag = gLicqDaemon->icqSetWorkInfo(
          codec->fromUnicode(nfoCompanyCity->text()).data(),
          codec->fromUnicode(nfoCompanyState->text()).data(),
          codec->fromUnicode(nfoCompanyPhone->text()).data(),
          codec->fromUnicode(nfoCompanyFax->text()).data(),
          codec->fromUnicode(nfoCompanyAddress->text()).data(),
          codec->fromUnicode(nfoCompanyZip->text()).data(),
        cc,
          codec->fromUnicode(nfoCompanyName->text()).data(),
          codec->fromUnicode(nfoCompanyDepartment->text()).data(),
          codec->fromUnicode(nfoCompanyPosition->text()).data(),
        occupation,
          nfoCompanyHomepage->text().toLocal8Bit().constData());
  break;

    case UserDlg::AboutPage:
    icqEventTag = gLicqDaemon->icqSetAbout(
          codec->fromUnicode(mlvAbout->toPlainText()).data());
    break;

    case UserDlg::PhonePage:
    {
      {
        Licq::OwnerWriteGuard o(myPpid);
        savePagePhoneBook(*o);
      }
      gLicqDaemon->icqUpdatePhoneBookTimestamp();
      icqEventTag = 0;
      break;
    }
    case UserDlg::PicturePage:
    {
      {
        Licq::OwnerWriteGuard o(myPpid);
        savePagePicture(*o);
      }
      gLicqDaemon->icqUpdatePictureTimestamp();
      icqEventTag = 0;
      break;
    }
    default:
      icqEventTag = 0;
  }

  return icqEventTag;
}

void UserPages::Info::addPhone()
{
    EditPhoneDlg* epd = new EditPhoneDlg(dynamic_cast<UserDlg*>(parent()));
  connect(epd, SIGNAL(updated(struct Licq::PhoneBookEntry, int)),
      SLOT(phoneBookUpdated(struct Licq::PhoneBookEntry, int)));
    epd->show();
}

void UserPages::Info::browsePicture()
{
    QString Filename;

    do
    {
#ifdef USE_KDE
      Filename = KFileDialog::getOpenFileName(KUrl(),
        "Images (*.bmp *.jpg *.jpeg *.jpe *.gif)",
        dynamic_cast<UserDlg*>(parent()),
          tr("Select your picture"));
#else
    Filename = QFileDialog::getOpenFileName(
        dynamic_cast<UserDlg*>(parent()),
          tr("Select your picture"),
          QString::null,
          "Images (*.bmp *.jpg *.jpeg *.jpe *.gif)");
#endif
      if (Filename.isNull())
        break;

      QFile file(Filename);
      if (file.size() <= Licq::MAX_PICTURE_SIZE)
        break;

      QString msg = Filename + tr(" is over %1 bytes.\nSelect another picture?")
        .arg(Licq::MAX_PICTURE_SIZE);
    if (!QueryYesNo(dynamic_cast<UserDlg*>(parent()), msg))
    {
        Filename = QString::null;
        break;
      }
    } while (1);

    if (Filename.isNull())
    return;

    m_sFilename = Filename;
    myPictureClearButton->setEnabled(true);
    QPixmap p;
    QString s = tr("Not Available");
    if (!p.load(Filename))
    {
      Licq::gLog.warning("Failed to load user picture, did you forget to compile GIF"
                      " support?");
      s = tr("Failed to Load");
    }

    if (p.isNull())
      lblPicture->setText(s);
    else
      lblPicture->setPixmap(p);
}

void UserPages::Info::userUpdated(const Licq::User* user, unsigned long subSignal)
{
  switch (subSignal)
  {
    case Licq::PluginSignal::UserInfo:
      if (myPpid == LICQ_PPID)
      {
        loadPageMore(user);
        loadPageMore2(user);
        loadPageWork(user);
        loadPageAbout(user);
        loadPagePhoneBook(user);
      }
      // fall through
    case Licq::PluginSignal::UserBasic:
      loadPageGeneral(user);
      break;
    case Licq::PluginSignal::UserPicture:
      loadPagePicture(user);
    break;
  }
}

void UserPages::Info::aliasChanged()
{
  chkKeepAliasOnUpdate->setChecked(true);
}
