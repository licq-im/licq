/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2012 Licq developers <licq-dev@googlegroups.com>
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

#include "config.h"

#include "contactdelegate.h"

#include <QApplication>
#include <QKeyEvent>
#include <QLineEdit>
#include <QPainter>

#include <licq/contactlist/user.h>

#include "config/contactlist.h"
#include "config/iconmanager.h"
#include "config/skin.h"

#include "userviewbase.h"

using Licq::User;
using namespace LicqQtGui;

ContactDelegate::ContactDelegate(UserViewBase* userView, bool useSkin, QObject* parent)
  : QAbstractItemDelegate(parent),
    myUserView(userView),
    myUseSkin(useSkin)
{
  // Empty
}

QSize ContactDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  // Make a copy so we can make our own style options
  QStyleOptionViewItem textStyle = option;

  ContactListModel::ItemType itemType = static_cast<ContactListModel::ItemType>
    (index.data(ContactListModel::ItemTypeRole).toInt());

  if (itemType == ContactListModel::GroupItem ||
      itemType == ContactListModel::BarItem)
  {
    // Smaller font for group and subgroup headings
    if (textStyle.font.pointSize() > 2)
      textStyle.font.setPointSize(textStyle.font.pointSize() - 2);
  }

  QSize size = QSize(0, 0);

  QVariant var;
  if ((var = index.data(Qt::DisplayRole)).isValid())
  {
    QStringList lines = var.toString().split('\n');
    size.setHeight(textStyle.fontMetrics.height() * lines.count());
    foreach (const QString& line, lines)
    {
      int w = textStyle.fontMetrics.width(line);
      if (w > size.width())
        size.setWidth(w);
    }
  }

  // Make sure we have enough height for icons
  // User icons are always 16x16 so set minimum size to 18
  if (itemType != ContactListModel::BarItem && size.height() < 18)
    size.setHeight(18);

  // For first column, add width for icon
  if (index.column() == 0)
    size.setWidth(size.width() + 18);

  return size;
}

void ContactDelegate::paint(QPainter* p, const QStyleOptionViewItem& option,
    const QModelIndex& index) const
{
  // Painter uses coordinates of entire widget,
  // move it so coordinates are relative to
  // the top left corner of this cell
  p->save();
  p->translate(option.rect.topLeft());

  QVariant var;

  // Fill up the data structure
  Parameters arg =
  {
    p,
    option,
    index,
    option.rect.width(),
    option.rect.height(),
    (1 << Config::ContactList::instance()->columnAlignment(index.column())) |
      Qt::AlignVCenter,
    static_cast<ContactListModel::ItemType>
      (index.data(ContactListModel::ItemTypeRole).toInt()),
    Config::Skin::active(),
    option.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled,
    index.data(ContactListModel::StatusRole).toUInt(),
    index.data(ContactListModel::ExtendedStatusRole).toUInt(),
    QString::null
  };

  // Some corrections to the data above
  if (arg.cg == QPalette::Normal && !(option.state & QStyle::State_Active))
    arg.cg = QPalette::Inactive;
  if ((var = index.data(Qt::DisplayRole)).isValid())
    arg.text = var.toString();

  fillBackground(arg);
  drawGrid(arg, !(index.model()->columnCount() - index.column() - 1));
  prepareForeground(arg, index.data(ContactListModel::OnlineAnimationRole));

  switch (arg.itemType)
  {
    case ContactListModel::BarItem:
      drawBar(arg);
      break;
    default:
      if ((var = index.data(ContactListModel::CarAnimationRole)).isValid())
        drawCarAnimation(arg, var.toInt());
      if (index.column() == 0)
        drawStatusIcon(arg);
      if (arg.itemType == ContactListModel::GroupItem)
        arg.align = Qt::AlignLeft | Qt::AlignVCenter;
      drawText(arg);
      drawExtIcons(arg);
  }

  p->restore();
}

void ContactDelegate::drawExtIcon(Parameters& arg, IconManager::IconType iconType) const
{
  drawExtIcon(arg,
      const_cast<QPixmap*>(&IconManager::instance()->getIcon(iconType)));
}

void ContactDelegate::drawExtIcon(Parameters& arg, const QPixmap* icon) const
{
  if (icon->isNull() || arg.width < icon->width())
    return;

  int x = 0, y = 0;
  int align = arg.align;

  switch (align & Qt::AlignVertical_Mask)
  {
    case Qt::AlignVCenter:
      y = (arg.height - icon->height()) / 2;
      break;
    case Qt::AlignBottom:
      y = arg.height - icon->height();
  }

  if (y < 0)
    y = 0;

  align &= Qt::AlignHorizontal_Mask;

  if (align == Qt::AlignRight)
    x = arg.width - icon->width();

  arg.p->drawPixmap(x, y, *icon);

  arg.width -= icon->width() + 3;
  if (align != Qt::AlignRight)
    arg.p->translate(icon->width() + 3, 0);
}

void ContactDelegate::fillBackground(Parameters& arg) const
{
#define FILL(color) \
  arg.p->fillRect(0, 0, arg.width, arg.height, (color))

  if (!myUseSkin)
  {
    if (arg.option.state & QStyle::State_Selected)
      FILL(arg.option.palette.brush(arg.cg, QPalette::Highlight));
    else
      FILL(arg.option.palette.brush(arg.cg, QPalette::Base));
    return;
  }

  // we assume the same coloring for groups and bars, since they are
  // mutually exclusive.
  if (arg.itemType == ContactListModel::GroupItem ||
      arg.itemType == ContactListModel::BarItem)
  {
    if (!arg.skin->groupBackImage.isNull())
      if (arg.skin->tileGroupBackImage)
        arg.p->drawTiledPixmap(0, 0, arg.width, arg.height,
            QPixmap::fromImage(arg.skin->groupBackImage));
      else
        arg.p->drawImage(0, 0,
            arg.skin->groupBackImage.scaled(arg.width, arg.height));
    else
      if (arg.option.state & QStyle::State_Selected)
        if (arg.skin->groupHighBackColor.isValid())
          FILL(arg.skin->groupHighBackColor);
        else
          FILL(arg.option.palette.brush(arg.cg, QPalette::Highlight));
      else
        if (arg.skin->groupBackColor.isValid())
          FILL(arg.skin->groupBackColor);
  }
  else
  {
    if ((arg.option.state & QStyle::State_Selected) != 0)
    {
      if (arg.skin->highBackColor.isValid())
        FILL(arg.skin->highBackColor);
      else
        FILL(arg.option.palette.brush(arg.cg, QPalette::Highlight));
    }
  }
#undef FILL
}

void ContactDelegate::drawGrid(Parameters& arg, bool last) const
{
  if (myUseSkin && Config::ContactList::instance()->showGridLines() &&
      arg.itemType == ContactListModel::UserItem)
  {
    arg.p->setPen(arg.skin->gridlineColor);
    arg.p->drawRect(0, 0, arg.width, arg.height);
    // FIXME: right border is not shown for the last column...
    // And giving width - 1 above would result in thick
    // vertical border between the columns.
    if (last)
      arg.p->drawLine(arg.width - 1, 0, arg.width - 1, arg.height - 1);
  }
}

void ContactDelegate::prepareForeground(Parameters& arg, QVariant animate) const
{
  arg.option.displayAlignment = (Qt::AlignmentFlag)arg.align;

  // Setup font and color to use
  arg.option.font.setItalic(false);
  arg.option.font.setStrikeOut(false);
  arg.option.font.setWeight(QFont::Normal);

  QColor textColor;

  switch (arg.itemType)
  {
    case ContactListModel::UserItem:
      {
        if (Config::ContactList::instance()->useFontStyles())
        {
          if (arg.extStatus & ContactListModel::OnlineNotifyStatus)
            arg.option.font.setWeight(QFont::DemiBold);
          if (arg.extStatus & ContactListModel::InvisibleListStatus)
            arg.option.font.setStrikeOut(true);
          if (arg.extStatus & ContactListModel::VisibleListStatus)
            arg.option.font.setItalic(true);
        }

        if (animate.isValid() && (animate.toInt() & 1))
          textColor = arg.skin->offlineColor;
        else if ((arg.option.state & QStyle::State_Selected) != 0)
          textColor = arg.skin->highTextColor;
        else if (arg.extStatus & ContactListModel::NewUserStatus)
          textColor = arg.skin->newUserColor;
        else if (arg.extStatus & ContactListModel::AwaitingAuthStatus)
          textColor = arg.skin->awaitingAuthColor;
        else
        {
          if (arg.status == User::OfflineStatus)
            textColor = arg.skin->offlineColor;
          else if (arg.status & (User::AwayStatuses | User::IdleStatus))
            textColor = arg.skin->awayColor;
          else
            textColor = arg.skin->onlineColor;
        }
        break;
      }

      case ContactListModel::GroupItem: // we assume the same coloring for
      case ContactListModel::BarItem:   // groups and bars, since they are
      default:                          // mutually exclusive.
        if (arg.itemType == ContactListModel::GroupItem)
          arg.option.font.setWeight(QFont::Bold);
        if (arg.option.font.pointSize() > 2)
          arg.option.font.setPointSize(arg.option.font.pointSize() - 2);
        if ((arg.option.state & QStyle::State_Selected) != 0)
          textColor = arg.skin->groupHighTextColor;
        else
          textColor = arg.skin->groupTextColor;
        break;
  }

  if (textColor.isValid() && myUseSkin)
  {
    arg.option.palette.setColor(QPalette::Text, textColor);
    arg.p->setPen(textColor);
  }
  else
  {
    // compatibility fallbacks
    if ((arg.option.state & QStyle::State_Selected) != 0)
      arg.p->setPen(arg.option.palette.color(arg.cg, QPalette::HighlightedText));
    else
      if ((arg.itemType == ContactListModel::GroupItem ||
           arg.itemType == ContactListModel::BarItem) &&
          arg.skin->gridlineColor.isValid())
        arg.p->setPen(arg.skin->gridlineColor);
      else
        arg.p->setPen(arg.option.palette.color(arg.cg, QPalette::Text));
  }

  arg.p->setFont(arg.option.font);
}

void ContactDelegate::drawBar(Parameters& arg) const
{
  int textWidth = arg.p->fontMetrics().width(arg.text);
  int barWidth = (arg.width >> 1) - 20 - (textWidth >> 1) - 5;

  if (barWidth > 0)
  {
    int barHeight = arg.height >> 1;

    qDrawShadeLine(arg.p,
        20, barHeight,
        20 + barWidth, barHeight,
        arg.option.palette);
    qDrawShadeLine(arg.p,
        arg.width - 20 - barWidth, barHeight,
        arg.width - 20, barHeight,
        arg.option.palette);
  }

  if (!arg.text.isEmpty())
    arg.p->drawText(0, 0, arg.width, arg.height, Qt::AlignCenter, arg.text);
}

void ContactDelegate::drawStatusIcon(Parameters& arg) const
{
  const QPixmap* icon = NULL;
  IconManager* iconman = IconManager::instance();

  if (arg.itemType == ContactListModel::UserItem)
  {
    QVariant var = arg.index.data(ContactListModel::EventAnimationRole);
    if (myUseSkin && var.isValid() && (var.toInt() & 1))
      icon = &iconman->iconForEvent(
          arg.index.data(ContactListModel::EventTypeRole).toUInt());
    else
      icon = &iconman->iconForStatus(
          arg.index.data(ContactListModel::StatusRole).toUInt(),
          arg.index.data(ContactListModel::UserIdRole).value<Licq::UserId>());
  }
  else if (arg.itemType == ContactListModel::GroupItem)
  {
    if (myUserView->isExpanded(arg.index))
      icon = &iconman->getIcon(IconManager::ExpandedIcon);
    else
      icon = &iconman->getIcon(IconManager::CollapsedIcon);
  }

  if (icon != NULL)
  {
    // Draw the icon
    int iconWidth = qMax(icon->width(), 18);
    arg.p->drawPixmap(
        (iconWidth - icon->width()) / 2,
        (arg.height - icon->height()) / 2,
        *icon);

    // Adjust drawing coordinates to exclude status icon
    // and 2-pixel padding
    arg.width -= iconWidth + 2;
    arg.p->translate(iconWidth + 2, 0);
  }
}

void ContactDelegate::drawCarAnimation(Parameters& arg, int counter) const
{
  if (!myUseSkin)
    return;

  QPen tmp = arg.p->pen();
  arg.p->setPen((counter & 1) ? Qt::white : Qt::black);

  // Draw a rectangle around the contact which will span over multiple cells
  // if the view has more than one column
  arg.p->drawLine(0, 0, arg.width - 1, 0);
  arg.p->drawLine(0, arg.height - 1, arg.width - 1, arg.height - 1);
  if (arg.index.column() == 0)
    arg.p->drawLine(0, 0, 0, arg.height - 1);
  if (arg.index.column() == arg.index.model()->columnCount() - 1)
    arg.p->drawLine(arg.width - 1, 0, arg.width - 1, arg.height - 1);
  arg.p->setPen(tmp);
}

void ContactDelegate::drawText(Parameters& arg) const
{
  if (arg.text.isEmpty())
    return;

  QStringList lines = arg.text.split('\n');
  for (int i = 0; i < lines.count(); ++i)
  {
    lines[i] = arg.p->fontMetrics().elidedText(
        lines[i], arg.option.textElideMode, arg.width - 6);
  }
  QString elidedText = lines.join("\n");

  arg.p->drawText(2, 0, arg.width - 4, arg.height, arg.align, elidedText);

  int textWidth = arg.p->fontMetrics().width(elidedText);

  switch (arg.align & Qt::AlignHorizontal_Mask)
  {
    case Qt::AlignHCenter: // Fall through
      textWidth += arg.width - 2;
      textWidth /= 2;
    case Qt::AlignLeft: // Fall through
      arg.p->translate(textWidth + 6, 0);
    case Qt::AlignRight:
      arg.width -= textWidth + 6;
  }
}

void ContactDelegate::drawExtIcons(Parameters& arg) const
{
  if (!myUseSkin)
    return;

  if (arg.itemType == ContactListModel::GroupItem)
  {
    if (arg.index.data(ContactListModel::UnreadEventsRole).toInt() > 0 &&
        !myUserView->isExpanded(arg.index))
      drawExtIcon(arg, IconManager::StandardMessageIcon);
    return;
  }

  if (Config::ContactList::instance()->columnFormat(arg.index.column())
      .contains("%a"))
  {
    if (Config::ContactList::instance()->showUserIcons())
    {
      QVariant var = arg.index.data(ContactListModel::UserIconRole);
      if (var.isValid() && var.canConvert(QVariant::Image))
      {
        QImage tmp = var.value<QImage>();
        if (tmp.height() > arg.height - 2)
          tmp = tmp.scaledToHeight(arg.height - 2, Qt::SmoothTransformation);
        QPixmap* pic = new QPixmap(QPixmap::fromImage(tmp));
        drawExtIcon(arg, pic);
        delete pic;
      }
    }

#define EXTICON(status, icon) \
  if (arg.extStatus & (status)) \
    drawExtIcon(arg, (icon))

    if (Config::ContactList::instance()->showExtendedIcons())
    {
      if (Config::ContactList::instance()->showPhoneIcons())
      {
        EXTICON(ContactListModel::PhoneStatus, IconManager::PhoneIcon);
        EXTICON(ContactListModel::CellularStatus, IconManager::CellularIcon);
      }
      EXTICON(ContactListModel::BirthdayStatus, IconManager::BirthdayIcon);
      EXTICON(ContactListModel::InvisibleStatus, IconManager::InvisibleIcon);

      // pmGPGKey
      if (arg.extStatus & ContactListModel::GpgKeyStatus)
      {
        EXTICON(ContactListModel::GpgKeyEnabledStatus,
            IconManager::GpgKeyEnabledIcon);
        else
          drawExtIcon(arg, IconManager::GpgKeyDisabledIcon);
      }

      if (arg.status != User::OfflineStatus)
      {
        if (Config::ContactList::instance()->showPhoneIcons())
        {
          EXTICON(ContactListModel::PhoneFollowMeActiveStatus,
              IconManager::PfmActiveIcon);
          else
          EXTICON(ContactListModel::PhoneFollowMeBusyStatus,
              IconManager::PfmBusyIcon);

          EXTICON(ContactListModel::IcqPhoneActiveStatus,
              IconManager::IcqPhoneActiveIcon);
          EXTICON(ContactListModel::IcqPhoneBusyStatus,
              IconManager::IcqPhoneBusyIcon);
        }

        EXTICON(ContactListModel::SharedFilesStatus, IconManager::SharedFilesIcon);
        EXTICON(ContactListModel::TypingStatus, IconManager::TypingIcon);
      }

      EXTICON(ContactListModel::SecureStatus, IconManager::SecureOnIcon);
      EXTICON(ContactListModel::CustomArStatus, IconManager::CustomArIcon);
    }
  }
#undef EXTICON
}

QWidget* ContactDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& /* option */, const QModelIndex& /* index */) const
{
  QLineEdit* editor = new QLineEdit(parent);

  // Don't use the view's skinned palette
  editor->setPalette(QApplication::palette());

  return editor;
}

void ContactDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  QString value = index.model()->data(index, ContactListModel::NameRole).toString();

  QLineEdit* lineedit = dynamic_cast<QLineEdit*>(editor);
  lineedit->setText(value);
}

void ContactDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  QLineEdit* lineedit = dynamic_cast<QLineEdit*>(editor);
  model->setData(index, lineedit->text(), ContactListModel::NameRole);
}

void ContactDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QRect r = option.rect;

  // Leave space for icon in first column
  if (index.column() == 0)
    r.setLeft(r.left() + 18);

  editor->setGeometry(r);
}

bool ContactDelegate::eventFilter(QObject* object, QEvent* event)
{
  QWidget* editor = qobject_cast<QWidget*>(object);
  if (editor == NULL)
    return false;

  if (event->type() == QEvent::KeyPress)
  {
    switch (dynamic_cast<QKeyEvent*>(event)->key())
    {
      case Qt::Key_Enter:
      case Qt::Key_Return:
        emit commitData(editor);
        emit closeEditor(editor, QAbstractItemDelegate::SubmitModelCache);
        return true;
      case Qt::Key_Escape:
        // don't commit data
        emit closeEditor(editor, QAbstractItemDelegate::RevertModelCache);
        return true;
    }
  }
  else if (event->type() == QEvent::FocusOut)
  {
    emit commitData(editor);
    emit closeEditor(editor, NoHint);
  }

  return QAbstractItemDelegate::eventFilter(object, event);
}

