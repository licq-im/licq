// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2010 Licq developers
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

#include <licq/contactlist/usermanager.h>
#include <licq/inifile.h>

#include "contactlist/contactlist.h"

using namespace LicqQtGui;

Config::ContactList* Config::ContactList::myInstance = NULL;

void Config::ContactList::createInstance(QObject* parent)
{
  myInstance = new Config::ContactList(parent);
}

Config::ContactList::ContactList(QObject* parent)
  : QObject(parent),
    myLayoutHasChanged(false),
    myListHasChanged(false),
    myLookHasChanged(false),
    myBlockUpdates(false)
{
}

void Config::ContactList::loadConfiguration(Licq::IniFile& iniFile)
{
  iniFile.setSection("appearance");
  iniFile.get("GridLines", myShowGridLines, false);
  iniFile.get("FontStyles", myUseFontStyles, true);
  iniFile.get("ShowHeader", myShowHeader, true);
  iniFile.get("ShowOfflineUsers", myShowOffline, true);
  iniFile.get("AlwaysShowONU", myAlwaysShowONU, true);
  iniFile.get("ShowDividers", myShowDividers, true);
  iniFile.get("SortByStatus", mySortByStatus, 1);
  iniFile.get("SortColumn", mySortColumn, 0);
  iniFile.get("SortColumnAscending", mySortColumnAscending, true);
  iniFile.get("UseMode2View", myMode2View, false);
  iniFile.get("ShowEmptyGroups", myShowEmptyGroups, true);
  iniFile.get("TVGroupStates", myGroupStates[0], 0xFFFFFFFE);
  iniFile.get("TVGroupStates2", myGroupStates[1], 0xFFFFFFFE);
  iniFile.get("ShowExtIcons", myShowExtendedIcons, true);
  iniFile.get("ShowPhoneIcons", myShowPhoneIcons, true);
  iniFile.get("ShowUserIcons", myShowUserIcons, true);
  iniFile.get("ScrollBar", myAllowScrollBar, true);
  iniFile.get("SystemBackground", myUseSystemBackground, false);
  iniFile.get("DragMovesUser", myDragMovesUser, true);

  int flash;
  iniFile.get("Flash", flash, FlashUrgent);
  myFlash = static_cast<FlashMode>(flash);

  if (!iniFile.get("GroupId", myGroupId, ContactListModel::AllGroupsGroupId))
  {
    // Group id missing, see if old parameters exist

    int oldGroupId; // 0=All, 1=OnlineNotify, 2=Visible, 3=Invisible, 4=Ignore, 5=NewUsers
    int oldGroupType; // 0=System, 1=User
    bool oldThreadView;
    iniFile.get("UseThreadView", oldThreadView, true);
    iniFile.get("StartUpGroupId", oldGroupId, 0);
    iniFile.get("StartUpGroupType", oldGroupType, 0);

    if (oldGroupType == 0 && oldGroupId == 0 && oldThreadView)
      // Threaded view
      myGroupId = ContactListModel::AllGroupsGroupId;
    else if (oldGroupType != 0)
      // User group
      myGroupId = oldGroupId;
    else if (oldGroupId == 0)
      myGroupId = ContactListModel::AllUsersGroupId;
    else if (oldGroupId == 1)
      myGroupId = ContactListModel::OnlineNotifyGroupId;
    else if (oldGroupId == 2)
      myGroupId = ContactListModel::VisibleListGroupId;
    else if (oldGroupId == 3)
      myGroupId = ContactListModel::InvisibleListGroupId;
    else if (oldGroupId == 4)
      myGroupId = ContactListModel::IgnoreListGroupId;
    else if (oldGroupId == 5)
      myGroupId = ContactListModel::NewUsersGroupId;
  }

  // Check that the group actually exists
  if (myGroupId <= 0 || (myGroupId >= ContactListModel::SystemGroupOffset &&
      myGroupId != ContactListModel::AllUsersGroupId && myGroupId != ContactListModel::AllGroupsGroupId) ||
      (myGroupId < ContactListModel::SystemGroupOffset && !Licq::gUserManager.groupExists(myGroupId)))
    myGroupId = ContactListModel::AllGroupsGroupId;

  iniFile.get("NumColumns", myColumnCount, 1);
  for (int i = 0; i < myColumnCount; i++)
  {
    std::string s;
    int us;

    QString key = QString("Column%1.").arg(i + 1);
    iniFile.get((key + "Title").toLatin1().data(), s, "Alias");
    myColumnHeading[i] = QString::fromLocal8Bit(s.c_str());
    iniFile.get((key + "Format").toLatin1().data(), s, "%a");
    myColumnFormat[i] = QString::fromLocal8Bit(s.c_str());
    iniFile.get((key + "Width").toLatin1().data(), myColumnWidth[i], 100);
    iniFile.get((key + "Align").toLatin1().data(), us, 0);
    myColumnAlignment[i] = static_cast<AlignmentMode>(us);
  }

  iniFile.get("showPopPicture", myPopupPicture, true);
  iniFile.get("showPopAlias", myPopupAlias, false);
  iniFile.get("showPopAuth", myPopupAuth, false);
  iniFile.get("showPopName", myPopupName, false);
  iniFile.get("showPopEmail", myPopupEmail, false);
  iniFile.get("showPopPhone", myPopupPhone, true);
  iniFile.get("showPopFax", myPopupFax, false);
  iniFile.get("showPopCellular", myPopupCellular, true);
  iniFile.get("showPopIP", myPopupIP, false);
  iniFile.get("showPopLastOnelin", myPopupLastOnline, false);
  iniFile.get("showPopOnlineSince", myPopupOnlineSince, false);
  iniFile.get("showPopIdleTime", myPopupIdleTime, true);
  iniFile.get("showPopLocalTime", myPopupLocalTime, false);
  iniFile.get("showPopID", myPopupID, true);

  emit listLayoutChanged();
  emit currentListChanged();
  emit listLookChanged();
}

void Config::ContactList::saveConfiguration(Licq::IniFile& iniFile) const
{
  iniFile.setSection("appearance");
  iniFile.set("GridLines", myShowGridLines);
  iniFile.set("FontStyles", myUseFontStyles);
  iniFile.set("ShowHeader", myShowHeader);
  iniFile.set("ShowDividers", myShowDividers);
  iniFile.set("SortByStatus", mySortByStatus);
  iniFile.set("SortColumn", mySortColumn);
  iniFile.set("SortColumnAscending", mySortColumnAscending);
  iniFile.set("ShowOfflineUsers", myShowOffline);
  iniFile.set("AlwaysShowONU", myAlwaysShowONU);
  iniFile.set("UseMode2View", myMode2View);
  iniFile.set("ShowEmptyGroups", myShowEmptyGroups);
  iniFile.set("TVGroupStates", myGroupStates[0]);
  iniFile.set("TVGroupStates2", myGroupStates[1]);
  iniFile.set("ShowExtIcons", myShowExtendedIcons);
  iniFile.set("ShowPhoneIcons", myShowPhoneIcons);
  iniFile.set("ShowUserIcons", myShowUserIcons);
  iniFile.set("Flash", static_cast<int>(myFlash));
  iniFile.set("ScrollBar", myAllowScrollBar);
  iniFile.set("SystemBackground", myUseSystemBackground);
  iniFile.set("DragMovesUser", myDragMovesUser);
  iniFile.set("GroupId", myGroupId);

  iniFile.set("NumColumns", myColumnCount);
  for (int i = 0; i < myColumnCount; i++)
  {
    QString key = QString("Column%1.").arg(i + 1);
    iniFile.set((key + "Title").toLatin1().data(), myColumnHeading[i].toLocal8Bit().data());
    iniFile.set((key + "Format").toLatin1().data(), myColumnFormat[i].toLocal8Bit().data());
    iniFile.set((key + "Width").toLatin1().data(), myColumnWidth[i]);
    iniFile.set((key + "Align").toLatin1().data(), static_cast<int>(myColumnAlignment[i]));
  }

  iniFile.set("showPopPicture", myPopupPicture);
  iniFile.set("showPopAlias", myPopupAlias);
  iniFile.set("showPopAuth", myPopupAuth);
  iniFile.set("showPopName", myPopupName);
  iniFile.set("showPopEmail", myPopupEmail);
  iniFile.set("showPopPhone", myPopupPhone);
  iniFile.set("showPopFax", myPopupFax);
  iniFile.set("showPopCellular", myPopupCellular);
  iniFile.set("showPopIP", myPopupIP);
  iniFile.set("showPopLastOnelin", myPopupLastOnline);
  iniFile.set("showPopOnlineSince", myPopupOnlineSince);
  iniFile.set("showPopIdleTime", myPopupIdleTime);
  iniFile.set("showPopLocalTime", myPopupLocalTime);
  iniFile.set("showPopID", myPopupID);
}

void Config::ContactList::blockUpdates(bool block)
{
  myBlockUpdates = block;

  if (block)
    return;

  if (myLayoutHasChanged)
  {
    myLayoutHasChanged = false;
    emit listLayoutChanged();
  }
  if (myListHasChanged)
  {
    myListHasChanged = false;
    emit currentListChanged();
  }
  if (myLookHasChanged)
  {
    myLookHasChanged = false;
    emit listLookChanged();
  }
}

void Config::ContactList::setColumnCount(int columnCount)
{
  if (columnCount == myColumnCount || columnCount < 0 || columnCount >= MAX_COLUMNCOUNT)
    return;

  myColumnCount = columnCount;

  changeListLayout();
}

void Config::ContactList::setColumn(int column, const QString& heading,
    const QString& format, int width, AlignmentMode alignment)
{
  if (column < 0 || column >= MAX_COLUMNCOUNT)
    return;

  if (myColumnFormat[column] != format)
  {
    myColumnFormat[column] = format;
    changeListLayout();
  }

  if (heading != myColumnHeading[column] ||
      width != myColumnWidth[column] ||
      alignment != myColumnAlignment[column])
  {
    myColumnHeading[column] = heading;
    myColumnWidth[column] = width;
    myColumnAlignment[column] = alignment;
    changeListLook();
  }
}

void Config::ContactList::setSortByStatus(int sortByStatus)
{
  if (sortByStatus == mySortByStatus)
    return;

  mySortByStatus = sortByStatus;

  changeListLayout();
}

void Config::ContactList::setShowGridLines(bool showGridLines)
{
  if (showGridLines == myShowGridLines)
    return;

  myShowGridLines = showGridLines;

  changeListLook();
}

void Config::ContactList::setUseFontStyles(bool useFontStyles)
{
  if (useFontStyles == myUseFontStyles)
    return;

  myUseFontStyles = useFontStyles;

  changeListLook();
}

void Config::ContactList::setShowHeader(bool showHeader)
{
  if (showHeader == myShowHeader)
    return;

  myShowHeader = showHeader;

  changeListLook();
}

void Config::ContactList::setShowExtendedIcons(bool showExtendedIcons)
{
  if (showExtendedIcons == myShowExtendedIcons)
    return;

  myShowExtendedIcons = showExtendedIcons;

  changeListLook();
}

void Config::ContactList::setShowPhoneIcons(bool showPhoneIcons)
{
  if (showPhoneIcons == myShowPhoneIcons)
    return;

  myShowPhoneIcons = showPhoneIcons;

  changeListLook();
}

void Config::ContactList::setShowUserIcons(bool showUserIcons)
{
  if (showUserIcons == myShowUserIcons)
    return;

  myShowUserIcons = showUserIcons;

  changeListLook();
}

void Config::ContactList::setFlash(FlashMode flash)
{
  if (flash == myFlash)
    return;

  myFlash = flash;

  changeListLook();
}

void Config::ContactList::setAllowScrollBar(bool allowScrollBar)
{
  if (allowScrollBar == myAllowScrollBar)
    return;

  myAllowScrollBar = allowScrollBar;

  changeListLook();
}

void Config::ContactList::setUseSystemBackground(bool useSystemBackground)
{
  if (useSystemBackground == myUseSystemBackground)
    return;

  myUseSystemBackground = useSystemBackground;

  changeListLook();
}

void Config::ContactList::setDragMovesUser(bool dragMovesUser)
{
  if (dragMovesUser == myDragMovesUser)
    return;

  myDragMovesUser = dragMovesUser;
}

void Config::ContactList::setShowDividers(bool showDividers)
{
  if (showDividers == myShowDividers)
    return;

  myShowDividers = showDividers;

  changeCurrentList();
}

void Config::ContactList::setAlwaysShowONU(bool alwaysShowONU)
{
  if (alwaysShowONU == myAlwaysShowONU)
    return;

  myAlwaysShowONU = alwaysShowONU;

  changeCurrentList();
}

void Config::ContactList::setShowOffline(bool showOffline)
{
  if (showOffline == myShowOffline)
    return;

  myShowOffline = showOffline;

  changeCurrentList();
}

void Config::ContactList::setMode2View(bool mode2View)
{
  if (mode2View == myMode2View)
    return;

  myMode2View = mode2View;

  changeCurrentList();
}

void Config::ContactList::setShowEmptyGroups(bool showEmptyGroups)
{
  if (showEmptyGroups == myShowEmptyGroups)
    return;

  myShowEmptyGroups = showEmptyGroups;

  changeCurrentList();
}

void Config::ContactList::setGroup(int groupId)
{
  if (groupId == myGroupId)
    return;

  myGroupId = groupId;

  changeCurrentList();
}

void Config::ContactList::setSortColumn(int column, bool ascending)
{
  mySortColumn = column;
  mySortColumnAscending = ascending;

  emit listSortingChanged();
}

bool Config::ContactList::groupState(int group, bool online) const
{
  return myGroupStates[online ? 0 : 1] & (1 << qMin(group, 31));
}

void Config::ContactList::setGroupState(int group, bool online, bool expanded)
{
  if(group > 31)
    group = 31;

  if (expanded)
    myGroupStates[online ? 0 : 1] |= (1 << group);
  else
    myGroupStates[online ? 0 : 1] &= ~(1 << group);

  // Called by view when a group has changed state so don't emit any signal
}

void Config::ContactList::setPopupPicture(bool popupPicture)
{
  myPopupPicture = popupPicture;
}

void Config::ContactList::setPopupAlias(bool popupAlias)
{
  myPopupAlias = popupAlias;
}

void Config::ContactList::setPopupAuth(bool popupAuth)
{
  myPopupAuth = popupAuth;
}

void Config::ContactList::setPopupName(bool popupName)
{
  myPopupName = popupName;
}

void Config::ContactList::setPopupEmail(bool popupEmail)
{
  myPopupEmail = popupEmail;
}

void Config::ContactList::setPopupPhone(bool popupPhone)
{
  myPopupPhone = popupPhone;
}

void Config::ContactList::setPopupFax(bool popupFax)
{
  myPopupFax = popupFax;
}

void Config::ContactList::setPopupCellular(bool popupCellular)
{
  myPopupCellular = popupCellular;
}

void Config::ContactList::setPopupIP(bool popupIP)
{
  myPopupIP = popupIP;
}

void Config::ContactList::setPopupLastOnline(bool popupLastOnline)
{
  myPopupLastOnline = popupLastOnline;
}

void Config::ContactList::setPopupOnlineSince(bool popupOnlineSince)
{
  myPopupOnlineSince = popupOnlineSince;
}

void Config::ContactList::setPopupIdleTime(bool popupIdleTime)
{
  myPopupIdleTime = popupIdleTime;
}

void Config::ContactList::setPopupLocalTime(bool popupLocalTime)
{
  myPopupLocalTime = popupLocalTime;
}

void Config::ContactList::setPopupID(bool popupID)
{
  myPopupID = popupID;
}

void Config::ContactList::changeListLayout()
{
  if (myBlockUpdates)
    myLayoutHasChanged = true;
  else
    emit listLayoutChanged();
}

void Config::ContactList::changeCurrentList()
{
  if (myBlockUpdates)
    myListHasChanged = true;
  else
    emit currentListChanged();
}

void Config::ContactList::changeListLook()
{
  if (myBlockUpdates)
    myLookHasChanged = true;
  else
    emit listLookChanged();
}
