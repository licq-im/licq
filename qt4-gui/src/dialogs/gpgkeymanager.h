/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2005-2006 Licq developers
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

#include <QDialog>
#include <QTreeWidget>

class ICQUser;

namespace LicqQtGui
{
class GPGKeySelect;
class KeyList;

class GPGKeyManager : public QDialog
{
  Q_OBJECT

public:
  GPGKeyManager(QWidget* parent = 0);

private:
  KeyList* lst_keyList;
  void initKeyList();

private slots:
  void slot_add();
  void slot_edit();
  void slot_remove();
  void slot_doubleClicked(QTreeWidgetItem* item);
};

class KeyList : public QTreeWidget
{
  Q_OBJECT

public:
  KeyList(QWidget* parent = 0);

  void editUser(QString id, unsigned long ppid);
  void resizeColumnsToContents();

private:
  void dragEnterEvent(QDragEnterEvent* event);
  void dragMoveEvent(QDragMoveEvent* /* event */) {};
  void dropEvent(QDropEvent* event);
  virtual void resizeEvent(QResizeEvent* event);
};

class KeyListItem : public QObject, public QTreeWidgetItem
{
  Q_OBJECT

public:
  KeyListItem(QTreeWidget* parent, const ICQUser* u);

  void edit();
  void unsetKey();

  QString getszId() { return szId; };
  unsigned long getnPPID() { return nPPID; };

private:
  QString szId;
  unsigned long nPPID;
  GPGKeySelect* keySelect;
  void updateText(const ICQUser* u);

private slots:
  void slot_done();
};

} // namespace LicqQtGui

#endif
