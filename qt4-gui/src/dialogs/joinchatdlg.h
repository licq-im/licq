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

#ifndef CHATJOINDLG_H
#define CHATJOINDLG_H

#include <QDialog>
#include <QListWidget>

#include "chatdlg.h"

class QPushButton;

namespace LicqQtGui
{
class JoinChatDlg : public QDialog
{
  Q_OBJECT

public:
  JoinChatDlg(bool bRequesting, QWidget *p = 0);
  ChatDlg* JoinedChat();
  QString ChatClients() { return lstChats->currentItem()->text(); }

private:
  QListWidget* lstChats;
  QPushButton* btnOk;
  QPushButton* btnCancel;
  ChatDlgList originalChats;

private slots:
  void slot_ok();
};

} // namespace LicqQtGui

#endif
