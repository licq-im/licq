/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2011 Licq developers
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

#ifndef CONF_H
#define CONF_H

#include <string>

#include <aosd.h>

typedef enum
{
  GROUP_TYPE_NONE = 0,
  GROUP_TYPE_ONLINE_NOTIFY,
  GROUP_TYPE_ALL
} UserGroupType;

class Conf
{
private:
  void loadConfig();

  bool markSecure;
  bool mouseActive;
  bool notifyOnly;
  bool wait;

  std::string backColor;
  std::string font;
  std::string ownerModes;
  std::string ownerModesMsg;
  std::string shadowColor;
  std::string textColor;
  std::string textControlColor;

  AosdCoordinate posHorizontal;
  AosdCoordinate posVertical;

  int offsetHorizontal;
  int offsetVertical;
  int shadowOffset;
  int wrapWidth;
  int maxLines;

  unsigned backOpacity;
  unsigned delayPerChar;
  unsigned fadeFull;
  unsigned fadeIn;
  unsigned fadeOut;
  unsigned marginHorizontal;
  unsigned marginVertical;
  unsigned quietTimeout;
  unsigned shadowOpacity;
  unsigned textOpacity;

  UserGroupType autoResponse;
  UserGroupType logonLogoff;
  UserGroupType showMessage;
  UserGroupType statusChange;

  friend class Iface;
};

#endif

/* vim: set ts=2 sw=2 et : */
