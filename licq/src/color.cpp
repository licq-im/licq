// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1998-2010 Licq developers
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

#include "config.h"

#include <licq/color.h>

using Licq::Color;

unsigned Color::myDefaultForeground = 0x000000;
unsigned Color::myDefaultBackground = 0xffffff;

Color::Color()
  : myForeground(0x000000),
    myBackground(0xffffff)
{
  // Empty
}

void Color::setToDefault()
{
  myForeground = myDefaultForeground;
  myBackground = myDefaultBackground;
}

void Color::set(unsigned fore, unsigned back)
{
  myForeground = fore;
  myBackground = back;
}

void Color::set(const Color* c)
{
  myForeground = c->myForeground;
  myBackground = c->myBackground;
}

void Color::setBackground(unsigned r, unsigned g, unsigned b)
{
  myBackground = (r & 0xFF) | ((g & 0xFF) << 8) | ((b & 0xFF) << 16);
}

void Color::setForeground(unsigned r, unsigned g, unsigned b)
{
  myForeground = (r & 0xFF) | ((g & 0xFF) << 8) | ((b & 0xFF) << 16);
}

void Color::setDefaultColors(const Color* c)
{
  myDefaultForeground = c->foreground();
  myDefaultBackground = c->background();
}


void Color::setDefaultForeground(unsigned c)
{
  myDefaultForeground = c;
}


void Color::setDefaultBackground(unsigned c)
{
  myDefaultBackground = c;
}


