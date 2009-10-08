// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2002-2009 Licq developers
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

// written by Graham Roff <graham@licq.org>
// contributions by Dirk A. Mueller <dirk@licq.org>

#include "mlview.h"

#include "config.h"

#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QTextDocumentFragment>
#include <QMenu>
#include <QRegExp>
#include <QScrollBar>

#include "config/emoticons.h"
#include "config/general.h"
#include "core/licqgui.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::MLView */

MLView::MLView(QWidget* parent)
  : QTextBrowser(parent),
    m_handleLinks(true)
{
  setLineWrapMode(QTextEdit::WidgetWidth);
  setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);

  updateFont();
  connect(Config::General::instance(), SIGNAL(fontChanged()), SLOT(updateFont()));
}

void MLView::appendNoNewLine(const QString& s)
{
  QTextCursor tc = textCursor();
  tc.movePosition(QTextCursor::End);
  tc.insertHtml(s);
}

QString MLView::toRichText(const QString& s, bool highlightURLs, bool useHTML, QRegExp highlight)
{
  // Expressions to match URIs and Mail addresses
  // If no matching should be done, they will be left empty
  QRegExp reURL;
  QRegExp reMail;

  // We must hightlight URLs at this step, before we convert
  // linebreaks to richtext tags and such.  Also, check to make sure
  // that the text is not prepared to be highlighted already (by AIM).
  QRegExp reAHREF("<a href", Qt::CaseInsensitive);
  if (highlightURLs && s.indexOf(reAHREF) == -1)
  {
    reURL.setPattern(
      "(?:(https?|ftp)://(.+(:.+)?@)?|www\\d?\\.)"  // protocoll://[user[:password]@] or www[digit].
      "[a-z0-9.-]+\\.([a-z]+|[0-9]+)"               // hostname.tld or ip address
      "(:[0-9]+)?"                                  // optional port
      "(/(([-\\w%{}|\\\\^~`;/?:@=&$_.+!*'(),#]|\\[|\\])*[^.,:;?!\\s])*)?");
    reURL.setMinimal(false);
    reURL.setCaseSensitivity(Qt::CaseInsensitive);

    reMail.setPattern(
      "(mailto:)?"
      "[a-z9-0._%+-]+"
      "@"
      "[a-z0-9.-]+\\.(?:[a-z]+|[0-9]+)");
    reMail.setMinimal(false);
    reMail.setCaseSensitivity(Qt::CaseInsensitive);
  }

  // The following will parse through the string adding <a> tags to URIs and
  // Mail addresses while highlighting anything matching the highlight regexp.
  // If a highlight crosses any <a> or </a> the <span> is closed and reopened.
  // If URI and mail matches overlap, only the first one will be tagged.

  // Variables to keep track of positions in string while parsing
  // *Pos and *Len is position and length of next match. *End is end of current
  // match. *Pos=-2 is used to mark that next match should be found. *End is >0
  // when a tag is currently open
  int urlPos = -2;
  int urlLen = 0;
  int urlEnd = 0;
  int mailPos = -2;
  int mailLen = 0;
  int mailEnd = 0;
  int hlPos = -2;
  int hlLen = 0;
  int hlEnd = 0;

  // New string build while parsing
  QString text;
  // Anything before lastpos has already been copied from input string (s) to text.
  int lastpos = 0;

  const QString highlightStart = "<span style=\"background-color: yellow; color: black\">";
  const QString highlightEnd = "</span>";

  do
  {
    // Find next match of a regexp but only if it has a pattern defined
    if (hlPos == -2 && !highlight.isEmpty())
    {
      hlPos = s.indexOf(highlight, qMax(hlEnd, lastpos));
      hlLen = highlight.matchedLength();

      // If we matched zero characters, ignore the match
      if (hlLen == 0)
        hlPos = -2;
    }
    if (urlPos == -2 && !reURL.isEmpty())
    {
      urlPos = s.indexOf(reURL, qMax(urlEnd, lastpos));
      urlLen = reURL.matchedLength();
      if (urlLen == 0)
        urlPos = -2;
    }
    if (mailPos == -2 && !reMail.isEmpty())
    {
      mailPos = s.indexOf(reMail, qMax(mailEnd, lastpos));
      mailLen = reMail.matchedLength();
      if (mailLen == 0)
        mailPos = -2;
    }

    // Next value for lastpos. Data between newpos and lastpos can be copied as is
    int newpos;
    // Tags to add after next block of text has been copied
    QString tags;
    // Does a highlight need to be closed and reopened to allow other tag to open/close
    bool breakhl = false;

    if (hlEnd > 0 &&
        (urlPos < 0 || hlEnd <= urlPos) && (urlEnd == 0 || hlEnd <= urlEnd) &&
        (mailPos < 0 || hlEnd <= mailPos) && (mailEnd == 0 || hlEnd <= mailEnd))
    {
      // End of highlight
      tags = highlightEnd;
      newpos = hlEnd;
      hlEnd = 0;
    }
    else if (hlEnd == 0 && hlPos > -1 &&
        (urlPos < 0 || hlPos < urlPos) && (urlEnd == 0 || hlPos < urlEnd) &&
        (mailPos < 0 || hlPos < mailPos) && (mailEnd == 0 || hlPos < mailEnd))
    {
      // Start of highlight
      tags = highlightStart;
      newpos = hlPos;
      hlEnd = hlPos + hlLen;
      hlPos = -2; // Trigger search to continue
    }
    else if (urlEnd > 0)
    {
      // End of URI
      tags = "</a>";
      breakhl = true;
      newpos = urlEnd;
      urlEnd = 0;
      mailPos = -2; // Make sure we don't have overlapping URL and mail
    }
    else if (mailEnd > 0)
    {
      // End of mail
      tags = "</a>";
      breakhl = true;
      newpos = mailEnd;
      mailEnd = 0;
      urlPos = -2; // Make sure we don't have overlapping URL and mail
    }
    else if (urlPos > -1 &&
        (mailPos == -1 || urlPos <= mailPos))
    {
      // Start of URI
      QString url = reURL.cap();
      QString fullurl = (reURL.cap(1).isEmpty() ? QString("http://%1").arg(url) : url);
      tags = "<a href=\"" + fullurl + "\">";
      breakhl = true;
      newpos = urlPos;
      urlEnd = urlPos + urlLen;
      urlPos = -2;
    }
    else if (mailPos > -1)
    {
      // Start of mail
      QString mail = reMail.cap();
      QString fullmail = (reMail.cap(1).isEmpty() ? QString("mailto:%1").arg(mail) : mail);
      tags = "<a href=\"" + fullmail + "\">";
      breakhl = true;
      newpos = mailPos;
      mailEnd = mailPos + mailLen;
      mailPos = -2;
    }
    else
    {
      // Nothing more to do, just get the remainder of the string
      newpos = s.length();
    }

    // Get next block of text that can be copied from input
    QString rawtext = s.mid(lastpos, newpos - lastpos);
    text.append(useHTML ? rawtext : Qt::escape(rawtext));

    // Add tags applicable for this position in the string
    if (breakhl && hlEnd > 0)
      tags = highlightEnd + tags + highlightStart;
    text.append(tags);

    lastpos = newpos;
  }
  while (urlEnd > 0 || mailEnd > 0 || hlEnd > 0 || lastpos < s.length());

  Emoticons::self()->parseMessage(text, Emoticons::NormalMode);

  // convert linebreaks to <br>
  text.replace(QRegExp("\n"), "<br>\n");
  // We keep the first space character as-is (to allow line wrapping)
  // and convert the next characters to &nbsp;s (to preserve multiple
  // spaces).
  QRegExp longSpaces(" ([ ]+)");
  QString cap;
  int pos = 0;
  while ((pos = longSpaces.indexIn(text)) > -1)
  {
    cap = longSpaces.cap(1);
    cap.replace(QRegExp(" "), "&nbsp;");
    text.replace(pos+1, longSpaces.matchedLength()-1, cap);
  }
  text.replace(QRegExp("\t"), " &nbsp;&nbsp;&nbsp;");

  return text;
}

void MLView::GotoHome()
{
  QTextCursor tc = textCursor();
  tc.movePosition(QTextCursor::Start);
  setTextCursor(tc);
}

void MLView::GotoEnd()
{
  QTextCursor tc = textCursor();
  tc.movePosition(QTextCursor::End);
  setTextCursor(tc);
}

void MLView::scrollPageDown()
{
  verticalScrollBar()->triggerAction(QScrollBar::SliderPageStepAdd);
}

void MLView::scrollPageUp()
{
  verticalScrollBar()->triggerAction(QScrollBar::SliderPageStepSub);
}

void MLView::setBackground(const QColor& c)
{
  QPalette pal = palette();

  pal.setColor(QPalette::Active, QPalette::Base, c);
  pal.setColor(QPalette::Inactive, QPalette::Base, c);

  setPalette(pal);
}

/** @brief Adds "Copy URL" to the popup menu if the user right clicks on a URL. */
void MLView::contextMenuEvent(QContextMenuEvent* event)
{
  QMenu* menu = createStandardContextMenu();

  m_url = anchorAt(event->pos());
  if (!m_url.isNull() && !m_url.isEmpty())
    menu->addAction(tr("Copy URL"), this, SLOT(slotCopyUrl()));
  if (hasMarkedText())
    menu->addAction(tr("Quote"), this, SLOT(makeQuote()));

  menu->exec(event->globalPos());
  delete menu;
}

/** @brief Adds the contents of m_url to the clipboard. */
void MLView::slotCopyUrl()
{
  if (!m_url.isNull() && !m_url.isEmpty())
  {
    // This copies m_url to both the normal clipboard (Ctrl+C/V/X)
    // and the selection clipboard (paste with middle mouse button).
    QClipboard *cb = QApplication::clipboard();
    cb->setText(m_url, QClipboard::Clipboard);
    if (cb->supportsSelection())
      cb->setText(m_url, QClipboard::Selection);
  }
}

QMimeData* MLView::createMimeDataFromSelection() const
{
  QMimeData* result = QTextEdit::createMimeDataFromSelection();

  if (result->hasHtml())
  {
    QString html = result->html();
    Emoticons::unparseMessage(html);
    QTextDocumentFragment fragment =
      QTextDocumentFragment::fromHtml(html, document());
    result->setText(fragment.toPlainText());
  }

  return result;
}

void MLView::makeQuote()
{
  QTextCursor cr = textCursor();
  if (!cr.hasSelection())
    return;

  QString html = cr.selection().toHtml();

  Emoticons::unparseMessage(html);

  QString text = QTextDocumentFragment::fromHtml(html).toPlainText();

  text.insert(0, "> ");
  text.replace("\n", "\n> ");

  emit quote(text);
}

void MLView::setForeground(const QColor& c)
{
  QPalette pal;
  pal.setColor(QPalette::WindowText, c);
  setPalette(pal);
}

void MLView::setHandleLinks(bool enable)
{
  m_handleLinks = enable;
}

void MLView::setSource(const QUrl& url)
{
  if (m_handleLinks && !url.scheme().isEmpty())
    LicqGui::instance()->viewUrl(url.toString());
}

bool MLView::hasMarkedText() const
{
  return textCursor().hasSelection();
}

QString MLView::markedText() const
{
  return textCursor().selectedText();
}

void MLView::updateFont()
{
  setFont(Config::General::instance()->historyFont());

  // Get height of current font
  myFontHeight = fontMetrics().height();

  // Set minimum height of text area to one line of text.
  setMinimumHeight(heightForLines(1));
}

int MLView::heightForLines(int lines) const
{
  // We need to add frame width and the added height of the scroll area as
  // we're calculating height for the widget, not the viewport.
  return lines*myFontHeight + height() - viewport()->height() + 2 * frameWidth();
}

void MLView::setSizeHintLines(int lines)
{
  myLinesHint = lines;
}

QSize MLView::sizeHint() const
{
  QSize s = QTextBrowser::sizeHint();
  if (myLinesHint > 0)
    s.setHeight(heightForLines(myLinesHint));
  return s;
}
