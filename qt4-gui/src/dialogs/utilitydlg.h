/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2010 Licq developers
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

#include <licq/userid.h>

class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSocketNotifier;
class QSplitter;


namespace Licq
{
class Utility;
class UtilityInternalWindow;
}

namespace LicqQtGui
{
class InfoField;
class MLEdit;

class UtilityDlg : public QDialog
{
  Q_OBJECT

public:
  UtilityDlg(Licq::Utility* u, const Licq::UserId& userId);
  ~UtilityDlg();

private:
  Licq::Utility* myUtility;
  Licq::UserId myUserId;
  bool m_bIntWin, m_bStdOutClosed, m_bStdErrClosed;
  Licq::UtilityInternalWindow* myInternalWindow;

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
