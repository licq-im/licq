/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2005-2009 Licq developers
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

#ifndef GPGKEYMANAGER_H
#define GPGKEYMANAGER_H

#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlistview.h>

#include "licqdialog.h"
#include "licq_user.h"
#include "licq_events.h"
#include "mainwin.h"
#include "gpgkeyselect.h"

class KeyList;

class GPGKeyManager : public LicqDialog
{
  Q_OBJECT

public:
  GPGKeyManager( QWidget *_parent = 0 );
  ~GPGKeyManager();

protected:
  KeyList *lst_keyList;
  void editUser(const LicqUser* u);
  void initKeyList();

protected slots:
  void slot_setPassphrase();
  void slot_close();
  void slot_add();
  void slot_edit();
  void slot_remove();
  void slot_doubleClicked( QListViewItem *item, const QPoint &point, int i);

private:
  QLabel *lbl_passphrase, *lbl_dragndrop;
};

class KeyList : public QListView
{
public:
  KeyList( QWidget *_parent );
  ~KeyList();
  
protected:
  void dragEnterEvent( QDragEnterEvent *event );
  void dropEvent( QDropEvent *de );
  virtual void resizeEvent( QResizeEvent *e );
};

class KeyListItem : public QObject, public QListViewItem
{
  Q_OBJECT
public:
  KeyListItem(QListView* parent, const LicqUser* u);
  ~KeyListItem();

  void edit();
  void unsetKey();

  const UserId& userId() const { return myUserId; }

protected:
  UserId myUserId;
  GPGKeySelect *keySelect;
  void updateText(const LicqUser* u);

protected slots:
  void slot_done();
};

#endif //GPGKEYMANAGER_H
