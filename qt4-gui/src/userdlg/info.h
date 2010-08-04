// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2010 Licq developers
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

#ifndef USERDLG_INFO_H
#define USERDLG_INFO_H

#include <config.h>

#include <QObject>

#include <licq/contactlist/user.h>

#include "userdlg.h"

class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QPushButton;
class QSpinBox;
class QTreeWidget;
class QTreeWidgetItem;
class QVBoxLayout;


namespace LicqQtGui
{
class InfoField;
class MLEdit;
class MLView;
class TimeZoneEdit;
class UserDlg;

namespace UserPages
{

class Info : public QObject
{
  Q_OBJECT

public:
  /**
   * Constructor
   *
   * @param isOwner True if the user is a protocol owner
   * @param protocolId Protocol to create info pages for
   * @param parent User dialog that is parent
   */
  Info(bool isOwner, unsigned long protocolId, UserDlg* parent);

  /**
   * Destructor
   */
  virtual ~Info() {}

  /**
   * Load user data for pages
   *
   * @param user User to get data from
   */
  void load(const Licq::User* user);

  /**
   * Save user data for pages
   *
   * @param user User to write data to
   */
  void apply(Licq::User* user);

  /**
   * Save user data for pages
   * Call this function without user locked
   *
   * @param userId User id
   */
  void apply2(const Licq::UserId& userId);

  /**
   * User was updated
   *
   * @param user User locked for read access
   * @param subSignal Sub signal telling what the change was
   */
  void userUpdated(const Licq::User* user, unsigned long subSignal);

  /**
   * Retrieve info from server
   *
   * @param page to get info for
   * @return Icq event tag for retrieve request
   */
  unsigned long retrieve(UserDlg::UserPage page);

  /**
   * Send info to server
   * Only called if dialog is for owner
   *
   * @param page to send info for
   * @return Icq event tag for send request
   */
  unsigned long send(UserDlg::UserPage page);

private slots:
  /**
   * User alias field has been changed
   */
  void aliasChanged();

  /**
   * Open dialog to edit interest, organization or background
   *
   * @param selected Item to edit
   */
  void editCategory(QTreeWidgetItem* selected);

  /**
   * Set interest, organization or background after editing dialog has finished
   *
   * @param cat Category to update
   * @param category New category map
   */
  void setCategory(Licq::UserCat cat, const Licq::UserCategoryMap& category);

  /**
   * Add/update phone book entry after editing dialog has finished
   *
   * @param pbe Data for entry
   * @param nEntry Position in list to update
   */
  void phoneBookUpdated(struct Licq::PhoneBookEntry pbe, int nEntry);

  /**
   * Open dialog to edit phone entry
   *
   * @param selected Phone entry to edit
   */
  void editPhoneEntry(QTreeWidgetItem* selected);

  /**
   * Set active phone
   *
   * @param index Position in list to make active
   */
  void changeActivePhone(int index);

  /**
   * Remove entry from phone book
   */
  void clearPhone();

  /**
   * Add entry to phone book
   */
  void addPhone();

  /**
   * Browse for a file to use as owner picture
   */
  void browsePicture();

  /**
   * Clear owner picture
   */
  void clearPicture();

#ifdef USE_KABC
  /**
   * Open dialog to browse for KDE address book entry
   */
  void browseKabc();
#endif

private:
  /**
   * Setup the general page.
   *
   * @param parent Parent dialog
   * @return a widget with the general info
   */
  QWidget* createPageGeneral(QWidget* parent);

  /**
   * Load user data for general page
   *
   * @param user User to get data from
   */
  void loadPageGeneral(const Licq::User* user);

  /**
   * Save user data for general page
   *
   * @param user User to write data to
   */
  void savePageGeneral(Licq::User* user);

  /**
   * Setup the more page
   *
   * @param parent Parent dialog
   * @return A widget with the more info
   */
  QWidget* createPageMore(QWidget* parent);

  /**
   * Load user data for more page
   *
   * @param user User to get data from
   */
  void loadPageMore(const Licq::User* user);

  /**
   * Save user data for more page
   *
   * @param user User to write data to
   */
  void savePageMore(Licq::User* user);

  /**
   * Setup the more2 page
   *
   * @param parent Parent dialog
   * @return A widget with the more2 info
   */
  QWidget* createPageMore2(QWidget* parent);

  /**
   * Load user data for more2 page
   *
   * @param user User to get data from
   */
  void loadPageMore2(const Licq::User* user);

  /**
   * Save user data for more2 page
   *
   * @param user User to write data to
   */
  void savePageMore2(Licq::User* user);

  /**
   * Split interest, organization or background string for presentation
   *
   * @param parent Parent widget item to add items to
   * @param descr Description string to split
   * @return 0 on success or -1 on failure
   */
  int splitCategory(QTreeWidgetItem* parent, const char* descr);

  /**
   * Present interest, organization or background data
   *
   * @param cat Category to update
   * @param category New category map
   */
  void updateMore2Info(Licq::UserCat cat, const Licq::UserCategoryMap& category);

  /**
   * Setup the work page.
   *
   * @param parent Parent dialog
   * @return a widget with the work info
   */
  QWidget* createPageWork(QWidget* parent);

  /**
   * Load user data for work page
   *
   * @param user User to get data from
   */
  void loadPageWork(const Licq::User* user);

  /**
   * Save user data for work page
   *
   * @param user User to write data to
   */
  void savePageWork(Licq::User* user);

  /**
   * Setup the about page.
   *
   * @param parent Parent dialog
   * @return a widget with the about info
   */
  QWidget* createPageAbout(QWidget* parent);

  /**
   * Load user data for about page
   *
   * @param user User to get data from
   */
  void loadPageAbout(const Licq::User* user);

  /**
   * Save user data for about page
   *
   * @param user User to write data to
   */
  void savePageAbout(Licq::User* user);

  /**
   * Setup the phone book page.
   *
   * @param parent Parent dialog
   * @return a widget with the phone book
   */
  QWidget* createPagePhoneBook(QWidget* parent);

  /**
   * Load user data for phone book
   *
   * @param user User to get data from
   */
  void loadPagePhoneBook(const Licq::User* user);

  /**
   * Save user data for phone book
   *
   * @param user User to write data to
   */
  void savePagePhoneBook(Licq::User* user);

  /**
   * Update data in phone book widget
   */
  void updatePhoneBook();

  /**
   * Setup the picture page.
   *
   * @param parent Parent dialog
   * @return a widget with the picture
   */
  QWidget* createPagePicture(QWidget* parent);

  /**
   * Load user data for picture page
   *
   * @param user User to get data from
   */
  void loadPagePicture(const Licq::User* user);

  /**
   * Save user data for picture page
   *
   * @param user User to write data to
   */
  void savePagePicture(Licq::User* user);

  /**
   * Setup the counters page.
   *
   * @param parent Parent dialog
   * @return a widget with the last counters
   */
  QWidget* createPageCounters(QWidget* parent);

  /**
   * Load user data for counters page
   *
   * @param user User to get data from
   */
  void loadPageCounters(const Licq::User* user);

#ifdef USE_KABC
  /**
   * Setup the KDE adressbook page.
   *
   * @param parent Parent dialog
   * @return a widget with the KDE adressbook
   */
  QWidget* createPageKabc(QWidget* parent);

  /**
   * Load user data for KDE adressbook page
   *
   * @param user User to get data from
   */
  void loadPageKabc(const Licq::User* user);

  /**
   * Save user data for KDE adressbook page
   */
  void savePageKabc();
#endif

  Licq::UserId myUserId;
  QString myId;
  unsigned long myPpid;
  bool m_bOwner;
  const QTextCodec* codec;

  // General info
  QVBoxLayout* myPageGeneralLayout;
  QGroupBox* myGeneralBox;
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
  bool myAliasHasChanged;

  // More info
  QVBoxLayout* myPageMoreLayout;
  QGroupBox* myMoreBox;
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
  QVBoxLayout* myPageMore2Layout;
  QGroupBox* myMore2Box;
  QTreeWidget* lsvMore2;
  QTreeWidgetItem* lviMore2Top[3];
  Licq::UserCategoryMap myInterests;
  Licq::UserCategoryMap myBackgrounds;
  Licq::UserCategoryMap myOrganizations;

  // Work info
  QVBoxLayout* myPageWorkLayout;
  QGroupBox* myWorkBox;
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
  QVBoxLayout* myPageAboutLayout;
  QGroupBox* myAboutBox;
  QLabel* lblAbout;
  MLView* mlvAbout;

  // PhoneBook
  QVBoxLayout* myPagePhoneBookLayout;
  QGroupBox* myPhoneBookBox;
  QTreeWidget* lsvPhoneBook;
  QComboBox* cmbActive;
  InfoField* nfoActive;
  QPushButton* myPhoneAddButton;
  QPushButton* myPhoneClearButton;
  Licq::ICQUserPhoneBook* m_PhoneBook;

  // Picture
  QVBoxLayout* myPagePictureLayout;
  QGroupBox* myPictureBox;
  QPushButton* myPictureBrowseButton;
  QPushButton* myPictureClearButton;
  QLabel* lblPicture;
  QString m_sFilename;

  // Last Counters
  QVBoxLayout* myPageCountersLayout;
  QGroupBox* myCountersBox;
  InfoField* nfoLastOnline;
  InfoField* nfoLastSent;
  InfoField* nfoLastRecv;
  InfoField* nfoLastCheckedAR;
  InfoField* nfoOnlineSince;
  InfoField* nfoRegDate;

#ifdef USE_KABC
  // KDE AddressBook
  QVBoxLayout* myPageKabcLayout;
  QGroupBox* myKabcBox;
  InfoField* nfoKABCName;
  InfoField* nfoKABCEmail;
  QPushButton* myKabcBrowseButton;
  QString m_kabcID;
#endif
};

} // namespace UserPages
} // namespace LicqQtGui

#endif
