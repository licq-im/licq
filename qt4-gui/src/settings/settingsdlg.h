/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2012 Licq developers <licq-dev@googlegroups.com>
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

#ifndef SETTINGSDLG_H
#define SETTINGSDLG_H

#include <config.h>

#include <QDialog>
#include <QMap>

namespace LicqQtGui
{
class TreePager;

namespace Settings
{
  class Chat;
  class ContactList;
  class Events;
  class General;
  class Network;
  class Plugins;
  class Shortcuts;
  class Skin;
  class Status;
}

/**
 * Dialog for holding configuration settings that can be changed by the user.
 */
class SettingsDlg : public QDialog
{
  Q_OBJECT

public:
  enum SettingsPage
  {
    UnknownPage = -1,
    ContactListPage,
    SkinPage,
    DockingPage,
    FontsPage,
    MainwinShortcutsPage,
    ContactInfoPage,
    ChatPage,
    ChatDispPage,
    HistDispPage,
    ChatShortcutsPage,
    OnEventPage,
    SoundsPage,
    FilterPage,
    NetworkPage,
    StatusPage,
    PluginsPage,
  };

  /**
   * Create and show the settings dialog.
   * If dialog is already showing it just sets the current page to startPage.
   *
   * @param page Initial page to show
   */
  static void show(SettingsPage page = ContactListPage);

  /**
   * Select page to show
   *
   * @param page Page to show
   */
  void showPage(SettingsPage page);

  /**
   * Add page to settings dialog
   *
   * @param page Page id
   * @param widget Widget containg the actual page
   * @param title Page title
   * @param parent Parent page if not a top level page
   */
  void addPage(SettingsPage page, QWidget* widget, const QString& title, SettingsPage parent = UnknownPage);

private slots:
  /**
   * Apply settings to running gui.
   * Does not save settings to disk or close dialog
   */
  void apply();

  /**
   * Apply and save settings and close dialog.
   */
  void ok();

private:
  // Only allow one instance at a time so keep track if we have one open
  static SettingsDlg* myInstance;

  /**
   * Constructor
   *
   * @param parent Parent widget
   */
  SettingsDlg(QWidget* parent = 0);

  /**
   * Destructor
   */
  virtual ~SettingsDlg();


  TreePager* myPager;
  QMap<SettingsPage, QWidget*> myPages;

  Settings::Chat* myChatSettings;
  Settings::ContactList* myContactListSettings;
  Settings::Events* myEventsSettings;
  Settings::General* myGeneralSettings;
  Settings::Network* myNetworkSettings;
  Settings::Plugins* myPluginsSettings;
  Settings::Shortcuts* myShortcutsSettings;
  Settings::Skin* mySkinSettings;
  Settings::Status* myStatusSettings;
};

} // namespace LicqQtGui

#endif
