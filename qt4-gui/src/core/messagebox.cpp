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

#include <algorithm>
#include <ctype.h>
#include <stdio.h>

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
 * In it's easiest form, this dialog displays szQuery and the buttons szBtn1 and szBtn2.
 * szBtn1 means true/yes,
 * szBtn2 means false/no.
 *
 * When bConfirmYes is true, then a second dialog asks the user to confirm his positive
 * decision, the displayed confirmation message is passed using the QString szConfirmYes.
 *
 * When bConfirmNo is true, then a second dialog asks the user to confirm his negative
 * decision, the displayed confirmation message is passed using the QString szConfirmNo.
 */
bool LicqQtGui::QueryUser(QWidget* q, QString szQuery, QString szBtn1, QString szBtn2,
    bool bConfirmYes, QString szConfirmYes, bool bConfirmNo, QString szConfirmNo)
{
  bool result;

#ifdef USE_KDE
  result = (KMessageBox::questionYesNo(q, szQuery, QMessageBox::tr("Licq Question"),
        KGuiItem(szBtn1), KGuiItem(szBtn2)) == KMessageBox::Yes);
  // The user must confirm his decision!
  if (result == true && bConfirmYes && !szConfirmYes.isEmpty())
    result = (KMessageBox::questionYesNo(q, szConfirmYes,
          QMessageBox::tr("Licq Question")) == KMessageBox::Yes);
  else if (result == false && bConfirmNo && !szConfirmNo.isEmpty())
    result = (KMessageBox::questionYesNo(q, szConfirmNo,
          QMessageBox::tr("Licq Question")) == KMessageBox::Yes);
#else
  result = (QMessageBox::question(q, QMessageBox::tr("Licq Question"), szQuery,
        szBtn1, szBtn2) == 0);
  // The user must confirm his decision!
  if (result == true && bConfirmYes && !szConfirmYes.isEmpty())
    result = (QMessageBox::question(q, QMessageBox::tr("Licq Question"), szConfirmYes,
          QMessageBox::tr("Yes"), QMessageBox::tr("No")) == 0);
  else if (result == false && bConfirmNo && !szConfirmNo.isEmpty())
    result = (QMessageBox::question(q, QMessageBox::tr("Licq Question"), szConfirmNo,
          QMessageBox::tr("Yes"), QMessageBox::tr("No")) == 0);
#endif

  return result;
}

int LicqQtGui::QueryUser(QWidget* q, QString szQuery, QString szBtn1, QString szBtn2, QString szBtn3)
{
  return ( QMessageBox::question(q, QMessageBox::tr("Licq Question"), szQuery, szBtn1, szBtn2, szBtn3));
}

bool LicqQtGui::QueryYesNo(QWidget* parent, QString query)
{
  return (QMessageBox::question(parent, QMessageBox::tr("Licq Question"), query, QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes);
}


void LicqQtGui::InformUser(QWidget* q, QString sz)
{
  MessageManager::Instance()->addMessage(QMessageBox::Information, sz, q);
}

void LicqQtGui::WarnUser(QWidget* q, QString sz)
{
  MessageManager::Instance()->addMessage(QMessageBox::Warning, sz, q);
}

void LicqQtGui::CriticalUser(QWidget* q, QString sz)
{
  MessageManager::Instance()->addMessage(QMessageBox::Critical, sz, q);
}

// -----------------------------------------------------------------------------

MessageBox::MessageBox(QWidget* parent)
  : QDialog(parent, Qt::Dialog), m_nUnreadNum(0)
{
  setModal(true);
  setObjectName("LicqInfo");
  setWindowTitle(tr("Licq"));

  // Start with no message
  QString msg = "";
  QMessageBox::Icon type = QMessageBox::Information;

  QVBoxLayout* topLay = new QVBoxLayout(this);

  // Make the first horizontal layout for the icon and message
  QFrame* frmMessage = new QFrame();
  QHBoxLayout* lay = new QHBoxLayout(frmMessage);
  m_lblIcon = new QLabel();
  QPixmap icon = getMessageIcon(type);
  m_lblIcon->setPixmap(icon);
  m_lblMessage = new QLabel(msg);

  lay->addWidget(m_lblIcon, 0, Qt::AlignCenter);
  lay->addWidget(m_lblMessage);

  // Make the list box of all the pending messages, starts out hidden
  m_frmList = new QFrame();
  QHBoxLayout* layList = new QHBoxLayout(m_frmList);
  m_lstMsg = new QListWidget();
  m_lstMsg->setFixedHeight(100); // This seems to be a good height
  layList->addWidget(m_lstMsg);
  // Add this listbox as an extension to the dialog, and make it shown at the
  // bottom part of the dialog.
  setOrientation(Qt::Vertical);
  setExtension(m_frmList);

  // Make the second horizontal layout for the buttons
  QFrame* frmButtons = new QFrame();
  QHBoxLayout* lay2 = new QHBoxLayout(frmButtons);
  m_btnMore = new QPushButton(tr("&List"));
  //m_btnMore->setDisabled(true);
  m_btnNext = new QPushButton(tr("&Next"));
  m_btnNext->setDisabled(true);
  m_btnClear = new QPushButton(tr("&Ok"));
  m_btnClear->setDefault(true);

  lay2->addWidget(m_btnMore);
  lay2->addWidget(m_btnNext);
  lay2->addWidget(m_btnClear);

  topLay->addWidget(frmMessage, 0, Qt::AlignCenter);
  topLay->addWidget(frmButtons, 0, Qt::AlignCenter);

  // Connect all the signals here
  connect(m_btnMore, SIGNAL(clicked()), SLOT(slot_toggleMore()));
  connect(m_btnNext, SIGNAL(clicked()), SLOT(slot_clickNext()));
  connect(m_btnClear, SIGNAL(clicked()), SLOT(slot_clickClear()));
  connect(m_lstMsg, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
      SLOT(slot_listChanged(QListWidgetItem*, QListWidgetItem*)));

  show();
}

void MessageBox::addMessage(QMessageBox::Icon type, const QString& msg)
{
  bool unread = false;

  // The icons we will show
  QPixmap pix = getMessageIcon(type);
  QImage img;

  // If we have only one message in queue, show that one, otherwise update
  // the number of pending messages.
  if (m_lstMsg->count() == 0)
  {
    m_lblIcon->setPixmap(pix);
    m_lblMessage->setText(msg);
    m_btnNext->setText(tr("&Next"));
    m_btnNext->setEnabled(false);
    m_btnMore->setEnabled(false);
    m_btnNext->hide();
    m_btnMore->hide();
    m_btnClear->setText(tr("&Ok"));
    showExtension(false); // We are opening the window, so default to not showing this
  }
  else
  {
    m_nUnreadNum++;
    unread = true; // It is unread
    m_btnClear->setText(tr("&Clear All"));
    QString nextStr = QString(tr("&Next (%1)")).arg(m_nUnreadNum);
    m_btnNext->setText(nextStr);
    if (!m_btnNext->isEnabled())
    {
      m_btnNext->setEnabled(true);
      m_btnNext->show();
    }
    if (!m_btnMore->isEnabled())
    {
      m_btnMore->setEnabled(true);
      m_btnMore->show();
    }
  }

  MessageBoxItem* pEntry = new MessageBoxItem();
  // Resize the icon
  img = pix.toImage();
  if (img.width() > img.height())
    img = img.scaledToWidth(16);
  else
    img = img.scaledToHeight(16);
  QPixmap scaledPix(QPixmap::fromImage(img));
  // Add the columns now
  pEntry->setIcon(scaledPix);
  pEntry->setText(msg.left(qMin(50, msg.indexOf('\n')))); // Put this in setMessage()
  // Set the special data
  pEntry->setMessage(msg);
  pEntry->setFullIcon(pix);
  pEntry->setUnread(unread);
  pEntry->setType(type);

  m_lstMsg->insertItem(0, pEntry);

  // Set the caption if we set the text and icon here
  if (m_nUnreadNum == 0)
    updateCaption(pEntry);
}

/// ////////////////////////////////////////////////////////
/// @brief Toggline the detailed list view
///
/// When the more button is clicked, toggle showing the list view of
/// pending messages. When we hide the list view, we need to show the
/// oldest unread message.
/// ////////////////////////////////////////////////////////
void MessageBox::slot_toggleMore()
{
  showExtension(m_frmList->isHidden());
}

/// ////////////////////////////////////////////////////////
/// @brief Show the next pending message
///
/// When the next button is clicked, we will show the next pending
/// unread message. The text and icon will both be updated.
/// ////////////////////////////////////////////////////////
void MessageBox::slot_clickNext()
{
  // Find the next unread message
  bool bFound = false;
  MessageBoxItem* item = 0;
  for (int i = m_lstMsg->currentRow(); i >= 0; --i)
  {
    item = dynamic_cast<MessageBoxItem*>(m_lstMsg->item(i));
    if (item->isUnread())
    {
      bFound = true;
      break;
    }
  }

  // If no unread item was found, search from the bottom
  if (!bFound)
  {
    for (int i = m_lstMsg->count() - 1; i >= 0; --i)
    {
      item = dynamic_cast<MessageBoxItem*>(m_lstMsg->item(i));
      if (item->isUnread())
      {
        bFound = true;
        break;
      }
    }
  }

  // Only change the item if there something to change it to
  if (bFound)
    m_lstMsg->setCurrentItem(item);
}

/// ////////////////////////////////////////////////////////
/// @brief Clear all pending messages and close the message box
///
/// When the clear all button is clicked, we will close the dialog and
/// remove all messages from the queue.
/// ////////////////////////////////////////////////////////
void MessageBox::slot_clickClear()
{
  // Hide the window first
  hide();

  // Remove all items that have been read
  m_lstMsg->clear();
  m_nUnreadNum = 0;
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
void MessageBox::slot_listChanged(QListWidgetItem* i, QListWidgetItem* /* previous */)
{
  // Change the icon, message and caption
  MessageBoxItem* item = dynamic_cast<MessageBoxItem*>(i);
  if (item != NULL)
  {
    m_lblIcon->setPixmap(item->getFullIcon());
    m_lblMessage->setText(item->getMessage());
    updateCaption(item);

    // Mark it as read
    if (item->isUnread())
    {
      m_nUnreadNum--;
      item->setUnread(false);
    }
  }

  // Update the next button
  QString nextStr;
  if (m_nUnreadNum > 0)
    nextStr = QString(tr("&Next (%1)")).arg(m_nUnreadNum);
  else
  {
    // No more unread messages
    nextStr = QString(tr("&Next"));
    m_btnNext->setEnabled(false);
    m_nUnreadNum = 0;
  }
  m_btnNext->setText(nextStr);
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

  QString strCaption;
  switch (item->getType())
  {
    case QMessageBox::Information:
      strCaption = tr("Licq Information");
      break;

    case QMessageBox::Warning:
      strCaption = tr("Licq Warning");
      break;

    case QMessageBox::Critical:
      strCaption = tr("Licq Critical");
      break;

    case QMessageBox::NoIcon:
    case QMessageBox::Question:
    default:
      strCaption = tr("Licq");
      break;
  }
  setWindowTitle(strCaption);
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

MessageManager* MessageManager::m_pInstance = 0;

MessageManager::MessageManager()
  : m_pMsgDlg(0)
{
}

MessageManager::~MessageManager()
{
  if (m_pMsgDlg)
    delete m_pMsgDlg;
}

MessageManager* MessageManager::Instance()
{
  if (m_pInstance == 0)
    m_pInstance = new MessageManager;

  return m_pInstance;
}

void MessageManager::addMessage(QMessageBox::Icon type, const QString& msg,
  QWidget* parent)
{
  // We should pass parent to it, but it causes a crash after the parent closes
  // and we try to show another message box. I tried to reparent m_pMsgDlg, but
  // that didnt help much.
  // So for now.. we do this:
  parent = 0; // XXX See comment above
  if (m_pMsgDlg == 0)
    m_pMsgDlg = new MessageBox(parent);

  m_pMsgDlg->addMessage(type, msg);
  m_pMsgDlg->show();
}

MessageBoxItem::MessageBoxItem(QListWidget* parent)
  : QListWidgetItem(parent)
{
  m_unread = false;
}

void MessageBoxItem::setUnread(bool b)
{
  m_unread = b;
  if (m_unread)
    setForeground(Qt::red);
  else
    setForeground(Qt::black);
}
