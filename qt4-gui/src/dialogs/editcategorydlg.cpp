/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2004-2013 Licq developers <licq-dev@googlegroups.com>
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

#include <licq/icq/icqdata.h>
#include <licq/icq/user.h>
#include <licq/plugin/pluginmanager.h>

#include "helpers/support.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::EditCategoryDlg */

EditCategoryDlg::EditCategoryDlg(Licq::UserCat cat, const Licq::UserCategoryMap& category, QWidget* parent)
  : QDialog(parent),
    myUserCat(cat)
{
  Support::setWidgetProps(this, "EditCategoryDlg");
  setAttribute(Qt::WA_DeleteOnClose, true);

  QString title = "Licq - Edit @ Category";

  Licq::IcqData::Ptr icq = plugin_internal_cast<Licq::IcqData>(
      Licq::gPluginManager.getProtocolPlugin(ICQ_PPID));
  if (!icq)
  {
    close();
    return;
  }

  unsigned short tableSize;

  switch (myUserCat)
  {
    case Licq::CAT_INTERESTS:
      myNumCats = Licq::MAX_CATEGORIES;
      myIcqCategoryType = Licq::IcqCatTypeInterest;
      tableSize = Licq::NUM_INTERESTS;
      title.replace("@", tr("Personal Interests"));
      break;
    case Licq::CAT_ORGANIZATION:
      myNumCats = Licq::MAX_CATEGORIES - 1;
      myIcqCategoryType = Licq::IcqCatTypeOrganization;
      tableSize = Licq::NUM_ORGANIZATIONS;
      title.replace("@", tr("Organization, Affiliation, Group"));
      break;
    case Licq::CAT_BACKGROUND:
      myNumCats = Licq::MAX_CATEGORIES - 1;
      myIcqCategoryType = Licq::IcqCatTypeBackground;
      tableSize = Licq::NUM_BACKGROUNDS;
      title.replace("@", tr("Past Background"));
      break;
    default:
      close();
      return;
  }

  setWindowTitle(title);

  QGridLayout* top_lay = new QGridLayout(this);

  int i = 0;
  Licq::UserCategoryMap::const_iterator it = category.begin();
  for (; i < myNumCats ; i++)
  {
    myCats[i] = new QComboBox();
    myCats[i]->addItem(tr("Unspecified"));

    int selected = 0;
    unsigned short selection_id;
    QString descr;
    if (it != category.end())
    {
      selection_id = it->first;
      descr = QString::fromUtf8(it->second.c_str());
      ++it;
    }
    else
    {
      selection_id = 0;
      descr = "";
    }

    for (int j = 0; j < tableSize ; j++)
    {
      const struct Licq::IcqCategory* icqcat = icq->getCategoryByIndex(myIcqCategoryType, j);
      myCats[i]->addItem(QString::fromUtf8(icqcat->szName));
      if (icqcat->nCode == selection_id)
        selected = j + 1;
    }

    myCats[i]->setCurrentIndex(selected);
    connect(myCats[i], SIGNAL(activated(int)), SLOT(checkEnabled()));
    top_lay->addWidget(myCats[i], i, 0);

    myDescr[i] = new QLineEdit();
    myDescr[i]->setMinimumWidth(300);
    myDescr[i]->setMaxLength(Licq::MAX_CATEGORY_SIZE);
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
  Licq::IcqData::Ptr icq = plugin_internal_cast<Licq::IcqData>(
      Licq::gPluginManager.getProtocolPlugin(ICQ_PPID));
  if (!icq)
  {
    close();
    return;
  }

  Licq::UserCategoryMap cat;
  for (unsigned short i = 0; i < myNumCats; i++)
  {
    if (myCats[i]->currentIndex() != 0)
    {
      const struct Licq::IcqCategory* icqcat = icq->getCategoryByIndex(myIcqCategoryType, myCats[i]->currentIndex() - 1);
      cat[icqcat->nCode] = myDescr[i]->text().toUtf8().constData();
    }
  }

  emit updated(myUserCat, cat);

  close();
}

void EditCategoryDlg::checkEnabled()
{
  for (unsigned short i = 0; i < myNumCats; i++)
    myDescr[i]->setEnabled(myCats[i]->currentIndex() != 0);
}
