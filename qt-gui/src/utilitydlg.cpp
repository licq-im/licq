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

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qsocketnotifier.h>
#include <qlayout.h>

#include "utilitydlg.h"
#include "ewidgets.h"
#include "mledit.h"

#include "icqd.h"
#include "utility.h"
#include "user.h"


CUtilityDlg::CUtilityDlg(CUtility *u, unsigned long _nUin, CICQDaemon *_server)
  : QDialog(0)
{
  m_nUin = _nUin;
  m_xUtility = u;
  server = _server;
  fsCommand = NULL;
  m_bIntWin = false;

  m_xUtility->SetFields(m_nUin);

  QGridLayout *lay = new QGridLayout(this, 1, 2, 10, 5);

  setCaption(QString(tr("Licq Utility: %1")).arg(m_xUtility->Name()));
  lblUtility = new QLabel(tr("Command:"), this);
  lay->addWidget(lblUtility, 0, 0);
  nfoUtility = new CInfoField(this, true);
  nfoUtility->setMinimumWidth(nfoUtility->sizeHint().width()*2);
  lay->addWidget(nfoUtility, 0, 1);
  nfoUtility->setText(m_xUtility->FullCommand());

  lay->addWidget(new QLabel(tr("Window:"), this), 1, 0);
  nfoWinType = new CInfoField(this, true);
  lay->addWidget(nfoWinType, 1, 1);
  switch (m_xUtility->WinType()) {
  case UtilityWinGui: nfoWinType->setText(tr("GUI")); break;
  case UtilityWinTerm: nfoWinType->setText(tr("Terminal")); break;
  case UtilityWinLicq: nfoWinType->setText(tr("Internal")); break;
  }

  lay->addWidget(new QLabel(tr("Description:"), this), 2, 0);
  nfoDesc = new CInfoField(this, true);
  lay->addWidget(nfoDesc, 2, 1);
  nfoDesc->setText(m_xUtility->Description());

  chkEditFinal = new QCheckBox(tr("Edit final command"), this);
  lay->addMultiCellWidget(chkEditFinal, 3, 3, 0, 1);

  boxFields = new QGroupBox(1, Horizontal, tr("User Fields"), this);
  lay->addMultiCellWidget(boxFields, 4, 4, 0, 1);
  mleCommand = new MLEditWrap(true, boxFields);
  mleCommand->setReadOnly(true);
  mleCommand->hide();
  for (unsigned short i = 0; i < m_xUtility->NumUserFields(); i++) {
    QLabel *lbl = new QLabel(QString("%1 (%%%2):").arg(
      m_xUtility->UserField(i)->Title(), i+1), boxFields);
    lblFields.push_back(lbl);
    QLineEdit *edt = new QLineEdit(boxFields);
    edt->setText(m_xUtility->UserField(i)->FullDefault());
    edt->setMinimumSize(edt->sizeHint());
    edtFields.push_back(edt);
  }
  if (m_xUtility->NumUserFields() == 0) boxFields->hide();

  QHBoxLayout *hlay = new QHBoxLayout(lay);
  hlay->addStretch(1);
  btnRun = new QPushButton(tr("&Run"), this);
  btnRun->setDefault(true);
  btnRun->setMinimumWidth(75);
  hlay->addWidget(btnRun, 0, AlignRight);
  hlay->addSpacing(20);
  btnCancel = new QPushButton(tr("&Cancel"), this);
  btnCancel->setMinimumWidth(75);
  hlay->addWidget(btnCancel, 0, AlignLeft);
  lay->addMultiCell(hlay, 5, 5, 0, 1);

  connect(btnRun, SIGNAL(clicked()), SLOT(slot_run()));
  connect(btnCancel, SIGNAL(clicked()), SLOT(slot_cancel()));

  show();
}


CUtilityDlg::~CUtilityDlg(void)
{
}


void CUtilityDlg::slot_cancel()
{
  if (m_bIntWin)
  {
    if (fsCommand != NULL)
    {
      mleCommand->append("EOF");
      m_bIntWin = false;
      disconnect(snCommand, SIGNAL(activated(int)), this, SLOT(slot_command()));
      fclose(fsCommand);
      fsCommand = NULL;
    }
    lblUtility->setText(tr("Done:"));
    btnCancel->setText(tr("Close"));
    m_bIntWin = false;
  }
  else
    hide();
}

void CUtilityDlg::hide()
{
   QWidget::hide();
   if (fsCommand != NULL)
     fclose(fsCommand);
   delete this;
}



void CUtilityDlg::slot_run()
{
  // Use the readonly state of the nfoUtility as a flag for whether or
  // not we are editing it
  if (nfoUtility->ReadOnly())
  {
    // Set the user fields
    vector<const char *> vszFields(m_xUtility->NumUserFields());
    vector<QLineEdit *>::iterator iter;
    unsigned short i = 0;
    for (iter = edtFields.begin(); iter != edtFields.end(); iter++)
    {
      vszFields[i++] = (*iter)->text();
    }
    m_xUtility->SetUserFields(vszFields);
    nfoUtility->setText(m_xUtility->FullCommand());
    if (chkEditFinal->isChecked())
    {
      lblUtility->setText(tr("Edit:"));
      nfoUtility->SetReadOnly(false);
      chkEditFinal->setEnabled(false);
      return;
    }
  }

  nfoUtility->SetReadOnly(true);
  lblUtility->setText(tr("Running:"));

  // Run the command
  int nSystemResult = 0;
  QString cmd = nfoUtility->text();
  switch(m_xUtility->WinType())
  {
  case UtilityWinGui:
  {
    m_xUtility->SetBackgroundTask();
    nSystemResult = system(cmd);
    break;
  }
  case UtilityWinTerm:
  {
    char* szCmd = new char[cmd.length() + strlen(server->Terminal()) + 4];
    sprintf(szCmd, "%s %s &", server->Terminal(), (const char *)cmd);
    nSystemResult = system(szCmd);
    delete szCmd;
    break;
  }
  case UtilityWinLicq:
  {
    for (unsigned short i = 0; i < m_xUtility->NumUserFields(); i++)
    {
      lblFields[i]->hide();
      edtFields[i]->hide();
    }
    boxFields->setTitle(tr("Command Window"));
    boxFields->show();
    mleCommand->show();
    resize(width(), 300);
    fsCommand = popen(cmd, "r");
    if (fsCommand != NULL)
    {
      snCommand = new QSocketNotifier(fileno(fsCommand), QSocketNotifier::Read, this);
      connect(snCommand, SIGNAL(activated(int)), SLOT(slot_command()));
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
    m_xUtility->SetFields(m_nUin);
  }
  else
  {
    btnRun->setEnabled(false);
    btnCancel->setText(tr("Done"));
  }
}

void CUtilityDlg::slot_command()
{
  char buf[1024];
  if (fgets(buf, 1024, fsCommand) == NULL)
  {
    mleCommand->append("EOF");
    if (fsCommand != NULL)
    {
      fclose(fsCommand);
      fsCommand = NULL;
    }
    m_bIntWin = false;
    btnCancel->setText(tr("C&lose"));
    disconnect(snCommand, SIGNAL(activated(int)), this, SLOT(slot_command()));
  }
  else
  {
    if (buf[strlen(buf) - 1] == '\n') buf[strlen(buf) - 1] = '\0';
    mleCommand->append(buf);
  }
}

#include "utilitydlg.moc"
