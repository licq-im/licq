// -*- c-basic-offset: 2 -*-
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unistd.h>

#include <qlayout.h>
#include <qfile.h>
#include <qtextstream.h>
#ifdef USE_KDE
#include <kfiledialog.h>
#else
#include <qfiledialog.h>
#endif

#include "outputwin.h"
#include "ewidgets.h"
#include "mainwin.h"
#include "licq_icq.h"

#undef connect

//---------------------------------------------------------------------------

CQtLogWindow::CQtLogWindow(QWidget *parent)
  : LicqDialog(parent, "NetworkLog")
{
  setCaption(tr("Licq Network Log"));

  QBoxLayout* top_lay = new QVBoxLayout(this, 8);

  outputBox = new CLogWidget(this);
  outputBox->setMinimumHeight(outputBox->frameWidth() * 2
      + 16 * outputBox->fontMetrics().lineSpacing());
  outputBox->setMinimumWidth(outputBox->minimumHeight() * 2);
  top_lay->addWidget(outputBox);

  QBoxLayout* lay = new QHBoxLayout(top_lay, 8);

  btnSave = new QPushButton(tr("&Save"), this);
  connect(btnSave, SIGNAL(clicked()), SLOT(slot_save()));
  btnClear = new QPushButton(tr("C&lear"), this);
  connect(btnClear, SIGNAL(clicked()), outputBox, SLOT(clear()));
  btnHide = new QPushButton(tr("&Close"), this);
  btnHide->setDefault(true);
  connect(btnHide, SIGNAL(clicked()), this, SLOT(hide()));
  int bw = 75;
  bw = QMAX(bw, btnClear->sizeHint().width());
  bw = QMAX(bw, btnHide->sizeHint().width());
  bw = QMAX(bw, btnSave->sizeHint().width());
  btnClear->setFixedWidth(bw);
  btnHide->setFixedWidth(bw);
  btnSave->setFixedWidth(bw);
  lay->addStretch(1);
  lay->addWidget(btnSave);
  lay->addSpacing(30);
  lay->addWidget(btnClear);
  lay->addWidget(btnHide);

  adjustSize();

  sn = new QSocketNotifier(Pipe(), QSocketNotifier::Read, this);
  connect (sn, SIGNAL(activated(int)), this, SLOT(slot_log(int)));
}

// --------------------------------------------------------------------------

void CQtLogWindow::showEvent(QShowEvent *)
{
  // move Cursor to the end of the QMultiLineEdit
  outputBox->GotoEnd();
}

// --------------------------------------------------------------------------

void CQtLogWindow::slot_log(int s)
{
  char buf[4];
  read(s, buf, 1);

  QString str = QString::fromUtf8(NextLogMsg());

  outputBox->appendNoNewLine(str);
  outputBox->GotoEnd();

  // hardcoded limit, maybe should be user configurable?
  if (outputBox->paragraphs() > 564) {
      int todo = outputBox->paragraphs() - 500;
      for (int i = 0; i < todo; ++i)
          outputBox->removeParagraph(0);
  }

  /* The next call will block, so we need to clear the log so that processing
     can continue */
  unsigned short nNextLogType = NextLogType();
  ClearLog();

  if (nNextLogType == L_ERROR)
      CriticalUser(NULL, str);


  else if (nNextLogType == L_MESSAGE)
    InformUser(NULL, str);

}

void CQtLogWindow::slot_save()
{
  QString fn;
#ifdef USE_KDE
  KURL u = KFileDialog::getSaveURL(
      QString(QDir::homeDirPath() + "/licq.log"),QString::null, this);
  fn = u.path();
#else
  fn = QFileDialog::getSaveFileName(
         QString(QDir::homeDirPath() + "/licq.log"),
         QString::null, this);
#endif
  if (fn.isNull()) return;

  QFile f(fn);
  if (!f.open(IO_WriteOnly))
  {
    WarnUser(this, tr("Failed to open file:\n%1").arg(fn));
  }
  else
  {
    QTextStream t(&f);
    t << outputBox->text();
    f.close();
  }

}


// -----------------------------------------------------------------------------

CLogWidget::CLogWidget(QWidget* parent, const char* name)
  : MLEditWrap(false, parent, true, name)
{
  setReadOnly(true);
//  setTextFormat(LogText);
}

#include "outputwin.moc"
