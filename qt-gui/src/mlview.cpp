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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qapplication.h>
#include <qclipboard.h>
#include <qdragobject.h>
#include <qmime.h>
#include <qpopupmenu.h>
#include <qregexp.h>

#include "mlview.h"
#include "emoticon.h"

MLView::MLView (QWidget* parent, const char *name)
  : QTextBrowser(parent, name), m_handleLinks(true), m_clipboardMode(-1)
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
    QTextBrowser::append(s);
}

QString MLView::toRichText(const QString& s, bool highlightURLs, bool useHTML)
{
  // We cannot use QStyleSheet::convertFromPlainText
  // since it has a bug in Qt 3 which causes line breaks to mix up.
  // not used for html now QString text = QStyleSheet::escape(s);
  QString text = useHTML ? s: QStyleSheet::escape(s);

  // We must hightlight URLs at this step, before we convert
  // linebreaks to richtext tags and such.  Also, check to make sure
  // that the text is not prepared to be highlighted already (by AIM).
  QRegExp reAHREF("<a href", false);
  if (highlightURLs && text.find(reAHREF) == -1)
  {
    QRegExp reURL(
      "(?:(https?|ftp)://(.+(:.+)?@)?|www\\d?\\.)"  // protocoll://[user[:password]@] or www[digit].
      "[a-z0-9.-]+\\.([a-z]+|[0-9]+)"               // hostname.tld or ip address
      "(:[0-9]+)?"                                  // optional port
      "(/(([-\\w%{}|\\\\^~`;/?:@=&$_.+!*'(),#]|\\[|\\])*[^.,:;?!\\s])*)?");
    reURL.setMinimal(false);
    reURL.setCaseSensitive(false);

    QRegExp reMail(
      "(mailto:)?"
      "[a-z9-0._%+-]+"
      "@"
      "[a-z0-9.-]+\\.(?:[a-z]+|[0-9]+)");
    reMail.setMinimal(false);
    reMail.setCaseSensitive(false);

    int pos = 0;
    while ((unsigned int)pos < text.length())
    {
      // We try to match both url and mail at the same time, and then
      // uses the one that matched first.
      const int urlPos = text.find(reURL, pos);
      const int mailPos = text.find(reMail, pos);

      if (urlPos == -1 && mailPos == -1)
        break;

      if ((urlPos != -1 && urlPos <= mailPos) || mailPos == -1)
      {
        const QString url = reURL.cap();
        const QString fullurl = (reURL.cap(1).isEmpty() ? QString("http://%1").arg(url) : url);

        // Can't use QString("<a href=\"%1\">%2</a>").arg(fullurl).arg(url) since it breaks
        // down when fullurl contains urlencoded chars (%##).
        const QString link = QString::fromLatin1("<a href=\"") + fullurl + QString::fromLatin1("\">") +
            url + QString::fromLatin1("</a>");

        text.replace(urlPos, reURL.matchedLength(), link);
        pos = urlPos + link.length();
      }
      else
      {
        const QString mail = reMail.cap();
        const QString fullmail = (reMail.cap(1).isEmpty() ? QString("mailto:%1").arg(mail) : mail);
        const QString link = QString::fromLatin1("<a href=\"") + fullmail + QString::fromLatin1("\">") +
            mail + QString::fromLatin1("</a>");
        text.replace(mailPos, reMail.matchedLength(), link);
        pos = mailPos + link.length();
      }
    }
  }

  CEmoticons::self()->parseMessage(text, CEmoticons::NormalMode);

  // convert linebreaks to <br>
  text.replace(QRegExp("\n"), "<br>\n");
  // We keep the first space character as-is (to allow line wrapping)
  // and convert the next characters to &nbsp;s (to preserve multiple
  // spaces).
  QRegExp longSpaces(" ([ ]+)");
  QString cap;
  int pos = 0;
  while ((pos = longSpaces.search(text)) > -1)
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
  setContentsPos(0, 0);
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
    cb->setText(m_url, QClipboard::Clipboard);
    if (cb->supportsSelection())
      cb->setText(m_url, QClipboard::Selection);
  }
}

/** @see MLView::copy() */
void MLView::slotClipboardSelectionChanged()
{
  m_clipboardMode = QClipboard::Selection;
}

/** @see MLView::copy() */
void MLView::slotClipboardDataChanged()
{
  m_clipboardMode = QClipboard::Clipboard;
}

/**
 * After we have copied the text, we update the copy so that all emoticons are
 * replaced with their respective smiley.
 */
void MLView::copy()
{
  m_clipboardMode = -1;

  // We connect these so that we know which clipboard changed (selection or data)
  connect(QApplication::clipboard(), SIGNAL(selectionChanged()), this, SLOT(slotClipboardSelectionChanged()));
  connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(slotClipboardDataChanged()));

  QTextBrowser::copy();

  disconnect(QApplication::clipboard(), SIGNAL(selectionChanged()), this, SLOT(slotClipboardSelectionChanged()));
  disconnect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(slotClipboardDataChanged()));

  // m_clipboardMode is set in one of slotClipboard{Selection,Data}Changed()
  if (m_clipboardMode != -1)
  {
    // QTextEdit copies the text both as rich text and as regular text.
    // We parse the rich text and then update the regular text.
    QMimeSource *m = QApplication::clipboard()->data(static_cast<QClipboard::Mode>(m_clipboardMode));
    if (m->provides("application/x-qrichtext") && QTextDrag::canDecode(m))
    {
      if (QTextDrag *drag = dynamic_cast<QTextDrag*>(m))
      {
        QString text = QString::fromUtf8(m->encodedData("application/x-qrichtext").data());

        CEmoticons::unparseMessage(text);

        const QRegExp newline("<br( /)?>");
        text.replace(newline, "\n");

        const QRegExp htmlTags("</?[^>]+>");
        text.remove(htmlTags);

        text.replace("&lt;", "<");
        text.replace("&gt;", ">");
        text.replace("&quot;", "\"");

        // &amp; => & needs to be last
        text.replace("&amp;", "&");

        drag->setText(text);
      }
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

#include "mlview.moc"
