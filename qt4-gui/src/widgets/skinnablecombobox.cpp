// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007 Licq developers
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

#include "skinnablecombobox.h"

#include "config/skin.h"

using namespace LicqQtGui;

SkinnableComboBox::SkinnableComboBox(const Config::ComboSkin& skin, QWidget* parent)
  : QComboBox(parent)
{
  applySkin(skin);
}

SkinnableComboBox::SkinnableComboBox(QWidget* parent)
  : QComboBox(parent)
{
}

void SkinnableComboBox::applySkin(const Config::ComboSkin& skin)
{
  // Set colors
  QPalette pal;
  if (skin.background.isValid())
  {
    pal.setColor(QPalette::Base, skin.background);
    pal.setColor(QPalette::Button, skin.background);
  }
  if (skin.foreground.isValid())
  {
    pal.setColor(QPalette::Text, skin.foreground);
    pal.setColor(QPalette::ButtonText, skin.foreground);
  }
  setPalette(pal);
}
