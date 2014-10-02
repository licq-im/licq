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

#ifndef SPELLCHECKER_H
#define SPELLCHECKER_H

#include "config.h"

#include <QSyntaxHighlighter>

class Hunspell;

namespace LicqQtGui
{

/**
 * Syntax highlighter that will mark misspelled words
 */
class SpellChecker : public QSyntaxHighlighter
{
  Q_OBJECT
public:
  /**
   * Constructor
   *
   * @param parent Document to perform spell checking on
   * @param dicFile Dictionary file to use
   */
  SpellChecker(QTextDocument* parent, const QString& dicFile);

  /**
   * Destructor
   */
  ~SpellChecker();

  /**
   * Get a list of suggestions for a misspelled word
   *
   * @param word Word to get suggestions for
   * @return List of suggestions or empty if word is valid
   */
  QStringList getSuggestions(const QString& word);

  /**
   * Check spelling of a single word
   *
   * @param word Word to check
   * @return True if word is valid
   */
  bool checkWord(const QString& word);

public slots:
  /**
   * Set dictionary file to use
   *
   * @param dicFile Dictionary file to use
   */
  void setDictionary(const QString& dicFile);

protected:
  /**
   * Perform spell checking on a block of text
   *
   * @param text Text to spell check
   */
  virtual void highlightBlock(const QString& text);

private:
  QString myDicFile;
  Hunspell* mySpeller;
  QTextCodec* mySpellerCodec;
};

} // namespace LicqQtGui

#endif
