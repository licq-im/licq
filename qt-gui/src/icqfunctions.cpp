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

#include <time.h>

#ifdef USE_KDE
#include <kfiledialog.h>
#else
#include <qfiledialog.h>
#endif

#include <qhbox.h>
#include <qvbox.h>
#include <qhgroupbox.h>
#include <qvgroupbox.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qsplitter.h>
#include <qtabbar.h>
#include <qtabwidget.h>
#include <qwidgetstack.h>
#include <qstylesheet.h>
#include <qlayout.h>

#include <qtimer.h>
#include <qspinbox.h>

#include "icqfunctions.h"
#include "chatdlg.h"
#include "ewidgets.h"
#include "messagebox.h"
#include "filedlg.h"
#include "showawaymsgdlg.h"
#include "licq_log.h"
#include "sigman.h"
//#include "editfile.h"
#include "eventdesc.h"
//#include "gui-defines.h"
#include "refusedlg.h"
#include "forwarddlg.h"
#include "chatjoin.h"
#include "mainwin.h"
#include "mmlistview.h"
#include "mmsenddlg.h"
#include "authuserdlg.h"
#include "userinfodlg.h"
#include "usereventdlg.h"

//#include "licq_user.h"
#include "mledit.h"
#include "licq_events.h"
#include "licq_icqd.h"

//#define TEST_POS

unsigned short ICQFunctions::s_nX = 100;
unsigned short ICQFunctions::s_nY = 100;

//-----ICQFunctions::constructor---------------------------------------------
ICQFunctions::ICQFunctions(CICQDaemon *s, CSignalManager *theSigMan,
                           CMainWindow *m, unsigned long _nUin,
                           bool isAutoClose, QWidget *parent)
  : QWidget(parent, "UserEventDialog")
{
  server = s;
  mainwin = m;
  sigman = theSigMan;
  icqEventTag = NULL;
  m_nUin = _nUin;
  m_bOwner = (m_nUin == gUserManager.OwnerUin());
  m_xCurrentReadEvent = NULL;
  m_nMPChatPort = 0;
  m_bDeleteUser = false;

  for (unsigned short i = 0; i < 8; i++)
  {
    tabList[i].loaded = false;
    tabList[i].tab = NULL;
  }

  CreateReadEventTab();
  CreateSendEventTab();

  QBoxLayout* lay = new QVBoxLayout(this, 8);

  QBoxLayout *layt = new QHBoxLayout(lay, 8);
  layt->addWidget(new QLabel(tr("Status:"), this));
  nfoStatus = new CInfoField(this, true);
  layt->addWidget(nfoStatus);
  layt->addWidget(new QLabel(tr("Time:"), this));
  nfoTimezone = new CInfoField(this, true);
  layt->addWidget(nfoTimezone);

  tabs = new QTabWidget(this);
  lay->addWidget(tabs);

  tabs->addTab(tabList[TAB_READ].tab, tabList[TAB_READ].label);
  tabs->addTab(tabList[TAB_SEND].tab, tabList[TAB_SEND].label);

  QBoxLayout* l = new QHBoxLayout(lay, 8);

  chkAutoClose = new QCheckBox(tr("Aut&o Close"), this);
  chkAutoClose->setChecked(isAutoClose);
  l->addWidget(chkAutoClose);
  l->addSpacing(15);
  l->addStretch(1);
  if (m_bOwner) chkAutoClose->hide();

  int bw = 75;
  btnOk = new QPushButton(tr("O&k"), this);
  btnCancel = new CEButton(tr("&Close"), this);
  bw = QMAX(bw, btnOk->sizeHint().width());
  bw = QMAX(bw, btnCancel->sizeHint().width());
  btnOk->setFixedWidth(bw);
  btnCancel->setFixedWidth(bw);
  l->addSpacing(7);
  l->addWidget(btnOk);
  l->addWidget(btnCancel);

  setTabOrder(mleSend, btnOk);

  connect (mleSend, SIGNAL(signal_CtrlEnterPressed()), btnOk, SIGNAL(clicked()));
#ifdef USE_SPOOFING
  connect (chkSpoof, SIGNAL(clicked()), this, SLOT(setSpoofed()));
#endif
  connect (tabs, SIGNAL(selected(const QString &)), this, SLOT(tabSelected(const QString &)));
  connect (sigman, SIGNAL(signal_updatedUser(CICQSignal *)),
           this, SLOT(slot_updatedUser(CICQSignal *)));
  connect (btnCancel, SIGNAL(clicked()), this, SLOT(slot_close()));
  connect (btnOk, SIGNAL(clicked()), this, SLOT(callFcn()));
}


void ICQFunctions::CreateReadEventTab()
{
  tabList[TAB_READ].label = tr("&View");
  QVBox *p = new QVBox(this, tabList[TAB_READ].label.latin1());
  p->setMargin(8);
  p->setSpacing(5);
  tabList[TAB_READ].tab = p;
  tabList[TAB_READ].loaded = true;

  QHGroupBox *h_top = new QHGroupBox(/*tr("Conversation"),*/ p);
  msgView = new MsgView(h_top);
  /*btnReadNext = new QPushButton(tr("Nex&t"), h_top);
  btnReadNext->setEnabled(false);
  btnReadNext->setFixedHeight(msgView->height());*/
  //btnReadNext->setFixedWidth(btnReadNext->width());
  //connect(btnReadNext, SIGNAL(clicked()), this, SLOT(slot_nextMessage()));

  QHGroupBox *h_msg = new QHGroupBox(p);
  mleRead = new MLEditWrap(true, h_msg, true);
  mleRead->setReadOnly(true);
#if QT_VERSION >= 210
  p->setStretchFactor(h_msg, 1);
#endif

#if QT_VERSION >= 210
  connect (msgView, SIGNAL(clicked(QListViewItem *)), this, SLOT(slot_printMessage(QListViewItem *)));
#else
  connect (msgView, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(slot_printMessage(QListViewItem *)));
#endif

  QHGroupBox *h_action = new QHGroupBox(/*tr("Actions"),*/ p);
  btnRead1 = new QPushButton(h_action);
  btnRead2 = new QPushButton(h_action);
  btnRead3 = new QPushButton(h_action);
  btnRead4 = new QPushButton(h_action);

  btnRead1->setEnabled(false);
  btnRead2->setEnabled(false);
  btnRead3->setEnabled(false);
  btnRead4->setEnabled(false);

  connect(btnRead1, SIGNAL(clicked()), this, SLOT(slot_readbtn1()));
  connect(btnRead2, SIGNAL(clicked()), this, SLOT(slot_readbtn2()));
  connect(btnRead3, SIGNAL(clicked()), this, SLOT(slot_readbtn3()));
  connect(btnRead4, SIGNAL(clicked()), this, SLOT(slot_readbtn4()));
}


void ICQFunctions::CreateSendEventTab()
{
  tabList[TAB_SEND].label = tr("S&end");
  tabList[TAB_SEND].tab = new QWidget(this, tabList[TAB_SEND].label.latin1());
  laySend = new QGridLayout(tabList[TAB_SEND].tab, 5, 2, 8);
  tabList[TAB_SEND].loaded = true;

  grpCmd = new QButtonGroup(1, Vertical, tabList[TAB_SEND].tab);
  rdbMsg = new QRadioButton(tr("&Message"), grpCmd);
  rdbUrl = new QRadioButton(tr("&URL"), grpCmd);
  rdbChat = new QRadioButton(tr("Chat Re&quest"), grpCmd);
  rdbFile = new QRadioButton(tr("&File Transfer"), grpCmd);
  connect(grpCmd, SIGNAL(clicked(int)), this, SLOT(specialFcn(int)));
#if QT_VERSION < 210
  QWidget* dummy_w = new QWidget(grpCmd);
  dummy_w->setMinimumHeight(2);
#endif
  laySend->addMultiCellWidget(grpCmd, 0, 0, 0, 1);

  grpMR = NULL;

  QHGroupBox *h_mid_left = new QHGroupBox(tabList[TAB_SEND].tab);
  mleSend = new MLEditWrap(true, h_mid_left, true);
  mleSend->setMinimumHeight(150);

  laySend->addWidget(h_mid_left, 1, 0);
  laySend->setColStretch(0, 1);

  grpOpt = NULL;

  QGroupBox *box = new QGroupBox(tabList[TAB_SEND].tab);
  QBoxLayout *vlay = new QVBoxLayout(box, 10, 5);
  QBoxLayout *hlay = new QHBoxLayout(vlay);
  chkSendServer = new QCheckBox(tr("Se&nd through server"), box);
  hlay->addWidget(chkSendServer);
  chkUrgent = new QCheckBox(tr("U&rgent"), box);
  hlay->addWidget(chkUrgent);
  chkMass = new QCheckBox(tr("&Multiple recipients"), box);
  hlay->addWidget(chkMass);
  connect(chkMass, SIGNAL(toggled(bool)), this, SLOT(slot_masstoggled(bool)));
  laySend->addMultiCellWidget(box, 3, 3, 0, 1);

#ifdef USE_SPOOFING
  hlay = new QHBoxLayout(vlay);
  chkSpoof = new QCheckBox(tr("S&poof UIN:"), box);
  hlay->addWidget(chkSpoof);
  edtSpoof = new QLineEdit(box);
  hlay->addWidget(edtSpoof);
  edtSpoof->setEnabled(false);
  edtSpoof->setValidator(new QIntValidator(10000, 2000000000, edtSpoof));
  connect(chkSpoof, SIGNAL(toggled(bool)), edtSpoof, SLOT(setEnabled(bool)));
#else
  edtSpoof = NULL;
  chkSpoof = NULL;
#endif
  laySend->activate();
}


void ICQFunctions::SendTab_grpOpt()
{
  grpOpt = new QGroupBox(3, Horizontal, tabList[TAB_SEND].tab);
  lblItem = new QLabel(grpOpt);
  edtItem = new CInfoField(grpOpt, false);
  btnItem = new QPushButton(grpOpt);
  connect(btnItem, SIGNAL(clicked()), SLOT(slot_sendbtn()));
#if QT_VERSION < 210
  QWidget* dummy_w2 = new QWidget(grpOpt);
  dummy_w2->setMinimumHeight(2);
#endif
  laySend->addMultiCellWidget(grpOpt, 2, 2, 0, 1);
  //laySend->addWidget(grpOpt, 2, 0);
}


void ICQFunctions::SendTab_grpMR()
{
  grpMR = new QVGroupBox(tabList[TAB_SEND].tab);
  (void) new QLabel(tr("Drag Users Here\nRight Click for Options"), grpMR);
  lstMultipleRecipients = new CMMUserView(mainwin->UserView()->ColInfo(),
     mainwin->showHeader, m_nUin, mainwin, grpMR);
  lstMultipleRecipients->setFixedWidth(mainwin->UserView()->width());
  laySend->addWidget(grpMR, 1, 1);
  //laySend->addMultiCellWidget(grpMR, 1, 2, 1, 1);
}


void ICQFunctions::slot_masstoggled(bool b)
{
  if (grpMR == NULL) SendTab_grpMR();
  b ? grpMR->show() : grpMR->hide();
}





//-----ICQFunctions::keyPressEvent----------------------------------------------
void ICQFunctions::keyPressEvent(QKeyEvent *e)
{
  if (e->key() == Key_Escape)
  {
    close();
    return;
  }
  else if (tabs->currentPage() == tabList[TAB_READ].tab && !e->text().isEmpty())
  {
    tabs->showPage(tabList[TAB_SEND].tab);
    return;
  }
  QWidget::keyPressEvent(e);
}

//-----ICQFunctions::setupTabs--------------------------------------------------
void ICQFunctions::setupTabs(int index)
{
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);

  // read tab
  if (index == mnuUserView)
  {
    /*if (u->NewMessages() > 0)
    {
      gUserManager.DropUser(u);
      slot_nextMessage();
      u = gUserManager.FetchUser(m_nUin, LOCK_R);
    }*/
    if (u->NewMessages() > 0)
    {
      MsgViewItem *e = new MsgViewItem(u->EventPeek(0), msgView);
      for (unsigned short i = 1; i < u->NewMessages(); i++)
      {
        (void) new MsgViewItem(u->EventPeek(i), msgView);
      }
      gUserManager.DropUser(u);
      slot_printMessage(e);
      u = gUserManager.FetchUser(m_nUin, LOCK_R);
      msgView->setSelected(e, true);
      msgView->ensureItemVisible(e);
    }
  }
  //if (u->NewMessages() > 0) btnReadNext->setEnabled(true);

  // Send tab
  rdbFile->setEnabled(!u->StatusOffline());
  rdbChat->setEnabled(!u->StatusOffline());
  if (chkSendServer->isEnabled())
  {
    chkSendServer->setChecked(u->SendServer() || (u->StatusOffline() && u->SocketDesc() == -1));
  }
  if (u->Ip() == 0)
  {
    chkSendServer->setChecked(true);
    chkSendServer->setEnabled(false);
  }
  rdbMsg->setChecked(true);
  specialFcn(0);
  tabs->setTabEnabled(tabList[TAB_SEND].tab, !m_bOwner);

  // Info tabs
  SetInfo(u);

  bool bNewUser = u->NewUser();
  gUserManager.DropUser(u);

  // mark the user as no longer new if they are new
  if (bNewUser)
  {
    u = gUserManager.FetchUser(m_nUin, LOCK_W);
    u->SetNewUser(false);
    gUserManager.DropUser(u);
    CICQSignal s(SIGNAL_UPDATExUSER, USER_BASIC, m_nUin);
    emit signal_updatedUser(&s);
  }
  move (s_nX, s_nY);
  show();

  switch(index)
  {
    case mnuUserView:
      tabs->showPage(tabList[TAB_READ].tab);
      break;
    case mnuUserSendMsg:
      tabs->showPage(tabList[TAB_SEND].tab);
      rdbMsg->setChecked(true);
      specialFcn(0);
      break;
    case mnuUserSendUrl:
      tabs->showPage(tabList[TAB_SEND].tab);
      rdbUrl->setChecked(true);
      specialFcn(1);
      break;
    case mnuUserSendChat:
      tabs->showPage(tabList[TAB_SEND].tab);
      rdbChat->setChecked(true);
      specialFcn(2);
      break;
    case mnuUserSendFile:
      tabs->showPage(tabList[TAB_SEND].tab);
      rdbFile->setChecked(true);
      specialFcn(3);
      break;
    default:
      gLog.Warn("%sInternal Error: ICQFunctions::setupTabs(): Invalid index (%d).\n",
                L_WARNxSTR, index);
  }

}

void ICQFunctions::SetInfo(ICQUser *u)
{
  bool bDropUser = false;

  if (u == NULL)
  {
    u = gUserManager.FetchUser(m_nUin, LOCK_R);
    bDropUser = true;
  }

  nfoStatus->setData(u->StatusStr());
  if (u->GetTimezone() == TIMEZONE_UNKNOWN)
  {
    nfoTimezone->setText(tr("Unknown"));
  }
  else
  {
    m_nRemoteTimeOffset = u->LocalTimeOffset();
    QDateTime t;
    t.setTime_t(u->LocalTime());
    nfoTimezone->setText(tr("%1 (GMT%1%1%1)")
                         .arg(t.time().toString())
                         .arg(u->GetTimezone() > 0 ? "-" : "+")
                         .arg(abs(u->GetTimezone() / 2))
                         .arg(u->GetTimezone() % 2 ? "30" : "00") );
    tmrTime = new QTimer(this);
    connect(tmrTime, SIGNAL(timeout()), this, SLOT(slot_updatetime()));
    tmrTime->start(1000);
  }

  m_sBaseTitle = QString::fromLocal8Bit(u->GetAlias()) + " (" +
    QString::fromLocal8Bit(u->GetFirstName()) + " " +
    QString::fromLocal8Bit(u->GetLastName())+ ")";
  setCaption(m_sBaseTitle);
  setIconText(u->GetAlias());

  if (bDropUser) gUserManager.DropUser(u);
}

void ICQFunctions::slot_updatetime()
{
  QDateTime t;
  t.setTime_t(time(NULL) + m_nRemoteTimeOffset);
  nfoTimezone->setText(nfoTimezone->text().replace(0, t.time().toString().length(), t.time().toString()));
}


//-----ICQFunctions::SendMsg---------------------------------------------------
void ICQFunctions::SendMsg(QString msg)
{
  tabs->showPage(tabList[TAB_SEND].tab);
  rdbMsg->setChecked(true);
  specialFcn(0);
  mleSend->setText(msg);
}

//-----ICQFunctions::SendUrl---------------------------------------------------
void ICQFunctions::SendUrl(QString url, QString desc)
{
  tabs->showPage(tabList[TAB_SEND].tab);
  rdbUrl->setChecked(true);
  specialFcn(1);
  edtItem->setText(url);
  mleSend->setText(desc);
}

//-----ICQFunctions::SendFile--------------------------------------------------
void ICQFunctions::SendFile(QString file, QString desc)
{
  tabs->showPage(tabList[TAB_SEND].tab);
  rdbFile->setChecked(true);
  specialFcn(3);
  edtItem->setText(file);
  mleSend->setText(desc);
}


//-----ICQFunctions::tabSelected-----------------------------------------------
void ICQFunctions::tabSelected(const QString &tab)
{
  if (tab == tabList[TAB_SEND].label)
  {
     mleSend->setFocus();
     btnOk->setText(tr("&Send"));
     btnOk->show();
     currentTab = TAB_SEND;
  }

  else if (tab == tabList[TAB_READ].label)
  {
     btnOk->setText(tr("Ok"));
     btnOk->hide();
     msgView->triggerUpdate();
     currentTab = TAB_READ;
  }
}


//-----slot_updatedUser--------------------------------------------------------
void ICQFunctions::slot_updatedUser(CICQSignal *sig)
{
  if (m_nUin != sig->Uin()) return;

  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
  if(u == NULL) return;
  switch (sig->SubSignal())
  {
  case USER_STATUS:
  {
    nfoStatus->setData(u->StatusStr());
    if (u->Ip() == 0)
    {
      chkSendServer->setChecked(true);
      chkSendServer->setEnabled(false);
    }
    else
    {
      chkSendServer->setEnabled(true);
    }
    if (u->StatusOffline())
      chkSendServer->setChecked(true);
    break;
  }
  case USER_EVENTS:
  {
    /*if (u->NewMessages() > 1)
    {
      btnReadNext->setEnabled(true);
      btnReadNext->setText(tr("Nex&t\n(%1)").arg(u->NewMessages()));
    }
    else if (u->NewMessages() == 1)
    {
      btnReadNext->setEnabled(true);
      btnReadNext->setText(tr("Nex&t"));
    }
    else
    {
      btnReadNext->setEnabled(false);
      btnReadNext->setText(tr("Nex&t"));
    }*/
    if (sig->Argument() > 0)
    {
      CUserEvent *e = u->EventPeekId(sig->Argument());
      if (e != NULL)
      {
        MsgViewItem *m = new MsgViewItem(e, msgView);
        msgView->ensureItemVisible(m);
      }
      // If this is a new message, go to the view tab
      if (mleSend->text().length() == 0)
        tabs->showPage(tabList[TAB_READ].tab);
    }
    else
    {
      // FIXME we should probably remove the event now...
    }
    break;
  }
  }
  gUserManager.DropUser(u);
}


/*
void ICQFunctions::slot_nextMessage()
{
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_W);
  if (u->NewMessages() == 0)
  {
    gUserManager.DropUser(u);
    return;
  }
  MsgViewItem *e = new MsgViewItem(u->EventPop(), msgView);
  btnReadNext->setEnabled(u->NewMessages() > 0);
  if (u->NewMessages() > 1)
  {
    btnReadNext->setEnabled(true);
    btnReadNext->setText(tr("Nex&t\n(%1)").arg(u->NewMessages()));
  }
  else if (u->NewMessages() == 1)
  {
    btnReadNext->setEnabled(true);
    btnReadNext->setText(tr("Nex&t"));
  }
  else
  {
    btnReadNext->setEnabled(false);
    btnReadNext->setText(tr("Nex&t"));
  }

  gUserManager.DropUser(u);

  msgView->setSelected(e, true);
  slot_printMessage(e);
}
*/


//-----ICQFunctions::printMessage----------------------------------------------
void ICQFunctions::slot_printMessage(QListViewItem *eq)
{
  if (eq == NULL)
    return;

  MsgViewItem *e = (MsgViewItem *)eq;

  btnRead1->setText("");
  btnRead2->setText("");
  btnRead3->setText("");
  btnRead4->setText("");
  btnRead1->setEnabled(false);
  btnRead2->setEnabled(false);
  btnRead3->setEnabled(false);
  btnRead4->setEnabled(false);

  CUserEvent *m = e->msg;
  m_xCurrentReadEvent = m;
  mleRead->setText(QString::fromLocal8Bit(m->Text()));
  if (m->Direction() == D_RECEIVER && (m->Command() == ICQ_CMDxTCP_START || m->Command() == ICQ_CMDxRCV_SYSxMSGxONLINE))
  {
    switch (m->SubCommand())
    {
      case ICQ_CMDxSUB_CHAT:  // accept or refuse a chat request
      case ICQ_CMDxSUB_FILE:  // accept or refuse a file transfer
        if (m->IsCancelled())
        {
          mleRead->append(tr("\n--------------------\nRequest was cancelled."));
        }
        else
        {
          btnRead1->setText(tr("A&ccept"));
          btnRead2->setText(tr("&Refuse"));
          // If this is a chat, and we already have chats going, and this is
          // not a join request, then we can join
          if (m->SubCommand() == ICQ_CMDxSUB_CHAT &&
              ChatDlg::chatDlgs.size() > 0 &&
              ((CEventChat *)m)->Port() == 0)
            btnRead3->setText(tr("&Join"));
        }
        break;

      case ICQ_CMDxSUB_MSG:
        btnRead1->setText(tr("&Quote"));
        btnRead2->setText(tr("&Forward"));
        break;

      case ICQ_CMDxSUB_URL:   // view a url
        btnRead1->setText(tr("&Quote"));
        btnRead2->setText(tr("&Forward"));
        if (server->getUrlViewer() != NULL)
          btnRead3->setText(tr("&View"));
        break;

      case ICQ_CMDxSUB_AUTHxREQUEST:
      {
        btnRead1->setText(tr("A&uthorize"));
        btnRead2->setText(tr("&Refuse"));
        ICQUser *u = gUserManager.FetchUser( ((CEventAuthRequest *)m)->Uin(), LOCK_R);
        if (u == NULL)
          btnRead3->setText(tr("A&dd User"));
        else
          gUserManager.DropUser(u);
        break;
      }
      case ICQ_CMDxSUB_AUTHxGRANTED:
      {
        ICQUser *u = gUserManager.FetchUser( ((CEventAuthGranted *)m)->Uin(), LOCK_R);
        if (u == NULL)
          btnRead1->setText(tr("A&dd User"));
        else
          gUserManager.DropUser(u);
        break;
      }
      case ICQ_CMDxSUB_ADDEDxTOxLIST:
      {
        ICQUser *u = gUserManager.FetchUser( ((CEventAdded *)m)->Uin(), LOCK_R);
        if (u == NULL)
          btnRead1->setText(tr("A&dd User"));
        else
          gUserManager.DropUser(u);
        break;
      }
    } // switch
  }  // if

  if (!btnRead1->text().isEmpty()) btnRead1->setEnabled(true);
  if (!btnRead2->text().isEmpty()) btnRead2->setEnabled(true);
  if (!btnRead3->text().isEmpty()) btnRead3->setEnabled(true);
  if (!btnRead4->text().isEmpty()) btnRead4->setEnabled(true);

  if (e->m_nEventId != -1 && e->msg->Direction() == D_RECEIVER)
  {
    ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_W);
    u->EventClearId(e->m_nEventId);
    gUserManager.DropUser(u);
    e->MarkRead();
  }
}


void ICQFunctions::slot_readbtn1()
{
  if (m_xCurrentReadEvent == NULL) return;

  switch (m_xCurrentReadEvent->SubCommand())
  {
    case ICQ_CMDxSUB_CHAT:  // accept a chat request
    {
      btnRead1->setEnabled(false);
      btnRead2->setEnabled(false);
      CEventChat *c = (CEventChat *)m_xCurrentReadEvent;
      ChatDlg *chatDlg = new ChatDlg(m_nUin, server);
      if (c->Port() != 0)  // Joining a multiparty chat (we connect to them)
      {
        if (chatDlg->StartAsClient(c->Port()))
          server->icqChatRequestAccept(m_nUin, chatDlg->LocalPort(), c->Sequence());
      }
      else  // single party (other side connects to us)
      {
        if (chatDlg->StartAsServer())
          server->icqChatRequestAccept(m_nUin, chatDlg->LocalPort(), c->Sequence());
      }
      break;
    }

    case ICQ_CMDxSUB_FILE:  // accept a file transfer
    {
      btnRead1->setEnabled(false);
      btnRead2->setEnabled(false);
      CEventFile *f = (CEventFile *)m_xCurrentReadEvent;
      CFileDlg *fileDlg = new CFileDlg(m_nUin, server);
      if (fileDlg->ReceiveFiles())
        server->icqFileTransferAccept(m_nUin, fileDlg->LocalPort(), f->Sequence());
      break;
    }

    case ICQ_CMDxSUB_MSG:
      generateReply();
      break;

    case ICQ_CMDxSUB_URL:
      generateReply();
      break;

    case ICQ_CMDxSUB_AUTHxREQUEST:
      //server->icqAuthorizeGrant( ((CEventAuthRequest *)m_xCurrentReadEvent)->Uin(), "" );
      (void) new AuthUserDlg(server, ((CEventAuthRequest *)m_xCurrentReadEvent)->Uin(), true);
      break;

    case ICQ_CMDxSUB_AUTHxGRANTED:
      server->AddUserToList( ((CEventAuthGranted *)m_xCurrentReadEvent)->Uin());
      break;

    case ICQ_CMDxSUB_ADDEDxTOxLIST:
      server->AddUserToList( ((CEventAdded *)m_xCurrentReadEvent)->Uin());
      break;
  } // switch

}


void ICQFunctions::slot_readbtn2()
{
  if (m_xCurrentReadEvent == NULL) return;

  switch (m_xCurrentReadEvent->SubCommand())
  {
    case ICQ_CMDxSUB_CHAT:  // refuse a chat request
    {
      CRefuseDlg *r = new CRefuseDlg(m_nUin, tr("Chat"), this);
      if (r->exec())
      {
        btnRead1->setEnabled(false);
        btnRead2->setEnabled(false);
        server->icqChatRequestRefuse(m_nUin, r->RefuseMessage().local8Bit(),
           m_xCurrentReadEvent->Sequence());
      }
      delete r;
      break;
    }

    case ICQ_CMDxSUB_FILE:  // refuse a file transfer
    {
      CRefuseDlg *r = new CRefuseDlg(m_nUin, tr("File Transfer"), this);
      if (r->exec())
      {
        btnRead1->setEnabled(false);
        btnRead2->setEnabled(false);
        server->icqFileTransferRefuse(m_nUin, r->RefuseMessage().local8Bit(),
           m_xCurrentReadEvent->Sequence());
      }
      delete r;
      break;
    }

    case ICQ_CMDxSUB_MSG:  // Forward
    case ICQ_CMDxSUB_URL:
    {
      CForwardDlg *f = new CForwardDlg(mainwin, m_xCurrentReadEvent);
      // Move should go here to avoid flicker, but qt is stupid and
      // will ignore any move()s until after a show().
      //f->move(x() + width() / 2 - f->width() / 2, y() + height() / 2 - f->height() / 2);
      f->show();
      f->move(x() + width() / 2 - f->width() / 2, y() + height() / 2 - f->height() / 2);
      break;
    }

    case ICQ_CMDxSUB_AUTHxREQUEST:
    {
      (void) new AuthUserDlg(server, ((CEventAuthRequest *)m_xCurrentReadEvent)->Uin(), false);
      break;
    }
  } // switch

}


void ICQFunctions::slot_readbtn3()
{
  if (m_xCurrentReadEvent == NULL) return;

  switch (m_xCurrentReadEvent->SubCommand())
  {
    case ICQ_CMDxSUB_URL:   // view a url
      if (!server->ViewUrl(((CEventUrl *)m_xCurrentReadEvent)->Url()))
        WarnUser(this, tr("View URL failed"));
      break;

    case ICQ_CMDxSUB_CHAT:  // join to current chat
    {
      CEventChat *c = (CEventChat *)m_xCurrentReadEvent;
      if (c->Port() != 0)  // Joining a multiparty chat (we connect to them)
      {
        ChatDlg *chatDlg = new ChatDlg(m_nUin, server);
        if (chatDlg->StartAsClient(c->Port()))
          server->icqChatRequestAccept(m_nUin, chatDlg->LocalPort(), c->Sequence());
      }
      else  // single party (other side connects to us)
      {
        ChatDlg *chatDlg = NULL;
        CJoinChatDlg *j = new CJoinChatDlg(this);
        if (j->exec() && (chatDlg = j->JoinedChat()) != NULL)
          server->icqChatRequestAccept(m_nUin, chatDlg->LocalPort(), c->Sequence());
        delete j;
      }
      break;
    }

    case ICQ_CMDxSUB_AUTHxREQUEST:
      server->AddUserToList( ((CEventAuthRequest *)m_xCurrentReadEvent)->Uin());
      break;

  }
}


void ICQFunctions::slot_readbtn4()
{
  UserViewEvent* uidlg = new UserViewEvent(server, sigman, mainwin, m_nUin);
  uidlg->show();
}


void ICQFunctions::slot_sendbtn()
{
  if (icqEventTag != NULL) return;

  if (rdbUrl->isChecked())
  {
    if (!server->ViewUrl(edtItem->text().local8Bit()))
      WarnUser(this, tr("View URL failed"));
  }
  else if (rdbChat->isChecked())
  {
    if (m_nMPChatPort == 0)
    {
      if (ChatDlg::chatDlgs.size() > 0)
      {
        ChatDlg *chatDlg = NULL;
        CJoinChatDlg *j = new CJoinChatDlg(true, this);
        if (j->exec() && (chatDlg = j->JoinedChat()) != NULL)
        {
          edtItem->setText(j->ChatClients());
          m_nMPChatPort = chatDlg->LocalPort();
          m_szMPChatClients = chatDlg->ChatClients();
        }
        delete j;
        btnItem->setText(tr("Clear"));
      }
    }
    else
    {
      m_nMPChatPort = 0;
      m_szMPChatClients = "";
      edtItem->setText("");
      btnItem->setText(tr("Invite"));
    }
  }
  else if (rdbFile->isChecked())
  {
#ifdef USE_KDE
    QStringList fl = KFileDialog::getOpenFileNames(NULL, NULL, this);
#else
    QStringList fl = QFileDialog::getOpenFileNames(NULL, NULL, this);
#endif
    if (fl.isEmpty()) return;
    QStringList::ConstIterator it;
    QString f;
    for( it = fl.begin(); it != fl.end(); it++ )
    {
      if (it != fl.begin())
        f += ", ";
      f += (*it);
    }
    edtItem->setText(f);
  }
}


//-----ICQFunctions::generateReply-------------------------------------------
void ICQFunctions::generateReply()
{
  QString s;
  for (int i = 0; i < mleRead->numLines(); i++)
     s += QString("> ") + mleRead->textLine(i) + "\n";
  mleSend->setText(s);
  mleSend->GotoEnd();
  mleSend->setEdited(false);
  tabs->showPage(tabList[TAB_SEND].tab);
}

//---------------------------------------------------------------------------
void ICQFunctions::setSpoofed()
{
  if (chkSpoof && chkSpoof->isChecked())
  {
    if (!QueryUser(this, tr("Spoofing messages is immoral and possibly illegal\n"
                            "In clicking OK you absolve the author from any \n"
                            "responsibility for your actions.\n"
                            "Do you want to continue?"),
                   tr("&Ok"), tr("&Cancel")))
    {
      chkSpoof->setChecked(false);
    }
  }
}


//-----ICQFunctions::specialFcn----------------------------------------------
void ICQFunctions::specialFcn(int theFcn)
{
  switch (theFcn)
  {
  case 0:
    if (grpOpt != NULL) grpOpt->hide();
    tabs->updateGeometry();
    chkSendServer->setEnabled(true);
    chkMass->setEnabled(true);
    break;
  case 1:  // Url
    if (grpOpt == NULL) SendTab_grpOpt();
    lblItem->setText(tr("URL:"));
    btnItem->setText(tr("View"));
    edtItem->clear();
    edtItem->SetReadOnly(false);
    grpOpt->show();
    tabs->updateGeometry();
    chkSendServer->setEnabled(true);
    chkMass->setEnabled(true);
    break;
  case 2: // Chat
    if (grpOpt == NULL) SendTab_grpOpt();
    lblItem->setText(tr("Multiparty:"));
    btnItem->setText(tr("Invite"));
    edtItem->SetReadOnly(true);
    edtItem->clear();
    grpOpt->show();
    tabs->updateGeometry();
    chkSendServer->setChecked(false);
    chkSendServer->setEnabled(false);
    chkMass->setChecked(false);
    chkMass->setEnabled(false);
    break;
  case 3:  // File transfer
    if (grpOpt == NULL) SendTab_grpOpt();
    lblItem->setText(tr("Filename:"));
    btnItem->setText(tr("Choose"));
    edtItem->clear();
    edtItem->SetReadOnly(false);
    grpOpt->show();
    tabs->updateGeometry();
    chkSendServer->setChecked(false);
    chkSendServer->setEnabled(false);
    chkMass->setChecked(false);
    chkMass->setEnabled(false);
    break;
  }
}


//-----ICQFunctions::callFcn-------------------------------------------------
void ICQFunctions::callFcn()
{
  // do nothing if a command is already being processed
  if (icqEventTag != NULL) return;

  switch (currentTab)
  {
  case TAB_READ: close(); return;
  case TAB_SEND:
  {
    if(!mleSend->edited() &&
       !QueryUser(this, tr("You didn't edit the message.\n"
                           "Do you really want to send it?"), tr("&Yes"), tr("&No")))
      break;

    unsigned short nMsgLen = mleSend->text().length();
    if (nMsgLen > MAX_MESSAGE_SIZE && chkSendServer->isChecked()
        && !QueryUser(this, tr("Message is %1 characters, over the ICQ server limit of %2.\n"
                               "The message will be truncated if sent through the server.")
                      .arg(nMsgLen).arg(MAX_MESSAGE_SIZE),
                      tr("C&ontinue"), tr("&Cancel")))
        break;

    unsigned long uin = (chkSpoof && chkSpoof->isChecked() ?
                         edtSpoof->text().toULong() : 0);
    if (rdbMsg->isChecked())  // send a message
    {
      // don't let the user send empty messages
      if(mleSend->text().stripWhiteSpace().isEmpty())
        break;

      if (chkMass->isChecked())
      {
        CMMSendDlg *m = new CMMSendDlg(server, sigman, lstMultipleRecipients, this);
        int r = m->go_message(mleSend->text());
        delete m;
        if (r != QDialog::Accepted) break;
      }
      m_sProgressMsg = tr("Sending msg ");
      m_sProgressMsg += chkSendServer->isChecked() ? tr("through server") : tr("direct");
      m_sProgressMsg += "...";
      icqEventTag = server->icqSendMessage(m_nUin, mleSend->text().local8Bit(),
         chkSendServer->isChecked() ? false : true,
         chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL, uin);
    }
    else if (rdbUrl->isChecked()) // send URL
    {
      if (chkMass->isChecked())
      {
        CMMSendDlg *m = new CMMSendDlg(server, sigman, lstMultipleRecipients, this);
        int r = m->go_url(edtItem->text(), mleSend->text());
        delete m;
        if (r != QDialog::Accepted) break;
      }
      m_sProgressMsg = tr("Sending URL ");
      m_sProgressMsg += chkSendServer->isChecked() ? tr("through server") : tr("direct");
      m_sProgressMsg += "...";
      icqEventTag = server->icqSendUrl(m_nUin, edtItem->text().latin1(), mleSend->text().local8Bit(),
         chkSendServer->isChecked() ? false : true,
         chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL, uin);
    }
    else if (rdbChat->isChecked())   // send chat request
    {
      m_sProgressMsg = tr("Sending chat request...");
      if (m_nMPChatPort == 0)
        icqEventTag = server->icqChatRequest(m_nUin,
           mleSend->text().local8Bit(),
           chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL);
      else
        icqEventTag = server->icqMultiPartyChatRequest(m_nUin,
           mleSend->text().local8Bit(), m_szMPChatClients.local8Bit(),
           m_nMPChatPort,
           chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL);
    }
    else if (rdbFile->isChecked())   // send file transfer
    {
      if (edtItem->text().isEmpty())
      {
        WarnUser(this, tr("You must specify a file to transfer!"));
        break;
      }
      m_sProgressMsg = tr("Sending file transfer...");
      icqEventTag = server->icqFileTransfer(m_nUin, edtItem->text().local8Bit(),
         mleSend->text().local8Bit(),
         chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL);
    }

    if (icqEventTag == NULL)
      doneFcn(NULL);

    break;
  }
  }


  if (icqEventTag != NULL)
  {
    QString title = m_sBaseTitle + " [" + m_sProgressMsg + "]";
    setCaption(title);
    setCursor(waitCursor);
    btnOk->setEnabled(false);
    btnCancel->setText(tr("&Cancel"));
    connect (sigman, SIGNAL(signal_doneUserFcn(ICQEvent *)), this, SLOT(doneFcn(ICQEvent *)));
  }
}


//-----RetrySend-------------------------------------------------------------
void ICQFunctions::RetrySend(ICQEvent *e, bool bOnline, unsigned short nLevel)
{
  btnOk->setEnabled(false);
  btnCancel->setText(tr("&Cancel"));
  if (!bOnline) chkSendServer->setChecked(true);

  chkUrgent->setChecked(nLevel == ICQ_TCPxMSG_URGENT);

  switch(e->SubCommand())
  {
    case ICQ_CMDxSUB_MSG:
    {
      CEventMsg *ue = (CEventMsg *)e->UserEvent();
      m_sProgressMsg = tr("Sending msg ");
      m_sProgressMsg += bOnline ? tr("direct") : tr("through server");
      m_sProgressMsg += "...";
      icqEventTag = server->icqSendMessage(m_nUin, ue->Message(), bOnline,
         nLevel, 0);
      break;
    }
    case ICQ_CMDxSUB_URL:
    {
      CEventUrl *ue = (CEventUrl *)e->UserEvent();
      m_sProgressMsg = tr("Sending URL ");
      m_sProgressMsg += bOnline ? tr("direct") : tr("through server");
      m_sProgressMsg += "...";
      icqEventTag = server->icqSendUrl(m_nUin, ue->Url(), ue->Description(),
         bOnline, nLevel, 0);
      break;
    }
    case ICQ_CMDxSUB_CHAT:
    {
      CEventChat *ue = (CEventChat *)e->UserEvent();
      m_sProgressMsg = tr("Sending chat request...");
      icqEventTag = server->icqChatRequest(m_nUin, ue->Reason(), nLevel);
      break;
    }
    case ICQ_CMDxSUB_FILE:
    {
      CEventFile *ue = (CEventFile *)e->UserEvent();
      m_sProgressMsg = tr("Sending file transfer...");
      icqEventTag = server->icqFileTransfer(m_nUin, ue->Filename(),
         ue->FileDescription(), nLevel);
      break;
    }
  }

  if (icqEventTag != NULL)
  {
    connect (sigman, SIGNAL(signal_doneUserFcn(ICQEvent *)), this, SLOT(doneFcn(ICQEvent *)));
    QString title = m_sBaseTitle + " [" + m_sProgressMsg + "]";
    setCaption(title);
  }
}


//-----ICQFunctions::doneFcn-------------------------------------------------
void ICQFunctions::doneFcn(ICQEvent *e)
{
  if ( !icqEventTag->Equals(e) )
    return;

  bool isOk = (e != NULL && (e->Result() == EVENT_ACKED || e->Result() == EVENT_SUCCESS));
  bool bForceOpen = !chkAutoClose->isChecked();

  QString title, result;
  if (e == NULL)
  {
    result = tr("error");
  }
  else
  {
    switch (e->Result())
    {
    case EVENT_ACKED:
    case EVENT_SUCCESS:
      result = tr("done");
      QTimer::singleShot(5000, this, SLOT(slot_resettitle()));
      break;
    case EVENT_FAILED:
      result = tr("failed");
      break;
    case EVENT_TIMEDOUT:
      result = tr("timed out");
      break;
    case EVENT_ERROR:
      result = tr("error");
      break;
    default:
      break;
    }
  }
  title = m_sBaseTitle + " [" + m_sProgressMsg + result + "]";
  setCaption(title);

  setCursor(arrowCursor);
  btnOk->setEnabled(true);
  btnCancel->setText(tr("&Close"));
  delete icqEventTag;
  icqEventTag = NULL;
  disconnect (sigman, SIGNAL(signal_doneUserFcn(ICQEvent *)), this, SLOT(doneFcn(ICQEvent *)));

  if (e == NULL) return;

  if (!isOk)
  {
    if (e->Command() == ICQ_CMDxTCP_START &&
        (e->SubCommand() == ICQ_CMDxSUB_MSG ||
         e->SubCommand() == ICQ_CMDxSUB_URL) &&
        QueryUser(this, tr("Direct send failed,\nsend through server?"), tr("Yes"), tr("No")) )
    {
      RetrySend(e, false, ICQ_TCPxMSG_NORMAL);
    }
  }
  else
  {
    switch(e->Command())
    {
      case ICQ_CMDxTCP_START:
      {
        ICQUser *u = NULL;
        CUserEvent *ue = e->UserEvent();
        QString msg;
        if (e->SubResult() == ICQ_TCPxACK_RETURN)
        {
          u = gUserManager.FetchUser(m_nUin, LOCK_R);
          msg = tr("%1 is in %2 mode:\n%3\n")
                   .arg(u->GetAlias()).arg(u->StatusStr())
                   .arg(QString::fromLocal8Bit(u->AutoResponse()));
          gUserManager.DropUser(u);
          switch (QueryUser(this, msg, tr("Send\nUrgent"), tr("Send to\nContact List"), tr("Cancel")))
          {
            case 0:
              RetrySend(e, true, ICQ_TCPxMSG_URGENT);
              break;
            case 1:
              RetrySend(e, true, ICQ_TCPxMSG_LIST);
              break;
            case 2:
              break;
          }
          bForceOpen = true;
        }
        else if (e->SubResult() == ICQ_TCPxACK_REFUSE)
        {
          u = gUserManager.FetchUser(m_nUin, LOCK_R);
          msg = tr("%1 refused %2, send through server.")
                .arg(u->GetAlias()).arg(EventDescription(ue));
          InformUser(this, msg);
          gUserManager.DropUser(u);
          bForceOpen = true;
        }
        else if (e->SubCommand() == ICQ_CMDxSUB_CHAT || e->SubCommand() == ICQ_CMDxSUB_FILE)
        {
          CExtendedAck *ea = e->ExtendedAck();
          if (ea == NULL || ue == NULL)
          {
            gLog.Error("%sInternal error: ICQFunctions::doneFcn(): chat or file request acknowledgement without extended result.\n", L_ERRORxSTR);
            return;
          }
          if (!ea->Accepted())
          {
             u = gUserManager.FetchUser(m_nUin, LOCK_R);
             QString result = tr("%1 with %2 refused:\n%3")
                .arg(EventDescription(ue)).arg(u->GetAlias()).arg(ea->Response());
             gUserManager.DropUser(u);
             InformUser(this, result);
          }
          else
          {
            switch (e->SubCommand())
            {
            case ICQ_CMDxSUB_CHAT:
            {
              CEventChat *c = (CEventChat *)ue;
              if (c->Port() == 0)  // If we requested a join, no need to do anything
              {
                ChatDlg *chatDlg = new ChatDlg(m_nUin, server);
                chatDlg->StartAsClient(ea->Port());
              }
              break;
            }
            case ICQ_CMDxSUB_FILE:
            {
              CEventFile *f = (CEventFile *)ue;
              CFileDlg *fileDlg = new CFileDlg(m_nUin, server);
              fileDlg->SendFiles(f->Filename(), ea->Port());
              break;
            }
            default:
              break;
            } // case
          } // if accepted

          if (bForceOpen)
          {
            (void) new MsgViewItem(e->GrabUserEvent(), msgView);
            // FIXME we should actually stick this message in so it appears
            // before any unread messages...
            mleSend->clear();
            if (grpOpt != NULL) edtItem->clear();
          }
        } // if file or chat
        else
        {
          u = gUserManager.FetchUser(m_nUin, LOCK_R);
          if (u->Away() && u->ShowAwayMsg()) {
            gUserManager.DropUser(u);
            (void) new ShowAwayMsgDlg(NULL, NULL, m_nUin);
          }
          else
            gUserManager.DropUser(u);

          if (bForceOpen)
          {
            (void) new MsgViewItem(e->GrabUserEvent(), msgView);
            mleSend->clear();
            if (grpOpt != NULL) edtItem->clear();
          }
        }

        break;
      } // case
      case ICQ_CMDxSND_THRUxSERVER:
      {
        if (bForceOpen)
        {
          (void) new MsgViewItem(e->GrabUserEvent(), msgView);
          mleSend->clear();
          if (grpOpt != NULL) edtItem->clear();
        }
        break;
      }

      case ICQ_CMDxSND_USERxGETINFO:
      case ICQ_CMDxSND_USERxGETDETAILS:
      case ICQ_CMDxSND_UPDATExBASIC:
      case ICQ_CMDxSND_UPDATExDETAIL:
      case ICQ_CMDxSND_META:
      {
        bForceOpen = true;
        break;
      }

      default:
        break;

    }

    if (!bForceOpen) close();
  }

}


void ICQFunctions::slot_resettitle()
{
  setCaption(m_sBaseTitle);
}


void ICQFunctions::slot_close()
{
  m_bDeleteUser = btnCancel->stateWhenPressed() & ControlButton;
  close();
}


void ICQFunctions::closeEvent(QCloseEvent *e)
{
  if (icqEventTag != NULL)
  {
    setCaption(m_sBaseTitle);
    server->CancelEvent(icqEventTag);
    delete icqEventTag;
    icqEventTag = NULL;
    btnOk->setEnabled(true);
    btnCancel->setText(tr("&Close"));
    setCursor(arrowCursor);
  }
  else
  {
    s_nX = x();
    s_nY = y();
    emit signal_finished(m_nUin);
    if (m_bDeleteUser && !m_bOwner)
    {
      mainwin->RemoveUserFromList(m_nUin, this);
    }
    e->accept();
    delete this;
  }
}


ICQFunctions::~ICQFunctions()
{
  if (icqEventTag != NULL) server->CancelEvent(icqEventTag);
  delete icqEventTag;
}

#include "icqfunctions.moc"
