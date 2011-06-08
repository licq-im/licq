// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2011 Licq developers
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
#include <QTimer>

#include <licq/userid.h>

#include "config/contactlist.h"
#include "config/iconmanager.h"
#include "config/skin.h"

#include "contactlist/maincontactlistproxy.h"
#include "contactlist/mode2contactlistproxy.h"

using Licq::UserId;
using namespace LicqQtGui;

UserView::UserView(ContactListModel* contactList, QWidget* parent)
  : UserViewBase(contactList, parent)
{
  myRemovedUserTimer = new QTimer(this);
  myRemovedUserTimer->setSingleShot(true);
  connect(myRemovedUserTimer, SIGNAL(timeout()), SLOT(forgetRemovedUser()));

  // Use a proxy model for sorting and filtering
  myListProxy = new MainContactListProxy(myContactList, this);
  setModel(myListProxy);

  // This is the main view
  myIsMainView = true;

  // Sorting
  header()->setClickable(true);
  header()->setMovable(false);
  resort();
  connect(header(), SIGNAL(sectionClicked(int)), SLOT(slotHeaderClicked(int)));

  // Appearance
  // Base class constructor doesn't know we overridden applySkin() so it we must call it here again
  applySkin();

  updateRootIndex();

  connect(this, SIGNAL(expanded(const QModelIndex&)), SLOT(slotExpanded(const QModelIndex&)));
  connect(this, SIGNAL(collapsed(const QModelIndex&)), SLOT(slotCollapsed(const QModelIndex&)));
  connect(IconManager::instance(), SIGNAL(iconsChanged()), SLOT(configUpdated()));
  connect(Config::ContactList::instance(), SIGNAL(listLookChanged()), SLOT(configUpdated()));
  connect(Config::ContactList::instance(), SIGNAL(currentListChanged()), SLOT(updateRootIndex()));
  connect(Config::ContactList::instance(), SIGNAL(listSortingChanged()), SLOT(resort()));
  connect(myListProxy, SIGNAL(modelReset()), SLOT(updateRootIndex()));
}

UserView::~UserView()
{
  // Empty
}

UserId UserView::currentUserId() const
{
  if (!currentIndex().isValid())
    return UserId();

  if (static_cast<ContactListModel::ItemType>
      (currentIndex().data(ContactListModel::ItemTypeRole).toInt()) != ContactListModel::UserItem)
    return UserId();

  return currentIndex().data(ContactListModel::UserIdRole).value<UserId>();
}

void UserView::updateRootIndex()
{
  bool mode2View = Config::ContactList::instance()->mode2View();
  int groupId = Config::ContactList::instance()->groupId();

  QModelIndex newRoot = QModelIndex();

  if (groupId == ContactListModel::AllGroupsGroupId)
  {
    // Hide the system groups that exist in the model but should not be displayed in threaded view
    dynamic_cast<MainContactListProxy*>(myListProxy)->setThreadedView(true, mode2View);
  }
  else
  {
    newRoot = myContactList->groupIndex(groupId);
    if (newRoot.isValid())
    {
      // Turn off group filtering first, otherwise we cannot switch from threaded view to a system group
      dynamic_cast<MainContactListProxy*>(myListProxy)->setThreadedView(false, false);

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
  for (int i = 0; i < Config::ContactList::instance()->columnCount(); i++)
    setColumnWidth(i, Config::ContactList::instance()->columnWidth(i));

  setVerticalScrollBarPolicy(Config::ContactList::instance()->allowScrollBar() ?
      Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff);

  if (Config::ContactList::instance()->showHeader())
    header()->show();
  else
    header()->hide();

  spanRowRange(rootIndex(), 0, model()->rowCount(rootIndex()) - 1);
}

void UserView::expandGroups()
{
  // No point in expanding groups unless we can actually see them
  if (rootIndex().isValid())
    return;

  for (int i = 0; i < myListProxy->rowCount(QModelIndex()); ++i)
  {
    QModelIndex index = myListProxy->index(i, 0, QModelIndex());
    if (static_cast<ContactListModel::ItemType>(index.data(ContactListModel::ItemTypeRole).toInt()) != ContactListModel::GroupItem)
      continue;

    int gid = index.data(ContactListModel::GroupIdRole).toInt();
    bool online = (index.data(ContactListModel::SortPrefixRole).toInt() < 2);
    setExpanded(index, Config::ContactList::instance()->groupState(gid, online));
  }
}

void UserView::spanRowRange(const QModelIndex& parent, int start, int end)
{
  for (int i = start; i <= end; i++)
  {
    // get the real model index
    QModelIndex index = model()->index(i, 0, parent);
    unsigned itemType = model()->data(index, ContactListModel::ItemTypeRole).toUInt();

    if (itemType == ContactListModel::GroupItem ||
        itemType == ContactListModel::BarItem)
      setFirstColumnSpanned(i, parent, true);
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

void UserView::rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
  if (currentIndex().isValid() && !myRemovedUser.isValid())
  {
    // Check all the removed rows and see if anyone of them is the currently select user
    for (int i = start; i <= end; ++i)
    {
      if (model()->index(i, 0, parent) != currentIndex())
        continue;

      ContactListModel::ItemType itemType = static_cast<ContactListModel::ItemType>
        (currentIndex().data(ContactListModel::ItemTypeRole).toInt());
      if (itemType == ContactListModel::UserItem)
      {
        // Currently select user is being removed, remember it so we can select it again if it reappears
        myRemovedUser = currentIndex().data(ContactListModel::UserIdRole).value<UserId>();

        // ...but if event loop resumes first, it wasn't just a move between groups so forget it happened
        myRemovedUserTimer->start();
      }
    }
  }

  UserViewBase::rowsAboutToBeRemoved(parent, start, end);
}

void UserView::rowsInserted(const QModelIndex& parent, int start, int end)
{
  UserViewBase::rowsInserted(parent, start, end);
  spanRowRange(parent, start, end);

  // If we just got a new group we may want to expand it
  if (!parent.isValid())
    expandGroups();

  if (myRemovedUser.isValid() && (!parent.isValid() || isExpanded(parent)))
  {
    // We have a user remembered that was just removed, check if he returned
    for (int i = start; i <= end; ++i)
    {
      QModelIndex index = model()->index(i, 0, parent);
      ContactListModel::ItemType itemType = static_cast<ContactListModel::ItemType>
        (index.data(ContactListModel::ItemTypeRole).toInt());

      if (itemType == ContactListModel::UserItem &&
          index.data(ContactListModel::UserIdRole).value<UserId>() == myRemovedUser)
        // User has returned, restore selection
        setCurrentIndex(index);

      if (itemType == ContactListModel::GroupItem && isExpanded(index))
      {
        // Inserted row was a group, then it might have users as sub items, check them too
        int rows = model()->rowCount(index);
        for (int j = 0; j < rows; ++j)
        {
          QModelIndex subindex = model()->index(j, 0, index);
          ContactListModel::ItemType subitemType = static_cast<ContactListModel::ItemType>
            (subindex.data(ContactListModel::ItemTypeRole).toInt());

          if (subitemType == ContactListModel::UserItem &&
              subindex.data(ContactListModel::UserIdRole).value<UserId>() == myRemovedUser)
            // The appeared group has the user as a sub item, restore selection
            setCurrentIndex(subindex);
        }
      }
    }
  }
}

void UserView::forgetRemovedUser()
{
  myRemovedUser = UserId();
}

void UserView::reset()
{
  UserViewBase::reset();
  // QTreeView::reset will collapse all groups so we have to reexpand them here
  expandGroups();
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
        {
          bool wasExpanded = isExpanded(clickedItem);
          setExpanded(clickedItem, !wasExpanded);

          // setExpand may fail, for example after changing sorting an
          // expanded group can be collapsed but sometimes cannot be expanded
          // again. This was seen with Qt 4.4.0.
          if (isExpanded(clickedItem) == wasExpanded)
          {
            // Setting expanded state to same state as view currently (falsely)
            // reports seems to fix it, then set it again to the state we
            // actually wanted, this times it works.
            setExpanded(clickedItem, wasExpanded);
            setExpanded(clickedItem, !wasExpanded);
          }
        }
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
      else
      {
        popupMenu(viewport()->mapToGlobal(QPoint(40, visualRect(currentIndex()).y())), currentIndex());
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

  QString id = index.data(ContactListModel::AccountIdRole).toString();
  unsigned long ppid = index.data(ContactListModel::PpidRole).toUInt();

  if ((event->buttons() & Qt::LeftButton) && !myMousePressPos.isNull() &&
      (QPoint(event->pos() - myMousePressPos).manhattanLength() >= QApplication::startDragDistance()))
  {
    char p[5];
    Licq::protocolId_toStr(p, ppid);
    QString data(p);
    data += id;

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

  // Group expansion gets confused when sorting is changed so refresh it
  expandGroups();
}

void UserView::slotExpanded(const QModelIndex& index)
{
  int gid = index.data(ContactListModel::GroupIdRole).toInt();
  bool online = (index.data(ContactListModel::SortPrefixRole).toInt() < 2);
  Config::ContactList::instance()->setGroupState(gid, online, true);
}

void UserView::slotCollapsed(const QModelIndex& index)
{
  int gid = index.data(ContactListModel::GroupIdRole).toInt();
  bool online = (index.data(ContactListModel::SortPrefixRole).toInt() < 2);
  Config::ContactList::instance()->setGroupState(gid, online, false);
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
