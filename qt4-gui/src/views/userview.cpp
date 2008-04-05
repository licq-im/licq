// -*- c-basic-offset: 2 -*-
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

#include "userview.h"

#include <QApplication>
#include <QHeaderView>
#include <QMouseEvent>

extern char* PPIDSTRING(unsigned long);

#include "config/contactlist.h"
#include "config/iconmanager.h"
#include "config/skin.h"

#include "contactlist/maincontactlistproxy.h"

#include "core/usermenu.h"

using namespace LicqQtGui;

UserView::UserView(ContactListModel* contactList, UserMenu* mnuUser, QWidget* parent)
  : UserViewBase(contactList, mnuUser, parent)
{
  // Use a proxy model for sorting and filtering
  myListProxy = new MainContactListProxy(myContactList, this);
  setModel(myListProxy);

  // Sorting
  header()->setClickable(true);
  header()->setMovable(false);
  resort();
  connect(header(), SIGNAL(sectionClicked(int)), SLOT(slotHeaderClicked(int)));

  // Appearance
  // Base class constructor doesn't know we overridden applySkin() so it we must call it here again
  applySkin();
  setVerticalScrollBarPolicy(Config::ContactList::instance()->allowScrollBar() ? Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff);

  updateRootIndex();

  connect(this, SIGNAL(expanded(const QModelIndex&)), SLOT(slotExpanded(const QModelIndex&)));
  connect(this, SIGNAL(collapsed(const QModelIndex&)), SLOT(slotCollapsed(const QModelIndex&)));
  connect(IconManager::instance(), SIGNAL(iconsChanged()), SLOT(configUpdated()));
  connect(Config::ContactList::instance(), SIGNAL(listLookChanged()), SLOT(configUpdated()));
  connect(Config::ContactList::instance(), SIGNAL(currentListChanged()), SLOT(updateRootIndex()));
  connect(Config::ContactList::instance(), SIGNAL(listSortingChanged()), SLOT(resort()));
}

UserView::~UserView()
{
  // Empty
}

bool UserView::MainWindowSelectedItemUser(QString& id, unsigned long& ppid) const
{
  if (static_cast<ContactListModel::ItemType>
      (currentIndex().data(ContactListModel::ItemTypeRole).toInt()) != ContactListModel::UserItem)
    return false;

  id = currentIndex().data(ContactListModel::UserIdRole).toString();
  ppid = currentIndex().data(ContactListModel::PpidRole).toUInt();
  return true;
}

void UserView::updateRootIndex()
{
  bool threadView = Config::ContactList::instance()->threadView();
  GroupType groupType = Config::ContactList::instance()->groupType();
  unsigned long groupId = Config::ContactList::instance()->groupId();

  QModelIndex newRoot = QModelIndex();

  // All Users group is considered to be a System Group
  if (groupId == GROUP_ALL_USERS && groupType != GROUPS_SYSTEM)
    groupType = GROUPS_SYSTEM;

  if (threadView && groupId == GROUP_ALL_USERS)
  {
    // Hide the system groups that exist in the model but should not be displayed in threaded view
    dynamic_cast<MainContactListProxy*>(myListProxy)->setThreadedView(true);
  }
  else
  {
    newRoot = myContactList->groupIndex(groupType, groupId);
    if (newRoot.isValid())
    {
      // Turn off group filtering first, otherwise we cannot switch from threaded view to a system group
      dynamic_cast<MainContactListProxy*>(myListProxy)->setThreadedView(false);

      // Hidden groups may not be sorted, force a resort just in case
      resort();
    }
  }

  UserViewBase::setRootIndex(myListProxy->mapFromSource(newRoot));
  expandGroups();
  configUpdated();
}

void UserView::configUpdated()
{
  // Set column widths
  for (unsigned short i = 0; i < Config::ContactList::instance()->columnCount(); i++)
    setColumnWidth(i, Config::ContactList::instance()->columnWidth(i));

  if (Config::ContactList::instance()->showHeader())
    header()->show();
  else
    header()->hide();

  setGroupSpanning();
}

void UserView::expandGroups()
{
  for (int i = 0; i < myListProxy->rowCount(QModelIndex()); ++i)
  {
    QModelIndex index = myListProxy->index(i, 0, QModelIndex());
    unsigned short gid = index.data(ContactListModel::GroupIdRole).toUInt();

    setExpanded(index, Config::ContactList::instance()->groupState(gid));
  }
}

void UserView::setGroupSpanning()
{
  int rows = model()->rowCount(rootIndex());

  if (rootIndex().isValid())
  {
    // Single group (non-threaded) view - bars are sorted with contacts
    for (int i = 0; i < rows; ++i)
      if (model()->data(model()->index(i, 0, rootIndex()), ContactListModel::ItemTypeRole).toUInt() == ContactListModel::BarItem)
        setFirstColumnSpanned(i, rootIndex(), true);
  }
  else
  {
    // Full tree (threaded) view - all top level items are groups
    for (int i = 0; i < rows; ++i)
      setFirstColumnSpanned(i, rootIndex(), true);
  }
}

void UserView::setColors(QColor back)
{
  UserViewBase::setColors(back);

  if (!Config::ContactList::instance()->useSystemBackground() &&
      Config::Skin::active()->frame.transparent)
  {
    QPalette pal = palette();
    pal.setBrush(QPalette::Base, Qt::NoBrush);
    setPalette(pal);
  }
}

void UserView::applySkin()
{
  setFrameStyle(Config::Skin::active()->frame.frameStyle);
  UserViewBase::applySkin();
}

void UserView::mousePressEvent(QMouseEvent* event)
{
  UserViewBase::mousePressEvent(event);

  if (event->button() == Qt::LeftButton)
  {
    QModelIndex clickedItem = indexAt(event->pos());
    if (clickedItem.isValid())
    {
      ContactListModel::ItemType itemType = static_cast<ContactListModel::ItemType>
        (currentIndex().data(ContactListModel::ItemTypeRole).toInt());
      if (itemType == ContactListModel::GroupItem)
      {
        if (event->pos().x() <= 18) // we clicked an icon area
          setExpanded(clickedItem, !isExpanded(clickedItem));
      }
    }
    else
    {
      // Clicking outiside list will clear selection
      selectionModel()->clearSelection();
      setCurrentIndex(QModelIndex());
    }
  }
}

void UserView::keyPressEvent(QKeyEvent* event)
{
  if (event->modifiers() & (Qt::ControlModifier | Qt::AltModifier))
  {
    event->ignore();
    UserViewBase::keyPressEvent(event);
    return;
  }

  ContactListModel::ItemType itemType = static_cast<ContactListModel::ItemType>
    (currentIndex().data(ContactListModel::ItemTypeRole).toInt());

  switch (event->key())
  {
    case Qt::Key_Return:
    case Qt::Key_Enter:
      if (itemType == ContactListModel::UserItem)
      {
        emit doubleClicked(currentIndex());
        break;
      }
      // Fall through so return key expands and collapses groups

    case Qt::Key_Space:
      if (itemType == ContactListModel::GroupItem)
      {
        setExpanded(currentIndex(), !isExpanded(currentIndex()));
      }
      else if (itemType == ContactListModel::UserItem)
      {
        QString id = currentIndex().data(ContactListModel::UserIdRole).toString();
        unsigned long ppid = currentIndex().data(ContactListModel::PpidRole).toUInt();

        myUserMenu->popup(viewport()->mapToGlobal(QPoint(40, visualRect(currentIndex()).y())), id, ppid);
      }
      return;

    default:
      UserViewBase::keyPressEvent(event);
  }
}

void UserView::mouseMoveEvent(QMouseEvent* event)
{
  UserViewBase::mouseMoveEvent(event);

  QModelIndex index = currentIndex();
  if (index.isValid() == false)
    return;

  if (static_cast<ContactListModel::ItemType>
      (index.data(ContactListModel::ItemTypeRole).toInt()) != ContactListModel::UserItem)
    return;

  QString id = index.data(ContactListModel::UserIdRole).toString();
  unsigned long ppid = index.data(ContactListModel::PpidRole).toUInt();

  if ((event->buttons() & Qt::LeftButton) && !myMousePressPos.isNull() &&
      (QPoint(event->pos() - myMousePressPos).manhattanLength() >= QApplication::startDragDistance()))
  {
    char* p = PPIDSTRING(ppid);
    QString data(p);
    data += id;
    delete [] p;

    QDrag* drag = new QDrag(this);
    QMimeData* mimeData = new QMimeData;
    mimeData->setText(data);
    drag->setMimeData(mimeData);
    drag->start(Qt::CopyAction);
  }
}

void UserView::resort()
{
  int column = Config::ContactList::instance()->sortColumn();
  Qt::SortOrder order = (Config::ContactList::instance()->sortColumnAscending() ? Qt::AscendingOrder : Qt::DescendingOrder);

  // Column 0 means sort on status or unsorted
  if (column == 0)
  {
    dynamic_cast<SortedContactListProxy*>(myListProxy)->sort(0, ContactListModel::SortRole, Qt::AscendingOrder);

    header()->setSortIndicatorShown(false);
  }
  else
  {
    // Column numbers in configuration is off by one
    column--;

    dynamic_cast<SortedContactListProxy*>(myListProxy)->sort(column, Qt::DisplayRole, order);

    header()->setSortIndicatorShown(true);
    header()->setSortIndicator(column, order);
  }
}

void UserView::slotExpanded(const QModelIndex& index)
{
  unsigned short gid = index.data(ContactListModel::GroupIdRole).toUInt();
  Config::ContactList::instance()->setGroupState(gid, true);
}

void UserView::slotCollapsed(const QModelIndex& index)
{
  unsigned short gid = index.data(ContactListModel::GroupIdRole).toUInt();
  Config::ContactList::instance()->setGroupState(gid, false);
}

void UserView::slotHeaderClicked(int column)
{
  // Clicking on a header will switch between three sorting modes
  //  - Ascending sort on the clicked column
  //  - Descending sort on the clicked colmun
  //  - Default sort (unsorted or sort by status)

  // Columns in configuration is off by one as status was previously a separate column
  column++;

  if (Config::ContactList::instance()->sortColumn() == 0)
  {
    // Sort mode was default, change to ascending of this column
    Config::ContactList::instance()->setSortColumn(column, true);
  }
  else if (Config::ContactList::instance()->sortColumn() != column)
  {
    // Sorting was of other column, change to ascending of this column
    Config::ContactList::instance()->setSortColumn(column, true);
  }
  else if (Config::ContactList::instance()->sortColumnAscending() == true)
  {
    // Sorting was ascending of current column, change to descending
    Config::ContactList::instance()->setSortColumn(column, false);
  }
  else
  {
    // Sorting was descending of current column, change to default
    Config::ContactList::instance()->setSortColumn(0, true);
  }
}
