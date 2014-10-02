/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2011 Licq developers
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

#ifndef FORWARDDLG_H
#define FORWARDDLG_H

#include <QDialog>

#include <licq/userid.h>

class QPushButton;

namespace Licq
{
class UserEvent;
}

namespace LicqQtGui
{
class InfoField;

class ForwardDlg : public QDialog
{
  Q_OBJECT

public:
  ForwardDlg(Licq::UserEvent* e, QWidget* p = NULL);
  ~ForwardDlg();

private:
  virtual void dragEnterEvent(QDragEnterEvent* dee);
  virtual void dropEvent(QDropEvent* de);
  QString s1, s2;
  unsigned myEventType;
  InfoField* edtUser;
  QPushButton* btnOk;
  QPushButton* btnCancel;
  Licq::UserId myUserId;

private slots:
  void slot_ok();
};

} // namespace LicqQtGui

#endif
