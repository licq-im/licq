/*
 * Licq - A ICQ Client for Unix
 *
 * Copyright (C) 2003 Licq developers <licq-devel@lists.sourceforge.net>
 *
 * This program is licensed under the terms found in the LICENSE file.
 *
 * \file support for emoticons themes. Compatible with kopete 0.6 format.
 * \todo lot of improvents (memory vs time)
 */

#include <list>
#include <qmap.h>
#include <qdir.h>
#include <qdom.h>
#include <qregexp.h>

#include "licq_log.h"

#include "emoticon.h"

struct node
{
  QStringList  emoticon;
  QString      file;
  QRegExp      reg;
};

typedef std::list<struct node> node_list_t;

/*! private definition of CEmotions */
struct Emoticons
{
  QString basedir;     /* base directory for resourses */
  QString altbasedir;  /* alternative base directory for resourses */
  QString theme;       /* current theme */

  node_list_t emoticons;
};

CEmoticons::CEmoticons(const char *basedir, const char *altbasedir, 
                       const char *theme  )
{
  this->data = new struct Emoticons;
  data->basedir = basedir;
  data->altbasedir = altbasedir;
}

CEmoticons::~CEmoticons()
{
  delete this->data;
}

QStringList CEmoticons::Themes()
{
  QDir    dir(data->basedir,    "*", 0, QDir::Dirs);
  QDir altdir(data->altbasedir, "*", 0, QDir::Dirs);
  QStringList list = dir.entryList().grep(QRegExp("^[^.].*")) + 
                     altdir.entryList().grep(QRegExp("^[^.].*"));

  // unique 
  QString last = "";
  list.sort();
  for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
  {
     // std::cout << "\t" << *it << "\n";
     if( *it == last )
        it = list.remove(it);
     else
        last = *it;
  }
  
  return list;
}

/*! 
 * \returns the real path for the image file in the themedir if exists
 *
 *  \param data     CDT for CEmoticons
 *  \param themedir path to the theme's directory
 *  \param file     file (without extension) to check
 */
static QString realFile(const struct Emoticons *data, const QString &themedir,
                     const QString &file)
{
  QString base;

  if (file != QString::null)
  {

    base = themedir + "/" + file;
    if (QFile(base + ".png").exists())
      base += ".png";
    else if (QFile(base + ".jpg").exists())
      base += ".jpg";
    else if (QFile(base + ".gif").exists())
      base += ".gif";
    else
    {
      gLog.Warn("%sWarning unknown file `%s'\n", L_WARNxSTR, base.ascii());
      base = QString::null;
    }
  }

  return base;
}

/*!
 * helper function for #loadTheme
 *
 * \return a list of ascii emoticons
 *
 * <string>:^)</string>
 * <string>:)</string>
 * <string>:-)</string>
 *
 */
static QStringList loadStrings(const struct Emoticons *data, QDomNode node,
                               unsigned *n)
{
  QStringList ret;

  *n = 0U;

  for (; !node.isNull() ; node=node.nextSibling())
  {
    QDomElement emo = node.toElement();
    if (!emo.isNull() && emo.tagName() == "string")
    {
      if (!emo.text().isEmpty())
      {
        (*n) +=1;
        ret << emo.text();
      }
    }
    else
      gLog.Warn("%sWarning element `%s'", L_WARNxSTR, emo.tagName().ascii());
  }

  return ret;
}

static void create_regexp(QStringList &list, QRegExp &reg)
{
  unsigned n = 0;
  QString s = "(";

  for (QStringList::Iterator it = list.begin(); it!=list.end(); ++it)
  {
    if (n != 0)
      s += "|";
#if QT_VERSION < 0x030100
    // we have to implement the functionality of QRegExp::escape() 
    // ourself since qt 3.0.x is missing it.
    // Our goal is to escape all special characters with a backslash:
    // The special characters are $, (, ), *, +, ., ?, [, \, ], ^, {, | and }.
    // The implementation is heavily inspired by QT QRegExp sources ;-)
    
    static const char *c = "\\$()*+.?[]^{}|";
    int i = 0;
    QString tmp = (*it).latin1();
    while (i < (int)tmp.length())
    {
      if (strstr(c, tmp.mid(i,1).latin1()) != 0)
        tmp.insert( i++, "\\");
      i++;
    }
    s += tmp;
#else
    s += QRegExp::escape(*it);
#endif
    n++;
  }
  s += ")";
  reg = QRegExp(s);
}

/*!
 *
 * \param data       CDT for the CEmoticon class
 * \param themedir   path to the theme directory
 * \param list       list where the results are stored
 *
 * \returns the number of loaded emoticons
 */
static unsigned loadTheme(const struct Emoticons *data,
                          const QString &themedir,
                          node_list_t &list)
{
  QDomDocument doc("doc");
  QFile file(themedir + "/emoticons.xml");
  unsigned ret = 0;

  if (file.open(IO_ReadOnly) && doc.setContent(&file))
  {
    QDomElement elem = doc.documentElement();
    QDomNode n = elem.firstChild();
    for (; !n.isNull(); n = n.nextSibling())
    {
      if (n.isElement())
      {
        elem = n.toElement();
        if (!elem.isNull() && elem.tagName() == QString::fromLatin1("emoticon"))
        {
          QString file = elem.attribute("file");
          QString f=realFile(data,themedir,file);
          if (f != QString::null)
          {
            struct node node;
            unsigned size;

            node.emoticon = loadStrings(data, n.firstChild(), &size);
	    if (size)
	    {
	      node.file = f;
	      create_regexp(node.emoticon, node.reg);
	      list.push_back(node);
	      ret += size;
	    }
          }
        }
      }
    }
  }

  file.close();
  return ret;
};

int CEmoticons::SetTheme(const char *theme)
{
  if (strcmp(theme, "None") == 0)
  {
    data->theme = QString("None");
    return 1;
  }

  QString szdir1 = data->altbasedir + "/" + theme + "/";
  QString szdir2 = data->basedir    + "/" + theme + "/";
  QDir d1(szdir1);
  QDir d2(szdir2);
  node_list_t list;
  int ret = -1;
  unsigned n = 0;

  if (d1.exists())
    n = loadTheme(data, szdir1.ascii(), list);
  else if(d2.exists())
    n = loadTheme(data, szdir2.ascii(), list);


  if (n)
  {
    ret = n;
    data->theme = theme;
    data->emoticons = list;
  }

  return ret;
}
  
const char *CEmoticons::Theme(void)
{
  return data->theme == QString::null ? 0 : data->theme.ascii() ;
}
  
QStringList CEmoticons::fileList()
{
  node_list_t::iterator iter;
  QStringList ret;
  struct node n;
  
  for( iter  = data->emoticons.begin();
       iter != data->emoticons.end() ;
       iter++ )
  {
    n = *iter;
    ret << n.file;
  }

  return ret;
}

QStringList CEmoticons::fileList(const char *theme)
{
  QString szdir = data->basedir + "/" + theme + "/";
  QString szaltdir = data->altbasedir + "/" + theme + "/";
  QStringList ret;
  QDir d(szdir);
  QDir altd(szaltdir);
  node_list_t list;
  node_list_t::iterator iter;
  struct node n;

  if (d.exists())
    loadTheme(data, szdir.ascii(), list);
  else if (altd.exists())
    loadTheme(data, szaltdir.ascii(), list);

  if (d.exists() || altd.exists())
  {
    for (iter  = list.begin();
         iter != list.end(); iter++)
    {
      n = *iter;
      ret << n.file;
    }
  }

  return ret;
}

QMap<QString, QString> CEmoticons::EmoticonsKeys()
{
  QMap<QString, QString> map;
  
  node_list_t list = data->emoticons;
  node_list_t::iterator iter;
  for (iter = list.begin(); iter != list.end(); ++iter)
  {
    map[(*iter).file] = (*iter).emoticon.first();
  }
  
  return map;
}

void CEmoticons::ParseMessage(QString &msg)
{
  /**
   * \todo this sucks: solution create a finite state machine to parse
   * the message
   */
  node_list_t::iterator iter;
  struct node n;

  if (data->theme != QString::null && data->theme != "None")
  {
    QString r;
    for( iter  = data->emoticons.begin();
         iter != data->emoticons.end() ; iter++ )
    {
      n = *iter;
      for ( QStringList::Iterator it = n.emoticon.begin();
             it != n.emoticon.end(); ++it)
      {
        msg.replace(n.reg," <img src=\""+n.file+"\"/>&nbsp;");
      }
    }
  }
}

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
