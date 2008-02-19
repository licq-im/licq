/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007 Licq developers
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

#ifndef SKINNABLELABEL_H
#define SKINNABLELABEL_H

#include <QLabel>
#include <QList>

class QMenu;

namespace LicqQtGui
{

namespace Config
{
class LabelSkin;
}

/**
 * Extended QLabel which can be skinned, show images and have a popup menu
 */
class SkinnableLabel : public QLabel
{
  Q_OBJECT

public:
  /**
   * Constructor, create a skinnable label and apply a skin
   *
   * @param skin Label skin to apply
   * @param menu Popup menu to show when right clicking on label
   * @param parent Parent widget
   */
  SkinnableLabel(const Config::LabelSkin& skin, QMenu* menu = NULL, QWidget* parent = NULL);

  /**
   * Constructor, create a default skinnable label
   *
   * @param menu Popup menu to show when right clicking on label
   * @param parent Parent widget
   */
  SkinnableLabel(QMenu* menu = NULL, QWidget* parent = NULL);

  /**
   * Apply a skin
   *
   * @param skin New skin to use
   */
  void applySkin(const Config::LabelSkin& skin);

  /**
   * Convenience function to set text boldness
   *
   * @param enable True to make text bold, false for normal text
   */
  void setBold(bool enable);

  /**
   * Convenience function to set text italicness
   *
   * @param enable True to make text italic, false for normal text
   */
  void setItalic(bool enable);

  /**
   * Specify an image to display in front of the label text
   *
   * @param p Image
   */
  void setPrependPixmap(const QPixmap& p);

  /**
   * Remove image in front of text
   */
  void clearPrependPixmap();

  /**
   * Add an image to list of images to show
   *
   * @param p Image to add last
   */
  void addPixmap(const QPixmap& p);

  /**
   * Clear list of images to show
   */
  void clearPixmaps();

signals:
  /**
   * User double clicked or middle clicked on label
   */
  void doubleClicked();

private:
  /**
   * User double clicked on widget
   *
   * @param e Event object
   */
  virtual void mouseDoubleClickEvent(QMouseEvent* e);

  /**
   * User clicked on widget
   *
   * @param e Event object
   */
  virtual void mousePressEvent(QMouseEvent* e);

  /**
   * Draw label
   *
   * @param e Event object
   */
  virtual void paintEvent(QPaintEvent* e);

  QMenu* myPopupMenu;
  QPixmap myBackgroundImage;
  QPixmap myAddPix;
  QList<QPixmap> myPixmaps;
  int myAddIndent, myStartingIndent;
};

} // namespace LicqQtGui

#endif
