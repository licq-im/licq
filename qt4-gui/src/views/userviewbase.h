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

#ifndef USERVIEWBASE_H
#define USERVIEWBASE_H

#include <QTreeView>

#include <licq_types.h>

class QAbstractProxyModel;

namespace LicqQtGui
{

class ContactListModel;

/**
 * Base class for contact views
 *
 * Contains common functionallity for the views such as accepting dragged items
 */
class UserViewBase : public QTreeView
{
  Q_OBJECT

public:
  /**
   * Constructor
   *
   * @param contactList The contact list instance
   * @param parent Parent object
   */
  UserViewBase(ContactListModel* contactList, QWidget* parent = 0);

  /**
   * Destructor
   */
  virtual ~UserViewBase();

  /**
   * Set skin colors
   *
   * @param back Background color
   */
  virtual void setColors(QColor back);

  /**
   * Make sure a specified index is visible
   * Overloaded to stop annoying auto scrolling triggered by layoutChanged
   *
   * @param index Index to scroll to
   * @param hint Where to place the index
   */
  virtual void scrollTo(const QModelIndex& index, ScrollHint hint = EnsureVisible);

  /**
   * Open editor for group name
   *
   * @param groupId Id of group to edit
   * @param online True to put editor at single or online instance of group
   */
  void editGroupName(int groupId, bool online);

signals:
  /**
   * Signal emitted when user has double clicked on a contact
   *
   * @param userId User id of contact
   */
  void userDoubleClicked(const UserId& userId);

protected slots:
  /**
   * Apply new skin
   */
  virtual void applySkin();

  /**
   * Current index has changed
   * Overloaded as workaround for scrollTo()
   *
   * @param current New current index
   * @param previous Previously current index
   */
  virtual void currentChanged(const QModelIndex& current, const QModelIndex& previous);

protected:
  /**
   * Mouse button was pressed
   *
   * @param event Mouse event
   */
  virtual void mousePressEvent(QMouseEvent* event);

  /**
   * Mouse button was released
   *
   * @param event Mouse event
   */
  virtual void mouseReleaseEvent(QMouseEvent* event);

  /**
   * Context menu should be displayed
   *
   * @param event Menu event
   */
  virtual void contextMenuEvent(QContextMenuEvent* event);

  /**
   * Show popup menu for an item
   *
   * @param point Coordinate to show menu from
   * @param item Index to show menu for
   */
  void popupMenu(QPoint point, QModelIndex item);

  /**
   * Item is being dragged
   *
   * @param event Drag event
   */
  virtual void dragEnterEvent(QDragEnterEvent* event);

  /**
   * Item was dropped
   *
   * @param event Drag event
   */
  virtual void dropEvent(QDropEvent* event);

  /**
   * Dragged item has moved
   *
   * @param event Drag event
   */
  virtual void dragMoveEvent(QDragMoveEvent* event);

  ContactListModel* myContactList;
  QAbstractProxyModel* myListProxy;
  QPoint myMousePressPos;

  /**
   * Overload branches drawing method with an empty one
   * to avoid unnecessary Qt processing
   */
  virtual void drawBranches(QPainter*, const QRect&, const QModelIndex&) const {}

  /**
   * A timer event happened
   * Overloaded as workaround for scrollTo()
   *
   * @param event Timer event
   */
  virtual void timerEvent(QTimerEvent* event);

private slots:
  /**
   * User double clicked in list
   *
   * @param index Item index that was double clicked
   */
  void slotDoubleClicked(const QModelIndex& index);

private:
  bool midEvent;
  bool myAllowScrollTo;
};

} // namespace LicqQtGui

#endif
