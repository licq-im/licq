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

extern "C" {

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifdef USE_SCRNSAVER
#include <X11/extensions/scrnsaver.h>
#endif

}

#undef Bool
#undef None
#undef KeyPress
#undef KeyRelease
#undef FocusIn
#undef FocusOut
#undef Status

#ifdef USE_KDE
#include <kapp.h>
#include <kglobal.h>
#include <kwin.h>
#else
#include <qapplication.h>
#endif

#include <qimage.h>
#include <qwindowsstyle.h>
#include <qdatetime.h>
#include <qclipboard.h>

#include "mainwin.h"
#include "licq_icq.h"
#include "gui-defines.h"
#include "licq_log.h"
#include "licq_translate.h"
#include "licq_utility.h"
#include "adduserdlg.h"
#include "authuserdlg.h"
#include "editgrp.h"
#include "searchuserdlg.h"
#include "icqfunctions.h"
#include "utilitydlg.h"
#include "registeruser.h"
#include "skinbrowser.h"
#include "licq_icqd.h"
#include "awaymsgdlg.h"
#include "outputwin.h"
#include "ewidgets.h"
#include "sigman.h"
#include "showawaymsgdlg.h"
#include "optionsdlg.h"
#include "skin.h"
#include "securitydlg.h"
#include "passworddlg.h"
#include "plugindlg.h"
#include "randomchatdlg.h"
#include "userinfodlg.h"
#include "usereventdlg.h"
#ifdef USE_DOCK
#include "wharf.h"
#endif

#include "licq_qt-gui.conf.h"

static QPixmap *ScaleWithBorder(const QPixmap &pm, int w, int h, struct Border border)
{
   QPainter p;

   // top left corner
   QPixmap pmTL(border.left, border.top);
   p.begin(&pmTL);
   p.drawPixmap(0, 0, pm, 0, 0, pmTL.width(), pmTL.height());
   p.end();

   // top border
   QPixmap pmT(pm.width() - border.left - border.right, border.top);
   p.begin(&pmT);
   p.drawPixmap(0, 0, pm, border.left, 0, pmT.width(), pmT.height());
   p.end();
   QImage imT( (pmT.convertToImage()).smoothScale(w - border.left - border.right, pmT.height()) );

   // top right corner
   QPixmap pmTR(border.right, border.top);
   p.begin(&pmTR);
   p.drawPixmap(0, 0, pm, pm.width() - border.right, 0, pmTR.width(), pmTR.height());
   p.end();

   // left border
   QPixmap pmL(border.left, pm.height() - border.top - border.bottom);
   p.begin(&pmL);
   p.drawPixmap(0, 0, pm, 0, border.top, pmL.width(), pmL.height());
   p.end();
   QImage imL( (pmL.convertToImage()).smoothScale(pmL.width(), h - border.top - border.bottom) );

   // center
   QPixmap pmC(pmT.width(), pmL.height());
   p.begin(&pmC);
   p.drawPixmap(0, 0, pm, border.left, border.top, pmC.width(), pmC.height());
   p.end();
   QImage imC( (pmC.convertToImage()).smoothScale(imT.width(), imL.height()) );

   // right border
   QPixmap pmR(border.right, pm.height() - border.top - border.bottom);
   p.begin(&pmR);
   p.drawPixmap(0, 0, pm, pm.width() - border.right, border.top, pmR.width(), pmR.height());
   p.end();
   QImage imR ( (pmR.convertToImage()).smoothScale(pmR.width(), h - border.top - border.bottom) );

   // bottom left border
   QPixmap pmBL(border.left, border.bottom);
   p.begin(&pmBL);
   p.drawPixmap(0, 0, pm, 0, pm.height() - border.bottom, pmBL.width(), pmBL.height());
   p.end();

   // bottom border
   QPixmap pmB(pm.width() - border.left - border.right, border.bottom);
   p.begin(&pmB);
   p.drawPixmap(0, 0, pm, border.left, pm.height() - border.bottom, pmB.width(), pmB.height());
   p.end();
   QImage imB( (pmB.convertToImage()).smoothScale(w - border.left - border.right, pmB.height()) );

   // bottom right border
   QPixmap pmBR(border.right, border.bottom);
   p.begin(&pmBR);
   p.drawPixmap(0, 0, pm, pm.width() - border.right, pm.height() - border.bottom, pmBR.width(), pmBR.height());
   p.end();

   // put the image together
   QPixmap *pmFinal = new QPixmap(w, h, pm.depth());
   p.begin(pmFinal);
   p.drawPixmap(0, 0, pmTL, 0, 0, -1, -1);
   p.drawImage(border.left, 0, imT, 0, 0, -1, -1);
   p.drawPixmap(pmFinal->width() - border.right, 0, pmTR, 0, 0, -1, -1);
   p.drawImage(0, border.top, imL, 0, 0, -1, -1);
   p.drawImage(pmFinal->width() - border.right, border.top, imR, 0, 0, -1, -1);
   p.drawPixmap(0, pmFinal->height() - border.bottom, pmBL, 0, 0, -1, -1);
   p.drawImage(border.left, pmFinal->height() - border.bottom, imB, 0, 0, -1, -1);
   p.drawPixmap(pmFinal->width() - border.right, pmFinal->height() - border.bottom, pmBR, 0, 0, -1, -1);
   p.drawImage(border.left, border.top, imC, 0, 0, -1, -1);
   p.end();

   return (pmFinal);
}


QPixmap& CMainWindow::iconForStatus(unsigned long Status)
{
  if((unsigned short) Status != ICQ_STATUS_OFFLINE && (Status & ICQ_STATUS_FxPRIVATE))
    return gMainWindow->pmPrivate;

  if ((unsigned short) Status == ICQ_STATUS_OFFLINE) return gMainWindow->pmOffline;
  if (Status & ICQ_STATUS_DND) return gMainWindow->pmDnd;
  if (Status & ICQ_STATUS_OCCUPIED) return gMainWindow->pmOccupied;
  if (Status & ICQ_STATUS_NA) return gMainWindow->pmNa;
  if (Status & ICQ_STATUS_AWAY) return gMainWindow->pmAway;
  if (Status & ICQ_STATUS_FREEFORCHAT) return gMainWindow->pmFFC;

  return gMainWindow->pmOnline;
}

QPixmap& CMainWindow::iconForEvent(unsigned short SubCommand)
{
  switch(SubCommand)
  {
  case ICQ_CMDxSUB_URL:
    return gMainWindow->pmUrl;
  case ICQ_CMDxSUB_CHAT:
    return gMainWindow->pmChat;
  case ICQ_CMDxSUB_FILE:
    return gMainWindow->pmFile;
  case ICQ_CMDxSUB_CONTACTxLIST:
    return gMainWindow->pmContact;
  case ICQ_CMDxSUB_AUTHxREQUEST:
  case ICQ_CMDxSUB_AUTHxREFUSED:
  case ICQ_CMDxSUB_AUTHxGRANTED:
    return gMainWindow->pmAuthorize;
  case ICQ_CMDxSUB_MSG:
  default:
    return gMainWindow->pmMessage;
  }
}


static XErrorHandler old_handler = 0;
static int licq_xerrhandler(Display* dpy, XErrorEvent* err)
{
  // XScreenSaverQueryInfo produces a BadDrawable error
  // if it cannot connect to the extension. This happens i.e. when
  // client runs on a 64bit machine and the server on a 32bit one.
  // We need to catch that here and tell the Xlib that we
  // ignore it, otherwise Qt's handler will terminate us. :-(
  if(err->error_code == BadDrawable)
    return 0;

  return (*old_handler)(dpy, err);
}

CMainWindow* gMainWindow = NULL;

//-----CMainWindow::constructor-------------------------------------------------
CMainWindow::CMainWindow(CICQDaemon *theDaemon, CSignalManager *theSigMan,
                         CQtLogWindow *theLogWindow, bool bStartHidden,
                         const char *skinName, const char *iconsName,
                         QWidget *parent)
  : QWidget(parent, "MainWindow")
{
  gMainWindow = this;
  licqDaemon = theDaemon;
  licqSigMan = theSigMan;
  licqLogWindow = theLogWindow;

  // Overwrite Qt's event handler
  old_handler = XSetErrorHandler(licq_xerrhandler);

  // allocating floating window vector
  if(CUserView::floaties == NULL)
      CUserView::floaties = new UserFloatyList;

  // set up appicon and docking, code supplied by Mark Deneed
  WId win = winId();     // get the window
  XWMHints *hints;  // hints
  Display *dsp = x11Display();  // get the display
  hints = XGetWMHints(dsp, win);  // init hints
  hints->window_group = win;  // set set the window hint
  hints->flags = WindowGroupHint;  // set the window group hint
  XSetWMHints(dsp, win, hints);  // set the window hints for WM to use.
  XFree( hints );

  // read in info from file
  char szTemp[MAX_FILENAME_LEN];

#ifdef USE_KDE
  gLog.Info("%sKDE GUI configuration.\n", L_INITxSTR);
#else
  gLog.Info("%sQt GUI configuration.\n", L_INITxSTR);
#endif
  sprintf(szTemp, "%s/licq_qt-gui.conf", BASE_DIR);
  CIniFile licqConf;
  if (! licqConf.LoadFile(szTemp) )
  {
    FILE *f = fopen(szTemp, "w");
    fprintf(f, "%s", QTGUI_CONF);
    fclose(f);
    licqConf.LoadFile(szTemp);
  }

  licqConf.SetSection("appearance");
  QFont f;
  licqConf.ReadStr("Font", szTemp, "default");
#ifdef USE_KDE
  defaultFont = KGlobal::generalFont();
#else
  defaultFont = qApp->font();
#endif
  if (strcmp(szTemp, "default") != 0)
  {
    f.setRawName(szTemp);
    qApp->setFont(f, true);
  }
  licqConf.ReadStr("EditFont", szTemp, "default");
  if(!strcmp(szTemp, "default"))
    f = qApp->font();
  else
    f.setRawName(szTemp);
  delete MLEditWrap::editFont;
  MLEditWrap::editFont = new QFont(f);

  licqConf.ReadBool("GridLines", m_bGridLines, false);
  licqConf.ReadBool("FontStyles", m_bFontStyles, true);
  licqConf.ReadBool("ShowHeader", m_bShowHeader, true);
  licqConf.ReadBool("ShowOfflineUsers", m_bShowOffline, true);
  licqConf.ReadBool("ShowDividers", m_bShowDividers, true);
  licqConf.ReadBool("SortByStatus", m_bSortByStatus, true);
  licqConf.ReadBool("ShowGroupIfNoMsg", m_bShowGroupIfNoMsg, true);
  licqConf.ReadBool("BoldOnMsg", m_bBoldOnMsg, true);
  licqConf.ReadBool("ManualNewUser", m_bManualNewUser, false);
  licqConf.ReadBool("UseThreadView", m_bThreadView, false);

  unsigned short nFlash;
  licqConf.ReadNum("Flash", nFlash, FLASH_URGENT);
  m_nFlash = (FlashType)nFlash;
  bool bFrameTransparent;
  licqConf.ReadBool("Transparent", bFrameTransparent, false);
  unsigned short nFrameStyle;
  licqConf.ReadNum("FrameStyle", nFrameStyle, 51);
  bool bDockIcon48;
  unsigned short nDockMode;
  licqConf.ReadNum("UseDock", nDockMode, (unsigned short)DockNone);
  m_nDockMode = (DockMode)nDockMode;
  licqConf.ReadBool("Dock64x48", bDockIcon48, false);
  char szDockTheme[64];
  licqConf.ReadStr("DockTheme", szDockTheme, "");
  bool bHidden;
  licqConf.ReadBool("Hidden", bHidden, false);
  licqConf.ReadBool("AutoRaise", m_bAutoRaise, true);

  licqConf.SetSection("startup");
  licqConf.ReadNum("Logon", m_nAutoLogon, 0);
  if (m_nAutoLogon > 16) m_nAutoLogon = 0;
  licqConf.ReadNum("AutoAway", autoAwayTime, 0);
  licqConf.ReadNum("AutoNA", autoNATime, 0);
  licqConf.ReadNum("AutoOffline", autoOfflineTime, 0);

  licqConf.SetSection("functions");
  licqConf.ReadBool("AutoClose", m_bAutoClose, true);
  licqConf.ReadBool("AutoPopup", m_bAutoPopup, false);

  m_nCurrentGroup = gUserManager.DefaultGroup();
  m_nGroupType = GROUPS_USER;

  // load up position and size from file
  licqConf.SetSection("geometry");
  unsigned short xPos, yPos, hVal, wVal;
  licqConf.ReadNum("x", xPos, 100);
  licqConf.ReadNum("y", yPos, 100);
  licqConf.ReadNum("h", hVal, 400);
  licqConf.ReadNum("w", wVal, 150);
  gLog.Info("%sGeometry configuration (%d, %d) (%d x %d)\n", L_INITxSTR,
            xPos, yPos, wVal, hVal);
  if (yPos > QApplication::desktop()->height() - 16) yPos = 0;
  if (xPos > QApplication::desktop()->width() - 16) xPos = 0;

  // Load the icons
  licqConf.SetSection("appearance");
  licqConf.SetFlags(0);
  char szIcons[MAX_FILENAME_LEN];
  if (strlen(iconsName) == 0)
    licqConf.ReadStr("Icons", szIcons);
  else
    strcpy(szIcons, iconsName);
  m_szIconSet = NULL;

  // Load the skin
  char szSkin[MAX_FILENAME_LEN];
  if (strlen(skinName) == 0)
    licqConf.ReadStr("Skin", szSkin);
  else
    strcpy(szSkin, skinName);

  style = new QWindowsStyle;
  awayMsgDlg = NULL;
  optionsDlg = NULL;
  registerUserDlg = NULL;
  m_nRealHeight = 0;

  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  m_szCaption = tr("Licq (%1)").arg(QString::fromLocal8Bit(o->GetAlias()));
  gUserManager.DropOwner();
  setCaption(m_szCaption);

  // Group Combo Box
  cmbUserGroups = new CEComboBox(false, this);
  connect(cmbUserGroups, SIGNAL(activated(int)), this, SLOT(setCurrentGroup(int)));

  // Widgets controlled completely by the skin
  btnSystem = NULL;
  lblMsg = NULL;
  lblStatus = NULL;
  pmBorder = NULL;
  pmMask = NULL;
  skin = NULL;
  menu = NULL;

  ApplyIcons(szIcons, true);
  initMenu();
  ApplySkin(szSkin, true);
  skin->frame.frameStyle = nFrameStyle;
  skin->frame.transparent = bFrameTransparent;

  // set the icon
  setIconText("Licq");

  // User List
  char key[16], colTitle[32], colFormat[32];
  unsigned short colWidth, colAlign, numColumns;
  licqConf.ReadNum("NumColumns", numColumns, 1);
  for (unsigned short i = 1; i <= numColumns; i++)
  {
     sprintf(key, "Column%d.Title", i);
     licqConf.ReadStr(key, colTitle, "Alias");
     sprintf(key, "Column%d.Format", i);
     licqConf.ReadStr(key, colFormat, "%a");
     sprintf(key, "Column%d.Width", i);
     licqConf.ReadNum(key, colWidth, 100);
     sprintf(key, "Column%d.Align", i);
     licqConf.ReadNum(key, colAlign, 0);
     colInfo.push_back(new CColumnInfo(QString::fromLocal8Bit(colTitle), colFormat, colWidth, colAlign));
  }
  CreateUserView();

  unsigned short nFloaties = 0, xPosF, yPosF, wValF;
  unsigned long nUin;
  licqConf.SetSection("floaties");
  licqConf.ReadNum("Num", nFloaties, 0);
  for (unsigned short i = 0; i < nFloaties; i++)
  {
    sprintf(key, "Floaty%d.Uin", i);
    licqConf.ReadNum(key, nUin, 0);
    sprintf(key, "Floaty%d.X", i);
    licqConf.ReadNum(key, xPosF, 0);
    sprintf(key, "Floaty%d.Y", i);
    licqConf.ReadNum(key, yPosF, 0);
    sprintf(key, "Floaty%d.W", i);
    licqConf.ReadNum(key, wValF, 80);

    if (nUin != 0)
      CreateUserFloaty(nUin, xPosF, yPosF, wValF);
  }


#ifdef USE_DOCK
  licqIcon = NULL;
  switch (m_nDockMode)
  {
    case DockDefault:
      licqIcon = new IconManager_Default(this, mnuSystem, bDockIcon48);
      break;
    case DockThemed:
      licqIcon = new IconManager_Themed(this, mnuSystem, szDockTheme);
      break;
    case DockNone:
      break;
  }
#endif

   autoAwayTimer.start(10000);  // start the inactivity timer for auto away

   connect (&autoAwayTimer, SIGNAL(timeout()), this, SLOT(autoAway()));
   connect (licqSigMan, SIGNAL(signal_updatedList(CICQSignal *)),
            this, SLOT(slot_updatedList(CICQSignal *)));
   connect (licqSigMan, SIGNAL(signal_updatedUser(CICQSignal *)),
            this, SLOT(slot_updatedUser(CICQSignal *)));
   connect (licqSigMan, SIGNAL(signal_updatedStatus()), this, SLOT(updateStatus()));
   connect (licqSigMan, SIGNAL(signal_doneOwnerFcn(ICQEvent *)),
            this, SLOT(slot_doneOwnerFcn(ICQEvent *)));
   connect (licqSigMan, SIGNAL(signal_logon()),
            this, SLOT(slot_logon()));

   m_bInMiniMode = false;
   updateStatus();
   updateEvents();
   updateGroups();
   manualAway = 0;

   resize(wVal, hVal);
   move(xPos, yPos);
   if (!bHidden && !bStartHidden) show();

   // automatically logon if requested in conf file
   if (m_nAutoLogon > 0)
   {
      if (m_nAutoLogon >= 10)
        mnuStatus->setItemChecked(ICQ_STATUS_FxPRIVATE, true);

      switch (m_nAutoLogon % 10)
      {
      case 1: changeStatus(ICQ_STATUS_ONLINE); break;
      case 2: changeStatus(ICQ_STATUS_AWAY); break;
      case 3: changeStatus(ICQ_STATUS_NA); break;
      case 4: changeStatus(ICQ_STATUS_OCCUPIED); break;
      case 5: changeStatus(ICQ_STATUS_DND); break;
      case 6: changeStatus(ICQ_STATUS_FREEFORCHAT); break;
      default: gLog.Warn("%sInvalid auto online id: %d.\n", L_WARNxSTR, m_nAutoLogon);
      }
   }

   // verify we exist
   if (gUserManager.OwnerUin() == 0)
     slot_register();
}

//-----ApplySkin----------------------------------------------------------------
void CMainWindow::ApplySkin(const char *_szSkin, bool _bInitial)
{
  gLog.Info("%sApplying %s skin.\n", L_INITxSTR, _szSkin);

  if (skin != NULL) delete skin;
  skin = new CSkin(_szSkin);

  // Set the background pixmap and mask
  if (skin->frame.pixmap != NULL)
  {
     if (pmBorder != NULL) delete pmBorder;
     pmBorder = new QPixmap(skin->frame.pixmap);
     if (pmBorder->isNull())
     {
       gLog.Error("%sError loading background pixmap (%s).\n", L_ERRORxSTR, skin->frame.pixmap);
       delete skin->frame.pixmap;
       skin->frame.pixmap = NULL;
     }
  }
  else
  {
    setBackgroundMode(PaletteBackground);
  }

  if (skin->frame.mask != NULL)
  {
     if (pmMask != NULL) delete pmMask;
     pmMask = new QPixmap(skin->frame.mask);
     if (pmMask->isNull())
     {
       gLog.Error("%sError loading background mask (%s).\n", L_ERRORxSTR, skin->frame.mask);
       delete skin->frame.mask;
       skin->frame.mask = NULL;
     }
  }
  else
  {
    clearMask();
  }

  // Group Combo Box
  cmbUserGroups->setNamedBgColor(skin->cmbGroups.color.bg);
  cmbUserGroups->setNamedFgColor(skin->cmbGroups.color.fg);

  // System Button
  if (btnSystem != NULL) delete btnSystem;
  if (menu != NULL)
  {
    menu->removeItemAt(0);
    delete menu;
  }
  if (!skin->frame.hasMenuBar)
  {
     if (skin->btnSys.pixmapUpNoFocus == NULL)
     {
        btnSystem = new CEButton(skin->btnSys.caption == NULL ?
                                 tr("System") : QString::fromLocal8Bit(skin->btnSys.caption), this);
     }
     else
     {
       btnSystem = new CEButton(new QPixmap(skin->btnSys.pixmapUpFocus),
                                new QPixmap(skin->btnSys.pixmapUpNoFocus),
                                new QPixmap(skin->btnSys.pixmapDown),
                                this);
       btnSystem->setStyle(style);
     }
     connect(btnSystem, SIGNAL(clicked()), this, SLOT(popupSystemMenu()));
     btnSystem->setNamedFgColor(skin->btnSys.color.fg);
     btnSystem->setNamedBgColor(skin->btnSys.color.bg);
     menu = NULL;
  }
  else
  {
#ifdef USE_KDE
    menu = new KMenuBar(this);
#else
    menu = new QMenuBar(this);
#endif
    menu->insertItem(skin->btnSys.caption == NULL ?
                     tr("&System") : QString::fromLocal8Bit(skin->btnSys.caption),
                     mnuSystem);
    btnSystem = NULL;
    skin->AdjustForMenuBar(menu->height());
  }

  // Message Label
  if (lblMsg != NULL) delete lblMsg;
  lblMsg = new CELabel(skin->lblMsg.transparent, mnuUserGroups, this);
  if (skin->lblMsg.pixmap != NULL || skin->lblMsg.transparent)
    lblMsg->setStyle(style);
  lblMsg->setFrameStyle(skin->lblMsg.frameStyle);
  lblMsg->setIndent(skin->lblMsg.margin);
  lblMsg->setNamedFgColor(skin->lblMsg.color.fg);
  lblMsg->setNamedBgColor(skin->lblMsg.color.bg);
  if (skin->lblMsg.pixmap != NULL)
    lblMsg->setBackgroundPixmap(QPixmap(skin->lblMsg.pixmap));
  connect(lblMsg, SIGNAL(doubleClicked()), this, SLOT(callMsgFunction()));
  QToolTip::add(lblMsg, tr("Right click - User groups\n"
                           "Double click - Show next message"));

  // Status Label
  if (lblStatus != NULL) delete lblStatus;
  lblStatus = new CELabel(skin->lblStatus.transparent, mnuStatus, this);
  if (skin->lblStatus.pixmap != NULL || skin->lblStatus.transparent)
    lblStatus->setStyle(style);
  lblStatus->setFrameStyle(skin->lblStatus.frameStyle);
  lblStatus->setIndent(skin->lblStatus.margin);
  lblStatus->setNamedFgColor(skin->lblStatus.color.fg);
  lblStatus->setNamedBgColor(skin->lblStatus.color.bg);
  if (skin->lblStatus.pixmap != NULL)
    lblStatus->setBackgroundPixmap(QPixmap(skin->lblStatus.pixmap));
  connect(lblStatus, SIGNAL(doubleClicked()), this, SLOT(slot_AwayMsgDlg()));
  QToolTip::add(lblStatus, tr("Right click - Status menu\n"
                              "Double click - Set auto response"));

  if (!_bInitial)
  {
    resizeEvent(NULL);
    userView->setFrameStyle(skin->frame.frameStyle);
    userView->setColors(skin->colors.online, skin->colors.away,
                        skin->colors.offline, skin->colors.newuser,
                        skin->colors.background, skin->colors.gridlines);
    // Update all the floaties
    CUserView::UpdateFloaties();

    if (btnSystem != NULL) btnSystem->show();
    lblStatus->show();
    lblMsg->show();
    if (menu != NULL) menu->show();
    updateUserWin();
    updateEvents();
    updateStatus();
  }
}


//-----CMainWindow::CreateUserView---------------------------------------------
void CMainWindow::CreateUserView()
{
  userView = new CUserView(mnuUser, this);
  userView->setColors(skin->colors.online, skin->colors.away, skin->colors.offline,
                      skin->colors.newuser, skin->colors.background, skin->colors.gridlines);
  connect (userView, SIGNAL(doubleClicked(QListViewItem *)), SLOT(callDefaultFunction(QListViewItem *)));
}


//-----CMainWindow::CreateUserFloaty---------------------------------------------
void CMainWindow::CreateUserFloaty(unsigned long nUin, unsigned short x,
   unsigned short y, unsigned short w)
{
  if (nUin == 0) return;
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
  if (u == NULL) return;

  CUserView *f = new CUserView(mnuUser);
  connect (f, SIGNAL(doubleClicked(QListViewItem *)), SLOT(callDefaultFunction(QListViewItem *)));

  CUserViewItem *i = new CUserViewItem(u, f);

  gUserManager.DropUser(u);

  // not so good, we should allow for multiple guys in one box...
  // perhaps use the viewport sizeHint
  f->setFixedHeight(i->height() + f->frameWidth() * 2);

  if (w != 0)
  {
    if (y > QApplication::desktop()->height() - 16) y = 0;
    if (x > QApplication::desktop()->width() - 16) x = 0;
    f->setGeometry(x, y, w, f->height());
  }

  f->show();
}


//-----CMainWindow::destructor--------------------------------------------------
CMainWindow::~CMainWindow()
{
#ifdef USE_DOCK
  if (licqIcon != NULL) delete licqIcon;
#endif

  gMainWindow = NULL;
}


//-----CMainWindow::resizeEvent------------------------------------------------
void CMainWindow::resizeEvent (QResizeEvent *)
{
  userView->setGeometry(skin->frame.border.left, skin->frame.border.top,
                        width() - skin->frameWidth(), height() - skin->frameHeight());

  if (!skin->frame.hasMenuBar)
    btnSystem->setGeometry(skin->borderToRect(&skin->btnSys, this));

  cmbUserGroups->setGeometry(skin->borderToRect(&skin->cmbGroups, this));
  lblMsg->setGeometry(skin->borderToRect(&skin->lblMsg, this));
  lblStatus->setGeometry(skin->borderToRect(&skin->lblStatus, this));

  // Resize the background pixmap and mask
  QPixmap *p;
  if (skin->frame.pixmap != NULL)
  {
    p = ScaleWithBorder(*pmBorder, width(), height(), skin->frame.border);
    setBackgroundPixmap(*p);
    delete p;
  }
  if (skin->frame.mask != NULL)
  {
    p = ScaleWithBorder(*pmMask, width(), height(), skin->frame.border);
    bmMask = *p;
    setMask(bmMask);
    delete p;
  }
}


void CMainWindow::closeEvent( QCloseEvent *e )
{
#if 0
    qDebug("closeEvent() visible %d", isVisible());
    qDebug("geometry: x: %d y: %d, w: %d, h: %d", geometry().x(), geometry().y(), geometry().width(), geometry().height());
    qDebug("x: %d, y: %d, w: %d, h: %d", x(), y(), size().width(), size().height());
    qDebug(" toGlobal: x %d, y %d", mapToGlobal(QPoint(0,0)).x(), mapToGlobal(QPoint(0,0)).y());
#endif

  if(isVisible())
  {
    // save window position and size
    char buf[MAX_FILENAME_LEN];
    sprintf(buf, "%s/licq_qt-gui.conf", BASE_DIR);
    CIniFile licqConf(INI_FxALLOWxCREATE | INI_FxWARN);
    // need some more error checking here...
    licqConf.LoadFile(buf);

    licqConf.SetSection("appearance");
    licqConf.WriteBool("Hidden", !isVisible());

    int x, y;
    if(pos().x() < 2 || pos().y() < 2) {
      // WMaker bug.  will investigate...
      QPoint p  = mapToGlobal(QPoint(0, 0));

//      qDebug("wmaker workaround enabled");

      x = p.x() - 1 - ( geometry().x() < p.x() ? geometry().x() : 0);
      y = p.y() - 1 - ( geometry().y() < p.y() ? geometry().y() : 0);
    }
    else
    {
      x = pos().x();
      y = pos().y();
    }

    x = x < 0 ? 0 : x;
    y = y < 0 ? 0 : y;

    licqConf.SetSection("geometry");
    // I'm not sure if we should really test for negative values...
    licqConf.WriteNum("x", (unsigned short)x);
    licqConf.WriteNum("y", (unsigned short)y);
    licqConf.WriteNum("h", (unsigned short)(size().height() < 0 ? 0 : (m_bInMiniMode ? m_nRealHeight : size().height())));
    licqConf.WriteNum("w", (unsigned short)(size().width() < 0 ? 0 : size().width()));

#if 0
    licqConf.SetSection("floaties");
    licqConf.WriteNum("Num", (unsigned short)CUserView::floaties->size());
    unsigned short i = 0;
    char key[32];
    for (; i < CUserView::floaties->size(); )
    {
      CUserView* iter = CUserView::floaties->at(i);
      sprintf(key, "Floaty%d.Uin", i);
      licqConf.WriteNum(key, iter->firstChild()->ItemUin());
      sprintf(key, "Floaty%d.X", i);
      licqConf.WriteNum(key, (unsigned short)(iter->x() > 0 ? iter->x() : 0));
      sprintf(key, "Floaty%d.Y", i);
      licqConf.WriteNum(key, (unsigned short)(iter->y() > 0 ? iter->y() : 0));
      sprintf(key, "Floaty%d.W", i);
      licqConf.WriteNum(key, (unsigned short)iter->width());
      i++;
    }
#endif

    licqConf.FlushFile();
    licqConf.CloseFile();
  }
  else
    qDebug("MainWindow was hidden, didn't update geometry");

  if (licqIcon != NULL) {
    e->ignore();
    hide();
  }
  else
    e->accept();
}


// Receive key press events for the main widget
// Ctrl-M : toggle mini mode
// Ctrl-H : hide window
// Ctrl-I : next message
// Ctrl-X : exit
// Ctrl-V : View message
// Ctrl-S : Send message
// Ctrl-U : Send Url
// Ctrl-C : Send chat request
// Ctrl-F : Send File
// Ctrl-A : Check Auto response
// Ctrl-P : Popup all messages
// Ctrl-O : Toggle offline users
// Ctrl-L : Redraw user window
// Delete : Remove the user from the group
// Ctrl-Delete : Remove the user from the list

void CMainWindow::keyPressEvent(QKeyEvent *e)
{
  unsigned long nUin = userView->MainWindowSelectedItemUin();

  if (e->key() == Qt::Key_Delete)
  {
    if (nUin == 0) return;
    if (e->state() & ControlButton)
      RemoveUserFromList(nUin, this);
    else
      RemoveUserFromGroup(m_nGroupType, m_nCurrentGroup, nUin, this);
    //e->accept();
    return;
  }
  else if (! (e->state() & ControlButton))
  {
    e->ignore();
    QWidget::keyPressEvent(e);
    return;
  }

  switch(e->key())
  {
    case Qt::Key_M:
      miniMode();
      break;

    case Qt::Key_H:
      hide();
      break;

    case Qt::Key_I:
      callMsgFunction();
      break;

    case Qt::Key_X:
      slot_shutdown();
      break;

    case Qt::Key_V:
      callFunction(mnuUserView, nUin);
      break;

    case Qt::Key_S:
      callFunction(mnuUserSendMsg, nUin);
      break;

    case Qt::Key_U:
      callFunction(mnuUserSendUrl, nUin);
      break;

    case Qt::Key_C:
      callFunction(mnuUserSendChat, nUin);
      break;

    case Qt::Key_F:
      callFunction(mnuUserSendFile, nUin);
      break;

    case Qt::Key_A:
      (void) new ShowAwayMsgDlg(licqDaemon, licqSigMan, nUin);
      break;

    case Qt::Key_P:
      slot_popupall();
      break;

    case Qt::Key_O:
      ToggleShowOffline();
      break;

    case Qt::Key_L:
      updateUserWin();
      break;

    default:
      e->ignore();
      QWidget::keyPressEvent(e);
      break;
  }

}


//-----CMainWindow::mouseEvent----------------------------------------------
void CMainWindow::mousePressEvent(QMouseEvent *m)
{
   mouseX = m->x();
   mouseY = m->y();
}

void CMainWindow::mouseMoveEvent(QMouseEvent *m)
{
   int deltaX = m->x() - mouseX;
   int deltaY = m->y() - mouseY;
   move(x() + deltaX, y() + deltaY);
}


// ---------------------------------------------------------------------------

void CMainWindow::slot_updatedUser(CICQSignal *sig)
{
  unsigned long nUin = sig->Uin();
  switch(sig->SubSignal())
  {
    case USER_EVENTS:
    {
      // Skip all this if it was just an away message check
      if (sig->Argument() == 0) {
        userView->AnimationAutoResponseCheck(nUin);
        break;
      }
      // Otherwise an event was added or removed
      updateEvents();
      // autoRaise if needed
      if(m_bAutoRaise && sig->Argument() > 0)  raise();

      if (m_bAutoPopup && sig->Argument() > 0)
      {
        ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
        if (u != NULL && u->NewMessages() > 0)
        {
          gUserManager.DropUser(u);
          ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
          unsigned short s = o->Status();
          gUserManager.DropOwner();
          if (s == ICQ_STATUS_ONLINE || s == ICQ_STATUS_FREEFORCHAT)
            callFunction(mnuUserView, nUin);
        }
        else
        {
          gUserManager.DropUser(u);
        }
      }
      // Fall through
    }
    case USER_BASIC:
    case USER_GENERAL:
    case USER_EXT:
    case USER_STATUS:
    {
      if (nUin == gUserManager.OwnerUin())
      {
        if (sig->SubSignal() == USER_STATUS || sig->SubSignal() == USER_EXT) break;
        ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
        m_szCaption = tr("Licq (%1)").arg(QString::fromLocal8Bit(o->GetAlias()));
        gUserManager.DropOwner();
        if (caption()[0] == '*')
          setCaption(QString("* ") + m_szCaption);
        else
          setCaption(m_szCaption);
        break;
      }
      ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
      if (u == NULL)
      {
        gLog.Warn("%sCMainWindow::slot_updatedUser(): Invalid uin received: %ld\n",
                   L_ERRORxSTR, nUin);
        break;
      }
      if(m_bThreadView && m_nGroupType == GROUPS_USER && m_nCurrentGroup == 0)
      {
        CUserViewItem* i = userView->firstChild();

        while(i) {
          if(u->GetInGroup(GROUPS_USER, i->GroupId())) {
            CUserViewItem* it = i->firstChild();

            while(it) {
              if(it->ItemUin() == nUin) {
                delete it;
                if (m_bShowOffline || !u->StatusOffline() || (!m_bShowOffline && u->NewMessages() > 0))
                  (void) new CUserViewItem(u, i);
                break;
              }
              it = it->nextSibling();
            }
            if(it == NULL) {
              if ( (m_bShowOffline || (!m_bShowOffline && u->NewMessages() > 0) || !u->StatusOffline()) &&
                   ((i->GroupId() != 0 && u->GetInGroup(GROUPS_USER, i->GroupId())) ||
                    (i->GroupId() == 0 && u->GetGroups(GROUPS_USER) == 0 && !u->IgnoreList())))
                (void) new CUserViewItem(u, i);
            }
          }
          i = i->nextSibling();
        }
      }
      else if(u->GetInGroup(m_nGroupType, m_nCurrentGroup))
      {
        // Update this user if they are in the current group
        CUserViewItem *i = (CUserViewItem *)userView->firstChild();
        while (i && i->ItemUin() != nUin)
          i = (CUserViewItem *)i->nextSibling();
        if (i != NULL)
        {
          delete i;
          if (m_bShowOffline || !u->StatusOffline() ||
              (!m_bShowOffline && u->NewMessages() > 0))
            (void) new CUserViewItem(u, userView);
        }
        else
        {
          if ( (m_bShowOffline || (!m_bShowOffline && u->NewMessages() > 0) ||
                !u->StatusOffline()) &&
               (!u->IgnoreList() || (m_nGroupType == GROUPS_SYSTEM && m_nCurrentGroup == GROUP_IGNORE_LIST)) )
            (void) new CUserViewItem(u, userView);
        }
      }

      if(sig->SubSignal() == USER_STATUS && sig->Argument() == 1)
        userView->AnimationOnline(nUin);
      // Update their floaty
      CUserView *v = CUserView::FindFloaty(nUin);
      if (v != NULL )
      {
        static_cast<CUserViewItem*>(v->firstChild())->setGraphics(u);
        v->triggerUpdate();
      }
      gUserManager.DropUser(u);
      break;
    }
  }
}


// ---------------------------------------------------------------------------

void CMainWindow::slot_updatedList(CICQSignal *sig)
{
  switch(sig->SubSignal())
  {
    case LIST_ALL:
    {
      updateUserWin();
      break;
    }
    case LIST_ADD:
    {
      ICQUser *u = gUserManager.FetchUser(sig->Uin(), LOCK_R);
      if (u == NULL)
      {
        gLog.Warn("%sCMainWindow::slot_updatedList(): Invalid uin received: %ld\n",
                   L_ERRORxSTR, sig->Uin());
        break;
      }

      if(m_bThreadView && m_nGroupType == GROUPS_USER && m_nCurrentGroup == 0)
      {
        CUserViewItem* i = userView->firstChild();

        while(i)
        {
          if(u->GetInGroup(GROUPS_USER, i->GroupId()) &&
             (m_bShowOffline || !u->StatusOffline() || (!m_bShowOffline && u->NewMessages() > 0)))
            (void) new CUserViewItem(u, i);

          i = i->nextSibling();
        }
      }
      else
      {

        if (u->GetInGroup(m_nGroupType, m_nCurrentGroup) &&
            (m_bShowOffline || !u->StatusOffline() || (!m_bShowOffline && u->NewMessages() > 0)) )
          (void) new CUserViewItem(u, userView);
      }
      gUserManager.DropUser(u);
      break;
    }

    case LIST_REMOVE:
    {
      // delete their entries in the user list
      QListViewItemIterator it(userView);
      while(it.current())
      {
        CUserViewItem* item = static_cast<CUserViewItem*>(it.current());

        if(sig->Uin() > 0 && item->ItemUin() == sig->Uin()) {
          ++it;
          delete item;
          item = 0;
        }
        else
          ++it;
      }

      updateEvents();
      // If their box is open, kill it
      {
        QListIterator<UserViewEvent> it(licqUserView);
        for (; it.current() != NULL; ++it)
        {
          if ((*it)->Uin() == sig->Uin())
          {
            delete it.current();
            licqUserView.remove(it.current());
            break;
          }
        }
      }
      {
        // if their info box is open, kill it
        QListIterator<UserInfoDlg> it(licqUserInfo);
        for(; it.current() != NULL; ++it)
        {
          if((*it)->Uin() == sig->Uin())
          {
            delete it.current();
            licqUserInfo.remove(it.current());
            break;
          }
        }
      }
      break;
    }

  }  // Switch
}

//-----CMainWindow::updateUserWin-----------------------------------------------
void CMainWindow::updateUserWin()
{
  // set the pixmap and color for each user and add them to the view
  userView->setUpdatesEnabled(false);
  userView->clear();

  bool doGroupView = m_bThreadView &&
    m_nGroupType == GROUPS_USER && m_nCurrentGroup == 0;

  if(doGroupView) {
    CUserViewItem* gi = new CUserViewItem(0, tr("Other Users"), userView);
    gi->setOpen(true);
    GroupList *g = gUserManager.LockGroupList(LOCK_R);
    for (unsigned short i = 0; i < g->size(); i++) {
      gi = new CUserViewItem(i+1, (*g)[i], userView);
      // FIXME This should respect users settings
      gi->setOpen(true);
    }
    gUserManager.UnlockGroupList();
  }
  FOR_EACH_USER_START(LOCK_R)
  {
    // Only show users on the current group and not on the ignore list
    if (!doGroupView && (!pUser->GetInGroup(m_nGroupType, m_nCurrentGroup) ||
      (pUser->IgnoreList() && m_nGroupType != GROUPS_SYSTEM && m_nCurrentGroup != GROUP_IGNORE_LIST) ))
      FOR_EACH_USER_CONTINUE

    // Ignore offline users if necessary
    if (!m_bShowOffline && pUser->StatusOffline() && pUser->NewMessages() == 0)
      FOR_EACH_USER_CONTINUE;

    if(doGroupView) {
      for(CUserViewItem* gi = userView->firstChild(); gi; gi = gi->nextSibling())
      {
        if((gi->GroupId() != 0 && pUser->GetInGroup(GROUPS_USER, gi->GroupId())) ||
           (gi->GroupId() == 0 && pUser->GetGroups(GROUPS_USER) == 0 && !pUser->IgnoreList()))
          (void) new CUserViewItem(pUser, gi);
      }
    }
    else
      // Add the user to the list
      (void) new CUserViewItem(pUser, userView);
  }
  FOR_EACH_USER_END
  userView->setUpdatesEnabled(true);
  userView->triggerUpdate();
}


void CMainWindow::updateEvents()
{
  QString szCaption;

  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  unsigned short nNumOwnerEvents = o->NewMessages();
  gUserManager.DropOwner();
  unsigned short nNumUserEvents = ICQUser::getNumUserEvents() - nNumOwnerEvents;

  lblMsg->setBold(false);

  if (nNumOwnerEvents > 0)
  {
    lblMsg->setText(tr("SysMsg"));
    if (m_bBoldOnMsg)
      lblMsg->setBold(true);
    szCaption = "* " + m_szCaption;
  }
  else if (nNumUserEvents > 0)
  {
    lblMsg->setText(tr("%1 msg%2")
                    .arg(nNumUserEvents)
                    .arg(nNumUserEvents == 1 ? tr(" ") : tr("s")));
    if (m_bBoldOnMsg)
      lblMsg->setBold(true);
    szCaption = "* " + m_szCaption;
  }
  else
  {
    // Update the msg label if necessary
    if (m_bShowGroupIfNoMsg && ICQUser::getNumUserEvents() == 0)
      lblMsg->setText(cmbUserGroups->currentText());
    else
      lblMsg->setText(tr("No msgs"));
    szCaption = m_szCaption;
  }
  lblMsg->update();
  setCaption(szCaption);

#ifdef USE_DOCK
  if (licqIcon != NULL) licqIcon->SetDockIconMsg(nNumUserEvents, nNumOwnerEvents);
#endif
}


//-----CMainWindow::setCurrentGroup---------------------------------------------
void CMainWindow::setCurrentGroupMenu(int id)
{
  int index = mnuUserGroups->indexOf(id);
  if (index > gUserManager.NumGroups() + 2)
    index -= 2;
  else if (index > 1)
    index -= 1;

  setCurrentGroup(index);
}

void CMainWindow::setCurrentGroup(int index)
{
  m_nCurrentGroup = index;
  m_nGroupType = GROUPS_USER;
  unsigned short nNumGroups = gUserManager.NumGroups();
  if (m_nCurrentGroup > nNumGroups)
  {
    m_nCurrentGroup -= nNumGroups;
    m_nGroupType = GROUPS_SYSTEM;
  }
  // Update the combo box
  cmbUserGroups->setCurrentItem(index);
  // Update the msg label if necessary
  if (m_bShowGroupIfNoMsg && ICQUser::getNumUserEvents() == 0)
    lblMsg->setText(cmbUserGroups->currentText());

  // Update the group menu
  for (unsigned short i = 0; i < mnuUserGroups->count(); i++)
    mnuUserGroups->setItemChecked(mnuUserGroups->idAt(i), false);
  if (index > gUserManager.NumGroups())
    index += 2;
  else if (index >= 1)
    index += 1;
  mnuUserGroups->setItemChecked(mnuUserGroups->idAt(index), true);

  // Update the user window
  updateUserWin();
}

//-----CMainWindow::updateGroups------------------------------------------------
void CMainWindow::updateGroups()
{
  // update the combo box
  cmbUserGroups->clear();
  mnuUserGroups->clear();
  mnuGroup->clear();
  cmbUserGroups->insertItem(tr("All Users"));
  mnuUserGroups->insertItem(tr("All Users"));
  mnuUserGroups->insertSeparator();

  GroupList *g = gUserManager.LockGroupList(LOCK_R);
  for (unsigned short i = 0; i < g->size(); i++)
  {
    cmbUserGroups->insertItem(QString::fromLocal8Bit((*g)[i]));
    mnuUserGroups->insertItem(QString::fromLocal8Bit((*g)[i]));
    mnuGroup->insertItem(QString::fromLocal8Bit((*g)[i]), i+1);
  }
  gUserManager.UnlockGroupList();
  mnuUserGroups->insertSeparator();
  mnuGroup->insertSeparator();

  cmbUserGroups->insertItem(tr("Online Notify"));
  mnuUserGroups->insertItem(tr("Online Notify"));
  mnuGroup->insertItem(tr("Online Notify"), 1000+GROUP_ONLINE_NOTIFY);
  cmbUserGroups->insertItem(tr("Visible List"));
  mnuUserGroups->insertItem(tr("Visible List"));
  mnuGroup->insertItem(tr("Visible List"), 1000+GROUP_VISIBLE_LIST);
  cmbUserGroups->insertItem(tr("Invisible List"));
  mnuUserGroups->insertItem(tr("Invisible List"));
  mnuGroup->insertItem(tr("Invisible List"), 1000+GROUP_INVISIBLE_LIST);
  cmbUserGroups->insertItem(tr("Ignore List"));
  mnuUserGroups->insertItem(tr("Ignore List"));
  mnuGroup->insertItem(tr("Ignore List"), 1000+GROUP_IGNORE_LIST);
  cmbUserGroups->insertItem(tr("New Users"));
  mnuUserGroups->insertItem(tr("New Users"));
  mnuGroup->insertItem(tr("New Users"), 1000+GROUP_NEW_USERS);

  int index = m_nCurrentGroup;
  if (m_nGroupType == GROUPS_SYSTEM)
    index += gUserManager.NumGroups();
  setCurrentGroup(index);
}


//-----CMainWindow::updateStatus------------------------------------------------
void CMainWindow::updateStatus()
{
   char *theColor = skin->colors.offline;
   ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
   unsigned long status = o->Status();
   switch (status)
   {
   case ICQ_STATUS_OFFLINE:
     theColor = skin->colors.offline;
     break;
   case ICQ_STATUS_ONLINE:
   case ICQ_STATUS_FREEFORCHAT:
     theColor = skin->colors.online;
     break;
   case ICQ_STATUS_AWAY:
   case ICQ_STATUS_NA:
   case ICQ_STATUS_OCCUPIED:
   case ICQ_STATUS_DND:
   default:
     theColor = skin->colors.away;
     break;
   }
   if (status != ICQ_STATUS_OFFLINE)
     mnuStatus->setItemChecked(mnuStatus->idAt(MNUxITEM_STATUSxINVISIBLE), o->StatusInvisible());

   lblStatus->setText(o->StatusStr());
   lblStatus->update();

   gUserManager.DropOwner();

   // set the color if it isn't set by the skin
   if (skin->lblStatus.color.fg == NULL) lblStatus->setNamedFgColor(theColor);

#ifdef USE_DOCK
  if (licqIcon != NULL) licqIcon->SetDockIconStatus();
#endif
}


void CMainWindow::slot_AwayMsgDlg()
{
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  if(o != NULL) {
    showAwayMsgDlg(o->Status());
    gUserManager.DropOwner();
  }
}


void CMainWindow::slot_doneAwayMsgDlg()
{
  awayMsgDlg = NULL;
}


void CMainWindow::showAwayMsgDlg(unsigned short nStatus)
{
  if(awayMsgDlg == NULL) {
    awayMsgDlg = new AwayMsgDlg();
    connect(awayMsgDlg, SIGNAL(popupOptions(int)), this, SLOT(showOptionsDlg(int)));
    connect(awayMsgDlg, SIGNAL(done()), this, SLOT(slot_doneAwayMsgDlg()));
  }
  else
    awayMsgDlg->raise();

  awayMsgDlg->SelectAutoResponse(nStatus);
}


//----CMainWindow::changeStatusManual-------------------------------------------
void CMainWindow::changeStatusManual(int id)
{
  if (id != ICQ_STATUS_OFFLINE && (id & 0xFF) != ICQ_STATUS_ONLINE)
    showAwayMsgDlg(id);

  changeStatus(id);
}


//----CMainWindow::changeStatus-------------------------------------------------
void CMainWindow::changeStatus(int id)
{
  unsigned long newStatus = ICQ_STATUS_OFFLINE;

  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  if (id == ICQ_STATUS_OFFLINE)
  {
    gUserManager.DropOwner();
    licqDaemon->icqLogoff();
    return;
  }
  else if (id == (int)ICQ_STATUS_FxPRIVATE) // toggle invisible status
  {
    mnuStatus->setItemChecked(ICQ_STATUS_FxPRIVATE,
                              !mnuStatus->isItemChecked(ICQ_STATUS_FxPRIVATE));
    if (o->StatusOffline())
    {
      gUserManager.DropOwner();
      return;
    }
    if (mnuStatus->isItemChecked(ICQ_STATUS_FxPRIVATE))
       newStatus = o->StatusFull() | ICQ_STATUS_FxPRIVATE;
    else
       newStatus = o->StatusFull() & (~ICQ_STATUS_FxPRIVATE);
  }
  else
  {
    newStatus = id;
  }

  // we may have been offline and gone online with invisible toggled
  if (mnuStatus->isItemChecked(ICQ_STATUS_FxPRIVATE))
     newStatus |= ICQ_STATUS_FxPRIVATE;

  // disable combo box, flip pixmap...
  //lblStatus->setEnabled(false);

  // call the right function
  bool b = o->StatusOffline();
  gUserManager.DropOwner();
  CICQEventTag *t = NULL;
  if (b)
     t = licqDaemon->icqLogon(newStatus);
  else
     t = licqDaemon->icqSetStatus(newStatus);
  if (t != NULL) delete t;
}


// -----------------------------------------------------------------------------

void CMainWindow::callDefaultFunction(QListViewItem *i)
{
  if(i == NULL)
    return;
  unsigned long nUin = ((CUserViewItem *)i)->ItemUin();
  //userView->SelectedItemUin();
  if (nUin == 0) return;
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
  // set default function to read or send depending on whether or not
  // there are new messages
  int fcn = (u->NewMessages() == 0 ? mnuUserSendMsg : mnuUserView);
  gUserManager.DropUser(u);

  // See if the clipboard contains a url
  if (fcn == mnuUserSendMsg)
  {
    QString c = QApplication::clipboard()->text();
    if (c.left(5) == "http:" || c.left(4) == "ftp:")
    {
      UserSendUrlEvent *e = (UserSendUrlEvent *)callFunction(mnuUserSendUrl, nUin);
      if (e == NULL) return;
      // Set the url
      e->setUrl(c, "");
      // Clear the buffer now
      QApplication::clipboard()->clear();
      return;
    }
    else if (c.left(5) == "file:")
    {
      UserSendFileEvent *e = (UserSendFileEvent *)callFunction(mnuUserSendFile, nUin);
      if (e == NULL) return;
      // Set the file
      c.remove(0, 5);
      while (c[0] == '/') c.remove(0, 1);
      c.prepend('/');
      e->setFile(c, "");
      // Clear the buffer now
      QApplication::clipboard()->clear();
      return;
    }

  }

  callFunction(fcn, nUin);
}

void CMainWindow::callOwnerFunction(int index)
{
  if (index == OwnerMenuView)
    callFunction(index, gUserManager.OwnerUin());

  else if (index == OwnerMenuGeneral ||
      index == OwnerMenuMore  || index == OwnerMenuWork ||
      index == OwnerMenuAbout || index == OwnerMenuHistory)
    callInfoTab(index, gUserManager.OwnerUin());

  else if (index == OwnerMenuSecurity)
    (void) new SecurityDlg(licqDaemon, licqSigMan);

  else if (index == OwnerMenuPassword)
    (void) new PasswordDlg(licqDaemon, licqSigMan);

  else if (index == OwnerMenuRandomChat)
    (void) new CSetRandomChatGroupDlg(licqDaemon, licqSigMan);

  else
    gLog.Warn("%sInternal Error: CMainWindow::callOwnerFunction(): Unknown index (%d).\n",
              L_WARNxSTR, index);
}


void CMainWindow::callUrlFunction (const char *_szUrl)
{
//  ICQFunctions *f = callFunction(-1, true);
//  if (f != NULL) f->SendUrl(_szUrl, "");
}

void CMainWindow::callFileFunction (const char *_szFile)
{
//  ICQFunctions *f = callFunction(-1, true);
//  if (f != NULL) f->SendFile(_szFile, "");
}


void CMainWindow::callMsgFunction()
{
  // Do nothing if there are no events pending
  if (ICQUser::getNumUserEvents() == 0) return;

  // Do system messages first
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  unsigned short nNumMsg = o->NewMessages();
  gUserManager.DropOwner();
  if (nNumMsg > 0)
  {
    callOwnerFunction(OwnerMenuView);
    return;
  }

  unsigned long nUin = 0;
  time_t t = time(NULL);
  FOR_EACH_USER_START(LOCK_R)
  {
    if (pUser->NewMessages() > 0 && pUser->Touched() <= t)
    {
      nUin = pUser->Uin();
      t = pUser->Touched();
    }
  }
  FOR_EACH_USER_END
  if (nUin != 0) callFunction(mnuUserView, nUin);
}

//-----CMainWindow::callUserFunction-------------------------------------------
void CMainWindow::callUserFunction(int index)
{
  unsigned long nUin = m_nUserMenuUin;

  if (nUin == 0) return;

  switch(index)
  {
    case mnuUserAuthorize:
    {
      (void) new AuthUserDlg(licqDaemon, nUin, true);
      break;
    }
    case mnuUserCheckResponse:
    {
      (void) new ShowAwayMsgDlg(licqDaemon, licqSigMan, nUin);
      break;
    }
    case mnuUserFloaty:
    {
      // Check that the floaty does not already exist
      CUserView *v = CUserView::FindFloaty(nUin);
      if (v == NULL)
      {
        CreateUserFloaty(nUin);
      }
      else
      {
        delete v->firstChild();
        if (v->childCount() == 0) delete v;
      }
      break;
    }
    case mnuUserHistory:
    case mnuUserGeneral:
    case mnuUserMore:
    case mnuUserWork:
    case mnuUserAbout:
      callInfoTab(index, nUin);
      break;
    case mnuUserRemoveFromList:
      RemoveUserFromList(m_nUserMenuUin, this);
      break;
    default:
      callFunction(index, nUin);
  }

}

void CMainWindow::callInfoTab(int fcn, unsigned long nUin)
{
  if(nUin == 0) return;

  UserInfoDlg *f = NULL;
  QListIterator<UserInfoDlg> it(licqUserInfo);

  for(; it.current(); ++it)
  {
    if((*it)->Uin() == nUin)
    {
      f = *it;
      break;
    }
  }

  if (f)
  {
    f->show();
    f->raise();
  }
  else
  {
    f = new UserInfoDlg(licqDaemon, licqSigMan, this, nUin);
    connect(f, SIGNAL(finished(unsigned long)), this, SLOT(UserInfoDlg_finished(unsigned long)));
    f->show();
    licqUserInfo.append(f);
  }

  switch(fcn)
  {
    case mnuUserHistory:
      f->showTab(UserInfoDlg::HistoryInfo);
      break;
    case mnuUserGeneral:
      f->showTab(UserInfoDlg::GeneralInfo);
      break;
    case mnuUserMore:
      f->showTab(UserInfoDlg::MoreInfo);
      break;
    case mnuUserWork:
      f->showTab(UserInfoDlg::WorkInfo);
      break;
    case mnuUserAbout:
      f->showTab(UserInfoDlg::AboutInfo);
      break;
  }
  f->show();
  f->raise();
}




//-----CMainWindow::callICQFunction-------------------------------------------
UserEventCommon *CMainWindow::callFunction(int fcn, unsigned long nUin)
{
  if (nUin == 0) return NULL;

  UserEventCommon *e = NULL;

  switch (fcn)
  {
    case mnuUserView:
    {
      QListIterator<UserViewEvent> it(licqUserView);

      for (; it.current(); ++it)
      {
        if ((*it)->Uin() == nUin)
        {
          e = *it;
          break;
        }
      }

      if (e == NULL)
      {
        e = new UserViewEvent(licqDaemon, licqSigMan, this, nUin);
        connect (e, SIGNAL(finished(unsigned long)), SLOT(slot_userfinished(unsigned long)));
        licqUserView.append(static_cast<UserViewEvent*>(e));
      }
      else
      {
        e->raise();
#ifdef USE_KDE
        KWin::setActiveWindow(e->winId());
#endif
        return e;
      }
      break;
    }
    case mnuUserSendMsg:
    {
      e = new UserSendMsgEvent(licqDaemon, licqSigMan, this, nUin);
      break;
    }
    case mnuUserSendUrl:
    {
      e = new UserSendUrlEvent(licqDaemon, licqSigMan, this, nUin);
      break;
    }
    case mnuUserSendChat:
    {
      e = new UserSendChatEvent(licqDaemon, licqSigMan, this, nUin);
      break;
    }
    case mnuUserSendFile:
    {
      e = new UserSendFileEvent(licqDaemon, licqSigMan, this, nUin);
      break;
    }
    case mnuUserSendContact:
    {
      e = new UserSendContactEvent(licqDaemon, licqSigMan, this, nUin);
      break;
    }
    default:
      gLog.Warn("%sunknown callFunction() fcn: %d", L_WARNxSTR, fcn);
  }
  if(e != NULL)
    e->show();
  return e;
}


// -----------------------------------------------------------------------------

void CMainWindow::UserInfoDlg_finished(unsigned long nUin)
{
  QListIterator<UserInfoDlg> it(licqUserInfo);

  for( ; it.current(); ++it){
    if((*it)->Uin() == nUin) {
      licqUserInfo.remove(*it);
      return;
    }
  }

  gLog.Warn("%sUser Info finished signal for user with no window (%ld)!",
            L_WARNxSTR, nUin);
}


// -----------------------------------------------------------------------------

void CMainWindow::slot_userfinished(unsigned long nUin)
{
  QListIterator<UserViewEvent> it(licqUserView);

  for ( ; it.current(); ++it)
  {
    if ((*it)->Uin() == nUin)
    {
      licqUserView.remove(*it);
      return;
    }
  }
  gLog.Warn("%sUser finished signal for user with no window (%ld)!",
            L_WARNxSTR, nUin);
}


void CMainWindow::slot_shutdown()
{
  licqDaemon->Shutdown();
}

//-----CMainWindow::slot_logon------------------------------------------------
void CMainWindow::slot_logon()
{
  updateStatus();
  //lblStatus->setEnabled(true);
}

//-----slot_doneOwnerFcn--------------------------------------------------------
void CMainWindow::slot_doneOwnerFcn(ICQEvent *e)
{
  updateStatus();
  switch (e->Command())
  {
    case ICQ_CMDxSND_LOGON:
      if (e->Result() != EVENT_SUCCESS)
        WarnUser(this, tr("Logon failed.\nSee network window for details."));
      break;
    case ICQ_CMDxSND_REGISTERxUSER:
      delete registerUserDlg;
      registerUserDlg = NULL;
      if (e->Result() == EVENT_SUCCESS)
      {
        InformUser(this, tr("Successfully registered, your user identification\n"
                            "number (UIN) is %1.\n"
                            "Now set your personal information.").arg(gUserManager.OwnerUin()));
        callInfoTab(mnuUserGeneral, gUserManager.OwnerUin());
      }
      else
      {
        InformUser(this, tr("Registration failed.  See network window for details."));
      }
      break;
    case ICQ_CMDxSND_AUTHORIZE:
       if (e->Result() != EVENT_ACKED)
         WarnUser(this, tr("Error sending authorization."));
       else
         InformUser(this, tr("Authorization granted."));
       break;
    default:
       break;
  }
}


bool CMainWindow::RemoveUserFromList(unsigned long nUin, QWidget *p)
{
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
  if (u == NULL) return true;
  QString warning(tr("Are you sure you want to remove\n%1 (%2)\nfrom your contact list?")
                     .arg(QString::fromLocal8Bit(u->GetAlias()))
                     .arg(nUin) );
  gUserManager.DropUser(u);
  if (QueryUser(p, warning, tr("Ok"), tr("Cancel")))
  {
    licqDaemon->RemoveUserFromList(nUin);
    return true;
  }
  return false;
}

void CMainWindow::FillUserGroup()
{
  ICQUser *u = gUserManager.FetchUser(m_nUserMenuUin, LOCK_R);
  if(u == NULL) return;

  mnuGroup->setItemChecked(1000+GROUP_ONLINE_NOTIFY, u->OnlineNotify());
  mnuGroup->setItemChecked(1000+GROUP_INVISIBLE_LIST, u->InvisibleList());
  mnuGroup->setItemChecked(1000+GROUP_VISIBLE_LIST, u->VisibleList());
  mnuGroup->setItemChecked(1000+GROUP_IGNORE_LIST, u->IgnoreList());
  mnuGroup->setItemChecked(1000+GROUP_NEW_USERS, u->NewUser());

  GroupList *g = gUserManager.LockGroupList(LOCK_R);
  for (unsigned short i = 0; i < g->size(); i++)
    mnuGroup->setItemChecked(i+1, u->GetInGroup(GROUPS_USER, i+1));
  gUserManager.UnlockGroupList();
  gUserManager.DropUser(u);
}


void CMainWindow::UserGroupToggled(int id)
{
  if(id >= 0 && id < 1000)
  {
    // User groups
    if(mnuGroup->isItemChecked(id))
      RemoveUserFromGroup(GROUPS_USER, id, m_nUserMenuUin, this);
    else {
      gUserManager.AddUserToGroup(m_nUserMenuUin, id);
      updateUserWin();
    }
  }
  else if(id >= 1000)
  {
    switch(id-1000) {
    case GROUP_NEW_USERS:
    {
      ICQUser *u = gUserManager.FetchUser(m_nUserMenuUin, LOCK_W);
      if (!u) return;
      u->SetNewUser(!u->NewUser());
      gUserManager.DropUser(u);
      updateUserWin();
      break;
    }
    case GROUP_ONLINE_NOTIFY:
    {
      ICQUser *u = gUserManager.FetchUser(m_nUserMenuUin, LOCK_W);
      if (!u) return;
      u->SetOnlineNotify(!u->OnlineNotify());
      gUserManager.DropUser(u);
      if (m_bFontStyles) updateUserWin();
      break;
    }
    case GROUP_VISIBLE_LIST:
    {
      licqDaemon->icqToggleVisibleList(m_nUserMenuUin);
      if (m_bFontStyles)
        updateUserWin();
      break;
    }
    case GROUP_INVISIBLE_LIST:
    {
      licqDaemon->icqToggleInvisibleList(m_nUserMenuUin);
      if (m_bFontStyles)
        updateUserWin();
      break;
    }
    case GROUP_IGNORE_LIST:
    {
      ICQUser *u = gUserManager.FetchUser(m_nUserMenuUin, LOCK_W);
      if (!u) return;
      if(!u->IgnoreList() && !QueryUser(this,
          tr("Do you really want to add\n%1 (%2)\nto your ignore list?")
          .arg(u->GetAlias()).arg(m_nUserMenuUin), tr("&Yes"), tr("&No")))
      {
        gUserManager.DropUser(u);
        break;
      }
      u->SetIgnoreList(!u->IgnoreList());
      gUserManager.DropUser(u);
      updateUserWin();
      break;
    }
    }
  }
}


bool CMainWindow::RemoveUserFromGroup(GroupType gtype, unsigned long group, unsigned long nUin, QWidget *p)
{
  if (gtype == GROUPS_USER)
  {
    if (group == 0)
      return RemoveUserFromList(nUin, p);
    else
    {
      ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
      if (u == NULL) return true;
      unsigned long nUin = u->Uin();
      GroupList *g = gUserManager.LockGroupList(LOCK_R);
      QString warning(tr("Are you sure you want to remove\n%1 (%2)\nfrom the '%3' group?")
                         .arg(QString::fromLocal8Bit(u->GetAlias()))
                         .arg(nUin).arg(QString::fromLocal8Bit( (*g)[group - 1] )) );
      gUserManager.UnlockGroupList();
      gUserManager.DropUser(u);
      if (QueryUser(p, warning, tr("Ok"), tr("Cancel")))
      {
         gUserManager.RemoveUserFromGroup(nUin, group);
         updateUserWin();
         return true;
      }
    }
  }
  else if (gtype == GROUPS_SYSTEM)
  {
    if (group == 0) return true;
    ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
    if (u == NULL) return true;
    u->RemoveFromGroup(GROUPS_SYSTEM, group);
    gUserManager.DropUser(u);
    updateUserWin();
    return true;
  }

  return false;
}


//-----CMainWindow::saveAllUsers-----------------------------------------------
void CMainWindow::saveAllUsers()
{
   gUserManager.SaveAllUsers();
}

void CMainWindow::slot_updateContactList()
{
  licqDaemon->icqUpdateContactList();
}

void CMainWindow::slot_updateAllUsers()
{
  licqDaemon->UpdateAllUsers();
}



//-----CMainWindow::saveOptions-----------------------------------------------
void CMainWindow::saveOptions()
{
  // Tell the daemon to save it's options
  licqDaemon->SaveConf();

  // Save all our options
  char filename[MAX_FILENAME_LEN];
  sprintf(filename, "%s/licq_qt-gui.conf", BASE_DIR);
  CIniFile licqConf(INI_FxERROR | INI_FxALLOWxCREATE);
  if (!licqConf.LoadFile(filename)) return;

  licqConf.SetSection("startup");
  licqConf.WriteNum("Logon", m_nAutoLogon);
  licqConf.WriteNum("AutoAway", autoAwayTime);
  licqConf.WriteNum("AutoNA", autoNATime);
  licqConf.WriteNum("AutoOffline", autoOfflineTime);

  licqConf.SetSection("functions");
  licqConf.WriteBool("AutoClose", m_bAutoClose);
  licqConf.WriteBool("AutoPopup", m_bAutoPopup);

  licqConf.SetSection("appearance");
  licqConf.WriteStr("Skin", skin->szSkinName);
  licqConf.WriteStr("Icons", m_szIconSet);
  licqConf.WriteStr("Font", qApp->font() == defaultFont ?
                    "default" : qApp->font().rawName().latin1());
  licqConf.WriteStr("EditFont",
                    (MLEditWrap::editFont == NULL ||
                     *MLEditWrap::editFont == defaultFont) ?
                     "default" : MLEditWrap::editFont->rawName().latin1());
  licqConf.WriteBool("GridLines", m_bGridLines);
  licqConf.WriteBool("FontStyles", m_bFontStyles);
  licqConf.WriteNum("Flash", (unsigned short)m_nFlash);
  licqConf.WriteBool("ShowHeader", m_bShowHeader);
  licqConf.WriteBool("ShowDividers", m_bShowDividers);
  licqConf.WriteBool("SortByStatus", m_bSortByStatus);
  licqConf.WriteBool("ShowGroupIfNoMsg", m_bShowGroupIfNoMsg);
  licqConf.WriteBool("UseThreadView", m_bThreadView);
  licqConf.WriteBool("BoldOnMsg", m_bBoldOnMsg);
  licqConf.WriteBool("ManualNewUser", m_bManualNewUser);
  licqConf.WriteBool("Transparent", skin->frame.transparent);
  licqConf.WriteNum("FrameStyle", skin->frame.frameStyle);
  licqConf.WriteBool("ShowOfflineUsers", m_bShowOffline);
  licqConf.WriteBool("AutoRaise", m_bAutoRaise);
#ifdef USE_DOCK
  licqConf.WriteNum("UseDock", (unsigned short)m_nDockMode);
  switch(m_nDockMode)
  {
    case DockDefault:
      licqConf.WriteBool("Dock64x48", ((IconManager_Default *)licqIcon)->FortyEight());
      break;
    case DockThemed:
      licqConf.WriteStr("DockTheme", ((IconManager_Themed *)licqIcon)->Theme().latin1());
      break;
    case DockNone:
      break;
  }
#endif

  // save the column info
  licqConf.WriteNum("NumColumns", (unsigned short)colInfo.size());
  char key[32];
  for (unsigned short i = 1; i <= colInfo.size(); i++)
  {
     sprintf(key, "Column%d.Title", i);
     licqConf.WriteStr(key, colInfo[i - 1]->m_sTitle.local8Bit());
     sprintf(key, "Column%d.Format", i);
     licqConf.WriteStr(key, colInfo[i - 1]->m_szFormat);
     sprintf(key, "Column%d.Width", i);
     licqConf.WriteNum(key, colInfo[i - 1]->m_nWidth);
     sprintf(key, "Column%d.Align", i);
     licqConf.WriteNum(key, colInfo[i - 1]->m_nAlign);
  }

  licqConf.SetSection("floaties");
  licqConf.WriteNum("Num", (unsigned short)CUserView::floaties->size());
  for (unsigned short i = 0; i < CUserView::floaties->size(); i++)
  {
    CUserView* iter = CUserView::floaties->at(i);
    sprintf(key, "Floaty%d.Uin", i);
    licqConf.WriteNum(key, static_cast<CUserViewItem*>(iter->firstChild())->ItemUin());
    sprintf(key, "Floaty%d.X", i);
    licqConf.WriteNum(key, (unsigned short)(iter->x() > 0 ? iter->x() : 0));
    sprintf(key, "Floaty%d.Y", i);
    licqConf.WriteNum(key, (unsigned short)(iter->y() > 0 ? iter->y() : 0));
    sprintf(key, "Floaty%d.W", i);
    licqConf.WriteNum(key, (unsigned short)iter->width());
  }

  licqConf.FlushFile();
}


//-----CMainWindow::aboutBox--------------------------------------------------
void CMainWindow::aboutBox()
{
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  QString about(tr("Licq version %1.\n"
                   "Qt GUI plugin version %2.\n"
                   "Compiled on: %7\n"
                   "%6\n"
                   "Author: Graham Roff\n"
                   "Contributions by Dirk A. Mueller\n"
                   "http://www.licq.org\n\n"
                   "%3 (%4)\n"
                   "%5 contacts.").arg(licqDaemon->Version())
                   .arg(VERSION).arg(QString::fromLocal8Bit(o->GetAlias()))
                   .arg(o->Uin()).arg(gUserManager.NumUsers())
#ifdef USE_KDE
                   .arg(tr("(with KDE support)\n"))
#else
                   .arg("\n")
#endif
                   .arg(__DATE__));
  gUserManager.DropOwner();
  InformUser(this, about);
}


//-----CMainWindow::changeDebug-----------------------------------------------
void CMainWindow::changeDebug(int _nId)
{
  int nLevel = mnuDebug->indexOf(_nId);
  if (nLevel == MNUxITEM_DEBUGxALL)
  {
    gLog.ModifyService(S_STDOUT, L_ALL);
    for (int i = 0; i < 5; i++)
      mnuDebug->setItemChecked(mnuDebug->idAt(i), true);
    return;
  }

  if (nLevel == MNUxITEM_DEBUGxNONE)
  {
    gLog.ModifyService(S_STDOUT, L_NONE);
    for (int i = 0; i < 5; i++)
      mnuDebug->setItemChecked(mnuDebug->idAt(i), false);
    return;
  }

  // First see if we are setting on or off the value
  if (mnuDebug->isItemChecked(_nId))
  {
    gLog.RemoveLogTypeFromService(S_STDOUT, 1 << nLevel);
    mnuDebug->setItemChecked(_nId, false);
  }
  else
  {
    gLog.AddLogTypeToService(S_STDOUT, 1 << nLevel);
    mnuDebug->setItemChecked(_nId, true);
  }
}


//-----CMainWindow::slot_utility----------------------------------------------
void CMainWindow::slot_utility(int _nId)
{
  int nUtility = mnuUtilities->indexOf(_nId);
  CUtility *u = gUtilityManager.Utility(nUtility);
  if (u == NULL) return;
  //unsigned long nUin = userView->SelectedItemUin();
  unsigned long nUin = m_nUserMenuUin;
  if (nUin != 0) (void) new CUtilityDlg(u, nUin, licqDaemon);
}


//-----CMainWindow::slot_awaymodes--------------------------------------------
void CMainWindow::slot_awaymodes(int _nId)
{
  int nAwayModes = mnuAwayModes->indexOf(_nId);
  //ICQUser *u = gUserManager.FetchUser(userView->SelectedItemUin(), LOCK_W);
  ICQUser *u = gUserManager.FetchUser(m_nUserMenuUin, LOCK_W);
  if (u == NULL) return;

  switch(nAwayModes)
  {
    case 0:
      u->SetAcceptInAway(!u->AcceptInAway());
      break;
    case 1:
      u->SetAcceptInNA(!u->AcceptInNA());
      break;
    case 2:
      u->SetAcceptInOccupied(!u->AcceptInOccupied());
      break;
    case 3:
      u->SetAcceptInDND(!u->AcceptInDND());
      break;

    case 5:
      if (u->StatusToUser() == ICQ_STATUS_ONLINE)
        u->SetStatusToUser(ICQ_STATUS_OFFLINE);
      else
        u->SetStatusToUser(ICQ_STATUS_ONLINE);
      break;
    case 6:
      if (u->StatusToUser() == ICQ_STATUS_AWAY)
        u->SetStatusToUser(ICQ_STATUS_OFFLINE);
      else
        u->SetStatusToUser(ICQ_STATUS_AWAY);
      break;
    case 7:
      if (u->StatusToUser() == ICQ_STATUS_NA)
        u->SetStatusToUser(ICQ_STATUS_OFFLINE);
      else
        u->SetStatusToUser(ICQ_STATUS_NA);
      break;
    case 8:
      if (u->StatusToUser() == ICQ_STATUS_OCCUPIED)
        u->SetStatusToUser(ICQ_STATUS_OFFLINE);
      else
        u->SetStatusToUser(ICQ_STATUS_OCCUPIED);
      break;
    case 9:
      if (u->StatusToUser() == ICQ_STATUS_DND)
        u->SetStatusToUser(ICQ_STATUS_OFFLINE);
      else
        u->SetStatusToUser(ICQ_STATUS_DND);
      break;

    case 11:
    {
      gUserManager.DropUser(u);
      u = NULL;
      //(void) new CustomAwayMsgDlg(userView->SelectedItemUin());
      (void) new CustomAwayMsgDlg(m_nUserMenuUin);
    }
  }
  if (u != NULL)
    gUserManager.DropUser(u);
}



//-----CMainWindow::nextServer------------------------------------------------
void CMainWindow::nextServer()
{
  // Error reporting is done at the server level
  licqDaemon->SwitchServer();
}


//-----ToggleShowOffline------------------------------------------------------
void CMainWindow::ToggleShowOffline()
{
  m_bShowOffline = !m_bShowOffline;
  mnuSystem->setItemChecked(mnuSystem->idAt(MNUxITEM_SHOWxOFFLINE), m_bShowOffline);
  updateUserWin();
}


//-----CMainWindow::miniMode--------------------------------------------------
void CMainWindow::miniMode()
{

   if (m_bInMiniMode)
   {
      userView->show();
      setMaximumHeight(4096);
      resize(width(), m_nRealHeight);
      setMinimumHeight(100);
   }
   else
   {
      userView->hide();
      m_nRealHeight = height();
      unsigned short newH = skin->frame.border.top + skin->frame.border.bottom;
      setMinimumHeight(newH);
      resize(width(), newH);
      setMaximumHeight(newH);
   }
   m_bInMiniMode = !m_bInMiniMode;
   mnuSystem->setItemChecked(mnuSystem->idAt(MNUxITEM_MINIxMODE), m_bInMiniMode);
}


//-----CMainWindow::autoAway--------------------------------------------------
void CMainWindow::autoAway()
{
#ifdef USE_SCRNSAVER
  static XScreenSaverInfo *mit_info = NULL;
  static bool bAutoAway = false;
  static bool bAutoNA = false;
  static bool bAutoOffline = false;

  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  unsigned short status = o->Status();
  gUserManager.DropOwner();

  if (mit_info == NULL) {
    int event_base, error_base;
    if(XScreenSaverQueryExtension(x11Display(), &event_base, &error_base)) {
      mit_info = XScreenSaverAllocInfo ();
    }
    else {
      gLog.Warn("%sNo XScreenSaver extension found on current XServer, disabling auto-away.\n",
                L_WARNxSTR);
      autoAwayTimer.stop();
      return;
    }
  }

  if (!XScreenSaverQueryInfo(x11Display(), qt_xrootwin(), mit_info)) {
    gLog.Warn("%sXScreenSaverQueryInfo failed, disabling auto-away.\n",
              L_WARNxSTR);
    autoAwayTimer.stop();
    return;
  }
  Time idleTime = mit_info->idle;

  // Check no one changed the status behind our back
  if ( (bAutoOffline && status != ICQ_STATUS_OFFLINE) ||
       (bAutoNA && status != ICQ_STATUS_NA && !bAutoOffline) ||
       (bAutoAway && status != ICQ_STATUS_AWAY && !bAutoNA && !bAutoOffline) )
  {
    bAutoOffline = false;
    bAutoNA = false;
    bAutoAway = false;
    return;
  }

//  gLog.Info("offl %d, n/a %d, away %d idlt %d\n",
//            bAutoOffline, bAutoNA, bAutoAway, idleTime);

  if ( (autoOfflineTime > 0) &&
       (unsigned long)idleTime > (unsigned long)(autoOfflineTime * 60000))
  {
    if (status == ICQ_STATUS_ONLINE || status == ICQ_STATUS_AWAY || status == ICQ_STATUS_NA)
    {
      changeStatus(ICQ_STATUS_OFFLINE);
      bAutoOffline = true;
      bAutoAway = (status == ICQ_STATUS_ONLINE || bAutoAway);
      bAutoNA = ((status == ICQ_STATUS_AWAY && bAutoAway) || bAutoNA);
    }
  }
  else if ( (autoNATime > 0) &&
       (unsigned long)idleTime > (unsigned long)(autoNATime * 60000))
  {
    if (status == ICQ_STATUS_ONLINE || status == ICQ_STATUS_AWAY)
    {
      changeStatus(ICQ_STATUS_NA);
      bAutoNA = true;
      bAutoAway = (status == ICQ_STATUS_ONLINE || bAutoAway);
    }
  }
  else if ( (autoAwayTime > 0) &&
            (unsigned long)idleTime > (unsigned long)(autoAwayTime * 60000))
  {
    if (status == ICQ_STATUS_ONLINE)
    {
      changeStatus(ICQ_STATUS_AWAY);
      bAutoAway = true;
    }
  }
  else
  {
    if (bAutoOffline)
    {
      if (bAutoNA && bAutoAway)
      {
        changeStatus(ICQ_STATUS_ONLINE);
        bAutoOffline = bAutoNA = bAutoAway = false;
      }
      else if (bAutoNA)
      {
        changeStatus(ICQ_STATUS_AWAY);
        bAutoNA = bAutoOffline = false;
      }
      else
      {
        changeStatus(ICQ_STATUS_NA);
        bAutoOffline = false;
      }
    }
    else if (bAutoNA)
    {
      if (bAutoAway)
      {
        changeStatus(ICQ_STATUS_ONLINE);
        bAutoNA = bAutoAway = false;
      }
      else
      {
        changeStatus(ICQ_STATUS_AWAY);
        bAutoNA = false;
      }
    }
    else if (bAutoAway)
    {
      changeStatus(ICQ_STATUS_ONLINE);
      bAutoAway = false;
    }
  }

#endif // USE_SCRNSAVER
}


void CMainWindow::popupSystemMenu()
{
   mnuSystem->popup(mapToGlobal(QPoint(btnSystem->x(), btnSystem->y())));
}


//-----CMainWindow::loadIcons-------------------------------------------------
void CMainWindow::ApplyIcons(const char *_sIconSet, bool _bInitial)
// load the pixmaps
{
   char sFilename[MAX_FILENAME_LEN],
        sFilepath[MAX_FILENAME_LEN],
        sIconPath[MAX_FILENAME_LEN];

   if (m_szIconSet != NULL) free (m_szIconSet);
   m_szIconSet = strdup(_sIconSet);
   if (_sIconSet[0] == '/')
   {
     strcpy(sIconPath, _sIconSet);
     if (sIconPath[strlen(sIconPath) - 1] != '/')
       strcat(sIconPath, "/");
   }
   else
   {
     sprintf(sIconPath, "%s%sicons.%s/", SHARE_DIR, QTGUI_DIR, _sIconSet);
   }
   sprintf(sFilename, "%s%s.icons", sIconPath, _sIconSet);
   CIniFile fIconsConf;
   if (!fIconsConf.LoadFile(sFilename))
   {
     if (_bInitial)
       gLog.Warn("%sUnable to open icons file %s.\n", L_WARNxSTR, sFilename);
     else
       WarnUser(this, tr("Unable to open icons file\n%1.").arg(sFilename));
     return;
   }

   fIconsConf.SetSection("icons");
   fIconsConf.ReadStr("Online", sFilename, "");
   sprintf(sFilepath, "%s%s", sIconPath, sFilename);
   pmOnline.load(sFilepath);

   fIconsConf.ReadStr("FFC", sFilename, "");
   sprintf(sFilepath, "%s%s", sIconPath, sFilename);
   pmFFC.load(sFilepath);
   if (pmFFC.isNull()) pmFFC = pmOnline;

   fIconsConf.ReadStr("Offline", sFilename, "");
   sprintf(sFilepath, "%s%s", sIconPath, sFilename);
   pmOffline.load(sFilepath);

   fIconsConf.ReadStr("Away", sFilename, "");
   sprintf(sFilepath, "%s%s", sIconPath, sFilename);
   pmAway.load(sFilepath);

   fIconsConf.ReadStr("NA", sFilename, "");
   sprintf(sFilepath, "%s%s", sIconPath, sFilename);
   pmNa.load(sFilepath);

   fIconsConf.ReadStr("Occupied", sFilename, "");
   sprintf(sFilepath, "%s%s", sIconPath, sFilename);
   pmOccupied.load(sFilepath);

   fIconsConf.ReadStr("DND", sFilename, "");
   sprintf(sFilepath, "%s%s", sIconPath, sFilename);
   pmDnd.load(sFilepath);

   fIconsConf.ReadStr("Private", sFilename, "");
   sprintf(sFilepath, "%s%s", sIconPath, sFilename);
   pmPrivate.load(sFilepath);

   fIconsConf.ReadStr("Message", sFilename, "none");
   sprintf(sFilepath, "%s%s", sIconPath, sFilename);
   pmMessage.load(sFilepath);

   fIconsConf.ReadStr("Url", sFilename, "");
   sprintf(sFilepath, "%s%s", sIconPath, sFilename);
   pmUrl.load(sFilepath);

   fIconsConf.ReadStr("Chat", sFilename, "");
   sprintf(sFilepath, "%s%s", sIconPath, sFilename);
   pmChat.load(sFilepath);

   fIconsConf.ReadStr("File", sFilename, "");
   sprintf(sFilepath, "%s%s", sIconPath, sFilename);
   pmFile.load(sFilepath);

   fIconsConf.ReadStr("Contact", sFilename, "");
   sprintf(sFilepath, "%s%s", sIconPath, sFilename);
   pmContact.load(sFilepath);
   if(pmContact.isNull()) pmContact = pmMessage;

   fIconsConf.ReadStr("Authorize", sFilename, "");
   sprintf(sFilepath, "%s%s", sIconPath, sFilename);
   pmAuthorize.load(sFilepath);
   if(pmAuthorize.isNull()) pmAuthorize = pmMessage;

   if (!_bInitial)
   {
     mnuStatus->changeItem(pmOnline, tr("&Online"), ICQ_STATUS_ONLINE);
     mnuStatus->changeItem(pmAway, tr("&Away"), ICQ_STATUS_AWAY);
     mnuStatus->changeItem(pmNa, tr("&Not Available"), ICQ_STATUS_NA);
     mnuStatus->changeItem(pmOccupied, tr("O&ccupied"), ICQ_STATUS_OCCUPIED);
     mnuStatus->changeItem(pmDnd, tr("&Do Not Disturb"), ICQ_STATUS_DND);
     mnuStatus->changeItem(pmFFC, tr("Free for C&hat"), ICQ_STATUS_FREEFORCHAT);
     mnuStatus->changeItem(pmOffline, tr("O&ffline"), ICQ_STATUS_OFFLINE);
     mnuStatus->changeItem(pmPrivate, tr("&Invisible"), ICQ_STATUS_FxPRIVATE);
     mnuUser->changeItem(pmMessage, tr("&Send Message"), mnuUserSendMsg);
     mnuUser->changeItem(pmUrl, tr("Send &Url"), mnuUserSendUrl);
     mnuUser->changeItem(pmChat, tr("Send &Chat Request"), mnuUserSendChat);
     mnuUser->changeItem(pmFile, tr("Send &File Transfer"), mnuUserSendFile);
     mnuUser->changeItem(pmContact, tr("Send &Contact List"), mnuUserSendContact);
     mnuUser->changeItem(pmAuthorize, tr("Send &Authorization"), mnuUserAuthorize);
     CUserView::UpdateFloaties();
     updateUserWin();
     updateEvents();
   }
}


//-----CMainWindow::initMenu--------------------------------------------------
void CMainWindow::initMenu()
{
   mnuStatus = new QPopupMenu(NULL);
   mnuStatus->insertItem(pmOnline, tr("&Online"), ICQ_STATUS_ONLINE);
   mnuStatus->insertItem(pmAway, tr("&Away"), ICQ_STATUS_AWAY);
   mnuStatus->insertItem(pmNa, tr("&Not Available"), ICQ_STATUS_NA);
   mnuStatus->insertItem(pmOccupied, tr("O&ccupied"), ICQ_STATUS_OCCUPIED);
   mnuStatus->insertItem(pmDnd, tr("&Do Not Disturb"), ICQ_STATUS_DND);
   mnuStatus->insertItem(pmFFC, tr("Free for C&hat"), ICQ_STATUS_FREEFORCHAT);
   mnuStatus->insertItem(pmOffline, tr("O&ffline"), ICQ_STATUS_OFFLINE);
   mnuStatus->insertSeparator();
   mnuStatus->insertItem(pmPrivate, tr("&Invisible"), ICQ_STATUS_FxPRIVATE);
   mnuStatus->setAccel(ALT + Key_O, ICQ_STATUS_ONLINE);
   mnuStatus->setAccel(ALT + Key_A, ICQ_STATUS_AWAY);
   mnuStatus->setAccel(ALT + Key_N, ICQ_STATUS_NA);
   mnuStatus->setAccel(ALT + Key_C, ICQ_STATUS_OCCUPIED);
   mnuStatus->setAccel(ALT + Key_D, ICQ_STATUS_DND);
   mnuStatus->setAccel(ALT + Key_H, ICQ_STATUS_FREEFORCHAT);
   mnuStatus->setAccel(ALT + Key_F, ICQ_STATUS_OFFLINE);
   mnuStatus->setAccel(ALT + Key_I, ICQ_STATUS_FxPRIVATE);
   connect(mnuStatus, SIGNAL(activated(int)), this, SLOT(changeStatusManual(int)));

   mnuUserGroups = new QPopupMenu(NULL);
   connect(mnuUserGroups, SIGNAL(activated(int)), this, SLOT(setCurrentGroupMenu(int)));

   mnuDebug = new QPopupMenu(NULL);
   mnuDebug->insertItem(tr("Status Info"));
   mnuDebug->insertItem(tr("Unknown Packets"));
   mnuDebug->insertItem(tr("Errors"));
   mnuDebug->insertItem(tr("Warnings"));
   mnuDebug->insertItem(tr("Packets"));
   mnuDebug->insertSeparator();
   mnuDebug->insertItem(tr("Set All"));
   mnuDebug->insertItem(tr("Clear All"));
   mnuDebug->setCheckable(true);
   for (int i = 0; i < 5; i++)
     mnuDebug->setItemChecked(mnuDebug->idAt(i), DEBUG_LEVEL & (1 << i));
   connect(mnuDebug, SIGNAL(activated(int)), this, SLOT(changeDebug(int)));

   mnuOwnerAdm = new QPopupMenu(NULL);
   mnuOwnerAdm->insertItem(tr("&View System Messages"), OwnerMenuView);
   mnuOwnerAdm->insertSeparator();
   mnuOwnerAdm->insertItem(tr("&General Info"), OwnerMenuGeneral);
   mnuOwnerAdm->insertItem(tr("&More Info"), OwnerMenuMore);
   mnuOwnerAdm->insertItem(tr("&Work Info"), OwnerMenuWork);
   mnuOwnerAdm->insertItem(tr("&About"), OwnerMenuAbout);
   mnuOwnerAdm->insertItem(tr("&History"), OwnerMenuHistory);
   mnuOwnerAdm->insertSeparator();
   mnuOwnerAdm->insertItem(tr("&Security Options"), OwnerMenuSecurity);
   mnuOwnerAdm->insertItem(tr("Change &Password"), OwnerMenuPassword);
   mnuOwnerAdm->insertItem(tr("&Random Chat Group"), OwnerMenuRandomChat);
   mnuOwnerAdm->insertSeparator();
   mnuOwnerAdm->insertItem(tr("Debug Level"), mnuDebug);
   connect (mnuOwnerAdm, SIGNAL(activated(int)), this, SLOT(callOwnerFunction(int)));

   mnuUserAdm = new QPopupMenu(NULL);
   mnuUserAdm->insertItem(tr("&Add User"), this, SLOT(showAddUserDlg()));
   mnuUserAdm->insertItem(tr("S&earch for User"), this, SLOT(showSearchUserDlg()));
   mnuUserAdm->insertItem(tr("A&uthorize User"), this, SLOT(showAuthUserDlg()));
   mnuUserAdm->insertItem(tr("R&andom Chat"), this, SLOT(slot_randomchatsearch()));
   mnuUserAdm->insertSeparator();
   mnuUserAdm->insertItem(tr("&Popup All Messages"), this, SLOT(slot_popupall()));
   mnuUserAdm->insertItem(tr("Edit &Groups"), this, SLOT(showEditGrpDlg()));
   //mnuUserAdm->insertItem(tr("&Update Contact List"), this, SLOT(slot_updateContactList()));
   //mnuUserAdm->insertItem(tr("Update All Users"), this, SLOT(slot_updateAllUsers()));
   mnuUserAdm->insertItem(tr("&Redraw User Window"), this, SLOT(updateUserWin()));
   mnuUserAdm->insertItem(tr("&Save All Users"), this, SLOT(saveAllUsers()));
   mnuUserAdm->insertSeparator();
   mnuUserAdm->insertItem(tr("Reg&ister User"), this, SLOT(slot_register()));

   QPopupMenu *mnuHelp = new QPopupMenu(NULL);
   mnuHelp->insertItem(tr("&Hints"), this, SLOT(slot_hints()));
   mnuHelp->insertItem(tr("&About"), this, SLOT(aboutBox()));
   mnuHelp->insertItem(tr("&Statistics"), this, SLOT(slot_stats()));

   mnuSystem = new QPopupMenu(NULL);
   mnuSystem->setCheckable(true);
   mnuSystem->insertItem(tr("System Functions"), mnuOwnerAdm);
   mnuSystem->insertItem(tr("User Functions"), mnuUserAdm);
   mnuSystem->insertItem(tr("&Status"), mnuStatus);
   mnuSystem->insertItem(tr("&Group"), mnuUserGroups);
   mnuSystem->insertItem(tr("Set &Auto Response..."), this, SLOT(slot_AwayMsgDlg()));
   mnuSystem->insertSeparator();
   mnuSystem->insertItem(tr("&Network Window"), licqLogWindow, SLOT(show()));
   mnuSystem->insertItem(tr("&Mini Mode"), this, SLOT(miniMode()));
   mnuSystem->insertItem(tr("Show Offline &Users"), this, SLOT(ToggleShowOffline()));
   mnuSystem->insertItem(tr("&Options..."), this, SLOT(popupOptionsDlg()));
   mnuSystem->insertItem(tr("S&kin Browser..."), this, SLOT(showSkinBrowser()));
   mnuSystem->insertItem(tr("&Plugin Manager..."), this, SLOT(showPluginDlg()));
   mnuSystem->insertSeparator();
   mnuSystem->insertItem(tr("Next &Server"), this, SLOT(nextServer()));
   mnuSystem->insertSeparator();
   mnuSystem->insertItem(tr("Sa&ve Settings"), this, SLOT(saveOptions()));
   mnuSystem->insertItem(tr("&Help"), mnuHelp);
   mnuSystem->insertItem(tr("E&xit"), this, SLOT(slot_shutdown()));
   mnuSystem->setItemChecked(mnuSystem->idAt(MNUxITEM_SHOWxOFFLINE), m_bShowOffline);

   mnuGroup = new QPopupMenu(NULL);
   mnuGroup->setCheckable(true);
   connect(mnuGroup, SIGNAL(activated(int)), this, SLOT(UserGroupToggled(int)));
   connect(mnuGroup, SIGNAL(aboutToShow()), this, SLOT(FillUserGroup()));

   mnuUtilities = new QPopupMenu(NULL);
   for (unsigned short i = 0; i < gUtilityManager.NumUtilities(); i++)
   {
     mnuUtilities->insertItem(gUtilityManager.Utility(i)->Name());
   }
   connect(mnuUtilities, SIGNAL(activated(int)), this, SLOT(slot_utility(int)));

   mnuAwayModes = new QPopupMenu(NULL);
   mnuAwayModes->setCheckable(true);
   mnuAwayModes->insertItem(tr("Accept in Away"));
   mnuAwayModes->insertItem(tr("Accept in Not Available"));
   mnuAwayModes->insertItem(tr("Accept in Occupied"));
   mnuAwayModes->insertItem(tr("Accept in Do Not Disturb"));
   mnuAwayModes->insertSeparator();
   mnuAwayModes->insertItem(tr("Online to User"));
   mnuAwayModes->insertItem(tr("Away to User"));
   mnuAwayModes->insertItem(tr("Not Available to User"));
   mnuAwayModes->insertItem(tr("Occupied to User"));
   mnuAwayModes->insertItem(tr("Do Not Disturb to User"));
   mnuAwayModes->insertSeparator();
   mnuAwayModes->insertItem(tr("Custom Auto Response..."));
   connect(mnuAwayModes, SIGNAL(activated(int)), this, SLOT(slot_awaymodes(int)));

   mnuUser = new QPopupMenu(NULL);
   mnuUser->insertItem(tr("&View Event"), mnuUserView);
   mnuSend = new QPopupMenu(this);
   mnuSend->insertItem(pmMessage, tr("&Send Message"), mnuUserSendMsg);
   mnuSend->insertItem(pmUrl, tr("Send &Url"), mnuUserSendUrl);
   mnuSend->insertItem(pmChat, tr("Send &Chat Request"), mnuUserSendChat);
   mnuSend->insertItem(pmFile, tr("Send &File Transfer"), mnuUserSendFile);
   mnuSend->insertItem(pmContact, tr("Send Contact List"), mnuUserSendContact);
   mnuSend->insertItem(pmAuthorize, tr("Send &Authorization"), mnuUserAuthorize);
   connect (mnuSend, SIGNAL(activated(int)), this, SLOT(callUserFunction(int)));
   mnuUser->insertItem(tr("Send"), mnuSend);
   mnuUser->insertItem(tr("Check Auto Response"), mnuUserCheckResponse);
   mnuUser->insertItem(tr("&Away Modes"), mnuAwayModes);
   mnuUser->insertItem(tr("U&tilities"), mnuUtilities);
   mnuUser->insertSeparator();
   QPopupMenu *m = new QPopupMenu(this);
   m->insertItem(tr("&General Info"), mnuUserGeneral);
   m->insertItem(tr("&More Info"), mnuUserMore);
   m->insertItem(tr("&Work Info"), mnuUserWork);
   m->insertItem(tr("&About"), mnuUserAbout);
   connect (m, SIGNAL(activated(int)), this, SLOT(callUserFunction(int)));
   mnuUser->insertItem(tr("&Info"), m);
   mnuUser->insertItem(tr("View &History"), mnuUserHistory);
   mnuUser->insertItem(tr("Toggle &Floaty"), mnuUserFloaty);
   mnuUser->insertItem(tr("Edit User Group"), mnuGroup);
   mnuUser->insertSeparator();
   mnuUser->insertItem(tr("Remove From List"), mnuUserRemoveFromList);
   connect (mnuUser, SIGNAL(activated(int)), this, SLOT(callUserFunction(int)));
   connect (mnuUser, SIGNAL(aboutToShow()), this, SLOT(slot_usermenu()));
}


void CMainWindow::slot_usermenu()
{
  //ICQUser *u = gUserManager.FetchUser(userView->SelectedItemUin(), LOCK_R);
  ICQUser *u = gUserManager.FetchUser(m_nUserMenuUin, LOCK_R);

  if (u == NULL)
  {
    mnuUser->changeItem(mnuUserCheckResponse, tr("Check Auto Response"));
    mnuUser->setItemEnabled(mnuUserCheckResponse, false);
    return;
  }

  const char *szStatus = u->StatusStrShort();
  unsigned short status = u->Status();

  if (status == ICQ_STATUS_OFFLINE)
  {
    mnuUser->changeItem(mnuUserCheckResponse, tr("Check Auto Response"));
    mnuUser->setItemEnabled(mnuUserCheckResponse, false);
  }
  else if (status == ICQ_STATUS_ONLINE)
  {
    mnuUser->changeItem(mnuUserCheckResponse, tr("Check Auto Response"));
    mnuUser->setItemEnabled(mnuUserCheckResponse, true);
  }
  else
  {
    mnuUser->changeItem(mnuUserCheckResponse, QString(tr("Check %1 Response")).arg(szStatus));
    mnuUser->setItemEnabled(mnuUserCheckResponse, true);
  }

  // AcceptIn[Away] mode checked/unchecked stuff -- Andypoo (andypoo@ihug.com.au)
  mnuAwayModes->setItemChecked(mnuAwayModes->idAt(0), u->AcceptInAway());
  mnuAwayModes->setItemChecked(mnuAwayModes->idAt(1), u->AcceptInNA());
  mnuAwayModes->setItemChecked(mnuAwayModes->idAt(2), u->AcceptInOccupied());
  mnuAwayModes->setItemChecked(mnuAwayModes->idAt(3), u->AcceptInDND());
  mnuAwayModes->setItemChecked(mnuAwayModes->idAt(5), u->StatusToUser() == ICQ_STATUS_ONLINE);
  mnuAwayModes->setItemChecked(mnuAwayModes->idAt(6), u->StatusToUser() == ICQ_STATUS_AWAY);
  mnuAwayModes->setItemChecked(mnuAwayModes->idAt(7), u->StatusToUser() == ICQ_STATUS_NA);
  mnuAwayModes->setItemChecked(mnuAwayModes->idAt(8), u->StatusToUser() == ICQ_STATUS_OCCUPIED);
  mnuAwayModes->setItemChecked(mnuAwayModes->idAt(9), u->StatusToUser() == ICQ_STATUS_DND);
  mnuAwayModes->setItemChecked(mnuAwayModes->idAt(11), u->CustomAutoResponse()[0] != '\0');
  // Send modes
  mnuSend->setItemEnabled(mnuUserSendChat, !u->StatusOffline());
  mnuSend->setItemEnabled(mnuUserSendFile, !u->StatusOffline());

  gUserManager.DropUser(u);
}


void CMainWindow::slot_stats()
{
#ifdef SAVE_STATS
  QString s = tr("Daemon Statistics\n(Today/Total)\n");
  QDateTime t_total, t_today;
  t_today.setTime_t(licqDaemon->StartTime());
  t_total.setTime_t(licqDaemon->ResetTime());
  s += tr("Up since %1\n").arg(t_today.toString());
  s += tr("Last reset %1\n\n").arg(t_total.toString());
  DaemonStatsList::iterator iter;
  for (iter = licqDaemon->AllStats().begin(); iter != licqDaemon->AllStats().end(); iter++)
  {
    s += tr("%1: %2 / %3\n").arg(iter->Name()).arg(iter->Today()).arg(iter->Total());
  }
  if (!QueryUser(this, s, tr("&Ok"), tr("&Reset")))
  {
    licqDaemon->ResetStats();
  }
#else
  QString s = tr("Daemon Statistics\n\n");
  QDateTime t_today;
  t_today.setTime_t(licqDaemon->StartTime());
  s += tr("Up since %1\n\n").arg(t_today.toString());
  DaemonStatsList::iterator iter;
  for (iter = licqDaemon->AllStats().begin(); iter != licqDaemon->AllStats().end(); iter++)
  {
    s += tr("%1: %2\n")
       .arg(iter->Name())
       .arg(iter->Today());
  }
  InformUser(this, s);
#endif
}


void CMainWindow::showSearchUserDlg()
{
  SearchUserDlg *searchUserDlg = new SearchUserDlg(licqDaemon, licqSigMan);
  searchUserDlg->show();
}


void CMainWindow::showAddUserDlg()
{
  AddUserDlg *addUserDlg = new AddUserDlg(licqDaemon);
  addUserDlg->show();
}


void CMainWindow::showAuthUserDlg()
{
  (void) new AuthUserDlg(licqDaemon, 0, true);
}


void CMainWindow::showEditGrpDlg()
{
  EditGrpDlg *d = new EditGrpDlg;
  connect (d, SIGNAL(signal_updateGroups()), this, SLOT(updateGroups()));
  d->show();
}

void CMainWindow::showOptionsDlg(int tab)
{
  if (optionsDlg) {
    optionsDlg->raise();
//    optionsDlg->showTab(tab);
  }
  else {
    optionsDlg = new OptionsDlg(this, (OptionsDlg::tabs) tab);
    connect(optionsDlg, SIGNAL(signal_done()), this, SLOT(slot_doneOptions()));
  }
}

void CMainWindow::showSkinBrowser()
{
  SkinBrowserDlg *d = new SkinBrowserDlg(this);
  d->show();
}

void CMainWindow::showPluginDlg()
{
  (void) new PluginDlg(licqDaemon);
}

void CMainWindow::slot_randomchatsearch()
{
  (void) new CRandomChatDlg(this, licqDaemon, licqSigMan);
}


void CMainWindow::slot_popupall()
{
  // Do nothing if there are no events pending
  if (ICQUser::getNumUserEvents() == 0) return;

  // Do system messages first
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  unsigned short nNumMsg = o->NewMessages();
  gUserManager.DropOwner();
  if (nNumMsg > 0)
  {
    callOwnerFunction(OwnerMenuView);
  }

  UinList uins;
  FOR_EACH_USER_START(LOCK_R)
  {
    if (pUser->NewMessages() > 0)
    {
      uins.push_back(pUser->Uin());
    }
  }
  FOR_EACH_USER_END

  for (UinList::iterator iter = uins.begin(); iter != uins.end(); iter++)
  {
    callFunction(mnuUserView, *iter);
  }
}


void CMainWindow::slot_doneOptions()
{
  optionsDlg = NULL;
}

void CMainWindow::slot_doneregister()
{
  registerUserDlg = NULL;
}

void CMainWindow::slot_register()
{
  if (gUserManager.OwnerUin() != 0)
  {
    QString buf = tr("You are currently registered as\n"
                    "UIN: %1\n"
                    "Base Directory: %2\n"
                    "Rerun licq with the -b option to select a new\n"
                    "base directory and then register a new user.")
                    .arg(gUserManager.OwnerUin()).arg(BASE_DIR);
    InformUser(this, buf);
    return;
  }

  if (registerUserDlg != NULL)
    registerUserDlg->raise();
  else
  {
    registerUserDlg = new RegisterUserDlg(licqDaemon);
    connect(registerUserDlg, SIGNAL(signal_done()), this, SLOT(slot_doneregister()));
  }
}


void CMainWindow::slot_hints()
{
  QString hints = tr(
"Hints for Using the Licq Qt-GUI Plugin\n\n"
"o  Change your status by right clicking on the status label.\n"
"o  Change your auto response by double-clicking on the status label.\n"
"o  View system messages by double clicking on the message label.\n"
"o  Change groups by right clicking on the message label.\n"
"o  Use the following shortcuts from the contact list:\n"
"   Ctrl-M : Toggle mini-mode\n"
"   Ctrl-O : Toggle show offline users\n"
"   Ctrl-X : Exit\n"
"   Ctrl-H : Hide\n"
"   Ctrl-I : View the next message\n"
"   Ctrl-V : View message\n"
"   Ctrl-S : Send message\n"
"   Ctrl-U : Send Url\n"
"   Ctrl-C : Send chat request\n"
"   Ctrl-F : Send File\n"
"   Ctrl-A : Check Auto response\n"
"   Ctrl-P : Popup all messages\n"
"   Ctrl-L : Redraw user window\n"
"   Delete : Delete user from current group\n"
"   Ctrl-Delete : Delete user from contact list\n"
"o  Hold control while clicking on close in the function window to remove\n"
"   the user from your contact list.\n"
"o  Hit Ctrl-Enter from most text entry fields to select \"Ok\" or \"Accept\".\n"
"   For example in the send tab of the user function window.\n");

  InformUser(NULL, hints);
}

#include "mainwin.moc"
