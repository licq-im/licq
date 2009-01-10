// -*- c-basic-offset: 2 -*-
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

#include "shortcuts.h"

#include <licq_file.h>

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::Config::Shortcuts */

Config::Shortcuts* Config::Shortcuts::myInstance = NULL;

void Config::Shortcuts::createInstance(QObject* parent)
{
  myInstance = new Config::Shortcuts(parent);
}

Config::Shortcuts::Shortcuts(QObject* parent)
  : QObject(parent),
    myBlockUpdates(false),
    myShortcutsHasChanged(false)
{
  // Define all shortcuts here by populating config file keys and default values in the maps

#define ADD_SHORTCUT(function, inikey, def) \
  myDefaultShortcutsMap.insert(function, def); \
  myConfigKeysMap.insert(function, inikey);


#undef ADD_SHORTCUT
}

void Config::Shortcuts::loadConfiguration(CIniFile& iniFile)
{
  iniFile.SetSection("shortcuts");

  QMap<ShortcutType, QString>::iterator i;
  for (i = myConfigKeysMap.begin(); i != myConfigKeysMap.end(); ++i)
  {
    std::string s;
    iniFile.readString(i.value().toAscii().data(), s);
    if (s.empty())
      myShortcutsMap[i.key()] = QKeySequence(myDefaultShortcutsMap[i.key()]);
    else
      myShortcutsMap[i.key()] = QKeySequence(s.c_str());
  }

  emit shortcutsChanged();
}

void Config::Shortcuts::saveConfiguration(CIniFile& iniFile) const
{
  iniFile.SetSection("shortcuts");

  QMap<ShortcutType, QString>::const_iterator i;
  for (i = myConfigKeysMap.begin(); i != myConfigKeysMap.end(); ++i)
    iniFile.writeString(i.value().toAscii().data(),
        myShortcutsMap[i.key()].toString(QKeySequence::PortableText).toLatin1().data());
}

void Config::Shortcuts::blockUpdates(bool block)
{
  myBlockUpdates = block;

  if (block)
    return;

  if (myShortcutsHasChanged)
  {
    myShortcutsHasChanged = false;
    emit shortcutsChanged();
  }
}

void Config::Shortcuts::setShortcut(ShortcutType function, const QKeySequence& shortcut)
{
  QKeySequence& var = myShortcutsMap[function];
  if (var == shortcut)
    return;

  var = shortcut;

  if (myBlockUpdates)
    myShortcutsHasChanged = true;
  else
    emit shortcutsChanged();
}
