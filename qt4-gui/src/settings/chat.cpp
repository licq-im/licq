// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007 Licq developers
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

#include "chat.h"

#include "config.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QTextCodec>
#include <QVBoxLayout>

#include <licq_icqd.h>

#include "config/chat.h"
#include "config/general.h"

#include "core/gui-defines.h"

#include "helpers/usercodec.h"

#include "widgets/colorbutton.h"
#include "widgets/historyview.h"
#include "widgets/tabwidget.h"

#include "settingsdlg.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::Settings::Chat */

Settings::Chat::Chat(SettingsDlg* parent)
  : QObject(parent)
{
  parent->addPage(SettingsDlg::ChatPage, createPageChat(parent),
      tr("Chat"));
  parent->addPage(SettingsDlg::ChatDispPage, createPageChatDisp(parent),
      tr("Chat Display"), SettingsDlg::ChatPage);
  parent->addPage(SettingsDlg::HistDispPage, createPageHistDisp(parent),
      tr("History Display"), SettingsDlg::ChatPage);

  load();
}

QWidget* Settings::Chat::createPageChat(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  layPageChat = new QVBoxLayout(w);
  layPageChat->setContentsMargins(0, 0, 0, 0);

  boxChat = new QGroupBox(tr("General Chat Options"));
  layChat = new QGridLayout(boxChat);

  chkMsgChatView = new QCheckBox(tr("Chatmode messageview"));
  chkMsgChatView->setToolTip(tr("Show the current chat history in Send Window"));
  connect(chkMsgChatView, SIGNAL(toggled(bool)), SLOT(slot_useMsgChatView(bool)));
  layChat->addWidget(chkMsgChatView, 0, 0);

  chkTabbedChatting = new QCheckBox(tr("Tabbed chatting"));
  chkTabbedChatting->setToolTip(tr("Use tabs in Send Window"));
  layChat->addWidget(chkTabbedChatting, 0, 1);

  chkSingleLineChatMode = new QCheckBox(tr("Single line chat mode"));
  chkSingleLineChatMode->setToolTip(tr("Send messages with Enter and insert new lines with Ctrl+Enter, opposite of the normal mode"));
  layChat->addWidget(chkSingleLineChatMode, 1, 0);

  chkUseDoubleReturn = new QCheckBox(tr("Use double return"));
  chkUseDoubleReturn->setToolTip(tr("Hitting Return twice will be used instead of Ctrl+Return\n"
    "to send messages and close input dialogs.\n"
    "Multiple new lines can be inserted with Ctrl+Return."));
  layChat->addWidget(chkUseDoubleReturn, 1, 1);

  chkShowSendClose = new QCheckBox(tr("Show Send/Close buttons"));
  chkShowSendClose->setToolTip(tr("Show Send and Close buttons in the chat dialog."));
  layChat->addWidget(chkShowSendClose, 2, 0);

  chkMsgWinSticky = new QCheckBox(tr("Sticky message window(s)"));
  chkMsgWinSticky->setToolTip(tr("Makes the message window(s) visible on all desktops"));
  layChat->addWidget(chkMsgWinSticky, 3, 0);

  chkAutoClose = new QCheckBox(tr("Auto close function window"));
  chkAutoClose->setToolTip(tr("Auto close the user function window after a successful event"));
  layChat->addWidget(chkAutoClose, 3, 1);

  chkSendFromClipboard = new QCheckBox(tr("Check clipboard For URIs/files"));
  chkSendFromClipboard->setToolTip(tr("When double-clicking on a user to send a message check for urls/files in the clipboard"));
  layChat->addWidget(chkSendFromClipboard, 4, 0);

  chkAutoPosReplyWin = new QCheckBox(tr("Auto position the reply window"));
  chkAutoPosReplyWin->setToolTip(tr("Position a new reply window just underneath the message view window"));
  layChat->addWidget(chkAutoPosReplyWin, 4, 1);

  chkAutoSendThroughServer = new QCheckBox(tr("Auto send through server"));
  chkAutoSendThroughServer->setToolTip(tr("Automatically send messages through the server if direct connection fails"));
  layChat->addWidget(chkAutoSendThroughServer, 5, 0);

  chkSendTN = new QCheckBox(tr("Send typing notifications"));
  chkSendTN->setToolTip(tr("Send a notification to the user so they can see when you are typing a message to them"));
  layChat->addWidget(chkSendTN, 5, 1);

  chkShowUserPic = new QCheckBox(tr("Show user picture"));
  chkShowUserPic->setToolTip(tr("Show user picture next to the input area"));
  layChat->addWidget(chkShowUserPic, 6, 0);

  chkShowUserPicHidden = new QCheckBox(tr("Minimize user picture"));
  chkShowUserPicHidden->setToolTip(tr("Hide user picture upon opening"));
  layChat->addWidget(chkShowUserPicHidden, 6, 1);


  myLocaleBox = new QGroupBox(tr("Localization"));
  myLocaleLayout = new QVBoxLayout(myLocaleBox);

  QHBoxLayout* defaultEncodingLayout = new QHBoxLayout();
  myDefaultEncodingLabel = new QLabel(tr("Default encoding:"));
  myDefaultEncodingLabel->setToolTip(tr(
      "Sets which default encoding should be used for newly added contacts."));
  defaultEncodingLayout->addWidget(myDefaultEncodingLabel);
  myDefaultEncodingCombo = new QComboBox();
  myDefaultEncodingCombo->addItem(tr("System default (%1)").arg(
      QString(QTextCodec::codecForLocale()->name())));
  {
    UserCodec::encoding_t* it = &UserCodec::m_encodings[0];
    while (it->encoding != NULL)
    {
      myDefaultEncodingCombo->addItem(UserCodec::nameForEncoding(it->encoding));
      ++it;
    }
  }
  myDefaultEncodingCombo->setToolTip(myDefaultEncodingLabel->toolTip());
  myDefaultEncodingLabel->setBuddy(myDefaultEncodingCombo);
  defaultEncodingLayout->addWidget(myDefaultEncodingCombo);
  myLocaleLayout->addLayout(defaultEncodingLayout);

  myShowAllEncodingsCheck = new QCheckBox(tr("Show all encodings"));
  myShowAllEncodingsCheck->setToolTip(tr(
      "Show all available encodings in the User Encoding selection menu.\n"
      "Normally, this menu shows only commonly used encodings."));
  myLocaleLayout->addWidget(myShowAllEncodingsCheck);


  boxExtensions = new QGroupBox(tr("Extensions"));
  layExtensions = new QGridLayout(boxExtensions);
  layExtensions->setColumnStretch(1, 1);

  lblUrlViewer = new QLabel(tr("URI viewer:"));
  lblUrlViewer->setToolTip(tr("The command to run to view a URL.  Will be passed the URL as a parameter."));
  layExtensions->addWidget(lblUrlViewer, 0, 0);

  cmbUrlViewer = new QComboBox();
  cmbUrlViewer->setEditable(true);
#ifdef USE_KDE
  cmbUrlViewer->addItem(tr("KDE default"));
#endif
  cmbUrlViewer->addItem("viewurl-firefox.sh");
  cmbUrlViewer->addItem("viewurl-lynx.sh");
  cmbUrlViewer->addItem("viewurl-mozilla.sh");
  cmbUrlViewer->addItem("viewurl-ncftp.sh");
  cmbUrlViewer->addItem("viewurl-netscape.sh");
  cmbUrlViewer->addItem("viewurl-opera.sh");
  cmbUrlViewer->addItem("viewurl-seamonkey.sh");
  cmbUrlViewer->addItem("viewurl-w3m.sh");
  cmbUrlViewer->setToolTip(lblUrlViewer->toolTip());
  lblUrlViewer->setBuddy(cmbUrlViewer);
  layExtensions->addWidget(cmbUrlViewer, 0, 1);

  lblTerminal = new QLabel(tr("Terminal:"));
  lblTerminal->setToolTip(tr("The command to run to start your terminal program."));
  layExtensions->addWidget(lblTerminal, 1, 0);

  edtTerminal = new QLineEdit(tr("Terminal:"));
  edtTerminal->setToolTip(lblTerminal->toolTip());
  lblTerminal->setBuddy(edtTerminal);
  layExtensions->addWidget(edtTerminal, 1, 1);


  layPageChat->addWidget(boxChat);
  layPageChat->addWidget(myLocaleBox);
  layPageChat->addWidget(boxExtensions);
  layPageChat->addStretch(1);

  return w;
}

static const int dateFormatsLength = 7;
static const char* const dateFormats[dateFormatsLength] = {
    "hh:mm:ss",
    "yyyy-MM-dd hh:mm:ss",
    "yyyy-MM-dd",
    "yyyy/MM/dd hh:mm:ss",
    "yyyy/MM/dd",
    "dd.MM.yyyy hh:mm:ss",
    "dd.MM.yyyy"
};

static const char* const helpDateFormat
  = QT_TRANSLATE_NOOP("LicqQtGui::OptionsDlg",
    "<p>Available custom date format variables.</p>"
    "<table>"
    "<tr><th>Expression</th><th>Output</th></tr>"
    "<tr><td>d</td><td>the day as number without a leading zero (1-31)</td></tr>"
    "<tr><td>dd</td><td>the day as number with a leading zero (01-31)</td></tr>"
    "<tr><td>ddd</td><td>the abbreviated localized day name (e.g. 'Mon'..'Sun')</td></tr>"
    "<tr><td>dddd</td><td>the long localized day name (e.g. 'Monday'..'Sunday')</td></tr>"
    "<tr><td>M</td><td>the month as number without a leading zero (1-12)</td></tr>"
    "<tr><td>MM</td><td>the month as number with a leading zero (01-12)</td></tr>"
    "<tr><td>MMM</td><td>the abbreviated localized month name (e.g. 'Jan'..'Dec')</td></tr>"
    "<tr><td>MMMM</td><td>the long localized month name (e.g. 'January'..'December')</td></tr>"
    "<tr><td>yy</td><td>the year as two digit number (00-99)</td></tr>"
    "<tr><td>yyyy</td><td>the year as four digit number (1752-8000)</td></tr>"
    "<tr><td colspan=2></td></tr>"
    "<tr><td>h</td><td>the hour without a leading zero (0..23 or 1..12 if AM/PM display)</td></tr>"
    "<tr><td>hh</td><td>the hour with a leading zero (00..23 or 01..12 if AM/PM display)</td></tr>"
    "<tr><td>m</td><td>the minute without a leading zero (0..59)</td></tr>"
    "<tr><td>mm</td><td>the minute with a leading zero (00..59)</td></tr>"
    "<tr><td>s</td><td>the second without a leading zero (0..59)</td></tr>"
    "<tr><td>ss</td><td>the second with a leading zero (00..59)</td></tr>"
    "<tr><td>z</td><td>the millisecond without leading zero (0..999)</td></tr>"
    "<tr><td>zzz</td><td>the millisecond with leading zero (000..999)</td></tr>"
    "<tr><td>AP</td><td>use AM/PM display. AP will be replaced by either 'AM' or 'PM'</td></tr>"
    "<tr><td>ap</td><td>use am/pm display. ap will be replaced by either 'am' or 'pm'</td></tr>"
    "</table>"
  );

QWidget* Settings::Chat::createPageChatDisp(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  layPageChatDisp = new QGridLayout(w);
  layPageChatDisp->setContentsMargins(0, 0, 0, 0);
  layPageChatDisp->setRowStretch(1, 1);

  boxChatDisp = new QGroupBox(tr("Chat Display"));
  layChatDisp = new QVBoxLayout(boxChatDisp);

  QHBoxLayout* layChatStyle = new QHBoxLayout();
  lblChatStyle = new QLabel(tr("Style:"));
  layChatStyle->addWidget(lblChatStyle);
  cmbChatStyle = new QComboBox();
  cmbChatStyle->addItems(HistoryView::getStyleNames(false));
  lblChatStyle->setBuddy(cmbChatStyle);
  connect(cmbChatStyle, SIGNAL(activated(int)), SLOT(slot_refresh_msgViewer()));
  layChatStyle->addWidget(cmbChatStyle);
  layChatDisp->addLayout(layChatStyle);

  QHBoxLayout* layChatDateFormat = new QHBoxLayout();
  lblChatDateFormat = new QLabel(tr("Date format:"));
  lblChatDateFormat->setToolTip(tr(helpDateFormat));
  layChatDateFormat->addWidget(lblChatDateFormat);
  cmbChatDateFormat = new QComboBox();
  cmbChatDateFormat->setEditable(true);
  cmbChatDateFormat->setToolTip(lblChatDateFormat->toolTip());
  lblChatDateFormat->setBuddy(cmbChatDateFormat);
  for(int i = 0; i < dateFormatsLength; ++i)
    cmbChatDateFormat->addItem(dateFormats[i]);
  connect(cmbChatDateFormat, SIGNAL(activated(int)), SLOT(slot_refresh_msgViewer()));
  connect(cmbChatDateFormat, SIGNAL(editTextChanged(const QString&)), SLOT(slot_refresh_msgViewer()));
  layChatDateFormat->addWidget(cmbChatDateFormat);
  layChatDisp->addLayout(layChatDateFormat);

  chkChatVertSpacing = new QCheckBox(tr("Insert vertical spacing"));
  connect(chkChatVertSpacing, SIGNAL(toggled(bool)), SLOT(slot_refresh_msgViewer()));
  chkChatVertSpacing->setToolTip(tr("Insert extra space between messages."));
  layChatDisp->addWidget(chkChatVertSpacing);

  chkChatLineBreak = new QCheckBox(tr("Insert horizontal line"));
  connect(chkChatLineBreak, SIGNAL(toggled(bool)), SLOT(slot_refresh_msgViewer()));
  chkChatLineBreak->setToolTip(tr("Insert a line between each message."));
  layChatDisp->addWidget(chkChatLineBreak);

  chkShowHistory = new QCheckBox(tr("Show recent messages"));
  chkShowHistory->setToolTip(tr("Show the last 5 messages when a Send Window is opened"));
  connect(chkShowHistory, SIGNAL(toggled(bool)), SLOT(slot_refresh_msgViewer()));
  layChatDisp->addWidget(chkShowHistory);

  chkShowNotices = new QCheckBox(tr("Show join/left notices"));
  chkShowNotices->setToolTip(tr("Show a notice in the chat window when a user joins or leaves the conversation."));
  connect(chkShowNotices, SIGNAL(toggled(bool)), SLOT(slot_refresh_msgViewer()));
  layChatDisp->addWidget(chkShowNotices);

  layChatDisp->addStretch(1);


  boxChatColors = new QGroupBox(tr("Colors"));
  layChatColors = new QGridLayout(boxChatColors);

  lblColorRcv = new QLabel(tr("Message received:"));
  layChatColors->addWidget(lblColorRcv, 0, 0);
  btnColorRcv = new ColorButton();
  connect(btnColorRcv, SIGNAL(changed(const QColor&)), SLOT(slot_refresh_msgViewer()));
  layChatColors->addWidget(btnColorRcv, 0, 1);

  lblColorSnt = new QLabel(tr("Message sent:"));
  layChatColors->addWidget(lblColorSnt, 1, 0);
  btnColorSnt = new ColorButton();
  connect(btnColorSnt, SIGNAL(changed(const QColor&)), SLOT(slot_refresh_msgViewer()));
  layChatColors->addWidget(btnColorSnt, 1, 1);

  lblColorRcvHistory = new QLabel(tr("History received:"));
  layChatColors->addWidget(lblColorRcvHistory, 2, 0);
  btnColorRcvHistory = new ColorButton();
  connect(btnColorRcvHistory, SIGNAL(changed(const QColor&)), SLOT(slot_refresh_msgViewer()));
  layChatColors->addWidget(btnColorRcvHistory, 2, 1);

  lblColorSntHistory = new QLabel(tr("History sent:"));
  layChatColors->addWidget(lblColorSntHistory, 3, 0);
  btnColorSntHistory = new ColorButton();
  connect(btnColorSntHistory, SIGNAL(changed(const QColor&)), SLOT(slot_refresh_msgViewer()));
  layChatColors->addWidget(btnColorSntHistory, 3, 1);

  lblColorNotice = new QLabel(tr("Notice:"));
  layChatColors->addWidget(lblColorNotice, 4, 0);
  btnColorNotice = new ColorButton();
  connect(btnColorNotice, SIGNAL(changed(const QColor&)), SLOT(slot_refresh_msgViewer()));
  layChatColors->addWidget(btnColorNotice, 4, 1);

  lblColorTypingLabel = new QLabel(tr("Typing notification color:"));
  layChatColors->addWidget(lblColorTypingLabel, 5, 0);
  btnColorTypingLabel = new ColorButton();
  connect(btnColorTypingLabel, SIGNAL(changed(const QColor&)), SLOT(slot_refresh_msgViewer()));
  layChatColors->addWidget(btnColorTypingLabel, 5, 1);

  lblColorChatBkg = new QLabel(tr("Background color:"));
  layChatColors->addWidget(lblColorChatBkg, 6, 0);
  btnColorChatBkg = new ColorButton();
  connect(btnColorChatBkg, SIGNAL(changed(const QColor&)), SLOT(slot_refresh_msgViewer()));
  layChatColors->addWidget(btnColorChatBkg, 6, 1);

  layChatColors->setRowStretch(7, 1);

  tabViewer = new TabWidget(w);
  msgChatViewer = new HistoryView(false, QString(), 0, tabViewer);
  tabViewer->addTab(msgChatViewer, "Marge");

  layPageChatDisp->addWidget(boxChatDisp, 0, 0);
  layPageChatDisp->addWidget(boxChatColors, 0, 1);
  layPageChatDisp->addWidget(tabViewer, 1, 0, 1, 2);

  return w;
}

QWidget* Settings::Chat::createPageHistDisp(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  layPageHistDisp = new QVBoxLayout(w);
  layPageHistDisp->setContentsMargins(0, 0, 0, 0);

  boxHistDisp = new QGroupBox(tr("History Display"));
  layHistDisp = new QVBoxLayout(boxHistDisp);

  QHBoxLayout* layHistStyle = new QHBoxLayout();
  lblHistStyle = new QLabel(tr("Style:"));
  layHistStyle->addWidget(lblHistStyle);
  cmbHistStyle = new QComboBox();
  cmbHistStyle->addItems(HistoryView::getStyleNames(true));
  lblHistStyle->setBuddy(cmbHistStyle);
  connect(cmbHistStyle, SIGNAL(activated(int)), SLOT(slot_refresh_msgViewer()));
  layHistStyle->addWidget(cmbHistStyle);
  layHistDisp->addLayout(layHistStyle);

  QHBoxLayout* layHistDateFormat = new QHBoxLayout();
  lblHistDateFormat = new QLabel(tr("Date format:"));
  lblHistDateFormat->setToolTip(tr(helpDateFormat));
  layHistDateFormat->addWidget(lblHistDateFormat);
  cmbHistDateFormat = new QComboBox();
  cmbHistDateFormat->setEditable(true);
  for(int i = 0; i < dateFormatsLength; ++i)
    cmbHistDateFormat->addItem(dateFormats[i]);
  cmbHistDateFormat->setToolTip(lblHistDateFormat->toolTip());
  lblHistDateFormat->setBuddy(cmbHistDateFormat);
  connect(cmbHistDateFormat, SIGNAL(activated(int)), SLOT(slot_refresh_msgViewer()));
  connect(cmbHistDateFormat, SIGNAL(editTextChanged(const QString&)), SLOT(slot_refresh_msgViewer()));
  layHistDateFormat->addWidget(cmbHistDateFormat);
  layHistDisp->addLayout(layHistDateFormat);

  QHBoxLayout* layHistOpts = new QHBoxLayout();
  chkHistVertSpacing = new QCheckBox(tr("Insert vertical spacing"));
  connect(chkHistVertSpacing, SIGNAL(toggled(bool)), SLOT(slot_refresh_msgViewer()));
  chkHistVertSpacing->setToolTip(tr("Insert extra space between messages."));
  layHistOpts->addWidget(chkHistVertSpacing);

  chkHistReverse = new QCheckBox(tr("Reverse history"));
  connect(chkHistReverse, SIGNAL(toggled(bool)), SLOT(slot_refresh_msgViewer()));
  chkHistReverse->setToolTip(tr("Put recent messages on top."));
  layHistOpts->addWidget(chkHistReverse);
  layHistDisp->addLayout(layHistOpts);

  boxHistPreview = new QGroupBox(tr("Preview"));
  layHistPreview = new QVBoxLayout(boxHistPreview);

  msgHistViewer = new HistoryView(true);
  layHistPreview->addWidget(msgHistViewer);

  layPageHistDisp->addWidget(boxHistDisp);
  layPageHistDisp->addWidget(boxHistPreview);

  return w;
}

void Settings::Chat::slot_useMsgChatView(bool b)
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

void Settings::Chat::slot_refresh_msgViewer()
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

  msgChatViewer->setChatConfig(cmbChatStyle->currentIndex(), cmbChatDateFormat->currentText(),
      chkChatVertSpacing->isChecked(), chkChatLineBreak->isChecked(), chkShowNotices->isChecked());

  msgChatViewer->setColors(btnColorChatBkg->colorName(), btnColorRcv->colorName(),
      btnColorSnt->colorName(), btnColorRcvHistory->colorName(),
      btnColorSntHistory->colorName(), btnColorNotice->colorName());
  tabViewer->setTabColor(msgChatViewer, btnColorTypingLabel->color());

  msgHistViewer->setHistoryConfig(cmbHistStyle->currentIndex(), cmbHistDateFormat->currentText(),
      chkHistVertSpacing->isChecked(), chkHistReverse->isChecked());
  msgHistViewer->setColors(btnColorChatBkg->colorName(), btnColorRcv->colorName(), btnColorSnt->colorName());

  msgChatViewer->clear();
  msgHistViewer->clear();

  QDateTime msgDate = date;
  for (unsigned int i = 0; i<7; i++)
  {
    if (i < 2 && chkShowHistory->isChecked() == false)
      continue;

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

void Settings::Chat::load()
{
  Config::Chat* chatConfig = Config::Chat::instance();
  Config::General* generalConfig = Config::General::instance();

  chkUseDoubleReturn->setChecked(generalConfig->useDoubleReturn());

  chkAutoClose->setChecked(chatConfig->autoClose());
  chkSendFromClipboard->setChecked(chatConfig->sendFromClipboard());
  chkMsgChatView->setChecked(chatConfig->msgChatView());
  cmbChatDateFormat->lineEdit()->setText(chatConfig->chatDateFormat());
  chkChatVertSpacing->setChecked(chatConfig->chatVertSpacing());
  chkChatLineBreak->setChecked(chatConfig->chatAppendLineBreak());
  cmbChatStyle->setCurrentIndex(chatConfig->chatMsgStyle());
  cmbHistStyle->setCurrentIndex(chatConfig->histMsgStyle());
  chkHistVertSpacing->setChecked(chatConfig->histVertSpacing());
  chkHistReverse->setChecked(chatConfig->reverseHistory());
  cmbHistDateFormat->lineEdit()->setText(chatConfig->histDateFormat());
  btnColorRcv->setColor(chatConfig->recvColor());
  btnColorSnt->setColor(chatConfig->sentColor());
  btnColorRcvHistory->setColor(chatConfig->recvHistoryColor());
  btnColorSntHistory->setColor(chatConfig->sentHistoryColor());
  btnColorNotice->setColor(chatConfig->noticeColor());
  btnColorTypingLabel->setColor(chatConfig->tabTypingColor());
  btnColorChatBkg->setColor(chatConfig->chatBackColor());
  chkAutoPosReplyWin->setChecked(chatConfig->autoPosReplyWin());
  chkAutoSendThroughServer->setChecked(chatConfig->autoSendThroughServer());
  chkShowSendClose->setChecked(chatConfig->showSendClose());
  chkMsgWinSticky->setChecked(chatConfig->msgWinSticky());
  chkSingleLineChatMode->setChecked(chatConfig->singleLineChatMode());
  chkTabbedChatting->setChecked(chatConfig->tabbedChatting());
  chkShowHistory->setChecked(chatConfig->showHistory());
  chkShowNotices->setChecked(chatConfig->showNotices());
  chkShowUserPic->setChecked(chatConfig->showUserPic());
  chkShowUserPicHidden->setChecked(chatConfig->showUserPicHidden());

  if (!chatConfig->msgChatView())
  {
    chkTabbedChatting->setEnabled(false);
    chkShowHistory->setEnabled(false);
    chkShowNotices->setChecked(false);
  }

  chkSendTN->setChecked(gLicqDaemon->SendTypingNotification());

  myDefaultEncodingCombo->setCurrentIndex(0);
  // first combo box item is the locale encoding, so we skip it
  for (int i = 1; i < myDefaultEncodingCombo->count(); i++)
  {
    if (UserCodec::encodingForName(myDefaultEncodingCombo->itemText(i)) == chatConfig->defaultEncoding())
    {
       myDefaultEncodingCombo->setCurrentIndex(i);
       break;
    }
  }
  myShowAllEncodingsCheck->setChecked(chatConfig->showAllEncodings());

  cmbUrlViewer->setItemText(cmbUrlViewer->currentIndex(),
      gLicqDaemon->getUrlViewer() == NULL ?
      DEFAULT_URL_VIEWER : QString(gLicqDaemon->getUrlViewer()));
  edtTerminal->setText(gLicqDaemon->Terminal() == NULL ?
      tr("none") : QString(gLicqDaemon->Terminal()));

  slot_refresh_msgViewer();
}

void Settings::Chat::apply()
{
  Config::Chat* chatConfig = Config::Chat::instance();
  Config::General* generalConfig = Config::General::instance();
  chatConfig->blockUpdates(true);
  generalConfig->blockUpdates(true);

  generalConfig->setUseDoubleReturn(chkUseDoubleReturn->isChecked());

  chatConfig->setAutoClose(chkAutoClose->isChecked());
  chatConfig->setSendFromClipboard(chkSendFromClipboard->isChecked());
  chatConfig->setMsgChatView(chkMsgChatView->isChecked());
  chatConfig->setChatVertSpacing(chkChatVertSpacing->isChecked());
  chatConfig->setChatAppendLineBreak(chkChatLineBreak->isChecked());
  chatConfig->setChatMsgStyle(cmbChatStyle->currentIndex());
  chatConfig->setChatDateFormat(cmbChatDateFormat->currentText());
  chatConfig->setHistMsgStyle(cmbHistStyle->currentIndex());
  chatConfig->setHistVertSpacing(chkHistVertSpacing->isChecked());
  chatConfig->setReverseHistory(chkHistReverse->isChecked());
  chatConfig->setHistDateFormat(cmbHistDateFormat->currentText());
  chatConfig->setRecvColor(btnColorRcv->colorName());
  chatConfig->setSentColor(btnColorSnt->colorName());
  chatConfig->setRecvHistoryColor(btnColorRcvHistory->colorName());
  chatConfig->setSentHistoryColor(btnColorSntHistory->colorName());
  chatConfig->setNoticeColor(btnColorNotice->colorName());
  chatConfig->setTabTypingColor(btnColorTypingLabel->colorName());
  chatConfig->setChatBackColor(btnColorChatBkg->colorName());
  chatConfig->setTabbedChatting(chkTabbedChatting->isChecked());
  chatConfig->setShowHistory(chkShowHistory->isChecked());
  chatConfig->setShowNotices(chkShowNotices->isChecked());
  chatConfig->setAutoPosReplyWin(chkAutoPosReplyWin->isChecked());
  chatConfig->setAutoSendThroughServer(chkAutoSendThroughServer->isChecked());
  chatConfig->setShowSendClose(chkShowSendClose->isChecked());
  chatConfig->setMsgWinSticky(chkMsgWinSticky->isChecked());
  chatConfig->setSingleLineChatMode(chkSingleLineChatMode->isChecked());
  chatConfig->setShowUserPic(chkShowUserPic->isChecked());
  chatConfig->setShowUserPicHidden(chkShowUserPicHidden->isChecked());

  gLicqDaemon->SetSendTypingNotification(chkSendTN->isChecked());

#ifdef USE_KDE
  if (cmbUrlViewer->currentText() == DEFAULT_URL_VIEWER)
    gLicqDaemon->setUrlViewer("none");
  else
#endif
    gLicqDaemon->setUrlViewer(cmbUrlViewer->currentText().toLocal8Bit());

  gLicqDaemon->SetTerminal(edtTerminal->text().toLocal8Bit());

  if (myDefaultEncodingCombo->currentIndex() > 0)
    chatConfig->setDefaultEncoding(UserCodec::encodingForName(myDefaultEncodingCombo->currentText()));
  else
    chatConfig->setDefaultEncoding(QByteArray());
  chatConfig->setShowAllEncodings(myShowAllEncodingsCheck->isChecked());

  chatConfig->blockUpdates(false);
  generalConfig->blockUpdates(false);
}
