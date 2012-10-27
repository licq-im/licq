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

#ifndef LICQGUI_OWNERCOMBOBOX_H
#define LICQGUI_OWNERCOMBOBOX_H

#include <QComboBox>

#include <licq/userid.h>

namespace LicqQtGui
{

/**
 * Combo box to select an owner
 */
class OwnerComboBox : public QComboBox
{
  Q_OBJECT

public:
  /**
   * Constructor
   *
   * @param filter Which owner to include
   * @param extra Text of additional selection (will return invalid userid)
   * @param parent Parent widget
   */
  OwnerComboBox(const QString& extra = QString(), QWidget* parent = 0);

  /// Get userid of selected owner
  Licq::UserId currentOwnerId() const;

  /// Set selection
  bool setCurrentOwnerId(const Licq::UserId& ownerId);
};

} // namespace LicqQtGui

#endif
