/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2009 Licq developers
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

#ifndef SETTINGS_SKIN_H
#define SETTINGS_SKIN_H

#include <QList>
#include <QObject>
#include <QPixmap>
#include <QWidget>

class QComboBox;
class QGroupBox;
class QLabel;
class QStringList;
class QVBoxLayout;

namespace LicqQtGui
{
class SettingsDlg;
class SkinBrowserPreviewArea;

typedef QList<QPixmap> IconList;

namespace Config
{
class Skin;
}

namespace Settings
{
/**
 * Provides settings for skin and icon sets with previews
 */
class Skin : public QObject
{
  Q_OBJECT

public:
  /**
   * Constructor
   *
   * @param parent Settings dialog
   */
  Skin(SettingsDlg* parent);

  /**
   * Destructor
   */
  virtual ~Skin() {};

  /**
   * Prepare page for viewing by loading configuration
   */
  void load();

  /**
   * Save configuration
   */
  void apply();

private slots:
  /**
   * Adjust the available space for previews
   *
   * @param event Resize event
   */
  void resizeEvent(QResizeEvent* event);

  /**
   * Opens an editor to allow editing of the currently selected skin
   */
  void editSkin();

  /**
   * Load a skin and render it in the preview panel
   *
   * @param skin Name of skin
   */
  void previewSkin(const QString& skin);

  /**
   * Load an icon set into the preview panel
   *
   * @param icon Name of icon set
   */
  void previewIcons(const QString& icon);

  /**
   * Load an extended icon set into the preview panel
   *
   * @param extIcon Name of icon set
   */
  void previewExtIcons(const QString& extIcon);

  /**
   * Load an emoticon set into the preview panel
   *
   * @param emoticon Name of icon set
   */
  void previewEmoticons(const QString& emoticon);

private:
  /**
   * Setup the skin page
   *
   * @param parent Parent widget for settings page
   * @return a widget with the skin settings
   */
  QWidget* createPageSkin(QWidget* parent);

  /**
   * Load list of icon sets
   *
   * @param subdir Sub directory to find icon sets in
   * @param iconCombo Combo box to populate with icon set names
   * @param current Currently active icon set
   * @param exampleIcon Name of icon to show with name in combo box
   */
  void loadIconsetList(const QString& subdir, QComboBox* iconCombo,
      const QString& current, const QString& exampleIcon);

  /**
   * Load an icon set
   *
   * @param iconSet Name of icon set to load
   * @param subdir Sub directory to find icon set in
   * @param iconNames List of icon names to show in preview
   * @return A list of pixmaps to show in preview area
   */
  IconList loadIcons(const QString& iconSet, const QString& subdir,
      const QStringList& iconNames);

  /**
   * Renders a dynamic skin preview
   *
   * @param skinName Name of skin to use
   * @return Miniature image of mainwin with skin
   */
  QPixmap renderSkin(const QString& skinName);


  QVBoxLayout* myPageSkinLayout;

  QComboBox* mySkinCombo;
  QComboBox* myIconCombo;
  QComboBox* myExtIconCombo;
  QComboBox* myEmoticonCombo;
  QLabel* mySkinPreview;
  SkinBrowserPreviewArea* myIconPreview;
  SkinBrowserPreviewArea* myExtIconPreview;
  SkinBrowserPreviewArea* myEmoticonPreview;
  QStringList myIconNames;
  QStringList myExtIconNames;
};

} // namespace Settings

/**
 * Helper class to provide a preview area for our icons using a modified QFrame
 *
 * This class is derived from QFrame and extends minor functionality
 * to provide support for many icons to be painted on this widget.
 * The main improvement is, to make it redraw the icons when the widget
 * gets modified by movement, hiding or resizing.
 */
class SkinBrowserPreviewArea : public QWidget
{
  Q_OBJECT

public:
  SkinBrowserPreviewArea(QWidget* parent = 0);

  /**
   * Sets list of icons to display in preview area
   *
   * @param iconList List of icons to display
   */
  void setPixmapList(const IconList& iconList);

private:
  void paintEvent(QPaintEvent* e);

  IconList myIconList;
};

} // namespace LicqQtGui

#endif
