// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2009 Licq developers
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

#include <qwidget.h>

#include "licq_history.h"
#include <licq_types.h>

#include "mainwin.h"
#include "catdlg.h"
#include "phonedlg.h"

class QSpinBox;
class QLabel;
class QCheckBox;
class QPushButton;
class QTabWidget;
class QProgressBar;
class QListView;
class QListViewItem;

class CInfoField;
class CICQDaemon;
class LicqEvent;
class CSignalManager;
class CMainWindow;
class CEInfoField;
class CEComboBox;
class LicqUser;

class UserInfoDlg : public QWidget
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
    HistoryInfo,
    LastCountersInfo,
#ifdef USE_KDE
    KABCInfo,
#endif
    InfoTabCount
  };

  UserInfoDlg(CICQDaemon *s, CSignalManager *theSigMan, CMainWindow *m,
      const UserId& userId, QWidget *parent = 0);
  virtual ~UserInfoDlg();

  const UserId& userId() const { return myUserId; }
  void showTab(int);
  bool isTabShown(int);
  void retrieveSettings() { slotRetrieve(); }

protected:
  // common stuff
  struct {
    QString label;
    QWidget *tab;
    bool loaded;
  } tabList[InfoTabCount];
  bool m_bOwner;
  int currentTab;
  UserId myUserId;
  QString m_sProgressMsg;
  QString m_sBasic;
  CICQDaemon *server;
  CMainWindow *mainwin;
  CSignalManager *sigman;
  unsigned long icqEventTag;
  QTabWidget* tabs;
  QPushButton* btnMain1, *btnMain2, *btnMain3, *btnMain4;

  // User Info tab
  void CreateGeneralInfo();
  void InitGeneralInfo();
  CInfoField *nfoFirstName, *nfoLastName, *nfoEmailPrimary, *nfoEmailSecondary, *nfoEmailOld,
             *nfoAlias, *nfoIp, *nfoUin, *nfoCity, *nfoState,
             *nfoZipCode, *nfoAddress, *nfoCountry, *nfoFax, *nfoCellular,
             *nfoPhone, *nfoStatus;
  QCheckBox *chkKeepAliasOnUpdate;
  CEComboBox *cmbCountry;
  QLabel *lblAuth, *lblICQHomepage;
  CTimeZoneField *tznZone;

  // More info
  void CreateMoreInfo();
  CInfoField *nfoAge, *nfoBirthday, *nfoLanguage[3], *nfoHomepage,
             *nfoGender;
  QListView *lvHomepageCategory;
  MLEditWrap *mleHomepageDesc;
  CEComboBox *cmbLanguage[3], *cmbGender;
  QSpinBox *spnBirthDay, *spnBirthMonth, *spnBirthYear;

  // More2 info
  void CreateMore2Info();
  static int SplitCategory(QListViewItem *parent, QTextCodec *codec,
                           const char *descr);
  QListView *lsvMore2;
  QListViewItem *lviMore2Top[3];

  // Work info
  void CreateWorkInfo();
  CInfoField *nfoCompanyName, *nfoCompanyCity, *nfoCompanyState,
             *nfoCompanyAddress, *nfoCompanyZip, *nfoCompanyCountry, *nfoCompanyPhone, *nfoCompanyFax,
             *nfoCompanyHomepage, *nfoCompanyPosition, *nfoCompanyDepartment, *nfoCompanyOccupation;
  CEComboBox *cmbCompanyCountry, *cmbCompanyOccupation;

  // About
  void CreateAbout();
  QLabel *lblAbout;
  MLView  *mlvAbout;

  // PhoneBook
  void CreatePhoneBook();
  QListView *lsvPhoneBook;
  QComboBox *cmbActive;
  CInfoField *nfoActive;

  // Picture
  void CreatePicture();
  QLabel *lblPicture;
  QString m_sFilename;

  // Last Counters
  void CreateLastCountersInfo();
  CInfoField *nfoLastOnline, *nfoLastSent, *nfoLastRecv, *nfoLastCheckedAR, *nfoOnlineSince, *nfoRegDate;

  // KDE AddressBook
  void CreateKABCInfo();
  CInfoField *nfoKABCName;
  CInfoField *nfoKABCEmail;
  QString m_kabcID;
  
  // History tab
  void CreateHistory();
  void SetupHistory();
  void SaveHistory();
  CInfoField *nfoHistory;
  CMessageViewWidget *mlvHistory;
  QLabel *lblHistory, *lblFilter;
  QLineEdit* lneFilter;
  QProgressBar* barFiltering;
  QCheckBox *chkHistoryReverse;
  HistoryList m_lHistoryList;
  HistoryListIter m_iHistorySIter;
  HistoryListIter m_iHistoryEIter;
  bool m_bHistoryReverse;
  unsigned short m_nHistoryIndex, m_nHistoryShowing;

  void SetGeneralInfo(const LicqUser* u);
  void SetMoreInfo(const LicqUser* u);
  void SetMore2Info(const LicqUser* u);
  void UpdateMore2Info(QTextCodec* codec, UserCat cat, const UserCategoryMap& category);
  void SetWorkInfo(const LicqUser*u);
  void SetAbout(const LicqUser* u);
  void SetPhoneBook(const LicqUser* u);
  void UpdatePhoneBook(QTextCodec *);
  void SetPicture(const LicqUser* u);
  void SetLastCountersInfo(const LicqUser* u);
  void SetKABCInfo(const LicqUser* u);
  void UpdateKABCInfo();
  void SaveGeneralInfo();
  void SaveMoreInfo();
  void SaveMore2Info();
  void SaveWorkInfo();
  void SaveAbout();
  void SavePhoneBook();
  void SavePicture();
  void SaveKABCInfo();
  
  virtual void keyPressEvent(QKeyEvent*);

protected slots:
  void ShowHistory();
  void ShowHistoryPrev();
  void ShowHistoryNext();
  void HistoryReverse(bool);
  void HistoryReload();
  void updateTab(const QString&);
  void updatedUser(const UserId& userId, unsigned long subSignal);
  void SaveSettings();
  void slotUpdate();
  void slotRetrieve();
  void doneFunction(LicqEvent*);
  void slot_aliasChanged(const QString &);
  void resetCaption();
  void ShowUsermenu() { gMainWindow->SetUserMenuUser(myUserId); }
  void slot_showHistoryTimer();
  void EditCategory(QListViewItem *selected);
  void setCategory(UserCat cat, const UserCategoryMap& category);
  void PhoneBookUpdated(struct PhoneBookEntry pbe, int nEntry);
  void EditPhoneEntry(QListViewItem *selected);
  void ChangeActivePhone(int index);

signals:
  void finished(const UserId& userId);

private:
  static bool chkContains(const char* text, const char* filter, int filterlen);
  QTimer *timer;
  UserCategoryMap myInterests;
  UserCategoryMap myBackgrounds;
  UserCategoryMap myOrganizations;
  ICQUserPhoneBook *m_PhoneBook;
};

#endif
