/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2006 Licq developers
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

#include <QFrame>
#include <QLinkedList>
#include <QObject>
#include <QPixmap>

class QComboBox;
class QGroupBox;
class QLabel;
class QStringList;
class QVBoxLayout;

namespace LicqQtGui
{
class SettingsDlg;
class SkinBrowserPreviewArea;

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
  virtual ~Skin();

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
  void loadSkin(const QString& skin);

  /**
   * Load an icon set into the preview panel
   *
   * @param icon Name of icon set
   */
  void loadIcons(const QString& icon);

  /**
   * Load an extended icon set into the preview panel
   *
   * @param extIcon Name of icon set
   */
  void loadExtIcons(const QString& extIcon);

  /**
   * Load an emoticon set into the preview panel
   *
   * @param emoticon Name of icon set
   */
  void loadEmoticons(const QString& emoticon);

private:
  /**
   * Setup the skin page
   *
   * @param parent Parent widget for settings page
   * @return a widget with the skin settings
   */
  QWidget* createPageSkin(QWidget* parent);

  /**
   * Renders a dynamic skin preview
   *
   * @param skinName Name of skin to use
   * @return Miniature image of mainwin with skin
   */
  QPixmap renderSkin(const QString& skinName);


  QVBoxLayout* myPageSkinLayout;

  /*! This Box holds the Dropdown menus for Skin selection */
  QGroupBox* boxSkin;
  /*! This Box holds the preview area */
  QGroupBox* boxPreview;
  /*! This Combo contains all available skin packs */
  QComboBox* cmbSkin;
  /*! This Combo contains all available icon packs */
  QComboBox* cmbIcon;
  /*! This Combo contains all available extended icon packs */
  QComboBox* cmbExtIcon;
  /*! This Combo contains all available emoticons themes */
  QComboBox* cmbEmoticon;
  /*! This QLabel contains the skin preview pixmap (75 x 130 Pixel)*/
  QLabel* lblPaintSkin;
  /*! This SkinBrowserPreviewArea contains the icon pixmaps (54 x 130 Pixel)*/
  SkinBrowserPreviewArea* lblPaintIcon;
  /*! This SkinBrowserPreviewArea contains the extended icons pixmaps (54 x 130 Pixel)*/
  SkinBrowserPreviewArea* lblPaintExtIcon;
  /*! This SkinBrowserPreviewArea contains the emoticons pixmaps (54x130 Px) */
  SkinBrowserPreviewArea* lblPaintEmoticon;
  /*! Holds the list of possible themeable icons in normal icon sets */
  QStringList* lstAIcons;
  /*! Holds the list of possible themeable icons in extended icon sets */
  QStringList* lstAExtIcons;
  /*! Stores the current QPixmap for the Skin preview */
  QPixmap* pmSkin;
  /*! Stores the list of the current QPixmaps for the Icons preview */
  QLinkedList<QPixmap>* lstIcons;
  /*! Stores the list of the current QPixmaps for the Extended Icons preview */
  QLinkedList<QPixmap>* lstExtIcons;
  /*! Stores the list of the current QPixmaps for the emoticons preview */
  QLinkedList<QPixmap>* lstEmoticons;
  Config::Skin* skin;
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
class SkinBrowserPreviewArea : public QFrame
{
Q_OBJECT
public:
  SkinBrowserPreviewArea(QWidget* parent = 0);
  void setPixmapList(QLinkedList<QPixmap>* _lstPm);

private:
  void paintEvent(QPaintEvent* e);
  /*! Holds a copy of the pixmaps that are drawn on the widget */
  QLinkedList<QPixmap> lstPm;
};

} // namespace LicqQtGui

#endif
