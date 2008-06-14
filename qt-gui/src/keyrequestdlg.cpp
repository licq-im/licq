// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2006 Licq developers
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

// written by Graham Roff <graham@licq.org>
// -----------------------------------------------------------------------------

#include <qapplication.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qtextcodec.h>
#include <qtimer.h>

#include "licq_icqd.h"
#include "licq_icq.h"
#include "keyrequestdlg.h"
#include "sigman.h"
#include "licq_user.h"
#include "usercodec.h"


// -----------------------------------------------------------------------------
KeyRequestDlg::KeyRequestDlg(CSignalManager* _sigman, const char *szId,
  unsigned long nPPID, QWidget *parent)
  : LicqDialog(parent, "KeyRequestDialog", false, WDestructiveClose)
{
  m_szId = szId ? strdup(szId) : 0;
  m_nPPID = nPPID;
  sigman = _sigman;
  icqEventTag = 0;

  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
  setCaption(tr("Licq - Secure Channel with %1").arg(QString::fromUtf8(u->GetAlias())));

  QBoxLayout *top_lay = new QVBoxLayout(this, 10);

  QString t1 = tr("Secure channel is established using SSL\n"
                  "with Diffie-Hellman key exchange and\n"
                  "the TLS version 1 protocol.\n\n");
  QString t2;
  switch (u->SecureChannelSupport())
  {
    case SECURE_CHANNEL_SUPPORTED:
      t2 = tr("The remote uses Licq %1/SSL.").arg(CUserEvent::LicqVersionToString(u->LicqVersion()));
      if(gLicqDaemon->CryptoEnabled())
        QTimer::singleShot(0, this, SLOT(startSend()));
      break;

    case SECURE_CHANNEL_NOTSUPPORTED:
      t2 = tr("The remote uses Licq %1, however it\n"
              "has no secure channel support compiled in.\n"
              "This probably won't work.").arg(CUserEvent::LicqVersionToString(u->LicqVersion()));
      break;

    default:
      t2 = tr("This only works with other Licq clients >= v0.85\n"
              "The remote doesn't seem to use such a client.\n"
              "This might not work.");
      break;
  }

  QLabel *lbl = new QLabel(t1 + t2, this);
  top_lay->addWidget(lbl);

  lblStatus = new QLabel(this);
  lblStatus->setFrameStyle(QFrame::Box | QFrame::Sunken);
  lblStatus->setAlignment(Qt::AlignCenter);
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
      lblStatus->setText(tr("Ready to close channel"));
    else
      lblStatus->setText(tr("Ready to request channel"));
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
  if (icqEventTag != 0)
  {
    gLicqDaemon->CancelEvent(icqEventTag);
    icqEventTag = 0;
  }
  if (m_szId) free(m_szId);
}



// -----------------------------------------------------------------------------

void KeyRequestDlg::startSend()
{
  connect (sigman, SIGNAL(signal_doneUserFcn(ICQEvent *)), this, SLOT(doneEvent(ICQEvent *)));
  btnSend->setEnabled(false);

  if (m_bOpen)
  {
    lblStatus->setText(tr("Requesting secure channel..."));
    QTimer::singleShot(100, this, SLOT(openConnection()));
  }
  else
  {
    lblStatus->setText(tr("Closing secure channel..."));
    QTimer::singleShot(100, this, SLOT(closeConnection()));
  }
}

void KeyRequestDlg::openConnection()
{
  if (m_nPPID == LICQ_PPID )
    icqEventTag = gLicqDaemon->icqOpenSecureChannel(m_szId);
}

void KeyRequestDlg::closeConnection()
{
  if (m_nPPID == LICQ_PPID )
    icqEventTag = gLicqDaemon->icqCloseSecureChannel(m_szId);
}

// -----------------------------------------------------------------------------

void KeyRequestDlg::doneEvent(ICQEvent *e)
{
  if ( !e->Equals(icqEventTag))
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
      QTimer::singleShot(500, this, SLOT(close()));
    }
    else
      btnSend->setEnabled(true);

/*
 * Disabled
 *   if ( m_bOpen ) {
 *     ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_W);
 *     if ( u ) {
 *       u->SetAutoSecure( e->Result() == EVENT_SUCCESS );
 *       gUserManager.DropUser( u );
 *     }
 *   }
 */
  }

  lblStatus->setText(result);

  icqEventTag = 0;
}


// -----------------------------------------------------------------------------

#include "keyrequestdlg.moc"
