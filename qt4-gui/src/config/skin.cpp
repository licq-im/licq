// -*- c-basic-offset: 2 -*-
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

#include "skin.h"

#include "config.h"

#include "core/gui-defines.h"

#include <QPainter>
#include <QWidget>

#include <licq_constants.h>
#include <licq_file.h>
#include <licq_log.h>

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::Config::Skin */

Config::Skin* Config::Skin::myInstance = NULL;

void Config::Skin::createInstance(const QString& skinName, QObject* parent)
{
  myInstance = new Skin(skinName, parent);
}

Config::Skin::Skin(const QString& skinName, QObject* parent)
  : QObject(parent)
{
  loadSkin(skinName);
}

void Config::Skin::loadSkin(const QString& skinName)
{
  gLog.Info("%sApplying %s skin.\n", L_INITxSTR, skinName.toLocal8Bit().data());

  // Set default values even if skin is valid as skin may not include all settings
  SetDefaultValues();

  if (skinName.isEmpty())
  {
    emit changed();
    emit frameChanged();
    return;
  }

  CIniFile skinFile/*(INI_FxFATAL | INI_FxERROR | INI_FxWARN)*/;

  QString skinFileName = skinName + ".skin";
  QString subdir = QString(QTGUI_DIR) + SKINS_DIR + skinName + "/";
  QString baseSkinDir = QString::fromLocal8Bit(BASE_DIR) + subdir;
  if (!skinFile.LoadFile((baseSkinDir + skinFileName).toLocal8Bit()))
  {
    baseSkinDir = QString::fromLocal8Bit(SHARE_DIR) + subdir;
    if (!skinFile.LoadFile((baseSkinDir + skinFileName).toLocal8Bit()))
    {
      emit changed();
      emit frameChanged();
      return;
    }
  }

  mySkinName = skinName;

  skinFile.SetFlag(INI_FxWARN);
  skinFile.SetSection("skin");

  // Frame
  frame.loadSkin(skinFile, "frame", baseSkinDir);

  // System Button
  btnSys.loadSkin(skinFile, "btnSys", baseSkinDir);

  // Status Label
  lblStatus.loadSkin(skinFile, "lblStatus", baseSkinDir);

  // Message Label
  lblMsg.loadSkin(skinFile, "lblMsg", baseSkinDir);

  // Group Combo Box
  cmbGroups.loadSkin(skinFile, "cmbGroups");

  // Read in the colors
  char temp[MAX_FILENAME_LEN];
  skinFile.ReadStr("colors.background", temp, "default");
  if (strncmp(temp, "default", 7) != 0)
    backgroundColor.setNamedColor(temp);

  skinFile.ReadStr("colors.gridlines", temp, "default");
  if (strncmp(temp, "default", 7) != 0)
    gridlineColor.setNamedColor(temp);

  skinFile.ReadStr("colors.scrollbar", temp, "default");
  if (strncmp(temp, "default", 7) != 0)
    scrollbarColor.setNamedColor(temp);

  skinFile.ReadStr("colors.btnTxt", temp, "default");
  if (strncmp(temp, "default", 7) != 0)
    buttonTextColor.setNamedColor(temp);

  skinFile.ReadStr("colors.online", temp, "default");
  if (strncmp(temp, "default", 7) != 0)
    onlineColor.setNamedColor(temp);

  skinFile.ReadStr("colors.offline", temp, "default");
  if (strncmp(temp, "default", 7) != 0)
    offlineColor.setNamedColor(temp);

  skinFile.ReadStr("colors.away", temp, "default");
  if (strncmp(temp, "default", 7) != 0)
    awayColor.setNamedColor(temp);

  skinFile.ReadStr("colors.newuser", temp, "default");
  if (strncmp(temp, "default", 7) != 0)
    newUserColor.setNamedColor(temp);

  skinFile.ReadStr("colors.authwait", temp, "default");
  if (strncmp(temp, "default", 7) != 0)
    awaitingAuthColor.setNamedColor(temp);

  skinFile.ReadStr("colors.highlight.bg", temp, "default");
  if (strncmp(temp, "default", 7) != 0)
    highBackColor.setNamedColor(temp);

  skinFile.ReadStr("colors.highlight.fg", temp, "default");
  if (strncmp(temp, "default", 7) != 0)
    highTextColor.setNamedColor(temp);

  skinFile.ReadStr("colors.group.bg", temp, "default");
  if (strncmp(temp, "default", 7) != 0)
    groupBackColor.setNamedColor(temp);

  skinFile.ReadStr("colors.group.fg", temp, "default");
  if (strncmp(temp, "default", 7) != 0)
    groupTextColor.setNamedColor(temp);

  skinFile.ReadStr("colors.group.highlight.bg", temp, "default");
  if (strncmp(temp, "default", 7) != 0)
    groupHighBackColor.setNamedColor(temp);

  skinFile.ReadStr("colors.group.highlight.fg", temp, "default");
  if (strncmp(temp, "default", 7) != 0)
    groupHighTextColor.setNamedColor(temp);

  // And images
  skinFile.ReadStr("images.groupBack", temp, "none");
  if (strncmp(temp, "none", 4) != 0)
    groupBackImage.load(baseSkinDir + temp);

  skinFile.ReadBool("images.groupBack.tile", tileGroupBackImage, false);

  emit changed();
  emit frameChanged();
}

void Config::Skin::setFrameTransparent(bool transparent)
{
  if (transparent == frame.transparent)
    return;

  frame.transparent = transparent;
  emit frameChanged();
}

void Config::Skin::setFrameStyle(unsigned short frameStyle)
{
  if (frameStyle == frame.frameStyle)
    return;

  frame.frameStyle = frameStyle;
  emit frameChanged();
}

void Config::Skin::SetDefaultValues()
{
  mySkinName = "";
  myMenuBarHeight = 0;

  frame.pixmap = QPixmap();
  frame.mask = QPixmap();
  frame.border.top = 0;
  frame.border.bottom = 80;
  frame.border.left = 0;
  frame.border.right = 0;
  frame.hasMenuBar = true;
  frame.frameStyle = 33;
  frame.transparent = false;

  lblStatus.rect.setCoords(5, -25, -5, -5);
  lblStatus.foreground = QColor();
  lblStatus.background = QColor();
  lblStatus.frameStyle = 51;
  lblStatus.pixmap = QPixmap();
  lblStatus.margin = 5;

  btnSys.rect.setCoords(20, -65, 70, -45);
  btnSys.pixmapUpFocus = QPixmap();
  btnSys.pixmapUpNoFocus = QPixmap();
  btnSys.pixmapDown = QPixmap();
  btnSys.foreground = QColor();
  btnSys.background = QColor();
  btnSys.caption = QString();

  lblMsg.rect.setCoords(5, -50, -5, -30);
  lblMsg.foreground = QColor();
  lblMsg.background = QColor();
  lblMsg.frameStyle = 51;
  lblMsg.pixmap = QPixmap();
  lblMsg.margin = 5;

  cmbGroups.rect.setCoords(5, -75, -5, -55);
  cmbGroups.foreground = QColor();
  cmbGroups.background = QColor();

  backgroundColor = QColor();
  gridlineColor.setNamedColor("black");
  scrollbarColor = QColor();
  buttonTextColor = QColor();

  onlineColor.setNamedColor("blue");
  offlineColor.setNamedColor("firebrick");
  awayColor.setNamedColor("darkgreen");
  newUserColor.setNamedColor("yellow");
  awaitingAuthColor.setNamedColor("darkcyan");

  highBackColor = QColor();
  highTextColor = QColor();

  groupBackColor = QColor();
  groupTextColor = QColor();
  groupHighBackColor = QColor();
  groupHighTextColor = QColor();

  groupBackImage = QImage();
  tileGroupBackImage = false;
}


void Config::Skin::AdjustForMenuBar(unsigned short h)
{
  frame.border.AdjustForMenuBar(myMenuBarHeight, h);
  lblStatus.AdjustForMenuBar(myMenuBarHeight, h);
  btnSys.AdjustForMenuBar(myMenuBarHeight, h);
  lblMsg.AdjustForMenuBar(myMenuBarHeight, h);
  cmbGroups.AdjustForMenuBar(myMenuBarHeight, h);

  myMenuBarHeight = h;
}

void Config::FrameSkin::loadSkin(CIniFile& skinFile, const QString& name, const QString& baseSkinDir)
{
  char temp[255];
  skinFile.SetFlags(0);
  skinFile.ReadStr((name + ".pixmap").toLatin1().data(), temp, "none");
  if (strncmp(temp, "none", 4) != 0)
    if (!pixmap.load(baseSkinDir + temp))
      gLog.Error("%sError loading background pixmap (%s).\n", L_ERRORxSTR, temp);

  skinFile.ReadStr((name + ".mask").toLatin1().data(), temp, "none");
  if (strncmp(temp, "none", 4) != 0)
    if (!mask.load(baseSkinDir + temp))
      gLog.Error("%sError loading background mask (%s).\n", L_ERRORxSTR, temp);

  skinFile.SetFlags(INI_FxFATAL | INI_FxERROR);
  skinFile.ReadNum((name + ".border.top").toLatin1().data(), border.top);
  skinFile.ReadNum((name + ".border.bottom").toLatin1().data(), border.bottom);
  skinFile.ReadNum((name + ".border.left").toLatin1().data(), border.left);
  skinFile.ReadNum((name + ".border.right").toLatin1().data(), border.right);
  skinFile.SetFlags(0);
  skinFile.ReadBool((name + ".hasMenuBar").toLatin1().data(), hasMenuBar, hasMenuBar);
  skinFile.ReadNum((name + ".frameStyle").toLatin1().data(), frameStyle, frameStyle);
  skinFile.ReadBool((name + ".transparent").toLatin1().data(), transparent, transparent);
}

void Config::ShapeSkin::loadSkin(CIniFile& skinFile, const QString& name)
{
  signed short x1, y1, x2, y2;
  skinFile.SetFlags(INI_FxFATAL | INI_FxERROR);
  skinFile.ReadNum((name + ".rect.x1").toLatin1().data(), x1);
  skinFile.ReadNum((name + ".rect.y1").toLatin1().data(), y1);
  skinFile.ReadNum((name + ".rect.x2").toLatin1().data(), x2);
  skinFile.ReadNum((name + ".rect.y2").toLatin1().data(), y2);
  skinFile.SetFlags(0);
  rect.setCoords(x1, y1, x2, y2);

  char temp[255];
  skinFile.ReadStr((name + ".color.fg").toLatin1().data(), temp, "default");
  foreground = (strncmp(temp, "default", 7) == 0 ? QColor() : QColor(temp));
  if (strncmp(temp, "transparent", 11) == 0)
    foreground.setAlpha(0);
  skinFile.ReadStr((name + ".color.bg").toLatin1().data(), temp, "default");
  background = (strncmp(temp, "default", 7) == 0 ? QColor() : QColor(temp));
  if (strncmp(temp, "transparent", 11) == 0)
    background.setAlpha(0);
}

void Config::ButtonSkin::loadSkin(CIniFile& skinFile, const QString& name, const QString& baseSkinDir)
{
  Config::ShapeSkin::loadSkin(skinFile, name);

  char temp[255];
  skinFile.ReadStr((name + ".caption").toLatin1().data(), temp, "default");
  caption = (strncmp(temp, "default", 7) == 0 ? QString() : QString::fromLocal8Bit(temp));

  skinFile.ReadStr((name + ".pixmapUpFocus").toLatin1().data(), temp, "none");
  if (strncmp(temp, "none", 4) != 0)
    pixmapUpFocus.load(baseSkinDir + temp);

  skinFile.ReadStr((name + ".pixmapUpNoFocus").toLatin1().data(), temp, "none");
  if (strncmp(temp, "none", 4) != 0)
    pixmapUpNoFocus.load(baseSkinDir + temp);

  skinFile.ReadStr((name + ".pixmapDown").toLatin1().data(), temp, "none");
  if (strncmp(temp, "none", 4) != 0)
    pixmapDown.load(baseSkinDir + temp);
}

void Config::LabelSkin::loadSkin(CIniFile& skinFile, const QString& name, const QString& baseSkinDir)
{
  Config::ShapeSkin::loadSkin(skinFile, name);
  transparent = (background.alpha() == 0);

  char temp[255];
  skinFile.ReadStr((name + ".pixmap").toLatin1().data(), temp, "none");
  if (strncmp(temp, "none", 4) != 0)
    pixmap.load(baseSkinDir + temp);

  skinFile.ReadNum((name + ".margin").toLatin1().data(), margin, margin);
  skinFile.SetFlags(INI_FxFATAL | INI_FxERROR);
  skinFile.ReadNum((name + ".frameStyle").toLatin1().data(), frameStyle, frameStyle);
  skinFile.SetFlags(0);
}

void Config::ShapeSkin::AdjustForMenuBar(unsigned short h_old, unsigned short h_new)
{
  if (rect.top() >= 0)
    rect.setTop(rect.top() + h_new - h_old);
  if (rect.bottom() >= 0)
    rect.setBottom(rect.bottom() + h_new - h_old);
}

void Config::Border::AdjustForMenuBar(unsigned short h_old, unsigned short h_new)
{
  top += (h_new - h_old);
}


int Config::Skin::frameWidth()
{
   return (frame.border.right + frame.border.left);
}

int Config::Skin::frameHeight()
{
   return (frame.border.top + frame.border.bottom);
}

QRect Config::ShapeSkin::borderToRect(const QWidget* w) const
{
   QRect ret;

   // X1
   if (rect.left() >= 0)
      ret.setX(rect.left());
   else
      ret.setX(w->width() + rect.left());

   // Y1
   if (rect.top() >= 0)
      ret.setY(rect.top());
   else
      ret.setY(w->height() + rect.top());

   // X2
   if (rect.right() >= 0)
      ret.setWidth(rect.right() - ret.x());
   else
      ret.setWidth((w->width() + rect.right()) - ret.x());

   // Y2
   if (rect.bottom() >= 0)
      ret.setHeight(rect.bottom() - ret.y());
   else
      ret.setHeight((w->height() + rect.bottom()) - ret.y());

   return ret;
}

QPixmap Config::Skin::scaleWithBorder(const QPixmap& pm, int width, int height) const
{
  if (pm.isNull())
    return QPixmap();

  QPainter p;
  const Border& border(frame.border);

  // top left corner
  QPixmap pmTL(border.left, border.top);
  p.begin(&pmTL);
  p.drawPixmap(0, 0, pm, 0, 0, pmTL.width(), pmTL.height());
  p.end();

  // top border
  QPixmap pmT(pm.width() - border.left - border.right, border.top);
  p.begin(&pmT);
  p.drawPixmap(0, 0, pm, border.left, 0, pmT.width(), pmT.height());
  p.end();
  QImage imT( (pmT.toImage()).scaled(width - border.left - border.right, pmT.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation) );

  // top right corner
  QPixmap pmTR(border.right, border.top);
  p.begin(&pmTR);
  p.drawPixmap(0, 0, pm, pm.width() - border.right, 0, pmTR.width(), pmTR.height());
  p.end();

  // left border
  QPixmap pmL(border.left, pm.height() - border.top - border.bottom);
  p.begin(&pmL);
  p.drawPixmap(0, 0, pm, 0, border.top, pmL.width(), pmL.height());
  p.end();
  QImage imL( (pmL.toImage()).scaled(pmL.width(), height - border.top - border.bottom, Qt::IgnoreAspectRatio, Qt::SmoothTransformation) );

  // center
  QPixmap pmC(pmT.width(), pmL.height());
  p.begin(&pmC);
  p.drawPixmap(0, 0, pm, border.left, border.top, pmC.width(), pmC.height());
  p.end();
  QImage imC( (pmC.toImage()).scaled(imT.width(), imL.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation) );

  // right border
  QPixmap pmR(border.right, pm.height() - border.top - border.bottom);
  p.begin(&pmR);
  p.drawPixmap(0, 0, pm, pm.width() - border.right, border.top, pmR.width(), pmR.height());
  p.end();
  QImage imR ( (pmR.toImage()).scaled(pmR.width(), height - border.top - border.bottom, Qt::IgnoreAspectRatio, Qt::SmoothTransformation) );

  // bottom left border
  QPixmap pmBL(border.left, border.bottom);
  p.begin(&pmBL);
  p.drawPixmap(0, 0, pm, 0, pm.height() - border.bottom, pmBL.width(), pmBL.height());
  p.end();

  // bottom border
  QPixmap pmB(pm.width() - border.left - border.right, border.bottom);
  p.begin(&pmB);
  p.drawPixmap(0, 0, pm, border.left, pm.height() - border.bottom, pmB.width(), pmB.height());
  p.end();
  QImage imB( (pmB.toImage()).scaled(width - border.left - border.right, pmB.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation) );

  // bottom right border
  QPixmap pmBR(border.right, border.bottom);
  p.begin(&pmBR);
  p.drawPixmap(0, 0, pm, pm.width() - border.right, pm.height() - border.bottom, pmBR.width(), pmBR.height());
  p.end();

  // put the image together
  QPixmap pmFinal(width, height);
  p.begin(&pmFinal);
  p.drawPixmap(0, 0, pmTL, 0, 0, -1, -1);
  p.drawImage(border.left, 0, imT, 0, 0, -1, -1);
  p.drawPixmap(pmFinal.width() - border.right, 0, pmTR, 0, 0, -1, -1);
  p.drawImage(0, border.top, imL, 0, 0, -1, -1);
  p.drawImage(pmFinal.width() - border.right, border.top, imR, 0, 0, -1, -1);
  p.drawPixmap(0, pmFinal.height() - border.bottom, pmBL, 0, 0, -1, -1);
  p.drawImage(border.left, pmFinal.height() - border.bottom, imB, 0, 0, -1, -1);
  p.drawPixmap(pmFinal.width() - border.right, pmFinal.height() - border.bottom, pmBR, 0, 0, -1, -1);
  p.drawImage(border.left, border.top, imC, 0, 0, -1, -1);
  p.end();

  return pmFinal;
}

QPixmap Config::Skin::mainwinPixmap(int width, int height) const
{
  return scaleWithBorder(frame.pixmap, width, height);
}

QPixmap Config::Skin::mainwinMask(int width, int height) const
{
  return scaleWithBorder(frame.mask, width, height);
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
QPalette Config::Skin::palette(QWidget* parent)
{
  QPalette pal = parent->palette();
  // ButtonText +  arrow of scrollbar
  if (buttonTextColor.isValid())
  {
    pal.setColor(QPalette::ButtonText, buttonTextColor);
    pal.setColor(QPalette::Foreground, buttonTextColor);
  }
  // Scrollbar
  if (scrollbarColor.isValid())
  {
    pal.setColor(QPalette::Highlight, scrollbarColor);
    pal.setColor(QPalette::Button, scrollbarColor);
    pal.setColor(QPalette::Background, scrollbarColor);
  }
  return pal;
}
