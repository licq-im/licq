/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2006-2009 Licq developers
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

#include "treepager.h"

#include <QHBoxLayout>
#include <QStackedLayout>
#include <QTreeWidget>

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::TreePager */

TreePager::TreePager(QWidget* parent)
  : QWidget(parent)
{
  QHBoxLayout* boxLayout = new QHBoxLayout(this);
  boxLayout->setContentsMargins(0, 0, 0, 0);

  // Display page titles in a tree widget
  myTreeList = new QTreeWidget;
  myTreeList->setColumnCount(1);
  myTreeList->setHeaderLabel(tr("Categories"));
  // Minimize the widget to be able to dynamically enlarge it later.
  myTreeList->resize(0, 0);
  boxLayout->addWidget(myTreeList);

  // Hold the actual pages in a stack layout
  myPageStack = new QStackedLayout;
  boxLayout->addLayout(myPageStack, 1);

  // Connect signals
  connect(myTreeList, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
      SLOT(flipPage(QTreeWidgetItem*)));
}

void TreePager::addPage(QWidget* page, const QString& title, /* const QIcon& icon, */ QWidget* parent)
{
  QTreeWidgetItem* parentItem = NULL;
  if (parent != NULL)
    parentItem = myPageMap.key(parent);

  QTreeWidgetItem* item;
  if (parentItem == NULL)
    item = new QTreeWidgetItem(myTreeList, QStringList(title));
  else
    item = new QTreeWidgetItem(parentItem, QStringList(title));

//  item->setIcon(0, icon);
  myTreeList->expandItem(item);
  myTreeList->resizeColumnToContents(0);
  myTreeList->setFixedWidth(myTreeList->columnWidth(0) + myTreeList->frameWidth() * 2);

  myPageStack->addWidget(page);
  myPageMap.insert(item, page);
}

void TreePager::showPage(QWidget* page)
{
  myPageStack->setCurrentWidget(page);
  myTreeList->setCurrentItem(myPageMap.key(page));
  emit currentPageChanged(page);
}

QWidget* TreePager::currentPage() const
{
  return myPageStack->currentWidget();
}

void TreePager::flipPage(QTreeWidgetItem* selection)
{
  QWidget* w = myPageMap[selection];
  if (w == NULL)
    return;

  myPageStack->setCurrentWidget(w);
  emit currentPageChanged(w);
}
