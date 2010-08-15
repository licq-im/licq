// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2010 Licq developers
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

#include <licq/contactlist/user.h>
#include <licq/event.h>
#include <licq/icqdefines.h>
#include <licq/protocolmanager.h>
#include <licq/userevents.h>

#include "core/gui-defines.h"
#include "core/messagebox.h"

#include "dialogs/filedlg.h"
#include "dialogs/editfilelistdlg.h"

#include "widgets/infofield.h"
#include "widgets/mledit.h"

using Licq::gProtocolManager;
using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::UserSendFileEvent */

UserSendFileEvent::UserSendFileEvent(const Licq::UserId& userId, QWidget* parent)
  : UserSendCommon(FileEvent, userId, parent, "UserSendFileEvent")
{
  myMassMessageCheck->setChecked(false);
  myMassMessageCheck->setEnabled(false);
  myForeColor->setEnabled(false);
  myBackColor->setEnabled(false);

  myMainWidget->addWidget(myViewSplitter);

  QHBoxLayout* h_lay = new QHBoxLayout();
  myMainWidget->addLayout(h_lay);
  myFileLabel = new QLabel(tr("File(s): "));
  h_lay->addWidget(myFileLabel);

  myFileEdit = new InfoField(false);
  myFileEdit->setReadOnly(true);
  h_lay->addWidget(myFileEdit);

  myBrowseButton = new QPushButton(tr("Browse"));
  connect(myBrowseButton, SIGNAL(clicked()), SLOT(browseFile()));
  h_lay->addWidget(myBrowseButton);

  myEditButton = new QPushButton(tr("Edit"));
  myEditButton->setEnabled(false);
  connect(myEditButton, SIGNAL(clicked()), SLOT(editFileList()));
  h_lay->addWidget(myEditButton);

  myBaseTitle += tr(" - File Transfer");

  setWindowTitle(myBaseTitle);
  myEventTypeGroup->actions().at(FileEvent)->setChecked(true);
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
    myFileEdit->setText(file);
    setText(description);
    myFileList.push_back(strdup(file.toLocal8Bit()));
    myEditButton->setEnabled(true);
  }
}

void UserSendFileEvent::addFile(const QString& file)
{
  if (myFileList.empty())
    return;

  myFileList.push_back(strdup(file.toLocal8Bit()));

  myEditButton->setEnabled(true);
  myFileEdit->setText(QString(tr("%1 Files")).arg(myFileList.size()));
}

bool UserSendFileEvent::sendDone(const Licq::Event* e)
{
  if (!e->ExtendedAck() || !e->ExtendedAck()->accepted())
  {
    Licq::UserReadGuard u(myUsers.front());
    if (!u.isLocked())
      return true;
    QString s = !e->ExtendedAck() ?
      tr("No reason provided") :
      myCodec->toUnicode(e->ExtendedAck()->response().c_str());
    QString result = tr("File transfer with %1 refused:\n%2")
      .arg(QString::fromUtf8(u->GetAlias()))
      .arg(s);
    u.unlock();
    InformUser(this, result);
  }
  else
  {
    const Licq::EventFile* f = dynamic_cast<const Licq::EventFile*>(e->userEvent());
    FileDlg* fileDlg = new FileDlg(myUsers.front());
    fileDlg->SendFiles(f->FileList(), e->ExtendedAck()->port());
  }

  return true;
}

void UserSendFileEvent::resetSettings()
{
  myMessageEdit->clear();
  myFileEdit->clear();
  myMessageEdit->setFocus();
  myFileList.clear();
  myEditButton->setEnabled(false);
  massMessageToggled(false);
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

  myEditButton->setEnabled(true);

  switch (count)
  {
    case 0:
      myEditButton->setEnabled(false);
      f = QString::null;
      break;

    case 1:
      f = myFileList.front().c_str();
      break;

    default:
      f = QString(tr("%1 Files")).arg(count);
      break;
  }

  myFileEdit->setText(f);
}

void UserSendFileEvent::send()
{
  // Take care of typing notification now
  mySendTypingTimer->stop();
  connect(myMessageEdit, SIGNAL(textChanged()), SLOT(messageTextChanged()));
  gProtocolManager.sendTypingNotification(myUsers.front(), false, myConvoId);

  if (myFileEdit->text().trimmed().isEmpty())
  {
    WarnUser(this, tr("You must specify a file to transfer!"));
    return;
  }

  unsigned long icqEventTag;
  //TODO in daemon
  icqEventTag = gProtocolManager.fileTransferPropose(
      myUsers.front(),
      myCodec->fromUnicode(myFileEdit->text()).data(),
      myCodec->fromUnicode(myMessageEdit->toPlainText()).data(),
      myFileList,
      myUrgentCheck->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL,
      mySendServerCheck->isChecked());

  myEventTag.push_back(icqEventTag);

  UserSendCommon::send();
}
