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

#include <licq_file.h>
#include <licq_user.h>

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

void Config::ContactList::loadConfiguration(CIniFile& iniFile)
{
  iniFile.SetSection("appearance");
  iniFile.ReadBool("GridLines", myShowGridLines, false);
  iniFile.ReadBool("FontStyles", myUseFontStyles, true);
  iniFile.ReadBool("ShowHeader", myShowHeader, true);
  iniFile.ReadBool("ShowOfflineUsers", myShowOffline, true);
  iniFile.ReadBool("AlwaysShowONU", myAlwaysShowONU, true);
  iniFile.ReadBool("ShowDividers", myShowDividers, true);
  iniFile.ReadNum("SortByStatus", mySortByStatus, 1);
  iniFile.ReadNum("SortColumn", mySortColumn, 0);
  iniFile.ReadBool("SortColumnAscending", mySortColumnAscending, true);
  iniFile.ReadBool("UseThreadView", myThreadView, true);
  iniFile.ReadBool("ShowEmptyGroups", myShowEmptyGroups, true);
  iniFile.ReadNum("TVGroupStates", myGroupStates, 0xFFFFFFFE);
  iniFile.ReadBool("ShowExtIcons", myShowExtendedIcons, true);
  iniFile.ReadBool("ShowPhoneIcons", myShowPhoneIcons, true);
  iniFile.ReadBool("ShowUserIcons", myShowUserIcons, true);
  iniFile.ReadBool("ScrollBar", myAllowScrollBar, true);
  iniFile.ReadBool("SystemBackground", myUseSystemBackground, false);
  iniFile.ReadBool("DragMovesUser", myDragMovesUser, true);

  unsigned short flash;
  iniFile.ReadNum("Flash", flash, FlashUrgent);
  myFlash = static_cast<FlashMode>(flash);

  unsigned short groupType;
  iniFile.ReadNum("StartUpGroupId", myGroupId, GROUP_ALL_USERS);
  iniFile.ReadNum("StartUpGroupType", groupType, GROUPS_SYSTEM);
  myGroupType = static_cast<GroupType>(groupType);

  // Check that the group actually exists
  if (!gUserManager.groupExists(myGroupType, myGroupId))
  {
    myGroupId = GROUP_ALL_USERS;
    myGroupType = GROUPS_SYSTEM;
  }

  iniFile.ReadNum("NumColumns", myColumnCount, 1);
  for (unsigned short i = 0; i < myColumnCount; i++)
  {
    char s[32];
    unsigned short us;

    QString key = QString("Column%1.").arg(i + 1);
    iniFile.ReadStr((key + "Title").toLatin1().data(), s, "Alias");
    myColumnHeading[i] = QString::fromLocal8Bit(s);
    iniFile.ReadStr((key + "Format").toLatin1().data(), s, "%a");
    myColumnFormat[i] = QString::fromLocal8Bit(s);
    iniFile.ReadNum((key + "Width").toLatin1().data(), myColumnWidth[i], 100);
    iniFile.ReadNum((key + "Align").toLatin1().data(), us, 0);
    myColumnAlignment[i] = static_cast<AlignmentMode>(us);
  }

  iniFile.ReadBool("showPopPicture", myPopupPicture, true);
  iniFile.ReadBool("showPopAlias", myPopupAlias, false);
  iniFile.ReadBool("showPopAuth", myPopupAuth, false);
  iniFile.ReadBool("showPopName", myPopupName, false);
  iniFile.ReadBool("showPopEmail", myPopupEmail, false);
  iniFile.ReadBool("showPopPhone", myPopupPhone, true);
  iniFile.ReadBool("showPopFax", myPopupFax, false);
  iniFile.ReadBool("showPopCellular", myPopupCellular, true);
  iniFile.ReadBool("showPopIP", myPopupIP, false);
  iniFile.ReadBool("showPopLastOnelin", myPopupLastOnline, false);
  iniFile.ReadBool("showPopOnlineSince", myPopupOnlineSince, false);
  iniFile.ReadBool("showPopIdleTime", myPopupIdleTime, true);
  iniFile.ReadBool("showPopLocalTime", myPopupLocalTime, false);
  iniFile.ReadBool("showPopID", myPopupID, true);

  emit listLayoutChanged();
  emit currentListChanged();
  emit listLookChanged();
}

void Config::ContactList::saveConfiguration(CIniFile& iniFile) const
{
  iniFile.SetSection("appearance");
  iniFile.WriteBool("GridLines", myShowGridLines);
  iniFile.WriteBool("FontStyles", myUseFontStyles);
  iniFile.WriteBool("ShowHeader", myShowHeader);
  iniFile.WriteBool("ShowDividers", myShowDividers);
  iniFile.WriteNum("SortByStatus", mySortByStatus);
  iniFile.WriteNum("SortColumn", mySortColumn);
  iniFile.WriteBool("SortColumnAscending", mySortColumnAscending);
  iniFile.WriteBool("ShowOfflineUsers", myShowOffline);
  iniFile.WriteBool("AlwaysShowONU", myAlwaysShowONU);
  iniFile.WriteBool("UseThreadView", myThreadView);
  iniFile.WriteBool("ShowEmptyGroups", myShowEmptyGroups);
  iniFile.WriteNum("TVGroupStates", myGroupStates);
  iniFile.WriteBool("ShowExtIcons", myShowExtendedIcons);
  iniFile.WriteBool("ShowPhoneIcons", myShowPhoneIcons);
  iniFile.WriteBool("ShowUserIcons", myShowUserIcons);
  iniFile.WriteNum("Flash", static_cast<unsigned short>(myFlash));
  iniFile.WriteBool("ScrollBar", myAllowScrollBar);
  iniFile.WriteBool("SystemBackground", myUseSystemBackground);
  iniFile.WriteBool("DragMovesUser", myDragMovesUser);
  iniFile.WriteNum("StartUpGroupId", myGroupId);
  iniFile.WriteNum("StartUpGroupType", static_cast<unsigned short>(myGroupType));

  iniFile.WriteNum("NumColumns", myColumnCount);
  for (unsigned short i = 0; i < myColumnCount; i++)
  {
    QString key = QString("Column%1.").arg(i + 1);
    iniFile.WriteStr((key + "Title").toLatin1().data(), myColumnHeading[i].toLocal8Bit().data());
    iniFile.WriteStr((key + "Format").toLatin1().data(), myColumnFormat[i].toLocal8Bit().data());
    iniFile.WriteNum((key + "Width").toLatin1().data(), myColumnWidth[i]);
    iniFile.WriteNum((key + "Align").toLatin1().data(), static_cast<unsigned short>(myColumnAlignment[i]));
  }

  iniFile.WriteBool("showPopPicture", myPopupPicture);
  iniFile.WriteBool("showPopAlias", myPopupAlias);
  iniFile.WriteBool("showPopAuth", myPopupAuth);
  iniFile.WriteBool("showPopName", myPopupName);
  iniFile.WriteBool("showPopEmail", myPopupEmail);
  iniFile.WriteBool("showPopPhone", myPopupPhone);
  iniFile.WriteBool("showPopFax", myPopupFax);
  iniFile.WriteBool("showPopCellular", myPopupCellular);
  iniFile.WriteBool("showPopIP", myPopupIP);
  iniFile.WriteBool("showPopLastOnelin", myPopupLastOnline);
  iniFile.WriteBool("showPopOnlineSince", myPopupOnlineSince);
  iniFile.WriteBool("showPopIdleTime", myPopupIdleTime);
  iniFile.WriteBool("showPopLocalTime", myPopupLocalTime);
  iniFile.WriteBool("showPopID", myPopupID);
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

void Config::ContactList::setColumn(int column, QString heading, QString format, unsigned short width, AlignmentMode alignment)
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

void Config::ContactList::setSortByStatus(unsigned short sortByStatus)
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

void Config::ContactList::setThreadView(bool threadView)
{
  if (threadView == myThreadView)
    return;

  myThreadView = threadView;

  changeCurrentList();
}

void Config::ContactList::setShowEmptyGroups(bool showEmptyGroups)
{
  if (showEmptyGroups == myShowEmptyGroups)
    return;

  myShowEmptyGroups = showEmptyGroups;

  changeCurrentList();
}

void Config::ContactList::setGroup(GroupType groupType, unsigned long groupId)
{
  if (groupType == myGroupType && groupId == myGroupId)
    return;

  myGroupType = groupType;
  myGroupId = groupId;

  changeCurrentList();
}

void Config::ContactList::setSortColumn(unsigned short column, bool ascending)
{
  mySortColumn = column;
  mySortColumnAscending = ascending;

  emit listSortingChanged();
}

bool Config::ContactList::groupState(unsigned short group) const
{
  return myGroupStates & (1 << qMin(static_cast<int>(group), 31));
}

void Config::ContactList::setGroupState(unsigned short group, bool expanded)
{
  if(group > 31)
    group = 31;

  if (expanded)
    myGroupStates |= (1 << group);
  else
    myGroupStates &= ~(1 << group);

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
