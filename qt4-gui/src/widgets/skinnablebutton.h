/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2009 Licq developers
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

#ifndef SKINNABLEBUTTON_H
#define SKINNABLEBUTTON_H

#include <QPushButton>

namespace LicqQtGui
{

namespace Config
{
class ButtonSkin;
}

/**
 * Extended QPushButton which can be skinned by using images instead of text
 */
class SkinnableButton : public QPushButton
{
  Q_OBJECT

public:
  /**
   * Constructor, create a skinnable button and apply a skin
   *
   * @param skin Button skin to apply
   * @param defaultText Text to display if skin doesn't contain a caption
   * @param parent Parent widget
   */
  SkinnableButton(const Config::ButtonSkin& skin, const QString& defaultText, QWidget* parent = NULL);

  /**
   * Constructor, create a default skinnable button
   *
   * @param defaultText Text to display if skin doesn't contain a caption
   * @param parent Parent widget
   */
  SkinnableButton(const QString& defaultText, QWidget* parent = 0);

  /**
   * Apply a skin
   *
   * @param skin New skin to use
   */
  void applySkin(const Config::ButtonSkin& skin);

  /**
   * Get the keyboard modifiers active when the button was last pressed
   *
   * @return Mask of active keyboard modifiers
   */
  Qt::KeyboardModifiers modifiersWhenPressed();

private:
  /**
   * Mouse button pressed
   *
   * @param e Event object
   */
  virtual void mousePressEvent(QMouseEvent* e);

  /**
   * Cursor has entered button
   *
   * @param e Event object
   */
  virtual void enterEvent(QEvent* e);

  /**
   * Cursor has left button
   *
   * @param e Event object
   */
  virtual void leaveEvent(QEvent* e);

  /**
   * Draw button
   *
   * @param e Event object
   */
  virtual void paintEvent(QPaintEvent* e);


  QString myDefaultText;
  Qt::KeyboardModifiers myPressedModifiers;
  QPixmap myNormalImage, myHighlightedImage, myPressedImage;
};

} // namespace LicqQtGui

#endif
