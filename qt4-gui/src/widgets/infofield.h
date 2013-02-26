/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2009, 2013 Licq developers <licq-dev@googlegroups.com>
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

#ifndef INFOFIELD_H
#define INFOFIELD_H

#include <QColor>
#include <QLineEdit>


namespace LicqQtGui
{

/**
 * LineEdit widget with a few convenience functions added
 */
class InfoField : public QLineEdit
{
  Q_OBJECT

public:
  /**
   * Constructor
   *
   * @param ro Initial state of read only property
   * @param parent Parent widget
   */
  InfoField(bool ro = false, QWidget* parent = NULL);

  /**
   * Set read only property
   *
   * @param ro False to make widget editable
   */
  void setReadOnly(bool ro);

  /**
   * Set field text from char string
   *
   * @param data New value for field
   */
  void setText(const char* data);

  /**
   * Set field text from numeral
   *
   * @param data New value for field
   */
  void setText(unsigned long data);

  /**
   * Set text
   *
   * @param text New value for field
   */
  void setText(QString text);

  /**
   * Set field to a timestamp
   *
   * @param timestamp Timestamp in UTC to set
   */
  void setDateTime(uint timestamp);

protected:
  /**
   * A key was pressed, overloaded to get keyboard shortcuts
   *
   * @param event Key event
   */
  virtual void keyPressEvent(QKeyEvent* event);

private:
  QColor myBaseRo;
  QColor myBaseRw;
};

} // namespace LicqQtGui

#endif
