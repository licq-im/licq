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

#include "utilitydlg.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSocketNotifier>
#include <QSplitter>

#include <licq/daemon.h>
#include <licq/utility.h>

#include "core/messagebox.h"

#include "widgets/infofield.h"
#include "widgets/mledit.h"

using Licq::Utility;
using Licq::gUtilityManager;
using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::UtilityDlg */

UtilityDlg::UtilityDlg(Utility* u, const Licq::UserId& userId)
  : myUserId(userId)
{
  setObjectName("UtilityDialog");
  setAttribute(Qt::WA_DeleteOnClose, true);

  myUtility = u;
  m_bIntWin = false;
  myInternalWindow = NULL;
  snOut = snErr = NULL;

  myUtility->setFields(myUserId);

  QGridLayout* lay = new QGridLayout(this);
  lay->setColumnStretch(2, 2);
  lay->setColumnMinimumWidth(1, 8);
  setWindowTitle(QString(tr("Licq Utility: %1")).arg(myUtility->name().c_str()));
  lblUtility = new QLabel(tr("Command:"));
  lay->addWidget(lblUtility, 0, 0);
  nfoUtility = new InfoField(true);
  nfoUtility->setMinimumWidth(nfoUtility->sizeHint().width()*2);
  lay->addWidget(nfoUtility, 0, 2);
  nfoUtility->setText(myUtility->fullCommand().c_str());

  lay->addWidget(new QLabel(tr("Window:")), 1, 0);
  nfoWinType = new InfoField(true);
  lay->addWidget(nfoWinType, 1, 2);
  switch (myUtility->winType())
  {
    case Utility::WinGui:
      nfoWinType->setText(tr("GUI"));
      break;
    case Utility::WinTerm:
      nfoWinType->setText(tr("Terminal"));
      break;
    case Utility::WinLicq:
      nfoWinType->setText(tr("Internal"));
      break;
  }

  lay->addWidget(new QLabel(tr("Description:")), 2, 0);
  nfoDesc = new InfoField(true);
  lay->addWidget(nfoDesc, 2, 2);
  nfoDesc->setText(myUtility->description().c_str());

  chkEditFinal = new QCheckBox(tr("Edit final command"));
  lay->addWidget(chkEditFinal, 3, 0, 1, 3);

  boxFields = new QGroupBox(tr("User Fields"));
  lay->addWidget(boxFields, 4, 0, 1, 3);
  QHBoxLayout* layFields = new QHBoxLayout(boxFields);
  for (int i = 0; i < myUtility->numUserFields(); i++)
  {
    QString s;
    s.sprintf("%s (%%%d): ", myUtility->userField(i)->title().c_str(), i+1);
    QLabel* lbl = new QLabel(s);
    lblFields.push_back(lbl);
    layFields->addWidget(lbl);
    QLineEdit* edt = new QLineEdit();
    edt->setText(myUtility->userField(i)->fullDefault().c_str());
    edt->setMinimumSize(edt->sizeHint());
    edtFields.push_back(edt);
    layFields->addWidget(edt);
  }

  splOutput = new QSplitter(Qt::Vertical);
  splOutput->setOpaqueResize();
  mleOut = new MLEdit(true, splOutput);
  mleOut->setReadOnly(true);
  mleErr = new MLEdit(true, splOutput);
  mleErr->setReadOnly(true);
  layFields->addWidget(splOutput);

  splOutput->hide();
  if (myUtility->numUserFields() == 0)
    boxFields->hide();

  QDialogButtonBox* buttons = new QDialogButtonBox();
  lay->addWidget(buttons, 5, 0, 1, 3);
  btnRun = new QPushButton(tr("&Run"));
  btnRun->setDefault(true);
  buttons->addButton(btnRun, QDialogButtonBox::ActionRole);
  btnCancel = new QPushButton(tr("&Cancel"));
  buttons->addButton(btnCancel, QDialogButtonBox::RejectRole);

  connect(btnRun, SIGNAL(clicked()), SLOT(slot_run()));
  connect(btnCancel, SIGNAL(clicked()), SLOT(slot_cancel()));

  show();
}

UtilityDlg::~UtilityDlg()
{
  delete myInternalWindow;
  delete snOut;
  delete snErr;
}


void UtilityDlg::slot_cancel()
{
  if (m_bIntWin)
  {
    if (!m_bStdOutClosed)
    {
      mleOut->append("--- EOF ---");
      snOut->setEnabled(false);
      disconnect(snOut, SIGNAL(activated(int)), this, SLOT(slot_stdout()));
    }
    if (!m_bStdErrClosed)
    {
      mleErr->append("--- EOF ---");
      snErr->setEnabled(false);
      disconnect(snErr, SIGNAL(activated(int)), this, SLOT(slot_stderr()));
    }
    CloseInternalWindow();
  }
  else
    close();
}


void UtilityDlg::slot_run()
{
  // Use the readonly state of the nfoUtility as a flag for whether or
  // not we are editing it
  if (nfoUtility->isReadOnly())
  {
    // Set the user fields
    std::vector<std::string> vszFields(myUtility->numUserFields());
    std::vector<QLineEdit*>::iterator iter;
    unsigned short i = 0;
    for (iter = edtFields.begin(); iter != edtFields.end(); iter++)
    {
      vszFields[i++] = strdup((*iter)->text().toLocal8Bit().data());
    }
    myUtility->setUserFields(vszFields);

    nfoUtility->setText(myUtility->fullCommand().c_str());
    if (chkEditFinal->isChecked())
    {
      lblUtility->setText(tr("Edit:"));
      nfoUtility->setReadOnly(false);
      chkEditFinal->setEnabled(false);
      return;
    }
  }

  nfoUtility->setReadOnly(true);
  lblUtility->setText(tr("Running:"));

  // Run the command
  int nSystemResult = 0;
  QString cmd = nfoUtility->text();
  switch (myUtility->winType())
  {
    case Utility::WinGui:
    {
      myUtility->setBackgroundTask();
      nSystemResult = system(cmd.toLocal8Bit().data());
      break;
    }
    case Utility::WinTerm:
    {
      QString szCmd = Licq::gDaemon.terminal().c_str();
      szCmd.append(" ").append(cmd).append(" &");
      nSystemResult = system(szCmd.toLocal8Bit());
      break;
    }
    case Utility::WinLicq:
    {
      for (int i = 0; i < myUtility->numUserFields(); i++)
      {
        lblFields[i]->hide();
        edtFields[i]->hide();
      }
      boxFields->setTitle(tr("Command Window"));
      boxFields->show();
      splOutput->show();
      resize(width(), 300);
      myInternalWindow = new Licq::UtilityInternalWindow;
      if (myInternalWindow->POpen(cmd.toLocal8Bit().data()))
      {
        m_bStdOutClosed = m_bStdErrClosed = false;
        snOut = new QSocketNotifier(fileno(myInternalWindow->StdOut()), QSocketNotifier::Read, this);
        connect(snOut, SIGNAL(activated(int)), SLOT(slot_stdout()));
        snErr = new QSocketNotifier(fileno(myInternalWindow->StdErr()), QSocketNotifier::Read, this);
        connect(snErr, SIGNAL(activated(int)), SLOT(slot_stderr()));
        nSystemResult = 0;
        m_bIntWin = true;
      }
      else
        nSystemResult = -1;
      break;
    }
  } // switch

  if (nSystemResult == -1)
  {
    lblUtility->setText(tr("Failed:"));
    myUtility->setFields(myUserId);
  }
  else
  {
    btnRun->setEnabled(false);
    btnCancel->setText(tr("Done"));
  }
}


void UtilityDlg::CloseInternalWindow()
{
  m_bIntWin = false;
  lblUtility->setText(tr("Done:"));
  btnCancel->setText(tr("C&lose"));
  myInternalWindow->PClose();
}



void UtilityDlg::slot_stdout()
{
  char buf[1024];
  if (fgets(buf, 1024, myInternalWindow->StdOut()) == NULL)
  {
    m_bStdOutClosed = true;
    disconnect(snOut, SIGNAL(activated(int)), this, SLOT(slot_stdout()));
    if (snOut != NULL)
    {
      snOut->setEnabled(false);
    }
    mleOut->append("--- EOF ---");
    if (m_bStdErrClosed) CloseInternalWindow();
    return;
  }

  if (buf[strlen(buf) - 1] == '\n') buf[strlen(buf) - 1] = '\0';
  mleOut->append(buf);
  mleOut->GotoEnd();
}



void UtilityDlg::slot_stderr()
{
  char buf[1024];
  if (fgets(buf, 1024, myInternalWindow->StdErr()) == NULL)
  {
    m_bStdErrClosed = true;
    disconnect(snErr, SIGNAL(activated(int)), this, SLOT(slot_stderr()));
    if (snErr != NULL)
    {
      snErr->setEnabled(false);
    }
    mleErr->append("--- EOF ---");
    if (m_bStdOutClosed) CloseInternalWindow();
    return;
  }

  if (buf[strlen(buf) - 1] == '\n') buf[strlen(buf) - 1] = '\0';
  mleErr->append(buf);
  mleErr->GotoEnd();
}
