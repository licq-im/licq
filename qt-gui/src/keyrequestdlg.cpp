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
// enhanced by Dirk A. Mueller <dmuell@gmx.net>
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

  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_W);
  setCaption(tr("Licq - Secure Key Request with %1").arg(u->GetAlias()));
  gUserManager.DropUser(u);

  QBoxLayout *top_lay = new QVBoxLayout(this, 10);

  QLabel *lbl = new QLabel(tr(
  "Secure channel will be established using\n"
  "Diffie-Hellman key exchange, and encrypted\n"
  "using DES XCBC encryption."
  ), this);
  top_lay->addWidget(lbl);

  lblStatus = new QLabel(this);
  top_lay->addWidget(lblStatus);

  btn = new QPushButton(this);
  btn->setMinimumWidth(75);
  btn->setDefault(true);
  connect(btn, SIGNAL(clicked()), SLOT(cancel()));
  top_lay->addWidget(btn);

  if (gLicqDaemon->CryptoEnabled())
  {
    lblStatus->setText(tr("Requesting secure channel..."));
    btn->setText(tr("&Cancel"));

    show();
    // Call sync to actually show the window before calling the next functions
    // still doesn't work properly...fucking Qt...
    QApplication::syncX();

    connect (sigman, SIGNAL(signal_doneUserFcn(ICQEvent *)), this, SLOT(doneEvent(ICQEvent *)));
    icqEventTag = gLicqDaemon->icqSendKeyRequest(m_nUin);
    if (icqEventTag == NULL) doneEvent(NULL);
  }
  else
  {
    lblStatus->setText(tr("Client does not support OpenSSL.\n"
                          "Rebuild Licq with OpenSSL support."));
    btn->setText(tr("&Close"));
    icqEventTag = NULL;

    show();
  }

}


// -----------------------------------------------------------------------------

KeyRequestDlg::~KeyRequestDlg()
{
  delete icqEventTag;
}



// -----------------------------------------------------------------------------

void KeyRequestDlg::cancel()
{
  if (icqEventTag != NULL)
  {
    gLicqDaemon->CancelEvent(icqEventTag);
    delete icqEventTag;
    icqEventTag = NULL;
  }

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
    result = tr("<font color=\"yellow\">Secure channel already established.</font>\n");
  }
  else
  {
    switch (e->Result())
    {
      case EVENT_FAILED:
        result = tr("<font color=\"red\">Remote client does not support OpenSSL.</font>");
        break;
      case EVENT_TIMEDOUT: // never happens
        result = tr("Request timedout.");
        break;
      case EVENT_ERROR: // could not connect to remote host (or out of memory)
        result = tr("<font color=\"red\">Error sending request.\n"
                    "Could not connect to remote client.</font>");
        break;
      case EVENT_SUCCESS:
        result = tr("<font color=\"green\">Secure channel established.</font>\n");
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

  btn->setText(tr("&Close"));
}

#include "keyrequestdlg.moc"
