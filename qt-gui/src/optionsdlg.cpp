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

#include <qdir.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qwhatsthis.h>
#include <qstylesheet.h>
#include <qtabwidget.h>
#include <qtoolbutton.h>
#ifdef USE_KDE
#include <kapp.h>
#include <kfontdialog.h>
#else
#include <qapplication.h>
#include <qfontdialog.h>
#endif

#include "optionsdlg.h"
#include "licq_translate.h"
#include "licq_log.h"
#include "licq_constants.h"
#include "mainwin.h"
#include "mledit.h"
#include "licq_icqd.h"
#include "userbox.h"
#include "licq_sar.h"
#include "wharf.h"
#include "skin.h"



OptionsDlg::OptionsDlg(CMainWindow *_mainwin, tabs settab, QWidget *parent)
  : QDialog(parent, "OptionsDialog", false, WStyle_ContextHelp | WDestructiveClose)
{
  setCaption(tr("Licq Options"));

  mainwin = _mainwin;

  QBoxLayout* top_lay = new QVBoxLayout(this, 4);

  tabw = new QTabWidget(this);
  top_lay->addWidget(tabw);

  QBoxLayout* lay = new QHBoxLayout(top_lay);
  int bw = 0;

  btnOk = new QPushButton(tr("&OK"), this);
  connect (btnOk, SIGNAL(clicked()), this, SLOT(slot_ok()));
  bw = QMAX(bw, btnOk->sizeHint().width());

  btnApply = new QPushButton(tr("&Apply"), this);
  connect(btnApply, SIGNAL(clicked()), this, SLOT(ApplyOptions()));
  bw = QMAX(bw, btnApply->sizeHint().width());

  btnCancel = new QPushButton(tr("&Cancel"), this);
  connect(btnCancel, SIGNAL(clicked()), this, SLOT(close()));
  bw = QMAX(bw, btnCancel->sizeHint().width());

  lay->addSpacing(15);
  lay->addWidget(QWhatsThis::whatsThisButton(this));
  lay->addStretch(2);
  btnOk->setFixedWidth(bw);
  lay->addWidget(btnOk);
  lay->addSpacing(6);
  btnApply->setFixedWidth(bw);
  lay->addWidget(btnApply);
  lay->addSpacing(15);
  btnCancel->setFixedWidth(bw);
  lay->addWidget(btnCancel);

  tab[0] = new_appearance_options();
  tab[1] = new_column_options();
  tab[2] = new_sounds_options();
  tab[3] = new_network_options();
  tab[4] = new_status_options();
  tab[5] = new_misc_options();

  tabw->addTab(tab[0], tr("General"));
  tabw->addTab(tab[1], tr("Contact List"));
  tabw->addTab(tab[2], tr("OnEvent"));
  tabw->addTab(tab[3], tr("Network"));
  tabw->addTab(tab[4], tr("Status"));
  tabw->addTab(tab[5], tr("Miscellaneous"));

  SetupOptions();

  tabw->showPage(tab[settab]);
  show();
}

OptionsDlg::~OptionsDlg()
{
  emit signal_done();
}

void OptionsDlg::accept()
{
  // ugly hack to make it work with Qt 2.1 && Qt 2.2
  clearWFlags(WDestructiveClose);
  close();
  delete this;
}

void OptionsDlg::reject()
{
  QTimer::singleShot(0, this, SLOT(close()));
  QDialog::reject();
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
      if (i < 4 && i > 0) chkColEnabled[i]->setEnabled(true);
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
      if (i > 1) chkColEnabled[i - 1]->setEnabled(true);
      if (i < 4)
      {
         edtColTitle[i]->setEnabled(false);
         edtColFormat[i]->setEnabled(false);
         spnColWidth[i]->setEnabled(false);
         cmbColAlign[i]->setEnabled(false);
      }
   }
   chkColEnabled[0]->setEnabled(false);
}


void OptionsDlg::SetupOptions()
{
  setupFontName(edtFont, qApp->font());
  if(!MLEditWrap::editFont)
    MLEditWrap::editFont = new QFont(qApp->font());
  setupFontName(edtEditFont, *MLEditWrap::editFont);
  ((QWidget*)edtEditFont)->setFont(*MLEditWrap::editFont, true);

  chkGridLines->setChecked(mainwin->m_bGridLines);
  chkFontStyles->setChecked(mainwin->m_bFontStyles);
  chkShowExtIcons->setChecked(mainwin->m_bShowExtendedIcons);
  chkHeader->setChecked(mainwin->m_bShowHeader);
  chkShowDividers->setChecked(mainwin->m_bShowDividers);
  chkSortByStatus->setChecked(mainwin->m_bSortByStatus);
  chkAlwaysShowONU->setChecked(mainwin->m_bAlwaysShowONU);
  chkShowGroupIfNoMsg->setChecked(mainwin->m_bShowGroupIfNoMsg);
  chkAutoClose->setChecked(mainwin->m_bAutoClose);
  chkTransparent->setChecked(mainwin->skin->frame.transparent);
  chkScrollBar->setChecked(mainwin->m_bScrollBar);
  chkFlashUrgent->setChecked(mainwin->m_nFlash >= FLASH_URGENT);
  chkFlashAll->setChecked(mainwin->m_nFlash == FLASH_ALL);
  chkAutoPopup->setChecked(mainwin->m_bAutoPopup);
  chkAutoRaise->setChecked(mainwin->m_bAutoRaise);
  chkBoldOnMsg->setChecked(mainwin->m_bBoldOnMsg);
  chkManualNewUser->setChecked(mainwin->m_bManualNewUser);
  edtFrameStyle->setText(QString::number((int)mainwin->skin->frame.frameStyle));
  edtHotKey->setText(mainwin->m_MsgAutopopupKey);
  chkSysBack->setChecked(mainwin->m_bSystemBackground);
  chkSendFromClipboard->setChecked(mainwin->m_bSendFromClipboard);
  chkAutoPosReplyWin->setChecked(mainwin->m_bAutoPosReplyWin);
  chkAutoSendThroughServer->setChecked(mainwin->m_bAutoSendThroughServer);
  chkEnableMainwinMouseMovement->setChecked(mainwin->m_bEnableMainwinMouseMovement);
  switch(mainwin->m_nDockMode)
  {
    case DockNone:
      chkUseDock->setChecked(false);
      cmbDockTheme->setEnabled(false);
      rdbDockDefault->setEnabled(false);
      rdbDockThemed->setEnabled(false);
      chkDockFortyEight->setEnabled(false);
      break;
#ifndef USE_KDE
    case DockDefault:
      chkUseDock->setChecked(true);
      rdbDockDefault->setChecked(true);
      chkDockFortyEight->setChecked( ((IconManager_Default *)mainwin->licqIcon)->FortyEight());
      chkDockFortyEight->setEnabled(true);
      cmbDockTheme->setEnabled(false);
      break;
    case DockThemed:
      chkUseDock->setChecked(true);
      rdbDockThemed->setChecked(true);
      cmbDockTheme->setEnabled(true);
      chkDockFortyEight->setEnabled(false);
      for (unsigned short i = 0; i < cmbDockTheme->count(); i++)
      {
        if (cmbDockTheme->text(i) == ((IconManager_Themed *)mainwin->licqIcon)->Theme())
        {
          cmbDockTheme->setCurrentItem(i);
          break;
        }
      }
      break;
#else
    case DockDefault:
    case DockThemed:
      chkUseDock->setChecked(true);
      cmbDockTheme->setEnabled(false);
      rdbDockDefault->setEnabled(false);
      rdbDockThemed->setEnabled(false);
      chkDockFortyEight->setEnabled(false);
      break;
#endif
  }

  spnDefServerPort->setValue(mainwin->licqDaemon->getDefaultRemotePort());
  //chkFirewall->setChecked(mainwin->licqDaemon->FirewallHost()[0] == '\0');
  chkTCPEnabled->setChecked(mainwin->licqDaemon->TCPEnabled());
  //edtFirewallHost->setText(mainwin->licqDaemon->FirewallHost());
  spnPortLow->setValue(mainwin->licqDaemon->TCPPortsLow());
  spnPortHigh->setValue(mainwin->licqDaemon->TCPPortsHigh());
  //chkFirewall->toggle();

  cmbServers->clear();
  unsigned short i;
  for (i = 0; i < mainwin->licqDaemon->icqServers.numServers(); i++)
    cmbServers->insertItem(mainwin->licqDaemon->icqServers.servers[i]->name());

  spnAutoAway->setValue(mainwin->autoAwayTime);
  spnAutoNa->setValue(mainwin->autoNATime);
  spnAutoOffline->setValue(mainwin->autoOfflineTime);
  cmbAutoLogon->setCurrentItem(mainwin->m_nAutoLogon % 10);
  chkAutoLogonInvisible->setChecked(mainwin->m_nAutoLogon >= 10);


  chkIgnoreNewUsers->setChecked(mainwin->licqDaemon->Ignore(IGNORE_NEWUSERS));
  chkIgnoreMassMsg->setChecked(mainwin->licqDaemon->Ignore(IGNORE_MASSMSG));
  chkIgnoreWebPanel->setChecked(mainwin->licqDaemon->Ignore(IGNORE_WEBPANEL));
  chkIgnoreEmailPager->setChecked(mainwin->licqDaemon->Ignore(IGNORE_EMAILPAGER));

  // plugins tab
  edtUrlViewer->setText(mainwin->licqDaemon->getUrlViewer() == NULL ?
                        tr("none") : QString(mainwin->licqDaemon->getUrlViewer()));
  edtTerminal->setText(mainwin->licqDaemon->Terminal() == NULL ?
                       tr("none") : QString(mainwin->licqDaemon->Terminal()));
  const char *pc = gTranslator.getMapName();
  if (strcmp(pc, "none") == 0)
  {
    cmbTrans->setCurrentItem(0);
  }
  else
   {
     unsigned short n = 1, c = cmbTrans->count();
     while (n < c && pc != cmbTrans->text(n)) n++;
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
   chkColEnabled[0]->setEnabled(false);

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
   edtSndMsgSent->setText(oem->Parameter(ON_EVENT_MSGSENT));
   oem->Unlock();
   ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
   chkOEAway->setChecked(o->AcceptInAway());
   chkOENA->setChecked(o->AcceptInNA());
   chkOEOccupied->setChecked(o->AcceptInOccupied());
   chkOEDND->setChecked(o->AcceptInDND());
   gUserManager.DropOwner();
   chkAlwaysOnlineNotify->setChecked(mainwin->licqDaemon->AlwaysOnlineNotify());
}


void OptionsDlg::slot_ok()
{
  ApplyOptions();
  gMainWindow->saveOptions();

  accept();
}


//-----OptionsDlg::ApplyOptions----------------------------------------------
void OptionsDlg::ApplyOptions()
{
  QFont f(mainwin->defaultFont);
  if(edtEditFont->text().find(tr("default"), 0, false) != 0)
    f.setRawName(edtEditFont->text());
  delete MLEditWrap::editFont;
  MLEditWrap::editFont = new QFont(f);

  f = mainwin->defaultFont;
  if(edtFont->text().find(tr("default"), 0, false) != 0)
    f.setRawName(edtFont->text());
  qApp->setFont(f, true);

  mainwin->m_bGridLines = chkGridLines->isChecked();
  mainwin->m_bFontStyles = chkFontStyles->isChecked();
  mainwin->m_bShowExtendedIcons = chkShowExtIcons->isChecked();
  mainwin->m_bShowHeader = chkHeader->isChecked();
  mainwin->m_bShowDividers = chkShowDividers->isChecked();
  mainwin->m_bSortByStatus = chkSortByStatus->isChecked();
  mainwin->m_bAlwaysShowONU = chkAlwaysShowONU->isChecked();
  mainwin->m_bShowGroupIfNoMsg = chkShowGroupIfNoMsg->isChecked();
  mainwin->m_bAutoClose = chkAutoClose->isChecked();
  mainwin->m_bAutoPopup = chkAutoPopup->isChecked();
  mainwin->m_bAutoRaise = chkAutoRaise->isChecked();
  mainwin->m_bBoldOnMsg = chkBoldOnMsg->isChecked();
  mainwin->m_bManualNewUser = chkManualNewUser->isChecked();
  mainwin->m_bScrollBar = chkScrollBar->isChecked();
  mainwin->m_nFlash = chkFlashAll->isChecked() ? FLASH_ALL :
                      chkFlashUrgent->isChecked() ? FLASH_URGENT : FLASH_NONE;
  mainwin->skin->frame.transparent = chkTransparent->isChecked();
  mainwin->skin->frame.frameStyle = edtFrameStyle->text().toUShort();
  mainwin->m_MsgAutopopupKey = edtHotKey->text();
  mainwin->m_bSystemBackground = chkSysBack->isChecked();
  mainwin->m_bSendFromClipboard = chkSendFromClipboard->isChecked();
  mainwin->m_bAutoPosReplyWin = chkAutoPosReplyWin->isChecked();
  mainwin->m_bAutoSendThroughServer = chkAutoSendThroughServer->isChecked();
  mainwin->m_bEnableMainwinMouseMovement = chkEnableMainwinMouseMovement->isChecked();
#ifndef USE_KDE
  if (chkUseDock->isChecked() &&
      (rdbDockDefault->isChecked() || rdbDockThemed->isChecked()) )
#else
  if (chkUseDock->isChecked())
#endif
  {
#ifndef USE_KDE
    if (rdbDockDefault->isChecked())
    {
      if (mainwin->m_nDockMode != DockDefault ||
          ((IconManager_Default *)mainwin->licqIcon)->FortyEight() != chkDockFortyEight->isChecked() )
      {
        delete mainwin->licqIcon;
        mainwin->licqIcon = new IconManager_Default(mainwin, mainwin->mnuSystem, chkDockFortyEight->isChecked());
        mainwin->m_nDockMode = DockDefault;
      }
    }
    else if (rdbDockThemed->isChecked())
    {
      if (mainwin->m_nDockMode != DockThemed)
      {
        delete mainwin->licqIcon;
        mainwin->licqIcon = new IconManager_Themed(mainwin, mainwin->mnuSystem, cmbDockTheme->currentText().local8Bit());
        mainwin->m_nDockMode = DockThemed;
      }
      else if ( ((IconManager_Themed *)mainwin->licqIcon)->Theme() != cmbDockTheme->currentText() )
      {
        ((IconManager_Themed *)mainwin->licqIcon)->SetTheme(cmbDockTheme->currentText().local8Bit());
      }
    }
#else
    if(!mainwin->licqIcon)
      mainwin->licqIcon = new IconManager_KDEStyle(mainwin, mainwin->mnuSystem);
    mainwin->m_nDockMode = DockDefault;
#endif
    mainwin->updateStatus();
    mainwin->updateEvents();
  }
  else
  {
    delete mainwin->licqIcon;
    mainwin->licqIcon = NULL;
    mainwin->m_nDockMode = DockNone;
  }

  mainwin->licqDaemon->setDefaultRemotePort(spnDefServerPort->value());
  //if (chkFirewall->isChecked())
  {
    mainwin->licqDaemon->SetTCPPorts(spnPortLow->value(), spnPortHigh->value());
    //mainwin->licqDaemon->SetFirewallHost(edtFirewallHost->text().local8Bit());
    mainwin->licqDaemon->SetTCPEnabled(chkTCPEnabled->isChecked());
  }
  /*else
  {
    mainwin->licqDaemon->SetTCPBasePort(0, 0);
    mainwin->licqDaemon->SetFirewallHost("");
    mainwin->licqDaemon->SetTCPEnabled(true);
  }*/
  mainwin->licqDaemon->SetIgnore(IGNORE_NEWUSERS, chkIgnoreNewUsers->isChecked());
  mainwin->licqDaemon->SetIgnore(IGNORE_MASSMSG, chkIgnoreMassMsg->isChecked());
  mainwin->licqDaemon->SetIgnore(IGNORE_WEBPANEL, chkIgnoreWebPanel->isChecked());
  mainwin->licqDaemon->SetIgnore(IGNORE_EMAILPAGER, chkIgnoreEmailPager->isChecked());

  // Plugin tab
  mainwin->licqDaemon->setUrlViewer(edtUrlViewer->text().local8Bit());
  mainwin->licqDaemon->SetTerminal(edtTerminal->text().local8Bit());
  if (cmbTrans->isEnabled())
  {
    if (cmbTrans->currentItem() == 0)
    {
      gTranslator.setDefaultTranslationMap();
    }
    else
    {
      char szTransFileName[MAX_FILENAME_LEN];
      snprintf(szTransFileName, MAX_FILENAME_LEN, "%s/%s/%s", SHARE_DIR, TRANSLATION_DIR, QFile::encodeName(cmbTrans->currentText()).data());
      gTranslator.setTranslationMap(szTransFileName);
    }
  }

  mainwin->autoAwayTime = spnAutoAway->value();
  mainwin->autoNATime = spnAutoNa->value();
  mainwin->autoOfflineTime = spnAutoOffline->value();
  mainwin->m_nAutoLogon = cmbAutoLogon->currentItem() +
   chkAutoLogonInvisible->isChecked() * 10;

  mainwin->autoAwayMess = cmbAutoAwayMess->currentItem();
  mainwin->autoNAMess = cmbAutoNAMess->currentItem();

  // set up the columns stuff
  unsigned short i, j = mainwin->colInfo.size();
  for (i = 0; i < j; i++) delete mainwin->colInfo[i];  // erase the old array
  mainwin->colInfo.clear();
  i = 0;
  while (i < 4 && chkColEnabled[i]->isChecked())
  {
     mainwin->colInfo.push_back(new CColumnInfo(edtColTitle[i]->text(),
                                                edtColFormat[i]->text().local8Bit(),
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
  const char *oemparams[7] = { edtSndMsg->text().latin1(), edtSndUrl->text().latin1(),
   edtSndChat->text().latin1(), edtSndFile->text().latin1(),
   edtSndNotify->text().latin1(), edtSndSysMsg->text().latin1(),
   edtSndMsgSent->text().latin1() };
  oem->SetParameters(edtSndPlayer->text().latin1(), oemparams);
  ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
  o->SetEnableSave(false);
  o->SetAcceptInAway(chkOEAway->isChecked());
  o->SetAcceptInNA(chkOENA->isChecked());
  o->SetAcceptInOccupied(chkOEOccupied->isChecked());
  o->SetAcceptInDND(chkOEDND->isChecked());
  o->SetEnableSave(true);
  o->SaveLicqInfo();
  gUserManager.DropOwner();
  mainwin->licqDaemon->SetAlwaysOnlineNotify(chkAlwaysOnlineNotify->isChecked());
}


void OptionsDlg::setupFontName(QLineEdit* le, const QFont& font)
{
  QString s;
  if (font == mainwin->defaultFont)
    s = tr("default (%1)").arg(font.rawName());
  else
    s = font.rawName();
  le->setFont(font);
  le->setText(s);
  le->setCursorPosition(0);
}


void OptionsDlg::slot_selectfont()
{
  bool fontOk;
#ifdef USE_KDE
  QFont f = edtFont->font();
  fontOk = (KFontDialog::getFont(f, false, this) == QDialog::Accepted);
#else
  QFont f = QFontDialog::getFont(&fontOk, edtFont->font(), this);
#endif
  if (fontOk) {
    setupFontName(edtFont, f);
    // default might have changed, so update that one as well
    setupFontName(edtEditFont, f);
  }
}

void OptionsDlg::slot_selecteditfont()
{
  bool fontOk;
#ifdef USE_KDE
  QFont f = edtFont->font();
  fontOk = (KFontDialog::getFont(f, false, this) == QDialog::Accepted);
#else
  QFont f = QFontDialog::getFont(&fontOk, edtEditFont->font(), this);
#endif
  if (fontOk) {
    setupFontName(edtEditFont, f);
    ((QWidget*)edtEditFont)->setFont(f, true);
  }
}


// -----------------------------------------------------------------------------

QWidget* OptionsDlg::new_appearance_options()
{
  QWidget* w = new QWidget(this);
  QBoxLayout *lay = new QVBoxLayout(w, 8);
  QBoxLayout *l = new QHBoxLayout(lay, 8);

  QGroupBox *boxMainWin = new QGroupBox(1, Horizontal, tr("Main Window"), w);
  l->addWidget(boxMainWin);

  chkShowGroupIfNoMsg = new QCheckBox(tr("Show group name if no messages"), boxMainWin);
  QWhatsThis::add(chkShowGroupIfNoMsg, tr("Show the name of the current group in the "
                                          "messages label when there are no new messages"));
  chkAutoClose = new QCheckBox(tr("Auto Close Function Window"), boxMainWin);
  QWhatsThis::add(chkAutoClose, tr("Sets the default behavior for auto closing "
                                  "the user function window after a succesful event"));
  chkAutoPopup = new QCheckBox(tr("Auto-Popup Incoming Msg"), boxMainWin);
  QWhatsThis::add(chkAutoPopup, tr("All incoming messages automatically "
                                           "open when received, if we are "
                                           "online (or free for chat)"));
  chkAutoRaise = new QCheckBox(tr("Auto-Raise on Incoming Msg"), boxMainWin);
  QWhatsThis::add(chkAutoRaise, tr("The main window will raise on incoming messages"));
  chkBoldOnMsg = new QCheckBox(tr("Bold Message Label on Incoming Msg"), boxMainWin);
  QWhatsThis::add(chkBoldOnMsg, tr("The message info label will be bold if there are incoming messages"));
  chkManualNewUser = new QCheckBox(tr("Manual \"New User\" group handling"), boxMainWin);
  QWhatsThis::add(chkManualNewUser, tr("If not checked, a user will be automatically "
                                       "removed from \"New User\" group when you first"
                                       "send an event to them"));
  chkSendFromClipboard = new QCheckBox(tr("Check Clipboard For Urls/Files"), boxMainWin);
  QWhatsThis::add(chkSendFromClipboard, tr("When double-clicking on a user to send a message "
   "check for urls/files in the clipboard"));
  chkAutoPosReplyWin = new QCheckBox(tr("Auto Position the Reply Window"), boxMainWin);
  QWhatsThis::add(chkAutoPosReplyWin, tr("Position a new reply window just underneath "
   "the message view window"));
  chkAutoSendThroughServer = new QCheckBox(tr("Auto send through server"), boxMainWin);
  QWhatsThis::add(chkAutoSendThroughServer, tr("Automatically send messages through server "
   "if direct connection fails"));
  chkEnableMainwinMouseMovement = new QCheckBox(tr("Allow dragging main window"), boxMainWin);
  QWhatsThis::add(chkEnableMainwinMouseMovement, tr("Lets you drag around the main window "
   "with your mouse"));

  l = new QVBoxLayout(l);
  boxLocale = new QGroupBox(2, Vertical, tr("Locale"), w);
  lblTrans = new QLabel(tr("Translation:"), boxLocale);
  QWhatsThis::add(lblTrans, tr("Sets which translation table should be used for "
                              "translating characters."));

  cmbTrans = new QComboBox(false, boxLocale);
  QString szTransFilesDir;
  szTransFilesDir.sprintf("%s%s", SHARE_DIR, TRANSLATION_DIR);
  QDir dTrans(szTransFilesDir, QString::null, QDir::Name, QDir::Files | QDir::Readable);
  cmbTrans->insertItem(tr("none"));
  cmbTrans->insertStringList(dTrans.entryList());

  /*QString szLocaleFilesDir;
  szLocaleFilesDir.sprintf("%sqt-gui/locale", SHARE_DIR);
  QDir dLocale(szLocaleFilesDir, "*.qm", QDir::Name, QDir::Files | QDir::Readable);

  if (!dLocale.count())
  {
    cmbLocale->insertItem(tr("NONE"));
    cmbLocale->setEnabled(false);
  }
  else
  {
    cmbLocale->insertItem(tr("Auto"));
    cmbLocale->insertStringList(dLocale.entryList());
  }*/
#if QT_VERSION < 210
  QWidget* dummy_w = new QWidget(boxLocale);
  if (dummy_w);
#endif
  l->addWidget(boxLocale);

  boxDocking = new QGroupBox(1, Horizontal, tr("Docking"), w);
  QButtonGroup *grpDocking = new QButtonGroup(boxDocking);
  grpDocking->setExclusive(true);
  grpDocking->hide();
  chkUseDock = new QCheckBox(tr("Use Dock Icon"), boxDocking);
  QWhatsThis::add(chkUseDock, tr("Controls whether or not the dockable icon should be displayed."));
  rdbDockDefault = new QRadioButton(tr("Default Icon"), boxDocking);
  chkDockFortyEight = new QCheckBox(tr("64 x 48 Dock Icon"), boxDocking);
  QWhatsThis::add(chkDockFortyEight, tr("Selects between the standard 64x64 icon used in the WindowMaker/Afterstep wharf "
                                        "and a shorter 64x48 icon for use in the Gnome/KDE panel."));
  rdbDockThemed = new QRadioButton(tr("Themed Icon"), boxDocking);
  grpDocking->insert(rdbDockDefault);
  grpDocking->insert(rdbDockThemed);
  (void) new QLabel(tr("Theme:"), boxDocking);
  cmbDockTheme = new QComboBox(boxDocking);
  // Set the currently available themes
  QString szDockThemesDir;
  szDockThemesDir.sprintf("%s%s", SHARE_DIR, QTGUI_DIR);
  QDir d(szDockThemesDir, "dock.*", QDir::Name, QDir::Dirs | QDir::Readable);
  QStringList s = d.entryList();
  QStringList::Iterator sit;
  for (sit = s.begin(); sit != s.end(); sit++)
    (*sit).remove(0, 5);
  cmbDockTheme->insertStringList(s);
  connect(rdbDockDefault, SIGNAL(toggled(bool)), chkDockFortyEight, SLOT(setEnabled(bool)));
  connect(rdbDockThemed, SIGNAL(toggled(bool)), cmbDockTheme, SLOT(setEnabled(bool)));
  connect(chkUseDock, SIGNAL(toggled(bool)), SLOT(slot_useDockToggled(bool)));

  l->addWidget(boxDocking);

  l = new QVBoxLayout(lay, 8);
  boxFont = new QGroupBox(3, Horizontal, tr("Font"), w);
  QWhatsThis::add(boxFont, tr("The fonts used"));
  lblFont = new QLabel(tr("Font:"), boxFont);
  edtFont = new QLineEdit(boxFont);
  btnFont = new QPushButton(tr("Select Font"), boxFont);
  QWhatsThis::add(btnFont, tr("Select a font from the system list"));
  connect(btnFont, SIGNAL(clicked()), this, SLOT(slot_selectfont()));

  lblEditFont = new QLabel(tr("Edit Font:"), boxFont);
  edtEditFont = new QLineEdit(boxFont);
  btnEditFont = new QPushButton(tr("Select Font"), boxFont);
  QWhatsThis::add(btnEditFont, tr("Font used in message editor etc."));
  connect(btnEditFont, SIGNAL(clicked()), this, SLOT(slot_selecteditfont()));
  l->addWidget(boxFont);

  return w;
}

void OptionsDlg::slot_useDockToggled(bool b)
{
#ifndef USE_KDE
if (!b)
  {
    cmbDockTheme->setEnabled(false);
    rdbDockDefault->setEnabled(false);
    rdbDockThemed->setEnabled(false);
    chkDockFortyEight->setEnabled(false);
    return;
  }

  // Turned on
  rdbDockDefault->setEnabled(true);
  rdbDockThemed->setEnabled(true);
  if (rdbDockDefault->isChecked())
  {
    cmbDockTheme->setEnabled(false);
    chkDockFortyEight->setEnabled(true);
  }
  else if (rdbDockThemed->isChecked())
  {
    cmbDockTheme->setEnabled(true);
    chkDockFortyEight->setEnabled(false);
  }
#endif
}


// -----------------------------------------------------------------------------

QWidget* OptionsDlg::new_sounds_options()
{
  QWidget* w = new QWidget(this);
  QBoxLayout* lay = new QVBoxLayout(w, 8, 4);

  QWidget* hor = new QHBox(w);
  lay->addWidget(hor);

  chkOnEvents = new QCheckBox(tr("OnEvents Enabled"), hor);
  QWidget* dummy = new QWidget(hor);
  dummy->setFixedSize(50, 1);

  QWhatsThis::add(chkOnEvents, tr("Enable running of \"Command\" when the relevant "
                                 "event occurs."));
  QLabel *lblSndPlayer = new QLabel(tr("Command:"), hor);
  QWhatsThis::add(lblSndPlayer, tr("<p>Command to execute when an event is received.<br>"
                                  "It will be passed the relevant parameters from "
                                  "below.<br>Parameters can contain the following "
                                  "expressions <br> which will be replaced with the relevant "
                                   "information:</p>") + gMainWindow->usprintfHelp);

  edtSndPlayer = new QLineEdit(hor);

  QGroupBox *boxSndEvents = new QGroupBox(2, Horizontal, tr("Parameters"), w);
  lay->addWidget(boxSndEvents);

  QLabel *lblSndMsg = new QLabel(tr("Message:"), boxSndEvents);
  QWhatsThis::add(lblSndMsg, tr("Parameter for received messages"));
  edtSndMsg = new QLineEdit(boxSndEvents);
  QLabel *lblSndUrl = new QLabel(tr("URL:"), boxSndEvents);
  QWhatsThis::add(lblSndUrl, tr("Parameter for received URLs"));
  edtSndUrl = new QLineEdit(boxSndEvents);
  QLabel *lblSndChat = new QLabel(tr("Chat Request:"), boxSndEvents);
  QWhatsThis::add(lblSndChat, tr("Parameter for received chat requests"));
  edtSndChat = new QLineEdit(boxSndEvents);
  QLabel *lblSndFile = new QLabel(tr("File Transfer:"), boxSndEvents);
  QWhatsThis::add(lblSndFile, tr("Parameter for received file transfers"));
  edtSndFile = new QLineEdit(boxSndEvents);
  QLabel *lblSndNotify = new QLabel(tr("Online Notify:"), boxSndEvents);
  QWhatsThis::add(lblSndNotify, tr("Parameter for online notification"));
  edtSndNotify = new QLineEdit(boxSndEvents);
  QLabel *lblSndSysMsg = new QLabel(tr("System Msg:"), boxSndEvents);
  QWhatsThis::add(lblSndSysMsg, tr("Parameter for received system messages"));
  edtSndSysMsg = new QLineEdit(boxSndEvents);
  QLabel *lblSndMsgSent = new QLabel(tr("Message Sent:"), boxSndEvents);
  QWhatsThis::add(lblSndMsgSent, tr("Parameter for sent messages"));
  edtSndMsgSent = new QLineEdit(boxSndEvents);

  QGroupBox *boxAcceptEvents = new QGroupBox(4, Vertical, tr("Accept Modes"), w);
  lay->addWidget(boxAcceptEvents, 1);

  chkOEAway = new QCheckBox(tr("OnEvent in Away"), boxAcceptEvents);
  QWhatsThis::add(chkOEAway, tr("Perform OnEvent command in away mode"));
  chkOENA = new QCheckBox(tr("OnEvent in N/A"), boxAcceptEvents);
  QWhatsThis::add(chkOENA, tr("Perform OnEvent command in not available mode"));
  chkOEOccupied = new QCheckBox(tr("OnEvent in Occupied"), boxAcceptEvents);
  QWhatsThis::add(chkOEOccupied, tr("Perform OnEvent command in occupied mode"));
  chkOEDND = new QCheckBox(tr("OnEvent in DND"), boxAcceptEvents);
  QWhatsThis::add(chkOEDND, tr("Perform OnEvent command in do not disturb mode"));
  chkAlwaysOnlineNotify = new QCheckBox(tr("Online Notify when Logging On"), boxAcceptEvents);
  QWhatsThis::add(chkAlwaysOnlineNotify, tr("Perform the online notify OnEvent "
     "when logging on (this is different from how the Mirabilis client works)"));

  //lay->addStretch(1);

  return w;
}

// -----------------------------------------------------------------------------

QWidget *OptionsDlg::new_network_options()
{
  QWidget *w = new QWidget(this);
  QBoxLayout *lay = new QVBoxLayout(w, 8, 4);

  QGroupBox* gbServer = new QGroupBox(2, QGroupBox::Horizontal, w);
  lay->addWidget(gbServer);

  gbServer->setTitle(tr("Server settings"));

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

  QGroupBox *gbFirewall = new QGroupBox(2, QGroupBox::Horizontal, w);
  lay->addWidget(gbFirewall);
  gbFirewall->setTitle(tr("Firewall"));

  //chkFirewall = new QCheckBox(tr("I am behind a firewall/proxy"), gbFirewall);
  //QPushButton *socks = new QPushButton(tr("SOCKS5 Proxy"), gbFirewall);
  //connect(socks, SIGNAL(clicked()), this, SLOT(slot_socks()));
  //QLabel *lbl = new QLabel(tr("Firewall/Proxy Host:"), gbFirewall);
  //connect(chkFirewall, SIGNAL(toggled(bool)), lbl, SLOT(setEnabled(bool)));
  //edtFirewallHost = new QLineEdit(gbFirewall);
  chkTCPEnabled = new QCheckBox(tr("I can receive direct connections"), gbFirewall);
  QWidget *dummy = new QWidget(gbFirewall);
  if (dummy);
  QLabel *lbl = new QLabel(tr("Port Range:"), gbFirewall);
  //connect(chkFirewall, SIGNAL(toggled(bool)), lbl, SLOT(setEnabled(bool)));
  QWhatsThis::add(lbl, tr("TCP port range for incoming connections."));
  spnPortLow = new QSpinBox(gbFirewall);
  spnPortLow->setRange(0, 0xFFFF);
  spnPortLow->setSpecialValueText(tr("Auto"));
  lbl = new QLabel(tr("\tto"), gbFirewall);
  //connect(chkFirewall, SIGNAL(toggled(bool)), lbl, SLOT(setEnabled(bool)));
  spnPortHigh = new QSpinBox(gbFirewall);
  spnPortHigh->setRange(0, 0xFFFF);
  spnPortHigh->setSpecialValueText(tr("Auto"));

  /*connect(chkFirewall, SIGNAL(toggled(bool)), edtFirewallHost, SLOT(setEnabled(bool)));
  connect(chkFirewall, SIGNAL(toggled(bool)), chkTCPEnabled, SLOT(setEnabled(bool)));
  connect(chkFirewall, SIGNAL(toggled(bool)), spnPortLow, SLOT(setEnabled(bool)));
  connect(chkFirewall, SIGNAL(toggled(bool)), spnPortHigh, SLOT(setEnabled(bool)));*/

  lay->addStretch(1);

  return w;
}

void OptionsDlg::slot_socks()
{
  if (mainwin->licqDaemon->SocksEnabled())
  {
    const char *env = mainwin->licqDaemon->SocksServer();
    if (env == NULL)
      InformUser(this, tr("SOCKS5 support is built in but disabled.\n"
                          "To enable it, set the SOCKS5_SERVER\n"
                          "environment variable to <server>:<port>."));
    else
      InformUser(this, tr("SOCKS5 support is built in and enabled at\n"
                          "\"%1\".").arg(env));
  }
  else
  {
    InformUser(this, tr("To enable socks proxy support, install NEC Socks or Dante\n"
                        "then configure the Licq daemon with \"--enable-socks5\"."));
  }

}

// ---------------------------------------------------------------------------

void OptionsDlg::slot_SARmsg_act(int n)
{
  if (n < 0)
    return;

  SARList &sar = gSARManager.Fetch(cmbSARgroup->currentItem());
  edtSARtext->setText(QString::fromLocal8Bit(sar[n]->AutoResponse()));
  gSARManager.Drop();
}


// ---------------------------------------------------------------------------

void OptionsDlg::slot_SARgroup_act(int n)
{
  if (n < 0)
    return;

  cmbSARmsg->clear();
  SARList &sar = gSARManager.Fetch(n);
  for (SARListIter i = sar.begin(); i != sar.end(); i++)
    cmbSARmsg->insertItem(QString::fromLocal8Bit((*i)->Name()));
  gSARManager.Drop();

  slot_SARmsg_act(0);
}

// ---------------------------------------------------------------------------

void OptionsDlg::slot_SARsave_act()
{
  SARList& sar = gSARManager.Fetch(cmbSARgroup->currentItem());
  delete sar[cmbSARmsg->currentItem()];
  sar[cmbSARmsg->currentItem()] =
    new CSavedAutoResponse(cmbSARmsg->currentText().local8Bit().data(),
                           edtSARtext->text().local8Bit().data());

  gSARManager.Drop();
  gSARManager.Save();

  buildAutoStatusCombos(0);
}


// -----------------------------------------------------------------------------

QWidget* OptionsDlg::new_status_options()
{
  QWidget *w = new QWidget(this);

  QGroupBox *gbStatus = new QGroupBox(tr("Default Auto Response Messages"), w);
  QBoxLayout *lay = new QVBoxLayout(gbStatus, 8);
  lay->addSpacing(16);

  QBoxLayout* l = new QHBoxLayout(lay);

  QLabel* lblcombo1 = new QLabel(tr("Status:"), gbStatus);
  l->addWidget(lblcombo1);

  cmbSARgroup = new QComboBox(false, gbStatus);
  l->addWidget(cmbSARgroup);
  cmbSARgroup->insertItem(tr("Away"), SAR_AWAY);
  cmbSARgroup->insertItem(tr("Not Available"), SAR_NA);
  cmbSARgroup->insertItem(tr("Occupied"), SAR_OCCUPIED);
  cmbSARgroup->insertItem(tr("Do Not Disturb"), SAR_DND);
  cmbSARgroup->insertItem(tr("Free For Chat"), SAR_FFC);
  connect(cmbSARgroup, SIGNAL(activated(int)), this, SLOT(slot_SARgroup_act(int)));

  l->addSpacing(35);

  QLabel* lblcombo2 = new QLabel(tr("Preset slot:"), gbStatus);
  l->addWidget(lblcombo2);
  cmbSARmsg = new QComboBox(true, gbStatus);
  l->addWidget(cmbSARmsg);
  l->addStretch(1);
  cmbSARmsg->setInsertionPolicy(QComboBox::AtCurrent);

  QLabel* lblcombo3 = new QLabel(tr("Text:"), gbStatus);
  lay->addWidget(lblcombo3);
  connect(cmbSARmsg, SIGNAL(activated(int)), this, SLOT(slot_SARmsg_act(int)));

  l = new QHBoxLayout(lay);
  edtSARtext = new MLEditWrap(true, gbStatus);
  l->addWidget(edtSARtext);

  QPushButton* btnSaveIt = new QPushButton(tr("Save"), gbStatus);
  btnSaveIt->setMinimumWidth(75);
  l->addWidget(btnSaveIt);
  connect(btnSaveIt, SIGNAL(clicked()), this, SLOT(slot_SARsave_act()));

  lay->addStretch(1);

  slot_SARgroup_act(SAR_AWAY);

  QGroupBox* gbAuto = new QGroupBox(2, QGroupBox::Horizontal, w);
  gbAuto->setTitle(tr("Startup"));

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
  QWidget *dummy = new QWidget(gbAuto);
  chkAutoLogonInvisible = new QCheckBox(tr("Invisible"), gbAuto);
  dummy->setMinimumHeight(chkAutoLogonInvisible->sizeHint().height()+10);

  lblAutoAway = new QLabel(tr("Auto Away:"), gbAuto);
  QWhatsThis::add(lblAutoAway, tr("Number of minutes of inactivity after which to "
                                 "automatically be marked \"away\".  Set to \"0\" "
                                 "to disable."));
  spnAutoAway = new QSpinBox(gbAuto);
  spnAutoAway->setSpecialValueText(tr("Never"));
  lblAutoNa = new QLabel(tr("Auto N/A:"), gbAuto);
  QWhatsThis::add(lblAutoNa, tr("Number of minutes of inactivity after which to "
                               "automatically be marked \"not available\".  Set to \"0\" "
                               "to disable."));
  spnAutoNa = new QSpinBox(gbAuto);
  spnAutoNa->setSpecialValueText(tr("Never"));
  lblAutoOffline = new QLabel(tr("Auto Offline:"), gbAuto);
  QWhatsThis::add(lblAutoOffline, tr("Number of minutes of inactivity after which to "
                               "automatically go offline.  Set to \"0\" "
                               "to disable."));
  spnAutoOffline = new QSpinBox(gbAuto);
  spnAutoOffline->setSpecialValueText(tr("Never"));

  QVBoxLayout *vlay = new QVBoxLayout(w, 8, 8);
  vlay->addWidget(gbStatus);
  vlay->addWidget(gbAuto);
  vlay->addStretch(1);

  return w;
}


// -----------------------------------------------------------------------------

QWidget* OptionsDlg::new_column_options()
{
  QWidget* w = new QWidget(this);

  QGroupBox *grp = new QGroupBox (tr("Column Configuration"), w);

  QGridLayout *lay1 = new QGridLayout(grp, 7, 5, 10);

  lblColTitle = new QLabel (tr("Title"), grp);
  QWhatsThis::add(lblColTitle, tr("The string which will appear in the list box column header"));
  lblColFormat = new QLabel (tr("Format"), grp);
  QWhatsThis::add(lblColFormat, QStyleSheet::convertFromPlainText(tr("The format string used to define what will "
                                   "appear in each column.\n"
                                   "The following parameters can be used:\n")) + gMainWindow->usprintfHelp);
  lblColWidth = new QLabel (tr("Width"), grp);
  QWhatsThis::add(lblColWidth, tr("The width of the column"));
  lblColAlign = new QLabel(tr("Alignment"), grp);
  QWhatsThis::add(lblColAlign, tr("The alignment of the column"));

  lay1->addWidget(lblColTitle, 1, 1);
  lay1->addWidget(lblColFormat, 1, 2);
  lay1->addWidget(lblColWidth, 1, 3);
  lay1->addWidget(lblColAlign, 1, 4);

  for (unsigned short i = 0; i < 4; i++)
  {
    chkColEnabled[i] = new QCheckBox(tr("Column %1").arg(i), grp);
    edtColTitle[i] = new QLineEdit(grp);
    QWhatsThis::add(edtColTitle[i], QWhatsThis::textFor(lblColTitle));
    edtColFormat[i] = new QLineEdit(grp);
    QWhatsThis::add(edtColFormat[i], QWhatsThis::textFor(lblColFormat));
    spnColWidth[i] = new QSpinBox(grp);
    QWhatsThis::add(spnColWidth[i], QWhatsThis::textFor(lblColWidth));
    spnColWidth[i]->setRange(0, 2048);
    cmbColAlign[i] = new QComboBox(grp);
    QWhatsThis::add(cmbColAlign[i], QWhatsThis::textFor(lblColAlign));
    cmbColAlign[i]->insertItem(tr("Left"));
    cmbColAlign[i]->insertItem(tr("Right"));
    cmbColAlign[i]->insertItem(tr("Center"));
    lay1->addWidget(chkColEnabled[i], 2+i, 0);
    lay1->addWidget(edtColTitle[i], 2+i, 1);
    lay1->addWidget(edtColFormat[i], 2+i, 2);
    lay1->addWidget(spnColWidth[i], 2+i, 3);
    lay1->addWidget(cmbColAlign[i], 2+i, 4);
    connect(chkColEnabled[i], SIGNAL(toggled(bool)), this, SLOT(colEnable(bool)));
  }
  lay1->activate();

  boxUserWin = new QGroupBox(2, Horizontal, tr("Options"), w);

  chkGridLines = new QCheckBox(tr("Show Grid Lines"), boxUserWin);
  QWhatsThis::add(chkGridLines, tr("Draw boxes around each square in the user list"));
  chkHeader = new QCheckBox(tr("Show Column Headers"), boxUserWin);
  QWhatsThis::add(chkHeader, tr("Turns on or off the display of headers above "
     "each column in the user list"));
  chkShowDividers = new QCheckBox(tr("Show User Dividers"), boxUserWin);
  QWhatsThis::add(chkShowDividers, tr("Show the \"--online--\" and \"--offline--\" bars "
     "in the contact list"));
  chkSortByStatus = new QCheckBox(tr("Sort Online Users by Status"), boxUserWin);
  QWhatsThis::add(chkSortByStatus, tr("Sort all online users by their actual status"));
  chkAlwaysShowONU = new QCheckBox(tr("Always show online notify users"), boxUserWin);
  QWhatsThis::add(chkAlwaysShowONU, tr("Show online notify users who are offline even "
     "when offline users are hidden."));
  chkTransparent = new QCheckBox(tr("Transparent when possible"), boxUserWin);
  QWhatsThis::add(chkTransparent, tr("Make the user window transparent when there "
     "is no scroll bar"));
  chkFontStyles = new QCheckBox(tr("Use Font Styles"), boxUserWin);
  QWhatsThis::add(chkFontStyles, tr("Use italics and bold in the user list to "
     "indicate special characteristics such as online notify and visible list"));
  chkFlashAll = new QCheckBox(tr("Flash Events"), boxUserWin);
  QWhatsThis::add(chkFlashAll, tr("All incoming events will flash"));
  chkFlashUrgent = new QCheckBox(tr("Flash Urgent Events"), boxUserWin);
  QWhatsThis::add(chkFlashUrgent, tr("Only urgent events will flash"));
  chkScrollBar = new QCheckBox(tr("Allow scroll bar"), boxUserWin);
  QWhatsThis::add(chkScrollBar, tr("Allow the vertical scroll bar in the user list"));
  chkShowExtIcons = new QCheckBox(tr("Show Extended Icons"), boxUserWin);
  QWhatsThis::add(chkShowExtIcons, tr("Show birthday, invisible, and custom auto response "
     "icons to the right of users in the list"));

  QHBox *hlay = new QHBox(boxUserWin);
  lblFrameStyle = new QLabel(tr("Frame Style: "), hlay);
  edtFrameStyle = new QLineEdit(hlay);
  QWhatsThis::add(lblFrameStyle, tr("Override the skin setting for the frame "
                                    "style of the user window:\n"
                                    "   0 (No frame), 1 (Box), 2 (Panel), 3 (WinPanel)\n"
                                    " + 16 (Plain), 32 (Raised), 48 (Sunken)\n"
                                    " + 240 (Shadow)"));
  edtFrameStyle->setValidator(new QIntValidator(edtFrameStyle));

  chkSysBack = new QCheckBox(tr("Use System Background Color"), boxUserWin);

  hlay = new QHBox(boxUserWin);
  lblHotKey = new QLabel(tr("Hot key: "), hlay);
  edtHotKey = new QLineEdit(hlay);
  QWhatsThis::add(lblHotKey, tr("Hotkey pops up the next pending message\n"
                                "Enter the hotkey literally, like \"shift+f10\", "
                                "\"none\" for disabling\n"
                                "changes here require a Restart to take effect!\n"));

  QVBoxLayout *g_main = new QVBoxLayout(w, 10, 5);
  g_main->addWidget(grp);
  g_main->addWidget(boxUserWin);
  g_main->addStretch(1);

  return w;
}


// -----------------------------------------------------------------------------

QWidget* OptionsDlg::new_misc_options()
{
  QWidget* w = new QWidget(this);
  QBoxLayout* lay = new QVBoxLayout(w, 8, 4);

  boxExtensions = new QGroupBox(2, Horizontal, tr("Extensions"), w);
  lay->addWidget(boxExtensions);

  lblUrlViewer = new QLabel(tr("Url Viewer:"), boxExtensions);
  QWhatsThis::add(lblUrlViewer, tr("The command to run to view a URL.  Will be passed the URL "
                                  "as a parameter."));
  edtUrlViewer = new QLineEdit(boxExtensions);
  lblTerminal = new QLabel(tr("Terminal:"), boxExtensions);
  edtTerminal = new QLineEdit(tr("Terminal:"), boxExtensions);
  QWhatsThis::add(edtTerminal, tr("The command to run to start your terminal program."));

  boxParanoia = new QGroupBox(3, Vertical, tr("Paranoia"), w);
  lay->addWidget(boxParanoia);

  chkIgnoreNewUsers = new QCheckBox(tr("Ignore New Users"), boxParanoia);
  QWhatsThis::add(chkIgnoreNewUsers, tr("Determines if new users are automatically added "
                                      "to your list or must first request authorization."));
  chkIgnoreMassMsg = new QCheckBox(tr("Ignore Mass Messages"), boxParanoia);
  QWhatsThis::add(chkIgnoreMassMsg, tr("Determines if mass messages are ignored or not."));

  chkIgnoreWebPanel = new QCheckBox(tr("Ignore Web Panel"), boxParanoia);
  QWhatsThis::add(chkIgnoreWebPanel, tr("Determines if web panel messages are ignored or not."));

  chkIgnoreEmailPager = new QCheckBox(tr("Ignore Email Pager"), boxParanoia);
  QWhatsThis::add(chkIgnoreEmailPager, tr("Determines if email pager messages are ignored or not."));

#if QT_VERSION < 210
  QWidget* dummy_w= new QWidget(boxParanoia);
  dummy_w->setMinimumHeight(10);
#endif

  QWidget* boxAutoStatus = new QGroupBox(2, Horizontal, tr("Auto Away Messages"), w);
  lay->addWidget(boxAutoStatus);
  new QLabel(tr("Away:"), boxAutoStatus);
  cmbAutoAwayMess = new QComboBox(boxAutoStatus);
  new QLabel(tr("N/A:"), boxAutoStatus);
  cmbAutoNAMess = new QComboBox(boxAutoStatus);

  buildAutoStatusCombos(1);

  lay->addStretch(1);
  lay->activate();

  return w;
}

// -----------------------------------------------------------------------------
// I want to rebuild the Combo boxes when user saves the
// messages therefor we have a separate function.
void OptionsDlg::buildAutoStatusCombos(bool firstTime)
{
  int selectedNA, selectedAway;

  //Save selection (or get first selection)
  if (firstTime) {
    selectedAway = mainwin->autoAwayMess;
    selectedNA   = mainwin->autoNAMess;
  } else {
    selectedAway = cmbAutoAwayMess->currentItem();
    selectedNA   = cmbAutoNAMess->currentItem();
  }

  cmbAutoAwayMess->clear();
  cmbAutoAwayMess->insertItem(tr("Previous Message"),0);
  SARList &sara = gSARManager.Fetch(SAR_AWAY);
  for (unsigned i = 0; i < sara.size(); i++)
    cmbAutoAwayMess->insertItem(sara[i]->Name(),i+1);
  gSARManager.Drop();

  cmbAutoNAMess->clear();
  cmbAutoNAMess->insertItem(tr("Previous Message"),0);
  SARList &sarn = gSARManager.Fetch(SAR_NA);
  for (unsigned i = 0; i < sarn.size(); i++)
    cmbAutoNAMess->insertItem(sarn[i]->Name(),i+1);
  gSARManager.Drop();

  cmbAutoAwayMess->setCurrentItem(selectedAway);
  cmbAutoNAMess->setCurrentItem(selectedNA);
}


// -----------------------------------------------------------------------------

#include "optionsdlg.moc"
