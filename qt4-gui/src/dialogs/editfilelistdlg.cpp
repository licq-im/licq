// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2003-2010 Licq developers
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

#include "editfilelistdlg.h"

#include "config.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>

#include "helpers/support.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::EditFileListDlg */

EditFileListDlg::EditFileListDlg(std::list<std::string>* fileList, QWidget* parent)
  : QDialog(parent),
    myFileList(fileList)
{
  Support::setWidgetProps(this, "EditFileListDlg");
  setAttribute(Qt::WA_DeleteOnClose, true);
  setWindowTitle(tr("Licq - Files to send"));
  setModal(true);

  QHBoxLayout* lay = new QHBoxLayout(this);

  lstFiles = new QListWidget();
  lstFiles->setMinimumWidth(400);
  lay->addWidget(lstFiles);

  QDialogButtonBox* buttonBox = new QDialogButtonBox(Qt::Vertical);

  btnDone = buttonBox->addButton(QDialogButtonBox::Ok);
  btnDone->setText(tr("D&one"));
  connect(btnDone, SIGNAL(clicked()), SLOT(close()));

  btnUp = new QPushButton(tr("&Up"));
  btnDown = new QPushButton(tr("&Down"));
  btnDelete = new QPushButton(tr("D&elete"));

  buttonBox->addButton(btnUp, QDialogButtonBox::ActionRole);
  buttonBox->addButton(btnDown, QDialogButtonBox::ActionRole);
  buttonBox->addButton(btnDelete, QDialogButtonBox::ActionRole);

  connect(btnUp, SIGNAL(clicked()), SLOT(up()));
  connect(btnDown, SIGNAL(clicked()), SLOT(down()));
  connect(btnDelete, SIGNAL(clicked()), SLOT(remove()));

  lay->addWidget(buttonBox);

  connect(lstFiles, SIGNAL(currentRowChanged(int)), SLOT(currentChanged(int)));

  refreshList();

  show();
}

void EditFileListDlg::refreshList()
{
  std::list<std::string>::iterator it = myFileList->begin();

  lstFiles->clear();

  for (; it != myFileList->end(); it++)
    lstFiles->addItem(QString::fromLocal8Bit(it->c_str()));
}

void EditFileListDlg::moveCurrentItem(bool up)
{
  int i = 0;
  int n = lstFiles->currentRow();
  int newRow = up ? n - 1 : n + 1;
  std::list<std::string>::iterator it = myFileList->begin();

  if ((up && n == 0) || (!up && n == lstFiles->count() - 1))
    return;

  for (; i != n && it != myFileList->end(); it++, i++)
    ;

  if (i == n)
  {
    const std::string& s = *it;

    it = myFileList->erase(it);
    myFileList->insert(up ? --it : ++it, s);
  }

  lstFiles->insertItem(newRow, lstFiles->takeItem(n));
  lstFiles->setCurrentRow(newRow);
}

void EditFileListDlg::currentChanged(int newCurrent)
{
  btnUp->setEnabled(newCurrent > 0);
  btnDown->setEnabled(newCurrent >= 0 && newCurrent < lstFiles->count() - 1);
  btnDelete->setEnabled(newCurrent != -1);
}

void EditFileListDlg::up()
{
  moveCurrentItem(true);
}

void EditFileListDlg::down()
{
  moveCurrentItem(false);
}

void EditFileListDlg::remove()
{
  int i = 0;
  int n = lstFiles->currentRow();
  std::list<std::string>::iterator it = myFileList->begin();

  for (; i != n && it != myFileList->end(); it++, i++)
    ;

  if (i == n)
  {
    myFileList->erase(it);
    emit fileDeleted(myFileList->size());
  }

  delete lstFiles->takeItem(n--);
  lstFiles->setCurrentRow(n);
  currentChanged(n);
}
