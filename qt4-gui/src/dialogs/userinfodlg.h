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

#ifndef USERINFODLG_H
#define USERINFODLG_H

#include <QDialog>

#include <licq_user.h>

class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;
class QSpinBox;
class QTreeWidget;
class QTreeWidgetItem;

class CICQSignal;
class ICQEvent;

namespace LicqQtGui
{
class InfoField;
class MLEdit;
class MLView;
class TabWidget;
class TimeZoneEdit;

class UserInfoDlg : public QDialog
{
  Q_OBJECT
public:
  enum Tab {
    GeneralInfo,
    MoreInfo,
    More2Info,
    WorkInfo,
    AboutInfo,
    PhoneInfo,
    PictureInfo,
    LastCountersInfo,
#ifdef USE_KDE
    KABCInfo,
#endif
    InfoTabCount
  };

  UserInfoDlg(QString id, unsigned long ppid, QWidget* parent = 0);
  virtual ~UserInfoDlg();

  QString Id() const { return myId; }
  unsigned long PPID() const { return m_nPPID; }
  void showTab(int);
  bool isTabShown(int);
  void retrieveSettings() { slotRetrieve(); }

signals:
  void finished(UserInfoDlg* dialog);

private:
  // common stuff
  struct {
    QString label;
    QWidget* tab;
    bool loaded;
  } tabList[InfoTabCount];
  bool m_bOwner;
  int currentTab;
  QString myId;
  unsigned long m_nPPID;
  unsigned long icqEventTag;
  QString m_sProgressMsg;
  QString m_sBasic;
  TabWidget* tabs;
  QPushButton* btnMain1;
  QPushButton* btnMain2;
  QPushButton* btnMain3;
  QPushButton* btnMain4;

  // User Info tab
  void CreateGeneralInfo();
  void InitGeneralInfo();
  InfoField* nfoFirstName;
  InfoField* nfoLastName;
  InfoField* nfoEmailPrimary;
  InfoField* nfoEmailSecondary;
  InfoField* nfoEmailOld;
  InfoField* nfoAlias;
  InfoField* nfoIp;
  InfoField* nfoUin;
  InfoField* nfoCity;
  InfoField* nfoState;
  InfoField* nfoZipCode;
  InfoField* nfoAddress;
  InfoField* nfoCountry;
  InfoField* nfoFax;
  InfoField* nfoCellular;
  InfoField* nfoPhone;
  InfoField* nfoStatus;
  QCheckBox* chkKeepAliasOnUpdate;
  QComboBox* cmbCountry;
  QLabel* lblAuth;
  QLabel* lblICQHomepage;
  TimeZoneEdit* tznZone;

  // More info
  void CreateMoreInfo();
  InfoField* nfoAge;
  InfoField* nfoBirthday;
  InfoField* nfoLanguage[3];
  InfoField* nfoHomepage;
  InfoField* nfoGender;
  QTreeWidget* lvHomepageCategory;
  MLEdit* mleHomepageDesc;
  QComboBox* cmbLanguage[3];
  QComboBox* cmbGender;
  QSpinBox* spnBirthDay;
  QSpinBox* spnBirthMonth;
  QSpinBox* spnBirthYear;

  // More2 info
  void CreateMore2Info();
  static int SplitCategory(QTreeWidgetItem* parent, QTextCodec* codec,
      const char* descr);
  QTreeWidget* lsvMore2;
  QTreeWidgetItem* lviMore2Top[3];

  // Work info
  void CreateWorkInfo();
  InfoField* nfoCompanyName;
  InfoField* nfoCompanyCity;
  InfoField* nfoCompanyState;
  InfoField* nfoCompanyAddress;
  InfoField* nfoCompanyZip;
  InfoField* nfoCompanyCountry;
  InfoField* nfoCompanyPhone;
  InfoField* nfoCompanyFax;
  InfoField* nfoCompanyHomepage;
  InfoField* nfoCompanyPosition;
  InfoField* nfoCompanyDepartment;
  InfoField* nfoCompanyOccupation;
  QComboBox* cmbCompanyCountry;
  QComboBox* cmbCompanyOccupation;

  // About
  void CreateAbout();
  QLabel* lblAbout;
  MLView* mlvAbout;

  // PhoneBook
  void CreatePhoneBook();
  QTreeWidget* lsvPhoneBook;
  QComboBox* cmbActive;
  InfoField* nfoActive;

  // Picture
  void CreatePicture();
  QLabel* lblPicture;
  QString m_sFilename;

  // Last Counters
  void CreateLastCountersInfo();
  InfoField* nfoLastOnline;
  InfoField* nfoLastSent;
  InfoField* nfoLastRecv;
  InfoField* nfoLastCheckedAR;
  InfoField* nfoOnlineSince;
  InfoField* nfoRegDate;

  // KDE AddressBook
  void CreateKABCInfo();
  InfoField* nfoKABCName;
  InfoField* nfoKABCEmail;
  QString m_kabcID;

  void SetGeneralInfo(const ICQUser* u);
  void SetMoreInfo(const ICQUser* u);
  void SetMore2Info(const ICQUser* u);
  void UpdateMore2Info(QTextCodec* codec, const ICQUserCategory* cat);
  void SetWorkInfo(const ICQUser* u);
  void SetAbout(const ICQUser* u);
  void SetPhoneBook(const ICQUser* u);
  void UpdatePhoneBook(QTextCodec* u);
  void SetPicture(const ICQUser* u);
  void SetLastCountersInfo(const ICQUser* u);
  void SetKABCInfo(const ICQUser* u);
  void UpdateKABCInfo();
  void SaveGeneralInfo();
  void SaveMoreInfo();
  void SaveMore2Info();
  void SaveWorkInfo();
  void SaveAbout();
  void SavePhoneBook();
  void SavePicture();
  void SaveKABCInfo();

  static bool chkContains(const char* text, const char* filter, int filterlen);
  QTimer* timer;
  ICQUserCategory* m_Interests;
  ICQUserCategory* m_Backgrounds;
  ICQUserCategory* m_Organizations;
  ICQUserPhoneBook* m_PhoneBook;

private slots:
  void updateTab(int index);
  void updatedUser(CICQSignal*);
  void SaveSettings();
  void slotUpdate();
  void slotRetrieve();
  void doneFunction(ICQEvent*);
  void slot_aliasChanged(const QString&);
  void resetCaption();
  void ShowUsermenu();
  void EditCategory(QTreeWidgetItem* selected);
  void setCategory(ICQUserCategory* cat);
  void PhoneBookUpdated(struct PhoneBookEntry pbe, int nEntry);
  void EditPhoneEntry(QTreeWidgetItem* selected);
  void ChangeActivePhone(int index);
};

} // namespace LicqQtGui

#endif
