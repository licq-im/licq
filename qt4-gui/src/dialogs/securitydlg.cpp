// -*- c-basic-offset: 2 -*-
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

#include "securitydlg.h"

#include "config.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QPushButton>
#include <QVBoxLayout>

#include <licq_events.h>
#include <licq_icqd.h>
#include <licq_user.h>

#include "core/licqgui.h"
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

  const ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
  if (o == NULL)
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

  gUserManager.DropOwner(o);

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
  const ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);

  if (o == NULL)
  {
    // Even though we protected from this in constructor,
    // it's never too bad to persuade
    close();
    return;
  }

  if (o->Status() == ICQ_STATUS_OFFLINE)
  {
    gUserManager.DropOwner(o);
    InformUser(this, tr("You need to be connected to the\n"
          "ICQ Network to change the settings."));
    return;
  }

  bool auth = chkAuthorization->isChecked();
  bool web = chkWebAware->isChecked();
  bool ip = chkHideIp->isChecked();

  if (auth != o->GetAuthorization() ||
      web != o->WebAware() ||
      ip != o->HideIp())
  {
    gUserManager.DropOwner(o);
    btnUpdate->setEnabled(false);

    connect(LicqGui::instance()->signalManager(),
        SIGNAL(doneUserFcn(ICQEvent*)), SLOT(doneUserFcn(ICQEvent*)));

    setWindowTitle(title + " [" + tr("Setting...") + "]");

    eSecurityInfo = gLicqDaemon->icqSetSecurityInfo(auth, ip, web);

    return; // prevents the dialog from closing
  }

  gUserManager.DropOwner(o);

  close();
}

void SecurityDlg::doneUserFcn(ICQEvent* e)
{
  if (!e->Equals(eSecurityInfo))
    return;

  eSecurityInfo = 0;
  QString result = QString::null;
  btnUpdate->setEnabled(true);

  disconnect(LicqGui::instance()->signalManager(),
      SIGNAL(doneUserFcn(ICQEvent*)), this, SLOT(doneUserFcn(ICQEvent*)));

  switch (e->Result())
  {
    case EVENT_FAILED:
      result = tr("failed");
      InformUser(this, tr("Setting security options failed."));
      break;

    case EVENT_TIMEDOUT:
      result = tr("timed out");
      InformUser(this, tr("Timeout while setting security options."));
      break;

    case EVENT_ERROR:
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
