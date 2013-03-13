/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2010, 2013 Licq developers <licq-dev@googlegroups.com>
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

#include "conf.h"

#include <licq/inifile.h>

template <typename T, typename U>
static void getOrSet(Licq::IniFile& conf, const std::string& key,
                     T& value, const U& defaultValue)
{
  if (!conf.get(key, value, defaultValue))
    conf.set(key, value);
}

template <typename T, typename U>
static void getOrSetWithLimit(Licq::IniFile& conf, const std::string& key,
                              T& value, const U& defaultValue, const U& limit)
{
  unsigned int temp;
  if (!conf.get(key, temp, static_cast<unsigned int>(defaultValue))
      || temp > static_cast<unsigned int>(limit))
  {
    temp = defaultValue;
    conf.set(key, temp);
  }

  value = static_cast<T>(temp);
}

void Conf::loadConfig()
{
  Licq::IniFile conf("licq_aosd.ini");
  conf.loadFile();

  conf.setSection("Appearance");

  getOrSet(conf, "Font", font, "");
  getOrSet(conf, "MarkSecureMessages", markSecure, true);

  conf.setSection("Colouring");

  getOrSet(conf, "Background", backColor, "");
  getOrSet(conf, "Message", textColor, "yellow");
  getOrSet(conf, "Control", textControlColor, "gray");
  getOrSet(conf, "Shadow", shadowColor, "black");
  getOrSetWithLimit(conf, "BackOpacity", backOpacity, 0, 255);
  getOrSetWithLimit(conf, "TextOpacity", textOpacity, 255, 255);
  getOrSetWithLimit(conf, "ShadowOpacity", shadowOpacity, 255, 255);

  conf.setSection("Geometry");

  getOrSetWithLimit(conf, "HPosition", posHorizontal,
                    COORDINATE_MINIMUM, COORDINATE_MAXIMUM);
  getOrSetWithLimit(conf, "VPosition", posVertical,
                    COORDINATE_MINIMUM, COORDINATE_MAXIMUM);

  getOrSet(conf, "HOffset", offsetHorizontal, 0);
  getOrSet(conf, "VOffset", offsetVertical, 0);

  getOrSet(conf, "HMargin", marginHorizontal, 0);
  getOrSet(conf, "VMargin", marginVertical, 0);

  getOrSet(conf, "ShadowOffset", shadowOffset, 2);
  getOrSet(conf, "Width", wrapWidth, 0);
  getOrSet(conf, "Lines", maxLines, 0);

  conf.setSection("Timing");

  getOrSet(conf, "FadeIn", fadeIn, 150);
  getOrSet(conf, "FadeFull", fadeFull, 3000);
  getOrSet(conf, "FadeOut", fadeOut, 150);
  getOrSet(conf, "DelayPerCharacter", delayPerChar, 0);

  conf.setSection("Behavior");

  getOrSet(conf, "Wait", wait, true);
  getOrSet(conf, "MouseActive", mouseActive, true);
  getOrSet(conf, "QuietTimeout", quietTimeout, 0);

  conf.setSection("Filtering");

  getOrSetWithLimit(conf, "ShowLogonLogoff", logonLogoff,
                    GROUP_TYPE_ALL, GROUP_TYPE_ALL);
  getOrSetWithLimit(conf, "ShowStatusChange", statusChange,
                    GROUP_TYPE_ALL, GROUP_TYPE_ALL);
  getOrSetWithLimit(conf, "ShowAutoResponseCheck", autoResponse,
                    GROUP_TYPE_ALL, GROUP_TYPE_ALL);
  getOrSetWithLimit(conf, "ShowMessages", showMessage,
                    GROUP_TYPE_ALL, GROUP_TYPE_ALL);

  getOrSet(conf, "ShowMessagesNotification", notifyOnly, false);
  getOrSet(conf, "ShowInModes", ownerModes, "");
  getOrSet(conf, "ShowMsgsInModes", ownerModesMsg, "");

  conf.writeFile();
}

/* vim: set ts=2 sw=2 et : */
