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

#ifndef UTILITYDLG_H
#define UTILITYDLG_H

#include <stdio.h>

#include <qwidget.h>

#include <vector>

#include <licq_types.h>

class QCheckBox;
class QLineEdit;
class QLabel;
class QSocketNotifier;
class QGroupBox;
class QSplitter;

class CICQDaemon;
class CUtility;
class CInfoField;
class CUtilityInternalWindow;

class MLEditWrap;

class CUtilityDlg : public QWidget
{
  Q_OBJECT
public:
  CUtilityDlg(CUtility *u, const UserId& userId, CICQDaemon *server);
  ~CUtilityDlg();
protected:
  CUtility *m_xUtility;
  CICQDaemon *server;
  UserId myUserId;
  bool m_bIntWin, m_bStdOutClosed, m_bStdErrClosed;
  CUtilityInternalWindow *intwin;

  QLabel *lblUtility;
  CInfoField *nfoUtility, *nfoWinType, *nfoDesc;
  QCheckBox *chkEditFinal;
  QGroupBox *boxFields;
  std::vector <QLabel *> lblFields;
  std::vector <QLineEdit *> edtFields;
  QPushButton *btnRun, *btnCancel;
  MLEditWrap *mleOut, *mleErr;
  QSocketNotifier *snOut, *snErr;
  QSplitter *splOutput;

  void CloseInternalWindow();
protected slots:
  void slot_run();
  void slot_cancel();
  void slot_stdout();
  void slot_stderr();
};

#endif
