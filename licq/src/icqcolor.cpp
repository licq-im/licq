// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998 - 2003 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "licq_color.h"

// CICQColor statics declarations
unsigned long CICQColor::default_foreground = 0x00000000;
unsigned long CICQColor::default_background = 0x00FFFFFF;


CICQColor::CICQColor()
{
  background = 0x00FFFFFF;
  foreground = 0x00000000;
}


unsigned long CICQColor::ForeRed() const   { return foreground & 0xFF; }
unsigned long CICQColor::ForeGreen() const { return ( foreground & 0xFF00) >> 8; }
unsigned long CICQColor::ForeBlue() const  { return ( foreground & 0xFF0000) >> 16; }
unsigned long CICQColor::BackRed() const   { return background & 0xFF; }
unsigned long CICQColor::BackGreen() const { return ( background & 0xFF00) >> 8; }
unsigned long CICQColor::BackBlue() const  { return ( background & 0xFF0000) >> 16; }

unsigned long CICQColor::Foreground() const { return foreground; }
unsigned long CICQColor::Background() const { return background; }


void CICQColor::SetToDefault()
{
  foreground = default_foreground;
  background = default_background;
}


void CICQColor::Set(unsigned long fore, unsigned long back)
{
  foreground = fore;
  background = back;
}


void CICQColor::Set(CICQColor const* c)
{
  foreground = c->Foreground();
  background = c->Background();
}


void CICQColor::SetBackground(unsigned long r, unsigned long g, unsigned long b)
{
  background = (r & 0xFF) | ((g & 0xFF) << 8) | ((b & 0xFF) << 16);
}


void CICQColor::SetForeground(unsigned long r, unsigned long g, unsigned long b)
{
  foreground = (r & 0xFF) | ((g & 0xFF) << 8) | ((b & 0xFF) << 16);
}


unsigned long CICQColor::DefaultForeRed()
{
  return default_foreground & 0xFF;
}


unsigned long CICQColor::DefaultForeGreen()
{
  return (default_foreground & 0xFF00) >> 8;
}


unsigned long CICQColor::DefaultForeBlue()
{
  return ( default_foreground & 0xFF0000) >> 16;
}


unsigned long CICQColor::DefaultBackRed()
{
  return default_background & 0xFF;
}


unsigned long CICQColor::DefaultBackGreen()
{
  return (default_background & 0xFF00) >> 8;
}


unsigned long CICQColor::DefaultBackBlue()
{
  return ( default_background & 0xFF0000) >> 16;
}


unsigned long CICQColor::DefaultForeground()  { return default_foreground; }
unsigned long CICQColor::DefaultBackground()  { return default_background; }

void CICQColor::SetDefaultColors(CICQColor const* c)
{
  default_foreground = c->Foreground();
  default_background = c->Background();
}


void CICQColor::SetDefaultForeground(unsigned long c)
{
  default_foreground = c;
}


void CICQColor::SetDefaultBackground(unsigned long c)
{
  default_background = c;
}


