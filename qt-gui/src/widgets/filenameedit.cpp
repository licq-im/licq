/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2008-2009 Licq developers
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

#ifdef USE_KDE
# include <KDE/KFileDialog>
#else
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
#ifdef USE_KDE
  connect(this, SIGNAL(openFileDialog(KUrlRequester*)), SLOT(dialogAboutToOpen()));
#else
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

#ifdef USE_KDE
void FileNameEdit::dialogAboutToOpen()
{
  // Set the default path for dialog to start in
  if (url().pathOrUrl().isEmpty() && !myDefaultPath.isEmpty())
    fileDialog()->setUrl(KUrl(myDefaultPath));
}
#else
void FileNameEdit::setFilter(const QString& filter)
{
  myFilter = filter;

  // Convert from KFileDialog fiter to QFileDialog filter syntax

  // Remove suffixes before pipes (i.e. "*.txt|Text files (*.txt)" => "Text files (*.txt)"
  myFilter.replace(QRegExp("[^\\n\\|]*\\|"), "");

  // Remove escaping backslash before slash
  myFilter.replace("\\/", "/");

  // Replace line breaks (\n) with two semicolons (;;).
  myFilter.replace("\n", ";;");
}

void FileNameEdit::browse()
{
  QString filename = editField->text();
  if (filename.isEmpty())
    filename = myDefaultPath;

  filename = QFileDialog::getOpenFileName(this, QString(), filename, myFilter);

  if (filename.isNull())
    return;

  editField->setText(filename);
}
#endif
