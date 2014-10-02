/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2012 Licq developers <licq-dev@googlegroups.com>
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

#ifndef LICQQTGUI_SPECIALSPINBOX_H
#define LICQQTGUI_SPECIALSPINBOX_H

#include <QSpinBox>

namespace LicqQtGui
{

/**
 * Convenience overload of QSpinBox to handle special value better
 */
class SpecialSpinBox : public QSpinBox
{
  Q_OBJECT

public:
  /**
   * Constructor
   *
   * @param minimum Lower end of range (used for special value)
   * @param maximum Upper end of range
   * @param specialValueText Special text to show for minimum value
   * @parent Widget parent
   */
  SpecialSpinBox(int minimum, int maximum, const QString& specialValueText, QWidget* parent = NULL);

private slots:
  /// Editing has finished
  void finished();
};

} // namespace LicqQtGui

#endif
