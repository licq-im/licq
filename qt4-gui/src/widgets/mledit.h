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

#ifndef MLEDIT_H
#define MLEDIT_H

#include "config.h"

#ifdef USE_KDE
# include <kdeversion.h>
#  define MLEDIT_USE_KTEXTEDIT 1
#endif

#ifdef MLEDIT_USE_KTEXTEDIT
# include <ktextedit.h>
#else
# include <QTextEdit>
#endif


namespace LicqQtGui
{

class MLEdit
#ifdef MLEDIT_USE_KTEXTEDIT
  : public KTextEdit
#else
  : public QTextEdit
#endif
{
  Q_OBJECT

#ifdef MLEDIT_USE_KTEXTEDIT
  typedef KTextEdit BaseClass;
#else
  typedef QTextEdit BaseClass;
#endif

public:
  MLEdit(bool wordWrap, QWidget* parent = 0, bool handlequotes = false, const char* name = 0);
  virtual ~MLEdit();

  void appendNoNewLine(const QString& s);
  void GotoEnd();

  void setBackground(const QColor& color);
  void setForeground(const QColor& color);

#ifndef USE_KDE
  void setCheckSpellingEnabled(bool /* check */) {}
  bool checkSpellingEnabled() const { return false; }
#endif

  /**
   * Caclulate height for widget to fit a specified number of lines with
   * current font
   *
   * @param lines Number of text lines to calculate for
   * @return Widget height in pixels
   */
  int heightForLines(int lines) const;

  /**
   * Set size hint as number of lines of text
   *
   * @param lines Lines of text that should be visible
   */
  void setSizeHintLines(int lines);

  /**
   * Get recommended widget size
   *
   * @return Recommended size
   */
  QSize sizeHint() const;

signals:
  void ctrlEnterPressed();
  void clicked();

private:
  bool myFixSetTextNewlines;
  bool myLastKeyWasReturn;
  int myFontHeight;
  int myLinesHint;

  virtual void keyPressEvent(QKeyEvent* event);
  virtual void mousePressEvent(QMouseEvent* event);
#ifndef USE_KDE
  virtual void contextMenuEvent(QContextMenuEvent* event);
#endif

#if 0
//TODO: This may or may not be needed for KTextEdit in KDE 4
public slots:
  void setText(const QString& text);
  virtual void setText(const QString& text, const QString& context);
#endif

private slots:
  void updateFont();
  void toggleAllowTab();
};

} // namespace LicqQtGui

#endif
