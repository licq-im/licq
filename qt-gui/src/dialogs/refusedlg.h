/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2010 Licq developers
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

#ifndef REFUSEDLG_H
#define REFUSEDLG_H

#include <QDialog>

namespace Licq
{
class UserId;
}

namespace LicqQtGui
{
class MLEdit;

class RefuseDlg : public QDialog
{
  Q_OBJECT

public:
  /**
   * Constructor, create and show refusal dialog
   *
   * @param userId User to send refusal message to
   * @param t Type of event to refuse
   * @param parent Parent widget
   */
  RefuseDlg(const Licq::UserId& userId, const QString& t, QWidget* parent = NULL);
  QString RefuseMessage();

private:
  MLEdit* mleRefuseMsg;
};

} // namespace LicqQtGui

#endif
