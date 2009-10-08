/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2009 Licq developers
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

#ifndef MMBOX_H
#define MMBOX_H

#include <vector>
#include <list>

#include <qlistview.h>
#include <qtooltip.h>

#include <licq_types.h>

#include "userbox.h"

class LicqUser;
class CMainWindow;


//=====UserViewItem================================================================================
class CMMUserViewItem : public QListViewItem
{
public:
  CMMUserViewItem(const LicqUser* u , QListView *);
  virtual ~CMMUserViewItem();
  const UserId& userId() const { return myUserId; }

protected:
  UserId myUserId;

  friend class CMMUserView;
};


//=====MMUserView===============================================================
class CMMUserView : public QListView
{
  Q_OBJECT
public:
  CMMUserView(ColumnInfos &_colInfo, bool, const UserId& userId,
     CMainWindow *, QWidget *parent = 0);
  virtual ~CMMUserView();

  void AddUser(const UserId& userId);

protected:
  QPopupMenu *mnuMM;
  ColumnInfos colInfo;
  UserId myUserId;
  CMainWindow *mainwin;

  virtual void viewportMousePressEvent(QMouseEvent *e);
  virtual void keyPressEvent(QKeyEvent *e);
  virtual void dragEnterEvent(QDragEnterEvent * dee);
  virtual void dropEvent(QDropEvent * de);
  virtual void resizeEvent(QResizeEvent *);

  friend class CMMUserViewItem;

protected slots:
  void slot_menu(int);
};

#endif
