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

// written by Graham Roff <graham@licq.org>
// enhanced by Dirk A. Mueller <dmuell@gmx.net>
// -----------------------------------------------------------------------------

#include "awaymsgdlg.h"

#include <boost/foreach.hpp>

#include <QCloseEvent>
#include <QDialogButtonBox>
#include <QEvent>
#include <QHBoxLayout>
#include <QMenu>
#include <QPushButton>
#include <QTextCodec>
#include <QTimer>
#include <QVBoxLayout>

#include <licq/contactlist/owner.h>
#include <licq/contactlist/usermanager.h>
#include <licq/sarmanager.h>
#include <licq/logging/log.h>

#include "core/licqgui.h"

#include "helpers/support.h"
#include "helpers/usercodec.h"

#include "settings/settingsdlg.h"

#include "widgets/mledit.h"

#include "hintsdlg.h"

using Licq::User;
using Licq::SarManager;
using Licq::gSarManager;
using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::AwayMsgDlg */

AwayMsgDlg* AwayMsgDlg::myInstance = NULL;

void AwayMsgDlg::showAwayMsgDlg(unsigned status, bool autoClose, unsigned long ppid)
{
  if (myInstance == NULL)
    myInstance = new AwayMsgDlg();
  else
    myInstance->raise();

  myInstance->selectAutoResponse(status, autoClose, ppid);
}

AwayMsgDlg::AwayMsgDlg(QWidget* parent)
  : QDialog(parent),
    myAutoCloseCounter(-1)
{
  Support::setWidgetProps(this, "AwayMessageDialog");
  setAttribute(Qt::WA_DeleteOnClose, true);

  installEventFilter(this);

  QVBoxLayout* top_lay = new QVBoxLayout(this);

  myAwayMsg = new MLEdit(true);
  myAwayMsg->setSizeHintLines(5);
  connect(myAwayMsg, SIGNAL(clicked()), SLOT(autoCloseStop()));
  connect(myAwayMsg, SIGNAL(ctrlEnterPressed()), SLOT(ok()));
  myAwayMsg->installEventFilter(this);
  top_lay->addWidget(myAwayMsg);

  myMenu = new QMenu(this);
  connect(myMenu, SIGNAL(aboutToShow()), SLOT(autoCloseStop()));

  myButtons = new QDialogButtonBox(
      QDialogButtonBox::Ok |
      QDialogButtonBox::Cancel |
      QDialogButtonBox::Help);

  myButtons->button(QDialogButtonBox::Help)->setText(tr("&Hints"));
  myButtons->addButton(tr("&Select"), QDialogButtonBox::ActionRole)->setMenu(myMenu);

  connect(myButtons, SIGNAL(accepted()), SLOT(ok()));
  connect(myButtons, SIGNAL(rejected()), SLOT(close()));
  connect(myButtons, SIGNAL(helpRequested()), SLOT(autoCloseStop()));
  connect(myButtons, SIGNAL(helpRequested()), SLOT(hints()));

  myOkText = myButtons->button(QDialogButtonBox::Ok)->text();

  top_lay->addWidget(myButtons);
}

AwayMsgDlg::~AwayMsgDlg()
{
  myInstance = NULL;
}

void AwayMsgDlg::selectAutoResponse(unsigned status, bool autoClose, unsigned long ppid)
{
  // If requested status doesn't support message, set away
  if ((status & User::MessageStatuses) == 0)
    status |= User::AwayStatus;
  status |= User::OnlineStatus;

  myStatus = status;
  myPpid = ppid;
  SarManager::List sarList;

  // Fill in the select menu
  myMenu->clear();
  if (myStatus & User::DoNotDisturbStatus)
    sarList = SarManager::DoNotDisturbList;
  else if (myStatus & User::OccupiedStatus)
    sarList = SarManager::OccupiedList;
  else if (myStatus & User::NotAvailableStatus)
    sarList = SarManager::NotAvailableList;
  else if (myStatus & User::FreeForChatStatus)
    sarList = SarManager::FreeForChatList;
  else // if (myStatus & User::AwayStatus)
    sarList = SarManager::AwayList;

  const Licq::SarList& sars(gSarManager.getList(sarList));
  for (Licq::SarList::const_iterator i = sars.begin(); i != sars.end(); ++i)
  {
    QAction* a = myMenu->addAction(QString::fromLocal8Bit(i->name.c_str()), this, SLOT(selectMessage()));
    a->setData(QString::fromLocal8Bit(i->text.c_str()));
  }
  gSarManager.releaseList();

  myMenu->addSeparator();
  QAction* a = myMenu->addAction(tr("&Edit Items"), this, SLOT(selectMessage()));
  a->setData(QString());

  {
    QString statusStr = User::statusToString(myStatus, true, false).c_str();
    QString autoResponse;

    if (myPpid == 0)
    {
      setWindowTitle(QString(tr("Set %1 Response for all accounts"))
          .arg(statusStr));

      // Check all owners for existing away messages
      Licq::OwnerListGuard ownerList;
      BOOST_FOREACH(const Licq::Owner* owner, **ownerList)
      {
        Licq::OwnerReadGuard o(owner);

        if (!o->autoResponse().empty())
        {
          const QTextCodec* codec = UserCodec::defaultEncoding();
          autoResponse = codec->toUnicode(o->autoResponse().c_str());
          break;
        }
      }
    }
    else
    {
      Licq::OwnerReadGuard o(myPpid);
      if (!o.isLocked())
        return;

      setWindowTitle(QString(tr("Set %1 Response for %2"))
          .arg(statusStr)
          .arg(QString::fromUtf8(o->GetAlias())));

      if (!o->autoResponse().empty())
      {
        const QTextCodec* codec = UserCodec::defaultEncoding();
        autoResponse = codec->toUnicode(o->autoResponse().c_str());
      }
    }

    if (!autoResponse.isEmpty())
      myAwayMsg->setText(autoResponse);
    else
      myAwayMsg->setText(tr("I'm currently %1, %a.\n"
            "You can leave me a message.\n"
            "(%m messages pending from you).")
          .arg(statusStr));
  }

  myAwayMsg->setFocus();
  QTimer::singleShot(0, myAwayMsg, SLOT(selectAll()));

  if (autoClose)
  {
    myAutoCloseCounter = 9;
    autoCloseTick();
  }

  if (!isVisible())
    show();
}

void AwayMsgDlg::showAutoResponseHints(QWidget* parent)
{
  QString h = tr(
    "<h2>Hints for Setting<br>your Auto-Response</h2><hr>"
    "<ul>"
    "<li>You can include any of the % expansions (described in the main hints page).</li>"

    "<li>Any line beginning with a pipe (|) will be treated as a command "
    "to be run.  The line will be replaced by the output of the command. "
    "The command is parsed by /bin/sh so any shell commands or meta-characters "
    "are allowed.  For security reasons, any % expansions are automatically "
    "passed to the command surrounded by single quotes to prevent shell parsing "
    "of any meta-characters included in an alias.<br>"
    "Examples of popular uses include:"
    "<ul>"
    "<li><tt>|date</tt>: Will replace that line by the current date</li>"
    "<li><tt>|fortune</tt>: Show a fortune, as a tagline for example</li>"
    "<li><tt>|myscript.sh %u %a</tt>: Run a script, passing the uin and alias</li>"
    "<li><tt>|myscript.sh %u %a > /dev/null</tt>: Run the same script but ignore the output"
        " (for tracking auto response checks or something)</li>"
    "<li><tt>|if [ %u -lt 100000 ]; then echo \"You are special\"; fi</tt>"
        ": Useless, but shows how you can use shell script.</li>"
    "</ul>"
    "Of course, multiple \"|\" can appear in the auto response, and commands and regular "
    "text can be mixed line by line.</li>"

    "<hr><p> For more information, see the Licq webpage (<tt>http://www.licq.org</tt>).</p>");

  new HintsDlg(h, parent);
}

bool AwayMsgDlg::eventFilter(QObject* /* watched */, QEvent* event)
{
  if (event->type() == QEvent::KeyPress ||
      event->type() == QEvent::MouseButtonPress ||
      event->type() == QEvent::Shortcut)
    autoCloseStop();

  return false;
}

void AwayMsgDlg::ok()
{
  myAutoCloseCounter = -1;

  bool invisible = (myStatus & User::InvisibleStatus) != 0;

  QString s = myAwayMsg->toPlainText().trimmed();
  if (myPpid == 0)
    gLicqGui->changeStatus(myStatus, invisible, s);
  else
  {
    Licq::UserId userId = Licq::gUserManager.ownerUserId(myPpid);
    gLicqGui->changeStatus(myStatus, userId, invisible, s);
  }

  close();
}

void AwayMsgDlg::autoCloseTick()
{
  if (myAutoCloseCounter >= 0)
  {
    myButtons->button(QDialogButtonBox::Ok)->
      setText(tr("(Closing in %1)").arg(myAutoCloseCounter--));

    if (myAutoCloseCounter < 0)
      ok();
    else
      QTimer::singleShot(1000, this, SLOT(autoCloseTick()));
  }
}

void AwayMsgDlg::autoCloseStop()
{
  if (myAutoCloseCounter >= 0)
  {
     myAutoCloseCounter = -1;
     myButtons->button(QDialogButtonBox::Ok)->setText(myOkText);
  }
}

void AwayMsgDlg::hints()
{
  showAutoResponseHints(this);
}

void AwayMsgDlg::selectMessage()
{
  QAction* a = qobject_cast<QAction*>(sender());
  if (a == NULL)
    return;

  QString text = a->data().toString();

  if (text.isNull()) // User chose "Edit Items"
    SettingsDlg::show(SettingsDlg::RespMsgPage);
  else
    myAwayMsg->setText(text);
}
