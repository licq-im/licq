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

#include "editfiledlg.h"

#include "config.h"

#include <QDialogButtonBox>
#include <QFile>
#include <QFileInfo>
#include <QPushButton>
#include <QTextStream>
#include <QVBoxLayout>

#include "core/messagebox.h"

#include "helpers/support.h"

#include "widgets/mledit.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::EditFileDlg */

EditFileDlg::EditFileDlg(const QString& fname, QWidget* parent)
  : QDialog(parent),
    myFile(fname)
{
  setAttribute(Qt::WA_DeleteOnClose, true);
  Support::setWidgetProps(this, "EditFileDialog");

  QVBoxLayout* top_lay = new QVBoxLayout(this);

  myFileEdit = new MLEdit(false, this, true);
  myFileEdit->setMinimumHeight(myFileEdit->frameWidth() * 2 + 20 * myFileEdit->fontMetrics().lineSpacing());
  myFileEdit->setMinimumWidth(myFileEdit->fontMetrics().width("_") * 80);
  connect(myFileEdit, SIGNAL(undoAvailable(bool)), SLOT(saveEnable(bool)));
  top_lay->addWidget(myFileEdit);

  QDialogButtonBox* buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Save |
      QDialogButtonBox::Close |
      QDialogButtonBox::Reset);

  connect(buttonBox, SIGNAL(accepted()), SLOT(save()));
  connect(buttonBox, SIGNAL(rejected()), SLOT(close()));

  btnSave = buttonBox->button(QDialogButtonBox::Reset); // Temporarily
  btnSave->setText(tr("Revert"));
  connect(btnSave, SIGNAL(clicked()), SLOT(revert()));

  btnSave = buttonBox->button(QDialogButtonBox::Save);

  top_lay->addWidget(buttonBox);

  revert();

  show();
}

void EditFileDlg::save()
{
  QFile f(myFile);
  if (!f.open(QIODevice::WriteOnly))
  {
    WarnUser(this, tr("Failed to open file:\n%1").arg(myFile));
    return;
  }
  QTextStream t(&f);
  t << myFileEdit->toPlainText();
  f.close();
  revert();
}

void EditFileDlg::saveEnable(bool yes)
{
  btnSave->setEnabled(yes);
}

void EditFileDlg::setTitle(QString postfix)
{
  if (!postfix.isNull())
    postfix.prepend(" ");
  postfix.prepend(tr("Licq File Editor - %1").arg(myFile));
  setWindowTitle(postfix);
}

void EditFileDlg::revert()
{
  QFile f(myFile);
  if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    WarnUser(this, tr("Failed to open file:\n%1").arg(myFile));
    return;
  }

  QTextStream t(&f);
  myFileEdit->setPlainText(t.readAll());
  f.close();

  QFileInfo fi(f);
  if (fi.isWritable())
  {
    setTitle();
    myFileEdit->setReadOnly(false);
    myFileEdit->setFocus();
    myFileEdit->ensureCursorVisible();
  }
  else
  {
    setTitle(tr("[ Read-Only ]"));
    myFileEdit->setReadOnly(true);
  }

  saveEnable(false);
}
