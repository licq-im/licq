/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2009 Licq developers
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

// Skin Spec 0.1

#ifndef SKIN_H
#define SKIN_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qrect.h>
#include <qpixmap.h>

#include "gui-defines.h"
#include "licq_file.h"


class Border
{
public:
  unsigned short top, bottom;
  unsigned short left, right;

  void AdjustForMenuBar(unsigned short h1, unsigned short h2);
};

class CFrameSkin
{
public:
   struct Border border;
   unsigned short frameStyle;
   bool maintainBorder;
   bool hasMenuBar;
   bool transparent;
   char *pixmap;
   char *mask;
};

class Rect
{
public:
   signed short x1, y1;
   signed short x2, y2;

   void AdjustForMenuBar(unsigned short h1, unsigned short h2);
};

struct Color
{
   char *fg, *bg;
};

struct CColors
{
  char *online, *offline, *away, *background, *gridlines, *newuser, *scrollbar, *btnTxt, *groupBack;
};

class CShapeSkin
{
public:
   struct Rect rect;
   struct Color color;
};

class CButtonSkin : public CShapeSkin
{
public:
   char *pixmapUpFocus, *pixmapUpNoFocus, *pixmapDown;
   char *caption;
};

class CLabelSkin : public CShapeSkin
{
public:
   char *pixmap;
   unsigned short frameStyle;
   bool transparent;
   unsigned short margin;
};

class CComboSkin : public CShapeSkin { };

class CListSkin : public CShapeSkin { };

class CSkin
{
public:
   CSkin(const char *);
   ~CSkin(void);
   CFrameSkin frame;
   CButtonSkin btnSys;
   CLabelSkin lblStatus, lblMsg;
   CComboSkin cmbGroups;
   CListSkin lstUsers;
   CColors colors;
   char *szSkinName;

   // Functions
   void AdjustForMenuBar(unsigned short n);
   QRect borderToRect(CShapeSkin *, QWidget *);
   int frameWidth(void);
   int frameHeight(void);
   QPalette palette(QWidget *parent);

protected:
   unsigned short m_nMenuBarHeight;
   void SetDefaultValues();
};


#endif
