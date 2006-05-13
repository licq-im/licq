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
#if QT_VERSION >= 300

#include <qfont.h>
#include <qpainter.h>
#include <qaccel.h>
#include <qregexp.h>
#include <qapplication.h>
#include <qclipboard.h>

#include "ewidgets.h"
#include "licq_icqd.h"

#include "mlview3.h"

MLView::MLView (QWidget* parent, const char *name)
  : QTextBrowser(parent, name), m_handleLinks(true)
{
  setWordWrap(WidgetWidth);
#if QT_VERSION >= 0x030100
  setWrapPolicy(AtWordOrDocumentBoundary);
#else
  setWrapPolicy(AtWhiteSpace);
#endif
  setReadOnly(true);
  setTextFormat(RichText);
}

void MLView::appendNoNewLine(const QString& s)
{
  int p = paragraphs() - 1;
  insertAt(s, p, paragraphLength(p));
}

void MLView::append(const QString& s)
{
  if (strcmp(qVersion(), "3.0.0") == 0 ||
      strcmp(qVersion(), "3.0.1") == 0 ||
      strcmp(qVersion(), "3.0.2") == 0 ||
      strcmp(qVersion(), "3.0.3") == 0 ||
      strcmp(qVersion(), "3.0.4") == 0)
  {
     // Workaround --
     // In those versions, QTextEdit::append didn't add a new paragraph.
     QTextBrowser::append("<p>" + s);
  }
  else
  {
     QTextBrowser::append(s);
  }
}

#include "emoticon.h"
#include "mainwin.h" // for the CEmoticon instance

QString MLView::toRichText(const QString& s, bool highlightURLs, bool useHTML)
{
  // We cannot use QStyleSheet::convertFromPlainText
  // since it has a bug in Qt 3 which causes line breaks to mix up.
  // not used for html now QString text = QStyleSheet::escape(s);
  QString text = useHTML ? s: QStyleSheet::escape(s);

  gMainWindow->emoticons->ParseMessage(text);

    // We must hightlight URLs at this step, before we convert
    // linebreaks to richtext tags and such.  Also, check to make sure
    // that the text is not prepared to be highlighted already (by AIM).
    QRegExp reAHREF("<a href", false);
    int pos = 0;
    if (highlightURLs && (pos = text.find(reAHREF, pos)) == -1)
    {
       QRegExp reURL("(\\b|^)((https?|ftp)://([-a-z0-9]+(:[-a-z0-9]+)?@)?[-a-z0-9.]+[-a-z0-9](:[0-9]+)?(/([-a-z0-9%{}|\\\\^~`;/?:@=&$_.+!*'(),]|\\[|\\])*)?)");
       reURL.setMinimal(false);
       reURL.setCaseSensitive(false);
       pos = 0;
       while ( (pos = text.find(reURL, pos)) != -1 ) {
          QString url = reURL.cap(2);
          QString link = QString::fromLatin1("<a href=\"") + url + QString::fromLatin1("\">") + url + QString::fromLatin1("</a>");
          text.replace(pos, url.length(), link);
          pos += reURL.matchedLength() - url.length() + link.length();
       }

       QRegExp reMail("(mailto:)?([\\d\\w\\.\\-_]+@[\\d\\w\\.\\-_]+)(\\s+|$)");
       reMail.setMinimal(true);
       pos = 0;
       while ( (pos = text.find(reMail, pos)) != -1 ) {
          QString mail = reMail.cap(2);
          QString link = QString::fromLatin1("<a href=\"mailto:") + mail + QString::fromLatin1("\">") + mail + QString::fromLatin1("</a>");
          text.replace(pos, mail.length(), link);
          pos += reMail.matchedLength() - mail.length() + link.length();
       }

    }

    // convert linebreaks to <br>
    text.replace(QRegExp("\n"), "<br>\n");
    // We keep the first space character as-is (to allow line wrapping)
    // and convert the next characters to &nbsp;s (to preserve multiple
    // spaces).
    QRegExp longSpaces(" ([ ]+)");
    pos = 0;
    QString cap;
    while ((pos = longSpaces.search(text)) > -1)
    {
       cap = longSpaces.cap(1);
       cap.replace(QRegExp(" "), "&nbsp;");
       text.replace(pos+1, longSpaces.matchedLength()-1, cap);
    }
    text.replace(QRegExp("\t"), " &nbsp;&nbsp;&nbsp;");

  return text;
}

void MLView::GotoEnd()
{
  moveCursor(QTextBrowser::MoveEnd, false);
}

void MLView::setBackground(const QColor& c)
{
  setPaper(QBrush(c));
}

/** @brief Adds "Copy URL" to the popup menu if the user right clicks on a URL. */
QPopupMenu* MLView::createPopupMenu(const QPoint& point)
{
  QPopupMenu *menu = QTextBrowser::createPopupMenu(point);

  m_url = anchorAt(point);
  if (!m_url.isNull() && !m_url.isEmpty())
    menu->insertItem(tr("Copy URL"), this, SLOT(slotCopyUrl()));

  return menu;
}

/** @brief Adds the contents of m_url to the clipboard. */
void MLView::slotCopyUrl()
{
  if (!m_url.isNull() && !m_url.isEmpty())
  {
    // This copies m_url to both the normal clipboard (Ctrl+C/V/X)
    // and the selection clipboard (paste with middle mouse button).
    QClipboard *cb = QApplication::clipboard();
    cb->setText(m_url);
    if (cb->supportsSelection())
    {
      bool enabled = cb->selectionModeEnabled();
      cb->setSelectionMode(!enabled);
      cb->setText(m_url);
      cb->setSelectionMode(enabled);
    }
  }
}

// -----------------------------------------------------------------------------


void MLView::setForeground(const QColor& c)
{
  setColor(c);
}

void MLView::setHandleLinks(bool enable)
{
  m_handleLinks = enable;
}

void MLView::setSource(const QString& name)
{
  if (m_handleLinks && ((name.find(QRegExp("^\\w+://")) > -1) || name.startsWith("mailto:")))
    emit viewurl(this, name);
}

bool MLView::hasMarkedText() const
{
  return hasSelectedText();
}

QString MLView::markedText() const
{
  return selectedText();
}

#include "mlview3.moc"

#endif
