/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2010 Licq developers
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

#include "config.h"

#include <QColor>
#include <QObject>
#include <QPalette>
#include <QPixmap>
#include <QRect>

namespace Licq
{
class IniFile;
}

namespace LicqQtGui
{
namespace Config
{
class Border
{
public:
  int top, bottom;
  int left, right;

  void AdjustForMenuBar(int h1, int h2);
};

class FrameSkin
{
public:
  struct Border border;
  unsigned frameStyle;
  bool maintainBorder;
  bool hasMenuBar;
  bool transparent;
  QPixmap pixmap;
  QPixmap mask;

  virtual ~FrameSkin() {}
  virtual void loadSkin(const Licq::IniFile& skinFile, const QString& name, const QString& baseSkinDir);
};

class ShapeSkin
{
public:
  QRect rect;
  QColor foreground;
  QColor background;

  virtual ~ShapeSkin() { }
  virtual void loadSkin(const Licq::IniFile& skinFile, const QString& name);
  QRect borderToRect(const QWidget* w) const;
  void AdjustForMenuBar(int h1, int h2);
};

class ButtonSkin : public ShapeSkin
{
public:
  QPixmap pixmapUpFocus;
  QPixmap pixmapUpNoFocus;
  QPixmap pixmapDown;
  QString caption;

  virtual ~ButtonSkin() { }
  virtual void loadSkin(const Licq::IniFile& skinFile, const QString& name, const QString& baseSkinDir);

private:
  using ShapeSkin::loadSkin;
};

class LabelSkin : public ShapeSkin
{
public:
  QPixmap pixmap;
  unsigned frameStyle;
  bool transparent;
  int margin;

  virtual ~LabelSkin() { }
  virtual void loadSkin(const Licq::IniFile& skinFile, const QString& name, const QString& baseSkinDir);

private:
  using ShapeSkin::loadSkin;
};

class ComboSkin : public ShapeSkin { };

class ListSkin : public ShapeSkin { };

/**
 * Data for a gui skin
 * A singleton instance is used for the skin currently used for the gui.
 * Other instances may also be created to hold data for other skins, used by
 * SkinBrowser dialog.
 */
class Skin : public QObject
{
  Q_OBJECT

public:
  /**
   * Create the active skin instance
   *
   * @param skinName Initial skin name to load
   * @param parent Parent object
   */
  static void createInstance(const QString& skinName = QString(), QObject* parent = NULL);

  /**
   * Get the active skin
   *
   * @return The skin data singleton
   */
  static Skin* active()
  { return myInstance; }


  Skin(const QString& skinName = QString(), QObject* parent = NULL);
  virtual ~Skin() {}
  void loadSkin(const QString& skinName);

  const QString& skinName() const { return mySkinName; }
  QPixmap mainwinPixmap(int width, int height) const;
  QPixmap mainwinMask(int width, int height) const;

  void setFrameTransparent(bool transparent);
  void setFrameStyle(unsigned frameStyle);

  FrameSkin frame;
  ButtonSkin btnSys;
  LabelSkin lblStatus, lblMsg;
  ComboSkin cmbGroups;
  ListSkin lstUsers;

  QColor backgroundColor;
  QColor gridlineColor;
  QColor scrollbarColor;
  QColor buttonTextColor;

  QColor onlineColor;
  QColor offlineColor;
  QColor awayColor;
  QColor newUserColor;
  QColor awaitingAuthColor;

  QColor highBackColor;
  QColor highTextColor;

  QColor groupBackColor;
  QColor groupTextColor;
  QColor groupHighBackColor;
  QColor groupHighTextColor;

  QImage groupBackImage;

  bool tileGroupBackImage;

  // Functions
  void AdjustForMenuBar(int n);
  int frameWidth(void);
  int frameHeight(void);
  QPalette palette(QWidget* parent);

signals:
  void changed();
  void frameChanged();

private:
  // Singleton instance
  static Skin* myInstance;

  void SetDefaultValues();
  QPixmap scaleWithBorder(const QPixmap& pm, int width, int height) const;

  QString mySkinName;
  int myMenuBarHeight;
};

} // namespace Config
} // namespace LicqQtGui

#endif
