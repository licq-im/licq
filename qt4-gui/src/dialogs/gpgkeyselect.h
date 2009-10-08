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

#ifndef GPGKEYSELECT_H
#define GPGKEYSELECT_H

#include <QDialog>
#include <QTreeWidget>

#include <licq_types.h>

class QCheckBox;
class QLineEdit;

class LicqUser;

namespace LicqQtGui
{
class KeyView : public QTreeWidget
{
  Q_OBJECT

public:
  KeyView(const UserId& userId, QWidget* parent = 0);
  ~KeyView() {};

private:
  UserId myUserId;
  void testViewItem(QTreeWidgetItem* item, const LicqUser* u);
  int maxItemVal;
  QTreeWidgetItem* maxItem;
  void initKeyList();
  virtual void resizeEvent(QResizeEvent* event);
};

class GPGKeySelect : public QDialog
{
  Q_OBJECT
public:
  GPGKeySelect(const UserId& userId, QWidget* parent = 0);
  ~GPGKeySelect();

signals:
  void signal_done();

private:
  QTreeWidget* keySelect;
  QCheckBox* useGPG;
  UserId myUserId;
  void updateIcon();
  QLineEdit* filterText;

private slots:
  void slot_ok();
  void slotNoKey();
  void slotCancel();
  void slot_doubleClicked(QTreeWidgetItem* item, int column);
  void filterTextChanged(const QString& str);
};

} // namespace LicqQtGui

#endif
