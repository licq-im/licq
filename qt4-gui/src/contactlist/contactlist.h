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

#ifndef CONTACTLISTMODEL_H
#define CONTACTLISTMODEL_H

#include <QAbstractItemModel>
#include <QList>

#include <licq/userid.h>

namespace Licq
{
class User;
}

// Allow UserId to be used in QVariant and QSet
Q_DECLARE_METATYPE(Licq::UserId)
uint qHash(const Licq::UserId& userId);

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
    AccountIdRole,                      // Account id for user (UserItems only)
    PpidRole,                           // Protocol id for user (UserItems only)
    StatusRole,                         // Contact status (UserItems only)
    ExtendedStatusRole,                 // Various status flags needed by the delegate (UserItems only)
    UserIconRole,                       // User picture for use as icon (UserItems only)
    CarAnimationRole,                   // Auto response read animation counter (UserItems only)
    OnlineAnimationRole,                // Online animation counter (UserItems only)
    EventAnimationRole,                 // Unread event animation counter (UserItems only)
    VisibilityRole,                     // Item should always be visible
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

  // Constants for system groups
  static const int SystemGroupOffset = 1000;
  static const int OtherUsersGroupId = 0;

  // "Normal" system groups
  // Note: These constants are used by Config::Contactlist and written to config file
  //       Changing existing numbers will make old config files be read wrong
  static const int OnlineNotifyGroupId                  = SystemGroupOffset + 0;
  static const int VisibleListGroupId                   = SystemGroupOffset + 1;
  static const int InvisibleListGroupId                 = SystemGroupOffset + 2;
  static const int IgnoreListGroupId                    = SystemGroupOffset + 3;
  static const int NewUsersGroupId                      = SystemGroupOffset + 4;
  static const int AwaitingAuthGroupId                  = SystemGroupOffset + 5;
  static const int NumSystemGroups                      = 6;
  static const int LastSystemGroup                      = SystemGroupOffset + NumSystemGroups - 1;
  static const int AllUsersGroupId                      = SystemGroupOffset + 100;

  // Not real group but need unique id in menus
  static const int AllGroupsGroupId                     = SystemGroupOffset + 101;

  /**
   * Get display name for system groups
   *
   * @param groupId Id of a system group
   * @return Name of group
   */
  static QString systemGroupName(int groupId);

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
   * Add a user to the contact list
   *
   * @param licqUser The user to add
   */
  void addUser(const Licq::User* licqUser);

  /**
   * Remove a user from the contact list
   *
   * @param userId Licq user id
   */
  void removeUser(const Licq::UserId& userId);

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
   * @param userId Licq user id
   * @param column The column to return an index for
   * @return An index for the given user and column from the "All Users" group
   */
  QModelIndex userIndex(const Licq::UserId& userId, int column) const;

  /**
   * Get index for a group to use as root item for a view
   *
   * @param id Id of the group
   * @return An index for the group or an invalid index if the group does not exist
   */
  QModelIndex groupIndex(int id) const;

  /**
   * Get index for the All Users group
   *
   * @return Index for "All Users" group
   */
  QModelIndex allUsersGroupIndex() const
  { return groupIndex(AllUsersGroupId); }

  /**
   * Convenience function to get name of a group
   *
   * @param groupId Id of user group or system group
   * @return Name of group
   */
  QString groupName(int groupId) const;

public slots:
  /**
   * The daemon list has changed
   *
   * @param subSignal Sub signal telling what the change was
   * @param argument Additional data, usage depend on sub signal type
   * @param userId Id for affected user, if applicable
   */
  void listUpdated(unsigned long subSignal, int argument, const Licq::UserId& userId);

  /**
   * The data for a user has changed in the daemon
   *
   * @param userId Id for affected user
   * @param subSignal Sub signal telling what the change was
   * @param argument Additional data, usage depend on sub signal type
   */
  void userUpdated(const Licq::UserId& userId, unsigned long subSignal, int argument);

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
  void updateUserGroups(ContactUserData* user, const Licq::User* licqUser);

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
   * @param userId Licq user id
   * @return The user object or NULL if it was not found
   */
  ContactUserData* findUser(const Licq::UserId& userId) const;

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

  QList<ContactGroup*> myGroups;
  ContactGroup* myAllUsersGroup;
  QList<ContactUserData*> myUsers;
  int myColumnCount;
  bool myBlockUpdates;
};

extern ContactListModel* gGuiContactList;

} // namespace LicqQtGui

#endif
