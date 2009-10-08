// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2009 Licq developers
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

#include "joinchatdlg.h"

#include "config.h"

#include <QGridLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>

#include "helpers/support.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::JoinChatDlg */

JoinChatDlg::JoinChatDlg(bool bRequesting, QWidget* p)
  : QDialog(p)
{
  Support::setWidgetProps(this, "ChatJoinDialog");
  setModal(true);

  QGridLayout* lay = new QGridLayout(this);

  QLabel* l = new QLabel();
  lay->addWidget(l, 0, 0, 1, 5);

  lstChats = new QListWidget();
  lay->addWidget(lstChats, 1, 0, 1, 5);

  lay->setColumnStretch(0, 2);
  btnOk = new QPushButton();
  lay->addWidget(btnOk, 2, 1);

  lay->setColumnMinimumWidth(2, 10);
  btnCancel = new QPushButton();
  lay->addWidget(btnCancel, 2, 3);
  lay->setColumnStretch(4, 2);

  if (bRequesting)
  {
    l->setText(tr("Select chat to invite:"));
    setWindowTitle(tr("Invite to Join Chat"));
    btnOk->setText(tr("&Invite"));
    btnCancel->setText(tr("&Cancel"));
  }
  else
  {
    l->setText(tr("Select chat to join:"));
    setWindowTitle(tr("Join Multiparty Chat"));
    btnOk->setText(tr("&Join"));
    btnCancel->setText(tr("&Cancel"));
  }

  connect(btnOk, SIGNAL(clicked()), SLOT(slot_ok()));
  connect(btnCancel, SIGNAL(clicked()), SLOT(reject()));

  // Fill in the combo box
  ChatDlgList::iterator iter;
  for (iter = ChatDlg::chatDlgs.begin();
       iter != ChatDlg::chatDlgs.end(); iter++)
  {
  /*
    QString n;
    if ((*iter)->chatUser == NULL)  // check if the first user closed already
    {
      n.setNum((*iter)->m_nUin);
    }
    else
    {
      // This is bad as the user is not locked at this point...but
      // should work as the name is never changed
      n = (*iter)->chatUser->Name();
      if (n.isEmpty()) n.setNum((*iter)->chatUser->Uin());
    }
    QString c = (*iter)->ChatClients();
    if (!c.isEmpty())
    {
      n += " (" + c + ")";
    }*/
    QString n = (*iter)->ChatClients();
    lstChats->addItem(n);

    originalChats.push_back(*iter);
  }
  lstChats->setCurrentItem(0);
}


void JoinChatDlg::slot_ok()
{
  if (lstChats->currentItem() == NULL)
    return;
  accept();
}


ChatDlg* JoinChatDlg::JoinedChat()
{
  if (lstChats->currentItem() == NULL)
    return NULL;

  unsigned short n = 0;
  ChatDlgList::iterator iter;
  for (iter = originalChats.begin();
       iter != originalChats.end() && n < lstChats->currentRow();
       iter++, n++)
  {
    // Empty
  }

  ChatDlg* cd = *iter;

  for (iter = ChatDlg::chatDlgs.begin();
       iter != ChatDlg::chatDlgs.end() && *iter != cd;
       iter++)
  {
    // Empty
  }

  // Check that this chat still exists
  if (iter == ChatDlg::chatDlgs.end())
    return NULL;

  return cd;
}
