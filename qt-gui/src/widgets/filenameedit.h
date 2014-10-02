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

#ifndef FILENAMEEDIT_H
#define FILENAMEEDIT_H

#include <config.h>

#ifdef USE_KDE
#include <KDE/KUrlRequester>
#define FILENAMEEDIT_BASE KUrlRequester
#else
#include <QWidget>
#define FILENAMEEDIT_BASE QWidget

class QLineEdit;
#endif

namespace LicqQtGui
{

/**
 * Input field for a filename together with a browse button that will open a
 * file selection dialog.
 * Uses KUrlRequester when building with KDE support.
 */
class FileNameEdit : public FILENAMEEDIT_BASE
{
  Q_OBJECT

public:
  /**
   * Constructor
   *
   * @parent Parent widget
   */
  FileNameEdit(QWidget* parent = NULL);

  /**
   * Set file name
   *
   * @param fileName New file name
   */
  void setFileName(const QString& fileName);

  /**
   * Get file name
   *
   * @return File name
   */
  QString fileName() const;

  /**
   * Set a default path to use if opening dialog with no current file selected
   *
   * @param path New default path to use
   */
  void setDefaultPath(const QString& path) { myDefaultPath = path; }

#ifndef USE_KDE
  /**
   * Set filter for the file dialog
   * Note: This function uses the same filter syntax as KFileDialog::setFilter
   * which is not the same as for QFileDialog.
   *
   * @param filter Filter(s) to use, example "*.txt|Text files (*.txt)"
   */
  void setFilter(const QString& filter);
#endif

private slots:
#ifdef USE_KDE
  /**
   * File dialog is about to open
   */
  void dialogAboutToOpen();
#else
  /**
   * Open a file dialog to browse for file
   */
  void browse();
#endif

private:
#ifndef USE_KDE
  QLineEdit* editField;
  QString myFilter;
#endif
  QString myDefaultPath;
};

} // namespace LicqQtGui

#endif
