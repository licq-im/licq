// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2010 Licq developers
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

#include "securitydlg.h"

#include "config.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QPushButton>
#include <QVBoxLayout>

#include <licq/contactlist/owner.h>
#include <licq/event.h>
#include <licq/icq.h>

#include "core/messagebox.h"
#include "core/signalmanager.h"

#include "helpers/support.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::SecurityDlg */

SecurityDlg::SecurityDlg(QWidget* parent)
  : QDialog(parent),
    title(tr("ICQ Security")),
    eSecurityInfo(0)
{
  Support::setWidgetProps(this, "SecurityDialog");
  setAttribute(Qt::WA_DeleteOnClose, true);
  setWindowTitle(title);

  Licq::OwnerReadGuard o(LICQ_PPID);
  if (!o.isLocked())
  {
    InformUser(this, tr("No ICQ owner found.\nPlease create one first."));
    close();
    return;
  }

  QVBoxLayout* top_lay = new QVBoxLayout(this);

  QGroupBox* boxOptions = new QGroupBox(tr("Options"));
  QVBoxLayout* layOptions = new QVBoxLayout(boxOptions);

#define ADD_CHECK(var, name, tip, status) \
  (var) = new QCheckBox((name)); \
  (var)->setChecked((status)); \
  (var)->setToolTip((tip)); \
  layOptions->addWidget((var));

  ADD_CHECK(chkAuthorization, tr("&Authorization Required"), tr("Determines "
        "whether regular ICQ clients require\nyour authorization to add you to "
        "their contact list."), o->GetAuthorization());
  ADD_CHECK(chkWebAware, tr("&Web Presence"), tr("Web Presence allows users to"
        " see\nif you are online through your web indicator."), o->WebAware());
  ADD_CHECK(chkHideIp, tr("&Hide IP"), tr("Hide IP stops users from seeing your"
        " IP address.\nIt doesn't guarantee it will be hidden though."),
      o->HideIp());
#undef ADD_CHECK

  top_lay->addWidget(boxOptions);

  QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Cancel);

  btnUpdate = buttons->addButton(QDialogButtonBox::Ok);
  btnUpdate->setText(tr("&Update"));

  connect(buttons, SIGNAL(accepted()), SLOT(ok()));
  connect(buttons, SIGNAL(rejected()), SLOT(close()));

  top_lay->addWidget(buttons);

  show();
}

void SecurityDlg::ok()
{
  bool auth, web, ip;
  bool changed;
  {
    Licq::OwnerReadGuard o(LICQ_PPID);

    if (!o.isLocked())
    {
      // Even though we protected from this in constructor,
      // it's never too bad to persuade
      close();
      return;
    }

    if (o->status() == Licq::User::OfflineStatus)
    {
      InformUser(this, tr("You need to be connected to the\n"
          "ICQ Network to change the settings."));
      return;
    }

    auth = chkAuthorization->isChecked();
    web = chkWebAware->isChecked();
    ip = chkHideIp->isChecked();

    changed = (auth != o->GetAuthorization() || web != o->WebAware() || ip != o->HideIp());
  }

  if (changed)
  {
    btnUpdate->setEnabled(false);

    connect(gGuiSignalManager, SIGNAL(doneUserFcn(const Licq::Event*)),
        SLOT(doneUserFcn(const Licq::Event*)));

    setWindowTitle(title + " [" + tr("Setting...") + "]");

    eSecurityInfo = gLicqDaemon->icqSetSecurityInfo(auth, ip, web);

    return; // prevents the dialog from closing
  }

  close();
}

void SecurityDlg::doneUserFcn(const Licq::Event* e)
{
  if (!e->Equals(eSecurityInfo))
    return;

  eSecurityInfo = 0;
  QString result = QString::null;
  btnUpdate->setEnabled(true);

  disconnect(gGuiSignalManager, SIGNAL(doneUserFcn(const Licq::Event*)),
      this, SLOT(doneUserFcn(const Licq::Event*)));

  switch (e->Result())
  {
    case Licq::Event::ResultFailed:
      result = tr("failed");
      InformUser(this, tr("Setting security options failed."));
      break;

    case Licq::Event::ResultTimedout:
      result = tr("timed out");
      InformUser(this, tr("Timeout while setting security options."));
      break;

    case Licq::Event::ResultError:
      result = tr("error");
      InformUser(this, tr("Internal error while setting security options."));
      break;

    default:
      break;
  }

  if (result.isEmpty())
    close();
  else
    setWindowTitle(title + " [" + tr("Setting...") + " " + result + "]");
}
