/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2011 Licq developers
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

#ifndef CONFIG_CONTACTLIST_H
#define CONFIG_CONTACTLIST_H

#include "config.h"

#include <QObject>

#include "core/gui-defines.h"

namespace Licq
{
class IniFile;
}

namespace LicqQtGui
{
namespace Config
{
/**
 * Contact list configuration
 */
class ContactList : public QObject
{
  Q_OBJECT

public:
  enum AlignmentMode
  {
    AlignLeft = 0,
    AlignRight = 1,
    AlignCenter = 2,
  };

  enum FlashMode
  {
    FlashNone = 0,
    FlashAll = 1,
    FlashUrgent = 2,
  };

  /**
   * Create the singleton instance
   *
   * @param parent Parent object
   */
  static void createInstance(QObject* parent = NULL);

  /**
   * Get the singleton instance
   *
   * @return The instance
   */
  static ContactList* instance()
  { return myInstance; }

  /**
   * Constuctor
   */
  ContactList(QObject* parent = 0);

  ~ContactList() {}

  void blockUpdates(bool block);

  // Get functions
  int columnCount() const { return myColumnCount; }
  const QString& columnHeading(int column) { return myColumnHeading[column]; }
  const QString& columnFormat(int column) { return myColumnFormat[column]; }
  int columnWidth(int column) { return myColumnWidth[column]; }
  AlignmentMode columnAlignment(int column) { return myColumnAlignment[column]; }

  bool showOffline() const { return myShowOffline; }
  bool alwaysShowONU() const { return myAlwaysShowONU; }
  bool mode2View() const { return myMode2View; }
  bool showEmptyGroups() const { return myShowEmptyGroups; }
  int groupId() { return myGroupId; }
  bool groupState(int group, bool online) const;

  bool showGridLines() const { return myShowGridLines; }
  bool useFontStyles() const { return myUseFontStyles; }
  bool showHeader() const { return myShowHeader; }
  bool showDividers() const { return myShowDividers; }
  int sortByStatus() const { return mySortByStatus; }
  int sortColumn() const { return mySortColumn; }
  bool sortColumnAscending() const { return mySortColumnAscending; }
  bool showExtendedIcons() const { return myShowExtendedIcons; }
  bool showPhoneIcons() const { return myShowPhoneIcons; }
  bool showUserIcons() const { return myShowUserIcons; }
  FlashMode flash() const { return myFlash; }
  bool allowScrollBar() const { return myAllowScrollBar; }
  bool useSystemBackground() const { return myUseSystemBackground; }
  bool dragMovesUser() const { return myDragMovesUser; }

  bool popupPicture() const { return myPopupPicture; }
  bool popupAlias() const { return myPopupAlias; }
  bool popupAuth() const { return myPopupAuth; }
  bool popupName() const { return myPopupName; }
  bool popupEmail() const { return myPopupEmail; }
  bool popupPhone() const { return myPopupPhone; }
  bool popupFax() const { return myPopupFax; }
  bool popupCellular() const { return myPopupCellular; }
  bool popupIP() const { return myPopupIP; }
  bool popupLastOnline() const { return myPopupLastOnline; }
  bool popupOnlineSince() const { return myPopupOnlineSince; }
  bool popupAwayTime() const { return myPopupAwayTime; }
  bool popupIdleTime() const { return myPopupIdleTime; }
  bool popupLocalTime() const { return myPopupLocalTime; }
  bool popupID() const { return myPopupID; }

public slots:
  /**
   * Load configuration from file
   */
  void loadConfiguration(Licq::IniFile& iniFile);

  /**
   * Save configuration to file
   */
  void saveConfiguration(Licq::IniFile& iniFile) const;

  // Set functions
  void setColumnCount(int columnCount);
  void setColumn(int column, const QString& heading, const QString& format,
      int width, AlignmentMode alignment);

  void setShowOffline(bool showOffline);
  void setAlwaysShowONU(bool alwaysShowONU);
  void setMode2View(bool mode2View);
  void setShowEmptyGroups(bool showEmptyGroups);

  /**
   * Set current group to display
   *
   * @param groupId Id of group to display
   */
  void setGroup(int groupId);

  void setShowGridLines(bool showGridLines);
  void setUseFontStyles(bool useFontStyles);
  void setShowHeader(bool showHeader);
  void setShowDividers(bool showDividers);
  void setSortByStatus(int sortByStatus);
  void setSortColumn(int column, bool ascending = true);
  void setGroupState(int group, bool online, bool expanded);
  void setShowExtendedIcons(bool showExtendedIcons);
  void setShowPhoneIcons(bool showPhoneIcons);
  void setShowUserIcons(bool showUserIcons);
  void setFlash(FlashMode flash);
  void setAllowScrollBar(bool allowScrollBar);
  void setUseSystemBackground(bool useSystemBackground);
  void setDragMovesUser(bool dragMovesUser);

  void setPopupPicture(bool popupPicture);
  void setPopupAlias(bool popupAlias);
  void setPopupAuth(bool popupAuth);
  void setPopupName(bool popupName);
  void setPopupEmail(bool popupEmail);
  void setPopupPhone(bool popupPhone);
  void setPopupFax(bool popupFax);
  void setPopupCellular(bool popupCellular);
  void setPopupIP(bool popupIP);
  void setPopupLastOnline(bool popupLastOnline);
  void setPopupOnlineSince(bool popupOnlineSince);
  void setPopupAwayTime(bool popupAwayTime);
  void setPopupIdleTime(bool popupIdleTime);
  void setPopupLocalTime(bool popupLocalTime);
  void setPopupID(bool popupID);

  // Toggle functions for convenience
  void toggleShowOffline() { setShowOffline(!myShowOffline); }
  void toggleShowHeader() { setShowHeader(!myShowHeader); }

signals:
  /**
   * Configuration affecting contact list layout has changed
   * Used by model and includes column settings and status sorting
   */
  void listLayoutChanged();

  /**
   * Configuration affecting listed contacts has changed
   * Used by view and proxies to select filtering and model root
   */
  void currentListChanged();

  /**
   * Configuration affecting contact list look has changed
   * Used by view and delegate to updated list apperance
   */
  void listLookChanged();

  /**
   * Configuration affecting list sorting
   * Used by view
   */
  void listSortingChanged();

private:
  static ContactList* myInstance;

  // Changes have been made that should trigger changed() signal
  bool myLayoutHasChanged;
  bool myListHasChanged;
  bool myLookHasChanged;
  bool myBlockUpdates;

  // Contact list layout
  int myColumnCount;
  QString myColumnHeading[MAX_COLUMNCOUNT];
  QString myColumnFormat[MAX_COLUMNCOUNT];
  int myColumnWidth[MAX_COLUMNCOUNT];
  AlignmentMode myColumnAlignment[MAX_COLUMNCOUNT];

  // Contact list contents
  bool myShowOffline;
  bool myAlwaysShowONU;
  bool myMode2View;
  bool myShowEmptyGroups;
  int myGroupId;

  // Contact list look
  bool myShowGridLines;
  bool myUseFontStyles;
  bool myShowHeader;
  bool myShowDividers;
  bool myShowExtendedIcons;
  bool myShowPhoneIcons;
  bool myShowUserIcons;
  FlashMode myFlash;
  bool myAllowScrollBar;
  bool myUseSystemBackground;

  // Contact list behaviour
  bool myDragMovesUser;

  // Contact list sorting
  int mySortByStatus;

  // Contact list state
  int mySortColumn;
  bool mySortColumnAscending;
  int myGroupStates[2];

  // Contact popup information
  bool myPopupPicture;
  bool myPopupAlias;
  bool myPopupAuth;
  bool myPopupName;
  bool myPopupEmail;
  bool myPopupPhone;
  bool myPopupFax;
  bool myPopupCellular;
  bool myPopupIP;
  bool myPopupLastOnline;
  bool myPopupOnlineSince;
  bool myPopupAwayTime;
  bool myPopupIdleTime;
  bool myPopupLocalTime;
  bool myPopupID;

  // Conditional emitters
  void changeListLayout();
  void changeCurrentList();
  void changeListLook();
};

} // namespace Config
} // namespace LicqQtGui

#endif
