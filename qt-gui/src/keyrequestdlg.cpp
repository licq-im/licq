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

// written by Graham Roff <graham@licq.org>
// -----------------------------------------------------------------------------

#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qapplication.h>

#include "licq_icqd.h"
#include "keyrequestdlg.h"
#include "sigman.h"
#include "licq_user.h"


// -----------------------------------------------------------------------------

KeyRequestDlg::KeyRequestDlg(CSignalManager* _sigman, unsigned long nUin, QWidget *parent)
  : QWidget(parent, "KeyRequestDialog", WDestructiveClose)
{
  m_nUin = nUin;
  sigman = _sigman;
  icqEventTag = NULL;

  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
  setCaption(tr("Licq - Secure Channel with %1").arg(u->GetAlias()));

  QBoxLayout *top_lay = new QVBoxLayout(this, 10);

  QLabel *lbl = new QLabel(tr(
  "Secure channel is established using\n"
  "Diffie-Hellman key exchange, and encrypted\n"
  "using DES XCBC encryption.\n"
  "This only works with other Licq clients."
  ), this);
  top_lay->addWidget(lbl);

  lblStatus = new QLabel(this);
  top_lay->addWidget(lblStatus);

  btn = new QPushButton(tr("&Send"), this);
  btn->setMinimumWidth(75);
  btn->setDefault(true);
  connect(btn, SIGNAL(clicked()), SLOT(button()));
  top_lay->addWidget(btn);

  QPushButton *btnCancel = new QPushButton(tr("&Close"), this);
  btnCancel->setMinimumWidth(75);
  connect(btnCancel, SIGNAL(clicked()), SLOT(cancel()));
  top_lay->addWidget(btnCancel);

  if (gLicqDaemon->CryptoEnabled())
  {
    m_bOpen = !u->Secure();
    if (u->Secure())
      lblStatus->setText("Ready to close channel");
    else
      lblStatus->setText("Ready to request channel");
  }
  else
  {
    lblStatus->setText(tr("Client does not support OpenSSL.\n"
                          "Rebuild Licq with OpenSSL support."));
    btn->setEnabled(false);
  }

  gUserManager.DropUser(u);

  show();
}


// -----------------------------------------------------------------------------

KeyRequestDlg::~KeyRequestDlg()
{
  if (icqEventTag != NULL)
  {
    gLicqDaemon->CancelEvent(icqEventTag);
    delete icqEventTag;
    icqEventTag = NULL;
  }
}



// -----------------------------------------------------------------------------

void KeyRequestDlg::button()
{
  connect (sigman, SIGNAL(signal_doneUserFcn(ICQEvent *)), this, SLOT(doneEvent(ICQEvent *)));

  if (m_bOpen)
  {
    lblStatus->setText(tr("Requesting secure channel..."));
    QApplication::syncX();
    icqEventTag = gLicqDaemon->icqOpenSecureChannel(m_nUin);
  }
  else
  {
    lblStatus->setText(tr("Closing secure channel..."));
    QApplication::syncX();
    icqEventTag = gLicqDaemon->icqCloseSecureChannel(m_nUin);
  }
}


void KeyRequestDlg::cancel()
{
  QWidget::close(true);
}


// -----------------------------------------------------------------------------

void KeyRequestDlg::doneEvent(ICQEvent *e)
{
  if ( !icqEventTag->Equals(e))
    return;


  QString result;
  if (e == NULL)
  {
    if (m_bOpen)
      result = tr("<font color=\"yellow\">Secure channel already established.</font>\n");
    else
      result = tr("<font color=\"yellow\">Secure channel not established.</font>\n");
  }
  else
  {
    switch (e->Result())
    {
      case EVENT_FAILED:
        result = tr("<font color=\"red\">Remote client does not support OpenSSL.</font>");
        break;
      case EVENT_ERROR: // could not connect to remote host (or out of memory)
        result = tr("<font color=\"red\">Could not connect to remote client.</font>");
        break;
      case EVENT_SUCCESS:
        if (m_bOpen)
          result = tr("<font color=\"green\">Secure channel established.</font>\n");
        else
          result = tr("<font color=\"blue\">Secure channel closed.</font>\n");
        break;
      default:
        break;
    }
  }

  lblStatus->setText(result);

  if (icqEventTag != NULL)
  {
    delete icqEventTag;
    icqEventTag = NULL;
  }

  btn->setEnabled(false);
}

#include "keyrequestdlg.moc"
