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

#include <qpushbutton.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qwhatsthis.h>
#include <qtoolbutton.h>
#include <qlineedit.h>
#include <qlabel.h>

#include "passworddlg.h"
#include "sigman.h"
#include "ewidgets.h"

#include "icqd.h"
#include "icqevent.h"
#include "user.h"

PasswordDlg::PasswordDlg(CICQDaemon *s, CSignalManager *_sigman,
                         QWidget *parent, const char *name)
   : QDialog(parent, name)
{
  server = s;
  sigman = _sigman;
  tag = NULL;

  QVBoxLayout *lay = new QVBoxLayout(this, 8);
  QGroupBox *box = new QGroupBox(tr("Password"), this);
  lay->addWidget(box);

  QGridLayout *blay = new QGridLayout(box, 2, 2, 15, 5);
  edtFirst = new QLineEdit(box);
  edtFirst->setEchoMode(QLineEdit::Password);
  QWhatsThis::add(edtFirst, tr("Enter your ICQ password here."));
  edtSecond = new QLineEdit(box);
  edtSecond->setEchoMode(QLineEdit::Password);
  QWhatsThis::add(edtSecond, tr("Verify your ICQ password here."));
  blay->addWidget(new QLabel(tr("&Password:"), box), 0, 0);
  blay->addWidget(edtFirst, 0, 1);
  blay->addWidget(new QLabel(tr("&Verify:"), box), 1, 0);
  blay->addWidget(edtSecond, 1, 1);

  btnUpdate = new QPushButton("&Update", this);
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
  connect (btnCancel, SIGNAL(clicked()), SLOT(reject()) );

  setCaption(tr("Set Password"));

  show();
}


PasswordDlg::~PasswordDlg()
{
  if (tag != NULL)
    delete tag;
}



void PasswordDlg::hide()
{
  QDialog::hide();
  delete this;
}


void PasswordDlg::ok()
{
  // Validate password
  if (edtFirst->text().isEmpty() || edtFirst->text().length() > 8)
  {
    InformUser (this, tr("Invalid password, must be 8 characters or less."));
    return;
  }
  if (edtFirst->text() != edtSecond->text())
  {
    InformUser (this, tr("Passwords do not match, try again."));
    return;
  }

  btnUpdate->setEnabled(false);
  QObject::connect(sigman, SIGNAL(signal_doneUserFcn(ICQEvent *)),
                   this, SLOT(slot_doneUserFcn(ICQEvent *)));
  tag = server->icqSetPassword(edtFirst->text());
  setCaption(tr("ICQ Password [Setting...]"));
}


void PasswordDlg::slot_doneUserFcn(ICQEvent *e)
{
  if (!tag->Equals(e)) return;

  btnUpdate->setEnabled(true);
  if (tag != NULL)
  {
    delete tag;
    tag = NULL;
  }

  QString result;
  switch (e->m_eResult)
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
    setCaption(tr("Set Password [Setting...") + result + "]");
  else
  {
    setCaption(tr("Set Password"));
    QWidget::close();
  }
}


#include "moc/moc_passworddlg.h"
