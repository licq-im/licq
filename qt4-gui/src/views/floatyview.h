// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2006 Licq developers
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

#ifndef FLOATYVIEW_H
#define FLOATYVIEW_H

#include "userviewbase.h"

class ICQUser;

namespace LicqQtGui
{
class FloatyView;

typedef QVector<FloatyView*> UserFloatyList;

/**
 * Separate window that shows a single user
 */
class FloatyView : public UserViewBase
{
  Q_OBJECT

public:
  /**
   * Constructor
   *
   * @param contactList The contact list instance
   * @param licqUser The daemon contact to create floaty for
   * @param parent Parent object
   */
  FloatyView(ContactListModel* contactList, const ICQUser* licqUser, QWidget* parent = 0);

  /**
   * Destructor
   */
  virtual ~FloatyView();

  /**
   * Return the user protocol id
   */
  unsigned long ppid() const { return myPpid; }

  /**
   * Return the user id
   */
  QString id() const { return myId; }

  /**
   * Find the floaty for a user
   *
   * @param id User id
   * @param ppid Protocol id
   * @return The floaty for the user or null if no floaty exists for the user
   */
  static FloatyView* findFloaty(QString id, unsigned long ppid);

  // List of existing floaties
  static UserFloatyList floaties;

private:
  /**
   * Mouse was moved
   *
   * @param event Mouse event
   */
  virtual void mouseMoveEvent(QMouseEvent* event);

  QString myId;
  unsigned long myPpid;

private slots:
  /**
   * Gets called when contact list config was updated
   */
  void configUpdated();
};

} // namespace LicqQtGui

#endif
