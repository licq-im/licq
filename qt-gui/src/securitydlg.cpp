// -*- c-basic-offset: 2 -*-
/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qwhatsthis.h>
#include <qtoolbutton.h>

#include "ewidgets.h"
#include "securitydlg.h"
#include "sigman.h"

#include "licq_icqd.h"
#include "licq_events.h"
#include "licq_user.h"

SecurityDlg::SecurityDlg(CICQDaemon *s, CSignalManager *_sigman,
                         QWidget *parent)
   : QWidget(parent, "SecurityDialog", WStyle_ContextHelp | WDestructiveClose )
{
  server = s;
  sigman = _sigman;
  tag = NULL;

  QVBoxLayout *lay = new QVBoxLayout(this, 8);
  QGroupBox *box = new QGroupBox(tr("Options"), this);
  lay->addWidget(box);

  QVBoxLayout *blay = new QVBoxLayout(box, 15);
  chkAuthorization = new QCheckBox(tr("Authorization Required"), box);
  QWhatsThis::add(chkAuthorization, tr("Determines whether regular ICQ clients "
                                       "require your authorization to add you to "
                                       "their contact list."));
  chkWebAware = new QCheckBox(tr("Web Presence"), box);
  QWhatsThis::add(chkWebAware, tr("Web presence allows users to see if you are online "
                                  "through your web indicator."));
  chkHideIp = new QCheckBox(tr("Hide IP"), box);
  QWhatsThis::add(chkHideIp, tr("Hiding ip stops users from seeing your ip."));
  blay->addWidget(chkAuthorization);
  blay->addWidget(chkWebAware);
  blay->addWidget(chkHideIp);

  btnUpdate = new QPushButton(tr("&Update"), this);
  btnUpdate->setDefault(true);
  btnUpdate->setMinimumWidth(75);
  btnCancel = new QPushButton(tr("&Cancel"), this);
  btnCancel->setMinimumWidth(75);
  QHBoxLayout *hlay = new QHBoxLayout;
  hlay->addWidget(QWhatsThis::whatsThisButton(this), 0, AlignLeft);
  hlay->addSpacing(20);
  hlay->addStretch(1);
  hlay->addWidget(btnUpdate, 0, AlignRight);
  hlay->addSpacing(20);
  hlay->addWidget(btnCancel, 0, AlignLeft);
  lay->addLayout(hlay);

  connect (btnUpdate, SIGNAL(clicked()), SLOT(ok()) );
  connect (btnCancel, SIGNAL(clicked()), SLOT(close()) );

  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  chkAuthorization->setChecked(o->GetAuthorization());
  chkWebAware->setChecked(o->WebAware());
  chkHideIp->setChecked(o->HideIp());
  gUserManager.DropOwner();

  setCaption(tr("ICQ Security Options"));

  show();
}


SecurityDlg::~SecurityDlg()
{
  delete tag;
}


void SecurityDlg::ok()
{
  ICQOwner* o = gUserManager.FetchOwner(LOCK_R);
  if(o == NULL)  return;
  unsigned short status = o->Status();
  gUserManager.DropOwner();

  if(status == ICQ_STATUS_OFFLINE) {
    InformUser(this, tr("You need to be connected to the\n"
                        "ICQ Network to change the settings."));
    return;
  }

  btnUpdate->setEnabled(false);
  QObject::connect(sigman, SIGNAL(signal_doneUserFcn(ICQEvent *)),
                   this, SLOT(slot_doneUserFcn(ICQEvent *)));
  tag = server->icqSetSecurityInfo(chkAuthorization->isChecked(),
                                chkHideIp->isChecked(),
                                chkWebAware->isChecked());
  setCaption(tr("ICQ Security Options [Setting...]"));
}


void SecurityDlg::slot_doneUserFcn(ICQEvent *e)
{
  if (!tag->Equals(e)) return;

  btnUpdate->setEnabled(true);
  if (tag != NULL)
  {
    delete tag;
    tag = NULL;
  }

  QString result;
  switch (e->Result())
  {
  case EVENT_FAILED:
    result = tr("failed");
    break;
  case EVENT_TIMEDOUT:
    result = tr("timed out");
    break;
  case EVENT_ERROR:
    result = tr("error");
    break;
  default:
    break;
  }

  if(!result.isEmpty())
    setCaption(tr("ICQ Security Options [Setting...") + result + "]");
  else
  {
    setCaption(tr("ICQ Security Options"));
    close();
  }


}


#include "securitydlg.moc"
