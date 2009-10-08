// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2009 Licq developers
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

#ifndef USERVIEW_H
#define USERVIEW_H

#include "contactlist/contactlist.h"

#include "userviewbase.h"

class QTimer;

namespace LicqQtGui
{

/**
 * Contact list view that can show either single groups or a threaded view
 */
class UserView : public UserViewBase
{
  Q_OBJECT

public:
  /**
   * Constructor
   *
   * @param contactList The contact list instance
   * @param parent Parent object
   */
  UserView(ContactListModel* contactList, QWidget* parent = 0);

  /**
   * Destructor
   */
  virtual ~UserView();

  /**
   * Get the currently selected user
   *
   * @return id of current user
   */
  UserId currentUserId() const;

  /**
   * Set skin colors
   *
   * @param back Background color
   */
  virtual void setColors(QColor back);

public slots:
  /**
   * Set current view to display
   */
  void updateRootIndex();

  /**
   * Update expanded status of all groups (only relevant for threaded view mode)
   */
  void expandGroups();

  /**
   * Reset internal state of view
   * Overloaded to restore group states after QTreeView has lost them
   */
  virtual void reset();

protected slots:
  /**
   * Apply new skin
   */
  virtual void applySkin();

  /**
   * Overload the base class so we can analyze the rows
   * which were inserted and act accordingly
   *
   * @param parent The view index which was populated
   * @param start The number of the first inserted row
   * @param end The number of the last inserted row
   */
  virtual void rowsInserted(const QModelIndex& parent, int start, int end);

  /**
   * Overloaded the base class so we can analyze the rows
   * which are removed and act accordingly
   *
   * @param parent The view index which will loose a child
   * @param start The number of the first row to be removed
   * @param end The number of the last row to be removed
   */
  virtual void rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);

private:
  /**
   * Mouse button was pressed
   *
   * @param event Mouse event
   */
  virtual void mousePressEvent(QMouseEvent* event);

  /**
   * Key was pressed
   *
   * @param event Key event
   */
  virtual void keyPressEvent(QKeyEvent* event);

  /**
   * Mouse was moved
   *
   * @param event Mouse event
   */
  virtual void mouseMoveEvent(QMouseEvent* event);

  /**
   * Sets row spanning for particular rows in the given group
   *
   * @param parent The view index to analyze
   * @param start The count of the first row
   * @param end The count of the last row
   */
  void spanRowRange(const QModelIndex& parent, int start, int end);

  UserId myRemovedUser;
  QTimer* myRemovedUserTimer;

private slots:
  /**
   * Tell proxy to resort list according to current settings
   */
  void resort();

  /**
   * A group has been expanded
   *
   * @param index Model index of the expanded group
   */
  void slotExpanded(const QModelIndex& index);

  /**
   * A group has been collapsed
   *
   * @param index Model index of the expanded group
   */
  void slotCollapsed(const QModelIndex& index);

  /**
   * A header has been clicked and the list should be resorted
   *
   * @param column Number of the column to sort by
   */
  void slotHeaderClicked(int column);

  /**
   * Reload config settings
   */
  void configUpdated();

  /**
   * Forget that we have remembered a removed user
   */
  void forgetRemovedUser();
};

} // namespace LicqQtGui

#endif
