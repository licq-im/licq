// -*- c-basic-offset: 2 -*-
/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

// written by Graham Roff <graham@licq.org>
// contributions by Dirk A. Mueller <dirk@licq.org>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qglobal.h>
#if QT_VERSION < 300

#include <qfont.h>
#include <qpainter.h>
#include <qaccel.h>
#include <qregexp.h>
#if USE_KDE
#include <kurl.h>
#endif

#include "ewidgets.h"
#include "licq_icqd.h"

#include "mlview3.h"

MLViewQt3::MLViewQt3 (QWidget* parent, const char *name)
  : QTextBrowser(parent, name), m_handleLinks(true), m_licqDaemon(NULL)
{
  setWordWrap(WidgetWidth);
  setWrapPolicy(AtWhiteSpace);
  setReadOnly(true);
}

void MLViewQt3::appendNoNewLine(const QString& s)
{
  int p = paragraphs() - 1;
  insertAt(s, p, paragraphLength(p));
}

void MLViewQt3::append(const QString& s)
{
  if (strcmp(qVersion(), "3.0.0") == 0 ||
      strcmp(qVersion(), "3.0.1") == 0 ||
      strcmp(qVersion(), "3.0.2") == 0 ||
      strcmp(qVersion(), "3.0.3") == 0 || 
      strcmp(qVersion(), "3.0.4") == 0)
  {
     // Workaround --
     // In those versions, QTextEdit::append didn't add a new paragraph.
     QTextEdit::append("<p>");
     QTextEdit::append(s);
  }
  QTextEdit::append(s);
}

QString MLViewQt3::toRichText(const QString& s, bool highlightURLs)
{
  // We cannot use QStyleSheet::convertFromPlainText
  // since it has a bug in Qt 3 which causes line breaks to mix up.
  QString text = QStyleSheet::escape(s);
  text.replace(QRegExp("\n"), "<br>");
  // We keep the first space character as-is (to allow line wrapping)
  // and convert the next characters to &nbsp;s (to preserve multiple
  // spaces).
  QRegExp longSpaces(" ([ ]+)");
  int pos;
  QString cap;
  while ((pos = longSpaces.search(text)) > -1)
  {
     cap = longSpaces.cap(1);
     cap.replace(QRegExp(" "), "&nbsp;");
     text.replace(pos+1, longSpaces.matchedLength()-1, cap); 
  }
  text.replace(QRegExp("\t"), " &nbsp;&nbsp;&nbsp;");
  
  if (highlightURLs)
  {
     QRegExp reURL("(\\w+://.+)(\\s+|$)");
     reURL.setMinimal(true);
     int pos = 0;
     while ( (pos = text.find(reURL, pos)) != -1 ) {
        QString url = reURL.cap(1);
        QString urlEscaped = QStyleSheet::escape(url);
        QString link = QString::fromLatin1("<a href=\"") + urlEscaped + QString::fromLatin1("\">") + urlEscaped + QString::fromLatin1("</a>");
        text.replace(pos, url.length(), link);
        pos += reURL.matchedLength() - url.length() + link.length();
     }
     
     QRegExp reMail("(mailto:)?([\\d\\w\\.\\-_]+@[\\d\\w\\.\\-_]+)(\\s+|$)");
     reMail.setMinimal(true);
     pos = 0;
     while ( (pos = text.find(reMail, pos)) != -1 ) {
        QString mail = reMail.cap(2);
        QString mailEscaped = QStyleSheet::escape(mail);
        QString link = QString::fromLatin1("<a href=\"mailto:") + mailEscaped + QString::fromLatin1("\">") + mailEscaped + QString::fromLatin1("</a>");
        text.replace(pos, mail.length(), link);
        pos += reMail.matchedLength() - mail.length() + link.length();
     }
     
  }
  
  return text;
}

void MLViewQt3::GotoEnd()
{
  moveCursor(QTextEdit::MoveEnd, false);
}

void MLViewQt3::setBackground(const QColor& c)
{
  QPalette pal = palette();

  pal.setColor(QPalette::Active, QColorGroup::Base, c);
  pal.setColor(QPalette::Inactive, QColorGroup::Base, c);

  setPalette(pal);
}


// -----------------------------------------------------------------------------


void MLViewQt3::setForeground(const QColor& c)
{
  QPalette pal = palette();

  pal.setColor(QPalette::Active, QColorGroup::Text, c);
  pal.setColor(QPalette::Inactive, QColorGroup::Text, c);

  setPalette(pal);
}

void MLViewQt3::setHandleLinks(bool enable)
{
  m_handleLinks = enable;
}

void MLViewQt3::setICQDaemon(CICQDaemon* licqDaemon)
{
  m_licqDaemon = licqDaemon;
}

void MLViewQt3::setSource(const QString& name)
{
  if (m_handleLinks)
  {
#ifdef USE_KDE
    KApplication* app = static_cast<KApplication*>(qApp);
    if (name.find(QRegExp("^\\w+://")) > -1)
       app->invokeBrowser( name );
    else if (name.startsWith("mailto:"))
       app->invokeMailer( KURL(name) );
#else
    if (name.find(QRegExp("^\\w+:")) > -1)
    {
       if (m_licqDaemon == NULL)
           WarnUser(this, tr("Licq is unable to find a browser application due to an internal error."));
       else if (!m_licqDaemon->ViewUrl(name.local8Bit().data()))
           WarnUser(this, tr("Licq is unable to start your browser and open the URL.\nYou will need to start the browser and open the URL manually."));
    }
#endif
  }
}

#include "mlview3.moc"

#endif
