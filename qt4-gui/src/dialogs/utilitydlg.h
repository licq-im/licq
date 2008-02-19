/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2006 Licq developers
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

#include <vector>

#include <QDialog>

class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSocketNotifier;
class QSplitter;

class CUtility;
class CUtilityInternalWindow;

namespace LicqQtGui
{
class InfoField;
class MLEdit;

class UtilityDlg : public QDialog
{
  Q_OBJECT

public:
  UtilityDlg(CUtility* u, const QString& szId, unsigned long nPPID);
  ~UtilityDlg();

private:
  CUtility* m_xUtility;
  QString m_szId;
  unsigned long m_nPPID;
  bool m_bIntWin, m_bStdOutClosed, m_bStdErrClosed;
  CUtilityInternalWindow* intwin;

  QLabel* lblUtility;
  InfoField* nfoUtility;
  InfoField* nfoWinType;
  InfoField* nfoDesc;
  QCheckBox* chkEditFinal;
  QGroupBox* boxFields;
  std::vector<QLabel*> lblFields;
  std::vector<QLineEdit*> edtFields;
  QPushButton* btnRun;
  QPushButton* btnCancel;
  MLEdit* mleOut;
  MLEdit* mleErr;
  QSocketNotifier* snOut;
  QSocketNotifier* snErr;
  QSplitter* splOutput;

  void CloseInternalWindow();

private slots:
  void slot_run();
  void slot_cancel();
  void slot_stdout();
  void slot_stderr();
};

} // namespace LicqQtGui

#endif
