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

#ifndef EMOTICONS_H
#define EMOTICONS_H

#include <QMap>
#include <QObject>

namespace LicqQtGui
{
class Emoticons : public QObject
{
  Q_OBJECT;

public:
  /// Theme name constants (not translated).
  static const QString DEFAULT_THEME;
  static const QString NO_THEME;

  /// Helper functions so that when we save the theme name we can save
  /// it untranslated, and later when we load it we can translate it again before
  /// setting the theme.
  static QString translateThemeName(const QString& name);
  static QString untranslateThemeName(const QString& name);

  Emoticons();
  virtual ~Emoticons();

  /// Get singleton instance
  static Emoticons* self();

  /// Set dirs to search themes in to @a basedirs
  void setBasedirs(const QStringList& basedirs);

  /// @returns the list of available (translated) theme names
  QStringList themes() const;

  /// @returns the list of files of the current emoticon theme
  QStringList fileList() const;

  /// @param theme is the translated name of the theme
  /// @returns the list of files for @a theme
  QStringList fileList(const QString& theme) const;

  /// Loads @a theme and returns true; or false if @a theme could not be loaded.
  /// @param theme is the translated name of the theme
  bool setTheme(const QString& theme);

  /// @returns the current theme name (translated)
  QString theme() const;

  /// @returns a mapping of files to smileys
  QMap<QString, QString> emoticonsKeys() const;

  /**
   * In no mode is any replacing done within "<a ...</a>" or "<...>".
   * StrictMode: Require a blank (space) before and after the smiley.
   * NormalMode: Require a blank (space) before and a blank or punctuation after the smiley.
   * RelaxedMode: Anything matching a smiley is replaced.
   */
  enum ParseMode { StrictMode, NormalMode, RelaxedMode };

  /// Replaces all smileys in @a message with their icon.
  void parseMessage(QString& message, ParseMode mode) const;

  /// Replace all emoticons with their smiley
  static void unparseMessage(QString& message);

signals:
  void themeChanged();

private:
  Emoticons(const Emoticons&);
  Emoticons& operator=(const Emoticons&);

  static Emoticons* m_self;

  class Impl;
  Impl* pimpl;
};

} // namespace LicqQtGui

#endif // EMOTICONS_H
