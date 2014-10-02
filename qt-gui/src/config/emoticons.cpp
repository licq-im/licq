/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2003-2011 Licq developers
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

#include "emoticons.h"

#include "config.h"

#ifdef USE_KDE
# include <kapplication.h>
#else
# include <QApplication>
#endif

//#define EMOTICON_DEBUG

#ifdef EMOTICON_DEBUG
# define TRACE(x...) qDebug(x)
#else
# define TRACE(x...) ((void)0)
#endif

#include <QDir>
#include <QDomDocument>
#include <QLinkedList>
#include <QRegExp>
#include <QTextDocument>

#include <licq/logging/log.h>


using namespace LicqQtGui;

const QString Emoticons::DEFAULT_THEME =
  QString::fromLatin1(QT_TRANSLATE_NOOP("LicqQtGui::Emoticons", "Default"));
const QString Emoticons::NO_THEME =
  QString::fromLatin1(QT_TRANSLATE_NOOP("LicqQtGui::Emoticons", "None"));

struct Emoticon
{
  QString file;
  QString smiley;
  QString escapedSmiley;
};

/// Private data and functions for Emotions
class Emoticons::Impl
{
public:
  QStringList basedirs;
  QString currentTheme;

  // Maps first char in smiley to its Emoticon instance.
  QMap<QChar, QLinkedList<Emoticon> > emoticons;

  // Maps an emoticon's filename to a smiley.
  QMap<QString, QString> fileSmiley;

  QString themeDir(const QString &theme) const;
};

/**
 * @param theme the untranslated name of a theme
 * @returns the full path to @a theme, or QString::null if no such theme was found.
 */
QString Emoticons::Impl::themeDir(const QString &theme) const
{
  QStringList::ConstIterator basedir = basedirs.begin();
  for (; basedir != basedirs.end(); basedir++)
  {
    const QString dir = QString("%1/%2").arg(*basedir).arg(theme);
    if (QFile::exists(QString("%1/emoticons.xml").arg(dir)))
      return dir;
  }

  return QString::null;
}


// By making the application object parent, this instance will be
// deleted when the application is closed.
Emoticons::Emoticons()
#ifdef USE_KDE
  : QObject(kapp)
#else
  : QObject(qApp)
#endif
{
  pimpl = new Impl;
  pimpl->currentTheme = NO_THEME;
}

Emoticons::~Emoticons()
{
  delete pimpl;
}

Emoticons* Emoticons::m_self = 0L;
Emoticons* Emoticons::self()
{
  if (!m_self)
    m_self = new Emoticons;
  return m_self;
}

QString Emoticons::translateThemeName(const QString &name)
{
  if (name == DEFAULT_THEME || name == NO_THEME)
    return tr(name.toLatin1());
  return name;
}

QString Emoticons::untranslateThemeName(const QString &name)
{
  if (name == tr(DEFAULT_THEME.toLatin1()))
    return DEFAULT_THEME;
  else if (name == tr(NO_THEME.toLatin1()))
    return NO_THEME;
  else
    return name;
}

void Emoticons::setBasedirs(const QStringList &basedirs)
{
  pimpl->basedirs.clear();
  QStringList::ConstIterator basedir = basedirs.begin();
  for (; basedir != basedirs.end(); basedir++)
    pimpl->basedirs += QDir(*basedir).absolutePath();
}

/**
 * In every subdir in every basedir, we check for a file
 * named emoticons.xml, and if we find one, subdir is added
 * to the list of themes.
 */
QStringList Emoticons::themes() const
{
  QStringList themes;
  bool defaultExists = false;

  QStringList::ConstIterator basedir = pimpl->basedirs.begin();
  for (; basedir != pimpl->basedirs.end(); basedir++)
  {
    QDir dir(*basedir, QString::null, QDir::Unsorted, QDir::Dirs);
    const QStringList subdirs = dir.entryList();

    QStringList::ConstIterator subdir = subdirs.begin();
    for (; subdir != subdirs.end(); subdir++)
    {
      if (*subdir == "." || *subdir == "..")
        continue;

      if (*subdir == NO_THEME)
        continue; // Add this later

      if (QFile::exists(QString("%1/%2/emoticons.xml").arg(*basedir).arg(*subdir)))
      {
        if (*subdir == DEFAULT_THEME)
        {
          defaultExists = true;
          continue; // Add this later
        }

        // Only add unique entires
        if (themes.indexOf(*subdir) == -1)
          themes += *subdir;
      }
    }
  }

  themes.sort();

  // Adding these at the front so that they will be first in the list shown to the user.
  if (defaultExists)
    themes.push_front(translateThemeName(DEFAULT_THEME));
  themes.push_front(translateThemeName(NO_THEME));

  return themes;
}

/**
 * @param dir directory to search in
 * @param file filename (without extension) to search for
 * @returns the full filename or QString::null if no such file exists.
 */
static QString fullFilename(const QString& dir, const QString& file)
{
  const QString base = QString("%1/%2").arg(dir).arg(file);

  if (QFile::exists(base)) // First try without extension
    return base;
  else if (QFile::exists(base + ".png"))
    return base + ".png";
  else if (QFile::exists(base + ".jpg"))
    return base + ".jpg";
  else if (QFile::exists(base + ".gif"))
    return base + ".gif";
  else if (QFile::exists(base + ".mng"))
    return base + ".mng";

  Licq::gLog.warning("Unknown file '%s'", base.toLatin1().constData());
  return QString::null;
}

/**
 * Parses the emoticons.xml file in @a dir.
 * @param emoticons  For every smiley, the first character is added as a key
 *                   and its Emoticon instance is appened to the list.
 * @param fileSmiley Maps the filename of an emoticon to a smiley.
 * @returns true on success; otherwise false.
 *
 * A short emoticons.xml file could look like this:
 * <?xml version="1.0"?>
 * <messaging-emoticon-map >
 *
 * <emoticon file="biggrin">
 * <string>:-&lt;</string>
 * <string>:D</string>
 * </emoticon>
 *
 * <emoticon file="confused">
 * <string>:-S</string>
 * </emoticon>
 *
 * </messaging-emoticon-map>
 */
static bool parseXml(const QString& dir, QMap<QChar, QLinkedList<Emoticon> >* emoticons, QMap<QString, QString>* fileSmiley)
{
  QFile xmlfile(dir + QString::fromLatin1("/emoticons.xml"));
  if (!xmlfile.open(QIODevice::ReadOnly))
    return false;

  QDomDocument doc("emoticons");
  if (!doc.setContent(&xmlfile))
  {
    xmlfile.close();
    return false;
  }
  xmlfile.close();

  QDomElement docElem = doc.documentElement();

  // Walk through all <emoticon> elements
  QDomNode n = docElem.firstChild();
  for (; !n.isNull(); n = n.nextSibling())
  {
    QDomElement e = n.toElement();
    if (!e.isNull() && e.tagName() == QString::fromLatin1("emoticon"))
    {
      const QString file = fullFilename(dir, e.attribute("file"));
      if (file.isNull())
        continue;

      bool first = true;
      QDomNode stringNode = n.firstChild();
      for (; !stringNode.isNull(); stringNode = stringNode.nextSibling())
      {
        // We extract all smileys from <string> elements (<string>smiley</string>).
        // The first one is added to fileSmiley, so that when the user clicks
        // on the icon, this is the smiley that is inserted into the document.
        //
        // All smileys are then indexed in the emoticons map on the first character
        // in the escaped smiley.
        QDomElement string = stringNode.toElement();
        if (!string.isNull() && string.tagName() == QString::fromLatin1("string"))
        {
          Emoticon emo;
          emo.smiley = string.text();
          emo.escapedSmiley = Qt::escape(emo.smiley);
          emo.file = file;

          if (first)
          {
            (*fileSmiley)[emo.file] = emo.smiley;
            first = false;
          }

          // Insert the smiley sorted by length with longest first. This way, if we have
          // a smiley :) with image A and :)) with image B, the string :)) will always
          // be replaced by image B.
          QLinkedList<Emoticon>::iterator it = (*emoticons)[emo.escapedSmiley[0]].begin();
          QLinkedList<Emoticon>::iterator end = (*emoticons)[emo.escapedSmiley[0]].end();
          while (it != end)
          {
#ifdef EMOTICON_DEBUG
            if ((*it).escapedSmiley == emo.escapedSmiley)
              TRACE("The smiley '%s' (%s) is already mapped to %s",
                  emo.smiley.toLatin1().constData(),
                  QFileInfo(file).fileName().toLatin1().constData(),
                  QFileInfo((*it).file).fileName().toLatin1().constData());
#endif
            if ((*it).escapedSmiley.length() < emo.escapedSmiley.length())
              break;
            else
              it++;
          }
          (*emoticons)[emo.escapedSmiley[0]].insert(it, emo);
        }
        else
        {
          Licq::gLog.warning("Element '%s' in '%s' unknown",
              string.tagName().toLatin1().constData(),
              xmlfile.fileName().toLatin1().constData());
        }
      }
    }
  }

  return true;
}

QStringList Emoticons::fileList() const
{
  return pimpl->fileSmiley.keys();
}

// Similar to setTheme(const QString_&) but with the difference that
// here we don't update currentTheme. We're just interested in getting
// the filelist.
QStringList Emoticons::fileList(const QString& theme_in) const
{
  const QString theme = untranslateThemeName(theme_in);

  if (theme.isEmpty() || theme == NO_THEME)
    return QStringList();

  if (theme == pimpl->currentTheme)
    return fileList();

  const QString dir = pimpl->themeDir(theme);
  if (dir.isNull())
    return QStringList();

  QMap<QChar, QLinkedList<Emoticon> > emoticons;
  QMap<QString, QString> fileSmiley;

  const bool parsed = parseXml(dir, &emoticons, &fileSmiley);
  if (parsed)
    return fileSmiley.keys();

  return QStringList();
}

bool Emoticons::setTheme(const QString& theme_in)
{
  const QString theme = untranslateThemeName(theme_in);

  if (theme.isEmpty() || theme == NO_THEME)
  {
    if (pimpl->currentTheme == NO_THEME)
      return true;

    pimpl->currentTheme = NO_THEME;
    pimpl->emoticons.clear();
    pimpl->fileSmiley.clear();
    emit themeChanged();
    return true;
  }

  if (theme == pimpl->currentTheme)
    return true;

  const QString dir = pimpl->themeDir(theme);
  if (dir.isNull())
    return false;

  QMap<QChar, QLinkedList<Emoticon> > emoticons;
  QMap<QString, QString> fileSmiley;

  if (!parseXml(dir, &emoticons, &fileSmiley))
    return false;

  pimpl->currentTheme = theme;
  pimpl->emoticons = emoticons;
  pimpl->fileSmiley = fileSmiley;
  emit themeChanged();
  return true;
}

QString Emoticons::theme() const
{
  return translateThemeName(pimpl->currentTheme);
}

QMap<QString, QString> Emoticons::emoticonsKeys() const
{
  return pimpl->fileSmiley;
}

/**
 * @returns true if s1[start:start+s2.length] == s2
 */
static bool containsAt(const QString& s1, const QString& s2, const uint start)
{
  const uint end = start + s2.length();
  const uint s1_length = static_cast<uint>(s1.length());
  if (s1_length < end || start > s1_length)
    return false;

  for (uint pos = start; pos < end; pos++)
  {
    if (s1[pos] != s2[pos - start])
      return false;
  }
  return true;
}

/**
 * @param message is assumed to be in html, so that all \< is part of a tag
 * @param mode the parsing mode
 */
void Emoticons::parseMessage(QString& message, ParseMode mode) const
{
  // Short-circuit if we don't have any emoticons
  if (pimpl->emoticons.isEmpty())
    return;

  TRACE("message pre: '%s'", message.toLatin1().constData());

  QChar p(' '), c; // previous and current char
  for (int pos = 0; pos < message.length(); pos++)
  {
    c = message[pos];

    if (c == '<')
    {
      // If this is an a tag ("<a "), skip it completly
      if (message[pos + 1] == 'a' && message[pos + 2].isSpace())
      {
        const int index = message.indexOf("</a>", pos);
        if (index == -1)
          return; // Bad html
        pos = index + 3; // Fast-forward pos to point at '>'
      }
      else // Skip just the tag
      {
        const int index = message.indexOf('>', pos);
        if (index == -1)
          return; // Bad html
        pos = index; // Fast-forward pos to point at '>'
      }
      p = '>';
      continue;
    }

    // Only insert smileys after a space in strict and normal mode
    if (mode == StrictMode || mode == NormalMode)
    {
      if (!p.isSpace() && !containsAt(message, QString::fromLatin1("<br />"), pos - 6))
      {
        p = c;
        continue;
      }
    }

    if (pimpl->emoticons.contains(c))
    {
      const QLinkedList<Emoticon> emolist = pimpl->emoticons[c];
      QLinkedList<Emoticon>::ConstIterator it = emolist.begin();

      for (; it != emolist.end(); it++)
      {
        const Emoticon& emo = *it;
        if (containsAt(message, emo.escapedSmiley, pos))
        {
          // In strict and normal mode we need to check the char after the smiley
          if (mode == StrictMode || mode == NormalMode)
          {
            const uint nextPos = pos + emo.escapedSmiley.length();
            const QChar& n = message[nextPos];
            if (!(n.isSpace() || n.isNull() || containsAt(message, QString::fromLatin1("<br"), nextPos)))
            {
              if (mode == StrictMode)
                break;
              else if (!n.isPunct()) // In normal mode we allow punct as well
                break;
            }
          }

          QString img = QString::fromLocal8Bit("<img src=\"file://%1#LICQ%2\">")
            .arg(emo.file)
            .arg(emo.escapedSmiley);
          TRACE("Replacing '%s' with '%s'",
              message.mid(pos, emo.escapedSmiley.length()).toLatin1().constData(),
              img.toLatin1().constData());
          message.replace(pos, emo.escapedSmiley.length(), img);
          pos += img.length() - 1; // Point pos at '>'
          c = '>';
          break;
        }
      }
    }

    p = c;
  }
  TRACE("message post: '%s'", message.toLatin1().constData());
}

/**
 * "unparse" the message, removing all \<img\> tags and replacing them with the smiley.
 */
void Emoticons::unparseMessage(QString& message)
{
  QRegExp deicon("<img src=\"file://.*#LICQ(.*)\".*>");
  deicon.setMinimal(true);
  message.replace(deicon, "\\1");
}
