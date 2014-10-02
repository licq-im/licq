/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2010 Licq developers
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

#ifndef MMUSERVIEW_H
#define MMUSERVIEW_H

#include "userviewbase.h"

#include <set>

#include <licq/userid.h>

class QMenu;


namespace LicqQtGui
{
class ContactListModel;;

class MMUserView : public UserViewBase
{
  Q_OBJECT
public:
  MMUserView(const Licq::UserId& userId, ContactListModel* contactList, QWidget* parent = 0);
  virtual ~MMUserView();

  const std::set<Licq::UserId>& contacts() const;

  void add(const Licq::UserId& userId);
  void removeFirst();

public slots:
  void clear();

private:
  QMenu* myMenu;
  Licq::UserId myUserId;

  virtual void mousePressEvent(QMouseEvent* event);
  virtual void keyPressEvent(QKeyEvent* event);
  virtual void dragEnterEvent(QDragEnterEvent* event);
  virtual void dropEvent(QDropEvent* event);
  virtual void contextMenuEvent(QContextMenuEvent* event);

private slots:
  void remove();
  void crop();
  void addCurrentGroup();
  void addAll();
};

} // namespace LicqQtGui

#endif
