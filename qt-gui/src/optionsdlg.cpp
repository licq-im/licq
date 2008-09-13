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
#include <qcolordialog.h>
#include <qtextcodec.h>
#ifdef USE_KDE
#include <kapp.h>
#include <kfontdialog.h>
#include <kurlrequester.h>
#else
#include <qapplication.h>
#include <qfontdialog.h>
#endif

#include "ewidgets.h"
#include "optionsdlg.h"
#include "licq_log.h"
#include "licq_constants.h"
#include "mainwin.h"
#include "mledit.h"
#include "licq_icqd.h"
#include "userbox.h"
#include "licq_sar.h"
#include "wharf.h"
#include "skin.h"
#include "usercodec.h"


OptionsDlg::OptionsDlg(CMainWindow *_mainwin, tabs settab, QWidget *parent)
  : LicqDialog(parent, "OptionsDialog", false, WStyle_ContextHelp | WDestructiveClose)
{
  setCaption(tr("Licq Options"));

  mainwin = _mainwin;

  QBoxLayout* top_lay = new QVBoxLayout(this, 4);

  tabw = new CETabWidget(this);
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
  tab[6] = new_chat_options();
  //  tab[6] = new_phone_options();

  tabw->addTab(tab[0], tr("General"));
  tabw->addTab(tab[1], tr("Contact List"));
  tabw->addTab(tab[2], tr("OnEvent"));
  tabw->addTab(tab[3], tr("Network"));
  tabw->addTab(tab[4], tr("Status"));
  tabw->addTab(tab[5], tr("Miscellaneous"));
  tabw->addTab(tab[6], tr("Message Display"));
  //  tabw->addTab(tab[6], tr("Phone"));

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

  chkSSList->setChecked(mainwin->licqDaemon->UseServerContactList());
  chkGridLines->setChecked(mainwin->m_bGridLines);
  chkFontStyles->setChecked(mainwin->m_bFontStyles);
  chkShowExtIcons->setChecked(mainwin->m_bShowExtendedIcons);
  chkHeader->setChecked(mainwin->m_bShowHeader);
  chkShowDividers->setChecked(mainwin->m_bShowDividers);
  cmbSortBy->setCurrentItem(mainwin->m_nSortByStatus);
  chkAlwaysShowONU->setChecked(mainwin->m_bAlwaysShowONU);
  chkShowGroupIfNoMsg->setChecked(mainwin->m_bShowGroupIfNoMsg);
  chkAutoClose->setChecked(mainwin->m_bAutoClose);
  chkTransparent->setChecked(mainwin->skin->frame.transparent);
  chkShowUserIcons->setChecked(mainwin->m_bShowUserIcons);
  chkScrollBar->setChecked(mainwin->m_bScrollBar);
  chkFlashUrgent->setChecked(mainwin->m_nFlash == FLASH_URGENT || mainwin->m_nFlash == FLASH_ALL);
  chkFlashAll->setChecked(mainwin->m_nFlash == FLASH_ALL);
  chkAutoPopup->setChecked(mainwin->m_bAutoPopup);
  chkAutoRaise->setChecked(mainwin->m_bAutoRaise);
  chkHidden->setChecked(mainwin->m_bHidden);
  chkBoldOnMsg->setChecked(mainwin->m_bBoldOnMsg);
  chkManualNewUser->setChecked(mainwin->m_bManualNewUser);
  edtFrameStyle->setText(QString::number((int)mainwin->skin->frame.frameStyle));
  edtHotKey->setText(mainwin->m_MsgAutopopupKey);
  chkSysBack->setChecked(mainwin->m_bSystemBackground);
  chkSendFromClipboard->setChecked(mainwin->m_bSendFromClipboard);
  chkMsgChatView->setChecked(mainwin->m_bMsgChatView);
  cmbChatDateFormat->lineEdit()->setText(mainwin->m_chatDateFormat);
  chkChatVertSpacing->setChecked(mainwin->m_chatVertSpacing);
  chkChatLineBreak->setChecked(mainwin->m_chatAppendLineBreak);
  cmbChatStyle->setCurrentItem(mainwin->m_chatMsgStyle);
  cmbHistStyle->setCurrentItem(mainwin->m_histMsgStyle);
  chkHistVertSpacing->setChecked(mainwin->m_histVertSpacing);
  cmbHistDateFormat->lineEdit()->setText(mainwin->m_histDateFormat);
  btnColorRcv->setPaletteBackgroundColor(mainwin->m_colorRcv);
  btnColorSnt->setPaletteBackgroundColor(mainwin->m_colorSnt);
  btnColorRcvHistory->setPaletteBackgroundColor(mainwin->m_colorRcvHistory);
  btnColorSntHistory->setPaletteBackgroundColor(mainwin->m_colorSntHistory);
  btnColorNotice->setPaletteBackgroundColor(mainwin->m_colorNotice);
  /*btnColorTabLabel->setPaletteBackgroundColor(mainwin->m_colorTab);*/
  btnColorTypingLabel->setPaletteBackgroundColor(mainwin->m_colorTabTyping);
  btnColorChatBkg->setPaletteBackgroundColor(mainwin->m_colorChatBkg);
  slot_refresh_msgViewer();

  if (mainwin->m_bMsgChatView)
  {
    chkTabbedChatting->setChecked(mainwin->m_bTabbedChatting);
    chkShowHistory->setChecked(mainwin->m_bShowHistory);
    chkShowNotices->setChecked(mainwin->m_showNotices);
  }
  else
  {
    chkTabbedChatting->setEnabled(false);
    chkShowHistory->setEnabled(false);
    chkShowNotices->setChecked(false);
  }
  chkSendTN->setChecked(mainwin->licqDaemon->SendTypingNotification());
  chkAutoPosReplyWin->setChecked(mainwin->m_bAutoPosReplyWin);
  chkFlashTaskbar->setChecked(mainwin->m_bFlashTaskbar);
  chkAutoSendThroughServer->setChecked(mainwin->m_bAutoSendThroughServer);
  chkEnableMainwinMouseMovement->setChecked(mainwin->m_bEnableMainwinMouseMovement);
  chkMainWinSticky->setChecked(mainwin->m_bMainWinSticky);
  chkMsgWinSticky->setChecked(mainwin->m_bMsgWinSticky);
  chkSingleLineChatMode->setChecked(mainwin->m_bSingleLineChatMode);
  chkUseDoubleReturn->setChecked(MLEditWrap::useDoubleReturn);
  popPicture->setChecked(mainwin->m_bPopPicture);
  popAlias->setChecked(mainwin->m_bPopAlias);
  popName->setChecked(mainwin->m_bPopName);
  popEmail->setChecked(mainwin->m_bPopEmail);
  popPhone->setChecked(mainwin->m_bPopPhone);
  popFax->setChecked(mainwin->m_bPopFax);
  popCellular->setChecked(mainwin->m_bPopCellular);
  popIP->setChecked(mainwin->m_bPopIP);
  popLastOnline->setChecked(mainwin->m_bPopLastOnline);
  popOnlineSince->setChecked(mainwin->m_bPopOnlineSince);
  popIdleTime->setChecked(mainwin->m_bPopIdleTime);
  popLocalTime->setChecked(mainwin->m_bPopLocalTime);
  popID->setChecked(mainwin->m_bPopID);

  switch(mainwin->m_nDockMode)
  {
    case DockNone:
      chkUseDock->setChecked(false);
      cmbDockTheme->setEnabled(false);
      rdbDockDefault->setEnabled(false);
      rdbDockThemed->setEnabled(false);
      rdbDockSmall->setChecked(false);
      rdbDockSmall->setEnabled(false);
      chkDockFortyEight->setEnabled(false);
      chkHidden->setEnabled(false);
      chkHidden->setChecked(false);
      break;
#ifndef USE_KDE
    case DockSmall:
      chkUseDock->setChecked(true);
      rdbDockSmall->setChecked(true);
      chkDockFortyEight->setEnabled(false);
      cmbDockTheme->setEnabled(false);
      chkHidden->setEnabled(true);
      break;
    case DockDefault:
      chkUseDock->setChecked(true);
      rdbDockDefault->setChecked(true);
      chkDockFortyEight->setChecked( ((IconManager_Default *)mainwin->licqIcon)->FortyEight());
      chkDockFortyEight->setEnabled(true);
      cmbDockTheme->setEnabled(false);
      chkHidden->setEnabled(true);
      break;
    case DockThemed:
      chkUseDock->setChecked(true);
      rdbDockThemed->setChecked(true);
      cmbDockTheme->setEnabled(true);
      chkDockFortyEight->setEnabled(false);
      chkHidden->setEnabled(true);
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
    case DockSmall:
    case DockDefault:
    case DockThemed:
      chkUseDock->setChecked(true);
      cmbDockTheme->setEnabled(false);
      rdbDockDefault->setEnabled(false);
      rdbDockThemed->setEnabled(false);
      rdbDockSmall->setChecked(true);
      rdbDockSmall->setEnabled(false);
      chkDockFortyEight->setEnabled(false);
      chkHidden->setEnabled(true);
      break;
#endif
  }
  if (mainwin->m_bDisableDockIcon)
    boxDocking->setEnabled(false);

  edtICQServer->setText(QString(mainwin->licqDaemon->ICQServer()));
  chkFirewall->setChecked(mainwin->licqDaemon->Firewall());
  chkTCPEnabled->setChecked(mainwin->licqDaemon->TCPEnabled());
  spnPortLow->setValue(mainwin->licqDaemon->TCPPortsLow());
  spnPortHigh->setValue(mainwin->licqDaemon->TCPPortsHigh());

  if (!mainwin->licqDaemon->Firewall())
  {
    chkTCPEnabled->setEnabled(false);
    spnPortLow->setEnabled(false);
    spnPortHigh->setEnabled(false);
  }
  else
  {
    if (!mainwin->licqDaemon->TCPEnabled())
    {
      spnPortLow->setEnabled(false);
      spnPortHigh->setEnabled(false);
    }
  }

  chkProxyEnabled->setChecked(mainwin->licqDaemon->ProxyEnabled());
  cmbProxyType->setCurrentItem(mainwin->licqDaemon->ProxyType() - 1);
  edtProxyHost->setText(QString(mainwin->licqDaemon->ProxyHost()));
  spnProxyPort->setValue(mainwin->licqDaemon->ProxyPort());
  chkProxyAuthEnabled->setChecked(mainwin->licqDaemon->ProxyAuthEnabled());
  edtProxyLogin->setText(QString(mainwin->licqDaemon->ProxyLogin()));
  edtProxyPasswd->setText(QString(mainwin->licqDaemon->ProxyPasswd()));

  // Set server port after chkProxyEnabled as it will trigger slot_useProxy
  // which overwrites server port field.
  spnICQServerPort->setValue(mainwin->licqDaemon->ICQServerPort());

  chkReconnectAfterUinClash->setChecked(mainwin->licqDaemon->ReconnectAfterUinClash());

  if (!mainwin->licqDaemon->ProxyEnabled())
  {
    cmbProxyType->setEnabled(false);
    edtProxyHost->setEnabled(false);
    spnProxyPort->setEnabled(false);
    chkProxyAuthEnabled->setEnabled(false);
    edtProxyLogin->setEnabled(false);
    edtProxyPasswd->setEnabled(false);
  } else if (!mainwin->licqDaemon->ProxyAuthEnabled())
  {
    edtProxyLogin->setEnabled(false);
    edtProxyPasswd->setEnabled(false);
  }

  spnAutoAway->setValue(mainwin->autoAwayTime);
  spnAutoNa->setValue(mainwin->autoNATime);
  spnAutoOffline->setValue(mainwin->autoOfflineTime);
  cmbAutoLogon->setCurrentItem(mainwin->m_nAutoLogon % 10);
  chkAutoLogonInvisible->setChecked(mainwin->m_nAutoLogon >= 10);


  chkIgnoreNewUsers->setChecked(mainwin->licqDaemon->Ignore(IGNORE_NEWUSERS));
  chkIgnoreMassMsg->setChecked(mainwin->licqDaemon->Ignore(IGNORE_MASSMSG));
  chkIgnoreWebPanel->setChecked(mainwin->licqDaemon->Ignore(IGNORE_WEBPANEL));
  chkIgnoreEmailPager->setChecked(mainwin->licqDaemon->Ignore(IGNORE_EMAILPAGER));

  chkAutoUpdateInfo->setChecked(mainwin->licqDaemon->AutoUpdateInfo());
  chkAutoUpdateInfoPlugins->setChecked(mainwin->licqDaemon
                                              ->AutoUpdateInfoPlugins());
  chkAutoUpdateStatusPlugins->setChecked(mainwin->licqDaemon
                                                ->AutoUpdateStatusPlugins());

  // plugins tab
  cmbUrlViewer->setCurrentText(mainwin->licqDaemon->getUrlViewer() == NULL ?
             DEFAULT_URL_VIEWER : QString(mainwin->licqDaemon->getUrlViewer()));
  edtTerminal->setText(mainwin->licqDaemon->Terminal() == NULL ?
                       tr("none") : QString(mainwin->licqDaemon->Terminal()));

  cmbDefaultEncoding->setCurrentItem(0);
  // first combo box item is the locale encoding, so we skip it
  for (int i = 1; i < cmbDefaultEncoding->count(); i++)
  {
    if (UserCodec::encodingForName(cmbDefaultEncoding->text(i)) == mainwin->m_DefaultEncoding)
    {
       cmbDefaultEncoding->setCurrentItem(i);
       break;
    }
  }
  chkShowAllEncodings->setChecked(mainwin->m_bShowAllEncodings);

   // set up the columns stuff
   int i;
   for (i = 0; i < int(mainwin->colInfo.size()); i++)
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
   edtSndPlayer->setURL(oem->command());
   edtSndMsg->setURL(oem->parameter(ON_EVENT_MSG));
   edtSndUrl->setURL(oem->parameter(ON_EVENT_URL));
   edtSndChat->setURL(oem->parameter(ON_EVENT_CHAT));
   edtSndFile->setURL(oem->parameter(ON_EVENT_FILE));
   edtSndNotify->setURL(oem->parameter(ON_EVENT_NOTIFY));
   edtSndSysMsg->setURL(oem->parameter(ON_EVENT_SYSMSG));
   edtSndMsgSent->setURL(oem->parameter(ON_EVENT_MSGSENT));
   oem->Unlock();

   //TODO make general for all plugins
   ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
   if (o)
   {
     chkOEAway->setChecked(o->AcceptInAway());
     chkOENA->setChecked(o->AcceptInNA());
     chkOEOccupied->setChecked(o->AcceptInOccupied());
     chkOEDND->setChecked(o->AcceptInDND());
     gUserManager.DropOwner(o);
   }
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
    f.fromString(edtEditFont->text());

  delete MLEditWrap::editFont;
  MLEditWrap::editFont = new QFont(f);

  f = mainwin->defaultFont;
  if(edtFont->text().find(tr("default"), 0, false) != 0)
    f.fromString(edtFont->text());
  qApp->setFont(f, true);

  mainwin->m_bGridLines = chkGridLines->isChecked();
  mainwin->m_bFontStyles = chkFontStyles->isChecked();
  mainwin->m_bShowExtendedIcons = chkShowExtIcons->isChecked();
  mainwin->m_bShowHeader = chkHeader->isChecked();
  mainwin->m_bShowDividers = chkShowDividers->isChecked();
  mainwin->m_nSortByStatus = cmbSortBy->currentItem();
  mainwin->m_bAlwaysShowONU = chkAlwaysShowONU->isChecked();
  mainwin->m_bShowGroupIfNoMsg = chkShowGroupIfNoMsg->isChecked();
  mainwin->m_bAutoClose = chkAutoClose->isChecked();
  mainwin->m_bAutoPopup = chkAutoPopup->isChecked();
  mainwin->m_bAutoRaise = chkAutoRaise->isChecked();
  mainwin->m_bHidden = chkHidden->isChecked();
  mainwin->m_bBoldOnMsg = chkBoldOnMsg->isChecked();
  mainwin->m_bManualNewUser = chkManualNewUser->isChecked();
  mainwin->m_bScrollBar = chkScrollBar->isChecked();
  mainwin->m_nFlash = chkFlashAll->isChecked() ? FLASH_ALL :
                      ( chkFlashUrgent->isChecked() ? FLASH_URGENT : FLASH_NONE );
  mainwin->skin->frame.transparent = chkTransparent->isChecked();
  mainwin->m_bShowUserIcons = chkShowUserIcons->isChecked();
  mainwin->skin->frame.frameStyle = edtFrameStyle->text().toUShort();
  mainwin->m_MsgAutopopupKey = edtHotKey->text();
  mainwin->m_bSystemBackground = chkSysBack->isChecked();
  mainwin->m_bSendFromClipboard = chkSendFromClipboard->isChecked();
  mainwin->m_bMsgChatView = chkMsgChatView->isChecked();
  mainwin->m_chatVertSpacing = chkChatVertSpacing->isChecked();
  mainwin->m_chatAppendLineBreak = chkChatLineBreak->isChecked();
  mainwin->m_chatMsgStyle = cmbChatStyle->currentItem();
  mainwin->m_chatDateFormat = cmbChatDateFormat->currentText();
  mainwin->m_histMsgStyle = cmbHistStyle->currentItem();
  mainwin->m_histVertSpacing = chkHistVertSpacing->isChecked();
  mainwin->m_histDateFormat = cmbHistDateFormat->currentText();
  mainwin->m_colorRcv = btnColorRcv->paletteBackgroundColor();
  mainwin->m_colorSnt = btnColorSnt->paletteBackgroundColor();
  mainwin->m_colorRcvHistory = btnColorRcvHistory->paletteBackgroundColor();
  mainwin->m_colorSntHistory = btnColorSntHistory->paletteBackgroundColor();
  mainwin->m_colorNotice = btnColorNotice->paletteBackgroundColor();
  /*mainwin->m_colorTab = btnColorTabLabel->paletteBackgroundColor();*/
  mainwin->m_colorTabTyping = btnColorTypingLabel->paletteBackgroundColor();
  mainwin->m_colorChatBkg = btnColorChatBkg->paletteBackgroundColor();

  mainwin->m_bTabbedChatting = chkTabbedChatting->isChecked();
  mainwin->m_bShowHistory = chkShowHistory->isChecked();
  mainwin->m_showNotices = chkShowNotices->isChecked();
  mainwin->m_bAutoPosReplyWin = chkAutoPosReplyWin->isChecked();
  mainwin->m_bFlashTaskbar = chkFlashTaskbar->isChecked();
  mainwin->m_bAutoSendThroughServer = chkAutoSendThroughServer->isChecked();
  mainwin->m_bEnableMainwinMouseMovement = chkEnableMainwinMouseMovement->isChecked();

  mainwin->m_bMainWinSticky = chkMainWinSticky->isChecked();
  mainwin->changeMainWinSticky(chkMainWinSticky->isChecked());
  mainwin->m_bMsgWinSticky = chkMsgWinSticky->isChecked();
  mainwin->m_bSingleLineChatMode = chkSingleLineChatMode->isChecked();
  MLEditWrap::useDoubleReturn = chkUseDoubleReturn->isChecked();

  mainwin->m_bPopPicture = popPicture->isChecked();
  mainwin->m_bPopAlias = popAlias->isChecked();
  mainwin->m_bPopName = popName->isChecked();
  mainwin->m_bPopEmail = popEmail->isChecked();
  mainwin->m_bPopPhone = popPhone->isChecked();
  mainwin->m_bPopFax = popFax->isChecked();
  mainwin->m_bPopCellular = popCellular->isChecked();
  mainwin->m_bPopIP = popIP->isChecked();
  mainwin->m_bPopLastOnline = popLastOnline->isChecked();
  mainwin->m_bPopOnlineSince = popOnlineSince->isChecked();
  mainwin->m_bPopIdleTime = popIdleTime->isChecked();
  mainwin->m_bPopLocalTime = popLocalTime->isChecked();
  mainwin->m_bPopID = popID->isChecked();


#ifndef USE_KDE
  if (chkUseDock->isChecked() &&
      (rdbDockDefault->isChecked() || rdbDockThemed->isChecked() ||
       rdbDockSmall->isChecked()) )
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
        if (mainwin->licqIcon)
        {
          mainwin->licqIcon->close();
          delete mainwin->licqIcon;
        }
        mainwin->licqIcon = new IconManager_Default(mainwin, mainwin->mnuSystem, chkDockFortyEight->isChecked());
        mainwin->m_nDockMode = DockDefault;
      }
    }
    else if (rdbDockThemed->isChecked())
    {
      if (mainwin->m_nDockMode != DockThemed)
      {
        if (mainwin->licqIcon)
        {
          mainwin->licqIcon->close();
          delete mainwin->licqIcon; 
        }
        mainwin->licqIcon = new IconManager_Themed(mainwin, mainwin->mnuSystem, cmbDockTheme->currentText().local8Bit());
        mainwin->m_nDockMode = DockThemed;
      }
      else if ( ((IconManager_Themed *)mainwin->licqIcon)->Theme() != cmbDockTheme->currentText() )
      {
        ((IconManager_Themed *)mainwin->licqIcon)->SetTheme(cmbDockTheme->currentText().local8Bit());
      }
    }
    else if (rdbDockSmall->isChecked())
    {
      if (mainwin->m_nDockMode != DockSmall)
      {
        if (mainwin->licqIcon)
        {
          mainwin->licqIcon->close();
          delete mainwin->licqIcon;
        }
        mainwin->licqIcon = new IconManager_KDEStyle(mainwin, mainwin->mnuSystem);
        mainwin->m_nDockMode = DockSmall;
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
    if (mainwin->licqIcon)
      delete mainwin->licqIcon;
    mainwin->licqIcon = NULL;
    mainwin->m_nDockMode = DockNone;
  }

  mainwin->licqDaemon->SetSendTypingNotification(chkSendTN->isChecked());
  mainwin->licqDaemon->SetUseServerContactList(chkSSList->isChecked());
  mainwin->licqDaemon->SetICQServer(edtICQServer->text().local8Bit());
  mainwin->licqDaemon->SetICQServerPort(spnICQServerPort->value());
  mainwin->licqDaemon->SetTCPPorts(spnPortLow->value(), spnPortHigh->value());
  mainwin->licqDaemon->SetTCPEnabled(chkTCPEnabled->isChecked());
  mainwin->licqDaemon->SetFirewall(chkFirewall->isChecked());
  mainwin->licqDaemon->SetProxyEnabled(chkProxyEnabled->isChecked());
  mainwin->licqDaemon->SetProxyType(cmbProxyType->currentItem() + 1);
  mainwin->licqDaemon->SetProxyHost(edtProxyHost->text().local8Bit());
  mainwin->licqDaemon->SetProxyPort(spnProxyPort->value());
  mainwin->licqDaemon->SetProxyAuthEnabled(chkProxyAuthEnabled->isChecked());
  mainwin->licqDaemon->SetProxyLogin(edtProxyLogin->text().local8Bit());
  mainwin->licqDaemon->SetProxyPasswd(edtProxyPasswd->text().local8Bit());

  mainwin->licqDaemon->setReconnectAfterUinClash(chkReconnectAfterUinClash->isChecked());

  mainwin->licqDaemon->SetIgnore(IGNORE_NEWUSERS, chkIgnoreNewUsers->isChecked());
  mainwin->licqDaemon->SetIgnore(IGNORE_MASSMSG, chkIgnoreMassMsg->isChecked());
  mainwin->licqDaemon->SetIgnore(IGNORE_WEBPANEL, chkIgnoreWebPanel->isChecked());
  mainwin->licqDaemon->SetIgnore(IGNORE_EMAILPAGER, chkIgnoreEmailPager->isChecked());

  mainwin->licqDaemon->SetAutoUpdateInfo(chkAutoUpdateInfo->isChecked());
  mainwin->licqDaemon->SetAutoUpdateInfoPlugins(chkAutoUpdateInfoPlugins->isChecked());
  mainwin->licqDaemon->SetAutoUpdateStatusPlugins(chkAutoUpdateStatusPlugins->isChecked());

  // Plugin tab
#ifdef USE_KDE
  if (cmbUrlViewer->currentText() == DEFAULT_URL_VIEWER)
    mainwin->licqDaemon->setUrlViewer("none");  // untranslated!
  else
#endif
    mainwin->licqDaemon->setUrlViewer(cmbUrlViewer->currentText().local8Bit());

  mainwin->licqDaemon->SetTerminal(edtTerminal->text().local8Bit());
  if (cmbDefaultEncoding->currentItem() > 0)
    mainwin->m_DefaultEncoding = UserCodec::encodingForName(cmbDefaultEncoding->currentText());
  else
    mainwin->m_DefaultEncoding = QString::null;
  mainwin->m_bShowAllEncodings = chkShowAllEncodings->isChecked();

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

  oem->setCommand(edtSndPlayer->url());
  oem->setParameter(ON_EVENT_MSG, edtSndMsg->url());
  oem->setParameter(ON_EVENT_URL, edtSndUrl->url());
  oem->setParameter(ON_EVENT_CHAT, edtSndChat->url());
  oem->setParameter(ON_EVENT_FILE, edtSndFile->url());
  oem->setParameter(ON_EVENT_NOTIFY, edtSndNotify->url());
  oem->setParameter(ON_EVENT_SYSMSG, edtSndSysMsg->url());
  oem->setParameter(ON_EVENT_MSGSENT, edtSndMsgSent->url());

  //TODO Make general for all plugins
  ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
  if (o)
  {
    o->SetEnableSave(false);
    o->SetAcceptInAway(chkOEAway->isChecked());
    o->SetAcceptInNA(chkOENA->isChecked());
    o->SetAcceptInOccupied(chkOEOccupied->isChecked());
    o->SetAcceptInDND(chkOEDND->isChecked());
    o->SetEnableSave(true);
    o->SaveLicqInfo();
    gUserManager.DropOwner(o);
  }
  mainwin->licqDaemon->SetAlwaysOnlineNotify(chkAlwaysOnlineNotify->isChecked());
}


void OptionsDlg::setupFontName(QLineEdit* le, const QFont& font)
{
  QString s;
  if (font == mainwin->defaultFont)
    s = tr("default (%1)").arg(font.toString());
  else
    s = font.toString();

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
  QWhatsThis::add(chkAutoClose, tr("Auto close the user function window "
                                   "after a succesful event"));
  chkAutoPopup = new QCheckBox(tr("Auto-Popup Incoming Msg"), boxMainWin);
  QWhatsThis::add(chkAutoPopup, tr("Open all incoming messages automatically "
                                           "when received if we are "
                                           "online (or free for chat)"));
  chkAutoRaise = new QCheckBox(tr("Auto-Raise on Incoming Msg"), boxMainWin);
  QWhatsThis::add(chkAutoRaise, tr("Raise the main window on incoming messages"));
  chkHidden = new QCheckBox(tr("Start Hidden"), boxMainWin);
  QWhatsThis::add(chkHidden, tr("Start main window hidden. Only the dock icon will be visible."));
  chkBoldOnMsg = new QCheckBox(tr("Bold Message Label on Incoming Msg"), boxMainWin);
  QWhatsThis::add(chkBoldOnMsg, tr("Show the message info label in bold font if there are incoming messages"));
  chkManualNewUser = new QCheckBox(tr("Manual \"New User\" group handling"), boxMainWin);
  QWhatsThis::add(chkManualNewUser, tr("If not checked, a user will be automatically "
                                       "removed from \"New User\" group when you first "
                                       "send an event to them"));
  chkSendFromClipboard = new QCheckBox(tr("Check Clipboard For Urls/Files"), boxMainWin);
  QWhatsThis::add(chkSendFromClipboard, tr("When double-clicking on a user to send a message "
   "check for urls/files in the clipboard"));
  chkAutoPosReplyWin = new QCheckBox(tr("Auto Position the Reply Window"), boxMainWin);
  QWhatsThis::add(chkAutoPosReplyWin, tr("Position a new reply window just underneath "
   "the message view window"));
  chkFlashTaskbar = new QCheckBox(tr("Flash Taskbar on Incoming Msg"), boxMainWin);
  QWhatsThis::add(chkFlashTaskbar, tr("Flash the Taskbar on incoming messages"));
  chkAutoSendThroughServer = new QCheckBox(tr("Auto send through server"), boxMainWin);
  QWhatsThis::add(chkAutoSendThroughServer, tr("Automatically send messages through the server "
   "if direct connection fails"));
  chkEnableMainwinMouseMovement = new QCheckBox(tr("Allow dragging main window"), boxMainWin);
  QWhatsThis::add(chkEnableMainwinMouseMovement, tr("Lets you drag around the main window "
   "with your mouse"));

  chkSingleLineChatMode = new QCheckBox(tr("Single line chat mode"), boxMainWin);
  QWhatsThis::add(chkSingleLineChatMode, tr("In single line chat mode you send messages with Enter "
    "and insert new lines with Ctrl+Enter, opposite of the normal mode"));

  chkUseDoubleReturn = new QCheckBox(tr("Use double return"), boxMainWin);
  QWhatsThis::add(chkUseDoubleReturn, tr("Hitting Return twice will be used instead of Ctrl+Return "
    "to send messages and close input dialogs. Multiple new lines can be inserted with Ctrl+Return."));

  chkMsgChatView = new QCheckBox(tr("Chatmode Messageview"), boxMainWin);
  QWhatsThis::add(chkMsgChatView, tr("Show the current chat history in Send Window"));

  chkTabbedChatting = new QCheckBox(tr("Tabbed Chatting"), boxMainWin);
  QWhatsThis::add(chkTabbedChatting, tr("Use tabs in Send Window"));
  //connect(chkMsgChatView, SIGNAL(toggled(bool)), this, SLOT(slot_useMsgChatView(bool)));

  chkShowHistory = new QCheckBox(tr("Show recent messages"), boxMainWin);
  QWhatsThis::add(chkShowHistory, tr("Show the last 5 messages when a Send Window is opened"));
  connect(chkMsgChatView, SIGNAL(toggled(bool)), this, SLOT(slot_useMsgChatView(bool)));

  chkShowNotices = new QCheckBox(tr("Show Join/Left Notices"), boxMainWin);
  QWhatsThis::add(chkShowNotices, tr("Show a notice in the chat window when a user joins or leaves the conversation."));
  connect(chkShowNotices, SIGNAL(toggled(bool)), this, SLOT(slot_refresh_msgViewer()));

  chkSendTN = new QCheckBox(tr("Send typing notifications"), boxMainWin);
  QWhatsThis::add(chkSendTN, tr("Send a notification to the user so they can see when you are typing a message to them"));

  chkMainWinSticky = new QCheckBox(tr("Sticky Main Window"), boxMainWin);
  QWhatsThis::add(chkMainWinSticky, tr("Makes the Main window visible on all desktops"));

  chkMsgWinSticky = new QCheckBox(tr("Sticky Message Window"), boxMainWin);
  QWhatsThis::add(chkMsgWinSticky, tr("Makes the Message window visible on all desktops"));

  l = new QVBoxLayout(l);
  boxLocale = new QGroupBox(1, Horizontal, tr("Localization"), w);
  lblDefaultEncoding = new QLabel(tr("Default Encoding:"), boxLocale);
  QWhatsThis::add(lblDefaultEncoding, tr("Sets which default encoding should be used for newly added contacts."));

  cmbDefaultEncoding = new QComboBox(false, boxLocale);
  cmbDefaultEncoding->insertItem(tr("System default (%1)").arg(QString(QTextCodec::codecForLocale()->name())));
  {
    UserCodec::encoding_t * it = &UserCodec::m_encodings[0];
    while (it->encoding != NULL) {
      cmbDefaultEncoding->insertItem(UserCodec::nameForEncoding(it->encoding));
      ++it;
    }
  }

  chkShowAllEncodings = new QCheckBox(tr("Show all encodings"), boxLocale);
  QWhatsThis::add(chkShowAllEncodings, tr("Show all available encodings in the User Encoding selection menu. Normally, this menu shows only commonly used encodings."));

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
  rdbDockSmall = new QRadioButton(tr("Small Icon"), boxDocking);
  QWhatsThis::add(rdbDockSmall, tr("Uses the freedesktop.org standard to dock a small icon into the window manager.  Works with many different window managers."));
  grpDocking->insert(rdbDockDefault);
  grpDocking->insert(rdbDockThemed);
  grpDocking->insert(rdbDockSmall);
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

  l->addStretch(1);

  return w;
}

void OptionsDlg::slot_useDockToggled(bool b)
{
  if (!b)
  {
#ifndef USE_KDE
    cmbDockTheme->setEnabled(false);
    rdbDockDefault->setEnabled(false);
    rdbDockThemed->setEnabled(false);
    rdbDockSmall->setEnabled(false);
    chkDockFortyEight->setEnabled(false);
#endif
    chkHidden->setEnabled(false);
    chkHidden->setChecked(false);
    return;
  }
  else
  {
    chkHidden->setEnabled(true);
  }

  // Turned on
#ifndef USE_KDE
  rdbDockDefault->setEnabled(true);
  rdbDockThemed->setEnabled(true);
  rdbDockSmall->setEnabled(true);
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
  else
    rdbDockDefault->setChecked(true);
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
  
  connect(chkOnEvents, SIGNAL(toggled(bool)), this, SLOT(slot_chkOnEventsToggled(bool)));

  QWhatsThis::add(chkOnEvents, tr("Enable running of \"Command\" when the relevant "
                                 "event occurs."));
  QLabel *lblSndPlayer = new QLabel(tr("Command:"), hor);
  QWhatsThis::add(lblSndPlayer, tr("<p>Command to execute when an event is received.<br>"
                                  "It will be passed the relevant parameters from "
                                  "below.<br>Parameters can contain the following "
                                  "expressions <br> which will be replaced with the relevant "
                                   "information:</p>") + gMainWindow->usprintfHelp);

  edtSndPlayer = new KURLRequester(hor);

  QGroupBox *boxSndEvents = new QGroupBox(2, Horizontal, tr("Parameters"), w);
  lay->addWidget(boxSndEvents);

  QLabel *lblSndMsg = new QLabel(tr("Message:"), boxSndEvents);
  QWhatsThis::add(lblSndMsg, tr("Parameter for received messages"));
  edtSndMsg = new KURLRequester(boxSndEvents);
  QLabel *lblSndUrl = new QLabel(tr("URL:"), boxSndEvents);
  QWhatsThis::add(lblSndUrl, tr("Parameter for received URLs"));
  edtSndUrl = new KURLRequester(boxSndEvents);
  QLabel *lblSndChat = new QLabel(tr("Chat Request:"), boxSndEvents);
  QWhatsThis::add(lblSndChat, tr("Parameter for received chat requests"));
  edtSndChat = new KURLRequester(boxSndEvents);
  QLabel *lblSndFile = new QLabel(tr("File Transfer:"), boxSndEvents);
  QWhatsThis::add(lblSndFile, tr("Parameter for received file transfers"));
  edtSndFile = new KURLRequester(boxSndEvents);
  QLabel *lblSndNotify = new QLabel(tr("Online Notify:"), boxSndEvents);
  QWhatsThis::add(lblSndNotify, tr("Parameter for online notification"));
  edtSndNotify = new KURLRequester(boxSndEvents);
  QLabel *lblSndSysMsg = new QLabel(tr("System Msg:"), boxSndEvents);
  QWhatsThis::add(lblSndSysMsg, tr("Parameter for received system messages"));
  edtSndSysMsg = new KURLRequester(boxSndEvents);
  QLabel *lblSndMsgSent = new QLabel(tr("Message Sent:"), boxSndEvents);
  QWhatsThis::add(lblSndMsgSent, tr("Parameter for sent messages"));
  edtSndMsgSent = new KURLRequester(boxSndEvents);

  QGroupBox *boxAcceptEvents = new QGroupBox(4, Vertical, tr("Accept Modes"), w);
  lay->addWidget(boxAcceptEvents);

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

  slot_chkOnEventsToggled(chkOnEvents->isChecked());

  lay->addStretch(1);

  return w;
}

// -----------------------------------------------------------------------------
/*! 
 * This slot enables/disables the relevant onEvent configuration widgets,
 * depending on wether chkOnEvent is checked or not.
 */
void OptionsDlg::slot_chkOnEventsToggled(bool b_checked)
{
  edtSndPlayer->setEnabled(b_checked);
  edtSndMsg->setEnabled(b_checked);
  edtSndUrl->setEnabled(b_checked);
  edtSndChat->setEnabled(b_checked);
  edtSndFile->setEnabled(b_checked);
  edtSndNotify->setEnabled(b_checked);
  edtSndSysMsg->setEnabled(b_checked);
  edtSndMsgSent->setEnabled(b_checked);
  chkOEAway->setEnabled(b_checked);
  chkOENA->setEnabled(b_checked);
  chkOEOccupied->setEnabled(b_checked);
  chkOEDND->setEnabled(b_checked);
  chkAlwaysOnlineNotify->setEnabled(b_checked);
}

// -----------------------------------------------------------------------------
QWidget *OptionsDlg::new_network_options()
{
  QWidget *w = new QWidget(this);
  QBoxLayout *lay = new QVBoxLayout(w, 8, 4);

  QGroupBox* gbServer = new QGroupBox(2, QGroupBox::Horizontal, w);
  lay->addWidget(gbServer);
  gbServer->setTitle(tr("Server settings"));

  lblICQServer = new QLabel(tr("ICQ Server:"), gbServer);
  edtICQServer = new QLineEdit(tr("ICQ Server:"), gbServer);
  lblICQServerPort = new QLabel(tr("ICQ Server Port:"), gbServer);
  spnICQServerPort = new QSpinBox(gbServer);
  spnICQServerPort->setRange(0, 0xFFFF);


  QGroupBox *gbFirewall = new QGroupBox(2, QGroupBox::Horizontal, w);
  lay->addWidget(gbFirewall);
  gbFirewall->setTitle(tr("Firewall"));

  chkFirewall = new QCheckBox(tr("I am behind a firewall"), gbFirewall);
  connect(chkFirewall, SIGNAL(toggled(bool)), SLOT(slot_useFirewall(bool)));
  QHBox *boxFw1 = new QHBox(gbFirewall);
  QLabel *lbl = new QLabel(tr("Port Range:"), boxFw1);
  QWhatsThis::add(lbl, tr("TCP port range for incoming connections."));
  spnPortLow = new QSpinBox(boxFw1);
  spnPortLow->setRange(0, 0xFFFF);
  spnPortLow->setSpecialValueText(tr("Auto"));
  
  chkTCPEnabled = new QCheckBox(tr("I can receive direct connections"), gbFirewall);
  connect(chkTCPEnabled, SIGNAL(toggled(bool)), SLOT(slot_usePortRange(bool)));
  QHBox *boxFw2 = new QHBox(gbFirewall);
  lbl = new QLabel(tr("\tto"), boxFw2);
  spnPortHigh = new QSpinBox(boxFw2);
  spnPortHigh->setRange(0, 0xFFFF);
  spnPortHigh->setSpecialValueText(tr("Auto"));

  QGroupBox *gbProxy = new QGroupBox(2, QGroupBox::Horizontal, w);
  lay->addWidget(gbProxy);
  gbProxy->setTitle(tr("Proxy"));

  chkProxyEnabled = new QCheckBox(tr("Use proxy server"), gbProxy);
  chkProxyEnabled->setFixedWidth(220);
  QWidget* h = new QHBox(gbProxy);
  lblProxyType = new QLabel(tr("Proxy Type:"), h);
  cmbProxyType = new QComboBox(h);
  cmbProxyType->setFixedWidth(80);
  cmbProxyType->insertItem(tr("HTTPS"));

  lblProxyHost = new QLabel(tr("Proxy Server:"), gbProxy);
  edtProxyHost = new QLineEdit(tr("Proxy Server:"), gbProxy);
  lblProxyPort = new QLabel(tr("Proxy Server Port:"), gbProxy);
  spnProxyPort = new QSpinBox(gbProxy);
  spnProxyPort->setRange(0, 0xFFFF);

  chkProxyAuthEnabled = new QCheckBox(tr("Use authorization"), gbProxy);
  chkProxyAuthEnabled->setFixedWidth(220);
  new QWidget(gbProxy);
  lblProxyLogin = new QLabel(tr("Username:"), gbProxy);
  edtProxyLogin = new QLineEdit(tr("Username:"), gbProxy);
  lblProxyPasswd = new QLabel(tr("Password:"), gbProxy);
  edtProxyPasswd = new QLineEdit(tr("Password:"), gbProxy);
  edtProxyPasswd->setEchoMode(QLineEdit::Password);

  connect(chkProxyEnabled, SIGNAL(toggled(bool)), SLOT(slot_useProxy(bool)));
  connect(chkProxyAuthEnabled, SIGNAL(toggled(bool)), edtProxyLogin, SLOT(setEnabled(bool)));
  connect(chkProxyAuthEnabled, SIGNAL(toggled(bool)), edtProxyPasswd, SLOT(setEnabled(bool)));
  
  QGroupBox *gbConnection = new QGroupBox(2, QGroupBox::Horizontal, w);
  lay->addWidget(gbConnection);
  gbConnection->setTitle(tr("Connection"));
  
  chkReconnectAfterUinClash = new QCheckBox(tr("Reconnect after Uin clash"), gbConnection);
  QWhatsThis::add(chkReconnectAfterUinClash, tr("Licq can reconnect you when you got "
                                                "disconnected because your Uin was used "
                                                "from another location. Check this if you "
                                                "want Licq to reconnect automatically."));

  lay->addStretch(1);

  return w;
}

void OptionsDlg::slot_useFirewall(bool b)
{
  chkTCPEnabled->setEnabled(b);
  slot_usePortRange(b && chkTCPEnabled->isChecked());
}

void OptionsDlg::slot_usePortRange(bool b)
{
  spnPortLow->setEnabled(b);
  spnPortHigh->setEnabled(b);
}

void OptionsDlg::slot_useMsgChatView(bool b)
{
  if (!b)
  {
    chkTabbedChatting->setChecked(false);
    chkShowHistory->setChecked(false);
    chkShowNotices->setEnabled(false);
  }

  chkTabbedChatting->setEnabled(b);
  chkShowHistory->setEnabled(b);
  chkShowNotices->setEnabled(b);
}

void OptionsDlg::slot_useProxy(bool b)
{
  if (b)
  {
    cmbProxyType->setEnabled(true);
    edtProxyHost->setEnabled(true);
    spnProxyPort->setEnabled(true);
    chkProxyAuthEnabled->setEnabled(true);
    if (chkProxyAuthEnabled->isChecked())
    {
      edtProxyLogin->setEnabled(true);
      edtProxyPasswd->setEnabled(true);
    }
    spnICQServerPort->setValue(DEFAULT_SSL_PORT);
  } else
  {
    cmbProxyType->setEnabled(false);
    edtProxyHost->setEnabled(false);
    spnProxyPort->setEnabled(false);
    chkProxyAuthEnabled->setEnabled(false);
    edtProxyLogin->setEnabled(false);
    edtProxyPasswd->setEnabled(false);
    spnICQServerPort->setValue(DEFAULT_SERVER_PORT);
  }
}

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
  QWidget* w = new QWidget(this, "main");

  QGroupBox *grp = new QGroupBox (0, Horizontal, tr("Column Configuration"), w);
  QGridLayout *lay1 = new QGridLayout(grp->layout(), 7, 5, 10);

  lblColTitle = new QLabel (tr("Title"), grp);
  QWhatsThis::add(lblColTitle, tr("The string which will appear in the list box column header"));
  lblColFormat = new QLabel (tr("Format"), grp);
  QWhatsThis::add(lblColFormat, QStyleSheet::convertFromPlainText(tr("The format string used to define what will "
                                   "appear in each column.\n"
                                   "The following parameters can be used:")) + gMainWindow->usprintfHelp);
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

  boxUserWin = new QGroupBox(2, Horizontal, tr("Options"), w);

  chkSSList = new QCheckBox(tr("Use server side contact list"), boxUserWin);
  QWhatsThis::add(chkSSList, tr("Store your contacts on the server so they are accessible from different locations and/or programs"));
  chkGridLines = new QCheckBox(tr("Show Grid Lines"), boxUserWin);
  QWhatsThis::add(chkGridLines, tr("Draw boxes around each square in the user list"));
  chkHeader = new QCheckBox(tr("Show Column Headers"), boxUserWin);
  QWhatsThis::add(chkHeader, tr("Turns on or off the display of headers above "
     "each column in the user list"));
  chkShowDividers = new QCheckBox(tr("Show User Dividers"), boxUserWin);
  QWhatsThis::add(chkShowDividers, tr("Show the \"--online--\" and \"--offline--\" bars "
     "in the contact list"));
  chkAlwaysShowONU = new QCheckBox(tr("Always show online notify users"), boxUserWin);
  QWhatsThis::add(chkAlwaysShowONU, tr("Show online notify users who are offline even "
     "when offline users are hidden."));
  chkTransparent = new QCheckBox(tr("Transparent when possible"), boxUserWin);
  QWhatsThis::add(chkTransparent, tr("Make the user window transparent when there "
     "is no scroll bar"));
  chkFontStyles = new QCheckBox(tr("Use Font Styles"), boxUserWin);
  QWhatsThis::add(chkFontStyles, tr("Use italics and bold in the user list to "
     "indicate special characteristics such as online notify and visible list"));
  chkFlashAll = new QCheckBox(tr("Blink All Events"), boxUserWin);
  QWhatsThis::add(chkFlashAll, tr("All incoming events will blink"));
  chkFlashUrgent = new QCheckBox(tr("Blink Urgent Events"), boxUserWin);
  QWhatsThis::add(chkFlashUrgent, tr("Only urgent events will blink"));
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
  lblSortBy = new QLabel(tr("Additional &sorting:"), boxUserWin);
  cmbSortBy = new QComboBox(boxUserWin);
  cmbSortBy->insertItem(tr("none"), 0);
  cmbSortBy->insertItem(tr("status"), 1);
  cmbSortBy->insertItem(tr("status + last event"), 2);
  cmbSortBy->insertItem(tr("status + new messages"), 3);
  lblSortBy->setBuddy(cmbSortBy);
  QWhatsThis::add(lblSortBy, tr("<b>none:</b> - Don't sort online users by Status<br>\n"
                                "<b>status</b> - Sort online users by status<br>\n"
                                "<b>status + last event</b> - Sort online users by status and by last event<br>\n"
                                "<b>status + new messages</b> - Sort online users by status and number of new messages"));

  chkShowUserIcons = new QCheckBox(tr("Show user display picture"), boxUserWin);
  QWhatsThis::add(chkShowUserIcons, tr("Show the user's display picture"
				       " instead of a status icon, if the user"
				       " is online and has a display picture"));

  boxPopWin = new QGroupBox(1, Horizontal, tr("Popup info"), w);

  popPicture= new QCheckBox(tr("Picture"), boxPopWin);
  popAlias  = new QCheckBox(tr("Alias"), boxPopWin);
  popName   = new QCheckBox(tr("Name"),  boxPopWin);
  popEmail  = new QCheckBox(tr("Email"), boxPopWin);
  popPhone  = new QCheckBox(tr("Phone"), boxPopWin);
  popFax    = new QCheckBox(tr("Fax"),   boxPopWin);
  popCellular= new QCheckBox(tr("Cellular"), boxPopWin);
  popIP     = new QCheckBox(tr("IP"), boxPopWin);
  popLastOnline = new QCheckBox(tr("Last online"), boxPopWin);
  popOnlineSince = new QCheckBox(tr("Online Time"), boxPopWin);
  popIdleTime = new QCheckBox(tr("Idle Time"), boxPopWin);
  popLocalTime = new QCheckBox(tr("Local time"), boxPopWin);
  popID = new QCheckBox(tr("Protocol ID"), boxPopWin);


  QVBoxLayout *g_main = new QVBoxLayout(w, 10, 5);
  g_main->addWidget(grp);

  QHBoxLayout *g_options = new QHBoxLayout(g_main,8);
  g_options->addWidget(boxUserWin);
  g_options->addWidget(boxPopWin);

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
  cmbUrlViewer = new QComboBox(true, boxExtensions);
#ifdef USE_KDE
  cmbUrlViewer->insertItem(DEFAULT_URL_VIEWER);
#endif
  cmbUrlViewer->insertItem("viewurl-firefox.sh");
  cmbUrlViewer->insertItem("viewurl-lynx.sh");
  cmbUrlViewer->insertItem("viewurl-mozilla.sh");
  cmbUrlViewer->insertItem("viewurl-ncftp.sh");
  cmbUrlViewer->insertItem("viewurl-netscape.sh");
  cmbUrlViewer->insertItem("viewurl-opera.sh");
  cmbUrlViewer->insertItem("viewurl-seamonkey.sh");
  cmbUrlViewer->insertItem("viewurl-w3m.sh");

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

  QWidget* boxAutoStatus = new QGroupBox(2, Horizontal, tr("Auto Away Messages"), w);
  lay->addWidget(boxAutoStatus);
  new QLabel(tr("Away:"), boxAutoStatus);
  cmbAutoAwayMess = new QComboBox(boxAutoStatus);
  new QLabel(tr("N/A:"), boxAutoStatus);
  cmbAutoNAMess = new QComboBox(boxAutoStatus);

  buildAutoStatusCombos(1);

  boxAutoUpdate = new QGroupBox(2, Vertical, tr("Auto Update"), w);
  lay->addWidget(boxAutoUpdate);
  chkAutoUpdateInfo = new QCheckBox(tr("Auto Update Info"), boxAutoUpdate);
  QWhatsThis::add(chkAutoUpdateInfo, tr("Automatically update users' server "
                                        "stored information."));

  chkAutoUpdateInfoPlugins = new QCheckBox(tr("Auto Update Info Plugins"),
                                           boxAutoUpdate);
  QWhatsThis::add(chkAutoUpdateInfoPlugins, tr("Automatically update users' "
                                               "Phone Book and Picture."));

  chkAutoUpdateStatusPlugins = new QCheckBox(tr("Auto Update Status Plugins"),
                                             boxAutoUpdate);
  QWhatsThis::add(chkAutoUpdateStatusPlugins, tr("Automatically update users' "
                     "Phone \"Follow Me\", File Server and ICQphone status."));

  lay->addStretch(1);
  lay->activate();

  return w;
}

QWidget* OptionsDlg::new_chat_options()
{
  QWidget* w = new QWidget(this);
  QVBoxLayout* l = new QVBoxLayout(w, 8, 4);
  QBoxLayout* lay = new QHBoxLayout();

  QVBox* boxRight = new QVBox(w);
  lay->addWidget(boxRight);

  QGroupBox* boxChatOptions = new QGroupBox(1, Horizontal, tr("Chat Options"), boxRight);

  static const int dateFormatsLength = 7;
  static const char *const dateFormats[dateFormatsLength] = {
      "hh:mm:ss",
      "yyyy-MM-dd hh:mm:ss",
      "yyyy-MM-dd",
      "yyyy/MM/dd hh:mm:ss",
      "yyyy/MM/dd",
      "dd.MM.yyyy hh:mm:ss",
      "dd.MM.yyyy"
  };

  new QLabel(tr("Style:"), boxChatOptions);
  cmbChatStyle = new QComboBox(false, boxChatOptions);
  cmbChatStyle->insertStringList(CMessageViewWidget::getStyleNames(false));
  connect(cmbChatStyle, SIGNAL(activated(int)), this, SLOT(slot_refresh_msgViewer()));

  chkChatVertSpacing = new QCheckBox(tr("Insert Vertical Spacing"), boxChatOptions);
  connect(chkChatVertSpacing, SIGNAL(toggled(bool)), this, SLOT(slot_refresh_msgViewer()));
  QWhatsThis::add(chkChatVertSpacing, tr("Insert extra space between messages."));

  chkChatLineBreak = new QCheckBox(tr("Insert Horizontal Line"), boxChatOptions);
  connect(chkChatLineBreak, SIGNAL(toggled(bool)), this, SLOT(slot_refresh_msgViewer()));
  QWhatsThis::add(chkChatLineBreak, tr("Insert a line between each message."));

  QLabel* lblChatDateFormat = new QLabel(tr("Date Format:"), boxChatOptions);
  cmbChatDateFormat = new QComboBox(true, boxChatOptions);
  for(int i = 0; i < dateFormatsLength; ++i)
    cmbChatDateFormat->insertItem(dateFormats[i]);
  connect(cmbChatDateFormat, SIGNAL(activated(int)), this, SLOT(slot_refresh_msgViewer()));
  connect(cmbChatDateFormat, SIGNAL(textChanged(const QString&)), this, SLOT(slot_refresh_msgViewer()));

  QString helpDateFormat = tr(
      "<p>Available custom date format variables.</p>\n"
      "<table>\n"
      "<tr><th>Expression</th><th>Output</th></tr>\n"
      "<tr><td>d</td><td>the day as number without a leading zero (1-31)</td></tr>\n"
      "<tr><td>dd</td><td>the day as number with a leading zero (01-31)</td></tr>\n"
      "<tr><td>ddd</td><td>the abbreviated localized day name (e.g. 'Mon'..'Sun')</td></tr>\n"
      "<tr><td>dddd</td><td>the long localized day name (e.g. 'Monday'..'Sunday')</td></tr>\n"
      "<tr><td>M</td><td>the month as number without a leading zero (1-12)</td></tr>\n"
      "<tr><td>MM</td><td>the month as number with a leading zero (01-12)</td></tr>\n"
      "<tr><td>MMM</td><td>the abbreviated localized month name (e.g. 'Jan'..'Dec')</td></tr>\n"
      "<tr><td>MMMM</td><td>the long localized month name (e.g. 'January'..'December')</td></tr>\n"
      "<tr><td>yy</td><td>the year as two digit number (00-99)</td></tr>\n"
      "<tr><td>yyyy</td><td>the year as four digit number (1752-8000)</td></tr>\n"
      "<tr><td colspan=2></td></tr>\n"
      "<tr><td>h</td><td>the hour without a leading zero (0..23 or 1..12 if AM/PM display)</td></tr>\n"
      "<tr><td>hh</td><td>the hour with a leading zero (00..23 or 01..12 if AM/PM display)</td></tr>\n"
      "<tr><td>m</td><td>the minute without a leading zero (0..59)</td></tr>\n"
      "<tr><td>mm</td><td>the minute with a leading zero (00..59)</td></tr>\n"
      "<tr><td>s</td><td>the second without a leading zero (0..59)</td></tr>\n"
      "<tr><td>ss</td><td>the second with a leading zero (00..59)</td></tr>\n"
      "<tr><td>z</td><td>the millisecond without leading zero (0..999)</td></tr>\n"
      "<tr><td>zzz</td><td>the millisecond with leading zero (000..999)</td></tr>\n"
      "<tr><td>AP</td><td>use AM/PM display. AP will be replaced by either 'AM' or 'PM'</td></tr>\n"
      "<tr><td>ap</td><td>use am/pm display. ap will be replaced by either 'am' or 'pm'</td></tr>\n"
      "</table>");

  QWhatsThis::add(lblChatDateFormat, helpDateFormat);
  QWhatsThis::add(cmbChatDateFormat, helpDateFormat);

  QGroupBox* boxHistOptions = new QGroupBox(1, Horizontal, tr("History Options"), boxRight);
  new QLabel(tr("Style:"), boxHistOptions);
  cmbHistStyle = new QComboBox(false, boxHistOptions);
  cmbHistStyle->insertStringList(CMessageViewWidget::getStyleNames(true));
  connect(cmbHistStyle, SIGNAL(activated(int)), this, SLOT(slot_refresh_msgViewer()));

  chkHistVertSpacing = new QCheckBox(tr("Insert Vertical Spacing"), boxHistOptions);
  connect(chkHistVertSpacing, SIGNAL(toggled(bool)), this, SLOT(slot_refresh_msgViewer()));
  QWhatsThis::add(chkHistVertSpacing, tr("Insert extra space between messages."));

  QLabel* lblHistDateFormat = new QLabel(tr("Date Format:"), boxHistOptions);
  cmbHistDateFormat = new QComboBox(true, boxHistOptions);
  for(int i = 0; i < dateFormatsLength; ++i)
    cmbHistDateFormat->insertItem(dateFormats[i]);
  connect(cmbHistDateFormat, SIGNAL(activated(int)), this, SLOT(slot_refresh_msgViewer()));
  connect(cmbHistDateFormat, SIGNAL(textChanged(const QString&)), this, SLOT(slot_refresh_msgViewer()));
  QWhatsThis::add(lblHistDateFormat, helpDateFormat);
  QWhatsThis::add(cmbHistDateFormat, helpDateFormat);


  QGroupBox *boxColors = new QGroupBox(2, Horizontal, tr("Colors"), boxRight);

  new QLabel(tr("Message Received:"), boxColors);
  btnColorRcv = new CColorOption(boxColors);

  new QLabel(tr("Message Sent:"), boxColors);
  btnColorSnt = new CColorOption(boxColors);

  new QLabel(tr("History Received:"), boxColors);
  btnColorRcvHistory = new CColorOption(boxColors);

  new QLabel(tr("History Sent:"), boxColors);
  btnColorSntHistory = new CColorOption(boxColors);

  new QLabel(tr("Notice:"), boxColors);
  btnColorNotice = new CColorOption(boxColors);

  /*new QLabel(tr("Tab Label Color:"), boxColors);
  btnColorTabLabel = new CColorOption(boxColors);*/

  new QLabel(tr("Typing Notification Color:"), boxColors);
  btnColorTypingLabel = new CColorOption(boxColors);

  new QLabel(tr("Background Color:"), boxColors);
  btnColorChatBkg = new CColorOption(boxColors);

  connect(btnColorSnt, SIGNAL(changed()), this, SLOT(slot_refresh_msgViewer()));
  connect(btnColorRcv, SIGNAL(changed()), this, SLOT(slot_refresh_msgViewer()));
  connect(btnColorSntHistory, SIGNAL(changed()), this, SLOT(slot_refresh_msgViewer()));
  connect(btnColorRcvHistory, SIGNAL(changed()), this, SLOT(slot_refresh_msgViewer()));
  connect(btnColorNotice, SIGNAL(changed()), this, SLOT(slot_refresh_msgViewer()));
  /*connect(btnColorTabLabel, SIGNAL(changed()), this, SLOT(slot_refresh_msgViewer()));*/
  connect(btnColorTypingLabel, SIGNAL(changed()), this, SLOT(slot_refresh_msgViewer()));
  connect(btnColorChatBkg, SIGNAL(changed()), this, SLOT(slot_refresh_msgViewer()));

  tabViewer = new CETabWidget(w);
  lay->addWidget(tabViewer);

  msgChatViewer = new CMessageViewWidget(NULL, 0, gMainWindow, tabViewer);
  tabViewer->insertTab(msgChatViewer, "Marge");

  msgHistViewer = new CMessageViewWidget(NULL, 0, gMainWindow, tabViewer, 0, true);
  tabViewer->insertTab(msgHistViewer, tr("History"));

  lay->activate();

  l->addLayout(lay);
  l->addStretch(1);

  return w;
}

void OptionsDlg::slot_refresh_msgViewer()
{
  // Don't update the time at every refresh
  static QDateTime date = QDateTime::currentDateTime();

  const char* const names[2] = {"Marge", "Homer"};
  const char* const msgs[8] = {
      QT_TR_NOOP("This is a received message"),
      QT_TR_NOOP("This is a sent message"),
      QT_TR_NOOP("Have you gone to the Licq IRC Channel?"),
      QT_TR_NOOP("No, where is it?"),
      QT_TR_NOOP("#Licq on irc.freenode.net"),
      QT_TR_NOOP("Cool, I'll see you there :)"),
      QT_TR_NOOP("We'll be waiting!"),
      QT_TR_NOOP("Marge has left the conversation.")
  };

  msgChatViewer->m_nMsgStyle = cmbChatStyle->currentItem();
  msgChatViewer->m_extraSpacing = chkChatVertSpacing->isChecked();
  msgChatViewer->m_appendLineBreak = chkChatLineBreak->isChecked();
  msgChatViewer->m_showNotices = chkShowNotices->isChecked();
  msgChatViewer->m_colorSnt = btnColorSnt->paletteBackgroundColor();
  msgChatViewer->m_colorRcv = btnColorRcv->paletteBackgroundColor();
  msgChatViewer->m_colorSntHistory = btnColorSntHistory->paletteBackgroundColor();
  msgChatViewer->m_colorRcvHistory = btnColorRcvHistory->paletteBackgroundColor();
  msgChatViewer->m_colorNotice = btnColorNotice->paletteBackgroundColor();
  tabViewer->setTabColor(msgChatViewer, btnColorTypingLabel->paletteBackgroundColor());
  msgChatViewer->setPaletteBackgroundColor(btnColorChatBkg->paletteBackgroundColor());

  msgChatViewer->m_nDateFormat = cmbChatDateFormat->currentText();

  msgHistViewer->m_nMsgStyle = cmbHistStyle->currentItem();
  msgHistViewer->m_extraSpacing = chkHistVertSpacing->isChecked();
  msgHistViewer->m_colorSnt = btnColorSnt->paletteBackgroundColor();
  msgHistViewer->m_colorRcv = btnColorRcv->paletteBackgroundColor();
  msgHistViewer->setPaletteBackgroundColor(btnColorChatBkg->paletteBackgroundColor());
  msgHistViewer->m_nDateFormat = cmbHistDateFormat->currentText();

  msgChatViewer->clear();
  msgHistViewer->clear();

  QDateTime msgDate = date;
  for (unsigned int i = 0; i<7; i++)
  {
    msgChatViewer->addMsg(i%2 == 0 ? D_RECEIVER : D_SENDER, (i<2),
          QString(""),
          msgDate,
          true, false, false, false, 
          names[i % 2],
          MLView::toRichText(tr(msgs[i]), true, true));

    msgHistViewer->addMsg(i%2 == 0 ? D_RECEIVER : D_SENDER, false,
          QString(""),
          msgDate,
          true, false, false, false,
          names[i % 2],
          MLView::toRichText(tr(msgs[i]), true, true));

    msgDate = msgDate.addSecs(i + 12);
  }
  msgDate = msgDate.addSecs(12);
  msgChatViewer->addNotice(msgDate, MLView::toRichText(tr(msgs[7]), true, true));

  msgHistViewer->updateContent();
}

CColorOption::CColorOption (QWidget* parent)
    : QPushButton(parent) 
{
  setFixedSize(40, 20);
  connect(this, SIGNAL(clicked()), this, SLOT(slot_select_color()));
}

void CColorOption::slot_select_color()
{
  QColor color = QColorDialog::getColor(
      paletteBackgroundColor(), this);
  if (color.isValid()) 
  {
    setPaletteBackgroundColor(color);
    emit changed();
  }
}

// -----------------------------------------------------------------------------
QWidget* OptionsDlg::new_phone_options()
{
  QWidget* w = new QWidget(this);
  /*QBoxLayout* lay =*/ (void) new QVBoxLayout(w, 8, 4);

  return w;
}


// -----------------------------------------------------------------------------
// I want to rebuild the Combo boxes when user saves the
// messages therefor we have a separate function.
void OptionsDlg::buildAutoStatusCombos(bool firstTime)
{
  int selectedNA, selectedAway;

  //Save selection (or get first selection)
  if (firstTime)
  {
    selectedAway = mainwin->autoAwayMess;
    selectedNA   = mainwin->autoNAMess;
  }
  else
  {
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
