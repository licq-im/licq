/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2004-2006 Licq developers
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

#include <qlineedit.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qapplication.h>
#include <qwidget.h>
#include <qtextcodec.h>

#include "catdlg.h"
#include "usercodec.h"
#include "licq_interestcodes.h"
#include "licq_organizationcodes.h"
#include "licq_backgroundcodes.h"


EditCategoryDlg::EditCategoryDlg(QWidget *parent, ICQUserCategory *uCat)
  : QDialog(parent, "EditCategoryDlg", false, WDestructiveClose)
{
  QBoxLayout *top_lay, *ll, *l;

  m_uc = uCat->GetCategory();

  unsigned short nTableSize;

  switch (m_uc)
  {
  case CAT_INTERESTS:
    m_nCats = MAX_CAT;
    m_fGetEntry = GetInterestByIndex;
    nTableSize = NUM_INTERESTS;
    break;
  case CAT_ORGANIZATION:
    m_nCats = MAX_CAT - 1;
    m_fGetEntry = GetOrganizationByIndex;
    nTableSize = NUM_ORGANIZATIONS;
    break;
  case CAT_BACKGROUND:
    m_nCats = MAX_CAT - 1;
    m_fGetEntry = GetBackgroundByIndex;
    nTableSize = NUM_BACKGROUNDS;
    break;
  default:
    close();
    return;
  }

  top_lay = new QVBoxLayout(this, 10);

  for(unsigned short i = 0 ; i < m_nCats ; i++ )
  {
    ll = new QHBoxLayout(top_lay, 10);
    cbCat[i] = new QComboBox(this);
    cbCat[i]->insertItem(tr("Unspecified"), 0);
    int selected = 0;
    const char *descr;
    unsigned short selection_id;
    if (!uCat->Get(i, &selection_id, &descr))
    {
      selection_id = 0;
      descr = "";
    }
    for(unsigned short j = 0; j < nTableSize ; j++ )
    {
      cbCat[i]->insertItem(m_fGetEntry(j)->szName);
      if (m_fGetEntry(j)->nCode == selection_id)
        selected = j + 1;
    }
    connect(cbCat[i], SIGNAL(activated(int)), SLOT(checkEnabled(int)));
    leDescr[i] = new QLineEdit(this);
    leDescr[i]->setMinimumWidth(430);
    leDescr[i]->setMaxLength(MAX_CATEGORY_SIZE);
    ll->addWidget(cbCat[i]);
    ll->addWidget(leDescr[i]);
    cbCat[i]->setCurrentItem(selected);
    leDescr[i]->setText(descr);
    leDescr[i]->setEnabled(selection_id != 0);
  }

  QPushButton *btnOk = new QPushButton(tr("&OK"), this);
  btnOk->setDefault(true);
  connect(btnOk, SIGNAL(clicked()), SLOT(ok()));

  QPushButton *btnCancel = new QPushButton(tr("&Cancel"), this);
  connect(btnCancel, SIGNAL(clicked()), SLOT(close()));

  l = new QHBoxLayout(top_lay, 10);
  l->addStretch(1);
  l->addWidget(btnOk);
  l->addWidget(btnCancel);
}

//------------------------------------------------------------------------------

void EditCategoryDlg::ok()
{
  ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
  if (o == NULL)
  {
    close();
    return;
  }
  QTextCodec *codec = UserCodec::codecForICQUser(o);
  gUserManager.DropOwner(o);

  ICQUserCategory *cat = new ICQUserCategory(m_uc);
  for (unsigned short i = 0; i < m_nCats; i++)
  {
    if (cbCat[i]->currentItem() != 0)
    {
      cat->AddCategory(m_fGetEntry(cbCat[i]->currentItem() - 1)->nCode,
                       codec->fromUnicode(leDescr[i]->text()));
    }
  }

  emit updated(cat);
  close();
}

//------------------------------------------------------------------------------

void EditCategoryDlg::checkEnabled(int /* index */)
{
  for (unsigned short i = 0; i < m_nCats; i++)
    leDescr[i]->setEnabled(cbCat[i]->currentItem() != 0);
}

//------------------------------------------------------------------------------


#include "catdlg.moc"
