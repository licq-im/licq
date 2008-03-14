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

#include "config.h"

#include "usersendfileevent.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QPushButton>
#include <QFileInfo>
#include <QFileDialog>
#include <QTextCodec>
#include <QAction>

#ifdef USE_KDE
#include <KDE/KFileDialog>
#endif

#include <licq_icqd.h>

#include "core/gui-defines.h"
#include "core/licqgui.h"
#include "core/messagebox.h"

#include "dialogs/filedlg.h"
#include "dialogs/editfilelistdlg.h"

#include "widgets/infofield.h"
#include "widgets/mledit.h"

#include "usereventtabdlg.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::UserSendFileEvent */

UserSendFileEvent::UserSendFileEvent(QString id, unsigned long ppid, QWidget* parent)
  : UserSendCommon(FileEvent, id, ppid, parent, "UserSendFileEvent")
{
  chkMass->setChecked(false);
  chkMass->setEnabled(false);
  myForeColor->setEnabled(false);
  myBackColor->setEnabled(false);

  mainWidget->addWidget(splView);

  QHBoxLayout* h_lay = new QHBoxLayout();
  mainWidget->addLayout(h_lay);
  lblItem = new QLabel(tr("File(s): "));
  h_lay->addWidget(lblItem);

  edtItem = new InfoField(false);
  edtItem->setReadOnly(true);
  h_lay->addWidget(edtItem);

  btnBrowse = new QPushButton(tr("Browse"));
  connect(btnBrowse, SIGNAL(clicked()), SLOT(browseFile()));
  h_lay->addWidget(btnBrowse);

  btnEdit = new QPushButton(tr("Edit"));
  btnEdit->setEnabled(false);
  connect(btnEdit, SIGNAL(clicked()), SLOT(editFileList()));
  h_lay->addWidget(btnEdit);

  myBaseTitle += tr(" - File Transfer");

  UserEventTabDlg* tabDlg = LicqGui::instance()->userEventTabDlg();
  if (tabDlg != NULL && tabDlg->tabIsSelected(this))
    tabDlg->setWindowTitle(myBaseTitle);

  setWindowTitle(myBaseTitle);
  grpSendType->actions().at(FileEvent)->setChecked(true);
}

UserSendFileEvent::~UserSendFileEvent()
{
  // Empty
}

void UserSendFileEvent::setFile(const QString& file, const QString& description)
{
  QFileInfo fileinfo(file);
  if (fileinfo.exists() && fileinfo.isFile() && fileinfo.isReadable())
  {
    edtItem->setText(file);
    setText(description);
    myFileList.push_back(strdup(file.toLocal8Bit()));
    btnEdit->setEnabled(true);
  }
}

void UserSendFileEvent::addFile(const QString& file)
{
  if (myFileList.size() == 0)
    return;

  myFileList.push_back(strdup(file.toLocal8Bit()));

  btnEdit->setEnabled(true);
  edtItem->setText(QString(tr("%1 Files")).arg(myFileList.size()));
}

bool UserSendFileEvent::sendDone(ICQEvent* e)
{
  if (!e->ExtendedAck() || !e->ExtendedAck()->Accepted())
  {
    ICQUser* u = gUserManager.FetchUser(myUsers.front().c_str(), myPpid, LOCK_R);
    QString s = !e->ExtendedAck() ?
      tr("No reason provided") :
      myCodec->toUnicode(e->ExtendedAck()->Response());
    QString result = tr("File transfer with %1 refused:\n%2")
      .arg(u == NULL ?
          QString(myUsers.front().c_str()) :
          QString::fromUtf8(u->GetAlias()))
      .arg(s);
    if (u != NULL)
      gUserManager.DropUser(u);
    InformUser(this, result);
  }
  else
  {
    CEventFile* f = dynamic_cast<CEventFile*>(e->UserEvent());
    FileDlg* fileDlg = new FileDlg(myUsers.front().c_str(), myPpid);
    fileDlg->SendFiles(f->FileList(), e->ExtendedAck()->Port());
  }

  return true;
}

void UserSendFileEvent::resetSettings()
{
  mleSend->clear();
  edtItem->clear();
  mleSend->setFocus();
  myFileList.clear();
  btnEdit->setEnabled(false);
  slotMassMessageToggled(false);
}

void UserSendFileEvent::browseFile()
{
#ifdef USE_KDE
  QStringList fl = KFileDialog::getOpenFileNames(KUrl(), QString(), this, tr("Select files to send"));
#else
  QStringList fl = QFileDialog::getOpenFileNames(this, tr("Select files to send"));
#endif

  if (fl.isEmpty())
    return;

  QStringList::ConstIterator it = fl.begin();

  for(; it != fl.end(); it++)
    myFileList.push_back(strdup((*it).toLocal8Bit()));

  updateLabel(myFileList.size());
}

void UserSendFileEvent::editFileList()
{
  EditFileListDlg* dlg = new EditFileListDlg(&myFileList);

  connect(dlg, SIGNAL(fileDeleted(unsigned)), SLOT(updateLabel(unsigned)));
}

void UserSendFileEvent::updateLabel(unsigned count)
{
  QString f;

  btnEdit->setEnabled(true);

  switch (count)
  {
    case 0:
      btnEdit->setEnabled(false);
      f = QString::null;
      break;

    case 1:
      f = myFileList.front();
      break;

    default:
      f = QString(tr("%1 Files")).arg(count);
      break;
  }

  edtItem->setText(f);
}

void UserSendFileEvent::send()
{
  // Take care of typing notification now
  tmrSendTyping->stop();
  connect(mleSend, SIGNAL(textChanged()), SLOT(messageTextChanged()));
  gLicqDaemon->ProtoTypingNotification(myUsers.front().c_str(), myPpid, false, myConvoId);

  if (edtItem->text().trimmed().isEmpty())
  {
    WarnUser(this, tr("You must specify a file to transfer!"));
    return;
  }

  unsigned long icqEventTag;
  //TODO in daemon
  icqEventTag = gLicqDaemon->icqFileTransfer(
      strtoul(myUsers.front().c_str(), NULL, 10),
      myCodec->fromUnicode(edtItem->text()),
      myCodec->fromUnicode(mleSend->toPlainText()),
      myFileList,
      chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL,
      chkSendServer->isChecked());

  myEventTag.push_back(icqEventTag);

  UserSendCommon::send();
}
