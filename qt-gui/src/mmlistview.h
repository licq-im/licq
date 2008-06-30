/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2006 Licq developers
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

#include "userbox.h"

class ICQUser;
class CMainWindow;


//=====UserViewItem================================================================================
class CMMUserViewItem : public QListViewItem
{
public:
  CMMUserViewItem (ICQUser *, QListView *);
  virtual ~CMMUserViewItem();
  char *Id()  { return m_szId; }
  unsigned long PPID()  { return m_nPPID; }

protected:
  char *m_szId;
  unsigned long m_nPPID;

  friend class CMMUserView;
};


//=====MMUserView===============================================================
class CMMUserView : public QListView
{
  Q_OBJECT
public:
  CMMUserView(ColumnInfos &_colInfo, bool, const char*, unsigned long,
     CMainWindow *, QWidget *parent = 0);
  virtual ~CMMUserView();

  void AddUser(unsigned long);
  void AddUser(const char *, unsigned long);

protected:
  QPopupMenu *mnuMM;
  ColumnInfos colInfo;
  char *m_szId;
  unsigned long m_nPPID;
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
