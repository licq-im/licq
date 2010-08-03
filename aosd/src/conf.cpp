/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2010 Licq developers
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

void Conf::loadConfig()
{
  unsigned int tmp;
  Licq::IniFile conf("licq_aosd.ini");
  conf.loadFile();

#define GET(key, dst, def); \
  if (!conf.get((key), (dst), (def))) \
    conf.set((key), (dst));

#define GETTMP(key, dst, def, limit) \
  GET((key), tmp, static_cast<unsigned int>((def))); \
  if (tmp > static_cast<unsigned int>((limit))) \
  { \
    tmp = (dst) = (def); \
    conf.set((key), tmp); \
  } \
  else \
    (dst) = static_cast<typeof((dst))>(tmp);

  conf.setSection("Appearance");

  GET("Font", font, "");
  GET("MarkSecureMessages", markSecure, true);

  conf.setSection("Colouring");

  GET("Background", backColor, "");
  GET("Message", textColor, "yellow");
  GET("Control", textControlColor, "gray");
  GET("Shadow", shadowColor, "black");
  GETTMP("BackOpacity", backOpacity, 0, 255);
  GETTMP("TextOpacity", textOpacity, 255, 255);
  GETTMP("ShadowOpacity", shadowOpacity, 255, 255);

  conf.setSection("Geometry");

  GETTMP("HPosition", posHorizontal, COORDINATE_MINIMUM, COORDINATE_MAXIMUM);
  GETTMP("VPosition", posVertical, COORDINATE_MINIMUM, COORDINATE_MAXIMUM);

  GET("HOffset", offsetHorizontal, 0);
  GET("VOffset", offsetVertical, 0);

  GET("HMargin", marginHorizontal, 0);
  GET("VMargin", marginVertical, 0);

  GET("ShadowOffset", shadowOffset, 2);
  GET("Width", wrapWidth, 0);
  GET("Lines", maxLines, 0);

  conf.setSection("Timing");

  GET("FadeIn", fadeIn, 150);
  GET("FadeFull", fadeFull, 3000);
  GET("FadeOut", fadeOut, 150);
  GET("DelayPerCharacter", delayPerChar, 0);

  conf.setSection("Behavior");

  GET("Wait", wait, true);
  GET("MouseActive", mouseActive, true);
  GET("QuietTimeout", quietTimeout, 0);

  conf.setSection("Filtering");

  GETTMP("ShowLogonLogoff", logonLogoff, GROUP_TYPE_ALL, GROUP_TYPE_ALL);
  GETTMP("ShowStatusChange", statusChange, GROUP_TYPE_ALL, GROUP_TYPE_ALL);
  GETTMP("ShowAutoResponseCheck", autoResponse, GROUP_TYPE_ALL, GROUP_TYPE_ALL);
  GETTMP("ShowMessages", showMessage, GROUP_TYPE_ALL, GROUP_TYPE_ALL);

  GET("ShowMessagesNotification", notifyOnly, false);
  GET("ShowInModes", ownerModes, "");
  GET("ShowMsgsInModes", ownerModesMsg, "");

#undef GET
#undef GETTMP

  conf.writeFile();
}

/* vim: set ts=2 sw=2 et : */
