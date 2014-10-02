/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2009 Licq developers
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

#include "spellchecker.h"

#include <QFileInfo>
#include <QRegExp>
#include <QSyntaxHighlighter>
#include <QTextCodec>

#include <hunspell/hunspell.hxx>


using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::SpellChecker */

SpellChecker::SpellChecker(QTextDocument* parent, const QString& dicFile)
  : QSyntaxHighlighter(parent),
    mySpeller(NULL)
{
  setDictionary(dicFile);
}

SpellChecker::~SpellChecker()
{
  if (mySpeller != NULL)
    delete mySpeller;
}

void SpellChecker::setDictionary(const QString& dicFile)
{
  if (dicFile == myDicFile)
    return;
  myDicFile = dicFile;

  if (mySpeller != NULL)
  {
    delete mySpeller;
    mySpeller = NULL;
  }

  // If dicFile is empty or refers to a non-existent file, disable spell checking
  if (dicFile.isEmpty() || !QFileInfo(dicFile).isReadable())
    return;

  QString affFile = dicFile.left(dicFile.lastIndexOf('.')) + ".aff";
  mySpeller = new Hunspell(affFile.toLatin1(), dicFile.toLatin1());
  mySpellerCodec = QTextCodec::codecForName(mySpeller->get_dic_encoding());
}

void SpellChecker::highlightBlock(const QString& text)
{
  if (mySpeller == NULL)
    return;

  QTextCharFormat myBadSpelling;
  myBadSpelling.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
  myBadSpelling.setUnderlineColor(Qt::red);

  QRegExp wordSplit("\\b\\w+\\b");
  int pos = 0;
  while ((pos = wordSplit.indexIn(text, pos)) != -1)
  {
    int len = wordSplit.matchedLength();
    if (!checkWord(wordSplit.cap()))
      setFormat(pos, len, myBadSpelling);
    pos += len;
  }
}

QStringList SpellChecker::getSuggestions(const QString& word)
{
  if (mySpeller == NULL || checkWord(word))
    return QStringList();

  char** wordList;
  int count = mySpeller->suggest(&wordList, mySpellerCodec->fromUnicode(word).data());
  if (count <= 0)
    return QStringList();

  QStringList ret;
  for (int i = 0; i < count; ++i)
    ret.append(mySpellerCodec->toUnicode(wordList[i]));
  mySpeller->free_list(&wordList, count);

  return ret;
}

bool SpellChecker::checkWord(const QString& word)
{
  if (mySpeller == NULL)
    return true;

  return (mySpeller->spell(mySpellerCodec->fromUnicode(word).data()) != 0);
}
