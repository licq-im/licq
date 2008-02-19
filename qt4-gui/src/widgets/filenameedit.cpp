// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2008 Licq developers
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

#include "config.h"

#include "filenameedit.h"

#ifndef USE_KDE
#include <QFileDialog>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QToolButton>
#endif

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::FileNameEdit */

FileNameEdit::FileNameEdit(QWidget* parent)
  : FILENAMEEDIT_BASE(parent)
{
#ifndef USE_KDE
  QHBoxLayout* lay = new QHBoxLayout(this);
  lay->setContentsMargins(0, 0, 0, 0);

  // Input field
  editField = new QLineEdit();
  lay->addWidget(editField);

  // Button to open file dialog
  QToolButton* browseButton = new QToolButton();
  // TODO: Use an open file icon instead of text for browseButton
  browseButton->setText(tr("Browse..."));
  connect(browseButton, SIGNAL(clicked()), SLOT(browse()));
  lay->addWidget(browseButton);
#endif
}

void FileNameEdit::setFileName(const QString& fileName)
{
#ifdef USE_KDE
  setUrl(KUrl(fileName));
#else
  editField->setText(fileName);
#endif
}

QString FileNameEdit::fileName() const
{
#ifdef USE_KDE
  return url().pathOrUrl();
#else
  return editField->text();
#endif
}

#ifndef USE_KDE
void FileNameEdit::browse()
{
  QString filename = QFileDialog::getOpenFileName(this, QString(), editField->text(), QString());

  if (filename.isNull())
    return;

  editField->setText(filename);
}
#endif
