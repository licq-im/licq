// -*- c-basic-offset: 2 -*-
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

#include "editcategorydlg.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLineEdit>
#include <QTextCodec>

#include <licq_interestcodes.h>
#include <licq_organizationcodes.h>
#include <licq_backgroundcodes.h>

#include "helpers/usercodec.h"
#include "helpers/support.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::EditCategoryDlg */

EditCategoryDlg::EditCategoryDlg(ICQUserCategory* cat, QWidget* parent)
  : QDialog(parent),
    myUserCat(cat->GetCategory())
{
  Support::setWidgetProps(this, "EditCategoryDlg");
  setAttribute(Qt::WA_DeleteOnClose, true);

  QString title = "Licq - Edit @ Category";

  unsigned short tableSize, i = 0;

  switch (myUserCat)
  {
    case CAT_INTERESTS:
      myNumCats = MAX_CAT;
      getEntry = GetInterestByIndex;
      tableSize = NUM_INTERESTS;
      title.replace("@", tr("Personal Interests"));
      break;
    case CAT_ORGANIZATION:
      myNumCats = MAX_CAT - 1;
      getEntry = GetOrganizationByIndex;
      tableSize = NUM_ORGANIZATIONS;
      title.replace("@", tr("Organization, Affiliation, Group"));
      break;
    case CAT_BACKGROUND:
      myNumCats = MAX_CAT - 1;
      getEntry = GetBackgroundByIndex;
      tableSize = NUM_BACKGROUNDS;
      title.replace("@", tr("Past Background"));
      break;
    default:
      close();
      return;
  }

  setWindowTitle(title);

  QGridLayout* top_lay = new QGridLayout(this);

  for (; i < myNumCats ; i++)
  {
    myCats[i] = new QComboBox();
    myCats[i]->addItem(tr("Unspecified"));

    int selected = 0;
    const char* descr;
    unsigned short selection_id;
    if (!cat->Get(i, &selection_id, &descr))
    {
      selection_id = 0;
      descr = "";
    }

    for (unsigned short j = 0; j < tableSize ; j++)
    {
      myCats[i]->addItem(getEntry(j)->szName);
      if (getEntry(j)->nCode == selection_id)
        selected = j + 1;
    }

    myCats[i]->setCurrentIndex(selected);
    connect(myCats[i], SIGNAL(activated(int)), SLOT(checkEnabled()));
    top_lay->addWidget(myCats[i], i, 0);

    myDescr[i] = new QLineEdit();
    myDescr[i]->setMinimumWidth(300);
    myDescr[i]->setMaxLength(MAX_CATEGORY_SIZE);
    myDescr[i]->setText(descr);
    myDescr[i]->setEnabled(selection_id != 0);
    top_lay->addWidget(myDescr[i], i, 1);
  }

  QDialogButtonBox* buttons = new QDialogButtonBox(
      QDialogButtonBox::Ok |
      QDialogButtonBox::Cancel);
  connect(buttons, SIGNAL(accepted()), SLOT(ok()));
  connect(buttons, SIGNAL(rejected()), SLOT(close()));

  top_lay->setRowStretch(i++, 1);
  top_lay->addWidget(buttons, i++, 0, 1, 2);
  top_lay->setColumnStretch(1, 1);

  show();
}

void EditCategoryDlg::ok()
{
  const ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
  if (o != NULL)
  {
    QTextCodec* codec = UserCodec::codecForICQUser(o);
    gUserManager.DropOwner(o);

    ICQUserCategory* cat = new ICQUserCategory(myUserCat);
    for (unsigned short i = 0; i < myNumCats; i++)
    {
      if (myCats[i]->currentIndex() != 0)
        cat->AddCategory(getEntry(myCats[i]->currentIndex() - 1)->nCode,
            codec->fromUnicode(myDescr[i]->text()));
    }

    emit updated(cat);
  }
  close();
}

void EditCategoryDlg::checkEnabled()
{
  for (unsigned short i = 0; i < myNumCats; i++)
    myDescr[i]->setEnabled(myCats[i]->currentIndex() != 0);
}
