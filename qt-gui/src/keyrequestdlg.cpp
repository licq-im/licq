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

// written by Graham Roff <graham@licq.org>
// -----------------------------------------------------------------------------

#include <qapplication.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qtimer.h>

#include "licq_icqd.h"
#include "keyrequestdlg.h"
#include "sigman.h"
#include "licq_user.h"


// -----------------------------------------------------------------------------

KeyRequestDlg::KeyRequestDlg(CSignalManager* _sigman, unsigned long nUin, QWidget *parent)
  : QDialog(parent, "KeyRequestDialog", false, WDestructiveClose)
{
  m_nUin = nUin;
  sigman = _sigman;
  icqEventTag = NULL;

  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
  setCaption(tr("Licq - Secure Channel with %1").arg(u->GetAlias()));

  QBoxLayout *top_lay = new QVBoxLayout(this, 10);

  QLabel *lbl = new QLabel(tr(
    "Secure channel is established using\n"
    "Diffie-Hellman key exchange, and \n"
    "encrypted using DES XCBC encryption.\n"
    "This only works with other Licq clients."), this);
  top_lay->addWidget(lbl);

  lblStatus = new QLabel(this);
  top_lay->addWidget(lblStatus);

  QBoxLayout* lay = new QHBoxLayout(top_lay);
  lay->addStretch(1);

  btnSend = new QPushButton(tr("&Send"), this);
  btnSend->setMinimumWidth(75);
  btnSend->setDefault(true);
  connect(btnSend, SIGNAL(clicked()), SLOT(startSend()));
  lay->addWidget(btnSend);

  btnCancel = new QPushButton(tr("&Close"), this);
  btnCancel->setMinimumWidth(75);
  connect(btnCancel, SIGNAL(clicked()), SLOT(close()));
  lay->addWidget(btnCancel);

  if (gLicqDaemon->CryptoEnabled())
  {
    m_bOpen = !u->Secure();
    if (u->Secure())
      lblStatus->setText("Ready to close channel");
    else
      lblStatus->setText("Ready to request channel");
    btnSend->setFocus();
  }
  else
  {
    lblStatus->setText(tr("Client does not support OpenSSL.\n"
                          "Rebuild Licq with OpenSSL support."));
    btnSend->setEnabled(false);
    btnCancel->setFocus();
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

void KeyRequestDlg::startSend()
{
  connect (sigman, SIGNAL(signal_doneUserFcn(ICQEvent *)), this, SLOT(doneEvent(ICQEvent *)));
  btnSend->setEnabled(false);

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


void KeyRequestDlg::done(int r)
{
  QDialog::done(r);
  QTimer::singleShot(0, this, SLOT(close()));
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
    btnSend->setEnabled(false);
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
          result = tr("<font color=\"ForestGreen\">Secure channel established.</font>\n");
        else
          result = tr("<font color=\"blue\">Secure channel closed.</font>\n");
        break;
      default:
        break;
    }
    if(e->Result() == EVENT_SUCCESS) {
      btnSend->setEnabled(false);
      btnCancel->setFocus();
    }
    else
      btnSend->setEnabled(true);
  }

  lblStatus->setText(result);

  if (icqEventTag != NULL)
  {
    delete icqEventTag;
    icqEventTag = NULL;
  }
}


// -----------------------------------------------------------------------------

#include "keyrequestdlg.moc"
