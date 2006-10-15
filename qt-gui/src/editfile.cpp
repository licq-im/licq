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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unistd.h>

#include <qpushbutton.h>
#include <qlayout.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qfileinfo.h>

#include "editfile.h"
#include "mledit.h"
#include "ewidgets.h"
#include "licq_constants.h"

//---------------------------------------------------------------------------

EditFileDlg::EditFileDlg(QString fname, QWidget *parent)
  : QWidget(parent, "EditFileDialog", WDestructiveClose), sFile(fname)
{
  setCaption(tr("Licq File Editor - %1").arg(fname));

  QBoxLayout* top_lay = new QVBoxLayout(this, 10);

  mleFile = new MLEditWrap(false, this, false);
  mleFile->setMinimumHeight(mleFile->frameWidth() * 2
                            + 20 * mleFile->fontMetrics().lineSpacing());
  mleFile->setMinimumWidth(mleFile->fontMetrics().width("_") * 80);
  top_lay->addWidget(mleFile);

  QBoxLayout* lay = new QHBoxLayout(top_lay, 10);

  btnSave = new QPushButton(tr("&Save"), this);
  connect(btnSave, SIGNAL(clicked()), this, SLOT(slot_save()));
  btnClose = new QPushButton(tr("&Close"), this);
  btnClose->setDefault(true);
  connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));
  int bw = 75;
  bw = QMAX(bw, btnSave->sizeHint().width());
  bw = QMAX(bw, btnClose->sizeHint().width());
  btnSave->setFixedWidth(bw);
  btnClose->setFixedWidth(bw);
  lay->addStretch(1);
  lay->addWidget(btnSave);
  lay->addWidget(btnClose);

  show();

  // Load up the file
  QFile f(sFile);
  if (!f.open(IO_ReadOnly))
  {
    WarnUser(this, tr("Failed to open file:\n%1").arg(sFile));
    mleFile->setEnabled(false);
    btnSave->setEnabled(false);
  }
  else
  {
    QTextStream t(&f);
    mleFile->setText(t.read());
    f.close();
    QFileInfo fi(f);
    if (!fi.isWritable())
      setCaption(caption() + QString(tr("[ Read-Only ]")));
  }
}

// --------------------------------------------------------------------------

void EditFileDlg::slot_save()
{
  QFile f(sFile);
  if (!f.open(IO_WriteOnly))
  {
    WarnUser(this, tr("Failed to open file:\n%1").arg(sFile));
    mleFile->setEnabled(false);
    btnSave->setEnabled(false);
  }
  else
  {
    QTextStream t(&f);
    t << mleFile->text();
    f.close();
  }

}

#include "editfile.moc"
