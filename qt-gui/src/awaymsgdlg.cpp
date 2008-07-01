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

#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qapplication.h>
#include <qtextcodec.h>

#include "awaymsgdlg.h"
#include "gui-strings.h"
#include "licq_log.h"
#include "mainwin.h"
#include "mledit.h"
#include "optionsdlg.h"
#include "usercodec.h"
#include "licq_sar.h"
#include "licq_user.h"
#include "licq_icqd.h"

static const char hints[] = QT_TRANSLATE_NOOP("AwayMsgDlg",
 "<h2>Hints for Setting<br>your Auto-Response</h2><br><hr><br>"
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
 "<li><tt>|myscript.sh %u %a > /dev/null</tt>: Run the same script but ignore the output (for tracking auto response checks or something)</li>"
 "<li><tt>|if [ %u -lt 100000 ]; then echo \"You are special\"; fi</tt>: Useless, but shows how you can use shell script.</li>"
 "</ul>"
 "Of course, multiple \"|\" can appear in the auto response, and commands and regular "
 "text can be mixed line by line.</li>"

 "<hr><p> For more information, see the Licq webpage (<tt>http://www.licq.org</tt>).</p>");


// -----------------------------------------------------------------------------

QPoint AwayMsgDlg::snPos = QPoint();

// -----------------------------------------------------------------------------

AwayMsgDlg::AwayMsgDlg(QWidget *parent)
  : LicqDialog(parent, "AwayMessageDialog", false, WDestructiveClose)
{
  QBoxLayout* top_lay = new QVBoxLayout(this, 10);

  mleAwayMsg = new MLEditWrap(true, this);
  connect(mleAwayMsg, SIGNAL(signal_CtrlEnterPressed()), this, SLOT(ok()));
  top_lay->addWidget(mleAwayMsg);

  mnuSelect = new QPopupMenu(this);
  connect(mnuSelect, SIGNAL(activated(int)), this, SLOT(slot_selectMessage(int)));

  QBoxLayout* l = new QHBoxLayout(top_lay, 10);
  int bw = QPushButton(tr("(Closing in %1)"), this).sizeHint().width();
  btnSelect = new QPushButton(tr("&Select"), this);
  btnSelect->setPopup(mnuSelect);
  QPushButton *btnHints = new QPushButton(tr("&Hints"), this);
  connect(btnHints, SIGNAL(clicked()), SLOT(slot_hints()));
  btnOk = new QPushButton(tr("&Ok"), this );
  btnOk->setDefault(true);
  connect( btnOk, SIGNAL(clicked()), SLOT(ok()) );
  btnCancel = new QPushButton(tr("&Cancel"), this );
  connect( btnCancel, SIGNAL(clicked()), SLOT(close()) );
  bw = QMAX(bw, btnSelect->sizeHint().width());
  bw = QMAX(bw, btnOk->sizeHint().width());
  bw = QMAX(bw, btnCancel->sizeHint().width());
  bw = QMAX(bw, btnHints->sizeHint().width());
  btnSelect->setFixedWidth(bw);
  btnOk->setFixedWidth(bw);
  btnCancel->setFixedWidth(bw);
  btnHints->setFixedWidth(bw);

  l->addWidget(btnSelect);
  l->addStretch(1);
  l->addWidget(btnHints);
  l->addSpacing(20);
  l->addWidget(btnOk);
  l->addWidget(btnCancel);

  m_autocloseCounter = -1;
  installEventFilter(this);
  mleAwayMsg->installEventFilter(this);
  connect(mleAwayMsg, SIGNAL(clicked(int, int)), SLOT(slot_autocloseStop()));
  connect(mnuSelect, SIGNAL(aboutToShow()), SLOT(slot_autocloseStop()));
  connect(btnHints, SIGNAL(clicked()), SLOT(slot_autocloseStop()));
}


// -----------------------------------------------------------------------------

void AwayMsgDlg::SelectAutoResponse(unsigned short _status, bool autoclose)
{
  if((_status & 0xFF) == ICQ_STATUS_ONLINE || _status == ICQ_STATUS_OFFLINE)
    _status = (_status & 0xFF00) | ICQ_STATUS_AWAY;

  m_nStatus = _status;

  // Fill in the select menu
  mnuSelect->clear();
  switch (m_nStatus)
  {
    case ICQ_STATUS_NA: m_nSAR = SAR_NA; break;
    case ICQ_STATUS_OCCUPIED: m_nSAR = SAR_OCCUPIED; break;
    case ICQ_STATUS_DND: m_nSAR = SAR_DND; break;
    case ICQ_STATUS_FREEFORCHAT: m_nSAR = SAR_FFC; break;
    case ICQ_STATUS_AWAY:
    default:
      m_nSAR = SAR_AWAY;
  }

  if (m_nSAR >= 0) {
    SARList &sar = gSARManager.Fetch(m_nSAR);
    for (unsigned i = 0; i < sar.size(); i++)
      mnuSelect->insertItem(QString::fromLocal8Bit(sar[i]->Name()), i);
    gSARManager.Drop();
  }

  mnuSelect->insertSeparator();
  mnuSelect->insertItem(tr("&Edit Items"), 999);

  ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
  if (o == 0) return;
  setCaption(QString(tr("Set %1 Response for %2"))
             .arg(Strings::getStatus(m_nStatus, false))
             .arg(QString::fromUtf8(o->GetAlias())));
  QTextCodec *codec = UserCodec::defaultEncoding();
  if (*o->AutoResponse())
    mleAwayMsg->setText(codec->toUnicode(o->AutoResponse()));
  else
    mleAwayMsg->setText(tr("I'm currently %1, %a.\n"
                           "You can leave me a message.\n"
                           "(%m messages pending from you).")
                        .arg(Strings::getStatus(m_nStatus, false)));
  gUserManager.DropOwner(o);

  mleAwayMsg->setFocus();
  QTimer::singleShot(0, mleAwayMsg, SLOT(selectAll()));

  if (autoclose) {
    m_autocloseCounter = 9;
    slot_autocloseTick();
  }

  if (!isVisible())
  {
    if (!snPos.isNull()) move(snPos);
    show();
  }
}

AwayMsgDlg::~AwayMsgDlg()
{
  emit done();
}

void AwayMsgDlg::slot_hints()
{
  QString h = tr(hints);
  (void) new HintsDlg(h);
}


// -----------------------------------------------------------------------------

void AwayMsgDlg::ok()
{
  m_autocloseCounter = -1;
  QString s = mleAwayMsg->text();
  while (s[s.length()-1].isSpace())
    s.truncate(s.length()-1);

  ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
  if (o == 0)
  {
    close();
    return;
  }
  QTextCodec *codec = UserCodec::defaultEncoding();
  o->SetAutoResponse(codec->fromUnicode(s));
  gUserManager.DropOwner(o);
  close();
}

void AwayMsgDlg::reject()
{
  m_autocloseCounter = -1;
  QTimer::singleShot(0, this, SLOT(close()));
}

// -----------------------------------------------------------------------------

void AwayMsgDlg::slot_selectMessage(int result)
{
  if (result == 999)
    emit popupOptions(OptionsDlg::ODlgStatus);
  else
  {
    SARList &sar = gSARManager.Fetch(m_nSAR);
    if ((unsigned) result < sar.size())
      mleAwayMsg->setText(QString::fromLocal8Bit(sar[result]->AutoResponse()));

    gSARManager.Drop();
  }
}

void AwayMsgDlg::closeEvent(QCloseEvent *e)
{
  e->accept();
}

bool AwayMsgDlg::eventFilter(QObject* /* obj */, QEvent *e)
{
  if (e->type() == QEvent::KeyPress ||
      e->type() == QEvent::MouseButtonPress ||
      e->type() == QEvent::Accel)
  {      
    slot_autocloseStop();
  }
  return false;
}

void AwayMsgDlg::slot_autocloseTick()
{
  if (m_autocloseCounter >= 0)
  {
    btnOk->setText(tr("(Closing in %1)").arg(m_autocloseCounter));
    m_autocloseCounter--;
    if (m_autocloseCounter < 0)
      ok();
    else
      QTimer::singleShot(1000, this, SLOT(slot_autocloseTick()));
  }
}

void AwayMsgDlg::slot_autocloseStop()
{
  if (m_autocloseCounter >= 0)
  {
     m_autocloseCounter = -1;
     btnOk->setText(tr("&Ok"));
  }
}


// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
CustomAwayMsgDlg::CustomAwayMsgDlg(const char *szId,
    unsigned long nPPID, QWidget *parent)
    : LicqDialog(parent, "CustomAwayMessageDialog", false, WDestructiveClose)
{
  m_szId = szId ? strdup(szId) : 0;
  m_nPPID = nPPID;

  QBoxLayout* top_lay = new QVBoxLayout(this, 10);

  mleAwayMsg = new MLEditWrap(true, this);
  connect(mleAwayMsg, SIGNAL(signal_CtrlEnterPressed()), this, SLOT(slot_ok()));
  top_lay->addWidget(mleAwayMsg);

  QBoxLayout* l = new QHBoxLayout(top_lay, 10);

  int bw = 75;
  QPushButton *btnHints = new QPushButton(tr("&Hints"), this);
  connect(btnHints, SIGNAL(clicked()), SLOT(slot_hints()));
  QPushButton *btnOk = new QPushButton(tr("&Ok"), this );
  btnOk->setDefault(true);
  connect( btnOk, SIGNAL(clicked()), SLOT(slot_ok()) );
  QPushButton *btnClear = new QPushButton(tr("&Clear"), this );
  connect( btnClear, SIGNAL(clicked()), SLOT(slot_clear()) );
  QPushButton *btnCancel = new QPushButton(tr("&Cancel"), this );
  connect( btnCancel, SIGNAL(clicked()), SLOT(close()) );
  bw = QMAX(bw, btnOk->sizeHint().width());
  bw = QMAX(bw, btnClear->sizeHint().width());
  bw = QMAX(bw, btnCancel->sizeHint().width());
  bw = QMAX(bw, btnHints->sizeHint().width());
  btnOk->setFixedWidth(bw);
  btnClear->setFixedWidth(bw);
  btnCancel->setFixedWidth(bw);
  btnHints->setFixedWidth(bw);

  l->addStretch(1);
  l->addSpacing(30);
  l->addWidget(btnHints);
  l->addSpacing(20);
  l->addWidget(btnOk);
  l->addWidget(btnClear);
  l->addWidget(btnCancel);

  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);
  setCaption(QString(tr("Set Custom Auto Response for %1"))
             .arg(QString::fromUtf8(u->GetAlias())));
  if (*u->CustomAutoResponse())
    mleAwayMsg->setText(QString::fromLocal8Bit(u->CustomAutoResponse()));
  else if (u->StatusToUser() != ICQ_STATUS_OFFLINE)
    mleAwayMsg->setText(tr("I am currently %1.\nYou can leave me a message.")
                        .arg(Strings::getStatus(u->StatusToUser(), false)));

  gUserManager.DropUser(u);

  mleAwayMsg->setFocus();
  QTimer::singleShot(0, mleAwayMsg, SLOT(selectAll()));

  show();
}

void CustomAwayMsgDlg::slot_hints()
{
  QString h = tr(hints);
  (void) new HintsDlg(h);
}


void CustomAwayMsgDlg::slot_ok()
{
  QString s = mleAwayMsg->text();
  while(s[s.length()-1].isSpace())
    s.truncate(s.length()-1);

  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_W);
  if (u != NULL)
  {
    u->SetCustomAutoResponse(s.local8Bit());
    gUserManager.DropUser(u);
    CICQSignal sig(SIGNAL_UPDATExUSER, USER_BASIC, m_szId, m_nPPID);
    gMainWindow->slot_updatedUser(&sig);
  }
  close();
}


void CustomAwayMsgDlg::slot_clear()
{
  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_W);
  if (u != NULL)
  {
    u->ClearCustomAutoResponse();
    gUserManager.DropUser(u);
    CICQSignal sig(SIGNAL_UPDATExUSER, USER_BASIC, m_szId, m_nPPID);
    gMainWindow->slot_updatedUser(&sig);
  }
  close();
}


#include "awaymsgdlg.moc"
