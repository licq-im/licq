#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef USE_KDE
#include <kfiledialog.h>
#else
#include <qfiledialog.h>
#endif
#include <qtabbar.h>
#include <qwidgetstack.h>
#include <qstylesheet.h>

#include "icqfunctions.h"
#include "chatacceptdlg.h"
#include "chatdlg.h"
#include "fileacceptdlg.h"
#include "filedlg.h"
#include "showawaymsgdlg.h"
#include "countrycodes.h"
#include "log.h"
#include "sigman.h"
#include "eventdesc.h"

#include "user.h"
#include "mledit.h"
#include "icqevent.h"
#include "icqd.h"

#define MARGIN_LEFT 5
#define MARGIN_RIGHT 30
#define RM 6
#define LM 6
#define TM 6
#define BM 42
#define NUM_MSG_PER_HISTORY 20
#define COLOR_SENT "blue"
#define COLOR_RECEIVED "red"

unsigned short ICQFunctions::s_nX = 100;
unsigned short ICQFunctions::s_nY = 100;

//-----ICQFunctions::constructor-------------------------------------------------------------------
ICQFunctions::ICQFunctions(CICQDaemon *s, CSignalManager *theSigMan,
                           unsigned long _nUin, bool _bIsOwner,
                           bool isAutoClose, QWidget *parent, const char *name)
   : QTabWidget(parent, name)
{
   server = s;
   sigman = theSigMan;
   icqEvent = NULL;
   m_nUin = _nUin;
   m_bIsOwner = _bIsOwner;

   tabLabel[TAB_READ] = tr("View Event");
   fcnTab[TAB_READ] = new QWidget(this, tabLabel[TAB_READ]);
   splRead = new QSplitter(QSplitter::Vertical, fcnTab[TAB_READ]);
   msgView = new MsgView(splRead);
   msgView->setFrameStyle( QFrame::WinPanel | QFrame::Sunken);
   mleRead = new MLEditWrap(true, splRead);
   mleRead->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
   mleRead->setReadOnly(true);
   QValueList<int> v;
   v.append(60);
   v.append(180);
   splRead->setSizes(v);
   splRead->setOpaqueResize(true);
   splRead->setResizeMode(msgView, QSplitter::KeepSize);
   splRead->setResizeMode(mleRead, QSplitter::Stretch);
   connect (msgView, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(printMessage(QListViewItem *)));

   tabLabel[TAB_SEND] = tr("Send Event");
   fcnTab[TAB_SEND] = new QWidget(this, tabLabel[TAB_SEND]);
   grpCmd = new QButtonGroup(tr("Select Function"), fcnTab[TAB_SEND]);
   grpCmd->setExclusive(true);
   rdbMsg = new QRadioButton(tr("Message"), grpCmd);
   rdbMsg->setChecked(true);
   rdbUrl = new QRadioButton(tr("URL"), grpCmd);
   rdbUrl->setChecked(true);
   rdbChat = new QRadioButton(tr("Chat Request"), grpCmd);
   rdbFile = new QRadioButton(tr("File Transfer"), grpCmd);
   connect(grpCmd, SIGNAL(clicked(int)), this, SLOT(specialFcn(int)));
   lblDescription = new QLabel(fcnTab[TAB_SEND]);
   mleSend = new MLEditWrap(true, fcnTab[TAB_SEND]);
   mleSend->setFrameStyle( QFrame::WinPanel | QFrame::Sunken);
   lblItem = new QLabel(fcnTab[TAB_SEND]);
   edtItem = new QLineEdit(fcnTab[TAB_SEND]);
   chkSendServer = new QCheckBox(tr("Send through server"), fcnTab[TAB_SEND]);
   chkUrgent = new QCheckBox(tr("Urgent"), fcnTab[TAB_SEND]);
   chkSpoof = new QCheckBox(tr("Spoof UIN:"), fcnTab[TAB_SEND]);
   edtSpoof = new QLineEdit(fcnTab[TAB_SEND]);
   edtSpoof->setEnabled(false);
   edtSpoof->setMaxLength(10);
   edtSpoof->setValidator(new QIntValidator(0, 2147483647, edtSpoof));
   connect(chkSpoof, SIGNAL(toggled(bool)), edtSpoof, SLOT(setEnabled(bool)));

   tabLabel[TAB_BASICINFO] = tr("User Info");
   fcnTab[TAB_BASICINFO] = new QWidget(this, tabLabel[TAB_BASICINFO]);
   nfoAlias = new CInfoField(5, 5, 45, 5, 110, tr("Alias:"), false, fcnTab[TAB_BASICINFO]);
   nfoStatus = new CInfoField(180, 5, 35, 5, width() - MARGIN_RIGHT - 190,
                              tr("Status:"), true, fcnTab[TAB_BASICINFO]);
   nfoUin = new CInfoField(5, 35, 45, 5, 110, tr("UIN:"), true, fcnTab[TAB_BASICINFO]);
   nfoFirstName = new CInfoField(5, 65, 45, 5, 110, tr("Name:"), false, fcnTab[TAB_BASICINFO]);
   nfoLastName = new CInfoField(170, 65, 0, 0, width() - MARGIN_RIGHT - 185,
                                NULL, false, fcnTab[TAB_BASICINFO]);
   nfoEMail = new CInfoField(5, 95, 45, 5, width() - MARGIN_RIGHT - 70,
                             tr("EMail:"), false, fcnTab[TAB_BASICINFO]);
   nfoHistory = new CInfoField(5, 125, 45, 5, width() - MARGIN_RIGHT - 70,
                               tr("History:"), false, fcnTab[TAB_BASICINFO]);
   nfoIp = new CInfoField(180, 35, 35, 5, width() - MARGIN_RIGHT - 230,
                          tr("IP:"), true, fcnTab[TAB_BASICINFO]);
   chkAuthorization = new QCheckBox(tr("Authorization Needed"), fcnTab[TAB_BASICINFO]);
   chkAuthorization->setEnabled(m_bIsOwner);

   tabLabel[TAB_DETAILINFO] = tr("Details");
   fcnTab[TAB_DETAILINFO] = new QWidget(this, tabLabel[TAB_DETAILINFO]);
   nfoAge = new CInfoField(5, 5, 45, 5, 100, tr("Age:"), !m_bIsOwner, fcnTab[TAB_DETAILINFO]);
   nfoState = new CInfoField(5, 35, 45, 5, 100, tr("State:"), !m_bIsOwner, fcnTab[TAB_DETAILINFO]);
   nfoCity = new CInfoField(5, 65, 45, 5, 100, tr("City:"), !m_bIsOwner, fcnTab[TAB_DETAILINFO]);
   lblCountry = new QLabel(tr("Country:"), fcnTab[TAB_DETAILINFO]);
   cmbCountry = new CEComboBox(true, fcnTab[TAB_DETAILINFO]);
   cmbCountry->insertItem(tr("Unspecified"));
   cmbCountry->insertItem(tr("Unknown (unspecified)"));
   m_nUnknownCountryCode = 0xFFFF; // set the unknown value to default to unspecified
   cmbCountry->insertStrList(GetCountryList());
   cmbCountry->setEnabled(m_bIsOwner);
   nfoHomepage = new CInfoField(5, 125, 65, 5, width() - MARGIN_RIGHT - 90,
                                tr("Homepage:"), !m_bIsOwner, fcnTab[TAB_DETAILINFO]);
   lblSex = new QLabel(tr("Sex:"), fcnTab[TAB_DETAILINFO]);
   cmbSex = new CEComboBox(true, fcnTab[TAB_DETAILINFO]);
   cmbSex->insertItem(tr("Unspecified"));
   cmbSex->insertItem(tr("Female"));
   cmbSex->insertItem(tr("Male"));
   cmbSex->setEnabled(m_bIsOwner);
   nfoPhone = new CInfoField(180, 35, 45, 5, width() - MARGIN_RIGHT - 245,
                             tr("Phone:"), !m_bIsOwner, fcnTab[TAB_DETAILINFO]);
   nfoZipcode = new CInfoField(180, 65, 45, 5, width() - MARGIN_RIGHT - 245,
                             tr("Zip:"), !m_bIsOwner, fcnTab[TAB_DETAILINFO]);
   boxAboutMsg = new QGroupBox (fcnTab[TAB_DETAILINFO]);
   boxAboutMsg->setFrameStyle(QFrame::Box | QFrame::Sunken);
   boxAboutMsg->setAlignment(AlignLeft);
   boxAboutMsg->setTitle(tr("About"));
   mleAboutMsg = new MLEditWrap(true, boxAboutMsg);

   tabLabel[TAB_HISTORY] = tr("History");
   fcnTab[TAB_HISTORY] = new QWidget(this, tabLabel[TAB_HISTORY]);
   mleHistory = new QTextView(fcnTab[TAB_HISTORY]);
   //mleHistory->styleSheet()->setAlignment(Qt::WordBreak);
   lblHistory = new QLabel(fcnTab[TAB_HISTORY]);

   addTab(fcnTab[TAB_READ], tabLabel[TAB_READ]);
   addTab(fcnTab[TAB_SEND], tabLabel[TAB_SEND]);
   addTab(fcnTab[TAB_BASICINFO], tabLabel[TAB_BASICINFO]);
   addTab(fcnTab[TAB_DETAILINFO], tabLabel[TAB_DETAILINFO]);
   addTab(fcnTab[TAB_HISTORY], tabLabel[TAB_HISTORY]);

   chkAutoClose = new QCheckBox(tr("Auto Close"), this);
   chkAutoClose->setChecked(isAutoClose);
   btnOk = new QPushButton(tr("Ok"), this);
   btnCancel = new QPushButton(tr("Close"), this);
   btnSave = new QPushButton(tr("Save"), this);

   setTabOrder(mleSend, btnOk);

   connect (mleSend, SIGNAL(signal_CtrlEnterPressed()), btnOk, SIGNAL(clicked()));
   connect (chkSpoof, SIGNAL(clicked()), this, SLOT(setSpoofed()));
   connect (this, SIGNAL(selected(const QString &)), this, SLOT(tabSelected(const QString &)));
   connect (sigman, SIGNAL(signal_doneUserFcn(ICQEvent *)), this, SLOT(doneFcn(ICQEvent *)));
   connect (sigman, SIGNAL(signal_updatedUser(unsigned long, unsigned long)),
            this, SLOT(slot_updatedUser(unsigned long, unsigned long)));
   connect (btnCancel, SIGNAL(clicked()), this, SLOT(close()));
   connect (btnOk, SIGNAL(clicked()), this, SLOT(callFcn()));
   connect (btnSave, SIGNAL(clicked()), this, SLOT(save()));

  QWidgetStack *stack = Q_CHILD(this, QWidgetStack, "tab pages");
  QTabBar *tabs = Q_CHILD(this, QTabBar, "tab control");
  if (stack == NULL || tabs == NULL)
  {
    gLog.Error("%sICQFunctions::resizeEvent(): Unable to find widget stack or tab bar.", L_ERRORxSTR);
    setGeometry(s_nX, s_nY, 400, 360);
    setMinimumSize(400, 360);
  }
  else
  {
    QSize t(tabs->sizeHint());
    setGeometry(s_nX, s_nY, t.width() + RM + LM, 360);
    setMinimumSize(t.width() + RM + LM, 360);
  }

#ifdef TEST_POS
  printf("constructor: %d %d\n", x(), y());
#endif
}


void ICQFunctions::resizeEvent(QResizeEvent *e)
{
  QTabWidget::resizeEvent(e);

  // Overload the stack and tab bar placement because Qt's sucks
  QWidgetStack *stack = Q_CHILD(this, QWidgetStack, "tab pages");
  QTabBar *tabs = Q_CHILD(this, QTabBar, "tab control");
  QSize t(tabs->sizeHint());
  int lw = stack->lineWidth();
  tabs->setGeometry(QMAX(0, lw - 2) + LM, TM, t.width(), t.height());
  stack->setGeometry(LM, t.height() - lw + TM, width() - (RM + LM), height() - t.height() + lw - (TM + BM));

  splRead->setGeometry(MARGIN_LEFT, 5, width() - MARGIN_RIGHT, height() - 90);

  grpCmd->setGeometry(MARGIN_LEFT, 5, width() - MARGIN_RIGHT, 60);
  rdbMsg->setGeometry(10, 15, 90, 20);
  rdbUrl->setGeometry(125, 15, 90, 20);
  rdbChat->setGeometry(10, 35, 110, 20);
  rdbFile->setGeometry(125, 35, 100, 20);
  lblDescription->setGeometry(5, 70, 120, 20);
  lblItem->setGeometry(MARGIN_LEFT, height() - 140, 80, 20);
  edtItem->setGeometry(70, height() - 140, width()-MARGIN_RIGHT-70+MARGIN_LEFT, 20);
  chkSendServer->setGeometry(5, height() - 115, 150, 20);
  chkUrgent->setGeometry(5, height() - 97, 80, 20);
  chkSpoof->setGeometry(170, height() - 115, 180, 20);
  edtSpoof->setGeometry(270, height() - 115, width() - MARGIN_RIGHT - 265, 20);
  mleSend->setGeometry(MARGIN_LEFT, 90, width() - MARGIN_RIGHT, height() - (lblItem->isVisible() ? 235 : 210));

  nfoAlias->setGeometry(5, 5, 45, 5, 110);
  nfoStatus->setGeometry(180, 5, 35, 5, width() - MARGIN_RIGHT - 215);
  nfoUin->setGeometry(5, 35, 45, 5, 110);
  nfoFirstName->setGeometry(5, 65, 45, 5, 110);
  nfoLastName->setGeometry(170, 65, 0, 0, width() - MARGIN_RIGHT - 165);
  nfoEMail->setGeometry(5, 95, 45, 5, width() - MARGIN_RIGHT - 50);
  nfoHistory->setGeometry(5, 125, 45, 5, width() - MARGIN_RIGHT - 50);
  nfoIp->setGeometry(180, 35, 35, 5, width() - MARGIN_RIGHT - 215);
  chkAuthorization->setGeometry(5, 150, 200, 20);

  nfoAge->setGeometry(5, 5, 45, 5, 100);
  nfoState->setGeometry(5, 35, 45, 5, 100);
  nfoCity->setGeometry(5, 65, 45, 5, 100);
  lblCountry->setGeometry(5, 95, 48, 20);
  cmbCountry->setGeometry(55, 95, width() - MARGIN_RIGHT - 50, 20);
  nfoHomepage->setGeometry(5, 125, 65, 5, width() - MARGIN_RIGHT - 70);
  lblSex->setGeometry(180, 5, 45, 20);
  cmbSex->setGeometry(230, 5, width() - MARGIN_RIGHT - 225, 20);
  nfoPhone->setGeometry(180, 35, 45, 5, width() - MARGIN_RIGHT - 225);
  nfoZipcode->setGeometry(180, 65, 45, 5, width() - MARGIN_RIGHT - 225);
  boxAboutMsg->setGeometry(5, 155, width() - MARGIN_RIGHT, height() - 235);
  mleAboutMsg->setGeometry(10, 15, boxAboutMsg->width() - 20, boxAboutMsg->height() - 25);

  mleHistory->setGeometry(MARGIN_LEFT, 5, width() - MARGIN_RIGHT, height() - 110);
  lblHistory->setGeometry(5, height() - 100, width() - 10, 20);

  chkAutoClose->setGeometry(10, height() - 30, 180, 20);
  btnCancel->setGeometry(width() - 86, height() - 34, 80, 26);
  btnOk->setGeometry(btnCancel->x() - 86, btnCancel->y(), btnCancel->width(), btnCancel->height());
  btnSave->setGeometry(btnOk->x() - 86, btnOk->y(), btnOk->width(), btnOk->height());
}


//-----ICQFunctions::keyPressEvent----------------------------------------------
void ICQFunctions::keyPressEvent(QKeyEvent *e)
{
  if (e->key() == Key_Escape)
  {
    close();
    return;
  }
  else if (currentPage() == fcnTab[TAB_READ])
  {
    showPage(fcnTab[TAB_SEND]);
    return;
  }
  /*else if (e->key() == Key_Enter || e->key() == Key_Return)
  {
    callFcn();
    return;
  }*/
  QTabWidget::keyPressEvent(e);
}

//-----ICQFunctions::setupTabs--------------------------------------------------
void ICQFunctions::setupTabs(int index)
{
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);

  // read tab
  for (short i = u->getNumMessages() - 1; i >= 0; i--)
     (void) new MsgViewItem(u->GetEvent(i), i, msgView);

  // print the first event if it's a message
  MsgViewItem *e = (MsgViewItem *)msgView->firstChild();
  if (e != NULL && (e->msg->SubCommand() == ICQ_CMDxSUB_MSG || e->msg->Command() == 0))
  {
     msgView->setSelected(e, true);
     gUserManager.DropUser(u);
     printMessage(e);
     u = gUserManager.FetchUser(m_nUin, LOCK_R);
  }

  // Send tab
  rdbFile->setEnabled(!u->getStatusOffline());
  rdbChat->setEnabled(!u->getStatusOffline());
  if (chkSendServer->isEnabled())
  {
    chkSendServer->setChecked(u->getSendServer() || (u->getStatusOffline() && u->SocketDesc() == -1));
  }
  if (u->Ip() == 0)
  {
    chkSendServer->setChecked(true);
    chkSendServer->setEnabled(false);
  }
  chkSpoof->setChecked(false);
  rdbMsg->setChecked(true);
  specialFcn(0);
  setTabEnabled(fcnTab[TAB_SEND], !m_bIsOwner);

  // Info tabs
  setBasicInfo(u);
  setExtInfo(u);

  // History tab
  //mleHistory->setReadOnly(true);

  bool bIsNew = u->getIsNew();
  gUserManager.DropUser(u);

  // mark the user as no longer new if they are new
  if (bIsNew)
  {
    u = gUserManager.FetchUser(m_nUin, LOCK_W);
    u->setIsNew(false);
    gUserManager.DropUser(u);
    emit signal_updatedUser(USER_BASIC, m_nUin);
  }
  show();
  // Post a resize event to force proper geometry setting, again because qt sucks
  resizeEvent(NULL);
#ifdef TEST_POS
  printf("just shown: %d %d\n", x(), y());
  move(100,100);
  printf("just shown 2: %d %d\n", x(), y());
#endif
  switch (index)
  {
  case 0: showPage(fcnTab[0]); break;
  case 1: showPage(fcnTab[1]); rdbMsg->setChecked(true); specialFcn(0); break;
  case 2: showPage(fcnTab[1]); rdbUrl->setChecked(true); specialFcn(1); break;
  case 3: showPage(fcnTab[1]); rdbChat->setChecked(true); specialFcn(2); break;
  case 4: showPage(fcnTab[1]); rdbFile->setChecked(true); specialFcn(3); break;
  case 8: showPage(fcnTab[2]); break;
  case 9: showPage(fcnTab[3]); break;
  case 10: showPage(fcnTab[4]); break;
  }

}


//-----ICQFunctions::setBasicInfo-----------------------------------------------
void ICQFunctions::setBasicInfo(ICQUser *u)
{
  struct UserBasicInfo us;
  bool bDropUser = false;

  if (u == NULL)
  {
    u = gUserManager.FetchUser(m_nUin, LOCK_R);
    bDropUser = true;
  }
  chkAuthorization->setChecked(u->getAuthorization());
  u->getBasicInfo(us);
  if (bDropUser) gUserManager.DropUser(u);

  nfoAlias->setData(us.alias);
  nfoStatus->setData(us.status);
  nfoFirstName->setData(us.firstname);
  nfoLastName->setData(us.lastname);
  nfoEMail->setData(us.email);
  nfoHistory->setData(us.history);
  nfoUin->setData(us.uin);
  nfoIp->setData(us.ip_port);
  m_sBaseTitle = QString::fromLocal8Bit(us.alias) + " (" +
                 QString::fromLocal8Bit(us.name) + ")";
  setCaption(m_sBaseTitle);
  setIconText(us.alias);
}


//-----ICQFunctions::setExtInfo-------------------------------------------------
void ICQFunctions::setExtInfo(ICQUser *u)
{
  struct UserExtInfo ud;
  bool bDropUser = false;

  if (u == NULL)
  {
    u = gUserManager.FetchUser(m_nUin, LOCK_R);
    bDropUser = true;
  }
  u->getExtInfo(ud);
  cmbSex->setCurrentItem(u->getSexNum());

  unsigned short i = GetIndexByCountryCode(u->getCountryCode());
  if (i == COUNTRY_UNSPECIFIED)
    cmbCountry->setCurrentItem(0);
  else if (i == COUNTRY_UNKNOWN)
  {
    m_nUnknownCountryCode = u->getCountryCode();
    cmbCountry->changeItem(tr("Unknown (%1)").arg(m_nUnknownCountryCode), 1);
    cmbCountry->setCurrentItem(1);
  }
  else  // known
    cmbCountry->setCurrentItem(i + 2);

  if (bDropUser) gUserManager.DropUser(u);

  nfoCity->setData(ud.city);
  nfoState->setData(ud.state);
  nfoPhone->setData(ud.phone);
  nfoZipcode->setData(ud.zipcode);
  nfoAge->setData(ud.age);
  nfoHomepage->setData(ud.homepage);
  mleAboutMsg->setText(QString::fromLocal8Bit(ud.about));
}


//-----ICQFunctions::SendUrl---------------------------------------------------
void ICQFunctions::SendUrl(const char *url, const char *desc)
{
  showPage(fcnTab[1]);
  rdbUrl->setChecked(true);
  specialFcn(1);
  edtItem->setText(url);
  mleSend->setText(desc);
}

//-----ICQFunctions::SendFile--------------------------------------------------
void ICQFunctions::SendFile(const char *file, const char *desc)
{
  showPage(fcnTab[1]);
  rdbFile->setChecked(true);
  specialFcn(3);
  edtItem->setText(file);
  mleSend->setText(desc);
}

//-----ICQFunctions::tabSelected-------------------------------------------------------------------
void ICQFunctions::tabSelected(const QString &tab)
{
  if (tab == tabLabel[TAB_SEND])
  {
     mleSend->setFocus();
     btnOk->setText(tr("&Send"));
     btnSave->hide();
     currentTab = TAB_SEND;
  }
  else if (tab == tabLabel[TAB_BASICINFO])
  {
     btnOk->setText(tr("Update"));
     btnSave->setText(tr("Save"));
     btnSave->show();
     currentTab = TAB_BASICINFO;
  }
  else if (tab == tabLabel[TAB_READ])
  {
     btnOk->setText(tr("Ok"));
     btnSave->setText(tr("Quote"));
     m_bIsOwner ? btnSave->hide() : btnSave->show();
     msgView->triggerUpdate();
     currentTab = TAB_READ;
  }
  else if (tab == tabLabel[TAB_DETAILINFO])
  {
     btnOk->setText(tr("Update"));
     btnSave->setText(tr("Save"));
     btnSave->show();
     currentTab = TAB_DETAILINFO;
  }
  else if (tab == tabLabel[TAB_HISTORY])
  {
     if (m_lHistoryList.size() == 0) SetupHistory();  // if no history, then get it
     btnOk->setText(tr("Next ->"));
     btnSave->setText(tr("<- Prev"));
     btnSave->show();
     currentTab = TAB_HISTORY;
  }
  else
  {
     btnOk->setText(tr("Ok"));
  }
  btnSave->setGeometry(width() - (btnOk->width() * 3 + 7 + 7 + 6), btnOk->y(), btnOk->width(), btnOk->height());
#ifdef TEST_POS
  printf("just shown new page: %d %d\n", x(), y());
#endif
}


//-----slot_updatedUser---------------------------------------------------------
void ICQFunctions::slot_updatedUser(unsigned long _nUpdateType, unsigned long _nUin)
{
  if (m_nUin != _nUin) return;
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
  switch (_nUpdateType)
  {
  case USER_STATUS:
  {
    char szStatus[32];
    u->getStatusStr(szStatus);
    nfoStatus->setData(szStatus);
    if (u->Ip() == 0)
    {
      chkSendServer->setChecked(true);
      chkSendServer->setEnabled(false);
    }
    else
    {
      chkSendServer->setEnabled(true);
    }
    break;
  }
  case USER_EVENTS:
  {
    MsgViewItem *e = (MsgViewItem *)msgView->firstChild();
    short index = -1;
    if (e != NULL)
    {
      do
      {
        if (e->index > index) index = e->index;
        e = (MsgViewItem *)e->nextSibling();
      } while (e != NULL);
    }
    index++;
    (void) new MsgViewItem(u->GetEvent(index), index, msgView);
    break;
  }
  case USER_BASIC:
    setBasicInfo(u);
    break;
  case USER_EXT:
    setExtInfo(u);
    break;
  }
  gUserManager.DropUser(u);
}


//-----ICQFunctions::printMessage-----------------------------------------------
void ICQFunctions::printMessage(QListViewItem *e)
{
  CUserEvent *m = ((MsgViewItem *)e)->msg;
  mleRead->setText(QString::fromLocal8Bit(m->Text()));
  if (m->Command() == ICQ_CMDxTCP_START || m->Command() == ICQ_CMDxRCV_SYSxMSGxONLINE)
  {
    switch (m->SubCommand())
    {
    case ICQ_CMDxSUB_CHAT:  // accept or refuse a chat request
      (void) new CChatAcceptDlg(server, m_nUin, m->Sequence());
      break;
    case ICQ_CMDxSUB_FILE:  // accept or refuse a file transfer
      (void) new CFileAcceptDlg(server, m_nUin, (CEventFile *)m);
      break;
    case ICQ_CMDxSUB_URL:   // view a url
      if (server->getUrlViewer() != NULL && QueryUser(this, tr("View URL?"), tr("Yes"), tr("No")) )
      {
        char* szCmd = new char[strlen(server->getUrlViewer()) + strlen(((CEventUrl *)m)->Url()) + 8];
        sprintf(szCmd, "%s %s &", server->getUrlViewer(), ((CEventUrl *)m)->Url());
        if (system(szCmd) != 0) gLog.Error("%sView URL failed.\n", L_ERRORxSTR);
        delete szCmd;
      }
      break;
    case ICQ_CMDxSUB_REQxAUTH:
    {
      if (QueryUser(this, tr("Authorize?"), tr("Yes"), tr("No")))
        server->icqAuthorize( ((CEventAuthReq *)m)->Uin() );
      //...only ask if they aren't already there...
      ICQUser *u = gUserManager.FetchUser(((CEventAuthReq *)m)->Uin(), LOCK_R);
      if (u != NULL)
        gUserManager.DropUser(u);
      else
      {
        if (QueryUser(this, tr("Add?"), tr("Yes"), tr("No")))
          server->AddUserToList( ((CEventAuthReq *)m)->Uin());
      }
      break;
    }
    case ICQ_CMDxSUB_AUTHORIZED:
    {
      //...only ask if they aren't already there...
      ICQUser *u = gUserManager.FetchUser(((CEventAuth *)m)->Uin(), LOCK_R);
      if (u != NULL)
        gUserManager.DropUser(u);
      else
      {
        if (QueryUser(this, tr("Add?"), tr("Yes"), tr("No")))
          server->AddUserToList( ((CEventAuth *)m)->Uin());
      }
      break;
    }
    } // switch
  }  // if

  short index = ((MsgViewItem *)e)->index;
  if (index >= 0)   // the message had not been seen yet
  {
    ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_W);
    u->ClearEvent(index);
    gUserManager.DropUser(u);
    msgView->markRead(index);
    emit signal_updatedUser(USER_EVENTS, m_nUin);
  }
}


//-----ICQFunctions::save-------------------------------------------------------
void ICQFunctions::save()
{
  switch (currentTab)
  {
  case TAB_READ:
    generateReply();
    break;
  case TAB_BASICINFO:
    saveBasicInfo();
    break;
  case TAB_DETAILINFO:
    saveExtInfo();
    break;
  case TAB_HISTORY:
    ShowHistoryPrev();
    break;
  default:
    gLog.Warn("%sInternal error: ICQFunctions::save(): invalid tab - %d.\n",
              L_WARNxSTR, currentTab);
    break;
  }
}


//-----ICQFunctions::saveBasicInfo----------------------------------------------
void ICQFunctions::saveBasicInfo()
{
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_W);
  u->setEnableSave(false);
  u->setAlias(nfoAlias->text().local8Bit());
  u->setFirstName(nfoFirstName->text().local8Bit());
  u->setLastName(nfoLastName->text().local8Bit());
  u->setEmail(nfoEMail->text());
  u->setHistoryFile(nfoHistory->text());
  u->setAuthorization(chkAuthorization->isChecked());
  u->setEnableSave(true);
  u->saveBasicInfo();
  gUserManager.DropUser(u);
}


//-----ICQFunctions::saveExtInfo---------------------------------------------
void ICQFunctions::saveExtInfo()
{
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_W);
  u->setEnableSave(false);
  u->setCity(nfoCity->text().local8Bit());
  u->setState(nfoState->text().local8Bit());
  unsigned short i = cmbCountry->currentItem();
  if (i == 0)
    u->setCountry(COUNTRY_UNSPECIFIED);
  else if (i == 1)
    u->setCountry(m_nUnknownCountryCode);
  else
    u->setCountry(GetCountryCodeByIndex(i - 2));
  u->setAge(atol(nfoAge->text()));
  u->setSex(cmbSex->currentItem());
  u->setPhoneNumber(nfoPhone->text());
  u->setZipcode(nfoZipcode->text().toULong());
  u->setHomepage(nfoHomepage->text());
  u->setAbout(mleAboutMsg->text().local8Bit());
  u->setEnableSave(true);
  u->saveExtInfo();
  gUserManager.DropUser(u);
}


//-----ICQFunctions::SetupHistory--------------------------------------------
void ICQFunctions::SetupHistory(void)
{
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
  if (!u->GetHistory(m_lHistoryList))
  {
    mleHistory->setText(tr("Error loading history"));
  }
  else
  {
    m_iHistoryIter = m_lHistoryList.end();
    m_nHistoryIndex = m_lHistoryList.size();
    m_nHistoryShowing = 0;
    ShowHistoryPrev();
  }
  gUserManager.DropUser(u);
}


void ICQFunctions::ShowHistoryPrev(void)
{
  // Iterate back over what's currently showing
  while (m_nHistoryShowing > 0)
  {
    m_iHistoryIter--;
    m_nHistoryIndex--;
    m_nHistoryShowing--;
  }
  // Iterate far enough to show the previous messages
  for (unsigned short i = 0;
       i < NUM_MSG_PER_HISTORY && m_iHistoryIter != m_lHistoryList.begin();
       i++)
  {
    m_iHistoryIter--;
    m_nHistoryIndex--;
  }
  ShowHistory();
}

void ICQFunctions::ShowHistoryNext(void)
{
  if (m_iHistoryIter != m_lHistoryList.end())
  {
    ShowHistory();
  }
}

void ItalisizeLine(QString &t, QString pre, unsigned short inspos)
{
  int i = 0;
  while ( (i = t.find(pre, i)) != -1)
  {
    t.insert(i + inspos, "<i>");
    i = t.find("<br>", i + inspos);
    if (i == -1)
    {
      t.append("</i>");
      break;
    }
    else
      t.insert(i, "</i>");
  }
}


//-----ICQFunctions::ShowHistory--------------------------------------------
void ICQFunctions::ShowHistory(void)
{
  // Last check (should never be true)
  if (m_lHistoryList.size() == 0) return;

  QString s, st;
  QDateTime d;
  m_nHistoryShowing = 0;
  while (m_nHistoryShowing < NUM_MSG_PER_HISTORY)
  {
    d.setTime_t((*m_iHistoryIter)->Time());
    s.sprintf("<font color=\"%s\"><b>%s (%s) [%c%c%c]</b><br><br>",
              (*m_iHistoryIter)->Direction() == D_RECEIVER ? COLOR_RECEIVED : COLOR_SENT,
              (const char *)EventDescription(*m_iHistoryIter),
              (const char *)d.toString(),
              (*m_iHistoryIter)->IsDirect() ? 'D' : '-',
              (*m_iHistoryIter)->IsMultiRec() ? 'M' : '-',
              (*m_iHistoryIter)->IsUrgent() ? 'U' : '-');
    QString t = QStyleSheet::convertFromPlainText(QString::fromLocal8Bit( (*m_iHistoryIter)->Text() ));
    //printf("%s\n", (const char *)t);
    ItalisizeLine(t, "<p>&gt;", 3);
    ItalisizeLine(t, "<br>&gt;", 4);
    //s.append(QStyleSheet::convertFromPlainText(QString::fromLocal8Bit( (*m_iHistoryIter)->Text() )));
    s.append(t);
    s.append("</font><br><hr><br>");
    st.append(s);
    m_iHistoryIter++;
    m_nHistoryIndex++;
    m_nHistoryShowing++;
    if (m_iHistoryIter == m_lHistoryList.end())
    {
      break;
    }
  }
  lblHistory->setText(tr("[<font color=\"" COLOR_RECEIVED "\">Received</font>] "
                         "[<font color=\"" COLOR_SENT "\">Sent</font>] "
                         "%3 to %4 of %5")
                      .arg(m_nHistoryIndex - m_nHistoryShowing + 1)
                      .arg(m_nHistoryIndex)
                      .arg(m_lHistoryList.size()));
  mleHistory->setText(st);

}


//-----ICQFunctions::saveHistory-------------------------------------------------------------------
void ICQFunctions::saveHistory()
{
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
  u->SaveHistory(mleHistory->text().local8Bit());
  gUserManager.DropUser(u);
}


//-----ICQFunctions::generateReply-----------------------------------------------------------------
void ICQFunctions::generateReply()
{
  mleSend->clear();
  for (int i = 0; i < mleRead->numLines(); i++)
    mleSend->insertLine( QString("> ") + mleRead->textLine(i));
  mleSend->append("\n");
  mleSend->goToEnd();
  showPage(fcnTab[1]);
}



//-----ICQFunctions::setSpoofed--------------------------------------------------------------------
void ICQFunctions::setSpoofed()
{
  if (chkSpoof->isChecked())
  {
    if (!QueryUser(this, tr("Spoofing messages is immoral and possibly illegal.\nIn clicking OK you absolve the author from any \nresponsibility for your actions.\nDo you want to continue?"),
                   tr("Ok"), tr("Cancel")))
    {
      chkSpoof->setChecked(false);
    }
  }
}


//-----ICQFunctions::specialFcn--------------------------------------------------------------------
void ICQFunctions::specialFcn(int theFcn)
{
   switch (theFcn)
   {
   case 0:
      lblItem->hide();
      edtItem->hide();
      lblDescription->setText(tr("Message:"));
      mleSend->setEnabled(true);
      chkSendServer->setEnabled(true);
      mleSend->setGeometry(MARGIN_LEFT, 90, width() - MARGIN_RIGHT, height() - 210);
      break;
   case 1:
      lblItem->setText(tr("URL:"));
      lblItem->show();
      edtItem->show();
      lblDescription->setText(tr("Description:"));
      mleSend->setEnabled(true);
      chkSendServer->setEnabled(true);
      mleSend->setGeometry(MARGIN_LEFT, 90, width() - MARGIN_RIGHT, height() - 235);
      break;
   case 2:
      lblItem->hide();
      edtItem->hide();
      lblDescription->setText(tr("Reason:"));
      mleSend->setEnabled(true);
      chkSendServer->setChecked(false);
      chkSendServer->setEnabled(false);
      mleSend->setGeometry(MARGIN_LEFT, 90, width() - MARGIN_RIGHT, height() - 210);
      break;
   case 3:
      lblItem->setText(tr("Filename:"));
      lblItem->show();
      edtItem->show();
      lblDescription->setText(tr("Description:"));
      chkSendServer->setChecked(false);
      chkSendServer->setEnabled(false);
      mleSend->setEnabled(true);
      mleSend->setGeometry(MARGIN_LEFT, 90, width() - MARGIN_RIGHT, height() - 235);
#ifdef USE_KDE
      QStringList fl = KFileDialog::getOpenFileNames(NULL, NULL, this);
#else
      QStringList fl = QFileDialog::getOpenFileNames(NULL, NULL, this);
#endif
      QStringList::Iterator it;
      QString f;
      for( it = fl.begin(); it != fl.end(); it++ )
      {
        if (it != fl.begin())
          f += ", ";
        f += (*it);
      }
      edtItem->setText(f);
      break;
   }
}


//-----ICQFunctions::callFcn-----------------------------------------------------------------------
void ICQFunctions::callFcn()
{
  // disable user input
  btnOk->setEnabled(false);
  btnCancel->setText(tr("Cancel"));

  // do nothing if a command is already being processed
  if (icqEvent != NULL) return;

  switch (currentTab)
  {
  case TAB_READ: close(); return;
  case TAB_SEND:
  {
    unsigned short nMsgLen = mleSend->text().length();
    if (nMsgLen > MAX_MESSAGE_SIZE)
     {
        if(!QueryUser(this, tr("Message is %1 characters, over the ICQ98 limit of %2.  \nLicq, ICQ99, and other clones support longer messages \nhowever ICQ98 will not. Continue?").arg(nMsgLen).arg(MAX_MESSAGE_SIZE),
                      tr("Ok"), tr("Cancel")))
        {
          btnOk->setEnabled(true);
          break;
        }
     }
     unsigned long uin = (chkSpoof->isChecked() ? atoi(edtSpoof->text()) : 0);

     if (rdbMsg->isChecked())  // send a message
     {
        ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_W);
        u->setSendServer(chkSendServer->isChecked());
        gUserManager.DropUser(u);
        m_sProgressMsg = tr("Sending msg ");
        m_sProgressMsg += chkSendServer->isChecked() ? tr("through server") : tr("direct");
        m_sProgressMsg += "...";
        icqEvent = server->icqSendMessage(m_nUin, mleSend->text().local8Bit(),
                                          chkSendServer->isChecked() ? false : true,
                                          chkUrgent->isChecked() ? true : false, uin);
     }
     else if (rdbUrl->isChecked()) // send URL
     {
        m_sProgressMsg = tr("Sending URL ");
        m_sProgressMsg += chkSendServer->isChecked() ? tr("through server") : tr("direct");
        m_sProgressMsg += "...";
        icqEvent = server->icqSendUrl(m_nUin, edtItem->text(), mleSend->text().local8Bit(),
                                      chkSendServer->isChecked() ? false : true,
                                      chkUrgent->isChecked() ? true : false, uin);
     }
     else if (rdbChat->isChecked())   // send chat request
     {
        m_sProgressMsg = tr("Sending chat request ");
        m_sProgressMsg += chkSendServer->isChecked() ? tr("through server") : tr("direct");
        m_sProgressMsg += "...";
        icqEvent = server->icqChatRequest(m_nUin, mleSend->text().local8Bit(),
                                          chkSendServer->isChecked() ? false : true,
                                          chkUrgent->isChecked() ? true : false, uin);
     }
     else if (rdbFile->isChecked())   // send file transfer
     {
        m_sProgressMsg = tr("Sending file transfer ");
        m_sProgressMsg += chkSendServer->isChecked() ? tr("through server") : tr("direct");
        m_sProgressMsg += "...";
        icqEvent = server->icqFileTransfer(m_nUin, edtItem->text(), mleSend->text().local8Bit(),
                                           chkSendServer->isChecked() ? false : true,
                                           chkUrgent->isChecked() ? true : false, uin);
     }

     if (icqEvent == NULL)
        doneFcn(NULL);

     break;
  }

  case TAB_BASICINFO:
     if ( m_bIsOwner && (!QueryUser(this, tr("Update local or server information?"), tr("Local"), tr("Server"))) )
     {
        m_sProgressMsg = tr("Updating server...");
        icqEvent = server->icqUpdateBasicInfo(nfoAlias->text().local8Bit(),
                                           nfoFirstName->text().local8Bit(),
                                           nfoLastName->text().local8Bit(),
                                           nfoEMail->text(),
                                           chkAuthorization->isChecked());
     }
     else
     {
        m_sProgressMsg = tr("Updating...");
        icqEvent = server->icqUserBasicInfo(m_nUin);
     }
     break;
  case TAB_DETAILINFO:
     if ( m_bIsOwner && (!QueryUser(this, tr("Update local or server information?"), tr("Local"), tr("Server"))) )
     {
        m_sProgressMsg = tr("Updating server...");
        unsigned short i = cmbCountry->currentItem();
        unsigned short cc = ( i == 0 ? COUNTRY_UNSPECIFIED : (i == 1 ? m_nUnknownCountryCode : GetCountryCodeByIndex(i - 2)) );
        icqEvent = server->icqUpdateExtendedInfo(nfoCity->text().local8Bit(), cc,
                                         nfoState->text().local8Bit(), atol(nfoAge->text()),
                                         cmbSex->currentItem(), nfoPhone->text(),
                                         nfoHomepage->text(), mleAboutMsg->text().local8Bit(),
                                         nfoZipcode->text().toULong());
     }
     else
     {
        m_sProgressMsg = tr("Updating...");
        icqEvent = server->icqUserExtendedInfo(m_nUin);
     }
     break;
  case TAB_HISTORY:
    ShowHistoryNext();
    btnOk->setEnabled(true);
    btnCancel->setText(tr("Close"));
    break;
  }

  QString title = m_sBaseTitle + " [" + m_sProgressMsg + "]";
  setCaption(title);
}


//-----ICQFunctions::doneFcn----------------------------------------------------
void ICQFunctions::doneFcn(ICQEvent *e)
{
  if (e != icqEvent) return;
  bool isOk = (e->m_eResult == EVENT_ACKED || e->m_eResult == EVENT_SUCCESS);
  bool bForceOpen = false;

  QString title, result;
  switch (e->m_eResult)
  {
  case EVENT_ACKED:
  case EVENT_SUCCESS:
    result = tr("done");
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
  title = m_sBaseTitle + " [" + m_sProgressMsg + result + "]";
  setCaption(title);
  btnOk->setEnabled(true);
  btnCancel->setText(tr("Close"));
  icqEvent = NULL;

  if (!isOk)
  {
    if (e->m_nCommand == ICQ_CMDxTCP_START &&
        (e->m_nSubCommand == ICQ_CMDxSUB_MSG ||
         e->m_nSubCommand == ICQ_CMDxSUB_URL) &&
        QueryUser(this, tr("Direct send failed,\nsend through server?"), tr("Yes"), tr("No")) )
    {
      btnOk->setEnabled(false);
      btnCancel->setText(tr("Cancel"));
      chkSendServer->setChecked(true);
      ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_W);
      u->setSendServer(true);
      gUserManager.DropUser(u);
      if (e->m_nSubCommand == ICQ_CMDxSUB_MSG)
      {
        CEventMsg *ue = (CEventMsg *)e->m_xUserEvent;
        m_sProgressMsg = tr("Sending msg through server...");
        icqEvent = server->icqSendMessage(m_nUin, ue->Message(), false,
                                          false, 0);
      }
      else
      {
        CEventUrl *ue = (CEventUrl *)e->m_xUserEvent;
        m_sProgressMsg = tr("Sending URL through server...");
        icqEvent = server->icqSendUrl(m_nUin, ue->Url(), ue->Description(), false,
                                      false, 0);
      }
      QString title = m_sBaseTitle + " [" + m_sProgressMsg + "]";
      setCaption(title);
    }
  }
  else
  {
    switch(e->m_nCommand)
    {
    case ICQ_CMDxTCP_START:
    {
      ICQUser *u = NULL;
      CUserEvent *ue = e->m_xUserEvent;
      QString msg;
      if (e->m_nSubResult == ICQ_TCPxACK_RETURN)
      {
        char status[32];
        u = gUserManager.FetchUser(m_nUin, LOCK_R);
        u->getStatusStr(status);
        msg = tr("%1 is in %2 mode:\n%3\n[Send \"urgent\" to ignore]")
                 .arg(u->getAlias()).arg(status).arg(u->AutoResponse());
        InformUser(this, msg);
        gUserManager.DropUser(u);
        bForceOpen = true;
      }
      else if (e->m_nSubResult == ICQ_TCPxACK_REFUSE)
      {
        u = gUserManager.FetchUser(m_nUin, LOCK_R);
        msg = tr("%1 refused %2, send through server.")
              .arg(u->getAlias()).arg(EventDescription(ue));
        InformUser(this, msg);
        gUserManager.DropUser(u);
        bForceOpen = true;
      }
      else if (e->m_nSubCommand == ICQ_CMDxSUB_CHAT || e->m_nSubCommand == ICQ_CMDxSUB_FILE)
      {
        struct SExtendedAck *ea = e->m_sExtendedAck;
        if (ea == NULL || ue == NULL)
        {
          gLog.Error("%sInternal error: ICQFunctions::doneFcn(): chat or file request acknowledgement without extended result.\n", L_ERRORxSTR);
          return;
        }
        if (!ea->bAccepted)
        {
           u = gUserManager.FetchUser(m_nUin, LOCK_R);
           QString result;
           result.sprintf(tr("%s%1 with %2 refused:\n%s%3"), L_TCPxSTR, L_BLANKxSTR);
           result.arg(EventDescription(ue)).arg(u->getAlias()).arg(ea->szResponse);
           gUserManager.DropUser(u);
           InformUser(this, result);
        }
        else
        {
          switch (e->m_nSubCommand)
          {
          case ICQ_CMDxSUB_CHAT:
          {
            ChatDlg *chatDlg = new ChatDlg(m_nUin, false, ea->nPort);
            chatDlg->show();
            break;
          }
          case ICQ_CMDxSUB_FILE:
          {
            CFileDlg *fileDlg = new CFileDlg(m_nUin,
                                             ((CEventFile *)ue)->Filename(),
                                             ((CEventFile *)ue)->FileSize(),
                                             false, ea->nPort);
            fileDlg->show();
            break;
          }
          default:
            break;
          } // case
        } // if accepted
      } // if file or chat
      else
      {
        u = gUserManager.FetchUser(m_nUin, LOCK_R);
        if (u->isAway() && u->ShowAwayMsg()) {
          gUserManager.DropUser(u);
          (void) new ShowAwayMsgDlg(NULL, NULL, m_nUin);
        }
        else
          gUserManager.DropUser(u);

        mleSend->clear();
      }

      break;
    } // case
    case ICQ_CMDxSND_THRUxSERVER:
    {
      mleSend->clear();
      break;
    }

    case ICQ_CMDxSND_USERxGETINFO:
    case ICQ_CMDxSND_USERxGETDETAILS:
    case ICQ_CMDxSND_UPDATExBASIC:
    case ICQ_CMDxSND_UPDATExDETAIL:
    {
      bForceOpen = true;
      break;
    }

    default:
      break;

    }

    if (chkAutoClose->isChecked() && !bForceOpen) close();
  }

}

void ICQFunctions::closeEvent(QCloseEvent *e)
{
#ifdef TEST_POS
  printf("close event: %d %d\n", x(), y());
#endif
  if (icqEvent != NULL)
  {
    setCaption(m_sBaseTitle);
    server->CancelEvent(icqEvent);
    icqEvent = NULL;
    btnOk->setEnabled(true);
    btnCancel->setText(tr("Close"));
  }
  else
  {
    s_nX = x();
    s_nY = y();
    e->accept();
    ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_W);
    if (u != NULL)
    {
      u->fcnDlg = NULL;
      gUserManager.DropUser(u);
    }
    delete this;
  }
}


ICQFunctions::~ICQFunctions(void)
{
}

#include "moc/moc_icqfunctions.h"
