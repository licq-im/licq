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

#include <qdir.h>
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
#include "mledit.h"
#include "icqd.h"
#include "userbox.h"
#include "sar.h"
#include "wharf.h"


OptionsDlg::OptionsDlg(CMainWindow *_mainwin, QWidget *parent, char *name)
    : QTabDialog(parent, name)
{
  resize(550, 310);
  setCaption(tr("Licq Options"));

  mainwin = _mainwin;
  setOkButton(tr("Ok"));
  setApplyButton(tr("Apply"));
  setCancelButton(tr("Cancel"));
  connect (this, SIGNAL(applyButtonPressed()), this, SLOT(slot_apply()));

  QPushButton *btnWhat = new QPushButton(tr("What's This?"), this);
  btnWhat->setGeometry(6, height() - 30, 80, 23);
  connect(btnWhat, SIGNAL(clicked()), this, SLOT(slot_whatsthis()));

  // appearance tab
  tab[0] = new QWidget(this);

  // fonts
  boxFont = new QGroupBox(tr("Font"), tab[0]);
  boxFont->setGeometry(10, 125, 410, 90);
  QWhatsThis::add(boxFont, tr("The font used for all widgets"));
  nfoFont = new CInfoField(10, 20, 40, 5, 340, tr("Font:"), false, boxFont);
  btnFont = new QPushButton(tr("Select Font"), boxFont);
  btnFont->setGeometry(10, 45, 100, 30);
  QWhatsThis::add(btnFont, tr("Select a font from the system list"));
  connect(btnFont, SIGNAL(clicked()), this, SLOT(slot_selectfont()));

  chkFontStyles = new QCheckBox(tr("Use Font Styles"), boxFont);
  chkFontStyles->setGeometry(120, 45, 180, 20);
  QWhatsThis::add(chkFontStyles, tr("Use italics and bold in the user list to "
                                   "indicate special characteristics such as "
                                   "online notify and visible list"));
  chkGridLines = new QCheckBox(tr("Show Grid Lines"), tab[0]);
  chkGridLines->setGeometry(10, 20, 240, 20);
  QWhatsThis::add(chkGridLines, tr("Draw boxes around each square in the user list"));
  chkHeader = new QCheckBox(tr("Show Column Headers"), tab[0]);
  chkHeader->setGeometry(10, 45, 200, 20);
  QWhatsThis::add(chkHeader, tr("Turns on or off the display of headers above "
                                "each column in the user list"));
  chkShowDividers = new QCheckBox(tr("Show User Dividers"), tab[0]);
  chkShowDividers->setGeometry(10, 70, 240, 20);
  QWhatsThis::add(chkShowDividers, tr("Show the \"--online--\" and \"--offline--\" bars "
                                     "in the contact list"));
  chkAutoClose = new QCheckBox(tr("Auto Close Function Window"), tab[0]);
  chkAutoClose->setGeometry(10, 95, 240, 20);
  QWhatsThis::add(chkAutoClose, tr("Sets the default behavior for auto closing "
                                  "the user function window after a succesful event"));

  tab[1] = new QWidget(this);
  lblColTitle = new QLabel (tr("Title"), tab[1]);
  lblColTitle->setGeometry(80, 10, 80, 20);
  QWhatsThis::add(lblColTitle, tr("The string which will appear in the list box column header"));
  lblColFormat = new QLabel (tr("Format"), tab[1]);
  lblColFormat->setGeometry(170, 10, 90, 20);
  QWhatsThis::add(lblColFormat, tr("The format string used to define what will"
                                  "appear in each column, see OnEvent Command for"
                                  "more information about valid % values"));
  lblColWidth = new QLabel (tr("Width"), tab[1]);
  lblColWidth->setGeometry(270, 10, 60, 20);
  QWhatsThis::add(lblColWidth, tr("The width of the column"));
  lblColAlign = new QLabel(tr("Alignment"), tab[1]);
  lblColAlign->setGeometry(340, 10, 80, 20);
  QWhatsThis::add(lblColAlign, tr("The alignment of the column"));
  for (unsigned short i = 0; i < 4; i++)
  {
     chkColEnabled[i] = new QCheckBox(tr("Column %1").arg(i), tab[1]);
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
     cmbColAlign[i]->insertItem(tr("Left"));
     cmbColAlign[i]->insertItem(tr("Right"));
     cmbColAlign[i]->insertItem(tr("Center"));
     connect(chkColEnabled[i], SIGNAL(toggled(bool)), this, SLOT(colEnable(bool)));
  }

  tab[2] = new QWidget(this);
  chkOnEvents = new QCheckBox(tr("OnEvents Enabled"), tab[2]);
  chkOnEvents->setGeometry(10, 10, 130, 20);
  QWhatsThis::add(chkOnEvents, tr("Enable running of \"Command\" when the relevant "
                                 "event occurs."));
  lblSndPlayer = new QLabel(tr("Command:"), tab[2]);
  lblSndPlayer->setGeometry(160, 10, 60, 20);
  QWhatsThis::add(lblSndPlayer, tr("Command to execute when an event is received.  "
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
  boxSndEvents = new QGroupBox(tr("Parameters"), tab[2]);
  boxSndEvents->setGeometry(10, 40, 410, 180);
  lblSndMsg = new QLabel(tr("Message:"), boxSndEvents);
  lblSndMsg->setGeometry(10, 20, 80, 20);
  QWhatsThis::add(lblSndMsg, tr("Parameter for received messages"));
  edtSndMsg = new QLineEdit(boxSndEvents);
  edtSndMsg->setGeometry(90, 20, 280, 20);
  lblSndUrl = new QLabel(tr("URL:"), boxSndEvents);
  lblSndUrl->setGeometry(10, 45, 80, 20);
  QWhatsThis::add(lblSndUrl, tr("Parameter for received URLs"));
  edtSndUrl = new QLineEdit(boxSndEvents);
  edtSndUrl->setGeometry(90, 45, 280, 20);
  lblSndChat = new QLabel(tr("Chat Request:"), boxSndEvents);
  lblSndChat->setGeometry(10, 70, 80, 20);
  QWhatsThis::add(lblSndChat, tr("Parameter for received chat requests"));
  edtSndChat = new QLineEdit(boxSndEvents);
  edtSndChat->setGeometry(90, 70, 280, 20);
  lblSndFile = new QLabel(tr("File Transfer:"), boxSndEvents);
  lblSndFile->setGeometry(10, 95, 80, 20);
  QWhatsThis::add(lblSndFile, tr("Parameter for received file transfers"));
  edtSndFile = new QLineEdit(boxSndEvents);
  edtSndFile->setGeometry(90, 95, 280, 20);
  lblSndNotify = new QLabel(tr("Online Notify:"), boxSndEvents);
  lblSndNotify->setGeometry(10, 120, 80, 20);
  QWhatsThis::add(lblSndNotify, tr("Parameter for online notification"));
  edtSndNotify = new QLineEdit(boxSndEvents);
  edtSndNotify->setGeometry(90, 120, 280, 20);
  lblSndSysMsg = new QLabel(tr("System Msg:"), boxSndEvents);
  lblSndSysMsg->setGeometry(10, 145, 80, 20);
  QWhatsThis::add(lblSndSysMsg, tr("Parameter for received system messages"));
  edtSndSysMsg = new QLineEdit(boxSndEvents);
  edtSndSysMsg->setGeometry(90, 145, 280, 20);

  // Network tab
  tab[3] = new QWidget(this);
  new_network_options();

  // Status tab
//  tab[4] = new QWidget(this);
//  new_status_options();
 
  tab[5] = new QWidget(this);
  lblErrorLog = new QLabel(tr("Error Log:"), tab[5]);
  lblErrorLog->setGeometry(10, 60, 80, 20);
  lblErrorLog->setEnabled(false);
  edtErrorLog = new QLineEdit(tab[5]);
  edtErrorLog->setGeometry(100, 60, 200, 20);
  edtErrorLog->setEnabled(false);
  lblUrlViewer = new QLabel(tr("Url Viewer:"), tab[5]);
  lblUrlViewer->setGeometry(10, 10, 80, 20);
  QWhatsThis::add(lblUrlViewer, tr("The command to run to view a URL.  Will be passed the URL "
                                  "as a parameter."));
  edtUrlViewer = new QLineEdit(tab[5]);
  edtUrlViewer->setGeometry(100, 10, 200, 20);
  nfoTerminal = new CInfoField(10, 35, 80, 10, 200, tr("Terminal:"), false, tab[5]);
  QWhatsThis::add(nfoTerminal, tr("The command to run to start your terminal program."));
  lblTrans = new QLabel(tr("Translation:"), tab[5]);
  lblTrans->setGeometry(10, 85, 80, 20);
  QWhatsThis::add(lblTrans, tr("Sets which translation table should be used for "
                              "translating characters."));
  cmbTrans = new QComboBox(false, tab[5]);
  cmbTrans->setGeometry(100, 85, 200, 20);
  
  QString szTransFilesDir;
  szTransFilesDir.sprintf("%s%s", SHARE_DIR, TRANSLATION_DIR);
  QDir dTrans(szTransFilesDir, QString::null, QDir::Name, QDir::Files | QDir::Readable);
  if (!dTrans.count())
  {
    gLog.Error("%sError reading translation directory %s.\n",
               L_ERRORxSTR, szTransFilesDir.latin1());
    cmbTrans->insertItem(tr("ERROR"));
    cmbTrans->setEnabled(false);
  }
  else
  {
    cmbTrans->insertItem(tr("none"));
    cmbTrans->insertStringList(dTrans.entryList());
  }
  chkUseDock = new QCheckBox(tr("Use Dock Icon"), tab[5]);
  chkUseDock->setGeometry(10, 110, 120, 20);
  QWhatsThis::add(chkUseDock, tr("Controls whether or not the dockable icon should be displayed."));
  chkDockFortyEight = new QCheckBox(tr("64 x 48 Dock Icon"), tab[5]);
  chkDockFortyEight->setGeometry(30, 135, 180, 20);
  QWhatsThis::add(chkDockFortyEight, tr("Selects between the standard 64x64 icon used in the WindowMaker/Afterstep wharf "
                                "and a shorter 64x48 icon for use in the Gnome/KDE panel."));
  connect(chkUseDock, SIGNAL(toggled(bool)), chkDockFortyEight, SLOT(setEnabled(bool)));

  addTab(tab[0], tr("Appearance"));
  addTab(tab[1], tr("Columns"));
  addTab(tab[2], tr("OnEvent"));
  addTab(tab[3], tr("Network"));
//  addTab(tab[4], tr("Status"));
  addTab(tab[5], tr("Extensions"));

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
      sprintf(szTransFileName, "%s/%s/%s", BASE_DIR, TRANSLATION_DIR, (const char *)cmbTrans->currentText());
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

void OptionsDlg::new_network_options()
{
  QGridLayout* lay = new QGridLayout(tab[3], 5, 5, 10);
  QGroupBox* gbServer = new QGroupBox(2, QGroupBox::Horizontal, tab[3]);
  gbServer->setTitle(tr("Server settings"));
  lay->addWidget(gbServer, 1, 1);
  
  lblServers = new QLabel (tr("Servers:"), gbServer);
  lblServers->setEnabled(false);
  QWhatsThis::add(lblServers, tr("List of servers to connect to (read-only for now)"));
  cmbServers = new QComboBox(false, gbServer);
  cmbServers->setEnabled(false);
  lblDefServerPort = new QLabel(tr("Default Server Port:"), gbServer);
  QWhatsThis::add(lblDefServerPort, tr("Default port to connect to on the server (should be 4000)"));
  spnDefServerPort = new QSpinBox(gbServer);
  spnDefServerPort->setRange(0, 0xFFFF);
  spnDefServerPort->setSpecialValueText(tr("Default"));
  lblTcpServerPort = new QLabel(tr("TCP Server Port:"), gbServer);
  QWhatsThis::add(lblTcpServerPort, tr("TCP port for local server.  Set to \"0\" for "
                                      "system assigned port.  Use if you are behind"
                                      "a firewall or using ip masquerading."));
  spnTcpServerPort = new QSpinBox(gbServer);
  spnTcpServerPort->setRange(0, 0xFFFF);
  spnTcpServerPort->setSpecialValueText(tr("Auto"));
  lblMaxUsersPerPacket = new QLabel(tr("Max Users per Packet:"), gbServer);
  QWhatsThis::add(lblMaxUsersPerPacket, tr("Leave at 125 unless all your users appear "
                                          "offline when they should not be.  Then lower "
                                          "it until they appear again (try ~100)."));
  spnMaxUsersPerPacket = new QSpinBox(gbServer);
  spnMaxUsersPerPacket->setRange(10, 125);

  QGroupBox* gbAuto = new QGroupBox(2, QGroupBox::Horizontal, tab[3]);
  gbAuto->setTitle(tr("Network startup"));
  lay->addWidget(gbAuto, 1, 3);

  lblAutoLogon = new QLabel(tr("Auto Logon:"), gbAuto);
  QWhatsThis::add(lblAutoLogon, tr("Automatically log on when first starting up."));
  
  cmbAutoLogon = new QComboBox(gbAuto);
  cmbAutoLogon->insertItem(tr("Offline"));
  cmbAutoLogon->insertItem(tr("Online"));
  cmbAutoLogon->insertItem(tr("Away"));
  cmbAutoLogon->insertItem(tr("Not Available"));
  cmbAutoLogon->insertItem(tr("Occupied"));
  cmbAutoLogon->insertItem(tr("Do Not Disturb"));
  cmbAutoLogon->insertItem(tr("Free for Chat"));

  // dummy widget for layout
  QWidget* dummy = new QWidget(gbAuto);
  chkAutoLogonInvisible = new QCheckBox(tr("Invisible"), gbAuto);
  dummy->setMinimumHeight(chkAutoLogonInvisible->sizeHint().height()+10);

  lblAutoAway = new QLabel(tr("Auto Away:"), gbAuto);
  QWhatsThis::add(lblAutoAway, tr("Number of minutes of inactivity after which to "
                                 "automatically be marked \"away\".  Set to \"0\" "
                                 "to disable."));
  spnAutoAway = new QSpinBox(gbAuto);
  spnAutoAway->setSpecialValueText(tr("Disable"));
  lblAutoNa = new QLabel(tr("Auto N/A:"), gbAuto);
  QWhatsThis::add(lblAutoNa, tr("Number of minutes of inactivity after which to "
                               "automatically be marked \"not available\".  Set to \"0\" "
                               "to disable."));
  spnAutoNa = new QSpinBox(gbAuto);
  spnAutoNa->setSpecialValueText(tr("Disable"));

  QGroupBox* gbGeneral = new QGroupBox(1, QGroupBox::Vertical, tab[3]);
  lay->addMultiCellWidget(gbGeneral, 3, 3, 1, 3);
  
  chkHideIp = new QCheckBox(tr("Hide IP"), gbGeneral);
  QWhatsThis::add(chkHideIp, tr("Hiding ip stops users from seeing your ip."));
  chkAllowNewUsers = new QCheckBox(tr("Allow New Users"), gbGeneral);
  
  QWhatsThis::add(chkAllowNewUsers, tr("Determines if new users are automatically added"
                                      "to your list or must first request authorization."));
  chkWebPresence = new QCheckBox(tr("Web Presence Enabled"), gbGeneral);
  QWhatsThis::add(chkWebPresence, tr("Web presence allows users to see if you are online "
                                    "through your web indicator."));
}

// -----------------------------------------------------------------------------

void OptionsDlg::slot_SARmsg_act(int n)
{
  if (n < 0)
    return;
  
  SARList &sar = gSARManager.Fetch(cmbSARgroup->currentItem());
  edtSARtext->setText(sar[n]->AutoResponse());
  gSARManager.Drop();
  debug("slotactivated!!!!");
}

// -----------------------------------------------------------------------------

void OptionsDlg::new_status_options()
{
  QBoxLayout* lay = new QVBoxLayout(tab[4], 10);
  QGroupBox* gbStatus = new QGroupBox(1, QGroupBox::Horizontal, tab[4]);
  lay->addWidget(gbStatus);
  gbStatus->setTitle(tr("Default Auto Responses"));

  cmbSARgroup = new QComboBox(false, gbStatus);
  cmbSARgroup->insertItem(tr("Away"));
  cmbSARgroup->insertItem(tr("Not Available"));
  cmbSARgroup->insertItem(tr("Occupied"));
  cmbSARgroup->insertItem(tr("Do Not Disturb"));
  cmbSARgroup->insertItem(tr("Free For Chat"));
  
  cmbSARmsg = new QComboBox(true, gbStatus);
  connect(cmbSARmsg, SIGNAL(activated(int)), this, SLOT(slot_SARmsg_act(int)));
  SARList &sar = gSARManager.Fetch(SAR_AWAY);
  for (SARListIter i = sar.begin(); i != sar.end(); i++)
    cmbSARmsg->insertItem((*i)->Name());
  gSARManager.Drop();

  edtSARtext = new MLEditWrap(true, gbStatus);
}

// -----------------------------------------------------------------------------

#include "moc/moc_optionsdlg.h"
