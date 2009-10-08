/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2009 Licq developers
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

#ifndef AWAYMSG_H
#define AWAYMSG_H

#include <licq_types.h>

#include "licqdialog.h"

class QPushButton;
class QComboBox;
class MLEditWrap;

class AwayMsgDlg : public LicqDialog
{
  Q_OBJECT
public:
  AwayMsgDlg(QWidget *parent = 0);
  ~AwayMsgDlg();
  void SelectAutoResponse(unsigned short status, bool autoclose = false);

protected:
  MLEditWrap *mleAwayMsg;
  QPopupMenu* mnuSelect;
  QPushButton* btnSelect;
  QPushButton *btnOk, *btnCancel;
  unsigned short m_nStatus;
  static QPoint snPos;
  short m_nSAR;
  int m_autocloseCounter;

  void closeEvent(QCloseEvent *);
  bool eventFilter(QObject *, QEvent *);

signals:
  void popupOptions(int);
  void done();

protected slots:
  void ok();
  virtual void reject();
  void slot_selectMessage(int);
  void slot_hints();
  void slot_autocloseTick();
  void slot_autocloseStop();
};

class CustomAwayMsgDlg : public LicqDialog
{
  Q_OBJECT
public:
  CustomAwayMsgDlg(const UserId& userId, QWidget *parent = 0);

protected:
  MLEditWrap *mleAwayMsg;
  UserId myUserId;

protected slots:
  void slot_clear();
  void slot_ok();
  void slot_hints();
};


#endif

