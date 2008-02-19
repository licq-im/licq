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

#include "userviewbase.h"

#include <QUrl>

#include <licq_icqd.h>
extern char* PPIDSTRING(unsigned long);

#include "contactlist/contactlist.h"

#include "userevents/usersendcontactevent.h"
#include "userevents/usersendfileevent.h"
#include "userevents/usersendmsgevent.h"
#include "userevents/usersendurlevent.h"

#include "config/contactlist.h"
#include "config/skin.h"

#include "core/licqgui.h"
#include "core/mainwin.h"
#include "core/usermenu.h"

#include "contactdelegate.h"

using namespace LicqQtGui;

/**
 * Get a pointer to a dialog for sending events with the given @a type, to the contact
 * @a id over the protocol @a ppid. If the dialog is already open, the dialog is
 * converted to the correct type before being returned.
 *
 * @returns a pointer to a send event dialog of type @a T, or NULL on error.
 */
template<typename T>
static T* getSendEventDialog(EventType type, QString id, unsigned long ppid)
{
  int function;
  if (type == ET_CONTACT)
    function = mnuUserSendContact;
  else if (type == ET_FILE)
    function = mnuUserSendFile;
  else if (type == ET_MESSAGE)
    function = mnuUserSendMsg;
  else if (type == ET_URL)
    function = mnuUserSendUrl;
  else
    return NULL;

  UserEventCommon* common = LicqGui::instance()->showEventDialog(function, id, ppid);
  if (!common)
    return NULL;

  T* dialog = dynamic_cast<T*>(common);
  if (!dialog)
  {
    UserSendCommon* base = dynamic_cast<UserSendCommon*>(common);
    if (!base)
      return NULL;

    base->changeEventType(type);
    dialog = dynamic_cast<T*>(LicqGui::instance()->showEventDialog(function, id, ppid));
    if (!dialog)
      return NULL;
  }

  return dialog;
}

/// Convenient, wrapper functions for getSendEventDialog.
static inline UserSendContactEvent* getSendContactEventDialog(QString id, unsigned long ppid)
{
  return getSendEventDialog<UserSendContactEvent>(ET_CONTACT, id, ppid);
}

static inline UserSendFileEvent* getSendFileEventDialog(QString id, unsigned long ppid)
{
  return getSendEventDialog<UserSendFileEvent>(ET_FILE, id, ppid);
}

static inline UserSendMsgEvent* getSendMsgEventDialog(QString id, unsigned long ppid)
{
  return getSendEventDialog<UserSendMsgEvent>(ET_MESSAGE, id, ppid);
}

static inline UserSendUrlEvent* getSendUrlEventDialog(QString id, unsigned long ppid)
{
  return getSendEventDialog<UserSendUrlEvent>(ET_URL, id, ppid);
}

UserViewBase::UserViewBase(ContactListModel* contactList, UserMenu* mnuUser, QWidget* parent)
  : QTreeView(parent),
    myContactList(contactList),
    myUserMenu(mnuUser)
{
  setItemDelegate(new ContactDelegate(this, this));

  // Look'n'Feel
  setIndentation(0);
  setAcceptDrops(true);
  setRootIsDecorated(false);
  setAllColumnsShowFocus(true);
  applySkin();

  connect(this, SIGNAL(doubleClicked(const QModelIndex&)),
      SLOT(slotDoubleClicked(const QModelIndex&)));

  connect(Config::Skin::active(), SIGNAL(frameChanged()), SLOT(applySkin()));
}

UserViewBase::~UserViewBase()
{
  // Empty
}

void UserViewBase::setColors(QColor back)
{
  if (!Config::ContactList::instance()->useSystemBackground())
  {
    QPalette pal = palette();

    if (back.isValid())
      pal.setColor(QPalette::Base, back);
    else
      pal.setColor(QPalette::Base, QColor("silver"));

    setPalette(pal);
  }
}

void UserViewBase::applySkin()
{
  setPalette(Config::Skin::active()->palette(gMainWindow));
  setColors(Config::Skin::active()->backgroundColor);
}

void UserViewBase::mousePressEvent(QMouseEvent* event)
{
  QTreeView::mousePressEvent(event);

  if (event->button() == Qt::LeftButton)
  {
    // Save position for later, needed by dragging and moving floaties
    myMousePressPos = event->pos();
  }
  else if (event->button() == Qt::MidButton)
  {
    QModelIndex clickedItem = indexAt(event->pos());
    if (clickedItem.isValid())
    {
      // Needed to distinguish double click origination
      if (static_cast<ContactListModel::ItemType>
          (clickedItem.data(ContactListModel::ItemTypeRole).toInt()) == ContactListModel::GroupItem)
        midEvent = true;
      emit doubleClicked(clickedItem);
    }
  }
}

void UserViewBase::mouseReleaseEvent(QMouseEvent* event)
{
  QTreeView::mouseReleaseEvent(event);

  myMousePressPos.setX(0);
  myMousePressPos.setY(0);
}

void UserViewBase::contextMenuEvent(QContextMenuEvent* event)
{
  if (myUserMenu == NULL)
    return;

  QModelIndex clickedItem = indexAt(event->pos());
  if (clickedItem.isValid())
  {
    setCurrentIndex(clickedItem);

    if (static_cast<ContactListModel::ItemType>
        (clickedItem.data(ContactListModel::ItemTypeRole).toInt()) == ContactListModel::UserItem)
    {
      QString id = clickedItem.data(ContactListModel::UserIdRole).toString();
      unsigned long ppid = clickedItem.data(ContactListModel::PpidRole).toUInt();

      myUserMenu->popup(viewport()->mapToGlobal(event->pos()), id, ppid);
    }
  }
}

void UserViewBase::dragEnterEvent(QDragEnterEvent* event)
{
  if (event->mimeData()->hasText() ||
      event->mimeData()->hasUrls())
    event->acceptProposedAction();
}

void UserViewBase::dropEvent(QDropEvent* event)
{
  // We ignore the event per default and then accept it if we
  // get to the end of this function.
  event->ignore();

  QModelIndex dropIndex = indexAt(event->pos());
  if (!dropIndex.isValid())
    return;

  ContactListModel::ItemType itemType = static_cast<ContactListModel::ItemType>
    (dropIndex.data(ContactListModel::ItemTypeRole).toInt());

  switch (itemType)
  {
    case ContactListModel::UserItem:
    {
      QString id = dropIndex.data(ContactListModel::UserIdRole).toString();
      unsigned long ppid = dropIndex.data(ContactListModel::PpidRole).toUInt();

      if (event->mimeData()->hasUrls())
      {
        QList<QUrl> urlList = event->mimeData()->urls();
        QListIterator<QUrl> urlIter(urlList);
        QString text;
        QUrl firstUrl = urlIter.next();

        if (!(text = firstUrl.toLocalFile()).isEmpty())
        {
          UserSendFileEvent* sendFile = getSendFileEventDialog(id, ppid);
          if (!sendFile)
            return;

          sendFile->setFile(text, QString::null);

          // Add all the files
          while (urlIter.hasNext())
          {
            if (!(text = urlIter.next().toLocalFile()).isEmpty())
              sendFile->addFile(text);
          }

          sendFile->show();
        }
        else
        {
          UserSendUrlEvent* sendUrl = getSendUrlEventDialog(id, ppid);
          if (!sendUrl)
            return;

          sendUrl->setUrl(firstUrl.toString(), QString::null);
          sendUrl->show();
        }
      }
      else if (event->mimeData()->hasText())
      {
        QString text = event->mimeData()->text();

        unsigned long dropPpid = 0;
        FOR_EACH_PROTO_PLUGIN_START(gLicqDaemon)
        {
          if (text.startsWith(PPIDSTRING((*_ppit)->PPID())))
          {
            dropPpid = (*_ppit)->PPID();
            break;
          }
        }
        FOR_EACH_PROTO_PLUGIN_END

        if (dropPpid != 0 && text.length() > 4)
        {
          QString dropId = text.mid(4);
          if (id == dropId && ppid == dropPpid)
            return;

          UserSendContactEvent* sendContact = getSendContactEventDialog(id, ppid);
          if (!sendContact)
            return;

          sendContact->setContact(dropId, dropPpid);
          sendContact->show();
        }
        else
        {
          UserSendMsgEvent* sendMsg = getSendMsgEventDialog(id, ppid);
          if (!sendMsg)
            return;

          sendMsg->setText(text);
          sendMsg->show();
        }
      }
      break;
    }
    case ContactListModel::GroupItem:
    {
      unsigned short gid = dropIndex.data(ContactListModel::GroupIdRole).toUInt();

      if (event->mimeData()->hasText() && event->mimeData()->text().length() > 4)
      {
        QString text = event->mimeData()->text();

        unsigned long dropPpid = 0;
        FOR_EACH_PROTO_PLUGIN_START(gLicqDaemon)
        {
          if (text.startsWith(PPIDSTRING((*_ppit)->PPID())))
          {
            dropPpid = (*_ppit)->PPID();
            break;
          }
        }
        FOR_EACH_PROTO_PLUGIN_END;

        if (dropPpid == 0)
          return;

        QString dropId = text.mid(4);

        if (!dropId.isEmpty())
        {
          gUserManager.AddUserToGroup(dropId.toLatin1(), dropPpid, gid);
          gMainWindow->updateUserWin();
        }
      }
      else
        return; // Not accepted
      break;
    }
    default:
      break;
  }

  event->acceptProposedAction();
}

void UserViewBase::dragMoveEvent(QDragMoveEvent* /* event */)
{
  // Do nothing, just overload the function so base class won't interfere
}

void UserViewBase::slotDoubleClicked(const QModelIndex& index)
{
  if (static_cast<ContactListModel::ItemType>
      (index.data(ContactListModel::ItemTypeRole).toInt()) == ContactListModel::UserItem)
  {
    QString id = index.data(ContactListModel::UserIdRole).toString();
    unsigned long ppid = index.data(ContactListModel::PpidRole).toUInt();

    emit userDoubleClicked(id, ppid);
  }
  else
  if (static_cast<ContactListModel::ItemType>
      (index.data(ContactListModel::ItemTypeRole).toInt()) == ContactListModel::GroupItem &&
      (index.column() != 0 || midEvent))
  {
    midEvent = false;
    setExpanded(index, !isExpanded(index));
  }
}
