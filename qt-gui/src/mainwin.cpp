#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifdef USE_SCRNSAVER
#include <X11/extensions/scrnsaver.h>
#endif

#undef Bool
#undef None
#undef KeyPress
#undef KeyRelease
#undef FocusIn
#undef FocusOut

#ifdef USE_KDE
#include <kapp.h>
#include <kwm.h>
#include <kthemestyle.h>
#else
#include <qapp.h>
#endif

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



//-----CMainWindow::constructor-------------------------------------------------
CMainWindow::CMainWindow(CICQDaemon *theDaemon, CSignalManager *theSigMan,
                         CQtLogWindow *theLogWindow,
                         const char *skinName, const char *iconsName,
                         QWidget *parent, const char *name) : QWidget(parent, name)
{
  licqDaemon = theDaemon;
  licqSigMan = theSigMan;
  licqLogWindow = theLogWindow;

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
  licqConf.ReadStr("Font", szFont, "default");
#ifdef USE_KDE
  defaultFont = kapp->font();
#else
  defaultFont = qApp->font();
#endif
  if (strcmp(szFont, "default") != 0)
  {
    QFont f;
    f.setRawName(szFont);
#ifdef USE_KDE
    kapp->setFont(f, true);
#else
    qApp->setFont(f, true);
#endif
  }

  licqConf.ReadBool("GridLines", gridLines, false);
  licqConf.ReadBool("FontStyles", m_bFontStyles, true);
  licqConf.ReadBool("ShowHeader", showHeader, true);
  licqConf.ReadBool("ShowOfflineUsers", m_bShowOffline, true);
  licqConf.ReadBool("ShowDividers", m_bShowDividers, true);
  bool bUseDock;
  licqConf.ReadBool("UseDock", bUseDock, false);
  licqConf.ReadBool("Dock64x48", m_bDockIcon48, false);
  bool bHidden;
  licqConf.ReadBool("Hidden", bHidden, false);

  licqConf.SetSection("startup");
  licqConf.ReadNum("Logon", m_nAutoLogon, 0);
  if (m_nAutoLogon > 16) m_nAutoLogon = 0;
  licqConf.ReadNum("AutoAway", autoAwayTime, 0);
  licqConf.ReadNum("AutoNA", autoNATime, 0);

  licqConf.SetSection("functions");
  licqConf.ReadBool("AutoClose", autoClose, true);

  m_nCurrentGroup = gUserManager.DefaultGroup();
  m_nGroupType = GROUPS_USER;

  // load up position and size from file
  gLog.Info("%sGeometry configuration.\n", L_INITxSTR);
  licqConf.SetSection("geometry");
  unsigned short xPos, yPos, hVal, wVal;
  licqConf.ReadNum("x", xPos, 100);
  licqConf.ReadNum("y", yPos, 100);
  licqConf.ReadNum("h", hVal, 400);
  licqConf.ReadNum("w", wVal, 150);
  setGeometry(xPos, yPos, wVal, hVal);

  // Load the icons
  licqConf.SetSection("appearance");
  licqConf.SetFlags(INI_FxWARN);
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

  winstyle = new QWindowsStyle;
  awayMsgDlg = new AwayMsgDlg;
  optionsDlg = NULL;
  registerUserDlg = NULL;
  m_nRealHeight = 0;

  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
#ifdef USE_KDE
  m_szCaption = tr("KLicq (%1)").arg(QString::fromLocal8Bit(o->getAlias()));
#else
  m_szCaption = tr("Licq (%1)").arg(QString::fromLocal8Bit(o->getAlias()));
#endif
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

  // set the icon
#ifdef USE_KDE
  setIconText("KLicq");
#else
  setIconText("Licq");
#endif

#ifdef USE_DOCK
  licqIcon = NULL;
  if (bUseDock)
  {
    licqIcon = new IconManager(this, mnuSystem, m_bDockIcon48);
    licqIcon->setDockIconMsg(0, 0);
    licqIcon->setDockIconStatus();
  }
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
      colInfo.push_back(new CColumnInfo(colTitle, colFormat, colWidth, colAlign));
   }
   CreateUserView();

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
   {
     InformUser(this, tr("You have not yet registered a uin.  Select the\n"
                        "\"Register\" option from the system menu to\n"
                        "register an existing uin or create a new one."));
   }
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
        btnSystem = new CEButton(skin->btnSys.caption == NULL ? "System" : skin->btnSys.caption, this);
     }
     else
     {
        btnSystem = new CEButton(new QPixmap(skin->btnSys.pixmapUpFocus),
                                 new QPixmap(skin->btnSys.pixmapUpNoFocus),
                                 new QPixmap(skin->btnSys.pixmapDown),
                                 this);
        btnSystem->setStyle(winstyle);
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
    menu->setFrameStyle(QFrame::Panel | QFrame::Raised);
    menu->insertItem(skin->btnSys.caption == NULL ?
                     tr("&System") : QString::fromLocal8Bit(skin->btnSys.caption),
                     mnuSystem);
    btnSystem = NULL;
  }

  // Message Label
  if (lblMsg != NULL) delete lblMsg;
  lblMsg = new CELabel(skin->lblMsg.transparent, mnuUserGroups, this);
  if (skin->lblMsg.pixmap != NULL || skin->lblMsg.transparent)
    lblMsg->setStyle(winstyle);
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

  // Status Label
  if (lblStatus != NULL) delete lblStatus;
  lblStatus = new CELabel(skin->lblStatus.transparent, mnuStatus, this);
  if (skin->lblStatus.pixmap != NULL || skin->lblStatus.transparent)
    lblStatus->setStyle(winstyle);
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
  connect(lblStatus, SIGNAL(doubleClicked()), awayMsgDlg, SLOT(show()));

  if (!_bInitial)
  {
    resizeEvent(NULL);
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
void CMainWindow::CreateUserView(void)
{
   userView = new CUserView(mnuUser, mnuGroup, colInfo, showHeader, gridLines,
                            m_bFontStyles, this);
   userView->setPixmaps(pmOnline, pmOffline, pmAway, pmNa, pmOccupied, pmDnd,
                        pmPrivate, pmFFC, pmMessage, pmUrl, pmChat, pmFile);
   userView->setColors(skin->colors.online, skin->colors.away, skin->colors.offline,
                       skin->colors.newuser, skin->colors.background, skin->colors.gridlines);
   connect (userView, SIGNAL(doubleClicked(QListViewItem *)),
            this, SLOT(callDefaultFunction()));
#ifdef USE_KDE //Qt 2.0 has dnd support which should be used
/*   KDNDDropZone *dropzone = new KDNDDropZone(userView, DndURL);
   connect(dropzone, SIGNAL(dropAction(KDNDDropZone *)), userView, SLOT(slot_dropAction(KDNDDropZone *)));
   connect(dropzone, SIGNAL(dropEnter(KDNDDropZone *)), userView, SLOT(slot_dropEnter(KDNDDropZone *)));
   connect(dropzone, SIGNAL(dropLeave(KDNDDropZone *)), userView, SLOT(slot_dropLeave(KDNDDropZone *)));
   connect(userView, SIGNAL(signal_dropedFile(const char *)), this, SLOT(callFileFunction(const char *)));
   connect(userView, SIGNAL(signal_dropedURL(const char *)), this, SLOT(callURLFunction(const char *)));*/
#endif
}

//-----CMainWindow::destructor--------------------------------------------------
CMainWindow::~CMainWindow(void)
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


//-----CMainWindow::slot_updatedUser-----------------------------------------------
void CMainWindow::slot_updatedUser(unsigned long _nSubSignal, unsigned long _nUin)
{
  switch(_nSubSignal)
  {
  case USER_EVENTS:
    updateEvents();
    // Fall through
  case USER_EXT:
  case USER_BASIC:
    ICQUser *u = gUserManager.FetchUser(_nUin, LOCK_R);
    if (u->GetInGroup(m_nGroupType, m_nCurrentGroup))
    {
      CUserViewItem *i = (CUserViewItem *)userView->firstChild();
      while (i != NULL && i->ItemUin() != _nUin)
        i = (CUserViewItem *)i->nextSibling();
      if (i != NULL)
      {
        i->setGraphics(u);
        i->repaint();
      }
    }
    gUserManager.DropUser(u);
    break;
  }
}


//-----CMainWindow::slot_updatedList-----------------------------------------------
void CMainWindow::slot_updatedList(unsigned long _nSubSignal, unsigned long _nUin)
{
  bool bUpdateWin = true;
  switch(_nSubSignal)
  {
  case LIST_ALL:
  {
    updateUserWin();
    break;
  }
  case LIST_REORDER:
  case LIST_ADD:
  {
    ICQUser *u = gUserManager.FetchUser(_nUin, LOCK_R);
    if (u == NULL)
    {
      gLog.Warn("%sCMainWindow::slot_updatedList(): Invalid uin received: %ld\n",
                 L_ERRORxSTR, _nUin);
      break;
    }
    if (!u->GetInGroup(m_nGroupType, m_nCurrentGroup))
      bUpdateWin = false;
    gUserManager.DropUser(u);
    if (bUpdateWin) updateUserWin();
    break;
  }

  case LIST_REMOVE:
  {
    CUserViewItem *i = (CUserViewItem *)userView->firstChild();
    while (i != NULL && i->ItemUin() != _nUin) i = (CUserViewItem *)i->nextSibling();
    if (i != NULL) userView->removeItem(i);
    updateEvents();
    break;
  }

  }  // Switch
}


//-----CMainWindow::updateUserWin-----------------------------------------------
void CMainWindow::updateUserWin(void)
{
  unsigned short i = 0;

  // set the pixmap and color for each user and add them to the view
  userView->setUpdatesEnabled(false);
  userView->clear();
  bool bOfflineUsers = false;
  FOR_EACH_USER_START(LOCK_R)
  {
    // Only show users on the current group and not on the ignore list
    if (!pUser->GetInGroup(m_nGroupType, m_nCurrentGroup) ||
        (pUser->IgnoreList() && m_nGroupType != GROUPS_SYSTEM && m_nCurrentGroup != GROUP_IGNORE_LIST) )
      FOR_EACH_USER_CONTINUE

    if (i == 0 && m_bShowDividers && !pUser->getStatusOffline())
      (void) new CUserViewItem(NULL, -1, userView);
    if (!bOfflineUsers && pUser->getStatusOffline())
    {
      if (!m_bShowOffline)
        FOR_EACH_USER_BREAK

      if (m_bShowDividers)
        (void) new CUserViewItem(NULL, -2, userView);
      bOfflineUsers = true;
    }
    (void) new CUserViewItem(pUser, i, userView);
    i++;
  }
  FOR_EACH_USER_END
  userView->setUpdatesEnabled(true);
  userView->repaint();
}


void CMainWindow::updateEvents(void)
{
  QString szCaption;

  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  unsigned short nNumOwnerEvents = o->getNumMessages();
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
    lblMsg->setText(tr("No msgs"));
    szCaption = m_szCaption;
  }
  lblMsg->update();
  setCaption(szCaption);

#ifdef USE_DOCK
  if (licqIcon != NULL) licqIcon->setDockIconMsg(nNumUserEvents, nNumOwnerEvents);
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
    cmbUserGroups->insertItem((*g)[i]);
    mnuUserGroups->insertItem((*g)[i]);
    mnuGroup->insertItem((*g)[i]);
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
   char sStatus[32];
   ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
   o->getStatusStr(sStatus);
   unsigned long status = o->getStatus();
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
   mnuStatus->setItemChecked(mnuStatus->idAt(MNUxITEM_STATUSxINVISIBLE), o->getStatusInvisible());
   gUserManager.DropOwner();

   lblStatus->setText(sStatus);
   lblStatus->update();

   // set the color if it isn't set by the skin
   if (skin->lblStatus.color.fg == NULL) lblStatus->setNamedFgColor(theColor);

#ifdef USE_DOCK
  if (licqIcon != NULL) licqIcon->setDockIconStatus();
#endif
}


//----CMainWindow::changeStatusManual-------------------------------------------
void CMainWindow::changeStatusManual(int id)
{
  if (id != ICQ_STATUS_OFFLINE && (id & 0xFF) != ICQ_STATUS_ONLINE)
    awayMsgDlg->SelectAutoResponse(id);
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
    if (o->getStatusOffline())
    {
      gUserManager.DropOwner();
      return;
    }
    if (mnuStatus->isItemChecked(ICQ_STATUS_FxPRIVATE))
       newStatus = o->getStatusFull() | ICQ_STATUS_FxPRIVATE;
    else
       newStatus = o->getStatusFull() & (~ICQ_STATUS_FxPRIVATE);
  }
  else
  {
    newStatus = id;
  }

/*  default:
     gUserManager.DropOwner();
     gLog.Error("%sInternal error: CMainWindow::changeStatus(): bad index value %d.\n",
                L_ERRORxSTR, index);
     return;
  }*/

  // we may have been offline and gone online with invisible toggled
  if (mnuStatus->isItemChecked(ICQ_STATUS_FxPRIVATE))
     newStatus |= ICQ_STATUS_FxPRIVATE;

  // maintain the current extended status flags (we aren't changing any of
  // them in this function so it's ok)
  newStatus |= o->getStatusFlags();

  // disable combo box, flip pixmap...
  //lblStatus->setEnabled(false);

  // call the right function
  bool b = o->getStatusOffline();
  gUserManager.DropOwner();
  if (b)
     licqDaemon->icqLogon(newStatus);
  else
     licqDaemon->icqSetStatus(newStatus);
}


//-----CMainWindow::callDefaultFunction-------------------------------------------------------------
void CMainWindow::callDefaultFunction()      { callFunction(-1, true);   }
void CMainWindow::callOwnerFunction()        { callFunction(0, false);  }

void CMainWindow::callUrlFunction (const char *_szUrl)
{
  ICQFunctions *f = callFunction(-1, true);
  if (f != NULL) f->SendUrl(_szUrl, "");
}

void CMainWindow::callFileFunction (const char *_szFile)
{
  ICQFunctions *f = callFunction(-1, true);
  if (f != NULL) f->SendFile(_szFile, "");
}


void CMainWindow::callMsgFunction()
{
  // Do nothing if there are no events pending
  if (ICQUser::getNumUserEvents() == 0) return;

  // Do system messages first
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  unsigned short nNumMsg = o->getNumMessages();
  gUserManager.DropOwner();
  if (nNumMsg > 0)
  {
    callOwnerFunction();
    return;
  }

  // We go through backwards to (almost) get the oldest messages first
  unsigned long nUin = 0;
  FOR_EACH_USER_START(LOCK_R)
  {
    if (pUser->getNumMessages() > 0)
      nUin = pUser->getUin();
  }
  FOR_EACH_USER_END
  if (nUin != 0) callFunction(0, true, nUin);
}

//-----CMainWindow::callUserFunction-------------------------------------------
void CMainWindow::callUserFunction(int index)
{
  if(!userView->SelectedItemUin())
    return;

  switch(mnuUser->indexOf(index)) {
  case MNUxITEM_AUTHxUSER:
    licqDaemon->icqAuthorize(userView->SelectedItemUin());
    break;

  case MNUxITEM_CHECKxRESPONSE:
  {
    (void) new ShowAwayMsgDlg(licqDaemon, licqSigMan, userView->SelectedItemUin());
    break;
  }
  case MNUxITEM_ONLINExNOTIFY:
  {
    ICQUser *u = gUserManager.FetchUser(userView->SelectedItemUin(), LOCK_W);
    if (!u) break;
    u->SetOnlineNotify(!u->OnlineNotify());
    gUserManager.DropUser(u);
    if (m_bFontStyles) updateUserWin();
    break;
  }
  case MNUxITEM_INVISIBLExLIST:
  {
    ICQUser *u = gUserManager.FetchUser(userView->SelectedItemUin(), LOCK_W);
    if (!u) break;
    u->SetInvisibleList(!u->InvisibleList());
    gUserManager.DropUser(u);
    if (m_bFontStyles) updateUserWin();
    licqDaemon->icqSendInvisibleList(true);
    
    break;
  }
  case MNUxITEM_VISIBLExLIST:
  {
    ICQUser *u = gUserManager.FetchUser(userView->SelectedItemUin(), LOCK_W);
    if (!u)
      break;
    u->SetVisibleList(!u->VisibleList());
    gUserManager.DropUser(u);
    if (m_bFontStyles)
      updateUserWin();
    licqDaemon->icqSendVisibleList(true);

    break;
  }
  case MNUxITEM_IGNORExLIST:
  {
    ICQUser *u = gUserManager.FetchUser(userView->SelectedItemUin(), LOCK_W);
    if (!u)
      break;
    u->SetIgnoreList(!u->IgnoreList());
    gUserManager.DropUser(u);
    updateUserWin();

    break;
  }
  default:  
    if (mnuUser->indexOf(index) <= MNUxITEM_VIEWxHISTORY)
      callFunction(mnuUser->indexOf(index), true);
  }
}


//-----CMainWindow::callICQFunction---------------------------------------------
ICQFunctions *CMainWindow::callFunction(int fcn, bool isUser, unsigned long _nUin)
{
  ICQUser *u = NULL;
  ICQFunctions *f = NULL;
  if (!isUser)
  {
    u = gUserManager.FetchOwner(LOCK_W);
    _nUin = u->getUin();
  }
  else if (_nUin == 0)
  {
    _nUin = userView->SelectedItemUin();
    if (_nUin == 0) return f;
    u = gUserManager.FetchUser(_nUin, LOCK_W);
  }
  else
  {
    u = gUserManager.FetchUser(_nUin, LOCK_W);
  }

  if (u != NULL)
  {
    // set default function to read or send depending on whether or not
    // there are new messages
    if (fcn < 0) fcn = (u->getNumMessages() == 0 ? 1 : 0);
    if (u->fcnDlg == NULL)
    {
       f = new ICQFunctions(licqDaemon, licqSigMan, _nUin, !isUser, autoClose);
       u->fcnDlg = f;
       isUser ? gUserManager.DropUser(u) : gUserManager.DropOwner();
       connect (f, SIGNAL(signal_updatedUser(unsigned long, unsigned long)), this, SLOT(slot_updatedUser(unsigned long, unsigned long)));
       f->setupTabs(fcn);
    }
    else
    {
      Window win = ((ICQFunctions *)u->fcnDlg)->winId();
      f = (ICQFunctions *)u->fcnDlg;
#ifdef USE_KDE
      KWM::setIconify(win, false);
      KWM::raise(win);
      if (KWM::desktop(win) != KWM::currentDesktop())
        KWM::switchToDesktop(KWM::desktop(win));
#else
      XRaiseWindow(x11Display(), win);
#endif
    }
    isUser ? gUserManager.DropUser(u) : gUserManager.DropOwner();
  }
  else
  {
    gLog.Error("%sUnable to find user (uin %ld).\n", L_ERRORxSTR, _nUin);
  }
  return f;
}


void CMainWindow::slot_shutdown()
{
  licqDaemon->Shutdown();
  //qApp.quit();
}

//-----CMainWindow::slot_logon--------------------------------------------------
void CMainWindow::slot_logon()
{
  updateStatus();
  //lblStatus->setEnabled(true);
}

//-----slot_doneOwnerFcn--------------------------------------------------------
void CMainWindow::slot_doneOwnerFcn(ICQEvent *e)
{
  updateStatus();
  //bool isOk = (e->m_eResult == EVENT_ACKED || e->m_eResult == EVENT_SUCCESS);
  switch (e->m_nCommand)
  {
    case ICQ_CMDxSND_LOGON:
      if (e->m_eResult != EVENT_SUCCESS)
        WarnUser(this, tr("Logon failed.\nSee network window for details."));
      break;
/*  case ICQ_CMDxSND_LOGOFF:
     //lblStatus->setEnabled(true);
     if (!isOk) gLog.Error("%sError logging off.\n", L_ERRORxSTR);
     break;
  case ICQ_CMDxSND_SETxSTATUS:
     //lblStatus->setEnabled(true);
     if (!isOk) gLog.Error("%sError changing status.\n", L_ERRORxSTR);
     break;
  case ICQ_CMDxSND_USERxLIST:
    updateUserWin();
    if (!isOk) gLog.Error("%sError retrieving contact list.\n", L_ERRORxSTR);
    break;
  case ICQ_CMDxSND_INVISIBLExLIST:
     if (!isOk) gLog.Error("%sError sending invisible list.\n", L_ERRORxSTR);
     break;
  case ICQ_CMDxSND_VISIBLExLIST:
     if (!isOk) gLog.Error("%sError sending visible list.\n", L_ERRORxSTR);
     break;
  case ICQ_CMDxSND_SYSxMSGxREQ:
     if (!isOk) gLog.Error("%sError retrieving system messages.\n", L_ERRORxSTR);
     break;
  case ICQ_CMDxSND_PING:
     //if (!isOk) QMessageBox::critical( this, "Licq", "Error pinging Mirabilis, see Network Window for details.", QMessageBox::Ok, 0);
     break;
  case ICQ_CMDxSND_SYSxMSGxDONExACK:
     if (!isOk) gLog.Error("%sError acknowledging system messages.\n", L_ERRORxSTR);
     break;
  case ICQ_CMDxSND_USERxADD:
     if (!isOk) gLog.Error("%sError informing server of new user.\n", L_ERRORxSTR);
     break;*/
  case ICQ_CMDxSND_REGISTERxUSER:
    if (registerUserDlg != NULL)
    {
      delete registerUserDlg;
      registerUserDlg = NULL;
    }
    if (e->m_eResult == EVENT_SUCCESS)
    {
      char buf[256];
      sprintf(buf, tr("Successfully registered, your user identification\n"
                     "number (UIN) is %ld.  Now log on and update your\n"
                     "personal info."),
                   gUserManager.OwnerUin());
      InformUser(this, QString::fromLocal8Bit(buf));
      changeStatus(ICQ_STATUS_ONLINE);
      callFunction(8, false);
    }
    else
    {
      InformUser(this, tr("Registration failed.  See network window for details."));
    }
    break;
  case ICQ_CMDxSND_AUTHORIZE:
     if (e->m_eResult != EVENT_ACKED)
       gLog.Error("%sError sending autorization.\n", L_ERRORxSTR);
     else
       InformUser(this, tr("Authorization granted."));
     break;
  default:
     break;
  }
}



//-----CMainWindow::removeUser--------------------------------------------------
void CMainWindow::removeUserFromList()
{
  ICQUser *u = gUserManager.FetchUser(userView->SelectedItemUin(), LOCK_R);
  if (u == NULL) return;
  unsigned long nUin = u->getUin();
  QString warning(tr("Are you sure you want to remove\n%1 (%2)\nfrom your contact list?")
                     .arg(QString::fromLocal8Bit(u->getAlias()))
                     .arg(nUin) );
  gUserManager.DropUser(u);
  if (QueryUser(this, warning, tr("Ok"), tr("Cancel")))
     licqDaemon->RemoveUserFromList(nUin);
}

void CMainWindow::removeUserFromGroup()
{
  if (m_nCurrentGroup == 0)
    removeUserFromList();
  else
  {
    ICQUser *u = gUserManager.FetchUser(userView->SelectedItemUin(), LOCK_R);
    if (u == NULL) return;
    unsigned long nUin = u->getUin();
    GroupList *g = gUserManager.LockGroupList(LOCK_R);
    QString warning(tr("Are you sure you want to remove\n%1 (%2)\nfrom the '%3' group?")
                       .arg(QString::fromLocal8Bit(u->getAlias()))
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


//-----CMainWindow::saveAllUsers------------------------------------------------
void CMainWindow::saveAllUsers(void)
{
   gUserManager.SaveAllUsers();
}

//-----CMainWindow::addUserToGroup----------------------------------------------
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



//-----CMainWindow::saveOptions------------------------------------------------------------------------
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

  licqConf.SetSection("appearance");
  licqConf.WriteStr("Skin", skin->szSkinName);
  licqConf.WriteStr("Icons", m_szIconSet);
#ifdef USE_KDE
  if (defaultFont == kapp->font())
    licqConf.WriteStr("Font", "default");
  else
    licqConf.WriteStr("Font", kapp->font().rawName());
#else
  if (defaultFont == qApp->font())
    licqConf.WriteStr("Font", "default");
  else
    licqConf.WriteStr("Font", qApp->font().rawName());
#endif
  licqConf.WriteBool("GridLines", gridLines);
  licqConf.WriteBool("FontStyles", m_bFontStyles);
  licqConf.WriteBool("ShowHeader", showHeader);
  licqConf.WriteBool("ShowDividers", m_bShowDividers);
  licqConf.WriteBool("ShowOfflineUsers", m_bShowOffline);
  licqConf.WriteBool("UseDock", licqIcon != NULL);
  licqConf.WriteBool("Dock64x48", m_bDockIcon48);

  // save the column info
  licqConf.WriteNum("NumColumns", (unsigned short)colInfo.size());
  char colKey[32];
  for (unsigned short i = 1; i <= colInfo.size(); i++)
  {
     sprintf(colKey, "Column%d.Title", i);
     licqConf.WriteStr(colKey, colInfo[i - 1]->m_sTitle);
     sprintf(colKey, "Column%d.Format", i);
     licqConf.WriteStr(colKey, colInfo[i - 1]->m_szFormat);
     sprintf(colKey, "Column%d.Width", i);
     licqConf.WriteNum(colKey, colInfo[i - 1]->m_nWidth);
     sprintf(colKey, "Column%d.Align", i);
     licqConf.WriteNum(colKey, colInfo[i - 1]->m_nAlign);
  }

  licqConf.FlushFile();
}


//-----CMainWindow::aboutBox----------------------------------------------------
void CMainWindow::aboutBox()
{
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  QString about(tr("Licq version %1.\n"
                  "Qt GUI plugin version %2.\n\n"
                  "Author: Graham Roff\n"
                  "http://www.licq.org\n\n"
                  "%3 (%4)\n"
                  "%5 contacts.").arg(licqDaemon->Version())
                   .arg(VERSION).arg(QString::fromLocal8Bit(o->getAlias()))
                   .arg(o->getUin()).arg(gUserManager.NumUsers())  );
  gUserManager.DropOwner();
  InformUser(this, about);
}


//-----CMainWindow::changeDebug------------------------------------------------------------------------
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


//-----CMainWindow::slot_utility-------------------------------------------------
void CMainWindow::slot_utility(int _nId)
{
  int nUtility = mnuUtilities->indexOf(_nId);
  CUtility *u = gUtilityManager.Utility(nUtility);
  if (u == NULL) return;
  unsigned long nUin = userView->SelectedItemUin();
  if (nUin != 0) (void) new CUtilityDlg(u, nUin, licqDaemon);
}


//-----CMainWindow::nextServer--------------------------------------------------
void CMainWindow::nextServer()
{
  // Error reporting is done at the server level
  licqDaemon->SwitchServer();
}


//-----ToggleShowOffline--------------------------------------------------------
void CMainWindow::ToggleShowOffline()
{
  m_bShowOffline = !m_bShowOffline;
  mnuSystem->setItemChecked(mnuSystem->idAt(MNUxITEM_SHOWxOFFLINE), m_bShowOffline);
  updateUserWin();
}


//-----CMainWindow::miniMode---------------------------------------------------------------------------
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


//-----CMainWindow::autoAway-----------------------------------------------------------
void CMainWindow::autoAway()
{
#ifdef USE_SCRNSAVER
  static XScreenSaverInfo *mit_info = NULL;
  static bool bXScreenSaver = true;
  static bool bAutoAway = false;
  static bool bAutoNA = false;

  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  unsigned short status = o->getStatus();
  gUserManager.DropOwner();

  if (mit_info == NULL) mit_info = XScreenSaverAllocInfo ();
  bXScreenSaver = XScreenSaverQueryInfo (x11Display(), DefaultRootWindow (x11Display()), mit_info);
  if (!bXScreenSaver)
  {
    gLog.Warn("%sNo XScreenSaver extension found on current XServer, disabling auto-away.\n",
              L_WARNxSTR);
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


//-----CMainWindow::loadIcons---------------------------------------------------
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
   CIniFile fIconsConf(INI_FxERROR | INI_FxFATAL);
   fIconsConf.LoadFile(sFilename);
   fIconsConf.SetFlags(INI_FxWARN);

   fIconsConf.SetSection("icons");
   fIconsConf.ReadStr("Online", sFilename, "");
   sprintf(sFilepath, "%s%s", sIconPath, sFilename);
   pmOnline = new QPixmap(sFilepath);
   if (pmOnline->isNull())
     gLog.Warn("%sUnable to open 'online' pixmap '%s'.\n%sUsing blank.\n",
               L_WARNxSTR, sFilepath, L_BLANKxSTR);

   fIconsConf.ReadStr("FFC", sFilename, "");
   sprintf(sFilepath, "%s%s", sIconPath, sFilename);
   pmFFC = new QPixmap(sFilepath);
   if (pmFFC->isNull())
   {
     gLog.Warn("%sUnable to open 'free for chat' pixmap '%s'.\n%sUsing 'online' pixmap.\n",
                L_WARNxSTR, sFilepath, L_BLANKxSTR);
     pmFFC = pmOnline;
   }

   fIconsConf.ReadStr("Offline", sFilename, "");
   sprintf(sFilepath, "%s%s", sIconPath, sFilename);
   pmOffline = new QPixmap(sFilepath);
   if (pmOffline->isNull())
     gLog.Warn("%sUnable to open 'offline' pixmap '%s'.\n%sUsing blank.\n",
               L_WARNxSTR, sFilepath, L_BLANKxSTR);

   fIconsConf.ReadStr("Away", sFilename, "");
   sprintf(sFilepath, "%s%s", sIconPath, sFilename);
   pmAway = new QPixmap(sFilepath);
   if (pmAway->isNull())
     gLog.Warn("%sUnable to open 'away' pixmap '%s'.\n%sUsing blank.\n",
               L_WARNxSTR, sFilepath, L_BLANKxSTR);

   fIconsConf.ReadStr("NA", sFilename, "");
   sprintf(sFilepath, "%s%s", sIconPath, sFilename);
   pmNa = new QPixmap(sFilepath);
   if (pmNa->isNull())
     gLog.Warn("%sUnable to open 'N/A' pixmap '%s'.\n%sUsing blank.\n",
                L_WARNxSTR, sFilepath, L_BLANKxSTR);

   fIconsConf.ReadStr("Occupied", sFilename, "");
   sprintf(sFilepath, "%s%s", sIconPath, sFilename);
   pmOccupied = new QPixmap(sFilepath);
   if (pmOccupied->isNull())
     gLog.Warn("%sUnable to open 'occupied' pixmap '%s'.\n%sUsing blank.\n",
               L_WARNxSTR, sFilepath, L_BLANKxSTR);

   fIconsConf.ReadStr("DND", sFilename, "");
   sprintf(sFilepath, "%s%s", sIconPath, sFilename);
   pmDnd = new QPixmap(sFilepath);
   if (pmDnd->isNull())
     gLog.Warn("%sUnable to open 'DND' pixmap '%s'.\n%sUsing blank.\n",
               L_WARNxSTR, sFilepath, L_BLANKxSTR);

   fIconsConf.ReadStr("Private", sFilename, "");
   sprintf(sFilepath, "%s%s", sIconPath, sFilename);
   pmPrivate = new QPixmap(sFilepath);
   if (pmPrivate->isNull())
     gLog.Warn("%sUnable to open 'private' pixmap '%s'.\n%sUsing blank.\n",
               L_WARNxSTR, sFilepath, L_BLANKxSTR);

   fIconsConf.ReadStr("Message", sFilename, "");
   sprintf(sFilepath, "%s%s", sIconPath, sFilename);
   pmMessage = new QPixmap(sFilepath);
   if (pmMessage->isNull())
     gLog.Warn("%sUnable to open 'message' pixmap '%s'.\n%sUsing blank.\n",
               L_WARNxSTR, sFilepath, L_BLANKxSTR);

   fIconsConf.ReadStr("Url", sFilename, "");
   sprintf(sFilepath, "%s%s", sIconPath, sFilename);
   pmUrl = new QPixmap(sFilepath);
   if (pmUrl->isNull())
   {
     gLog.Warn("%sUnable to open 'url' pixmap '%s'.\n%sUsing 'message' pixmap.\n",
               L_WARNxSTR, sFilepath, L_BLANKxSTR);
     pmUrl = pmMessage;
   }

   fIconsConf.ReadStr("Chat", sFilename, "");
   sprintf(sFilepath, "%s%s", sIconPath, sFilename);
   pmChat = new QPixmap(sFilepath);
   if (pmChat->isNull())
   {
     gLog.Warn("%sUnable to open 'chat' pixmap '%s'.\n%sUsing 'message' pixmap.\n",
               L_WARNxSTR, sFilepath, L_BLANKxSTR);
     pmChat = pmMessage;
   }

   fIconsConf.ReadStr("File", sFilename, "");
   sprintf(sFilepath, "%s%s", sIconPath, sFilename);
   pmFile = new QPixmap(sFilepath);
   if (pmFile->isNull())
   {
     gLog.Warn("%sUnable to open 'file' pixmap '%s'.\n%sUsing 'message' pixmap.\n",
                L_WARNxSTR, sFilepath, L_BLANKxSTR);
     pmFile = pmMessage;
   }

   if (!_bInitial)
   {
     mnuStatus->changeItem(*pmOnline, tr("&Online"), mnuStatus->idAt(0));
     mnuStatus->changeItem(*pmAway, tr("&Away"), mnuStatus->idAt(1));
     mnuStatus->changeItem(*pmNa, tr("&Not Available"), mnuStatus->idAt(2));
     mnuStatus->changeItem(*pmOccupied, tr("O&ccupied"), mnuStatus->idAt(3));
     mnuStatus->changeItem(*pmDnd, tr("&Do Not Disturb"), mnuStatus->idAt(4));
     mnuStatus->changeItem(*pmFFC, tr("Free for C&hat"), mnuStatus->idAt(5));
     mnuStatus->changeItem(*pmOffline, tr("O&ffline"), mnuStatus->idAt(6));
     mnuStatus->changeItem(*pmPrivate, tr("&Invisible"), mnuStatus->idAt(8));
     mnuUser->changeItem(*pmMessage, tr("&Send Message"), mnuUser->idAt(1));
     mnuUser->changeItem(*pmUrl, tr("Send &Url"), mnuUser->idAt(2));
     mnuUser->changeItem(*pmChat, tr("Send &Chat Request"), mnuUser->idAt(3));
     mnuUser->changeItem(*pmFile, tr("Send &File Transfer"), mnuUser->idAt(4));
     userView->setPixmaps(pmOnline, pmOffline, pmAway, pmNa, pmOccupied, pmDnd,
                          pmPrivate, pmFFC, pmMessage, pmUrl, pmChat, pmFile);
     updateUserWin();
     updateEvents();
   }
}


//-----CMainWindow::initMenu----------------------------------------------------
void CMainWindow::initMenu(void)
{
   int mnuId;
   mnuStatus = new QPopupMenu(NULL);
   mnuId = mnuStatus->insertItem(*pmOnline, tr("&Online"), ICQ_STATUS_ONLINE);
   mnuStatus->setAccel(ALT + Key_O,mnuId);
   mnuId = mnuStatus->insertItem(*pmAway, tr("&Away"), ICQ_STATUS_AWAY);
   mnuStatus->setAccel(ALT + Key_A,mnuId);
   mnuId = mnuStatus->insertItem(*pmNa, tr("&Not Available"), ICQ_STATUS_NA);
   mnuStatus->setAccel(ALT + Key_N,mnuId);
   mnuId = mnuStatus->insertItem(*pmOccupied, tr("O&ccupied"), ICQ_STATUS_OCCUPIED);
   mnuStatus->setAccel(ALT + Key_C,mnuId);
   mnuId = mnuStatus->insertItem(*pmDnd, tr("&Do Not Disturb"), ICQ_STATUS_DND);
   mnuStatus->setAccel(ALT + Key_D,mnuId);
   mnuId = mnuStatus->insertItem(*pmFFC, tr("Free for C&hat"), ICQ_STATUS_FREEFORCHAT);
   mnuStatus->setAccel(ALT + Key_H,mnuId);
   mnuId = mnuStatus->insertItem(*pmOffline, tr("O&ffline"), ICQ_STATUS_OFFLINE);
   mnuStatus->setAccel(ALT + Key_F,mnuId);
   mnuStatus->insertSeparator();
   mnuId = mnuStatus->insertItem(*pmPrivate, tr("&Invisible"), ICQ_STATUS_FxPRIVATE);
   mnuStatus->setAccel(ALT + Key_I,mnuId);
   connect(mnuStatus, SIGNAL(activated(int)), this, SLOT(changeStatusManual(int)));

   mnuUserGroups = new QPopupMenu(NULL);
   connect(mnuUserGroups, SIGNAL(activated(int)), this, SLOT(setCurrentGroupMenu(int)));

   mnuUserAdm = new QPopupMenu(NULL);
   mnuUserAdm->insertItem(tr("&Add User"), this, SLOT(showAddUserDlg()));
   mnuUserAdm->insertItem(tr("S&earch for User"), this, SLOT(showSearchUserDlg()));
   mnuUserAdm->insertItem(tr("A&uthorize User"), this, SLOT(showAuthUserDlg()));
   mnuUserAdm->insertSeparator();
   mnuUserAdm->insertItem(tr("Edit Groups"), this, SLOT(showEditGrpDlg()));
   //mnuUserAdm->insertItem(tr("&Update Contact List"), this, SLOT(slot_updateContactList()));
   //mnuUserAdm->insertItem(tr("Update All Users"), this, SLOT(slot_updateAllUsers()));
   //mnuUserAdm->insertItem(tr("&Redraw User Window"), this, SLOT(updateUserWin()));
   mnuUserAdm->insertItem(tr("&Save All Users"), this, SLOT(saveAllUsers()));
   mnuUserAdm->insertSeparator();
   mnuUserAdm->insertItem(tr("Register User"), this, SLOT(slot_register()));

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
   // This hack only works assuming that the initial debug level has been set
   // to the stdout log server log types
   for (int i = 0; i < 5; i++)
     mnuDebug->setItemChecked(mnuDebug->idAt(i), DEBUG_LEVEL & (1 << i));
   connect(mnuDebug, SIGNAL(activated(int)), this, SLOT(changeDebug(int)));

   mnuSystem = new QPopupMenu(NULL);
   mnuSystem->setCheckable(true);
   ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
   mnuSystem->insertItem(tr("Owner") + " (" + QString::fromLocal8Bit(o->getAlias()) + ")",
                         this, SLOT(callOwnerFunction()));
   gUserManager.DropOwner();
   mnuSystem->insertItem(tr("User Functions"), mnuUserAdm);
   mnuSystem->insertItem(tr("&Status"), mnuStatus);
   mnuSystem->insertItem(tr("&Group"), mnuUserGroups);
   mnuSystem->insertItem(tr("Set &Auto Response"), awayMsgDlg, SLOT(show()));
   mnuSystem->insertSeparator();
   mnuSystem->insertItem(tr("Show &Network Window"), licqLogWindow, SLOT(show()));
   mnuSystem->insertItem(tr("Use &Mini Mode"), this, SLOT(miniMode()));
   mnuSystem->insertItem(tr("Show Offline &Users"), this, SLOT(ToggleShowOffline()));
   mnuSystem->insertItem(tr("&Options"), this, SLOT(showOptionsDlg()));
   mnuSystem->insertItem(tr("S&kin Browser"), this, SLOT(showSkinBrowser()));
   mnuSystem->insertItem(tr("Debug Level"), mnuDebug);
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

   mnuUser = new QPopupMenu(NULL);
   mnuUser->setCheckable(true);
   mnuUser->insertItem(tr("&View Event"));
   mnuUser->insertItem(*pmMessage, tr("&Send Message"));
   mnuUser->insertItem(*pmUrl, tr("Send &Url"));
   mnuUser->insertItem(*pmChat, tr("Send &Chat Request"));
   mnuUser->insertItem(*pmFile, tr("Send &File Transfer"));
   mnuUser->insertItem(tr("Send Authorization"));
   mnuUser->insertItem(tr("Check &Auto Response"));
   mnuUser->insertSeparator();
   mnuUser->insertItem(tr("User &Info"));
   mnuUser->insertItem(tr("User &Detail"));
   mnuUser->insertItem(tr("View &History"));
   mnuUser->insertSeparator();
   mnuUser->insertItem(tr("Online Notify"));
   mnuUser->insertItem(tr("Invisible List"));
   mnuUser->insertItem(tr("Visible List"));
   mnuUser->insertItem(tr("Ignore List"));
   mnuUser->insertSeparator();
   mnuUser->insertItem(tr("Remove"), mnuRemove);
   mnuUser->insertItem(tr("Add To Group"), mnuGroup);
   mnuUser->insertSeparator();
   mnuUser->insertItem(tr("&Utilities"), mnuUtilities);
   connect (mnuUser, SIGNAL(activated(int)), this, SLOT(callUserFunction(int)));
}


void CMainWindow::showSearchUserDlg(void)
{
  SearchUserDlg *searchUserDlg = new SearchUserDlg(licqDaemon, licqSigMan);
  searchUserDlg->show();
}


void CMainWindow::showAddUserDlg(void)
{
  AddUserDlg *addUserDlg = new AddUserDlg(licqDaemon);
  addUserDlg->show();
}


void CMainWindow::showAuthUserDlg(void)
{
  AuthUserDlg *authUserDlg = new AuthUserDlg(licqDaemon);
  authUserDlg->show();
}


void CMainWindow::showEditGrpDlg(void)
{
  EditGrpDlg *d = new EditGrpDlg;
  connect (d, SIGNAL(signal_updateGroups()), this, SLOT(updateGroups()));
  d->show();
}

void CMainWindow::showOptionsDlg(void)
{
  if (optionsDlg != NULL)
  {
    XRaiseWindow(x11Display(), optionsDlg->winId());
  }
  else
  {
    optionsDlg = new OptionsDlg(this);
  }
}

void CMainWindow::showSkinBrowser(void)
{
  SkinBrowserDlg *d = new SkinBrowserDlg(this);
  d->show();
}

void CMainWindow::slot_doneregister()
{
  registerUserDlg = NULL;
}


void CMainWindow::slot_register(void)
{
  if (gUserManager.OwnerUin() != 0)
  {
    char buf[256];
    sprintf (buf, tr("You are currently registered as\n"
                    "UIN: %ld\n"
                    "Base Directory: %s\n"
                    "Rerun licq with the -b option to select a new\n"
                    "base directory and then register a new user."),
                  gUserManager.OwnerUin(), BASE_DIR);
    InformUser(this, QString::fromLocal8Bit(buf));
    return;
  }

  if (registerUserDlg != NULL)
  {
    XRaiseWindow(x11Display(), registerUserDlg->winId());
    return;
  }

  registerUserDlg = new RegisterUserDlg(licqDaemon);
  connect(registerUserDlg, SIGNAL(signal_done()), this, SLOT(slot_doneregister()));
}

#include "moc/moc_mainwin.h"
