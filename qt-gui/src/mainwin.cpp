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

#include "mainwin.h"
#include "icq-defines.h"
#include "gui-defines.h"
#include "log.h"
#include "translate.h"
#include "utility.h"
#include "adduserdlg.h"
#include "authuserdlg.h"
#include "editgrp.h"
#include "searchuserdlg.h"
#include "icqfunctions.h"
#include "utilitydlg.h"
#include "registeruser.h"
#include "skinbrowser.h"
#include "icqd.h"
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


static QPixmap *MakeTransparentBg(QPixmap *_pm, const QRect &r)
{
  QPixmap *pm = new QPixmap(r.size());
  QPainter p(pm);
  p.drawPixmap(0, 0, *_pm, r.x(), r.y(), r.width(), r.height());
  return(pm);
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


//-----CMainWindow::constructor-------------------------------------------------
CMainWindow::CMainWindow(CICQDaemon *theDaemon, CSignalManager *theSigMan,
                         CQtLogWindow *theLogWindow,
                         const char *skinName, const char *iconsName,
                         QWidget *parent, const char *name) : QWidget(parent, name)
{
  licqDaemon = theDaemon;
  licqSigMan = theSigMan;
  licqLogWindow = theLogWindow;

  // Overwrite Qt's event handler
  old_handler = XSetErrorHandler(licq_xerrhandler);

  // set up appicon and docking, code supplied by Mark Deneed
  WId win = winId();     // get the window
  Display *dsp = x11Display();  // get the display
  XWMHints *hints;  // hints
  XClassHint classhint;  // class hints
  classhint.res_name = "licq";  // res_name
  classhint.res_class = "Licq";  // res_class
  XSetClassHint(dsp, win, &classhint); // set the class hints
  hints = XGetWMHints(dsp, win);  // init hints
  hints->window_group = win;  // set set the window hint
  hints->flags = WindowGroupHint;  // set the window group hint
  XSetWMHints(dsp, win, hints);  // set the window hints for WM to use.
  XFree( hints );

  // read in info from file
  char filename[MAX_FILENAME_LEN];

#ifdef USE_KDE
  gLog.Info("%sKDE GUI configuration.\n", L_INITxSTR);
#else
  gLog.Info("%sQt GUI configuration.\n", L_INITxSTR);
#endif
  sprintf(filename, "%s/licq_qt-gui.conf", BASE_DIR);
  CIniFile licqConf;
  if (! licqConf.LoadFile(filename) )
  {
    FILE *f = fopen(filename, "w");
    fprintf(f, "%s", QTGUI_CONF);
    fclose(f);
    licqConf.LoadFile(filename);
  }

  licqConf.SetSection("appearance");
  char szFont[256];
  QFont f;
  licqConf.ReadStr("Font", szFont, "default");
#ifdef USE_KDE
  defaultFont = KGlobal::generalFont();
#else
  defaultFont = qApp->font();
#endif
  if (strcmp(szFont, "default") != 0)
  {
    f.setRawName(szFont);
    qApp->setFont(f, true);
  }
  licqConf.ReadStr("EditFont", szFont, "default");
  if(!strcmp(szFont, "default"))
    f = qApp->font();
  else
    f.setRawName(szFont);
  delete MLEditWrap::editFont;
  MLEditWrap::editFont = new QFont(f);

  licqConf.ReadBool("GridLines", gridLines, false);
  licqConf.ReadBool("FontStyles", m_bFontStyles, true);
  licqConf.ReadBool("ShowHeader", showHeader, true);
  licqConf.ReadBool("ShowOfflineUsers", m_bShowOffline, true);
  licqConf.ReadBool("ShowDividers", m_bShowDividers, true);
  licqConf.ReadBool("SortByStatus", m_bSortByStatus, true);
  licqConf.ReadBool("ShowGroupIfNoMsg", m_bShowGroupIfNoMsg, true);
  bool bFrameTransparent;
  licqConf.ReadBool("Transparent", bFrameTransparent, false);
  unsigned short nFrameStyle;
  licqConf.ReadNum("FrameStyle", nFrameStyle, 51);
  char szDockTheme[MAX_FILENAME_LEN];
  bool bDockIcon48;
  unsigned short nDockMode;
  licqConf.ReadNum("UseDock", nDockMode, (unsigned short)DockNone);
  m_nDockMode = (DockMode)nDockMode;
  licqConf.ReadBool("Dock64x48", bDockIcon48, false);
  licqConf.ReadStr("DockTheme", szDockTheme, "");
  bool bHidden;
  licqConf.ReadBool("Hidden", bHidden, false);
  licqConf.ReadBool("AutoRaise", m_bAutoRaise, true);

  licqConf.SetSection("startup");
  licqConf.ReadNum("Logon", m_nAutoLogon, 0);
  if (m_nAutoLogon > 16) m_nAutoLogon = 0;
  licqConf.ReadNum("AutoAway", autoAwayTime, 0);
  licqConf.ReadNum("AutoNA", autoNATime, 0);

  licqConf.SetSection("functions");
  licqConf.ReadBool("AutoClose", autoClose, true);
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
#ifdef USE_KDE
  setIconText("KLicq");
#else
  setIconText("Licq");
#endif

  // User List
   char colKey[16], colTitle[32], colFormat[32];
   unsigned short colWidth, colAlign, numColumns;
   licqConf.ReadNum("NumColumns", numColumns, 1);
   for (unsigned short i = 1; i <= numColumns; i++)
   {
      sprintf(colKey, "Column%d.Title", i);
      licqConf.ReadStr(colKey, colTitle, "Alias");
      sprintf(colKey, "Column%d.Format", i);
      licqConf.ReadStr(colKey, colFormat, "%a");
      sprintf(colKey, "Column%d.Width", i);
      licqConf.ReadNum(colKey, colWidth, 100);
      sprintf(colKey, "Column%d.Align", i);
      licqConf.ReadNum(colKey, colAlign, 0);
      colInfo.push_back(new CColumnInfo(QString::fromLocal8Bit(colTitle), colFormat, colWidth, colAlign));
   }
   CreateUserView();

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
   connect (licqSigMan, SIGNAL(signal_updatedList(unsigned long, unsigned long)),
            this, SLOT(slot_updatedList(unsigned long, unsigned long)));
   connect (licqSigMan, SIGNAL(signal_updatedUser(unsigned long, unsigned long)),
            this, SLOT(slot_updatedUser(unsigned long, unsigned long)));
   connect (licqSigMan, SIGNAL(signal_updatedStatus()), this, SLOT(updateStatus()));
   connect (licqSigMan, SIGNAL(signal_doneOwnerFcn(ICQEvent *)),
            this, SLOT(slot_doneOwnerFcn(ICQEvent *)));
   connect (licqSigMan, SIGNAL(signal_logon()),
            this, SLOT(slot_logon()));
   connect (this, SIGNAL(destroyed()), this, SLOT(slot_shutdown()));

   inMiniMode = false;
   updateStatus();
   updateEvents();
   updateGroups();
   manualAway = 0;

   setGeometry(xPos, yPos, wVal, hVal);
   if (!bHidden) show();

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

  /*QTimer *t = new QTimer(this);
  connect(t, SIGNAL(timeout()), SLOT(ti()));
  t->start(1000);*/
}

void CMainWindow::ti()
{
  //printf("%d %d\n", qApp->focusWidget(), qApp->activeWindow());
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
  if (skin->lblMsg.transparent)
  {
    QPixmap *p = MakeTransparentBg(pmBorder, skin->borderToRect(&skin->lblMsg, pmBorder));
    lblMsg->setBackgroundPixmap(*p);
    delete p;
  }
  else if (skin->lblMsg.pixmap != NULL)
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
  if (skin->lblStatus.transparent)
  {
    QPixmap *p = MakeTransparentBg(pmBorder, skin->borderToRect(&skin->lblStatus, pmBorder));
    lblStatus->setBackgroundPixmap(*p);
    delete p;
  }
  else if (skin->lblStatus.pixmap != NULL)
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
  userView = new CUserView(mnuUser, mnuGroup, mnuAwayModes, colInfo, showHeader,
                           gridLines,
                           m_bFontStyles, skin->frame.transparent,
                           m_bShowDividers, m_bSortByStatus, this);
  userView->setFrameStyle(skin->frame.frameStyle);
  userView->setPixmaps(&pmOnline, &pmOffline, &pmAway, &pmNa, &pmOccupied, &pmDnd,
                       &pmPrivate, &pmFFC, &pmMessage, &pmUrl, &pmChat, &pmFile);
  userView->setColors(skin->colors.online, skin->colors.away, skin->colors.offline,
                      skin->colors.newuser, skin->colors.background, skin->colors.gridlines);
  connect (userView, SIGNAL(doubleClicked(QListViewItem *)),
           this, SLOT(callDefaultFunction()));
}

//-----CMainWindow::destructor--------------------------------------------------
CMainWindow::~CMainWindow()
{
#ifdef USE_DOCK
  if (licqIcon != NULL) delete licqIcon;
#endif

  // save window position and size
  char buf[MAX_FILENAME_LEN];
  sprintf(buf, "%s/licq_qt-gui.conf", BASE_DIR);
  CIniFile licqConf(INI_FxALLOWxCREATE | INI_FxWARN);
  // need some more error checking here...
  licqConf.LoadFile(buf);
  licqConf.SetSection("appearance");
  licqConf.WriteBool("Hidden", !isVisible());
  licqConf.SetSection("geometry");
	unsigned short n;
	n = x() < 0 ? 0 : x();
  licqConf.WriteNum("x", n);
	n = y() < 0 ? 0 : y();
  licqConf.WriteNum("y", n);
  n = height() < 0 ? 0 : (inMiniMode ? m_nRealHeight : height());
  licqConf.WriteNum("h", n);
  n = width() < 0 ? 0 : width();
  licqConf.WriteNum("w", n);
  licqConf.FlushFile();
  licqConf.CloseFile();
}


//-----CMainWindow::resizeEvent------------------------------------------------
void CMainWindow::resizeEvent (QResizeEvent *)
{
  userView->setGeometry(skin->frame.border.left, skin->frame.border.top,
                        width() - skin->frameWidth(), height() - skin->frameHeight());
  userView->maxLastColumn();

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
  if (licqIcon == NULL)
    QWidget::closeEvent(e);
  else
  {
    e->ignore();
    hide();
  }
}


// Receive key press events for the main widget
// Ctrl-M : toggle mini mode
// Ctrl-H : hide window
// Ctrl-I : next message
// Ctrl-X : exit
void CMainWindow::keyPressEvent(QKeyEvent *e)
{
  if (e->key() == Qt::Key_M && e->state() & ControlButton)
  {
    miniMode();
  }
  else if (e->key() == Qt::Key_H && e->state() & ControlButton)
  {
    hide();
  }
  else if (e->key() == Qt::Key_I && e->state() & ControlButton)
  {
    callMsgFunction();
  }
  else if (e->key() == Qt::Key_X && e->state() & ControlButton)
  {
    slot_shutdown();
  }
  else
  {
    e->ignore();
    QWidget::keyPressEvent(e);
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

void CMainWindow::slot_updatedUser(unsigned long _nSubSignal, unsigned long _nUin)
{
  switch(_nSubSignal)
  {
    case USER_EVENTS:
    {
      updateEvents();
      if (m_bAutoPopup)
      {
        ICQUser *u = gUserManager.FetchUser(_nUin, LOCK_R);
        if (u != NULL && u->NewMessages() > 0)
        {
          gUserManager.DropUser(u);
          ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
          unsigned short s = o->Status();
          gUserManager.DropOwner();
          if (s == ICQ_STATUS_ONLINE || s == ICQ_STATUS_FREEFORCHAT)
            callFunction(mnuUserView, _nUin);
        }
        else
        {
          if (u != NULL) gUserManager.DropUser(u);
        }
      }
      // Come to the top
      if (m_bAutoRaise) raise();
      // Fall through
    }
    case USER_BASIC:
    case USER_GENERAL:
    case USER_EXT:
    case USER_STATUS:
    {
      if (_nUin == gUserManager.OwnerUin())
      {
        if (_nSubSignal == USER_STATUS || _nSubSignal == USER_EXT) break;
        ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
        m_szCaption = tr("Licq (%1)").arg(QString::fromLocal8Bit(o->GetAlias()));
        gUserManager.DropOwner();
        if (caption()[0] == '*')
          setCaption(QString("* ") + m_szCaption);
        else
          setCaption(m_szCaption);
        break;
      }
      ICQUser *u = gUserManager.FetchUser(_nUin, LOCK_R);
      if (u == NULL)
      {
        gLog.Warn("%sCMainWindow::slot_updatedUser(): Invalid uin received: %ld\n",
                   L_ERRORxSTR, _nUin);
        break;
      }
      if (u->GetInGroup(m_nGroupType, m_nCurrentGroup))
      {
        CUserViewItem *i = (CUserViewItem *)userView->firstChild();
        while (i && i->ItemUin() != _nUin)
          i = (CUserViewItem *)i->nextSibling();
        if (i != NULL)
        {
          delete i;
          if (m_bShowOffline || !u->StatusOffline())
            (void) new CUserViewItem(u, userView);
          userView->triggerUpdate();
        }
        else
        {
          if ( (m_bShowOffline || !u->StatusOffline()) &&
               (!u->IgnoreList() || (m_nGroupType == GROUPS_SYSTEM && m_nCurrentGroup == GROUP_IGNORE_LIST)) )
            (void) new CUserViewItem(u, userView);
        }
      }
      gUserManager.DropUser(u);
      break;
    }
  }
}


// ---------------------------------------------------------------------------

void CMainWindow::slot_updatedList(unsigned long _nSubSignal, unsigned long _nUin)
{
  switch(_nSubSignal)
  {
    case LIST_ALL:
    {
      updateUserWin();
      break;
    }
    case LIST_ADD:
    {
      ICQUser *u = gUserManager.FetchUser(_nUin, LOCK_R);
      if (u == NULL)
      {
        gLog.Warn("%sCMainWindow::slot_updatedList(): Invalid uin received: %ld\n",
                   L_ERRORxSTR, _nUin);
        break;
      }
      if (u->GetInGroup(m_nGroupType, m_nCurrentGroup) &&
          (m_bShowOffline || !u->StatusOffline()) )
        (void) new CUserViewItem(u, userView);
      gUserManager.DropUser(u);
      break;
    }

    case LIST_REMOVE:
    {
      CUserViewItem *i = (CUserViewItem *)userView->firstChild();
      while (i != NULL && i->ItemUin() != _nUin) i = (CUserViewItem *)i->nextSibling();
      if (i != NULL) delete i;
      updateEvents();
      // If their box is open, kill it
      UserDataListIter it;
      for (it = licqUserData.begin(); it != licqUserData.end(); it++)
      {
        if ((*it)->Uin() == _nUin)
        {
          delete *it;
          licqUserData.erase(it);
          break;
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
  FOR_EACH_USER_START(LOCK_R)
  {
    // Only show users on the current group and not on the ignore list
    if (!pUser->GetInGroup(m_nGroupType, m_nCurrentGroup) ||
        (pUser->IgnoreList() && m_nGroupType != GROUPS_SYSTEM && m_nCurrentGroup != GROUP_IGNORE_LIST) )
      FOR_EACH_USER_CONTINUE

    // Ignore offline users if necessary
    if (!m_bShowOffline && pUser->StatusOffline())
      FOR_EACH_USER_CONTINUE;

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
  if (nNumOwnerEvents > 0)
  {
    lblMsg->setText(tr("SysMsg"));
    szCaption = "* " + m_szCaption;
  }
  else if (nNumUserEvents > 0)
  {
    lblMsg->setText(tr("%1 msg%2")
                    .arg(nNumUserEvents)
                    .arg(nNumUserEvents == 1 ? tr(" ") : tr("s")));
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
    mnuGroup->insertItem(QString::fromLocal8Bit((*g)[i]));
  }
  gUserManager.UnlockGroupList();
  mnuUserGroups->insertSeparator();

  cmbUserGroups->insertItem(tr("Online Notify"));
  mnuUserGroups->insertItem(tr("Online Notify"));
  cmbUserGroups->insertItem(tr("Visible List"));
  mnuUserGroups->insertItem(tr("Visible List"));
  cmbUserGroups->insertItem(tr("Invisible List"));
  mnuUserGroups->insertItem(tr("Invisible List"));
  cmbUserGroups->insertItem(tr("Ignore List"));
  mnuUserGroups->insertItem(tr("Ignore List"));

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

void CMainWindow::callDefaultFunction()
{
  unsigned long nUin = userView->SelectedItemUin();
  if (nUin == 0) return;
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
  // set default function to read or send depending on whether or not
  // there are new messages
  int fcn = (u->NewMessages() == 0 ? mnuUserSendMsg : mnuUserView);
  gUserManager.DropUser(u);

  callFunction(fcn, nUin);
}

void CMainWindow::callOwnerFunction(int index)
{
  if (index == OwnerMenuView  || index == OwnerMenuGeneral ||
      index == OwnerMenuMore  || index == OwnerMenuWork ||
      index == OwnerMenuAbout || index == OwnerMenuHistory)
    callFunction(index, gUserManager.OwnerUin());

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
  if(userView->SelectedItemUin() == 0)
    return;

  switch(index)
  {
    case mnuUserAuthorize:
      licqDaemon->icqAuthorize(userView->SelectedItemUin());
      break;
    case mnuUserCheckResponse:
      {
        (void) new ShowAwayMsgDlg(licqDaemon, licqSigMan, userView->SelectedItemUin());
      }
      break;
    case mnuUserOnlineNotify:
    {
      ICQUser *u = gUserManager.FetchUser(userView->SelectedItemUin(), LOCK_W);
      if (!u) return;
      u->SetOnlineNotify(!u->OnlineNotify());
      gUserManager.DropUser(u);
      if (m_bFontStyles) updateUserWin();
    }
    break;
    case mnuUserInvisibleList:
    {
      ICQUser *u = gUserManager.FetchUser(userView->SelectedItemUin(), LOCK_W);
      if (!u) return;
      u->SetInvisibleList(!u->InvisibleList());
      gUserManager.DropUser(u);
      if (m_bFontStyles) updateUserWin();
      licqDaemon->icqSendInvisibleList(true);
    }
    break;
    case mnuUserVisibleList:
    {
      ICQUser *u = gUserManager.FetchUser(userView->SelectedItemUin(), LOCK_W);
      if (!u) return;
      u->SetVisibleList(!u->VisibleList());
      gUserManager.DropUser(u);
      if (m_bFontStyles)
        updateUserWin();
      licqDaemon->icqSendVisibleList(true);
    }
    break;
    case mnuUserIgnoreList:
    {
      ICQUser *u = gUserManager.FetchUser(userView->SelectedItemUin(), LOCK_W);
      if (!u) return;
      u->SetIgnoreList(!u->IgnoreList());
      gUserManager.DropUser(u);
      updateUserWin();
    }
    break;
    default:
      callFunction(index, userView->SelectedItemUin());
  }

}


//-----CMainWindow::callICQFunction-------------------------------------------
ICQFunctions *CMainWindow::callFunction(int fcn, unsigned long nUin)
{
  ICQFunctions *f = NULL;

  UserDataListIter it;
  for (it = licqUserData.begin(); it != licqUserData.end(); it++)
  {
    if ((*it)->Uin() == nUin)
    {
      f = *it;
      break;
    }
  }

  if (f == NULL)
  {
     f = new ICQFunctions(licqDaemon, licqSigMan, this, nUin, autoClose);
     connect (f, SIGNAL(signal_updatedUser(unsigned long, unsigned long)), SLOT(slot_updatedUser(unsigned long, unsigned long)));
     connect (f, SIGNAL(signal_finished(unsigned long)), SLOT(slot_userfinished(unsigned long)));
     f->setupTabs(fcn);
     licqUserData.push_back(f);
  }
  else
  {
#ifdef USE_KDE
    KWin::setActiveWindow(f->winId());
#endif
    f->show();
    f->raise();
  }
  return f;
}

//-----CMainWindow::slot_userfinished------------------------------------------------
void CMainWindow::slot_userfinished(unsigned long nUin)
{
  UserDataListIter it;
  for (it = licqUserData.begin(); it != licqUserData.end(); it++)
  {
    if ((*it)->Uin() == nUin)
    {
      licqUserData.erase(it);
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
  switch (e->m_nCommand)
  {
    case ICQ_CMDxSND_LOGON:
      if (e->m_eResult != EVENT_SUCCESS)
        WarnUser(this, tr("Logon failed.\nSee network window for details."));
      break;
    case ICQ_CMDxSND_REGISTERxUSER:
      delete registerUserDlg;
      registerUserDlg = NULL;
      if (e->m_eResult == EVENT_SUCCESS)
      {
        char buf[256];
        sprintf(buf, tr("Successfully registered, your user identification\n"
                       "number (UIN) is %ld.\n"
                       "Now set your personal information."),
                     gUserManager.OwnerUin());
        InformUser(this, QString::fromLocal8Bit(buf));
        callFunction(mnuUserGeneral, gUserManager.OwnerUin());
      }
      else
      {
        InformUser(this, tr("Registration failed.  See network window for details."));
      }
      break;
    case ICQ_CMDxSND_AUTHORIZE:
       if (e->m_eResult != EVENT_ACKED)
         WarnUser(this, tr("Error sending autorization."));
       else
         InformUser(this, tr("Authorization granted."));
       break;
    default:
       break;
  }
}



//-----CMainWindow::removeUser-------------------------------------------------
void CMainWindow::removeUserFromList()
{
  ICQUser *u = gUserManager.FetchUser(userView->SelectedItemUin(), LOCK_R);
  if (u == NULL) return;
  unsigned long nUin = u->Uin();
  QString warning(tr("Are you sure you want to remove\n%1 (%2)\nfrom your contact list?")
                     .arg(QString::fromLocal8Bit(u->GetAlias()))
                     .arg(nUin) );
  gUserManager.DropUser(u);
  if (QueryUser(this, warning, tr("Ok"), tr("Cancel")))
    licqDaemon->RemoveUserFromList(nUin);
}

void CMainWindow::removeUserFromGroup()
{
  if (m_nGroupType == GROUPS_USER)
  {
    if (m_nCurrentGroup == 0)
      removeUserFromList();
    else
    {
      ICQUser *u = gUserManager.FetchUser(userView->SelectedItemUin(), LOCK_R);
      if (u == NULL) return;
      unsigned long nUin = u->Uin();
      GroupList *g = gUserManager.LockGroupList(LOCK_R);
      QString warning(tr("Are you sure you want to remove\n%1 (%2)\nfrom the '%3' group?")
                         .arg(QString::fromLocal8Bit(u->GetAlias()))
                         .arg(nUin).arg(QString::fromLocal8Bit( (*g)[m_nCurrentGroup - 1] )) );
      gUserManager.UnlockGroupList();
      gUserManager.DropUser(u);
      if (QueryUser(this, warning, tr("Ok"), tr("Cancel")))
      {
         gUserManager.RemoveUserFromGroup(nUin, m_nCurrentGroup);
         updateUserWin();
      }
    }
  }
  else if (m_nGroupType == GROUPS_SYSTEM)
  {
    if (m_nCurrentGroup == 0) return;
    ICQUser *u = gUserManager.FetchUser(userView->SelectedItemUin(), LOCK_W);
    if (u == NULL) return;
    u->RemoveFromGroup(GROUPS_SYSTEM, m_nCurrentGroup);
    gUserManager.DropUser(u);
    updateUserWin();
  }
}


//-----CMainWindow::saveAllUsers-----------------------------------------------
void CMainWindow::saveAllUsers()
{
   gUserManager.SaveAllUsers();
}

//-----CMainWindow::addUserToGroup---------------------------------------------
void CMainWindow::addUserToGroup(int _nId)
{
   gUserManager.AddUserToGroup(userView->SelectedItemUin(),
                               mnuGroup->indexOf(_nId) + 1);
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

  licqConf.SetSection("functions");
  licqConf.WriteBool("AutoClose", autoClose);
  licqConf.WriteBool("AutoPopup", m_bAutoPopup);

  licqConf.SetSection("appearance");
  licqConf.WriteStr("Skin", skin->szSkinName);
  licqConf.WriteStr("Icons", m_szIconSet);
  licqConf.WriteStr("Font", qApp->font() == defaultFont ?
                    QString("default") : qApp->font().rawName());
  licqConf.WriteStr("EditFont",
                    (MLEditWrap::editFont == NULL ||
                     *MLEditWrap::editFont == defaultFont) ?
                     QString("default") : MLEditWrap::editFont->rawName());
  licqConf.WriteBool("GridLines", gridLines);
  licqConf.WriteBool("FontStyles", m_bFontStyles);
  licqConf.WriteBool("ShowHeader", showHeader);
  licqConf.WriteBool("ShowDividers", m_bShowDividers);
  licqConf.WriteBool("SortByStatus", m_bSortByStatus);
  licqConf.WriteBool("ShowGroupIfNoMsg", m_bShowGroupIfNoMsg);
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
      licqConf.WriteStr("DockTheme", (const char *)((IconManager_Themed *)licqIcon)->Theme());
      break;
    case DockNone:
      break;
  }
#endif

  // save the column info
  licqConf.WriteNum("NumColumns", (unsigned short)colInfo.size());
  char colKey[32];
  for (unsigned short i = 1; i <= colInfo.size(); i++)
  {
     sprintf(colKey, "Column%d.Title", i);
     licqConf.WriteStr(colKey, colInfo[i - 1]->m_sTitle.local8Bit());
     sprintf(colKey, "Column%d.Format", i);
     licqConf.WriteStr(colKey, colInfo[i - 1]->m_szFormat);
     sprintf(colKey, "Column%d.Width", i);
     licqConf.WriteNum(colKey, colInfo[i - 1]->m_nWidth);
     sprintf(colKey, "Column%d.Align", i);
     licqConf.WriteNum(colKey, colInfo[i - 1]->m_nAlign);
  }

  licqConf.FlushFile();
}


//-----CMainWindow::aboutBox--------------------------------------------------
void CMainWindow::aboutBox()
{
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  QString about(tr("Licq version %1.\n"
                  "Qt GUI plugin version %2.\n"
                  "%6\n"
                  "Author: Graham Roff\n"
                  "Contributions by Dirk Mueller\n"
                  "http://www.licq.org\n\n"
                  "%3 (%4)\n"
                  "%5 contacts.").arg(licqDaemon->Version())
                   .arg(VERSION).arg(QString::fromLocal8Bit(o->GetAlias()))
                   .arg(o->Uin()).arg(gUserManager.NumUsers())
#ifdef USE_KDE
                   .arg(tr("(with KDE support)\n")));
#else
                   .arg("\n"));
#endif
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
  unsigned long nUin = userView->SelectedItemUin();
  if (nUin != 0) (void) new CUtilityDlg(u, nUin, licqDaemon);
}


//-----CMainWindow::slot_awaymodes--------------------------------------------
void CMainWindow::slot_awaymodes(int _nId)
{
  int nAwayModes = mnuAwayModes->indexOf(_nId);
  ICQUser *u = gUserManager.FetchUser(userView->SelectedItemUin(), LOCK_W);
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
  }
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

   if (inMiniMode)
   {
      userView->show();
      setMaximumHeight(2048);
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
   inMiniMode = !inMiniMode;
   mnuSystem->setItemChecked(mnuSystem->idAt(MNUxITEM_MINIxMODE), inMiniMode);
}


//-----CMainWindow::autoAway--------------------------------------------------
void CMainWindow::autoAway()
{
#ifdef USE_SCRNSAVER
  static XScreenSaverInfo *mit_info = NULL;
  static bool bAutoAway = false;
  static bool bAutoNA = false;

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
  if ( (bAutoNA && status != ICQ_STATUS_NA) ||
       (bAutoAway && status != ICQ_STATUS_AWAY && !bAutoNA) )
  {
    bAutoNA = false;
    bAutoAway = false;
    return;
  }

  if ( (autoNATime > 0) &&
       (unsigned long)idleTime > (unsigned long)(autoNATime * 60000))
  {
    if (status == ICQ_STATUS_ONLINE || status == ICQ_STATUS_AWAY)
    {
      changeStatus(ICQ_STATUS_NA);
      bAutoNA = true;
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
    if (bAutoNA && bAutoAway)
    {
      changeStatus(ICQ_STATUS_ONLINE);
      bAutoNA = false;
      bAutoAway = false;
    }
    else if (bAutoNA)
    {
      changeStatus(ICQ_STATUS_AWAY);
      bAutoNA = false;
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

   fIconsConf.ReadStr("Authorize", sFilename, "");
   sprintf(sFilepath, "%s%s", sIconPath, sFilename);
   pmAuthorize.load(sFilepath);

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
     mnuUser->changeItem(pmAuthorize, tr("Send &Authorization"), mnuUserAuthorize);
     userView->setPixmaps(&pmOnline, &pmOffline, &pmAway, &pmNa, &pmOccupied, &pmDnd,
                          &pmPrivate, &pmFFC, &pmMessage, &pmUrl, &pmChat, &pmFile);
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
   mnuUserAdm->insertItem(tr("Edit Groups"), this, SLOT(showEditGrpDlg()));
   //mnuUserAdm->insertItem(tr("&Update Contact List"), this, SLOT(slot_updateContactList()));
   //mnuUserAdm->insertItem(tr("Update All Users"), this, SLOT(slot_updateAllUsers()));
   mnuUserAdm->insertItem(tr("&Redraw User Window"), this, SLOT(updateUserWin()));
   mnuUserAdm->insertItem(tr("&Save All Users"), this, SLOT(saveAllUsers()));
   mnuUserAdm->insertSeparator();
   mnuUserAdm->insertItem(tr("Register User"), this, SLOT(slot_register()));

   mnuSystem = new QPopupMenu(NULL);
   mnuSystem->setCheckable(true);
   mnuSystem->insertItem(tr("System Functions"), mnuOwnerAdm);
   mnuSystem->insertItem(tr("User Functions"), mnuUserAdm);
   mnuSystem->insertItem(tr("&Status"), mnuStatus);
   mnuSystem->insertItem(tr("&Group"), mnuUserGroups);
   mnuSystem->insertItem(tr("Set &Auto Response..."), this, SLOT(slot_AwayMsgDlg()));
   mnuSystem->insertSeparator();
   mnuSystem->insertItem(tr("Show &Network Window"), licqLogWindow, SLOT(show()));
   mnuSystem->insertItem(tr("Use &Mini Mode"), this, SLOT(miniMode()));
   mnuSystem->insertItem(tr("Show Offline &Users"), this, SLOT(ToggleShowOffline()));
   mnuSystem->insertItem(tr("&Options..."), this, SLOT(popupOptionsDlg()));
   mnuSystem->insertItem(tr("S&kin Browser..."), this, SLOT(showSkinBrowser()));
   mnuSystem->insertItem(tr("&Plugin Manager..."), this, SLOT(showPluginDlg()));
   mnuSystem->insertSeparator();
   mnuSystem->insertItem(tr("Next &Server"), this, SLOT(nextServer()));
   mnuSystem->insertSeparator();
   mnuSystem->insertItem(tr("Sa&ve Options"), this, SLOT(saveOptions()));
   mnuSystem->insertItem(tr("&About"), this, SLOT(aboutBox()));
   mnuSystem->insertItem(tr("E&xit"), this, SLOT(slot_shutdown()));
   mnuSystem->setItemChecked(mnuSystem->idAt(MNUxITEM_SHOWxOFFLINE), m_bShowOffline);

   mnuRemove = new QPopupMenu(NULL);
   mnuRemove->insertItem(tr("From Current Group"), this, SLOT(removeUserFromGroup()));
   mnuRemove->insertItem(tr("From Contact List"), this, SLOT(removeUserFromList()));

   mnuGroup = new QPopupMenu(NULL);
   connect(mnuGroup, SIGNAL(activated(int)), this, SLOT(addUserToGroup(int)));

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
//   mnuAwayModes->insertSeparator();
//   mnuAwayModes->insertItem(tr("Custom Away Message..."));
   connect(mnuAwayModes, SIGNAL(activated(int)), this, SLOT(slot_awaymodes(int)));

   mnuUser = new QPopupMenu(NULL);
   mnuUser->setCheckable(true);
   mnuUser->insertItem(tr("&View Event"), mnuUserView);
   QPopupMenu *m = new QPopupMenu(this);
   m->insertItem(pmMessage, tr("&Send Message"), mnuUserSendMsg);
   m->insertItem(pmUrl, tr("Send &Url"), mnuUserSendUrl);
   m->insertItem(pmChat, tr("Send &Chat Request"), mnuUserSendChat);
   m->insertItem(pmFile, tr("Send &File Transfer"), mnuUserSendFile);
   m->insertItem(pmAuthorize, tr("Send &Authorization"), mnuUserAuthorize);
   connect (m, SIGNAL(activated(int)), this, SLOT(callUserFunction(int)));
   mnuUser->insertItem(tr("Send"), m);
   mnuUser->insertItem(tr("Check Auto Response"), mnuUserCheckResponse);
   mnuUser->insertItem(tr("&Away Modes"), mnuAwayModes);
   mnuUser->insertItem(tr("U&tilities"), mnuUtilities);
   mnuUser->insertSeparator();
   m = new QPopupMenu(this);
   m->insertItem(tr("&General Info"), mnuUserGeneral);
   m->insertItem(tr("&More Info"), mnuUserMore);
   m->insertItem(tr("&Work Info"), mnuUserWork);
   m->insertItem(tr("&About"), mnuUserAbout);
   connect (m, SIGNAL(activated(int)), this, SLOT(callUserFunction(int)));
   mnuUser->insertItem(tr("&Info"), m);
   mnuUser->insertItem(tr("View &History"), mnuUserHistory);
   mnuUser->insertSeparator();
   mnuUser->insertItem(tr("Online Notify"), mnuUserOnlineNotify);
   mnuUser->insertItem(tr("Invisible List"), mnuUserInvisibleList);
   mnuUser->insertItem(tr("Visible List"), mnuUserVisibleList);
   mnuUser->insertItem(tr("Ignore List"), mnuUserIgnoreList);
   mnuUser->insertSeparator();
   mnuUser->insertItem(tr("Remove"), mnuRemove);
   mnuUser->insertItem(tr("Add To Group"), mnuGroup);
   connect (mnuUser, SIGNAL(activated(int)), this, SLOT(callUserFunction(int)));
   connect (mnuUser, SIGNAL(aboutToShow()), this, SLOT(changeAutoResponse()));
}


void CMainWindow::changeAutoResponse()
{
  ICQUser *u = gUserManager.FetchUser(userView->SelectedItemUin(), LOCK_R);
  const char *szStatus = 0;
  unsigned short status = ICQ_STATUS_OFFLINE;

  if(u) {
    szStatus = u->StatusStrShort();
    status = u->Status() & 0xffff;
    gUserManager.DropUser(u);
  }

  if ( status == ICQ_STATUS_OFFLINE ||
       status == ICQ_STATUS_ONLINE)
  {
    mnuUser->changeItem(mnuUserCheckResponse, tr("Check Auto Response"));
    mnuUser->setItemEnabled(mnuUserCheckResponse, false);
  }
  else
  {
    mnuUser->changeItem(mnuUserCheckResponse, QString(tr("Check %1 Response")).arg(szStatus));
    mnuUser->setItemEnabled(mnuUserCheckResponse, true);
  }
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
  (void) new AuthUserDlg(licqDaemon);
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

#include "mainwin.moc"
