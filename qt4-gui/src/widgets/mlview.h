/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2002-2006 Licq developers
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

#ifndef MLVIEW_H
#define MLVIEW_H

#include <QTextBrowser>

namespace LicqQtGui
{

class MLView : public QTextBrowser
{
  Q_OBJECT
public:
  MLView(QWidget* parent = 0);
  virtual ~MLView() {}

  void appendNoNewLine(const QString& s);
  void GotoHome();
  void GotoEnd();

  bool hasMarkedText() const;
  QString markedText() const;

  void setBackground(const QColor&);
  void setForeground(const QColor&);
  void setHandleLinks(bool enable);

  static QString toRichText(const QString& s, bool highlightURLs = false, bool useHTML = false, QRegExp highlight = QRegExp());

protected:
  virtual void contextMenuEvent(QContextMenuEvent* event);

public slots:
  virtual void setSource(const QUrl& url);

private slots:
  void slotCopy();
  void slotCopyUrl();
  void makeQuote();

private:
  bool m_handleLinks;
  QString m_url;

signals:
  void viewurl(QWidget*, QString);
  void quote(const QString& text);
};

} // namespace LicqQtGui

#endif
