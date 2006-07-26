/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2003-2006 Licq developers
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef USE_KDE
# include <kapplication.h>
#else
# include <qapplication.h>
#endif

#include <qmap.h>
#include <qdir.h>
#include <qdom.h>
#include <qstylesheet.h>

#include "licq_log.h"

#include "emoticon.h"

static const QString DEFAULT_THEME("Default");
static const QString NO_THEME("None");

struct Emoticon
{
  QString file;
  QString smiley;
  QString escapedSmiley;
};

/// Private data and functions for CEmotions
class CEmoticons::Impl
{
public:
  QStringList basedirs;
  QString currentTheme;

  // Maps first char in smiley to its Emoticon instance.
  QMap<QChar, QValueList<Emoticon> > emoticons;

  // Maps an emoticon's filename to a smiley.
  QMap<QString, QString> fileSmiley;

  QString themeDir(const QString &theme) const;
};

/**
 * @returns the full path to @a theme, or QString::null if no such theme was found.
 */
QString CEmoticons::Impl::themeDir(const QString &theme) const
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
CEmoticons::CEmoticons()
#ifdef USE_KDE
  : QObject(kapp)
#else
  : QObject(qApp)
#endif
{
  pimpl = new Impl;
  pimpl->currentTheme = NO_THEME;
}

CEmoticons::~CEmoticons()
{
  delete pimpl;
}

CEmoticons *CEmoticons::m_self = 0L;
CEmoticons* CEmoticons::self()
{
  if (!m_self)
    m_self = new CEmoticons;
  return m_self;
}

void CEmoticons::setBasedirs(const QStringList &basedirs)
{
  pimpl->basedirs = basedirs;
}

/**
 * In every subdir in every basedir, we check for a file
 * named emoticons.xml, and if we find one, subdir is added
 * to the list of themes.
 */
QStringList CEmoticons::themes() const
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
        if (themes.find(*subdir) == themes.end())
          themes += *subdir;
      }
    }
  }

  themes.sort();

  if (defaultExists)
    themes.push_front(DEFAULT_THEME);
  themes.push_front(NO_THEME);

  return themes;
}

/**
 * @param dir directory to search in
 * @param file filename (without extension) to search for
 * @returns the full filename or QString::null if no such file exists.
 */
QString fullFilename(const QString &dir, const QString &file)
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

  gLog.Warn("%sUnknown file '%s'.\n", L_WARNxSTR, base.latin1());
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
bool parseXml(const QString &dir, QMap<QChar, QValueList<Emoticon> > *emoticons, QMap<QString, QString> *fileSmiley)
{
  QFile xmlfile(dir + QString::fromLatin1("/emoticons.xml"));
  if (!xmlfile.open(IO_ReadOnly))
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
        // All smileys are then indexed on the first character in the emoticons
        // map. Both the original and the escaped (< replaced with &lt; etc)
        // versions are inserted (if the differ).
        QDomElement string = stringNode.toElement();
        if (!string.isNull() && string.tagName() == QString::fromLatin1("string"))
        {
          Emoticon emo;
          emo.smiley = string.text();
          emo.escapedSmiley = QStyleSheet::escape(emo.smiley);
          emo.file = file;

          if (first)
          {
            (*fileSmiley)[emo.file] = emo.smiley;
            first = false;
          }

          //emoticons[emo.smiley[0]].append(emo);
          //if (emo.smiley != emo.escapedSmiley)
          (*emoticons)[emo.escapedSmiley[0]].append(emo);
        }
        else
        {
          gLog.Warn("%sElement '%s' in '%s' unknown.\n", L_WARNxSTR,
                    string.tagName().latin1(), xmlfile.name().latin1());
        }
      }
    }
  }

  return true;
}

QStringList CEmoticons::fileList() const
{
  return pimpl->fileSmiley.keys();
}

// Similar to setTheme(const QString_&) but with the difference that
// here we don't update currentTheme. We're just interested in getting
// the filelist.
QStringList CEmoticons::fileList(const QString &theme) const
{
  if (theme.isEmpty() || theme == NO_THEME)
    return QStringList();

  if (theme == pimpl->currentTheme)
    return fileList();

  const QString dir = pimpl->themeDir(theme);
  if (dir.isNull())
    return QStringList();

  QMap<QChar, QValueList<Emoticon> > emoticons;
  QMap<QString, QString> fileSmiley;

  const bool parsed = parseXml(dir, &emoticons, &fileSmiley);
  if (parsed)
    return fileSmiley.keys();

  return QStringList();
}

bool CEmoticons::setTheme(const QString &theme)
{
  if (theme.isEmpty() || theme == NO_THEME)
  {
    pimpl->currentTheme = NO_THEME;
    pimpl->emoticons.clear();
    pimpl->fileSmiley.clear();
    return true;
  }

  if (theme == pimpl->currentTheme)
    return true;

  const QString dir = pimpl->themeDir(theme);
  if (dir.isNull())
    return false;

  QMap<QChar, QValueList<Emoticon> > emoticons;
  QMap<QString, QString> fileSmiley;

  const bool parsed = parseXml(dir, &emoticons, &fileSmiley);
  if (parsed)
  {
    pimpl->currentTheme = theme;
    pimpl->emoticons = emoticons;
    pimpl->fileSmiley = fileSmiley;
    emit themeChanged();
  }

  return parsed;
}

QString CEmoticons::theme() const
{
  return pimpl->currentTheme;
}

QMap<QString, QString> CEmoticons::emoticonsKeys() const
{
  return pimpl->fileSmiley;
}

/**
 * @returns true if s1[start:start+s2.length] == s2
 */
static bool containsAt(const QString &s1, const QString &s2, const uint start)
{
  const uint end = start + s2.length();
  if (s1.length() < end)
    return false;

  for (uint pos = start; pos < end; pos++)
  {
    if (s1[pos] != s2[pos - start])
      return false;
  }
  return true;
}

void CEmoticons::parseMessage(QString &message) const
{
  // Short-circuit if we don't have any emoticons
  if (pimpl->emoticons.isEmpty())
    return;

//   qDebug("message b: '%s'", message.latin1());

  QChar p(' '), c; // previous and current char
  for (uint pos = 0; pos < message.length(); pos++)
  {
    c = message[pos];
    if (c == '<')
    {
      // If this is an a tag, skip it completly
      if (message[pos + 1] == 'a')
      {
        const int index = message.find("</a>", pos);
        if (index == -1)
          return; // We're done
//         qDebug(" Skipping '%s', point at '%c'", message.mid(pos, index + 3 - pos).latin1(), message[index + 3].latin1());
        pos = index + 3; // Fast-forward pos to point at '>'
      }
      else // Skip just the tag
      {
        const int index = message.find('>', pos);
        if (index == -1)
          return; // We're done
//         qDebug(" Skipping '%s', point at '%c'", message.mid(pos, index - pos).latin1(), message[index].latin1());
        pos = index; // Fast-forward pos to point at '>'
      }
      p = '>';
      continue;
    }

    // Only insert smileys after a space, tag or html entitiy
    if (!p.isSpace() && p != '>' && p != ';')
    {
      p = c;
      continue;
    }

    if (pimpl->emoticons.contains(c))
    {
      const QValueList<Emoticon> emolist = pimpl->emoticons[c];
      QValueList<Emoticon>::ConstIterator it = emolist.begin();
      for (; it != emolist.end(); it++)
      {
        const Emoticon &emo = *it;
        if (containsAt(message, emo.escapedSmiley, pos))
        {
          const QString img = QString::fromLatin1("<img src=\"%1\" />&nbsp;").arg(emo.file);
//           qDebug(" Replacing '%s' with '%s'", message.mid(pos, emo.escapedSmiley.length()).latin1(), img.latin1());
          message.replace(pos, emo.escapedSmiley.length(), img);
          pos += img.length() - 1; // Point pos at ';'
//           qDebug(" Pointing at '%c'", message[pos].latin1());
          c = ';';
          break;
        }
      }
    }

    p = c;
  }
//   qDebug("message a: '%s'", message.latin1());
}

#include "emoticon.moc"

#ifdef EMOTICON_TEST_DRIVER
#include <stdio.h>
#include <iostream>

int
main(int argc, char **argv)
{
  CEmoticons *e;

  if (argc != 1)
    e = new CEmoticons(argv[1], argv[2]);
  else
  {
    std::cout << "missing theme dir\n";
    return 0;
  }

  QStringList themes = e->Themes();

  std::cout << "Themes available:\n";
  for ( QStringList::Iterator it = themes.begin();
         it != themes.end(); ++it )
    std::cout << "\t" << *it << "\n";

  std::cout << "\n";
  printf("Current theme %s\n",e->Theme());
  std::cout << "Set Theme: to abcde(" << e->SetTheme("abcde") << ")\n";
  printf("Current theme %s\n",e->Theme());

  std::cout << "\n";
  printf("Current theme %s\n",e->Theme());
  
  const char *d = "Default";
  std::cout << "Set Theme: to (" << d << " " << e->SetTheme(d) << ")\n";
  printf("Current theme %s\n",e->Theme());

  std::cout << "\n";
  std::cout << "Filelist:\n";
  QStringList files = e->fileList();
  for ( QStringList::Iterator it = files.begin(); it != files.end(); ++it)
    std::cout << "\t" << *it << "\n";

  std::cout << "\nFilelist for theme: KMess\n";
  files = e->fileList("KMess");
  for( QStringList::Iterator it = files.begin(); it != files.end(); ++it)
    std::cout << "\t" << *it << "\n";

  QString s = "hello word :) :( :P :-) :P";
  std::cout << "-- Before\n";
  std::cout << s;
  e->ParseMessage(s);
  std::cout << "\n-- After\n";
  std::cout << s;
  std::cout << "\n";

  return 0;\
}
#endif
