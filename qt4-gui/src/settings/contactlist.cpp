// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007 Licq developers
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

#include "contactlist.h"

#include "config.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include <licq_icqd.h>

#include "config/chat.h"
#include "config/contactlist.h"
#include "config/general.h"
#include "config/skin.h"

#include "core/mainwin.h"

#include "settingsdlg.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::Settings::ContactList */

Settings::ContactList::ContactList(SettingsDlg* parent)
  : QObject(parent)
{
  parent->addPage(SettingsDlg::ContactListPage, createPageContactList(parent),
      tr("Contact List"));
  parent->addPage(SettingsDlg::ColumnsPage, createPageColumns(parent),
      tr("Columns"), SettingsDlg::ContactListPage);
  parent->addPage(SettingsDlg::ContactInfoPage, createPageContactInfo(parent),
      tr("Contact Info"), SettingsDlg::ContactListPage);

  load();
}

QWidget* Settings::ContactList::createPageContactList(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  myPageContactListLayout = new QVBoxLayout(w);
  myPageContactListLayout->setContentsMargins(0, 0, 0, 0);

  myAppearanceBox = new QGroupBox(tr("Contact List Appearance"));
  myAppearanceLayout = new QGridLayout(myAppearanceBox);

  myGridLinesCheck = new QCheckBox(tr("Show grid lines"));
  myGridLinesCheck->setToolTip(tr("Draw the box around each square in the user list"));
  myAppearanceLayout->addWidget(myGridLinesCheck, 0, 0);

  myHeaderCheck = new QCheckBox(tr("Show column headers"));
  myHeaderCheck->setToolTip(tr("Turns on or off the display of headers above each column in the user list"));
  myAppearanceLayout->addWidget(myHeaderCheck, 1, 0);

  myShowDividersCheck = new QCheckBox(tr("Show user dividers"));
  myShowDividersCheck->setToolTip(tr("Show the \"--online--\" and \"--offline--\" bars in the contact list"));
  myAppearanceLayout->addWidget(myShowDividersCheck, 2, 0);

  myFontStylesCheck = new QCheckBox(tr("Use font styles"));
  myFontStylesCheck->setToolTip(tr("Use italics and bold in the user list to "
      "indicate special characteristics such as online notify and visible list"));
  myAppearanceLayout->addWidget(myFontStylesCheck, 3, 0);

  myShowExtIconsCheck = new QCheckBox(tr("Show extended icons"));
  myShowExtIconsCheck->setToolTip(tr("Show birthday, invisible, and custom auto response icons to the right of users in the list"));
  myAppearanceLayout->addWidget(myShowExtIconsCheck, 4, 0);

  myShowPhoneIconsCheck = new QCheckBox(tr("Show phone icons"));
  myShowPhoneIconsCheck->setToolTip(tr("Show extended icons for phone statuses"));
  connect(myShowExtIconsCheck, SIGNAL(toggled(bool)),
      myShowPhoneIconsCheck, SLOT(setEnabled(bool)));
  myAppearanceLayout->addWidget(myShowPhoneIconsCheck, 5, 0);

  myShowUserIconsCheck = new QCheckBox(tr("Show user display picture"));
  myShowUserIconsCheck->setToolTip(tr("Show the user's display picture"
      " instead of a status icon, if the user"
      " is online and has a display picture"));
  myAppearanceLayout->addWidget(myShowUserIconsCheck, 6, 0);

  myAlwaysShowONUCheck = new QCheckBox(tr("Always show online notify users"));
  myAlwaysShowONUCheck->setToolTip(tr("Show online notify users who are offline even when offline users are hidden."));
  myAppearanceLayout->addWidget(myAlwaysShowONUCheck, 0, 1);

  myTransparentCheck = new QCheckBox(tr("Transparent when possible"));
  myTransparentCheck->setToolTip(tr("Make the user window transparent when there is no scroll bar"));
  myAppearanceLayout->addWidget(myTransparentCheck, 1, 1);

  myShowGroupIfNoMsgCheck = new QCheckBox(tr("Show group name if no messages"));
  myShowGroupIfNoMsgCheck->setToolTip(tr("Show the name of the current group in the messages label when there are no new messages"));
  myAppearanceLayout->addWidget(myShowGroupIfNoMsgCheck, 2, 1);

  mySysBackCheck = new QCheckBox(tr("Use system background color"));
  myAppearanceLayout->addWidget(mySysBackCheck, 3, 1);

  myScrollBarCheck = new QCheckBox(tr("Allow scroll bar"));
  myScrollBarCheck->setToolTip(tr("Allow the vertical scroll bar in the user list"));
  myAppearanceLayout->addWidget(myScrollBarCheck, 4, 1);

  QHBoxLayout* myFrameStyleLayout = new QHBoxLayout();
  myFrameStyleLabel = new QLabel(tr("Frame style:"));
  myFrameStyleLayout->addWidget(myFrameStyleLabel);
  myFrameStyleLabel->setToolTip(tr("Override the skin setting for the frame style of the user window:\n"
      "   0 (No frame), 1 (Box), 2 (Panel), 3 (WinPanel)\n"
      " + 16 (Plain), 32 (Raised), 48 (Sunken), 240 (Shadow)"));
  myFrameStyleEdit = new QLineEdit();
  myFrameStyleEdit->setValidator(new QIntValidator(myFrameStyleEdit));
  myFrameStyleEdit->setToolTip(myFrameStyleLabel->toolTip());
  myFrameStyleLabel->setBuddy(myFrameStyleEdit);
  myFrameStyleLayout->addWidget(myFrameStyleEdit);
  myAppearanceLayout->addLayout(myFrameStyleLayout, 5, 1);

  // Make the columns evenly wide, otherwise the QLineEdit steals the space
  myAppearanceLayout->setColumnStretch(0, 1);
  myAppearanceLayout->setColumnStretch(1, 1);


  myBehaviourBox = new QGroupBox(tr("Contact List Behaviour"));
  myBehaviourLayout = new QGridLayout(myBehaviourBox);

  mySSListCheck = new QCheckBox(tr("Use server side contact list"));
  mySSListCheck->setToolTip(tr("Store your contacts on the server so they are accessible from different locations and/or programs"));
  myBehaviourLayout->addWidget(mySSListCheck, 0, 0);

  myManualNewUserCheck = new QCheckBox(tr("Manual \"New User\" group handling"));
  myManualNewUserCheck->setToolTip(tr("If not checked, a user will be automatically removed from "
                                  "\"New User\" group when you first send an event to them."));
  myBehaviourLayout->addWidget(myManualNewUserCheck, 1, 0);

  myEnableMainwinMouseMovementCheck = new QCheckBox(tr("Allow dragging main window"));
  myEnableMainwinMouseMovementCheck->setToolTip(tr("Lets you drag around the main window with your mouse"));
  myBehaviourLayout->addWidget(myEnableMainwinMouseMovementCheck, 2, 0);

  myMainWinStickyCheck = new QCheckBox(tr("Sticky main window"));
  myMainWinStickyCheck->setToolTip(tr("Makes the Main window visible on all desktops"));
  myBehaviourLayout->addWidget(myMainWinStickyCheck, 0, 1);

  myDragMovesUserCheck = new QCheckBox(tr("Move users when dragging to groups"));
  myDragMovesUserCheck->setToolTip(tr("If checked a user will be moved when dragged to another group.\n"
      "If not checked user will only be added to the new group."));
  myBehaviourLayout->addWidget(myDragMovesUserCheck, 1, 1);

  QHBoxLayout* mySortByLayout = new QHBoxLayout();
  mySortByLabel = new QLabel(tr("Additional sorting:"));
  mySortByLabel->setToolTip(tr("<b>none:</b> - Don't sort online users by Status<br>\n"
      "<b>status</b> - Sort online users by status<br>\n"
      "<b>status + last event</b> - Sort online users by status and by last event<br>\n"
      "<b>status + new messages</b> - Sort online users by status and number of new messages"));
  mySortByLayout->addWidget(mySortByLabel);
  mySortByCombo = new QComboBox();
  mySortByCombo->addItem(tr("none"), 0);
  mySortByCombo->addItem(tr("status"), 1);
  mySortByCombo->addItem(tr("status + last event"), 2);
  mySortByCombo->addItem(tr("status + new messages"), 3);
  mySortByCombo->setToolTip(mySortByLabel->toolTip());
  mySortByLabel->setBuddy(mySortByCombo);
  mySortByLayout->addWidget(mySortByCombo);
  myBehaviourLayout->addLayout(mySortByLayout, 2, 1);


  myPageContactListLayout->addWidget(myAppearanceBox);
  myPageContactListLayout->addWidget(myBehaviourBox);
  myPageContactListLayout->addStretch(1);

  return w;
}

QWidget* Settings::ContactList::createPageColumns(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  myPageColumnsLayout = new QVBoxLayout(w);
  myPageColumnsLayout->setContentsMargins(0, 0, 0, 0);

  myColumnsBox = new QGroupBox (tr("Column Configuration"));
  myColumnsLayout = new QGridLayout(myColumnsBox);

  // Headings
  myColTitleLabel = new QLabel (tr("Title"));
  myColTitleLabel->setToolTip(tr("The string which will appear in the column header"));
  myColumnsLayout->addWidget(myColTitleLabel, 0, 1);
  myColFormatLabel = new QLabel (tr("Format"));
  myColFormatLabel->setToolTip(tr("The format string used to define what will appear in each column.<br>"
      "The following parameters can be used:")
      + gMainWindow->usprintfHelp);
  myColumnsLayout->addWidget(myColFormatLabel, 0, 2);
  myColWidthLabel = new QLabel (tr("Width"));
  myColWidthLabel->setToolTip(tr("The width of the column"));
  myColumnsLayout->addWidget(myColWidthLabel, 0, 3);
  myColAlignLabel = new QLabel(tr("Alignment"));
  myColAlignLabel->setToolTip(tr("The alignment of the column"));
  myColumnsLayout->addWidget(myColAlignLabel, 0, 4);

  for (unsigned short i = 0; i < MAX_COLUMNCOUNT; i++)
  {
    myColNumberRadio[i] = new QRadioButton(QString::number(i+1));
    myColNumberRadio[i]->setToolTip(tr("Number of columns"));
    connect(myColNumberRadio[i], SIGNAL(toggled(bool)), SLOT(numColumnsChanged()));
    myColumnsLayout->addWidget(myColNumberRadio[i], i+1, 0);

    myColTitleEdit[i] = new QLineEdit();
    myColTitleEdit[i]->setToolTip(myColTitleLabel->toolTip());
    myColumnsLayout->addWidget(myColTitleEdit[i], i+1, 1);

    myColFormatEdit[i] = new QLineEdit();
    myColFormatEdit[i]->setToolTip(myColFormatLabel->toolTip());
    myColumnsLayout->addWidget(myColFormatEdit[i], i+1, 2);

    myColWidthSpin[i] = new QSpinBox();
    myColWidthSpin[i]->setToolTip(myColWidthLabel->toolTip());
    myColWidthSpin[i]->setRange(0, 2048);
    myColumnsLayout->addWidget(myColWidthSpin[i], i+1, 3);

    myColAlignCombo[i] = new QComboBox();
    myColAlignCombo[i]->setToolTip(myColAlignLabel->toolTip());
    myColAlignCombo[i]->addItem(tr("Left"));
    myColAlignCombo[i]->addItem(tr("Right"));
    myColAlignCombo[i]->addItem(tr("Center"));
    myColumnsLayout->addWidget(myColAlignCombo[i], i+1, 4);
  }

  myPageColumnsLayout->addWidget(myColumnsBox);
  myPageColumnsLayout->addStretch(1);

  return w;
}

QWidget* Settings::ContactList::createPageContactInfo(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  myPageContactInfoLayout = new QVBoxLayout(w);
  myPageContactInfoLayout->setContentsMargins(0, 0, 0, 0);

  myPopupBox = new QGroupBox(tr("Popup info"));
  myPopupLayout = new QGridLayout(myPopupBox);

  myPopupPictureCheck = new QCheckBox(tr("Picture"));
  myPopupLayout->addWidget(myPopupPictureCheck, 0, 0);
  myPopupAliasCheck = new QCheckBox(tr("Alias"));
  myPopupLayout->addWidget(myPopupAliasCheck, 1, 0);
  myPopupNameCheck = new QCheckBox(tr("Full name"));
  myPopupLayout->addWidget(myPopupNameCheck, 2, 0);
  myPopupEmailCheck = new QCheckBox(tr("Email"));
  myPopupLayout->addWidget(myPopupEmailCheck, 3, 0);
  myPopupPhoneCheck = new QCheckBox(tr("Phone"));
  myPopupLayout->addWidget(myPopupPhoneCheck, 4, 0);
  myPopupFaxCheck = new QCheckBox(tr("Fax"));
  myPopupLayout->addWidget(myPopupFaxCheck, 5, 0);
  myPopupCellularCheck = new QCheckBox(tr("Cellular"));
  myPopupLayout->addWidget(myPopupCellularCheck, 6, 0);
  myPopupIpCheck = new QCheckBox(tr("IP address"));
  myPopupLayout->addWidget(myPopupIpCheck, 0, 1);
  myPopupLastOnlineCheck = new QCheckBox(tr("Last online"));
  myPopupLayout->addWidget(myPopupLastOnlineCheck, 1, 1);
  myPopupOnlineSinceCheck = new QCheckBox(tr("Online time"));
  myPopupLayout->addWidget(myPopupOnlineSinceCheck, 2, 1);
  myPopupIdleTimeCheck = new QCheckBox(tr("Idle time"));
  myPopupLayout->addWidget(myPopupIdleTimeCheck, 3, 1);
  myPopupLocalTimeCheck = new QCheckBox(tr("Local time"));
  myPopupLayout->addWidget(myPopupLocalTimeCheck, 4, 1);
  myPopupIdCheck = new QCheckBox(tr("Protocol ID"));
  myPopupLayout->addWidget(myPopupIdCheck, 5, 1);
  myPopupAuthCheck = new QCheckBox(tr("Authorization status"));
  myPopupLayout->addWidget(myPopupAuthCheck, 6, 1);

  myAutoUpdateBox = new QGroupBox(tr("Automatic Update"));
  myAutoUpdateLayout = new QVBoxLayout(myAutoUpdateBox);

  myAutoUpdateInfoCheck = new QCheckBox(tr("Contact information"));
  myAutoUpdateInfoCheck->setToolTip(tr("Automatically update users' server stored information."));
  myAutoUpdateLayout->addWidget(myAutoUpdateInfoCheck);

  myAutoUpdateInfoPluginsCheck = new QCheckBox(tr("Info plugins"));
  myAutoUpdateInfoPluginsCheck->setToolTip(tr("Automatically update users' Phone Book and Picture."));
  myAutoUpdateLayout->addWidget(myAutoUpdateInfoPluginsCheck);

  myAutoUpdateStatusPluginsCheck = new QCheckBox(tr("Status plugins"));
  myAutoUpdateStatusPluginsCheck->setToolTip(tr("Automatically update users' Phone \"Follow Me\", File Server and ICQphone status."));
  myAutoUpdateLayout->addWidget(myAutoUpdateStatusPluginsCheck);

  myPageContactInfoLayout->addWidget(myPopupBox);
  myPageContactInfoLayout->addWidget(myAutoUpdateBox);
  myPageContactInfoLayout->addStretch(1);

  return w;
}

void Settings::ContactList::numColumnsChanged()
{
  // Starting from the top, rows should be enabled
  bool enableRow = true;

  for (unsigned short i = 0; i < MAX_COLUMNCOUNT; ++i)
  {
    myColTitleEdit[i]->setEnabled(enableRow);
    myColFormatEdit[i]->setEnabled(enableRow);
    myColWidthSpin[i]->setEnabled(enableRow);
    myColAlignCombo[i]->setEnabled(enableRow);

    // We've found the checked row, so all rows below this should be disabled
    if(myColNumberRadio[i]->isChecked())
      enableRow = false;
  }
}

void Settings::ContactList::load()
{
  Config::Chat* chatConfig = Config::Chat::instance();
  Config::ContactList* contactListConfig = Config::ContactList::instance();
  Config::General* generalConfig = Config::General::instance();

  myManualNewUserCheck->setChecked(chatConfig->manualNewUser());

  myShowGroupIfNoMsgCheck->setChecked(generalConfig->showGroupIfNoMsg());
  myEnableMainwinMouseMovementCheck->setChecked(generalConfig->mainwinDraggable());
  myMainWinStickyCheck->setChecked(generalConfig->mainwinSticky());

  myGridLinesCheck->setChecked(contactListConfig->showGridLines());
  myFontStylesCheck->setChecked(contactListConfig->useFontStyles());
  myShowExtIconsCheck->setChecked(contactListConfig->showExtendedIcons());
  myShowPhoneIconsCheck->setChecked(contactListConfig->showPhoneIcons());
  myShowPhoneIconsCheck->setEnabled(contactListConfig->showExtendedIcons());
  myHeaderCheck->setChecked(contactListConfig->showHeader());
  myShowDividersCheck->setChecked(contactListConfig->showDividers());
  mySortByCombo->setCurrentIndex(contactListConfig->sortByStatus());
  myAlwaysShowONUCheck->setChecked(contactListConfig->alwaysShowONU());
  myShowUserIconsCheck->setChecked(contactListConfig->showUserIcons());
  myScrollBarCheck->setChecked(contactListConfig->allowScrollBar());
  mySysBackCheck->setChecked(contactListConfig->useSystemBackground());
  myDragMovesUserCheck->setChecked(contactListConfig->dragMovesUser());

  int numColumns = contactListConfig->columnCount();
  if(numColumns < 1)
    myColNumberRadio[0]->setChecked(true);
  else if(numColumns > MAX_COLUMNCOUNT)
    myColNumberRadio[MAX_COLUMNCOUNT - 1]->setChecked(true);
  else
    myColNumberRadio[numColumns - 1]->setChecked(true);

  for (int i = 0; i < MAX_COLUMNCOUNT; ++i)
  {
    myColTitleEdit[i]->setText(contactListConfig->columnHeading(i));
    myColFormatEdit[i]->setText(contactListConfig->columnFormat(i));
    myColWidthSpin[i]->setValue(contactListConfig->columnWidth(i));
    myColAlignCombo[i]->setCurrentIndex(contactListConfig->columnAlignment(i));

    myColTitleEdit[i]->setEnabled(i < numColumns);
    myColFormatEdit[i]->setEnabled(i < numColumns);
    myColWidthSpin[i]->setEnabled(i < numColumns);
    myColAlignCombo[i]->setEnabled(i < numColumns);
  }

  myPopupPictureCheck->setChecked(contactListConfig->popupPicture());
  myPopupAliasCheck->setChecked(contactListConfig->popupAlias());
  myPopupAuthCheck->setChecked(contactListConfig->popupAuth());
  myPopupNameCheck->setChecked(contactListConfig->popupName());
  myPopupEmailCheck->setChecked(contactListConfig->popupEmail());
  myPopupPhoneCheck->setChecked(contactListConfig->popupPhone());
  myPopupFaxCheck->setChecked(contactListConfig->popupFax());
  myPopupCellularCheck->setChecked(contactListConfig->popupCellular());
  myPopupIpCheck->setChecked(contactListConfig->popupIP());
  myPopupLastOnlineCheck->setChecked(contactListConfig->popupLastOnline());
  myPopupOnlineSinceCheck->setChecked(contactListConfig->popupOnlineSince());
  myPopupIdleTimeCheck->setChecked(contactListConfig->popupIdleTime());
  myPopupLocalTimeCheck->setChecked(contactListConfig->popupLocalTime());
  myPopupIdCheck->setChecked(contactListConfig->popupID());

  mySSListCheck->setChecked(gLicqDaemon->UseServerContactList());
  myTransparentCheck->setChecked(Config::Skin::active()->frame.transparent);
  myFrameStyleEdit->setText(QString::number(static_cast<int>(Config::Skin::active()->frame.frameStyle)));

  myAutoUpdateInfoCheck->setChecked(gLicqDaemon->AutoUpdateInfo());
  myAutoUpdateInfoPluginsCheck->setChecked(gLicqDaemon->AutoUpdateInfoPlugins());
  myAutoUpdateStatusPluginsCheck->setChecked(gLicqDaemon->AutoUpdateStatusPlugins());
}

void Settings::ContactList::apply()
{
  Config::Chat* chatConfig = Config::Chat::instance();
  Config::ContactList* contactListConfig = Config::ContactList::instance();
  Config::General* generalConfig = Config::General::instance();
  chatConfig->blockUpdates(true);
  contactListConfig->blockUpdates(true);
  generalConfig->blockUpdates(true);

  chatConfig->setManualNewUser(myManualNewUserCheck->isChecked());

  generalConfig->setShowGroupIfNoMsg(myShowGroupIfNoMsgCheck->isChecked());
  generalConfig->setMainwinDraggable(myEnableMainwinMouseMovementCheck->isChecked());
  generalConfig->setMainwinSticky(myMainWinStickyCheck->isChecked());

  contactListConfig->setShowGridLines(myGridLinesCheck->isChecked());
  contactListConfig->setUseFontStyles(myFontStylesCheck->isChecked());
  contactListConfig->setShowExtendedIcons(myShowExtIconsCheck->isChecked());
  contactListConfig->setShowPhoneIcons(myShowPhoneIconsCheck->isChecked());
  contactListConfig->setShowHeader(myHeaderCheck->isChecked());
  contactListConfig->setShowDividers(myShowDividersCheck->isChecked());
  contactListConfig->setSortByStatus(mySortByCombo->currentIndex());
  contactListConfig->setAlwaysShowONU(myAlwaysShowONUCheck->isChecked());
  contactListConfig->setShowUserIcons(myShowUserIconsCheck->isChecked());
  contactListConfig->setAllowScrollBar(myScrollBarCheck->isChecked());
  contactListConfig->setUseSystemBackground(mySysBackCheck->isChecked());
  contactListConfig->setDragMovesUser(myDragMovesUserCheck->isChecked());

  for (unsigned short i = 0; i < MAX_COLUMNCOUNT; ++i)
  {
    contactListConfig->setColumn(i,
        myColTitleEdit[i]->text(),
        myColFormatEdit[i]->text(),
        myColWidthSpin[i]->value(),
        static_cast<Config::ContactList::AlignmentMode>(myColAlignCombo[i]->currentIndex()));

    if (myColNumberRadio[i]->isChecked())
      contactListConfig->setColumnCount(i + 1);
  }

  contactListConfig->setPopupPicture(myPopupPictureCheck->isChecked());
  contactListConfig->setPopupAlias(myPopupAliasCheck->isChecked());
  contactListConfig->setPopupAuth(myPopupAuthCheck->isChecked());
  contactListConfig->setPopupName(myPopupNameCheck->isChecked());
  contactListConfig->setPopupEmail(myPopupEmailCheck->isChecked());
  contactListConfig->setPopupPhone(myPopupPhoneCheck->isChecked());
  contactListConfig->setPopupFax(myPopupFaxCheck->isChecked());
  contactListConfig->setPopupCellular(myPopupCellularCheck->isChecked());
  contactListConfig->setPopupIP(myPopupIpCheck->isChecked());
  contactListConfig->setPopupLastOnline(myPopupLastOnlineCheck->isChecked());
  contactListConfig->setPopupOnlineSince(myPopupOnlineSinceCheck->isChecked());
  contactListConfig->setPopupIdleTime(myPopupIdleTimeCheck->isChecked());
  contactListConfig->setPopupLocalTime(myPopupLocalTimeCheck->isChecked());
  contactListConfig->setPopupID(myPopupIdCheck->isChecked());

  gLicqDaemon->SetUseServerContactList(mySSListCheck->isChecked());
  Config::Skin::active()->setFrameTransparent(myTransparentCheck->isChecked());
  Config::Skin::active()->setFrameStyle(myFrameStyleEdit->text().toUShort());

  gLicqDaemon->SetAutoUpdateInfo(myAutoUpdateInfoCheck->isChecked());
  gLicqDaemon->SetAutoUpdateInfoPlugins(myAutoUpdateInfoPluginsCheck->isChecked());
  gLicqDaemon->SetAutoUpdateStatusPlugins(myAutoUpdateStatusPluginsCheck->isChecked());

  chatConfig->blockUpdates(false);
  contactListConfig->blockUpdates(false);
  generalConfig->blockUpdates(false);
}
