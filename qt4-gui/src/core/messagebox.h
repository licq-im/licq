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

#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <QListWidget>
#include <QMessageBox>
#include <QPixmap>

class QLabel;
class QPushButton;

class CUserEvent;
class ICQEvent;
class ICQUser;

namespace LicqQtGui
{
bool QueryUser(QWidget* q, QString query, QString button1, QString button2,
    bool confirmYes = false, QString confirmYesText = NULL,
    bool confirmNo = false, QString confirmNoText = NULL);
int QueryUser(QWidget* parent, QString query, QString button1, QString button2,
    QString button3);
bool QueryYesNo(QWidget* parent, QString query);
void InformUser(QWidget* parent, QString text);
void WarnUser(QWidget* parent, QString text);
void CriticalUser(QWidget* parent, QString text);


class MessageBoxItem : public QListWidgetItem
{
public:
  MessageBoxItem(QListWidget* parent = 0);

  void setMessage(const QString& s) { myMessage = s; }
  void setFullIcon(const QPixmap& p) { myFullIcon = p; }
  void setUnread(bool unread);
  void setType(QMessageBox::Icon t) { myType = t; }

  QString getMessage() const { return myMessage; }
  QPixmap getFullIcon() const { return myFullIcon; }
  bool isUnread() const { return myUnread; }
  QMessageBox::Icon getType() const { return myType; }

private:
  QString myMessage;
  QPixmap myFullIcon;
  bool myUnread;
  QMessageBox::Icon myType;
};

class MessageBox : public QDialog
{
  Q_OBJECT
public:
  static void addMessage(QMessageBox::Icon type, const QString& msg, QWidget* p);

public slots:
  void showNext();
  void closeDialog();
  void updateCurrentMessage(QListWidgetItem* current, QListWidgetItem* previous);

private:
  MessageBox(QWidget* parent = 0);
  ~MessageBox();
  void addMessage(QMessageBox::Icon type, const QString& msg);

  QPixmap getMessageIcon(QMessageBox::Icon);
  void updateCaption(MessageBoxItem*);

  static MessageBox* myMessageDialog;

  int myUnreadCount;
  QLabel* myIconLabel;
  QLabel* myMessageLabel;
  QPushButton* myNextButton;
  QPushButton* myMoreButton;
  QPushButton* myCloseButton;
  QListWidget* myMessageList;
  QWidget* myExtension;
};

} // namespace LicqQtGui

#endif
