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

#include <qvalidator.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qvgroupbox.h>
#include <qtextcodec.h>

#include "authuserdlg.h"
#include "mledit.h"

#include "licq_user.h"
#include "licq_icqd.h"
#include "usercodec.h"

//TODO Add drop down list of avaiable protocols
AuthUserDlg::AuthUserDlg(CICQDaemon *s, const char *szId, unsigned long nPPID,
   bool bGrant, QWidget *parent)
  : LicqDialog(parent, "AuthUserDialog", false, WDestructiveClose)
{
  server = s;
  m_szId = szId ? strdup(szId) : 0;
  m_nPPID = nPPID;
  m_bGrant = bGrant;

  if(bGrant)
    setCaption(tr("Licq - Grant Authorisation"));
  else
    setCaption(tr("Licq - Refuse Authorisation"));

  QBoxLayout* toplay = new QVBoxLayout(this, 8, 8);

  lblUin = new QLabel(this);
  lblUin->setAlignment(AlignCenter);
  if (m_szId == 0)
  {
    lblUin->setText(tr("Authorize which user (Id):"));
    edtUin = new QLineEdit(this);
    edtUin->setMinimumWidth(90);
    connect (edtUin, SIGNAL(returnPressed()), SLOT(ok()) );
    QBoxLayout* lay = new QHBoxLayout(toplay);
    lay->addWidget(lblUin);
    lay->addWidget(edtUin);
  }
  else
  {
    edtUin = NULL;
    toplay->addWidget(lblUin);
    QString userName;
    ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
    if (u != NULL) {
       QTextCodec *codec = UserCodec::codecForICQUser(u);
       userName = QString("%1 (%2)").arg(codec->toUnicode(u->GetAlias())).arg(m_szId);
       gUserManager.DropUser(u);
    } else {
       userName = QString("%1").arg(m_szId);
    }

    if (bGrant)
      lblUin->setText(tr("Grant authorization to %1").arg(userName));
    else
      lblUin->setText(tr("Refuse authorization to %1").arg(userName));
  }

  toplay->addSpacing(6);

  grpResponse = new QVGroupBox(tr("Response"), this);
  toplay->addWidget(grpResponse);
  toplay->setStretchFactor(grpResponse, 2);

  mleResponse = new MLEditWrap(true, grpResponse);

  QBoxLayout* lay = new QHBoxLayout(toplay);
  lay->addStretch(1);
  btnOk = new QPushButton(tr("&Ok"), this);
  btnOk->setMinimumWidth(75);
  lay->addWidget(btnOk);
  btnCancel = new QPushButton(tr("&Cancel"), this);
  btnCancel->setMinimumWidth(75);
  lay->addWidget(btnCancel);
  connect (mleResponse, SIGNAL(signal_CtrlEnterPressed()), this, SLOT(ok()));
  connect (btnOk, SIGNAL(clicked()), SLOT(ok()) );
  connect (btnCancel, SIGNAL(clicked()), SLOT(close()) );

  if (m_szId)
    mleResponse->setFocus();
  else
    edtUin->setFocus();

  show();
}

AuthUserDlg::AuthUserDlg(CICQDaemon *s, unsigned long nUin, bool bGrant,
   QWidget *parent)
  : LicqDialog(parent, "AuthUserDialog", false, WDestructiveClose)
{
  server = s;
  m_nUin = nUin;
  m_bGrant = bGrant;

  if(bGrant)
    setCaption(tr("Licq - Grant Authorisation"));
  else
    setCaption(tr("Licq - Refuse Authorisation"));

  QBoxLayout* toplay = new QVBoxLayout(this, 8, 8);

  lblUin = new QLabel(this);
  lblUin->setAlignment(AlignCenter);
  if (m_nUin == 0)
  {
    lblUin->setText(tr("Authorize which user (UIN):"));
    edtUin = new QLineEdit(this);
    edtUin->setMinimumWidth(90);
    edtUin->setValidator(new QIntValidator(10000, 2147483647, edtUin));
    connect (edtUin, SIGNAL(returnPressed()), SLOT(ok()) );
    QBoxLayout* lay = new QHBoxLayout(toplay);
    lay->addWidget(lblUin);
    lay->addWidget(edtUin);
  }
  else
  {
    edtUin = NULL;
    toplay->addWidget(lblUin);
    QString userName;
    ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
    if (u != NULL) {
       QTextCodec *codec = UserCodec::codecForICQUser(u);
       userName = QString("%1 (%2)").arg(codec->toUnicode(u->GetAlias())).arg(m_nUin);
       gUserManager.DropUser(u);
    } else {
       userName = QString::number(m_nUin);
    }

    if (bGrant)
      lblUin->setText(tr("Grant authorization to %1").arg(userName));
    else
      lblUin->setText(tr("Refuse authorization to %1").arg(userName));
  }

  toplay->addSpacing(6);

  grpResponse = new QVGroupBox(tr("Response"), this);
  toplay->addWidget(grpResponse);
  toplay->setStretchFactor(grpResponse, 2);

  mleResponse = new MLEditWrap(true, grpResponse);

  QBoxLayout* lay = new QHBoxLayout(toplay);
  lay->addStretch(1);
  btnOk = new QPushButton(tr("&Ok"), this);
  btnOk->setMinimumWidth(75);
  lay->addWidget(btnOk);
  btnCancel = new QPushButton(tr("&Cancel"), this);
  btnCancel->setMinimumWidth(75);
  lay->addWidget(btnCancel);
  connect (mleResponse, SIGNAL(signal_CtrlEnterPressed()), this, SLOT(ok()));
  connect (btnOk, SIGNAL(clicked()), SLOT(ok()) );
  connect (btnCancel, SIGNAL(clicked()), SLOT(close()) );

  if (m_nUin)
    mleResponse->setFocus();
  else
    edtUin->setFocus();

  show();
}

void AuthUserDlg::ok()
{
  if (edtUin != NULL)
  {
    if (edtUin->text().isEmpty()) return;
    m_nUin = edtUin->text().toULong();
  }

  if (m_szId == 0) m_szId = strdup(edtUin->text().latin1());

  if (m_szId != 0)
  {
    //TODO
    QTextCodec *codec = UserCodec::codecForProtoUser(m_szId, LICQ_PPID);
    if (m_bGrant)
      server->icqAuthorizeGrant(strtoul(m_szId, (char **)NULL, 10), codec->fromUnicode(mleResponse->text()));
    else
      server->icqAuthorizeRefuse(strtoul(m_szId, (char **)NULL, 10), codec->fromUnicode(mleResponse->text()));
    close(true);
  }
}

#include "authuserdlg.moc"
