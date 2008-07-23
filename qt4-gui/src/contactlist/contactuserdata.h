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

#ifndef CONTACTUSERDATA_H
#define CONTACTUSERDATA_H

#include <QList>
#include <QString>
#include <QTimer>
#include <QVariant>

#include "contactitem.h"
#include "contactlist.h"

class QImage;

class ICQUser;

namespace LicqQtGui
{
class ContactUser;

/**
 * Data for a user, shared between all instances of ContactUser for a user
 *
 * This class is used internally by ContactList and should not be accessed from any other class
 */
class ContactUserData : public QObject
{
  Q_OBJECT

public:
  /**
   * Constructor
   *
   * @param licqUser Licq user that this object will represent
   * @param parent Object to use as parent for those objects that needs it
   */
  ContactUserData(const ICQUser* licqUser, QObject* parent);

  /**
   * Destructor, will remove the user from all groups
   */
  virtual ~ContactUserData();

  /**
   * Update user information
   *
   * @param sig Licq signal with information on what to update
   */
  void update(CICQSignal* sig);

  /**
   * Update all user information from daemon
   *
   * @param licqUser Licq user to read information from
   */
  void updateAll(const ICQUser* licqUser);

  /**
   * Update all data related to the gui configuration
   */
  void configUpdated();

  /**
   * Get licq user id
   */
  QString id() const
  { return myId; }

  /**
   * Get licq protocol id
   */
  unsigned long ppid() const
  { return myPpid; }

  /**
   * Get user status
   */
  unsigned short status() const
  { return myStatus; }

  /**
   * Get current sub group
   */
  ContactListModel::SubGroupType subGroup() const
  { return mySubGroup; }

  /**
   * Get number of unread events
   */
  int numEvents() const
  { return myEvents; }

  /**
   * Get current visibility
   */
  bool visibility() const
  { return myVisibility; }

  /**
   * Get a list of all the groups this user is a member of
   *
   * @return the list of all user instances belonging to this user
   */
  QList<ContactUser*> groupList() const
  { return myUserInstances; }

  /**
   * Add this user from a group
   * Note: Do not call this method directly, it should only be called from the user instance constructor.
   *
   * @param user The user instance to add
   */
  void addGroup(ContactUser* user);

  /**
   * Remove this user from a group
   * Note: Do not call this method directly, it should only be called from the user instance destructor.
   *
   * @param user The user instance to remove
   */
  void removeGroup(ContactUser* user);

  /**
   * Get data for this user
   *
   * @param column A valid column in the contact list
   * @param role The qt role to get data for
   * @return Data for this user
   */
  QVariant data(int column, int role) const;

  /**
   * Set data for this user
   * Currently only alias may be change this way
   *
   * @param value New value to set
   * @param role Must be ContactListModel::NameRole
   * @return True if alias was changed
   */
  virtual bool setData(const QVariant& value, int role = ContactListModel::NameRole);

signals:
  /**
   * Signal emitted when user data has changed
   */
  void dataChanged(const ContactUserData* user);

  /**
   * Signal emitted when the user group memberships (may) have changed
   */
  void updateUserGroups(ContactUserData* user, const ICQUser* licqUser);

private:
  /**
   * Update extended status bits
   */
  void updateExtendedStatus();

  /**
   * Update icons
   */
  void updateIcons();

  /**
   * Update sorting keys
   */
  void updateSorting();

  /**
   * Update the display text for the user
   *
   * @param licqUser Licq user to read information from
   * @return True if any data was actually changed
   */
  bool updateText(const ICQUser* licqUser);

  /**
   * Update visibility status
   */
  void updateVisibility();

  /**
   * Activate animation timer
   */
  void startAnimation();

  /**
   * Deactivate animation timer if it is no longer needed
   */
  void stopAnimation();

  /**
   * Generate a tooltip with user information
   */
  QString tooltip() const;

private slots:
  /**
   * Refresh content that may contain timestamps
   */
  void refresh();

  /**
   * Cycle animations
   */
  void animate();

private:
  QString myId;
  unsigned long myPpid;
  unsigned short myStatus;
  unsigned long myStatusFull;
  int myEvents;
  bool myStatusInvisible, myStatusTyping, myCustomAR, mySecure, myFlash;
  bool myBirthday, myPhone, myCellular, myGPGKey, myGPGKeyEnabled;
  bool myNewUser, myNotInList, myAwaitingAuth;
  bool myInIgnoreList, myInOnlineNotify, myInInvisibleList, myInVisibleList;
  time_t myTouched;
  unsigned short myNewMessages;
  unsigned short myEventSubCommand;
  unsigned long myPhoneFollowMeStatus, myIcqPhoneStatus, mySharedFilesStatus;
  unsigned int myExtendedStatus;
  ContactListModel::SubGroupType mySubGroup;
  QString mySortKey;
  bool myVisibility;

  bool myFlashCounter;
  int myOnlCounter, myCarCounter;
  bool myAnimating;

  QImage* myUserIcon;
  bool myUrgent;
  QString myText[4];
  QString myAlias;
  QList<ContactUser*> myUserInstances;

  static QTimer* myRefreshTimer;
  static QTimer* myAnimateTimer;
  static int myAnimatorCount;
};

} // namespace LicqQtGui

#endif
