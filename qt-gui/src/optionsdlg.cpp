#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#else
extern int errno;
#endif

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <qlayout.h>
#include <qpushbutton.h>
#ifdef USE_KDE
#include <kapp.h>
#else
#include <qapplication.h>
#endif
#include <qfontdialog.h>
#include <qwhatsthis.h>

#include "optionsdlg.h"
#include "translate.h"
#include "log.h"
#include "constants.h"
#include "support.h"
#include "mainwin.h"
#include "licq-locale.h"
#include "icqd.h"
#include "userbox.h"
#include "wharf.h"


OptionsDlg::OptionsDlg(CMainWindow *_mainwin, QWidget *parent, char *name) : QTabDialog(parent, name)
{
  resize(450, 310);
  setCaption(_("Licq Options"));

  mainwin = _mainwin;
  setOkButton(_("Ok"));
  setApplyButton(_("Apply"));
  setCancelButton(_("Cancel"));
  connect (this, SIGNAL(applyButtonPressed()), this, SLOT(slot_apply()));

  QPushButton *btnWhat = new QPushButton(_("What's This?"), this);
  btnWhat->setGeometry(6, height() - 30, 80, 23);
  connect(btnWhat, SIGNAL(clicked()), this, SLOT(slot_whatsthis()));

  // appearance tab
  tab[0] = new QWidget(this);

  // fonts
  boxFont = new QGroupBox(_("Font"), tab[0]);
  boxFont->setGeometry(10, 125, 410, 90);
  QWhatsThis::add(boxFont, _("The font used for all widgets"));
  nfoFont = new CInfoField(10, 20, 40, 5, 340, _("Font:"), false, boxFont);
  btnFont = new QPushButton(_("Select Font"), boxFont);
  btnFont->setGeometry(10, 45, 100, 30);
  QWhatsThis::add(btnFont, _("Select a font from the system list"));
  connect(btnFont, SIGNAL(clicked()), this, SLOT(slot_selectfont()));

  chkFontStyles = new QCheckBox(_("Use Font Styles"), boxFont);
  chkFontStyles->setGeometry(120, 45, 180, 20);
  QWhatsThis::add(chkFontStyles, _("Use italics and bold in the user list to "
                                   "indicate special characteristics such as "
                                   "online notify and visible list"));
  chkGridLines = new QCheckBox(_("Show Grid Lines"), tab[0]);
  chkGridLines->setGeometry(10, 20, 240, 20);
  QWhatsThis::add(chkGridLines, _("Draw boxes around each square in the user list"));
  chkHeader = new QCheckBox(_("Show Column Headers"), tab[0]);
  chkHeader->setGeometry(10, 45, 200, 20);
  QWhatsThis::add(chkHeader, _("Turns on or off the display of headers above "
                                "each column in the user list"));
  chkShowDividers = new QCheckBox(_("Show User Dividers"), tab[0]);
  chkShowDividers->setGeometry(10, 70, 240, 20);
  QWhatsThis::add(chkShowDividers, _("Show the \"--online--\" and \"--offline--\" bars "
                                     "in the contact list"));
  chkAutoClose = new QCheckBox(_("Auto Close Function Window"), tab[0]);
  chkAutoClose->setGeometry(10, 95, 240, 20);
  QWhatsThis::add(chkAutoClose, _("Sets the default behavior for auto closing "
                                  "the user function window after a succesful event"));

  tab[1] = new QWidget(this);
  lblColTitle = new QLabel (_("Title"), tab[1]);
  lblColTitle->setGeometry(80, 10, 80, 20);
  QWhatsThis::add(lblColTitle, _("The string which will appear in the list box column header"));
  lblColFormat = new QLabel (_("Format"), tab[1]);
  lblColFormat->setGeometry(170, 10, 90, 20);
  QWhatsThis::add(lblColFormat, _("The format string used to define what will"
                                  "appear in each column, see OnEvent Command for"
                                  "more information about valid % values"));
  lblColWidth = new QLabel (_("Width"), tab[1]);
  lblColWidth->setGeometry(270, 10, 60, 20);
  QWhatsThis::add(lblColWidth, _("The width of the column"));
  lblColAlign = new QLabel(_("Alignment"), tab[1]);
  lblColAlign->setGeometry(340, 10, 80, 20);
  QWhatsThis::add(lblColAlign, _("The alignment of the column"));
  for (unsigned short i = 0; i < 4; i++)
  {
     chkColEnabled[i] = new QCheckBox(_("Column %1").arg(i), tab[1]);
     chkColEnabled[i]->setGeometry(10, 40 + i * 25, 75, 20);
     edtColTitle[i] = new QLineEdit(tab[1]);
     edtColTitle[i]->setGeometry(80, 40 + i * 25, 80, 20);
     edtColFormat[i] = new QLineEdit(tab[1]);
     edtColFormat[i]->setGeometry(170, 40 + i * 25, 90, 20);
     spnColWidth[i] = new QSpinBox(tab[1]);
     spnColWidth[i]->setGeometry(270, 40 + i * 25, 60, 20);
     spnColWidth[i]->setRange(0, 2048);
     cmbColAlign[i] = new QComboBox(tab[1]);
     cmbColAlign[i]->setGeometry(340, 40 + i * 25, 80, 20);
     cmbColAlign[i]->insertItem(_("Left"));
     cmbColAlign[i]->insertItem(_("Right"));
     cmbColAlign[i]->insertItem(_("Center"));
     connect(chkColEnabled[i], SIGNAL(toggled(bool)), this, SLOT(colEnable(bool)));
  }

  tab[2] = new QWidget(this);
  chkOnEvents = new QCheckBox(_("OnEvents Enabled"), tab[2]);
  chkOnEvents->setGeometry(10, 10, 130, 20);
  QWhatsThis::add(chkOnEvents, _("Enable running of \"Command\" when the relevant "
                                 "event occurs."));
  lblSndPlayer = new QLabel(_("Command:"), tab[2]);
  lblSndPlayer->setGeometry(160, 10, 60, 20);
  QWhatsThis::add(lblSndPlayer, _("Command to execute when an event is received.  "
                                  "It will be passed the relevant parameters from "
                                  "below.  Parameters can contain the following "
                                  "expressions which will be replaced with the relevant "
                                  "information:\n"
                                  "%a - user alias\n"
                                  "%i - user ip\n"
                                  "%p - user port\n"
                                  "%e - email\n"
                                  "%n - full name\n"
                                  "%f - first name\n"
                                  "%l - last name\n"
                                  "%u - uin\n"
                                  "%w - webpage\n"
                                  "%h - phone number"));
  edtSndPlayer = new QLineEdit(tab[2]);
  edtSndPlayer->setGeometry(230, 10, 190, 20);
  boxSndEvents = new QGroupBox(_("Parameters"), tab[2]);
  boxSndEvents->setGeometry(10, 40, 410, 180);
  lblSndMsg = new QLabel(_("Message:"), boxSndEvents);
  lblSndMsg->setGeometry(10, 20, 80, 20);
  QWhatsThis::add(lblSndMsg, _("Parameter for received messages"));
  edtSndMsg = new QLineEdit(boxSndEvents);
  edtSndMsg->setGeometry(90, 20, 280, 20);
  lblSndUrl = new QLabel(_("URL:"), boxSndEvents);
  lblSndUrl->setGeometry(10, 45, 80, 20);
  QWhatsThis::add(lblSndUrl, _("Parameter for received URLs"));
  edtSndUrl = new QLineEdit(boxSndEvents);
  edtSndUrl->setGeometry(90, 45, 280, 20);
  lblSndChat = new QLabel(_("Chat Request:"), boxSndEvents);
  lblSndChat->setGeometry(10, 70, 80, 20);
  QWhatsThis::add(lblSndChat, _("Parameter for received chat requests"));
  edtSndChat = new QLineEdit(boxSndEvents);
  edtSndChat->setGeometry(90, 70, 280, 20);
  lblSndFile = new QLabel(_("File Transfer:"), boxSndEvents);
  lblSndFile->setGeometry(10, 95, 80, 20);
  QWhatsThis::add(lblSndFile, _("Parameter for received file transfers"));
  edtSndFile = new QLineEdit(boxSndEvents);
  edtSndFile->setGeometry(90, 95, 280, 20);
  lblSndNotify = new QLabel(_("Online Notify:"), boxSndEvents);
  lblSndNotify->setGeometry(10, 120, 80, 20);
  QWhatsThis::add(lblSndNotify, _("Parameter for online notification"));
  edtSndNotify = new QLineEdit(boxSndEvents);
  edtSndNotify->setGeometry(90, 120, 280, 20);
  lblSndSysMsg = new QLabel(_("System Msg:"), boxSndEvents);
  lblSndSysMsg->setGeometry(10, 145, 80, 20);
  QWhatsThis::add(lblSndSysMsg, _("Parameter for received system messages"));
  edtSndSysMsg = new QLineEdit(boxSndEvents);
  edtSndSysMsg->setGeometry(90, 145, 280, 20);

  tab[3] = new QWidget(this);
  lblServers = new QLabel (_("Servers:"), tab[3]);
  lblServers->setGeometry(10, 10, 70, 20);
  lblServers->setEnabled(false);
  QWhatsThis::add(lblServers, _("List of servers to connect to (read-only for now)"));
  cmbServers = new QComboBox(false, tab[3]);
  cmbServers->setGeometry(80, 10, 150, 20);
  cmbServers->setEnabled(false);
  lblDefServerPort = new QLabel(_("Default Server Port:"), tab[3]);
  lblDefServerPort->setGeometry(10, 35, 125, 20);
  QWhatsThis::add(lblDefServerPort, _("Default port to connect to on the server (should be 4000)"));
  spnDefServerPort = new QSpinBox(tab[3]);
  spnDefServerPort->setGeometry(140, 35, 90, 20);
  spnDefServerPort->setRange(0, 0xFFFF);
  lblTcpServerPort = new QLabel(_("TCP Server Port:"), tab[3]);
  lblTcpServerPort->setGeometry(10, 60, 120, 20);
  QWhatsThis::add(lblTcpServerPort, _("TCP port for local server.  Set to \"0\" for "
                                      "system assigned port.  Use if you are behind"
                                      "a firewall or using ip masquerading."));
  spnTcpServerPort = new QSpinBox(tab[3]);
  spnTcpServerPort->setGeometry(140, 60, 90, 20);
  spnTcpServerPort->setRange(0, 0xFFFF);
  lblMaxUsersPerPacket = new QLabel(_("Max Users per Packet:"), tab[3]);
  lblMaxUsersPerPacket->setGeometry(10, 85, 150, 20);
  QWhatsThis::add(lblMaxUsersPerPacket, _("Leave at 125 unless all your users appear "
                                          "offline when they should not be.  Then lower "
                                          "it until they appear again (try ~100)."));
  spnMaxUsersPerPacket = new QSpinBox(tab[3]);
  spnMaxUsersPerPacket->setGeometry(170, 85, 60, 20);
  spnMaxUsersPerPacket->setRange(10, 125);
  lblAutoLogon = new QLabel(_("Auto Logon:"), tab[3]);
  lblAutoLogon->setGeometry(170, 110, 90, 20);
  QWhatsThis::add(lblAutoLogon, _("Automatically log on when first starting up."));
  cmbAutoLogon = new QComboBox(tab[3]);
  cmbAutoLogon->setGeometry(260, 110, 120, 20);
  cmbAutoLogon->insertItem(_("Offline"));
  cmbAutoLogon->insertItem(_("Online"));
  cmbAutoLogon->insertItem(_("Away"));
  cmbAutoLogon->insertItem(_("Not Available"));
  cmbAutoLogon->insertItem(_("Occupied"));
  cmbAutoLogon->insertItem(_("Do Not Disturb"));
  cmbAutoLogon->insertItem(_("Free for Chat"));
  chkAutoLogonInvisible = new QCheckBox(_("Invisible"), tab[3]);
  chkAutoLogonInvisible->setGeometry(260, 135, 100, 20);
  lblAutoAway = new QLabel(_("Auto Away:"), tab[3]);
  lblAutoAway->setGeometry(10, 110, 80, 20);
  QWhatsThis::add(lblAutoAway, _("Number of minutes of inactivity after which to "
                                 "automatically be marked \"away\".  Set to \"0\" "
                                 "to disable."));
  spnAutoAway = new QSpinBox(tab[3]);
  spnAutoAway->setGeometry(100, 110, 50, 20);
  lblAutoNa = new QLabel(_("Auto N/A:"), tab[3]);
  lblAutoNa->setGeometry(10, 135, 80, 20);
  QWhatsThis::add(lblAutoNa, _("Number of minutes of inactivity after which to "
                               "automatically be marked \"not available\".  Set to \"0\" "
                               "to disable."));
  spnAutoNa = new QSpinBox(tab[3]);
  spnAutoNa->setGeometry(100, 135, 50, 20);
  chkWebPresence = new QCheckBox(_("Web Presence Enabled"), tab[3]);
  chkWebPresence->setGeometry(10, 160, 200, 20);
  QWhatsThis::add(chkWebPresence, _("Web presence allows users to see if you are online "
                                    "through your web indicator."));
  chkHideIp = new QCheckBox(_("Hide IP"), tab[3]);
  chkHideIp->setGeometry(10, 185, 200, 20);
  QWhatsThis::add(chkHideIp, _("Hiding ip stops users from seeing your ip."));
  chkAllowNewUsers = new QCheckBox(_("Allow New Users"), tab[3]);
  chkAllowNewUsers->setGeometry(170, 185, 200, 20);
  QWhatsThis::add(chkAllowNewUsers, _("Determines if new users are automatically added"
                                      "to your list or must first request authorization."));

  tab[4] = new QWidget(this);
  lblErrorLog = new QLabel(_("Error Log:"), tab[4]);
  lblErrorLog->setGeometry(10, 60, 80, 20);
  lblErrorLog->setEnabled(false);
  edtErrorLog = new QLineEdit(tab[4]);
  edtErrorLog->setGeometry(100, 60, 200, 20);
  edtErrorLog->setEnabled(false);
  lblUrlViewer = new QLabel(_("Url Viewer:"), tab[4]);
  lblUrlViewer->setGeometry(10, 10, 80, 20);
  QWhatsThis::add(lblUrlViewer, _("The command to run to view a URL.  Will be passed the URL "
                                  "as a parameter."));
  edtUrlViewer = new QLineEdit(tab[4]);
  edtUrlViewer->setGeometry(100, 10, 200, 20);
  nfoTerminal = new CInfoField(10, 35, 80, 10, 200, _("Terminal:"), false, tab[4]);
  QWhatsThis::add(nfoTerminal, _("The command to run to start your terminal program."));
  lblTrans = new QLabel(_("Translation:"), tab[4]);
  lblTrans->setGeometry(10, 85, 80, 20);
  QWhatsThis::add(lblTrans, _("Sets which translation table should be used for "
                              "translating characters."));
  cmbTrans = new QComboBox(false, tab[4]);
  cmbTrans->setGeometry(100, 85, 200, 20);
  char szTransFilesDir[MAX_FILENAME_LEN];
  struct dirent **namelist;
  sprintf(szTransFilesDir, "%s%s", SHARE_DIR, TRANSLATION_DIR);
  int n = scandir_r(szTransFilesDir, &namelist, NULL, alphasort);
  if (n < 0)
  {
    gLog.Error("%sError reading translation directory %s:\n%s%s.\n",
               L_ERRORxSTR, szTransFilesDir, L_BLANKxSTR, strerror(errno));
    cmbTrans->insertItem(_("ERROR"));
    cmbTrans->setEnabled(false);
  }
  else
  {
    cmbTrans->insertItem(_("none"));
    for (unsigned short i = 0; i < n; i++)
    {
      if (namelist[i]->d_name[0] != '.')
        cmbTrans->insertItem(namelist[i]->d_name);
      free (namelist[i]);
    }
    free (namelist);
  }
  chkUseDock = new QCheckBox(_("Use Dock Icon"), tab[4]);
  chkUseDock->setGeometry(10, 110, 120, 20);
  QWhatsThis::add(chkUseDock, _("Controls whether or not the dockable icon should be displayed."));
  chkDockFortyEight = new QCheckBox(_("64 x 48 Dock Icon"), tab[4]);
  chkDockFortyEight->setGeometry(30, 135, 180, 20);
  QWhatsThis::add(chkDockFortyEight, _("Selects between the standard 64x64 icon used in the WindowMaker/Afterstep wharf "
                                "and a shorter 64x48 icon for use in the Gnome/KDE panel."));
  connect(chkUseDock, SIGNAL(toggled(bool)), chkDockFortyEight, SLOT(setEnabled(bool)));

  addTab(tab[0], _("Appearance"));
  addTab(tab[1], _("Columns"));
  addTab(tab[2], _("OnEvent"));
  addTab(tab[3], _("Network"));
  addTab(tab[4], _("Extensions"));

  SetupOptions();
  show();
}


void OptionsDlg::slot_apply()
{
  ApplyOptions();
}

void OptionsDlg::hide()
{
  QTabDialog::hide();
  mainwin->optionsDlg = NULL;
  delete this;
}


void OptionsDlg::slot_whatsthis()
{
  QWhatsThis::enterWhatsThisMode();
}


void OptionsDlg::colEnable(bool isOn)
{
   unsigned short i = 0;
   while (i < 4 && chkColEnabled[i]->isChecked()) i++;

   // the box is checked and now i points to the first unchecked box
   if (isOn)
   {
      // i is the next column, i - 1 is the clicked on column
      if (i > 1) chkColEnabled[i - 2]->setEnabled(false);
      if (i < 4) chkColEnabled[i]->setEnabled(true);
      if (i > 0)
      {
         edtColTitle[i - 1]->setEnabled(true);
         edtColFormat[i - 1]->setEnabled(true);
         spnColWidth[i - 1]->setEnabled(true);
         cmbColAlign[i - 1]->setEnabled(true);
      }
   }
   else
   {
      // i is clicked on column which is now unchecked
      if (i < 3) chkColEnabled[i + 1]->setEnabled(false);
      if (i > 0) chkColEnabled[i - 1]->setEnabled(true);
      if (i < 4)
      {
         edtColTitle[i]->setEnabled(false);
         edtColFormat[i]->setEnabled(false);
         spnColWidth[i]->setEnabled(false);
         cmbColAlign[i]->setEnabled(false);
      }
   }
}


//-----CMainWindow::showOptions------------------------------------------------------------------------
void OptionsDlg::SetupOptions()
{
#ifdef USE_KDE
  if (mainwin->defaultFont == kapp->font())
    nfoFont->setData("default");
  else
    nfoFont->setData(kapp->font().rawName());
#else
  if (mainwin->defaultFont == qApp->font())
    nfoFont->setData("default");
  else
    nfoFont->setData(qApp->font().rawName());
#endif

  chkGridLines->setChecked(mainwin->gridLines);
  chkFontStyles->setChecked(mainwin->m_bFontStyles);
  chkHeader->setChecked(mainwin->showHeader);
  chkShowDividers->setChecked(mainwin->m_bShowDividers);
  chkAutoClose->setChecked(mainwin->autoClose);
  chkUseDock->setChecked(mainwin->licqIcon != NULL);
  chkDockFortyEight->setChecked(mainwin->m_bDockIcon48);
  //chkDockFortyEight->set

  spnDefServerPort->setValue(mainwin->licqDaemon->getDefaultRemotePort());
  spnTcpServerPort->setValue(mainwin->licqDaemon->getTcpServerPort());
  spnMaxUsersPerPacket->setValue(mainwin->licqDaemon->getMaxUsersPerPacket());

  cmbServers->clear();
  unsigned short i;
  for (i = 0; i < mainwin->licqDaemon->icqServers.numServers(); i++)
    cmbServers->insertItem(mainwin->licqDaemon->icqServers.servers[i]->name());

  spnAutoAway->setValue(mainwin->autoAwayTime);
  spnAutoNa->setValue(mainwin->autoNATime);
  cmbAutoLogon->setCurrentItem(mainwin->m_nAutoLogon >= 10 ? mainwin->m_nAutoLogon - 10 : mainwin->m_nAutoLogon);
  chkAutoLogonInvisible->setChecked(mainwin->m_nAutoLogon > 10);

  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  chkHideIp->setChecked(o->getStatusHideIp());
  chkWebPresence->setChecked(o->getStatusWebPresence());
  gUserManager.DropOwner();
  chkAllowNewUsers->setChecked(mainwin->licqDaemon->AllowNewUsers());

  // plugins tab
  //optionsDlg->edtErrorLog->setText(server->getErrorLogName());
  edtUrlViewer->setText(mainwin->licqDaemon->getUrlViewer() == NULL ? "none" : mainwin->licqDaemon->getUrlViewer());
  nfoTerminal->setData(mainwin->licqDaemon->Terminal() == NULL ? "none" : mainwin->licqDaemon->Terminal());
  const char *pc = gTranslator.getMapName();
  if (strcmp(pc, "none") == 0)
  {
    cmbTrans->setCurrentItem(0);
  }
  else
   {
     unsigned short n = 1, c = cmbTrans->count();
     while (n < c && strcmp(pc, cmbTrans->text(n)) != 0) n++;
     if (n == c)
     {
       gLog.Error("%sError: Unable to find current translation map '%s' in translation directory.\n", 
                  L_ERRORxSTR, pc);
       cmbTrans->setEnabled(false);
     }
     else
       cmbTrans->setCurrentItem(n);
   }

   // set up the columns stuff
   for (i = 0; i < mainwin->colInfo.size(); i++)
   {
      chkColEnabled[i]->setChecked(true);
      edtColTitle[i]->setText(mainwin->colInfo[i]->m_sTitle);
      edtColFormat[i]->setText(mainwin->colInfo[i]->m_szFormat);
      spnColWidth[i]->setValue(mainwin->userView->columnWidth(i + 1));
      cmbColAlign[i]->setCurrentItem(mainwin->colInfo[i]->m_nAlign);
      chkColEnabled[i]->setEnabled(true);
      edtColTitle[i]->setEnabled(true);
      edtColFormat[i]->setEnabled(true);
      spnColWidth[i]->setEnabled(true);
      cmbColAlign[i]->setEnabled(true);
   }
   if (i < 4)
   {
      chkColEnabled[i]->setChecked(false);
      edtColTitle[i]->setEnabled(false);
      edtColFormat[i]->setEnabled(false);
      spnColWidth[i]->setEnabled(false);
      cmbColAlign[i]->setEnabled(false);
      i++;
   }
   for (; i < 4; i++)
   {
      chkColEnabled[i]->setChecked(false);
      chkColEnabled[i]->setEnabled(false);
      edtColTitle[i]->setEnabled(false);
      edtColFormat[i]->setEnabled(false);
      spnColWidth[i]->setEnabled(false);
      cmbColAlign[i]->setEnabled(false);
   }

   // set up the sound stuff
   COnEventManager *oem = mainwin->licqDaemon->OnEventManager();
   chkOnEvents->setChecked(oem->CommandType() != ON_EVENT_IGNORE);
   oem->Lock();
   edtSndPlayer->setText(oem->Command());
   edtSndMsg->setText(oem->Parameter(ON_EVENT_MSG));
   edtSndUrl->setText(oem->Parameter(ON_EVENT_URL));
   edtSndChat->setText(oem->Parameter(ON_EVENT_CHAT));
   edtSndFile->setText(oem->Parameter(ON_EVENT_FILE));
   edtSndNotify->setText(oem->Parameter(ON_EVENT_NOTIFY));
   edtSndSysMsg->setText(oem->Parameter(ON_EVENT_SYSMSG));
   oem->Unlock();
}


//-----OptionsDlg::ApplyOptions--------------------------------------------------
void OptionsDlg::ApplyOptions()
{
  if (strcmp(nfoFont->text(), "default") == 0)
#ifdef USE_KDE
    kapp->setFont(mainwin->defaultFont, true);
#else
    qApp->setFont(mainwin->defaultFont, true);
#endif
  else
  {
    QFont f;
    f.setRawName(nfoFont->text());
#ifdef USE_KDE
    kapp->setFont(f, true);
#else
    qApp->setFont(f, true);
#endif
  }

  mainwin->gridLines = chkGridLines->isChecked();
  mainwin->m_bFontStyles = chkFontStyles->isChecked();
  mainwin->showHeader = chkHeader->isChecked();
  mainwin->m_bShowDividers = chkShowDividers->isChecked();
  mainwin->autoClose = chkAutoClose->isChecked();
  if (chkUseDock->isChecked())
  {
    if (mainwin->licqIcon == NULL)
    {
      mainwin->licqIcon = new IconManager(mainwin, mainwin->mnuSystem, chkDockFortyEight->isChecked());
      mainwin->updateStatus();
      mainwin->updateEvents();
    }
    else if (mainwin->m_bDockIcon48 != chkDockFortyEight->isChecked())
    {
      delete mainwin->licqIcon;
      mainwin->licqIcon = new IconManager(mainwin, mainwin->mnuSystem, chkDockFortyEight->isChecked());
      mainwin->updateStatus();
      mainwin->updateEvents();
    }
  }
  else
  {
    if (mainwin->licqIcon != NULL)
    {
      delete mainwin->licqIcon;
      mainwin->licqIcon = NULL;
    }
  }
  mainwin->m_bDockIcon48 = chkDockFortyEight->isChecked();

  mainwin->licqDaemon->setDefaultRemotePort(spnDefServerPort->value());
  mainwin->licqDaemon->setTcpServerPort(spnTcpServerPort->value());
  mainwin->licqDaemon->setMaxUsersPerPacket(spnMaxUsersPerPacket->value());
  mainwin->licqDaemon->SetAllowNewUsers(chkAllowNewUsers->isChecked());

  // Plugin tab
  //server->setErrorLogName((const char *)optionsDlg->edtErrorLog->text());
  mainwin->licqDaemon->setUrlViewer(edtUrlViewer->text());
  mainwin->licqDaemon->SetTerminal(nfoTerminal->text());
  if (cmbTrans->isEnabled())
  {
    if (cmbTrans->currentItem() == 0)
    {
      gTranslator.setDefaultTranslationMap();
    }
    else
    {
      char szTransFileName[MAX_FILENAME_LEN];
      sprintf(szTransFileName, "%s%s%s", BASE_DIR, TRANSLATION_DIR, (const char *)cmbTrans->currentText());
      gTranslator.setTranslationMap(szTransFileName);
    }
  }

  //optionsDlg->cmbServers->clear();
  //unsigned short i;
  //for (i = 0; i < server->icqServers.numServers(); i++)
  //   optionsDlg->cmbServers->insertItem(server->icqServers.servers[i]->name());

  mainwin->autoAwayTime = spnAutoAway->value();
  mainwin->autoNATime = spnAutoNa->value();
  mainwin->m_nAutoLogon = cmbAutoLogon->currentItem() +
                          (chkAutoLogonInvisible->isChecked() ? 10 : 0);
  ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
  if (chkWebPresence->isChecked())
    o->setStatusFlag(ICQ_STATUS_FxWEBxPRESENCE);
  else
    o->clearStatusFlag(ICQ_STATUS_FxWEBxPRESENCE);
  if (chkHideIp->isChecked())
    o->setStatusFlag(ICQ_STATUS_FxHIDExIP);
  else
    o->clearStatusFlag(ICQ_STATUS_FxHIDExIP);
  gUserManager.DropOwner();

  // set up the columns stuff
  unsigned short i, j = mainwin->colInfo.size();
  for (i = 0; i < j; i++) delete mainwin->colInfo[i];  // erase the old array
  mainwin->colInfo.clear();
  i = 0;
  while (chkColEnabled[i]->isChecked())
  {
     mainwin->colInfo.push_back(new CColumnInfo(edtColTitle[i]->text(),
                                                edtColFormat[i]->text(),
                                                spnColWidth[i]->value(),
                                                cmbColAlign[i]->currentItem()));
     i++;
  }
  delete mainwin->userView;
  mainwin->CreateUserView();
  mainwin->updateUserWin();
  mainwin->resizeEvent(NULL);
  mainwin->userView->show();

  // set up the sound stuff
  COnEventManager *oem = mainwin->licqDaemon->OnEventManager();
  oem->SetCommandType(chkOnEvents->isChecked() ? ON_EVENT_RUN : ON_EVENT_IGNORE);
  const char *oemparams[6] = { edtSndMsg->text(), edtSndUrl->text(),
                               edtSndChat->text(), edtSndFile->text(),
                               edtSndNotify->text(), edtSndSysMsg->text() };
  oem->SetParameters(edtSndPlayer->text(), oemparams);

}


void OptionsDlg::slot_selectfont()
{
  bool fontOk;
  QFont f = QFontDialog::getFont(&fontOk, this);
  if (fontOk)
    nfoFont->setData(f.rawName());
}

#include "moc/moc_optionsdlg.h"
