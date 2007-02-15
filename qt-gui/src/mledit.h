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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef USE_KDE
# include <kdeversion.h>
# if KDE_IS_VERSION(3, 1, 0)
#  define MLEDIT_USE_KTEXTEDIT 1
# endif
#endif

// Since moc doesn't understand preprocessor directives we use this wrapper class,
// instead of inheriting directly from (K|Q)TextEdit, to avoid syntax errors.
#ifdef MLEDIT_USE_KTEXTEDIT
# include <ktextedit.h>
  class MLEditWrapBase : public KTextEdit
  {
    public:
      MLEditWrapBase(QWidget *parent, const char *name) : KTextEdit(parent, name) {}
  };
#else
# include <qtextedit.h>
  class MLEditWrapBase : public QTextEdit
  {
    public:
      MLEditWrapBase(QWidget *parent, const char *name) : QTextEdit(parent, name) {}
  };
#endif

class MLEditWrap : public MLEditWrapBase
{
  Q_OBJECT
public:
  MLEditWrap (bool wordWrap, QWidget* parent=0, bool handlequotes = false, const char *name=0);
  virtual ~MLEditWrap() {}

  void appendNoNewLine(const QString& s);
  void append(const QString& s);
  void GotoEnd();

  void setBackground(const QColor&);
  void setForeground(const QColor&);

  void setCheckSpellingEnabled(bool check);
  bool checkSpellingEnabled() const;

  static QFont *editFont;
  static bool useDoubleReturn;

protected:
  virtual void keyPressEvent(QKeyEvent *e);
  virtual QPopupMenu *createPopupMenu(const QPoint& pos);

public slots:
  void setText(const QString& text);
  virtual void setText(const QString& text, const QString& context);

private slots:
  void slotToggleAllowTab();

signals:
  void signal_CtrlEnterPressed();

private:
  bool m_fixSetTextNewlines;
  bool m_lastKeyWasReturn;
};

#endif
