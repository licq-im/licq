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

#ifndef RCDLG_H
#define RCDLG_H

#include "config.h"

#include <QDialog>

class QListWidget;
class QPushButton;

namespace Licq
{
class Event;
}

namespace LicqQtGui
{
class RandomChatDlg : public QDialog
{
  Q_OBJECT

public:
  RandomChatDlg(QWidget* parent = 0);
  ~RandomChatDlg();

private:
  QListWidget* myGroupsList;
  QPushButton* myOkButton;
  QPushButton* myCancelButton;
  unsigned long myTag;

private slots:
  void okPressed();
  void userEventDone(const Licq::Event* event);
};

class SetRandomChatGroupDlg : public QDialog
{
  Q_OBJECT

public:
  SetRandomChatGroupDlg(QWidget* parent = 0);
  ~SetRandomChatGroupDlg();

private:
  QListWidget* myGroupsList;
  QPushButton* myOkButton;
  QPushButton* myCancelButton;
  unsigned long myTag;

private slots:
  void okPressed();
  void userEventDone(const Licq::Event* event);
};

} // namespace LicqQtGui

#endif
