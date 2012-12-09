/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2004-2012 Licq developers <licq-dev@googlegroups.com>
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

#ifndef OWNERMANAGERDLG_H
#define OWNERMANAGERDLG_H

#include <QDialog>

class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;

namespace Licq
{
class UserId;
}

namespace LicqQtGui
{

/**
 * Dialog for managing accounts
 */
class OwnerManagerDlg : public QDialog
{
   Q_OBJECT

public:
  /**
   * Create and show owner manager dialog or raise it if already exists
   */
  static void showOwnerManagerDlg();

private slots:
  /**
   * Selection has changed
   */
  void listSelectionChanged();

  /**
   * Add an existing account
   *
   * @param protocolId Id of protocol to add owner for
   */
  void addOwner(unsigned long protocolId);

  /**
   * Register a new account
   *
   * @param protocolId Id of protocol to register owner with
   */
  void registerOwner(unsigned long protocolId);

  /**
   * Edit currently selected account
   */
  void modify();

  /**
   * An item was double clicked
   *
   * @param item Protocol or account item from list
   * @param column List column, not used
   */
  void itemDoubleClicked(QTreeWidgetItem* item, int column = 0);

  /**
   * Remove currently select account or plugin
   */
  void remove();

  /**
   * Refresh list of protocols and accounts
   */
  void updateList();

  /**
   * A protocol has been loaded
   *
   * @param protocolId Id of loaded protocol
   */
  void protocolLoaded(unsigned long protocolId);

  /**
   * Add button was pressed
   */
  void addPressed();

  /**
   * Register button was pressed
   */
  void registerPressed();

private:
  static OwnerManagerDlg* myInstance;

  /**
   * Constructor
   * Private so it can only be access through the singleton interface
   *
   * @parent Parent window
   */
  OwnerManagerDlg(QWidget* parent = NULL);

  /**
   * Destructor
   */
  virtual ~OwnerManagerDlg();

  QTreeWidget* myOwnerView;
  QPushButton* myAddButton;
  QPushButton* myRegisterButton;
  QPushButton* myModifyButton;
  QPushButton* myRemoveButton;

  bool myPendingAdd;
  bool myPendingRegister;
};

} // namespace LicqQtGui

#endif // OWNERMANAGERDLG_H
