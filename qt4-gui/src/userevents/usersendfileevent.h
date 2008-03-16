// -*- c-basic-offset: 2 -*-
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

#ifndef USERSENDFILEEVENT_H
#define USERSENDFILEEVENT_H

class QLabel;

#include <licq_filetransfer.h>

#include "usersendcommon.h"

namespace LicqQtGui
{

class UserSendFileEvent : public UserSendCommon
{
  Q_OBJECT

public:
  UserSendFileEvent(QString id, unsigned long ppid, QWidget* parent = 0);
  virtual ~UserSendFileEvent();

  void setFile(const QString& file, const QString& description);
  void addFile(const QString& file);

private:
  QLabel* myFileLabel;
  InfoField* myFileEdit;
  QPushButton* myBrowseButton;
  QPushButton* myEditButton;
  ConstFileList myFileList;

  virtual bool sendDone(ICQEvent* e);
  virtual void resetSettings();

private slots:
  void browseFile();
  void editFileList();
  void updateLabel(unsigned count);
  virtual void send();
};

} //namespace LicqQtGui

#endif
