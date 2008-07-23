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

#ifndef CONTACTLISTMODEL_H
#define CONTACTLISTMODEL_H

#include <QAbstractItemModel>
#include <QList>

#include <licq_user.h>

class CICQSignal;
class ICQUser;


namespace LicqQtGui
{
// Internal data classes for ContactList
class ContactUserData;
class ContactUser;
class ContactBar;
class ContactGroup;

/**
 * A QT-model representation of the licq contact list
 *
 * The contact list is presented as a two level tree structure with the groups
 * as top level items and contacts located in the groups.
 * Each group also contains separator bars as items together with the contacts.
 * Special groups like "Invisible", "Online Notify" and "All Users" are present
 * as normal groups but with special group IDs.
 *
 * Before this model can be used by a view a proxy model is needed to sort the
 * list and to hide the special groups, ignored contacts and offline contacts
 * depending on the view mode.
 * The separator bars also needs to be sorted with the users for correct
 * placement in the view or hidden if they are not used.
 *
 * To present a view with a custom list of contacts use a proxy model that
 * filters items from the "All Users" special group.
 *
 * The classes holding the actual data for the contact list is located in
 * contactlistdata.h as they are internal to the model and does not need to be
 * visible to other classes.
 */
class ContactListModel : public QAbstractItemModel
{
  Q_OBJECT

public:

  /**
   * Roles added for non visual data needed to use the contact list
   */
  enum DataRole
  {
    ItemTypeRole = Qt::UserRole,        // Type of item (one of enum ItemType)
    NameRole,                           // Item name (alias for UserItems)
    SortPrefixRole,                     // Primary sort index (UserItems only)
    SortRole,                           // Sort index (secondary index for UserItems, only index for GroupItems)
    UnreadEventsRole,                   // Number of unread events
    EventSubCommandRole,                // Type of event (UserItems only)
    GroupIdRole,                        // Id for groups, parent groups for other items
    SubGroupRole,                       // Sub group type (one of enum SubGroupType) (UserItems and BarItems only)
    UserCountRole,                      // Number of users in this group (GroupItems and BarItems only)
    UserIdRole,                         // Id for user (UserItems only)
    PpidRole,                           // Protocol id for user (UserItems only)
    StatusRole,                         // Contact status (UserItems only)
    FullStatusRole,                     // Contact full status (UserItems only)
    ExtendedStatusRole,                 // Various status flags needed by the delegate (UserItems only)
    UserIconRole,                       // User picture for use as icon (UserItems only)
    CarAnimationRole,                   // Auto response read animation counter (UserItems only)
    OnlineAnimationRole,                // Online animation counter (UserItems only)
    EventAnimationRole,                 // Unread event animation counter (UserItems only)
    VisibilityRole,                     // Item should always be visible (UserItems and GroupItems only)
  };

  /**
   * Contact list item types (returned for ItemTypeRole)
   */
  enum ItemType
  {
    InvalidItem = 0,
    GroupItem,
    BarItem,
    UserItem
  };

  /**
   * Separator bar types (returned for SubGroupRole)
   */
  enum SubGroupType
  {
    OnlineSubGroup = 0,
    OfflineSubGroup,
    NotInListSubGroup,
  };

  /**
   * Contact status (returned for StatusRole)
   */
  enum StatusType
  {
    OfflineStatus = ICQ_STATUS_OFFLINE,
    OnlineStatus = ICQ_STATUS_ONLINE,
    AwayStatus = ICQ_STATUS_AWAY,
    DoNotDisturbStatus = ICQ_STATUS_DND,
    NotAvailableStatus = ICQ_STATUS_NA,
    OccupiedStatus = ICQ_STATUS_OCCUPIED,
    FreeForChatStatus = ICQ_STATUS_FREEFORCHAT
  };

  /**
   * Bit values for flags in ExtendedStatusRole data
   * Mainly used for extended icons in delegate
   */
  enum ExtendedStatusBit
  {
    PhoneStatusBit = 0,
    CellularStatusBit,
    BirthdayStatusBit,
    InvisibleStatusBit,
    GpgKeyStatusBit,
    GpgKeyEnabledStatusBit,
    PhoneFollowMeActiveStatusBit,
    PhoneFollowMeBusyStatusBit,
    IcqPhoneActiveStatusBit,
    IcqPhoneBusyStatusBit,
    SharedFilesStatusBit,
    TypingStatusBit,
    SecureStatusBit,
    CustomArStatusBit,
    IgnoreStatusBit,
    OnlineNotifyStatusBit,
    NotInListStatusBit,
    InvisibleListStatusBit,
    VisibleListStatusBit,
    NewUserStatusBit,
    AwaitingAuthStatusBit,
  };
  static const unsigned long PhoneStatus                = 1 << PhoneStatusBit;
  static const unsigned long CellularStatus             = 1 << CellularStatusBit;
  static const unsigned long BirthdayStatus             = 1 << BirthdayStatusBit;
  static const unsigned long InvisibleStatus            = 1 << InvisibleStatusBit;
  static const unsigned long GpgKeyStatus               = 1 << GpgKeyStatusBit;
  static const unsigned long GpgKeyEnabledStatus        = 1 << GpgKeyEnabledStatusBit;
  static const unsigned long PhoneFollowMeActiveStatus  = 1 << PhoneFollowMeActiveStatusBit;
  static const unsigned long PhoneFollowMeBusyStatus    = 1 << PhoneFollowMeBusyStatusBit;
  static const unsigned long IcqPhoneActiveStatus       = 1 << IcqPhoneActiveStatusBit;
  static const unsigned long IcqPhoneBusyStatus         = 1 << IcqPhoneBusyStatusBit;
  static const unsigned long SharedFilesStatus          = 1 << SharedFilesStatusBit;
  static const unsigned long TypingStatus               = 1 << TypingStatusBit;
  static const unsigned long SecureStatus               = 1 << SecureStatusBit;
  static const unsigned long CustomArStatus             = 1 << CustomArStatusBit;
  static const unsigned long IgnoreStatus               = 1 << IgnoreStatusBit;
  static const unsigned long OnlineNotifyStatus         = 1 << OnlineNotifyStatusBit;
  static const unsigned long NotInListStatus            = 1 << NotInListStatusBit;
  static const unsigned long InvisibleListStatus        = 1 << InvisibleListStatusBit;
  static const unsigned long VisibleListStatus          = 1 << VisibleListStatusBit;
  static const unsigned long NewUserStatus              = 1 << NewUserStatusBit;
  static const unsigned long AwaitingAuthStatus         = 1 << AwaitingAuthStatusBit;

  /**
   * Offset on group id for system groups
   */
  static const unsigned short SystemGroupOffset = 1000;

  /**
   * Constructor
   * Will get the current list from the daemon and connect to the signal manager for updates.
   *
   * @param parent Parent object
   */
  ContactListModel(QObject* parent = 0);

  /**
   * Destructor
   */
  virtual ~ContactListModel();

  /**
   * Refresh data and group membership for a user
   * As the daemon does not signal some things the main window must use this function to notify us.
   *
   * @param id Licq user id
   * @param ppid Licq protocol id
   */
  void updateUser(QString id, unsigned long ppid);

  /**
   * Add a user to the contact list
   *
   * @param licqUser The user to add
   */
  void addUser(const ICQUser* licqUser);

  /**
   * Remove a user from the contact list
   *
   * @param id Licq user id
   * @param ppid Licq protocol id
   */
  void removeUser(QString id, unsigned long ppid);

  /**
   * Removes (and delete) all users and groups from the list
   */
  void clear();

  /**
   * Get a model index for a group or user that other components can use.
   *
   * @param row Group index or user index within a group
   * @param column A valid column for the model
   * @param parent An existing group to get a user or an invalid model index to get a group
   * @return A model index for the requested group or user
   */
  virtual QModelIndex index(int row, int column, const QModelIndex& parent) const;

  /**
   * Get the parent of a model index
   *
   * @param index An index for a group or user
   * @return A group index if a user is provided or an invalid index if a group is provided
   */
  virtual QModelIndex parent(const QModelIndex& index) const;

  /**
   * Get the number of groups or users in a group
   *
   * @param parent An index for a group or an invalid index
   * @return The number of users in the group or the number of groups
   */
  virtual int rowCount(const QModelIndex& parent) const;

  /**
   * Get the number of columns for a user or a group
   *
   * @param parent An index in the model
   * @return The number of columns
   */
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;

  /**
   * Get data for a user or a group
   *
   * @param index An index of a user or a group
   * @param role The qt role to get data for
   * @return The data for the given item and role
   */
  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

  /**
   * Get item flags for an item
   *
   * @param index An index of a user or group
   * @return The item flags for the user or group
   */
  virtual Qt::ItemFlags flags(const QModelIndex& index) const;

  /**
   * Get header titles for the contact list
   *
   * @param section A column in the list
   * @param orientation Specify the horizontal or vertical header
   * @param role The qt role to get data for
   * @return Header data for the given column and role
   */
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

  /**
   * Set item data
   *
   * @param index Index for the item to update
   * @param value Value to set
   * @param role Role to update
   */
  virtual bool setData(const QModelIndex& index, const QVariant& value, int role = ContactListModel::NameRole);

  /**
   * Get index for a specific user
   *
   * @param id Licq user id
   * @param ppid Licq protocol id
   * @param column The column to return an index for
   * @return An index for the given user and column from the "All Users" group
   */
  QModelIndex userIndex(QString id, unsigned long ppid, int column) const;

  /**
   * Get index for a group to use as root item for a view
   * Requesting group id 0 will return either the all users group (if type is system) or other users group (if type is user)
   *
   * @param type The type of group (user or system)
   * @param id Id of the group or 0 to get special group
   * @return An index for the group or an invalid index if the group does not exist
   */
  QModelIndex groupIndex(GroupType type, unsigned long id) const;

  /**
   * Get index for a group. This function uses model id for groups
   * Requesting group id 0 will return other users group.
   *
   * @param id Id of the group or 0 to get other users group
   * @return An index for the group or an invalid index if the group does not exist
   */
  QModelIndex groupIndex(unsigned long id) const;

public slots:
  /**
   * The daemon list has changed
   *
   * @param sig Signal data from the daemon with information on what has changed
   */
  void listUpdated(CICQSignal* sig);

  /**
   * The data for a user has changed in the daemon
   *
   * @param sig Signal data from the daemon with information on what has changed
   */
  void userUpdated(CICQSignal* sig);

  /**
   * Reload the entire contact list from the daemon
   */
  void reloadAll();

private slots:
  /**
   * Update everything related to GUI configuration
   */
  void configUpdated();

  /**
   * The model data for a user has changed
   * Will send a dataChanged signal for the user in all groups it is present
   *
   * @param user The user data object that has changed
   */
  void userDataChanged(const ContactUserData* user);

  /**
   * The model data for a group has changed
   * Will send a dataChanged signal for the group
   *
   * @param group The group object that has changed
   */
  void groupDataChanged(ContactGroup* group);

  /**
   * The model data for a bar has changed
   * Will send a dataChanged signal for the bar
   *
   * @param bar The bar object that has changed
   * @param row The row of the bar in it's group
   */
  void barDataChanged(ContactBar* bar, int row);

  /**
   * A group is about to add a user
   * Will send beginInsertRow signal for the user
   *
   * @param group The group that's adding the user
   * @param row The row the new user will have in the group
   */
  void groupBeginInsert(ContactGroup* group, int row);

  /**
   * A group has finished adding a user
   * Will send endInsertRow signal
   */
  void groupEndInsert();

  /**
   * A group is about to remove a user
   * Will send beginRemoveRow signal for the user
   *
   * @param group The group that's removing the user
   * @param row The row of the user about to be removed
   */
  void groupBeginRemove(ContactGroup* group, int row);

  /**
   * A group has finished removing a user
   * Will send endRemoveRow signal
   */
  void groupEndRemove();

  /**
   * Update the user membership in all groups
   * This is triggered by the user data object when its group membership (may) have changed
   *
   * @param user The model user to update groups for
   * @param licqUser The daemon user to get group membership from
   */
  void updateUserGroups(ContactUserData* user, const ICQUser* licqUser);

private:
  /**
   * Connect signals for a newly created group object
   *
   * @param group Group object to connect signals from
   */
  void connectGroup(ContactGroup* group);

  /**
   * Get the user object that represents an licq contact
   *
   * @param id Licq user id
   * @param ppid Licq protocol id
   * @return The user object or 0 if it was not found
   */
  ContactUserData* findUser(QString id, unsigned long ppid) const;

  /**
   * Check if a user is member of a group and add/remove the user to/from the group if needed
   *
   * @param user The user to update
   * @param group The group to check
   * @param shouldBeMember True to add the user if missing or false to remove if member
   */
  void updateUserGroup(ContactUserData* user, ContactGroup* group, bool shouldBeMember);

  /**
   * Get model row for a group
   *
   * @param group A group object
   * @return Row in model or -1 if not found
   */
  int groupRow(ContactGroup* group) const;

  QList<ContactGroup*> myUserGroups;
  ContactGroup* mySystemGroups[NUM_GROUPS_SYSTEM_ALL];
  QList<ContactUserData*> myUsers;
  int myColumnCount;
  bool myBlockUpdates;
};

} // namespace LicqQtGui

#endif
