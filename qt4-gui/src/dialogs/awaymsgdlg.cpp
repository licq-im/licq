// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2006 Licq developers
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

#include <QCloseEvent>
#include <QDialogButtonBox>
#include <QEvent>
#include <QHBoxLayout>
#include <QMenu>
#include <QPushButton>
#include <QTextCodec>
#include <QTimer>
#include <QVBoxLayout>

#include <licq_icqd.h>
#include <licq_log.h>
#include <licq_sar.h>
#include <licq_user.h>

#include "core/licqgui.h"

#include "helpers/licqstrings.h"
#include "helpers/support.h"
#include "helpers/usercodec.h"

#include "settings/settingsdlg.h"

#include "widgets/mledit.h"

#include "hintsdlg.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::AwayMsgDlg */

AwayMsgDlg* AwayMsgDlg::myInstance = NULL;

void AwayMsgDlg::showAwayMsgDlg(unsigned short status, bool autoClose,
    unsigned long ppid, bool invisible, bool setStatus)
{
  if (myInstance == NULL)
    myInstance = new AwayMsgDlg();
  else
    myInstance->raise();

  myInstance->selectAutoResponse(status, autoClose, ppid, invisible, setStatus);
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

void AwayMsgDlg::selectAutoResponse(unsigned short status, bool autoClose,
    unsigned long ppid, bool invisible, bool setStatus)
{
  switch (status & 0x00FF)
  {
    case ICQ_STATUS_ONLINE: // Fall through
    case ICQ_STATUS_OFFLINE:
      status = (status & 0xFF00) | ICQ_STATUS_AWAY;
      break;
  }

  myStatus = status;
  myInvisible = invisible;
  myPpid = ppid;
  mySetStatus = setStatus;

  // Fill in the select menu
  myMenu->clear();
  switch (myStatus)
  {
    case ICQ_STATUS_NA:
      mySAR = SAR_NA;
      break;
    case ICQ_STATUS_OCCUPIED:
      mySAR = SAR_OCCUPIED;
      break;
    case ICQ_STATUS_DND:
      mySAR = SAR_DND;
      break;
    case ICQ_STATUS_FREEFORCHAT:
      mySAR = SAR_FFC;
      break;
    case ICQ_STATUS_AWAY: // Fall through
    default:
      mySAR = SAR_AWAY;
  }

  if (mySAR >= 0)
  {
    SARList& sar = gSARManager.Fetch(mySAR);
    for (unsigned i = 0; i < sar.size(); i++)
    {
      QAction* a = myMenu->addAction(
          QString::fromLocal8Bit(sar[i]->Name()),
          this, SLOT(selectMessage()));
      a->setData(i);
    }
    gSARManager.Drop();
  }

  myMenu->addSeparator();
  QAction* a = myMenu->addAction(tr("&Edit Items"), this, SLOT(selectMessage()));
  a->setData(999);

  const ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
  if (o == NULL)
    return;

  setWindowTitle(QString(tr("Set %1 Response for %2"))
      .arg(LicqStrings::getStatus(myStatus, false))
      .arg(QString::fromUtf8(o->GetAlias())));

  QTextCodec* codec = UserCodec::defaultEncoding();
  if (*o->AutoResponse())
    myAwayMsg->setText(codec->toUnicode(o->AutoResponse()));
  else
    myAwayMsg->setText(tr("I'm currently %1, %a.\n"
          "You can leave me a message.\n"
          "(%m messages pending from you).")
        .arg(LicqStrings::getStatus(myStatus, false)));
  gUserManager.DropOwner(o);

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

  if (mySetStatus)
  {
    if (myPpid == 0)
      LicqGui::instance()->changeStatus(myStatus, myInvisible);
    else
      LicqGui::instance()->changeStatus(myStatus, myPpid, myInvisible);
  }

  QString s = myAwayMsg->toPlainText().trimmed();

  ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
  if (o != NULL)
  {
    QTextCodec* codec = UserCodec::defaultEncoding();
    o->SetAutoResponse(codec->fromUnicode(s));
    gUserManager.DropOwner(o);
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

  unsigned int result = a->data().toUInt();

  if (result == 999) // User chose "Edit Items"
    SettingsDlg::show(SettingsDlg::RespMsgPage);
  else
  {
    SARList& sar = gSARManager.Fetch(mySAR);
    if (result < sar.size())
      myAwayMsg->setText(QString::fromLocal8Bit(sar[result]->AutoResponse()));

    gSARManager.Drop();
  }
}
