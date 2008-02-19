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
  layPageContactList = new QVBoxLayout(w);
  layPageContactList->setContentsMargins(0, 0, 0, 0);

  appearanceBox = new QGroupBox(tr("Contact List Appearance"));
  appearanceLayout = new QGridLayout(appearanceBox);

  chkGridLines = new QCheckBox(tr("Show grid lines"));
  chkGridLines->setToolTip(tr("Draw boxes around each square in the user list"));
  appearanceLayout->addWidget(chkGridLines, 0, 0);

  chkHeader = new QCheckBox(tr("Show column headers"));
  chkHeader->setToolTip(tr("Turns on or off the display of headers above each column in the user list"));
  appearanceLayout->addWidget(chkHeader, 1, 0);

  chkShowDividers = new QCheckBox(tr("Show user dividers"));
  chkShowDividers->setToolTip(tr("Show the \"--online--\" and \"--offline--\" bars in the contact list"));
  appearanceLayout->addWidget(chkShowDividers, 2, 0);

  chkFontStyles = new QCheckBox(tr("Use font styles"));
  chkFontStyles->setToolTip(tr("Use italics and bold in the user list to "
      "indicate special characteristics such as online notify and visible list"));
  appearanceLayout->addWidget(chkFontStyles, 3, 0);

  chkShowExtIcons = new QCheckBox(tr("Show extended icons"));
  chkShowExtIcons->setToolTip(tr("Show birthday, invisible, and custom auto response icons to the right of users in the list"));
  appearanceLayout->addWidget(chkShowExtIcons, 4, 0);

  chkShowUserIcons = new QCheckBox(tr("Show user display picture"));
  chkShowUserIcons->setToolTip(tr("Show the user's display picture"
      " instead of a status icon, if the user"
      " is online and has a display picture"));
  appearanceLayout->addWidget(chkShowUserIcons, 5, 0);

  chkAlwaysShowONU = new QCheckBox(tr("Always show online notify users"));
  chkAlwaysShowONU->setToolTip(tr("Show online notify users who are offline even when offline users are hidden."));
  appearanceLayout->addWidget(chkAlwaysShowONU, 0, 1);

  chkTransparent = new QCheckBox(tr("Transparent when possible"));
  chkTransparent->setToolTip(tr("Make the user window transparent when there is no scroll bar"));
  appearanceLayout->addWidget(chkTransparent, 1, 1);

  chkShowGroupIfNoMsg = new QCheckBox(tr("Show group name if no messages"));
  chkShowGroupIfNoMsg->setToolTip(tr("Show the name of the current group in the messages label when there are no new messages"));
  appearanceLayout->addWidget(chkShowGroupIfNoMsg, 2, 1);

  chkSysBack = new QCheckBox(tr("Use system background color"));
  appearanceLayout->addWidget(chkSysBack, 3, 1);

  chkScrollBar = new QCheckBox(tr("Allow scroll bar"));
  chkScrollBar->setToolTip(tr("Allow the vertical scroll bar in the user list"));
  appearanceLayout->addWidget(chkScrollBar, 4, 1);

  QHBoxLayout* layFrameStyle = new QHBoxLayout();
  lblFrameStyle = new QLabel(tr("Frame style:"));
  layFrameStyle->addWidget(lblFrameStyle);
  lblFrameStyle->setToolTip(tr("Override the skin setting for the frame style of the user window:\n"
                               "   0 (No frame), 1 (Box), 2 (Panel), 3 (WinPanel)\n"
                               " + 16 (Plain), 32 (Raised), 48 (Sunken), 240 (Shadow)"));
  edtFrameStyle = new QLineEdit();
  edtFrameStyle->setValidator(new QIntValidator(edtFrameStyle));
  edtFrameStyle->setToolTip(lblFrameStyle->toolTip());
  lblFrameStyle->setBuddy(edtFrameStyle);
  layFrameStyle->addWidget(edtFrameStyle);
  appearanceLayout->addLayout(layFrameStyle, 5, 1);

  // Make the columns evenly wide, otherwise the QLineEdit steals the space
  appearanceLayout->setColumnStretch(0, 1);
  appearanceLayout->setColumnStretch(1, 1);


  behaviourBox = new QGroupBox(tr("Contact List Behaviour"));
  behaviourLayout = new QGridLayout(behaviourBox);

  chkSSList = new QCheckBox(tr("Use server side contact list"));
  chkSSList->setToolTip(tr("Store your contacts on the server so they are accessible from different locations and/or programs"));
  behaviourLayout->addWidget(chkSSList, 0, 0);

  chkManualNewUser = new QCheckBox(tr("Manual \"New User\" group handling"));
  chkManualNewUser->setToolTip(tr("If not checked, a user will be automatically removed from "
                                  "\"New User\" group when you first send an event to them."));
  behaviourLayout->addWidget(chkManualNewUser, 1, 0);

  chkEnableMainwinMouseMovement = new QCheckBox(tr("Allow dragging main window"));
  chkEnableMainwinMouseMovement->setToolTip(tr("Lets you drag around the main window with your mouse"));
  behaviourLayout->addWidget(chkEnableMainwinMouseMovement, 2, 0);

  chkMainWinSticky = new QCheckBox(tr("Sticky main window"));
  chkMainWinSticky->setToolTip(tr("Makes the Main window visible on all desktops"));
  behaviourLayout->addWidget(chkMainWinSticky, 0, 1);

  QHBoxLayout* laySortBy = new QHBoxLayout();
  lblSortBy = new QLabel(tr("Additional sorting:"));
  lblSortBy->setToolTip(tr("<b>none:</b> - Don't sort online users by Status<br>\n"
                                "<b>status</b> - Sort online users by status<br>\n"
                                "<b>status + last event</b> - Sort online users by status and by last event<br>\n"
                                "<b>status + new messages</b> - Sort online users by status and number of new messages"));
  laySortBy->addWidget(lblSortBy);
  cmbSortBy = new QComboBox();
  cmbSortBy->addItem(tr("none"), 0);
  cmbSortBy->addItem(tr("status"), 1);
  cmbSortBy->addItem(tr("status + last event"), 2);
  cmbSortBy->addItem(tr("status + new messages"), 3);
  cmbSortBy->setToolTip(lblSortBy->toolTip());
  lblSortBy->setBuddy(cmbSortBy);
  laySortBy->addWidget(cmbSortBy);
  behaviourLayout->addLayout(laySortBy, 1, 1);


  layPageContactList->addWidget(appearanceBox);
  layPageContactList->addWidget(behaviourBox);
  layPageContactList->addStretch(1);

  return w;
}

QWidget* Settings::ContactList::createPageColumns(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  layPageColumns = new QVBoxLayout(w);
  layPageColumns->setContentsMargins(0, 0, 0, 0);

  boxColumns = new QGroupBox (tr("Column Configuration"));
  layColumns = new QGridLayout(boxColumns);

  // Headings
  lblColTitle = new QLabel (tr("Title"));
  lblColTitle->setToolTip(tr("The string which will appear in the list box column header"));
  layColumns->addWidget(lblColTitle, 0, 1);
  lblColFormat = new QLabel (tr("Format"));
  lblColFormat->setToolTip(tr("The format string used to define what will appear in each column.<br>"
                              "The following parameters can be used:")
                           + gMainWindow->usprintfHelp);
  layColumns->addWidget(lblColFormat, 0, 2);
  lblColWidth = new QLabel (tr("Width"));
  lblColWidth->setToolTip(tr("The width of the column"));
  layColumns->addWidget(lblColWidth, 0, 3);
  lblColAlign = new QLabel(tr("Alignment"));
  lblColAlign->setToolTip(tr("The alignment of the column"));
  layColumns->addWidget(lblColAlign, 0, 4);

  for (unsigned short i = 0; i < MAX_COLUMNCOUNT; i++)
  {
    rdbColNumber[i] = new QRadioButton(QString::number(i+1));
    rdbColNumber[i]->setToolTip(tr("Number of columns"));
    connect(rdbColNumber[i], SIGNAL(toggled(bool)), SLOT(colEnable(bool)));
    layColumns->addWidget(rdbColNumber[i], i+1, 0);

    edtColTitle[i] = new QLineEdit();
    edtColTitle[i]->setToolTip(lblColTitle->toolTip());
    layColumns->addWidget(edtColTitle[i], i+1, 1);

    edtColFormat[i] = new QLineEdit();
    edtColFormat[i]->setToolTip(lblColFormat->toolTip());
    layColumns->addWidget(edtColFormat[i], i+1, 2);

    spnColWidth[i] = new QSpinBox();
    spnColWidth[i]->setToolTip(lblColWidth->toolTip());
    spnColWidth[i]->setRange(0, 2048);
    layColumns->addWidget(spnColWidth[i], i+1, 3);

    cmbColAlign[i] = new QComboBox();
    cmbColAlign[i]->setToolTip(lblColAlign->toolTip());
    cmbColAlign[i]->addItem(tr("Left"));
    cmbColAlign[i]->addItem(tr("Right"));
    cmbColAlign[i]->addItem(tr("Center"));
    layColumns->addWidget(cmbColAlign[i], i+1, 4);
  }

  layPageColumns->addWidget(boxColumns);
  layPageColumns->addStretch(1);

  return w;
}

QWidget* Settings::ContactList::createPageContactInfo(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  layPageContactInfo = new QVBoxLayout(w);
  layPageContactInfo->setContentsMargins(0, 0, 0, 0);

  boxPopup = new QGroupBox(tr("Popup info"));
  layPopup = new QGridLayout(boxPopup);

  popPicture = new QCheckBox(tr("Picture"));
  layPopup->addWidget(popPicture, 0, 0);
  popAlias = new QCheckBox(tr("Alias"));
  layPopup->addWidget(popAlias, 1, 0);
  popName = new QCheckBox(tr("Full name"));
  layPopup->addWidget(popName, 2, 0);
  popEmail = new QCheckBox(tr("Email"));
  layPopup->addWidget(popEmail, 3, 0);
  popPhone = new QCheckBox(tr("Phone"));
  layPopup->addWidget(popPhone, 4, 0);
  popFax = new QCheckBox(tr("Fax"));
  layPopup->addWidget(popFax, 5, 0);
  popCellular = new QCheckBox(tr("Cellular"));
  layPopup->addWidget(popCellular, 6, 0);
  popIP = new QCheckBox(tr("IP address"));
  layPopup->addWidget(popIP, 0, 1);
  popLastOnline = new QCheckBox(tr("Last online"));
  layPopup->addWidget(popLastOnline, 1, 1);
  popOnlineSince = new QCheckBox(tr("Online time"));
  layPopup->addWidget(popOnlineSince, 2, 1);
  popIdleTime = new QCheckBox(tr("Idle time"));
  layPopup->addWidget(popIdleTime, 3, 1);
  popLocalTime = new QCheckBox(tr("Local time"));
  layPopup->addWidget(popLocalTime, 4, 1);
  popID = new QCheckBox(tr("Protocol ID"));
  layPopup->addWidget(popID, 5, 1);
  popAuth = new QCheckBox(tr("Authorization status"));
  layPopup->addWidget(popAuth, 6, 1);

  boxAutoUpdate = new QGroupBox(tr("Automatic Update"));
  layAutoUpdate = new QVBoxLayout(boxAutoUpdate);

  chkAutoUpdateInfo = new QCheckBox(tr("Contact information"));
  chkAutoUpdateInfo->setToolTip(tr("Automatically update users' server stored information."));
  layAutoUpdate->addWidget(chkAutoUpdateInfo);

  chkAutoUpdateInfoPlugins = new QCheckBox(tr("Info plugins"));
  chkAutoUpdateInfoPlugins->setToolTip(tr("Automatically update users' Phone Book and Picture."));
  layAutoUpdate->addWidget(chkAutoUpdateInfoPlugins);

  chkAutoUpdateStatusPlugins = new QCheckBox(tr("Status plugins"));
  chkAutoUpdateStatusPlugins->setToolTip(tr("Automatically update users' Phone \"Follow Me\", File Server and ICQphone status."));
  layAutoUpdate->addWidget(chkAutoUpdateStatusPlugins);

  layPageContactInfo->addWidget(boxPopup);
  layPageContactInfo->addWidget(boxAutoUpdate);
  layPageContactInfo->addStretch(1);

  return w;
}

void Settings::ContactList::colEnable(bool /* isOn */)
{
  // Starting from the top, rows should be enabled
  bool enableRow = true;

  for (unsigned short i = 0; i < MAX_COLUMNCOUNT; ++i)
  {
    edtColTitle[i]->setEnabled(enableRow);
    edtColFormat[i]->setEnabled(enableRow);
    spnColWidth[i]->setEnabled(enableRow);
    cmbColAlign[i]->setEnabled(enableRow);

    // We've found the checked row, so all rows below this should be disabled
    if(rdbColNumber[i]->isChecked())
      enableRow = false;
  }
}

void Settings::ContactList::load()
{
  Config::Chat* chatConfig = Config::Chat::instance();
  Config::ContactList* contactListConfig = Config::ContactList::instance();
  Config::General* generalConfig = Config::General::instance();

  chkManualNewUser->setChecked(chatConfig->manualNewUser());

  chkShowGroupIfNoMsg->setChecked(generalConfig->showGroupIfNoMsg());
  chkEnableMainwinMouseMovement->setChecked(generalConfig->mainwinDraggable());
  chkMainWinSticky->setChecked(generalConfig->mainwinSticky());

  chkGridLines->setChecked(contactListConfig->showGridLines());
  chkFontStyles->setChecked(contactListConfig->useFontStyles());
  chkShowExtIcons->setChecked(contactListConfig->showExtendedIcons());
  chkHeader->setChecked(contactListConfig->showHeader());
  chkShowDividers->setChecked(contactListConfig->showDividers());
  cmbSortBy->setCurrentIndex(contactListConfig->sortByStatus());
  chkAlwaysShowONU->setChecked(contactListConfig->alwaysShowONU());
  chkShowUserIcons->setChecked(contactListConfig->showUserIcons());
  chkScrollBar->setChecked(contactListConfig->allowScrollBar());
  chkSysBack->setChecked(contactListConfig->useSystemBackground());

  int numColumns = contactListConfig->columnCount();
  if(numColumns < 1)
    rdbColNumber[0]->setChecked(true);
  else if(numColumns > MAX_COLUMNCOUNT)
    rdbColNumber[MAX_COLUMNCOUNT - 1]->setChecked(true);
  else
    rdbColNumber[numColumns - 1]->setChecked(true);

  for (int i = 0; i < MAX_COLUMNCOUNT; ++i)
  {
    edtColTitle[i]->setText(contactListConfig->columnHeading(i));
    edtColFormat[i]->setText(contactListConfig->columnFormat(i));
    spnColWidth[i]->setValue(contactListConfig->columnWidth(i));
    cmbColAlign[i]->setCurrentIndex(contactListConfig->columnAlignment(i));

    edtColTitle[i]->setEnabled(i < numColumns);
    edtColFormat[i]->setEnabled(i < numColumns);
    spnColWidth[i]->setEnabled(i < numColumns);
    cmbColAlign[i]->setEnabled(i < numColumns);
  }

  popPicture->setChecked(contactListConfig->popupPicture());
  popAlias->setChecked(contactListConfig->popupAlias());
  popAuth->setChecked(contactListConfig->popupAuth());
  popName->setChecked(contactListConfig->popupName());
  popEmail->setChecked(contactListConfig->popupEmail());
  popPhone->setChecked(contactListConfig->popupPhone());
  popFax->setChecked(contactListConfig->popupFax());
  popCellular->setChecked(contactListConfig->popupCellular());
  popIP->setChecked(contactListConfig->popupIP());
  popLastOnline->setChecked(contactListConfig->popupLastOnline());
  popOnlineSince->setChecked(contactListConfig->popupOnlineSince());
  popIdleTime->setChecked(contactListConfig->popupIdleTime());
  popLocalTime->setChecked(contactListConfig->popupLocalTime());
  popID->setChecked(contactListConfig->popupID());

  chkSSList->setChecked(gLicqDaemon->UseServerContactList());
  chkTransparent->setChecked(Config::Skin::active()->frame.transparent);
  edtFrameStyle->setText(QString::number(static_cast<int>(Config::Skin::active()->frame.frameStyle)));

  chkAutoUpdateInfo->setChecked(gLicqDaemon->AutoUpdateInfo());
  chkAutoUpdateInfoPlugins->setChecked(gLicqDaemon->AutoUpdateInfoPlugins());
  chkAutoUpdateStatusPlugins->setChecked(gLicqDaemon->AutoUpdateStatusPlugins());

}

void Settings::ContactList::apply()
{
  Config::Chat* chatConfig = Config::Chat::instance();
  Config::ContactList* contactListConfig = Config::ContactList::instance();
  Config::General* generalConfig = Config::General::instance();
  chatConfig->blockUpdates(true);
  contactListConfig->blockUpdates(true);
  generalConfig->blockUpdates(true);

  chatConfig->setManualNewUser(chkManualNewUser->isChecked());

  generalConfig->setShowGroupIfNoMsg(chkShowGroupIfNoMsg->isChecked());
  generalConfig->setMainwinDraggable(chkEnableMainwinMouseMovement->isChecked());
  generalConfig->setMainwinSticky(chkMainWinSticky->isChecked());

  contactListConfig->setShowGridLines(chkGridLines->isChecked());
  contactListConfig->setUseFontStyles(chkFontStyles->isChecked());
  contactListConfig->setShowExtendedIcons(chkShowExtIcons->isChecked());
  contactListConfig->setShowHeader(chkHeader->isChecked());
  contactListConfig->setShowDividers(chkShowDividers->isChecked());
  contactListConfig->setSortByStatus(cmbSortBy->currentIndex());
  contactListConfig->setAlwaysShowONU(chkAlwaysShowONU->isChecked());
  contactListConfig->setShowUserIcons(chkShowUserIcons->isChecked());
  contactListConfig->setAllowScrollBar(chkScrollBar->isChecked());
  contactListConfig->setUseSystemBackground(chkSysBack->isChecked());

  for (unsigned short i = 0; i < MAX_COLUMNCOUNT; ++i)
  {
    contactListConfig->setColumn(i,
        edtColTitle[i]->text(),
        edtColFormat[i]->text(),
        spnColWidth[i]->value(),
        static_cast<Config::ContactList::AlignmentMode>(cmbColAlign[i]->currentIndex()));

    if (rdbColNumber[i]->isChecked())
      contactListConfig->setColumnCount(i + 1);
  }

  contactListConfig->setPopupPicture(popPicture->isChecked());
  contactListConfig->setPopupAlias(popAlias->isChecked());
  contactListConfig->setPopupAuth(popAuth->isChecked());
  contactListConfig->setPopupName(popName->isChecked());
  contactListConfig->setPopupEmail(popEmail->isChecked());
  contactListConfig->setPopupPhone(popPhone->isChecked());
  contactListConfig->setPopupFax(popFax->isChecked());
  contactListConfig->setPopupCellular(popCellular->isChecked());
  contactListConfig->setPopupIP(popIP->isChecked());
  contactListConfig->setPopupLastOnline(popLastOnline->isChecked());
  contactListConfig->setPopupOnlineSince(popOnlineSince->isChecked());
  contactListConfig->setPopupIdleTime(popIdleTime->isChecked());
  contactListConfig->setPopupLocalTime(popLocalTime->isChecked());
  contactListConfig->setPopupID(popID->isChecked());

  gLicqDaemon->SetUseServerContactList(chkSSList->isChecked());
  Config::Skin::active()->setFrameTransparent(chkTransparent->isChecked());
  Config::Skin::active()->setFrameStyle(edtFrameStyle->text().toUShort());

  gLicqDaemon->SetAutoUpdateInfo(chkAutoUpdateInfo->isChecked());
  gLicqDaemon->SetAutoUpdateInfoPlugins(chkAutoUpdateInfoPlugins->isChecked());
  gLicqDaemon->SetAutoUpdateStatusPlugins(chkAutoUpdateStatusPlugins->isChecked());

  chatConfig->blockUpdates(false);
  contactListConfig->blockUpdates(false);
  generalConfig->blockUpdates(false);
}
