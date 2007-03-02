// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2006 Licq developers
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qwidget.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "skin.h"
#include "licq_constants.h"

CSkin::CSkin(const char *skinname)
{
   char temp[MAX_FILENAME_LEN];
   char baseSkinDir[MAX_FILENAME_LEN];
   szSkinName = strdup(skinname);
   m_nMenuBarHeight = 0;

   if (skinname[0] == '/')
   {
     strcpy(baseSkinDir, skinname);
     if (baseSkinDir[strlen(baseSkinDir) - 1] != '/') strcat(baseSkinDir, "/");
   }
   else
   {
     snprintf(baseSkinDir, MAX_FILENAME_LEN, "%s/%sskin.%s/", BASE_DIR, QTGUI_DIR, skinname);
     baseSkinDir[MAX_FILENAME_LEN - 1] = '\0';
   }

   char filename[MAX_FILENAME_LEN];
   snprintf(filename, MAX_FILENAME_LEN, "%s%s.skin", baseSkinDir, skinname);
   filename[MAX_FILENAME_LEN - 1] = '\0';
   CIniFile skinFile/*(INI_FxFATAL | INI_FxERROR | INI_FxWARN)*/;
   if (!skinFile.LoadFile(filename))
   {
     snprintf(baseSkinDir, MAX_FILENAME_LEN, "%s%sskin.%s/", SHARE_DIR, QTGUI_DIR, skinname);
     baseSkinDir[MAX_FILENAME_LEN - 1] = '\0';
     snprintf(filename, MAX_FILENAME_LEN, "%s%s.skin", baseSkinDir, skinname);
     filename[MAX_FILENAME_LEN - 1] = '\0';
     skinFile.SetFlag(INI_FxWARN);
     if (!skinFile.LoadFile(filename))
     {
       SetDefaultValues();
       return;
     }
   }

   skinFile.SetFlag(INI_FxWARN);

   // Frame
   skinFile.SetSection("skin");
   skinFile.SetFlags(0);
   skinFile.ReadStr("frame.pixmap", temp, "none");
   if (strncmp(temp, "none", 4) == 0 || strlen(temp) == 0)
      frame.pixmap = NULL;
   else
   {
      frame.pixmap = new char[strlen(baseSkinDir) + strlen(temp) + 1];
      sprintf(frame.pixmap, "%s%s", baseSkinDir, temp);
   }
   skinFile.ReadStr("frame.mask", temp, "none");
   if (strncmp(temp, "none", 4) == 0 || strlen(temp) == 0)
      frame.mask = NULL;
   else
   {
      frame.mask = new char[strlen(baseSkinDir) + strlen(temp) + 1];
      sprintf(frame.mask, "%s%s", baseSkinDir, temp);
   }
   skinFile.SetFlags(INI_FxFATAL | INI_FxERROR);
   skinFile.ReadNum("frame.border.top", frame.border.top);
   skinFile.ReadNum("frame.border.bottom", frame.border.bottom);
   skinFile.ReadNum("frame.border.left", frame.border.left);
   skinFile.ReadNum("frame.border.right", frame.border.right);
   skinFile.SetFlags(0);
   skinFile.ReadBool("frame.hasMenuBar", frame.hasMenuBar, false);
   skinFile.ReadNum("frame.frameStyle", frame.frameStyle, 51);
   skinFile.ReadBool("frame.transparent", frame.transparent, false);

   // System Button
   skinFile.SetFlags(0);
   skinFile.ReadStr("btnSys.caption", temp, "default");
   if (strncmp(temp, "default", 7) == 0)
      btnSys.caption = NULL;
   else
      btnSys.caption = strdup(temp);

   skinFile.ReadStr("btnSys.pixmapUpFocus", temp, "none");
   if (strncmp(temp, "none", 4) == 0)
      btnSys.pixmapUpFocus = NULL;
   else
   {
      btnSys.pixmapUpFocus = new char[strlen(baseSkinDir) + strlen(temp) + 1];
      sprintf(btnSys.pixmapUpFocus, "%s%s", baseSkinDir, temp);
   }

   skinFile.ReadStr("btnSys.pixmapUpNoFocus", temp, "none");
   if (strncmp(temp, "none", 4) == 0)
      btnSys.pixmapUpNoFocus = NULL;
   else
   {
      btnSys.pixmapUpNoFocus = new char[strlen(baseSkinDir) + strlen(temp) + 1];
      sprintf(btnSys.pixmapUpNoFocus, "%s%s", baseSkinDir, temp);
   }

   skinFile.ReadStr("btnSys.pixmapDown", temp, "none");
   if (strncmp(temp, "none", 4) == 0)
      btnSys.pixmapDown = NULL;
   else
   {
      btnSys.pixmapDown = new char[strlen(baseSkinDir) + strlen(temp) + 1];
      sprintf(btnSys.pixmapDown, "%s%s", baseSkinDir, temp);
   }

   skinFile.ReadStr("btnSys.color.fg", temp, "default");
   if (strncmp(temp, "default", 7) == 0)
      btnSys.color.fg = NULL;
   else
      btnSys.color.fg = strdup(temp);
   skinFile.ReadStr("btnSys.color.bg", temp);
   if (strncmp(temp, "default", 7) == 0)
      btnSys.color.bg = NULL;
   else
      btnSys.color.bg = strdup(temp);

   skinFile.SetFlags(INI_FxFATAL | INI_FxERROR);
   skinFile.ReadNum("btnSys.rect.x1", btnSys.rect.x1);
   skinFile.ReadNum("btnSys.rect.y1", btnSys.rect.y1);
   skinFile.ReadNum("btnSys.rect.x2", btnSys.rect.x2);
   skinFile.ReadNum("btnSys.rect.y2", btnSys.rect.y2);

   // Status Label
   skinFile.ReadNum("lblStatus.rect.x1", lblStatus.rect.x1);
   skinFile.ReadNum("lblStatus.rect.y1", lblStatus.rect.y1);
   skinFile.ReadNum("lblStatus.rect.x2", lblStatus.rect.x2);
   skinFile.ReadNum("lblStatus.rect.y2", lblStatus.rect.y2);
   skinFile.SetFlags(0);
   skinFile.ReadStr("lblStatus.pixmap", temp, "none");
   if (strncmp(temp, "none", 4) == 0)
      lblStatus.pixmap = NULL;
   else
   {
      lblStatus.pixmap = new char[strlen(baseSkinDir) + strlen(temp) + 1];
      sprintf(lblStatus.pixmap, "%s%s", baseSkinDir, temp);
   }
   skinFile.ReadStr("lblStatus.color.fg", temp, "default");
   if (strncmp(temp, "default", 7) == 0)
      lblStatus.color.fg = NULL;
   else
      lblStatus.color.fg = strdup(temp);
   skinFile.ReadStr("lblStatus.color.bg", temp, "default");
   lblStatus.transparent = false;
   if (strncmp(temp, "default", 7) == 0)
      lblStatus.color.bg = NULL;
   else if (strncmp(temp, "transparent", 11) == 0)
   {
      lblStatus.color.bg = NULL;
      lblStatus.transparent = true;
   }
   else
      lblStatus.color.bg = strdup(temp);
   skinFile.ReadNum("lblStatus.margin", lblStatus.margin, 4);
   skinFile.SetFlags(INI_FxFATAL | INI_FxERROR);
   skinFile.ReadNum("lblStatus.frameStyle", lblStatus.frameStyle);


   // Message Label
   skinFile.ReadNum("lblMsg.rect.x1", lblMsg.rect.x1);
   skinFile.ReadNum("lblMsg.rect.y1", lblMsg.rect.y1);
   skinFile.ReadNum("lblMsg.rect.x2", lblMsg.rect.x2);
   skinFile.ReadNum("lblMsg.rect.y2", lblMsg.rect.y2);
   skinFile.SetFlags(0);
   skinFile.ReadStr("lblMsg.pixmap", temp, "none");
   if (strncmp(temp, "none", 4) == 0)
      lblMsg.pixmap = NULL;
   else
   {
      lblMsg.pixmap = new char[strlen(baseSkinDir) + strlen(temp) + 1];
      sprintf(lblMsg.pixmap, "%s%s", baseSkinDir, temp);
   }
   skinFile.ReadStr("lblMsg.color.fg", temp, "default");
   if (strncmp(temp, "default", 7) == 0)
      lblMsg.color.fg = NULL;
   else
      lblMsg.color.fg = strdup(temp);
   skinFile.ReadStr("lblMsg.color.bg", temp, "default");
   lblMsg.transparent = false;
   if (strncmp(temp, "default", 7) == 0)
      lblMsg.color.bg = NULL;
   else if (strncmp(temp, "transparent", 11) == 0)
   {
      lblMsg.color.bg = NULL;
      lblMsg.transparent = true;
   }
   else
      lblMsg.color.bg = strdup(temp);
   skinFile.ReadNum("lblMsg.margin", lblMsg.margin, 4);
   skinFile.SetFlags(INI_FxFATAL | INI_FxERROR);
   skinFile.ReadNum("lblMsg.frameStyle", lblMsg.frameStyle);

   // Group Combo Box
   skinFile.ReadNum("cmbGroups.rect.x1", cmbGroups.rect.x1);
   skinFile.ReadNum("cmbGroups.rect.y1", cmbGroups.rect.y1);
   skinFile.ReadNum("cmbGroups.rect.x2", cmbGroups.rect.x2);
   skinFile.ReadNum("cmbGroups.rect.y2", cmbGroups.rect.y2);
   skinFile.SetFlags(0);
   skinFile.ReadStr("cmbGroups.color.fg", temp, "default");
   if (strncmp(temp, "default", 7) == 0)
      cmbGroups.color.fg = NULL;
   else
      cmbGroups.color.fg = strdup(temp);
   skinFile.ReadStr("cmbGroups.color.bg", temp, "default");
   if (strncmp(temp, "default", 7) == 0 || strlen(temp) == 0)
      cmbGroups.color.bg = NULL;
   else
      cmbGroups.color.bg = strdup(temp);

   // Read in the colors
   skinFile.ReadStr("colors.online", temp, "default");
   if (strncmp(temp, "default", 7) == 0)
     colors.online = strdup("blue");//"#0402FC");
   else
     colors.online = strdup(temp);
   skinFile.ReadStr("colors.away", temp, "default");
   if (strncmp(temp, "default", 7) == 0)
     colors.away = strdup("dark green");
   else
     colors.away = strdup(temp);
   skinFile.ReadStr("colors.offline", temp, "default");
   if (strncmp(temp, "default", 7) == 0)
     colors.offline = strdup("firebrick");//"#FC0204");
   else
     colors.offline = strdup(temp);
   skinFile.ReadStr("colors.background", temp, "default");
   if (strncmp(temp, "default", 7) == 0)
    colors.background = strdup("grey76");
   else
     colors.background = strdup(temp);
   skinFile.ReadStr("colors.gridlines", temp, "default");
   if (strncmp(temp, "default", 7) == 0)
     colors.gridlines = strdup("black");
   else
     colors.gridlines = strdup(temp);
   skinFile.ReadStr("colors.newuser", temp, "default");
   if (strncmp(temp, "default", 7) == 0)
     colors.newuser = strdup("yellow");
   else
     colors.newuser = strdup(temp);
   skinFile.ReadStr("colors.scrollbar", temp, "default");
   if (strncmp(temp, "default", 7) == 0)
     colors.scrollbar = NULL;
   else
     colors.scrollbar = strdup(temp);
   skinFile.ReadStr("colors.btnTxt", temp, "default");
   if (strncmp(temp, "default", 7) == 0)
     colors.btnTxt = NULL;
   else
     colors.btnTxt = strdup(temp);

  skinFile.ReadStr("colors.groupBack", temp, "default");
  if (strncmp(temp, "default", 7) == 0)
    colors.groupBack = strdup(colors.background);
  else
    colors.groupBack = strdup(temp);
}


CSkin::~CSkin(void)
{
  free(szSkinName);
  if (frame.pixmap != NULL) delete [] frame.pixmap;
  if (frame.mask != NULL) delete [] frame.mask;
  if (btnSys.caption != NULL) free (btnSys.caption);
  if (btnSys.pixmapUpNoFocus != NULL) delete [] btnSys.pixmapUpNoFocus;
  if (btnSys.pixmapUpFocus != NULL) delete [] btnSys.pixmapUpFocus;
  if (btnSys.pixmapDown != NULL) delete [] btnSys.pixmapDown;
  if (btnSys.color.fg != NULL) free (btnSys.color.fg);
  if (btnSys.color.bg != NULL) free (btnSys.color.bg);
  if (lblStatus.pixmap != NULL) delete [] lblStatus.pixmap;
  if (lblStatus.color.fg != NULL) free (lblStatus.color.fg);
  if (lblStatus.color.bg != NULL) free (lblStatus.color.bg);
  if (lblMsg.pixmap != NULL) delete [] lblMsg.pixmap;
  if (lblMsg.color.fg != NULL) free (lblMsg.color.fg);
  if (lblMsg.color.bg != NULL) free (lblMsg.color.bg);
  if (cmbGroups.color.fg != NULL) free (cmbGroups.color.fg);
  if (cmbGroups.color.bg != NULL) free (cmbGroups.color.bg);
  free (colors.online);
  free (colors.offline);
  free (colors.away);
  free (colors.background);
  free (colors.gridlines);
  free (colors.newuser);
  free (colors.scrollbar);
  free (colors.btnTxt);
  free(colors.groupBack);
}


void CSkin::SetDefaultValues()
{
  frame.pixmap = NULL;
  frame.mask = NULL;
  frame.border.top = 0;
  frame.border.bottom = 50;
  frame.border.left = 0;
  frame.border.right = 0;
  frame.hasMenuBar = 1;
  frame.frameStyle = 33;
  frame.transparent = false;

  lblStatus.rect.x1 = 5;
  lblStatus.rect.y1 = -25;
  lblStatus.rect.x2 = -5;
  lblStatus.rect.y2 = -5;
  lblStatus.color.fg = NULL;
  lblStatus.color.bg = NULL;
  lblStatus.frameStyle = 51;
  lblStatus.pixmap = NULL;

  btnSys.rect.x1 = 20;
  btnSys.rect.y1 = -65;
  btnSys.rect.x2 = 70;
  btnSys.rect.y2 = -45;
  btnSys.pixmapUpFocus = NULL;
  btnSys.pixmapUpNoFocus = NULL;
  btnSys.pixmapDown = NULL;
  btnSys.color.fg = NULL;
  btnSys.color.bg = NULL;
  btnSys.caption = NULL;

  lblMsg.rect.x1 = 5;
  lblMsg.rect.y1 = -50;
  lblMsg.rect.x2 = -5;
  lblMsg.rect.y2 = -30;
  lblMsg.color.fg = NULL;
  lblMsg.color.bg = NULL;
  lblMsg.frameStyle = 51;
  lblMsg.pixmap = NULL;

  cmbGroups.rect.x1 = 5;
  cmbGroups.rect.y1 = -75;
  cmbGroups.rect.x2 = -5;
  cmbGroups.rect.y2 = -55;
  cmbGroups.color.fg = NULL;
  cmbGroups.color.bg = NULL;

  colors.online = strdup("blue");
  colors.away = strdup("dark green");
  colors.offline = strdup("firebrick");
  colors.newuser = strdup("yellow");
  colors.background = strdup("grey76");
  colors.gridlines = strdup("black");
  colors.scrollbar = NULL;
  colors.btnTxt = NULL;
  colors.groupBack = strdup(colors.background);
}


void CSkin::AdjustForMenuBar(unsigned short h)
{
  frame.border.AdjustForMenuBar(m_nMenuBarHeight, h);
  lblStatus.rect.AdjustForMenuBar(m_nMenuBarHeight, h);
  btnSys.rect.AdjustForMenuBar(m_nMenuBarHeight, h);
  lblMsg.rect.AdjustForMenuBar(m_nMenuBarHeight, h);
  cmbGroups.rect.AdjustForMenuBar(m_nMenuBarHeight, h);

  m_nMenuBarHeight = h;
}

void Rect::AdjustForMenuBar(unsigned short h_old, unsigned short h_new)
{
  if (y1 >= 0)
    y1 += (h_new - h_old);
  if (y2 >= 0)
    y2 += (h_new - h_old);
}

void Border::AdjustForMenuBar(unsigned short h_old, unsigned short h_new)
{
  top += (h_new - h_old);
}


int CSkin::frameWidth(void)
{
   return (frame.border.right + frame.border.left);
}

int CSkin::frameHeight(void)
{
   return (frame.border.top + frame.border.bottom);
}

QRect CSkin::borderToRect(CShapeSkin *s, QWidget *w)
{
   struct Rect *r = &s->rect;
   QRect rect;

   // X1
   if (r->x1 >= 0)
      rect.setX(r->x1);
   else
      rect.setX(w->width() + r->x1);

   // Y1
   if (r->y1 >= 0)
      rect.setY(r->y1);
   else
      rect.setY(w->height() + r->y1);

   // X2
   if (r->x2 > 0)
      rect.setWidth(r->x2 - rect.x() + 1);
   else
      rect.setWidth((w->width() + r->x2) - rect.x() + 1);

   // Y2
   if (r->y2 > 0)
      rect.setHeight(r->y2 - rect.y() + 1);
   else
      rect.setHeight((w->height() + r->y2) - rect.y() + 1);

   return (rect);
}

/*! \brief Returns a palette with colored scrollbars
 *
 *  This method creates a palette with skin specific scrollbar colors.
 *  Parent should be a widget that holds a "default" active palette, which will
 *  be used as base for the resulting palette.
 *  The returned QPalette is a copy of the parent palette but with modified
 *  scrollbar colors: QColorGroup::Highlight, QColorGroup::Button,
 *  QColorGroup::Foreground, QColorGroup::Background and QColorGroup::ButtonText.
 */
QPalette CSkin::palette(QWidget *parent)
{
  QPalette pal;
  QColorGroup cg;
  cg = parent->QWidget::palette().active(); // copy active palette from parent
  // ButtonText +  arrow of scrollbar
  if (colors.btnTxt)
  {
    cg.setColor(QColorGroup::ButtonText, QColor(colors.btnTxt));
    cg.setColor(QColorGroup::Foreground, cg.buttonText());
  }
  // Scrollbar
  if (colors.scrollbar)
  {
    cg.setColor(QColorGroup::Highlight, QColor(colors.scrollbar));
    cg.setColor(QColorGroup::Button, cg.highlight());
    cg.setColor(QColorGroup::Background, cg.highlight());
  }
  pal.setActive(cg);
  pal.setInactive(cg);
  pal.setDisabled(cg);
  return pal;
}
