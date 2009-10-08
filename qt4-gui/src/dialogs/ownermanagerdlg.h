/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2004-2009 Licq developers
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

namespace LicqQtGui
{
class RegisterUserDlg;

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
   * List was clicked
   *
   * @param item Current list item
   */
  void listClicked(QTreeWidgetItem* item);

  /**
   * Add an existing account
   */
  void addOwner();

  /**
   * Register a new account
   */
  void registerOwner();

  /**
   * Registration dialog has finished
   *
   * @param success True if a new account was created
   * @param newId Contact id of new account
   * @param newPpid Protocal id of new account
   */
  void registerDone(bool success, const QString& newId, unsigned long newPpid);

  /**
   * Edit currently selected account
   */
  void modifyOwner();

  /**
   * Edit an account
   *
   * @param item Account item, from list, to edit
   * @param column List column, not used
   */
  void modifyOwner(QTreeWidgetItem* item, int column = 0);

  /**
   * Remove currently select account
   */
  void removeOwner();

  /**
   * Refresh account list
   */
  void updateOwners();

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

  RegisterUserDlg* registerUserDlg;

  QTreeWidget* ownerView;
  QPushButton* addButton;
  QPushButton* registerButton;
  QPushButton* modifyButton;
  QPushButton* removeButton;
  QPushButton* closeButton;
};

} // namespace LicqQtGui

#endif // OWNERMANAGERDLG_H
