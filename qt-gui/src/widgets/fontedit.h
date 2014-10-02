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

#ifndef FONTEDIT_H
#define FONTEDIT_H

#include "config.h"

#ifdef USE_KDE
#include <KDE/KFontRequester>
#define FONTEDIT_BASE KFontRequester
#else
#include <QWidget>
#define FONTEDIT_BASE QWidget

class QLineEdit;
#endif

namespace LicqQtGui
{

/**
 * Input field for a font together with a browse button that will open a
 * font selection dialog.
 * Uses KFontRequester when building with KDE support.
 */
class FontEdit : public FONTEDIT_BASE
{
  Q_OBJECT

public:
  /**
   * Constructor
   *
   * @parent Parent widget
   */
  FontEdit(QWidget* parent = NULL);

  /**
   * Set font
   *
   * @param font New font
   * @param onlyFixed Not used, only present to match setFont() in KFontRequester
   */
  virtual void setFont(const QFont& font, bool onlyFixed = false);

#ifndef USE_KDE
  /**
   * Get font
   *
   * @return Currently selected font
   */
  QFont font() const;

signals:
  /**
   * A new font was selected
   *
   * @param font New font
   */
  void fontSelected(const QFont& font);

private slots:
  /**
   * Open a font dialog to browse for file
   */
  void browse();

private:
  QLineEdit* myEditor;
#endif
};

} // namespace LicqQtGui

#endif
