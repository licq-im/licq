/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2006 Licq developers
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

#ifndef TREEPAGER_H
#define TREEPAGER_H

#include <QMap>
#include <QWidget>

class QStackedLayout;
class QTreeWidget;
class QTreeWidgetItem;

namespace LicqQtGui
{
/**
 * Widget for showing one of multiple pages at a time using a tree list for page selection.
 * Intended mainly for option dialogs.
 */
class TreePager : public QWidget
{
  Q_OBJECT

public:
  /**
   * Standard contructor.
   */
  TreePager(QWidget* parent = NULL);

  /**
   * Adds a page to this control.
   *
   * @param page The widget containing the page
   * @param title Title to display in the tree list.
   * @param parent Parent page  if this is a sub page or NULL to make it a top level page.
   */
  void addPage(QWidget* page, QString title, /* QIcon icon, */ QWidget* parent = NULL);

  /**
   * Changes which page to show.
   *
   * @param page A page to show. Must have been added to this control.
   */
  void showPage(QWidget* page);

  /**
   * Get current visible page
   *
   * @return Current page
   */
  QWidget* currentPage() const;

signals:
  /**
   * Current page has changed
   *
   * @param page New current page
   */
  void currentPageChanged(QWidget* page);

private:
  QStackedLayout* myPageStack;
  QTreeWidget* myTreeList;
  QMap<QTreeWidgetItem*, QWidget*> myPageMap;

private slots:

  /**
   * Handler for selection changes in the tree list.
   * Updates which page to show.
   */
  void flipPage(QTreeWidgetItem* selection);
};

} // namespace LicqQtGui

#endif
