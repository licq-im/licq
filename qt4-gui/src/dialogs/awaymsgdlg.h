/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2012 Licq developers <licq-dev@googlegroups.com>
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

#ifndef AWAYMSGDLG_H
#define AWAYMSGDLG_H

#include <QDialog>

#include <licq/userid.h>

class QCloseEvent;
class QDialogButtonBox;
class QEvent;
class QMenu;

namespace LicqQtGui
{
class MLEdit;

class AwayMsgDlg : public QDialog
{
  Q_OBJECT

public:
  /**
   * Create and show away message dialog or raise it if already exists
   *
   * @param status Status to prompt for away message to
   * @param autoClose True if dialog should close after a timeout
   * @param userId Owner to set status for or invalid to change globaly
   */
  static void showAwayMsgDlg(unsigned status, bool autoClose = false,
      const Licq::UserId& userId = Licq::UserId());

  static void showAutoResponseHints(QWidget* parent = 0);

private:
  static AwayMsgDlg* myInstance;

  AwayMsgDlg(QWidget* parent = 0);
  ~AwayMsgDlg();
  void selectAutoResponse(unsigned status, bool autoClose = false,
      const Licq::UserId& userId = Licq::UserId());

  MLEdit* myAwayMsg;
  QMenu* myMenu;
  QDialogButtonBox* myButtons;
  QString myOkText;

  unsigned myStatus;
  Licq::UserId myUserId;
  bool mySetStatus;
  int myAutoCloseCounter;

  bool eventFilter(QObject* watched, QEvent* event);

private slots:
  void ok();
  void autoCloseTick();
  void autoCloseStop();
  void hints();
  void selectMessage();
};

} // namespace LicqQtGui

#endif
