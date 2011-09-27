/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2004-2011 Licq developers
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

#include "phonedlg.h"

#include <QByteArray>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QTextCodec>

#include <licq/contactlist/owner.h>
#include <licq/icq/codes.h>

#include "config/iconmanager.h"

#include "core/messagebox.h"

#include "helpers/support.h"
#include "helpers/usercodec.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::EditPhoneDlg */

EditPhoneDlg::EditPhoneDlg(QWidget* parent, const struct Licq::PhoneBookEntry* pbe,
    int nEntry)
  : QDialog(parent)
{
  Support::setWidgetProps(this, "EditPhoneDlg");
  setAttribute(Qt::WA_DeleteOnClose, true);
  setModal(true);

  const QTextCodec* codec;
  {
    Licq::OwnerReadGuard o(LICQ_PPID);
    if (!o.isLocked())
    {
      close();
      return;
    }
    codec = UserCodec::codecForUser(*o);
  }

  m_nEntry = nEntry;

  QGridLayout* top_lay = new QGridLayout(this);

  // ROW 1
  top_lay->addWidget(new QLabel(tr("Description:")), 0, 0);

  cmbDescription = new QComboBox();
  cmbDescription->setEditable(true);
  cmbDescription->addItem(tr("Home Phone"));
  cmbDescription->addItem(tr("Work Phone"));
  cmbDescription->addItem(tr("Private Cellular"));
  cmbDescription->addItem(tr("Work Cellular"));
  cmbDescription->addItem(tr("Home Fax"));
  cmbDescription->addItem(tr("Work Fax"));
  cmbDescription->addItem(tr("Wireless Pager"));
  cmbDescription->lineEdit()->setMaxLength(Licq::MAX_DESCRIPTION_SIZE);
  cmbDescription->setDuplicatesEnabled(false);
  top_lay->addWidget(cmbDescription, 0, 1);

  // ROW 2
  top_lay->addWidget(new QLabel(tr("Type:")), 1, 0);

  IconManager* iconman = IconManager::instance();

  cmbType = new QComboBox();
  cmbType->addItem(iconman->getIcon(IconManager::PSTNIcon), tr("Phone"));
  cmbType->addItem(iconman->getIcon(IconManager::MobileIcon), tr("Cellular"));
  cmbType->addItem(iconman->getIcon(IconManager::SMSIcon), tr("Cellular SMS"));
  cmbType->addItem(iconman->getIcon(IconManager::FaxIcon), tr("Fax"));
  cmbType->addItem(iconman->getIcon(IconManager::PagerIcon), tr("Pager"));
  top_lay->addWidget(cmbType, 1, 1);

  // ROW 3
  top_lay->addWidget(new QLabel(tr("Country:")), 2, 0);

  cmbCountry = new QComboBox();
  for (unsigned short i = 0; i < NUM_COUNTRIES; i++)
    cmbCountry->addItem(GetCountryByIndex(i)->szName);
  top_lay->addWidget(cmbCountry, 2, 1);

  // ROWS 4-5
  QGridLayout* gdlay = new QGridLayout();
  top_lay->addLayout(gdlay, 3, 0, 1, 2);

  gdlay->addWidget(new QLabel(tr("Network #/Area code:")), 0, 0);

  leAreaCode = new QLineEdit();
  leAreaCode->setMaxLength(Licq::MAX_AREAxCODE_SIZE);
  gdlay->addWidget(leAreaCode, 1, 0);

  gdlay->addWidget(new QLabel(tr("Number:")), 0, 1);

  leNumber = new QLineEdit();
  leNumber->setMaxLength(Licq::MAX_PHONExNUMBER_SIZE);
  gdlay->addWidget(leNumber, 1, 1);

  gdlay->addWidget(new QLabel(tr("Extension:")), 0, 2);

  leExtension = new QLineEdit();
  leExtension->setMaxLength(Licq::MAX_EXTENSION_SIZE);
  gdlay->addWidget(leExtension, 1, 2);

  // ROW 6
  top_lay->addWidget(new QLabel(tr("Provider:")), 4, 0);

  cmbProvider = new QComboBox();
  cmbProvider->addItem(tr("Custom"));
  for (unsigned short i = 0; i < NUM_PROVIDERS; i++)
    cmbProvider->addItem(GetProviderByIndex(i)->szName);
  top_lay->addWidget(cmbProvider, 4, 1);

  // ROW 7
  top_lay->addWidget(new QLabel(tr("E-mail Gateway:")), 5, 0);

  leGateway = new QLineEdit();
  leGateway->setMaxLength(Licq::MAX_GATEWAY_SIZE);
  top_lay->addWidget(leGateway, 5, 1);

  // ROW 8
  cbRemove0s = new QCheckBox(tr("Remove leading 0s from Area Code/Network #"));
  top_lay->addWidget(cbRemove0s, 6, 0, 1, 2);

  // ROW 9
  QDialogButtonBox* buttons = new QDialogButtonBox();
  top_lay->addWidget(buttons, 8, 0, 1, 2);

  QPushButton* btnOk = buttons->addButton(QDialogButtonBox::Ok);
  btnOk->setText(tr("&OK"));
  connect(buttons, SIGNAL(accepted()), SLOT(ok()));

  QPushButton* btnCancel = buttons->addButton(QDialogButtonBox::Cancel);
  btnCancel->setText(tr("&Cancel"));
  connect(buttons, SIGNAL(rejected()), SLOT(close()));

  top_lay->setRowStretch(7, 1);


  if (pbe)
  {
    cmbDescription->addItem(codec->toUnicode(pbe->description.c_str()));
    cmbDescription->setCurrentIndex(cmbDescription->count() - 1);
    cmbType->setCurrentIndex(pbe->nType);
    const struct SCountry* c = GetCountryByName(pbe->country.c_str());
    if (c)
      cmbCountry->setCurrentIndex(c->nIndex);
    leAreaCode->setText(codec->toUnicode(pbe->areaCode.c_str()));
    leNumber->setText(codec->toUnicode(pbe->phoneNumber.c_str()));
    // avoid duplicating the pager number in the extension field
    if (pbe->nType != Licq::TYPE_PAGER || pbe->phoneNumber != pbe->extension)
    {
      leExtension->setText(codec->toUnicode(pbe->extension.c_str()));
    }
    if (pbe->nGatewayType == Licq::GATEWAY_BUILTIN)
    {
      const struct SProvider* p = GetProviderByName(pbe->gateway.c_str());
      if (p)
        cmbProvider->setCurrentIndex(p->nIndex + 1);
      else if (!pbe->gateway.empty())
        leGateway->setText(codec->toUnicode(pbe->gateway.c_str()));
      else
        leGateway->setText(tr("@"));
    }
    else
      leGateway->setText(codec->toUnicode(pbe->gateway.c_str()));

    cbRemove0s->setChecked(pbe->nRemoveLeading0s);
  }
  else
  {
    cbRemove0s->setChecked(true);
    leGateway->setText(tr("@"));
  }

  connect(cmbType, SIGNAL(activated(int)), SLOT(UpdateDlg(int)));
  connect(cmbProvider, SIGNAL(activated(int)), SLOT(ProviderChanged(int)));

  UpdateDlg(cmbType->currentIndex());
}

//------------------------------------------------------------------------------

void EditPhoneDlg::UpdateDlg(int nType)
{
  leExtension->setEnabled(nType == Licq::TYPE_PHONE);
  cmbProvider->setEnabled(nType == Licq::TYPE_PAGER);
  leGateway->setEnabled(nType == Licq::TYPE_PAGER && cmbProvider->currentIndex() == 0);
  cbRemove0s->setEnabled(nType != Licq::TYPE_PAGER);
  leAreaCode->setEnabled(nType != Licq::TYPE_PAGER);
  cmbCountry->setEnabled(nType != Licq::TYPE_PAGER);
}

//------------------------------------------------------------------------------

void EditPhoneDlg::ProviderChanged(int nIndex)
{
  if (nIndex == 0 && leGateway->text().isEmpty())
    leGateway->setText(tr("@"));

  UpdateDlg(cmbType->currentIndex());
}

//------------------------------------------------------------------------------

void EditPhoneDlg::ok()
{
  //Must at least have specified a phone number
  if (leNumber->text().length() == 0)
  {
    WarnUser(this, tr("Please enter a phone number"));
    return;
  }

  const QTextCodec* codec;
  {
    Licq::OwnerReadGuard o(LICQ_PPID);
    if (!o.isLocked())
    {
      close();
      return;
    }
    codec = UserCodec::codecForUser(*o);
  }

  struct Licq::PhoneBookEntry pbe;
  memset(&pbe, 0, sizeof(pbe));

  pbe.description = codec->fromUnicode(cmbDescription->currentText()).data();

  if (leAreaCode->isEnabled())
    pbe.areaCode = codec->fromUnicode(leAreaCode->text()).data();
  else
    pbe.areaCode = "";

  pbe.phoneNumber = codec->fromUnicode(leNumber->text()).data();

  pbe.nType = cmbType->currentIndex();

  if (leExtension->isEnabled())
    pbe.extension = codec->fromUnicode(leExtension->text()).data();
  else if (pbe.nType == Licq::TYPE_PAGER)
    // need to store the number in extension as well for some reason
    pbe.extension = pbe.phoneNumber;
  else
    pbe.extension = "";

  if (cmbCountry->isEnabled() && cmbCountry->currentIndex() != 0)
    pbe.country = codec->fromUnicode(cmbCountry->currentText()).data();
  else
    pbe.country = "";

  if (leGateway->isEnabled())
  {
    pbe.gateway = codec->fromUnicode(leGateway->text()).data();
    pbe.nGatewayType = Licq::GATEWAY_CUSTOM;
  }
  else if (cmbProvider->isEnabled())
  {
    pbe.gateway = codec->fromUnicode(cmbProvider->currentText()).data();
    pbe.nGatewayType = Licq::GATEWAY_BUILTIN;
  }
  else
  {
    pbe.gateway = "";
    pbe.nGatewayType = Licq::GATEWAY_BUILTIN;
  }

  pbe.nSmsAvailable = (pbe.nType == Licq::TYPE_CELLULARxSMS) ? 1 : 0;

  if (cbRemove0s->isEnabled() && !cbRemove0s->isChecked())
    pbe.nRemoveLeading0s = 0;
  else
    pbe.nRemoveLeading0s = 1;

  emit updated(pbe, m_nEntry);
  close();
}
