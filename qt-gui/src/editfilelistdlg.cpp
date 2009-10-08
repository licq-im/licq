// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2003-2009 Licq developers
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qlayout.h>

#include "editfilelistdlg.h"

CEditFileListDlg::CEditFileListDlg(ConstFileList *_lFileList, QWidget *parent)
  : LicqDialog(parent, "EditFileListDlg", true, WStyle_ContextHelp |
                                              WDestructiveClose)
{ 
  QGridLayout *lay = new QGridLayout(this, 5, 2, 10, 5);
  
  m_lFileList = _lFileList;
  
  setCaption(tr("Licq - Files to send"));

  lstFiles= new QListBox(this);
  lstFiles->setFixedWidth(400);
  lay->addMultiCellWidget(lstFiles, 0, 4, 0, 0, Qt::AlignHCenter);

  btnDone    = new QPushButton(tr("&Done"), this);
  btnUp      = new QPushButton(tr("&Up"), this);
  btnDown    = new QPushButton(tr("&Down"), this);
  btnDel     = new QPushButton(tr("&Delete"), this);

  lay->setColStretch(0, 100);
  lay->addWidget(btnDone, 0, 1);
  lay->addWidget(btnUp  , 2, 1);
  lay->addWidget(btnDown, 3, 1);
  lay->addWidget(btnDel , 4, 1);

  connect(btnDone, SIGNAL(clicked()), SLOT(slot_done()));
  connect(btnUp, SIGNAL(clicked()), SLOT(slot_up()));
  connect(btnDown, SIGNAL(clicked()), SLOT(slot_down()));
  connect(btnDel, SIGNAL(clicked()), SLOT(slot_del()));
  connect(lstFiles, SIGNAL(currentChanged(QListBoxItem *)),
           this, SLOT(slot_selectionChanged(QListBoxItem *)));

  // populate the listview
  RefreshList();
  
  show();
}

CEditFileListDlg::~CEditFileListDlg()
{
}

void CEditFileListDlg::RefreshList()
{
  ConstFileList::iterator it;
  
  lstFiles->clear();
  btnUp->setEnabled(false);
  btnDown->setEnabled(false);
  btnDel->setEnabled(false);
  
  for (it = m_lFileList->begin(); it != m_lFileList->end(); it++ )
    lstFiles->insertItem(QString::fromLocal8Bit(*it), -1);
}

void CEditFileListDlg::slot_selectionChanged(QListBoxItem *item)
{ 
  unsigned n = lstFiles->currentItem();
  
  if (item == 0)
    return;

  btnUp->setEnabled(n != 0);
  btnDown->setEnabled(n != lstFiles->count() - 1);
  btnDel->setEnabled(true);
}

void CEditFileListDlg::slot_done()
{
  close();
}

void CEditFileListDlg::slot_up()
{ 
  ConstFileList::iterator it;
  QString s;
  
  int n = lstFiles->currentItem();
  int i = 0;

  if( n  == 0 )
    return;
  
  s = lstFiles->text(n);
  lstFiles->removeItem(n);
  lstFiles->insertItem(s, n - 1);
  lstFiles->setCurrentItem(n - 1);

  for (it = m_lFileList->begin(); i != n && it != m_lFileList->end(); it++, i++)
      ;

  if (i == n)
  {  
    const char *s = *it;

    it = m_lFileList->erase(it);
    it--;
    m_lFileList->insert(it, s);
  }
  
}

void CEditFileListDlg::slot_down()
{
  ConstFileList::iterator it;
  QString s;
  int n = lstFiles->currentItem(); 
  int i = 0;
  
  if (n == (int)lstFiles->count() - 1) 
    return;

  s = lstFiles->text(n);
  lstFiles->removeItem(n);
  lstFiles->insertItem(s, n + 1);
  lstFiles->setCurrentItem(n + 1);

  for (it = m_lFileList->begin(); i != n && it != m_lFileList->end(); it++, i++)
      ;

  if (i == n)
  {
    const char *s = *it;

    it = m_lFileList->erase(it);
    it++;
    m_lFileList->insert(it, s);
  }
}

void CEditFileListDlg::slot_del()
{
  ConstFileList::iterator it;
  int n = lstFiles->currentItem();
  int i = 0;

  for (it = m_lFileList->begin(); i != n && it != m_lFileList->end(); it++, i++)
    ;

  if (i == n)
  {  
     free((void*)*it);
     m_lFileList->erase(it);
     emit file_deleted(m_lFileList->size());
  }

  lstFiles->setCurrentItem(n);
  lstFiles->removeItem(n);
  
}

#include "editfilelistdlg.moc"
