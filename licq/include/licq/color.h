/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1998-2012 Licq developers <licq-dev@googlegroups.com>
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
#ifndef LICQ_COLOR_H
#define LICQ_COLOR_H

namespace Licq
{

class Color
{
public:
  /// Default constructor
  Color();

  /// Copy constructor
  Color(const Color& c);

  /// Copy constructor, acts as default if c is NULL
  Color(const Color* c);

  static unsigned redPart(unsigned color) { return color & 0xff; }
  static unsigned greenPart(unsigned color) { return (color >> 8) & 0xff; }
  static unsigned bluePart(unsigned color) { return (color >> 16) & 0xff; }

  unsigned foreRed() const { return redPart(foreground()); }
  unsigned foreGreen() const { return greenPart(foreground()); }
  unsigned foreBlue() const { return bluePart(foreground()); }
  unsigned backRed() const { return redPart(background()); }
  unsigned backGreen() const { return greenPart(background()); }
  unsigned backBlue() const { return bluePart(background()); }

  unsigned foreground() const { return myForeground; }
  unsigned background() const { return myBackground; }

  void set(unsigned fore, unsigned back);
  void set(const Color* c);
  void setBackground(unsigned r, unsigned g, unsigned b);
  void setForeground(unsigned r, unsigned g, unsigned b);
  void setToDefault();

  // Default colors for sending messages
  static unsigned defaultForeRed() { return redPart(defaultForeground()); }
  static unsigned defaultForeGreen() { return greenPart(defaultForeground()); }
  static unsigned defaultForeBlue() { return bluePart(defaultForeground()); }
  static unsigned defaultBackRed() { return redPart(defaultBackground()); }
  static unsigned defaultBackGreen() { return greenPart(defaultBackground()); }
  static unsigned defaultBackBlue() { return bluePart(defaultBackground()); }

  static unsigned defaultForeground() { return myDefaultForeground; }
  static unsigned defaultBackground() { return myDefaultBackground; }

  static void setDefaultColors(const Color* c);
  static void setDefaultForeground(unsigned fore);
  static void setDefaultBackground(unsigned back);

protected:
  unsigned myForeground;
  unsigned myBackground;

  static unsigned myDefaultForeground;
  static unsigned myDefaultBackground;

};

} // namespace Licq

#endif
