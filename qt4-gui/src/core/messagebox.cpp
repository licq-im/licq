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

#include "messagebox.h"

#include "config.h"

#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#ifdef USE_KDE
#include <KDE/KApplication>
#include <KDE/KIconLoader>
#include <KDE/KMessageBox>
#endif

using namespace std;
using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::MessageBox */

/*! \brief Dialog with configurable yes/no buttons
 *
 * In it's easiest form, this dialog displays query and the buttons button1 and button2.
 * button1 means true/yes,
 * button2 means false/no.
 *
 * When bConfirmYes is true, then a second dialog asks the user to confirm his positive
 * decision, the displayed confirmation message is passed using the QString szConfirmYes.
 *
 * When bConfirmNo is true, then a second dialog asks the user to confirm his negative
 * decision, the displayed confirmation message is passed using the QString szConfirmNo.
 */
bool LicqQtGui::QueryUser(QWidget* parent, QString query, QString button1, QString button2,
    bool confirmYes, QString confirmYesText, bool confirmNo, QString confirmNoText)
{
  bool result;

#ifdef USE_KDE
  result = (KMessageBox::questionYesNo(parent, query, QMessageBox::tr("Licq Question"),
        KGuiItem(button1), KGuiItem(button2)) == KMessageBox::Yes);
  // The user must confirm his decision!
  if (result == true && confirmYes && !confirmYesText.isEmpty())
    result = (KMessageBox::questionYesNo(parent, confirmYesText,
          QMessageBox::tr("Licq Question")) == KMessageBox::Yes);
  else if (result == false && confirmNo && !confirmNoText.isEmpty())
    result = (KMessageBox::questionYesNo(parent, confirmNoText,
          QMessageBox::tr("Licq Question")) == KMessageBox::Yes);
#else
  result = (QMessageBox::question(parent, QMessageBox::tr("Licq Question"), query,
        button1, button2) == 0);
  // The user must confirm his decision!
  if (result == true && confirmYes && !confirmYesText.isEmpty())
    result = (QMessageBox::question(parent, QMessageBox::tr("Licq Question"), confirmYesText,
          QMessageBox::tr("Yes"), QMessageBox::tr("No")) == 0);
  else if (result == false && confirmNo && !confirmNoText.isEmpty())
    result = (QMessageBox::question(parent, QMessageBox::tr("Licq Question"), confirmNoText,
          QMessageBox::tr("Yes"), QMessageBox::tr("No")) == 0);
#endif

  return result;
}

int LicqQtGui::QueryUser(QWidget* parent, QString query, QString button1, QString button2, QString button3)
{
  return ( QMessageBox::question(parent, QMessageBox::tr("Licq Question"), query, button1, button2, button3));
}

bool LicqQtGui::QueryYesNo(QWidget* parent, QString query)
{
  return (QMessageBox::question(parent, QMessageBox::tr("Licq Question"), query, QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes);
}

void LicqQtGui::InformUser(QWidget* parent, QString text)
{
  MessageBox::addMessage(QMessageBox::Information, text, parent);
}

void LicqQtGui::WarnUser(QWidget* parent, QString text)
{
  MessageBox::addMessage(QMessageBox::Warning, text, parent);
}

void LicqQtGui::CriticalUser(QWidget* parent, QString text)
{
  MessageBox::addMessage(QMessageBox::Critical, text, parent);
}

// -----------------------------------------------------------------------------

MessageBox::MessageBox(QWidget* parent)
  : QDialog(parent, Qt::Dialog), myUnreadCount(0)
{
  setModal(true);
  setObjectName("LicqInfo");
  setWindowTitle(tr("Licq"));

  // Start with no message
  QString msg = "";
  QMessageBox::Icon type = QMessageBox::Information;

  QVBoxLayout* topLayout = new QVBoxLayout(this);

  // Make the first horizontal layout for the icon and message
  QHBoxLayout* messageLayout = new QHBoxLayout();
  myIconLabel = new QLabel();
  QPixmap icon = getMessageIcon(type);
  myIconLabel->setPixmap(icon);
  myMessageLabel = new QLabel(msg);

  messageLayout->addStretch(1);
  messageLayout->addWidget(myIconLabel);
  messageLayout->addWidget(myMessageLabel);
  messageLayout->addStretch(1);

  // Make the list box of all the pending messages, starts out hidden
  myExtension = new QWidget();
  myExtension->hide();
  QHBoxLayout* listLayout = new QHBoxLayout(myExtension);
  listLayout->setMargin(0);
  myMessageList = new QListWidget();
  myMessageList->setFixedHeight(100); // This seems to be a good height
  listLayout->addWidget(myMessageList);

  // Make the second horizontal layout for the buttons
  QHBoxLayout* buttonsLayout = new QHBoxLayout();
  myMoreButton = new QPushButton(tr("&List"));
  myMoreButton->setCheckable(true);
  //myMoreButton->setDisabled(true);
  myNextButton = new QPushButton(tr("&Next"));
  myNextButton->setDisabled(true);
  myCloseButton = new QPushButton(tr("&Ok"));
  myCloseButton->setDefault(true);

  buttonsLayout->addStretch(1);
  buttonsLayout->addWidget(myMoreButton);
  buttonsLayout->addWidget(myNextButton);
  buttonsLayout->addWidget(myCloseButton);
  buttonsLayout->addStretch(1);

  topLayout->addLayout(messageLayout);
  topLayout->addLayout(buttonsLayout);
  topLayout->addWidget(myExtension);

  // Connect all the signals here
  connect(myMoreButton, SIGNAL(toggled(bool)), myExtension, SLOT(setVisible(bool)));
  connect(myNextButton, SIGNAL(clicked()), SLOT(showNext()));
  connect(myCloseButton, SIGNAL(clicked()), SLOT(closeDialog()));
  connect(myMessageList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
      SLOT(updateCurrentMessage(QListWidgetItem*, QListWidgetItem*)));

  show();
}

MessageBox::~MessageBox()
{
  myMessageDialog = NULL;
}

void MessageBox::addMessage(QMessageBox::Icon type, const QString& msg)
{
  bool unread = false;

  // The icons we will show
  QPixmap pix = getMessageIcon(type);
  QImage img;

  // If we have only one message in queue, show that one, otherwise update
  // the number of pending messages.
  if (myMessageList->count() == 0)
  {
    myIconLabel->setPixmap(pix);
    myMessageLabel->setText(msg);
    myNextButton->setText(tr("&Next"));
    myNextButton->setEnabled(false);
    myMoreButton->setEnabled(false);
    myNextButton->hide();
    myMoreButton->hide();
    myCloseButton->setText(tr("&Ok"));
  }
  else
  {
    myUnreadCount++;
    unread = true; // It is unread
    myCloseButton->setText(tr("&Clear All"));
    QString nextStr = QString(tr("&Next (%1)")).arg(myUnreadCount);
    myNextButton->setText(nextStr);
    if (!myNextButton->isEnabled())
    {
      myNextButton->setEnabled(true);
      myNextButton->show();
    }
    if (!myMoreButton->isEnabled())
    {
      myMoreButton->setEnabled(true);
      myMoreButton->show();
    }
  }

  MessageBoxItem* entry = new MessageBoxItem();
  // Resize the icon
  img = pix.toImage();
  if (img.width() > img.height())
    img = img.scaledToWidth(16);
  else
    img = img.scaledToHeight(16);
  QPixmap scaledPix(QPixmap::fromImage(img));
  // Add the columns now
  entry->setIcon(scaledPix);
  entry->setText(msg.left(qMin(50, msg.indexOf('\n')))); // Put this in setMessage()
  // Set the special data
  entry->setMessage(msg);
  entry->setFullIcon(pix);
  entry->setUnread(unread);
  entry->setType(type);

  myMessageList->insertItem(0, entry);

  // Set the caption if we set the text and icon here
  if (myUnreadCount == 0)
    updateCaption(entry);
}

/// ////////////////////////////////////////////////////////
/// @brief Show the next pending message
///
/// When the next button is clicked, we will show the next pending
/// unread message. The text and icon will both be updated.
/// ////////////////////////////////////////////////////////
void MessageBox::showNext()
{
  // Find the next unread message
  bool found = false;
  MessageBoxItem* item = 0;
  for (int i = myMessageList->currentRow(); i >= 0; --i)
  {
    item = dynamic_cast<MessageBoxItem*>(myMessageList->item(i));
    if (item->isUnread())
    {
      found = true;
      break;
    }
  }

  // If no unread item was found, search from the bottom
  if (!found)
  {
    for (int i = myMessageList->count() - 1; i >= 0; --i)
    {
      item = dynamic_cast<MessageBoxItem*>(myMessageList->item(i));
      if (item->isUnread())
      {
        found = true;
        break;
      }
    }
  }

  // Only change the item if there something to change it to
  if (found)
    myMessageList->setCurrentItem(item);
}

/// ////////////////////////////////////////////////////////
/// @brief Clear all pending messages and close the message box
///
/// When the clear all button is clicked, we will close the dialog and
/// remove all messages from the queue.
/// ////////////////////////////////////////////////////////
void MessageBox::closeDialog()
{
  // Hide the window first
  hide();

  // Remove all items that have been read
  myMessageList->clear();
  myUnreadCount = 0;
}

/// ////////////////////////////////////////////////////////
/// @brief The user changed the selected list item
///
/// When the item changes, the message box's icon and message that
/// it shows, must be changed to match what the user selected.
///
/// @param i The new QListWidgetItem* that has been selected
/// @param previous The QListWidgetItem* that has been selected prior
/// ////////////////////////////////////////////////////////
void MessageBox::updateCurrentMessage(QListWidgetItem* i, QListWidgetItem* /* previous */)
{
  // Change the icon, message and caption
  MessageBoxItem* item = dynamic_cast<MessageBoxItem*>(i);
  if (item != NULL)
  {
    myIconLabel->setPixmap(item->getFullIcon());
    myMessageLabel->setText(item->getMessage());
    updateCaption(item);

    // Mark it as read
    if (item->isUnread())
    {
      myUnreadCount--;
      item->setUnread(false);
    }
  }

  // Update the next button
  QString nextStr;
  if (myUnreadCount > 0)
    nextStr = QString(tr("&Next (%1)")).arg(myUnreadCount);
  else
  {
    // No more unread messages
    nextStr = QString(tr("&Next"));
    myNextButton->setEnabled(false);
    myUnreadCount = 0;
  }
  myNextButton->setText(nextStr);
}

/// ////////////////////////////////////////////////////////
/// @brief Change the caption of the message box
///
/// @param item The MessageBoxItem that will be shown
/// ////////////////////////////////////////////////////////
void MessageBox::updateCaption(MessageBoxItem* item)
{
  if (!item)
    return;

  QString caption;
  switch (item->getType())
  {
    case QMessageBox::Information:
      caption = tr("Licq Information");
      break;

    case QMessageBox::Warning:
      caption = tr("Licq Warning");
      break;

    case QMessageBox::Critical:
      caption = tr("Licq Critical");
      break;

    case QMessageBox::NoIcon:
    case QMessageBox::Question:
    default:
      caption = tr("Licq");
      break;
  }
  setWindowTitle(caption);
}

QPixmap MessageBox::getMessageIcon(QMessageBox::Icon type)
{
#ifdef USE_KDE
  QString iconName;
  switch (type)
  {
    case QMessageBox::Information:
      iconName = "messagebox_info";
      break;
    case QMessageBox::Warning:
      iconName = "messagebox_warning";
      break;
    case QMessageBox::Critical:
      iconName = "messagebox_critical";
      break;
    case QMessageBox::NoIcon:
    case QMessageBox::Question:
    default:
      return QPixmap();
  }

  QPixmap icon = KIconLoader::global()->loadIcon(iconName,
    KIconLoader::NoGroup, KIconLoader::SizeMedium, KIconLoader::DefaultState,
    QStringList(), 0, true);
  if (icon.isNull())
    icon = QMessageBox::standardIcon(type);
#else
  QPixmap icon = QMessageBox::standardIcon(type);
#endif

  return icon;
}

MessageBox* MessageBox::myMessageDialog = 0;

void MessageBox::addMessage(QMessageBox::Icon type, const QString& msg,
  QWidget* parent)
{
  // We should pass parent to it, but it causes a crash after the parent closes
  // and we try to show another message box. I tried to reparent myMessageDialog, but
  // that didnt help much.
  // So for now.. we do this:
  parent = 0; // XXX See comment above
  if (myMessageDialog == 0)
    myMessageDialog = new MessageBox(parent);

  myMessageDialog->addMessage(type, msg);
  myMessageDialog->show();
}

MessageBoxItem::MessageBoxItem(QListWidget* parent)
  : QListWidgetItem(parent)
{
  myUnread = false;
}

void MessageBoxItem::setUnread(bool unread)
{
  myUnread = unread;
  if (myUnread)
    setForeground(Qt::red);
  else
    setForeground(Qt::black);
}
