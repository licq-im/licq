// -*- c-basic-offset: 2 -*-
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

#ifndef CONTACTDELEGATE_H
#define CONTACTDELEGATE_H

#include <QAbstractItemDelegate>

#include "config/iconmanager.h"

#include "contactlist/contactlist.h"

namespace LicqQtGui
{

namespace Config
{
class Skin;
}

class UserViewBase;

/**
 * Delegate that renders contacts in a contact list view
 */
class ContactDelegate : public QAbstractItemDelegate
{
  Q_OBJECT

public:
  /**
   * Constructor
   *
   * @param userView The contact list view the we will render for
   * @param parent Parent object
   */
  ContactDelegate(UserViewBase* userView, QObject* parent = 0);

  /**
   * Calculate how large the a table cell needs to be to display the data
   *
   * @param option Style options
   * @param index Model index to return required size for
   * @return Recomended cell size
   */
  virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

  /**
   * Render a table cell for the contact list view
   *
   * @param p The painter used to draw on the view
   * @param option Style options
   * @param index Model index with data to render
   */
  virtual void paint(QPainter* p, const QStyleOptionViewItem& option, const QModelIndex& index) const;

  /**
   * Create widget that can be used for editing data in an item
   *
   * @param parent Parent widget for the editor
   * @param option Style options
   * @param index Item to edit
   * @return An editor widget
   */
  virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;

  /**
   * Set data for an editor widget
   *
   * @param editor Existing editor widget to update with data
   * @param index Item to get data from
   */
  virtual void setEditorData(QWidget* editor, const QModelIndex& index) const;

  /**
   * Get data from editor widget and update model
   *
   * @param editor Existing editor widget with data
   * @param model Model to write data to
   * @param index Index of item to update
   */
  virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;

  /**
   * Update geometry of an editor widget
   *
   * @param editor Existing editor widget to update geometry for
   * @param option Style options, including new rect for editor
   * @param index Index of item being edited
   */
  virtual void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const;

  /**
   * Catch events for other objects. Used to get events for editor widget
   *
   * @param object Object the event happened for
   * @param event Event for that object
   * @return True if event should not be forwarded to object
   */
  virtual bool eventFilter(QObject* object, QEvent* event);

private:
  /**
   * The data structure to be passed to private helpers
   */
  typedef struct
  {
    QPainter* p;
    QStyleOptionViewItem option;
    QModelIndex index;
    int width;
    int height;
    int align;
    ContactListModel::ItemType itemType;
    Config::Skin* skin;
    QPalette::ColorGroup cg;
    ContactListModel::StatusType status;
    unsigned extStatus;
    QString text;
  } Parameters;

  /**
   * Convenient wrapper to draw an extended icon
   *
   * @param arg The data structure
   * @param iconType Type of the icon to draw
   */
  void drawExtIcon(Parameters& arg, IconManager::IconType iconType) const;

  /**
   * Draw an extended icon
   *
   * @param arg The data structure
   * @param icon The actual icon to draw
   */
  void drawExtIcon(Parameters& arg, const QPixmap* icon) const;

  /**
   * Fills the cell background
   *
   * @param arg The data structure
   */
  void fillBackground(Parameters& arg) const;

  /**
   * Draws the grid around user cells
   *
   * @param arg The data structure
   * @param last Denotes whether this cell is from the last column
   */
  void drawGrid(Parameters& arg, bool last) const;

  /**
   * Prepares the pen in the painter.
   *
   * @param arg The data structure
   * @param animate Current animation role
   */
  void prepareForeground(Parameters& arg, QVariant animate) const;

  /**
   * Draws the delimiting bar.
   *
   * @param arg The data structure
   */
  void drawBar(Parameters& arg) const;

  /**
   * Draws the status icon in the first column,
   * always left-aligned
   *
   * @param arg The data structure
   */
  void drawStatusIcon(Parameters& arg) const;

  /**
   * Draws "Checked Auto-Response" animation
   *
   * @param arg The data structure
   * @param counter The animation step
   */
  void drawCarAnimation(Parameters& arg, int counter) const;

  /**
   * Draws the text
   *
   * @param arg The data structure
   */
  void drawText(Parameters& arg) const;

  /**
   * Draws all extended icons
   *
   * @param arg The data structure
   */
  void drawExtIcons(Parameters& arg) const;

  UserViewBase* myUserView;
};

} // nemaspace LicqQtGui

#endif
