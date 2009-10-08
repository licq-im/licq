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

#ifndef OUTPUTWIN_H
#define OUTPUTWIN_H

#include <qwidget.h>
#include <qpushbutton.h>
#include <qsocketnotifier.h>

#include "mledit.h"
#include "licq_log.h"
#include "licqdialog.h"


class CQtLogWindow : public LicqDialog, public CPluginLog
{
  Q_OBJECT
public:
  CQtLogWindow (QWidget *parent = 0);
protected:
  MLEditWrap* outputBox;
  QPushButton *btnHide, *btnClear, *btnSave;
  QSocketNotifier *sn;
protected slots:
  void slot_log(int);
  void slot_save();

  virtual void showEvent(QShowEvent*);
};

#endif
