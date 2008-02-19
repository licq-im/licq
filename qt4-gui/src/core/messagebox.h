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
bool QueryUser(QWidget*, QString, QString, QString, bool bConfirmYes=false,
    QString szConfirm=NULL, bool bConfirmNo=false, QString szConfirmNo=NULL);
int QueryUser(QWidget*, QString, QString, QString, QString);
bool QueryYesNo(QWidget* parent, QString query);
void InformUser(QWidget* q, QString);
void WarnUser(QWidget* q, QString szInfo);
void CriticalUser(QWidget* q, QString szInfo);

class MessageBoxItem : public QListWidgetItem
{
public:
  MessageBoxItem(QListWidget* parent = 0);

  void setMessage(const QString& s) { m_msg = s; }
  void setFullIcon(const QPixmap& p) { m_fullIcon = p; }
  void setUnread(bool b);
  void setType(QMessageBox::Icon t) { m_type = t; }

  QString getMessage() const { return m_msg; }
  QPixmap getFullIcon() const { return m_fullIcon; }
  bool isUnread() const { return m_unread; }
  QMessageBox::Icon getType() const { return m_type; }

private:
  QString m_msg;
  QPixmap m_fullIcon;
  bool m_unread;
  QMessageBox::Icon m_type;
};

class MessageBox : public QDialog
{
  Q_OBJECT
public:
  MessageBox(QWidget* parent = 0);
  void addMessage(QMessageBox::Icon type, const QString& msg);

public slots:
  void slot_toggleMore();
  void slot_clickNext();
  void slot_clickClear();
  void slot_listChanged(QListWidgetItem* current, QListWidgetItem* previous);

private:
  QPixmap getMessageIcon(QMessageBox::Icon);
  void updateCaption(MessageBoxItem*);

  int m_nUnreadNum;
  QLabel* m_lblIcon;
  QLabel* m_lblMessage;
  QPushButton* m_btnNext;
  QPushButton* m_btnMore;
  QPushButton* m_btnClear;
  QListWidget* m_lstMsg;
  QFrame* m_frmList;
};

class MessageManager
{
public:
  ~MessageManager();

  static MessageManager* Instance();

  void addMessage(QMessageBox::Icon type, const QString& msg, QWidget* p);

private:
  MessageManager();
  MessageManager(const MessageManager&);
  MessageManager& operator=(const MessageManager&);

  static MessageManager* m_pInstance;
  MessageBox* m_pMsgDlg;
};

} // namespace LicqQtGui

#endif
