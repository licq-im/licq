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

#ifndef USERVIEWBASE_H
#define USERVIEWBASE_H

#include <QTreeView>

class QAbstractProxyModel;

namespace LicqQtGui
{

class ContactListModel;
class UserMenu;

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
   * @param mnuUser User menu instance
   * @param parent Parent object
   */
  UserViewBase(ContactListModel* contactList, UserMenu* mnuUser, QWidget* parent = 0);

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

signals:
  /**
   * Signal emitted when user has double clicked on a contact
   *
   * @param id User id of contact
   * @param ppid Protocol id of contact
   */
  void userDoubleClicked(QString id, unsigned long ppid);

protected slots:
  /**
   * Apply new skin
   */
  virtual void applySkin();

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
  UserMenu* myUserMenu;
  QPoint myMousePressPos;

  /**
   * Overload branches drawing method with an empty one
   * to avoid unnecessary Qt processing
   */
  virtual void drawBranches(QPainter*, const QRect&, const QModelIndex&) const {}

private slots:
  /**
   * User double clicked in list
   *
   * @param index Item index that was double clicked
   */
  void slotDoubleClicked(const QModelIndex& index);

private:
  bool midEvent;
};

} // namespace LicqQtGui

#endif
