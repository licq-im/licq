// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2010 Licq developers
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

#include <licq/contactlist/usermanager.h>
#include <licq/daemon.h>

#include "config/chat.h"
#include "config/general.h"

#include "core/gui-defines.h"

#include "helpers/usercodec.h"

#include "widgets/colorbutton.h"
#include "widgets/historyview.h"
#include "widgets/tabwidget.h"
#ifdef HAVE_HUNSPELL
# include "widgets/filenameedit.h"
#endif

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
  myPageChatLayout = new QVBoxLayout(w);
  myPageChatLayout->setContentsMargins(0, 0, 0, 0);

  myChatBox = new QGroupBox(tr("General Chat Options"));
  myChatLayout = new QGridLayout(myChatBox);

  myMsgChatViewCheck = new QCheckBox(tr("Chatmode messageview"));
  myMsgChatViewCheck->setToolTip(tr("Show the current chat history in Send Window"));
  connect(myMsgChatViewCheck, SIGNAL(toggled(bool)), SLOT(useMsgChatViewChanged(bool)));
  myChatLayout->addWidget(myMsgChatViewCheck, 0, 0);

  myTabbedChattingCheck = new QCheckBox(tr("Tabbed chatting"));
  myTabbedChattingCheck->setToolTip(tr("Use tabs in Send Window"));
  myChatLayout->addWidget(myTabbedChattingCheck, 1, 0);

  mySingleLineChatModeCheck = new QCheckBox(tr("Single line chat mode"));
  mySingleLineChatModeCheck->setToolTip(tr("Send messages with Return and insert new lines with (Ctrl|Shift)+Return, opposite of the normal mode"));
  myChatLayout->addWidget(mySingleLineChatModeCheck, 0, 1);

  myUseDoubleReturnCheck = new QCheckBox(tr("Use double return"));
  myUseDoubleReturnCheck->setToolTip(tr("Hitting Return twice will be used instead of Ctrl+Return\n"
      "to send messages and close input dialogs.\n"
      "Multiple new lines can be inserted with Ctrl+Return."));
  myChatLayout->addWidget(myUseDoubleReturnCheck, 1, 1);

  myShowSendCloseCheck = new QCheckBox(tr("Show Send/Close buttons"));
  myShowSendCloseCheck->setToolTip(tr("Show Send and Close buttons in the chat dialog."));
  myChatLayout->addWidget(myShowSendCloseCheck, 2, 0);

  myPopupAutoResponseCheck = new QCheckBox(tr("Popup auto response"));
  myPopupAutoResponseCheck->setToolTip(tr("Popup auto responses received when sending to contacts that are away."));
  myChatLayout->addWidget(myPopupAutoResponseCheck, 2, 1);

  myMsgWinStickyCheck = new QCheckBox(tr("Sticky message window(s)"));
  myMsgWinStickyCheck->setToolTip(tr("Makes the message window(s) visible on all desktops"));
  myChatLayout->addWidget(myMsgWinStickyCheck, 3, 0);

  myAutoCloseCheck = new QCheckBox(tr("Auto close function window"));
  myAutoCloseCheck->setToolTip(tr("Auto close the user function window after a successful event"));
  myChatLayout->addWidget(myAutoCloseCheck, 3, 1);

  mySendFromClipboardCheck = new QCheckBox(tr("Check clipboard For URIs/files"));
  mySendFromClipboardCheck->setToolTip(tr("When double-clicking on a user to send a message check for urls/files in the clipboard"));
  myChatLayout->addWidget(mySendFromClipboardCheck, 4, 0);

  myAutoPosReplyWinCheck = new QCheckBox(tr("Auto position the reply window"));
  myAutoPosReplyWinCheck->setToolTip(tr("Position a new reply window just underneath the message view window"));
  myChatLayout->addWidget(myAutoPosReplyWinCheck, 4, 1);

  myAutoSendThroughServerCheck = new QCheckBox(tr("Auto send through server"));
  myAutoSendThroughServerCheck->setToolTip(tr("Automatically send messages through the server if direct connection fails"));
  myChatLayout->addWidget(myAutoSendThroughServerCheck, 5, 0);

  mySendTNCheck = new QCheckBox(tr("Send typing notifications"));
  mySendTNCheck->setToolTip(tr("Send a notification to the user so they can see when you are typing a message to them"));
  myChatLayout->addWidget(mySendTNCheck, 5, 1);

  myShowUserPicCheck = new QCheckBox(tr("Show user picture"));
  myShowUserPicCheck->setToolTip(tr("Show user picture next to the input area"));
  myChatLayout->addWidget(myShowUserPicCheck, 6, 0);

  myShowUserPicHiddenCheck = new QCheckBox(tr("Minimize user picture"));
  myShowUserPicHiddenCheck->setToolTip(tr("Hide user picture upon opening"));
  myChatLayout->addWidget(myShowUserPicHiddenCheck, 6, 1);

#if defined USE_KDE or defined HAVE_HUNSPELL
  myCheckSpelling = new QCheckBox(tr("Check spelling"));
  myCheckSpelling->setToolTip(tr("Mark misspelled words as you type."));
  myChatLayout->addWidget(myCheckSpelling, 7, 0);
#endif

#ifdef HAVE_HUNSPELL
  QHBoxLayout* dictionaryLayout = new QHBoxLayout();
  myDictionaryLabel = new QLabel(tr("Dictionary file:"));
  myDictionaryLabel->setToolTip(tr("Dictionary file to use when checking spelling."));
  dictionaryLayout->addWidget(myDictionaryLabel);
  myDictionaryEdit = new FileNameEdit();
  myDictionaryEdit->setToolTip(myDictionaryLabel->toolTip());
  myDictionaryEdit->setFilter(tr("*.dic|Dictionary files for Hunspell\\/Myspell (*.dic)"));
  myDictionaryEdit->setDefaultPath(HUNSPELL_DICTS_DIR);
  myDictionaryLabel->setBuddy(myDictionaryEdit);
  dictionaryLayout->addWidget(myDictionaryEdit);
  myDictionaryLabel->setEnabled(false);
  myDictionaryEdit->setEnabled(false);
  connect(myCheckSpelling, SIGNAL(toggled(bool)), myDictionaryLabel, SLOT(setEnabled(bool)));
  connect(myCheckSpelling, SIGNAL(toggled(bool)), myDictionaryEdit, SLOT(setEnabled(bool)));
  myChatLayout->addLayout(dictionaryLayout, 7, 1);
#endif

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


  myExtensionsBox = new QGroupBox(tr("Extensions"));
  myExtensionsLayout = new QGridLayout(myExtensionsBox);
  myExtensionsLayout->setColumnStretch(1, 1);

  myTerminalLabel = new QLabel(tr("Terminal:"));
  myTerminalLabel->setToolTip(tr("The command to run to start your terminal program."));
  myExtensionsLayout->addWidget(myTerminalLabel, 0, 0);

  myTerminalEdit = new QLineEdit(tr("Terminal:"));
  myTerminalEdit->setToolTip(myTerminalLabel->toolTip());
  myTerminalLabel->setBuddy(myTerminalEdit);
  myExtensionsLayout->addWidget(myTerminalEdit, 0, 1);

  myPageChatLayout->addWidget(myChatBox);
  myPageChatLayout->addWidget(myLocaleBox);
  myPageChatLayout->addWidget(myExtensionsBox);
  myPageChatLayout->addStretch(1);

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
  myPageChatDispLayout = new QGridLayout(w);
  myPageChatDispLayout->setContentsMargins(0, 0, 0, 0);
  myPageChatDispLayout->setRowStretch(1, 1);

  myChatDispBox = new QGroupBox(tr("Chat Display"));
  myChatDispLayout = new QVBoxLayout(myChatDispBox);

  QHBoxLayout* myChatLayoutStyle = new QHBoxLayout();
  myChatStyleLabel = new QLabel(tr("Style:"));
  myChatLayoutStyle->addWidget(myChatStyleLabel);
  myChatStyleCombo = new QComboBox();
  myChatStyleCombo->addItems(HistoryView::getStyleNames(false));
  myChatStyleLabel->setBuddy(myChatStyleCombo);
  connect(myChatStyleCombo, SIGNAL(activated(int)), SLOT(updatePreviews()));
  myChatLayoutStyle->addWidget(myChatStyleCombo);
  myChatDispLayout->addLayout(myChatLayoutStyle);

  QHBoxLayout* myChatLayoutDateFormat = new QHBoxLayout();
  myChatDateFormatLabel = new QLabel(tr("Date format:"));
  myChatDateFormatLabel->setToolTip(tr(helpDateFormat));
  myChatLayoutDateFormat->addWidget(myChatDateFormatLabel);
  myChatDateFormatCombo = new QComboBox();
  myChatDateFormatCombo->setEditable(true);
  myChatDateFormatCombo->setToolTip(myChatDateFormatLabel->toolTip());
  myChatDateFormatLabel->setBuddy(myChatDateFormatCombo);
  for(int i = 0; i < dateFormatsLength; ++i)
    myChatDateFormatCombo->addItem(dateFormats[i]);
  connect(myChatDateFormatCombo, SIGNAL(activated(int)), SLOT(updatePreviews()));
  connect(myChatDateFormatCombo, SIGNAL(editTextChanged(const QString&)), SLOT(updatePreviews()));
  myChatLayoutDateFormat->addWidget(myChatDateFormatCombo);
  myChatDispLayout->addLayout(myChatLayoutDateFormat);

  myChatVertSpacingCheck = new QCheckBox(tr("Insert vertical spacing"));
  connect(myChatVertSpacingCheck, SIGNAL(toggled(bool)), SLOT(updatePreviews()));
  myChatVertSpacingCheck->setToolTip(tr("Insert extra space between messages."));
  myChatDispLayout->addWidget(myChatVertSpacingCheck);

  myChatLineBreakCheck = new QCheckBox(tr("Insert horizontal line"));
  connect(myChatLineBreakCheck, SIGNAL(toggled(bool)), SLOT(updatePreviews()));
  myChatLineBreakCheck->setToolTip(tr("Insert a line between each message."));
  myChatDispLayout->addWidget(myChatLineBreakCheck);

  myShowHistoryCheck = new QCheckBox(tr("Show recent messages"));
  myShowHistoryCheck->setToolTip(tr("Show the last 5 messages when a Send Window is opened"));
  connect(myShowHistoryCheck, SIGNAL(toggled(bool)), SLOT(updatePreviews()));
  myChatDispLayout->addWidget(myShowHistoryCheck);

  myShowNoticesCheck = new QCheckBox(tr("Show join/left notices"));
  myShowNoticesCheck->setToolTip(tr("Show a notice in the chat window when a user joins or leaves the conversation."));
  connect(myShowNoticesCheck, SIGNAL(toggled(bool)), SLOT(updatePreviews()));
  myChatDispLayout->addWidget(myShowNoticesCheck);

  myChatDispLayout->addStretch(1);


  myChatColorsBox = new QGroupBox(tr("Colors"));
  myChatColorsLayout = new QGridLayout(myChatColorsBox);

  myColorRcvLabel = new QLabel(tr("Message received:"));
  myChatColorsLayout->addWidget(myColorRcvLabel, 0, 0);
  myColorRcvButton = new ColorButton();
  connect(myColorRcvButton, SIGNAL(changed(const QColor&)), SLOT(updatePreviews()));
  myChatColorsLayout->addWidget(myColorRcvButton, 0, 1);

  myColorSntLabel = new QLabel(tr("Message sent:"));
  myChatColorsLayout->addWidget(myColorSntLabel, 1, 0);
  myColorSntButton = new ColorButton();
  connect(myColorSntButton, SIGNAL(changed(const QColor&)), SLOT(updatePreviews()));
  myChatColorsLayout->addWidget(myColorSntButton, 1, 1);

  myColorRcvHistoryLabel = new QLabel(tr("History received:"));
  myChatColorsLayout->addWidget(myColorRcvHistoryLabel, 2, 0);
  myColorRcvHistoryButton = new ColorButton();
  connect(myColorRcvHistoryButton, SIGNAL(changed(const QColor&)), SLOT(updatePreviews()));
  myChatColorsLayout->addWidget(myColorRcvHistoryButton, 2, 1);

  myColorSntHistoryLabel = new QLabel(tr("History sent:"));
  myChatColorsLayout->addWidget(myColorSntHistoryLabel, 3, 0);
  myColorSntHistoryButton = new ColorButton();
  connect(myColorSntHistoryButton, SIGNAL(changed(const QColor&)), SLOT(updatePreviews()));
  myChatColorsLayout->addWidget(myColorSntHistoryButton, 3, 1);

  myColorNoticeLabel = new QLabel(tr("Notice:"));
  myChatColorsLayout->addWidget(myColorNoticeLabel, 4, 0);
  myColorNoticeButton = new ColorButton();
  connect(myColorNoticeButton, SIGNAL(changed(const QColor&)), SLOT(updatePreviews()));
  myChatColorsLayout->addWidget(myColorNoticeButton, 4, 1);

  myColorTypingLabelLabel = new QLabel(tr("Typing notification color:"));
  myChatColorsLayout->addWidget(myColorTypingLabelLabel, 5, 0);
  myColorTypingLabelButton = new ColorButton();
  connect(myColorTypingLabelButton, SIGNAL(changed(const QColor&)), SLOT(updatePreviews()));
  myChatColorsLayout->addWidget(myColorTypingLabelButton, 5, 1);

  myColorChatBkgLabel = new QLabel(tr("Background color:"));
  myChatColorsLayout->addWidget(myColorChatBkgLabel, 6, 0);
  myColorChatBkgButton = new ColorButton();
  connect(myColorChatBkgButton, SIGNAL(changed(const QColor&)), SLOT(updatePreviews()));
  myChatColorsLayout->addWidget(myColorChatBkgButton, 6, 1);

  myChatColorsLayout->setRowStretch(7, 1);

  myChatTabs = new TabWidget(w);
  myChatView = new HistoryView(false, Licq::UserId(), myChatTabs);
  myChatTabs->addTab(myChatView, "Marge");

  myPageChatDispLayout->addWidget(myChatDispBox, 0, 0);
  myPageChatDispLayout->addWidget(myChatColorsBox, 0, 1);
  myPageChatDispLayout->addWidget(myChatTabs, 1, 0, 1, 2);

  return w;
}

QWidget* Settings::Chat::createPageHistDisp(QWidget* parent)
{
  QWidget* w = new QWidget(parent);
  myPageHistDispLayout = new QVBoxLayout(w);
  myPageHistDispLayout->setContentsMargins(0, 0, 0, 0);

  myHistDispBox = new QGroupBox(tr("History Display"));
  myHistDispLayout = new QVBoxLayout(myHistDispBox);

  QHBoxLayout* myHistStyleLayout = new QHBoxLayout();
  myHistStyleLabel = new QLabel(tr("Style:"));
  myHistStyleLayout->addWidget(myHistStyleLabel);
  myHistStyleCombo = new QComboBox();
  myHistStyleCombo->addItems(HistoryView::getStyleNames(true));
  myHistStyleLabel->setBuddy(myHistStyleCombo);
  connect(myHistStyleCombo, SIGNAL(activated(int)), SLOT(updatePreviews()));
  myHistStyleLayout->addWidget(myHistStyleCombo);
  myHistDispLayout->addLayout(myHistStyleLayout);

  QHBoxLayout* myHistDateFormatLayout = new QHBoxLayout();
  myHistDateFormatLabel = new QLabel(tr("Date format:"));
  myHistDateFormatLabel->setToolTip(tr(helpDateFormat));
  myHistDateFormatLayout->addWidget(myHistDateFormatLabel);
  myHistDateFormatCombo = new QComboBox();
  myHistDateFormatCombo->setEditable(true);
  for(int i = 0; i < dateFormatsLength; ++i)
    myHistDateFormatCombo->addItem(dateFormats[i]);
  myHistDateFormatCombo->setToolTip(myHistDateFormatLabel->toolTip());
  myHistDateFormatLabel->setBuddy(myHistDateFormatCombo);
  connect(myHistDateFormatCombo, SIGNAL(activated(int)), SLOT(updatePreviews()));
  connect(myHistDateFormatCombo, SIGNAL(editTextChanged(const QString&)), SLOT(updatePreviews()));
  myHistDateFormatLayout->addWidget(myHistDateFormatCombo);
  myHistDispLayout->addLayout(myHistDateFormatLayout);

  QHBoxLayout* myHistOptsLayout = new QHBoxLayout();
  myHistVertSpacingCheck = new QCheckBox(tr("Insert vertical spacing"));
  connect(myHistVertSpacingCheck, SIGNAL(toggled(bool)), SLOT(updatePreviews()));
  myHistVertSpacingCheck->setToolTip(tr("Insert extra space between messages."));
  myHistOptsLayout->addWidget(myHistVertSpacingCheck);

  myHistReverseCheck = new QCheckBox(tr("Reverse history"));
  connect(myHistReverseCheck, SIGNAL(toggled(bool)), SLOT(updatePreviews()));
  myHistReverseCheck->setToolTip(tr("Put recent messages on top."));
  myHistOptsLayout->addWidget(myHistReverseCheck);
  myHistDispLayout->addLayout(myHistOptsLayout);

  myHistPreviewBox = new QGroupBox(tr("Preview"));
  myHistPreviewLayout = new QVBoxLayout(myHistPreviewBox);

  myHistoryView = new HistoryView(true);
  myHistPreviewLayout->addWidget(myHistoryView);

  myPageHistDispLayout->addWidget(myHistDispBox);
  myPageHistDispLayout->addWidget(myHistPreviewBox);

  return w;
}

void Settings::Chat::useMsgChatViewChanged(bool b)
{
  if (!b)
  {
    myTabbedChattingCheck->setChecked(false);
    myShowHistoryCheck->setChecked(false);
    myShowNoticesCheck->setEnabled(false);
  }

  myTabbedChattingCheck->setEnabled(b);
  myShowHistoryCheck->setEnabled(b);
  myShowNoticesCheck->setEnabled(b);
}

void Settings::Chat::updatePreviews()
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

  myChatView->setChatConfig(myChatStyleCombo->currentIndex(), myChatDateFormatCombo->currentText(),
      myChatVertSpacingCheck->isChecked(), myChatLineBreakCheck->isChecked(), myShowNoticesCheck->isChecked());

  myChatView->setColors(myColorChatBkgButton->colorName(), myColorRcvButton->colorName(),
      myColorSntButton->colorName(), myColorRcvHistoryButton->colorName(),
      myColorSntHistoryButton->colorName(), myColorNoticeButton->colorName());
  myChatTabs->setTabColor(myChatView, myColorTypingLabelButton->color());

  myHistoryView->setHistoryConfig(myHistStyleCombo->currentIndex(), myHistDateFormatCombo->currentText(),
      myHistVertSpacingCheck->isChecked(), myHistReverseCheck->isChecked());
  myHistoryView->setColors(myColorChatBkgButton->colorName(), myColorRcvButton->colorName(), myColorSntButton->colorName());

  myChatView->clear();
  myHistoryView->clear();

  QDateTime msgDate = date;
  for (unsigned int i = 0; i<7; i++)
  {
    if (i < 2 && myShowHistoryCheck->isChecked() == false)
      continue;

    myChatView->addMsg(i%2 == 0, (i<2),
          QString(""),
          msgDate,
          true, false, false, false,
          names[i % 2],
          MLView::toRichText(tr(msgs[i]), true, true));

    myHistoryView->addMsg(i%2 == 0, false,
          QString(""),
          msgDate,
          true, false, false, false,
          names[i % 2],
          MLView::toRichText(tr(msgs[i]), true, true));

    msgDate = msgDate.addSecs(i + 12);
  }
  msgDate = msgDate.addSecs(12);
  myChatView->addNotice(msgDate, MLView::toRichText(tr(msgs[7]), true, true));

  myHistoryView->updateContent();
}

void Settings::Chat::load()
{
  Config::Chat* chatConfig = Config::Chat::instance();
  Config::General* generalConfig = Config::General::instance();

  myUseDoubleReturnCheck->setChecked(generalConfig->useDoubleReturn());

  myAutoCloseCheck->setChecked(chatConfig->autoClose());
  mySendFromClipboardCheck->setChecked(chatConfig->sendFromClipboard());
  myMsgChatViewCheck->setChecked(chatConfig->msgChatView());
  myChatDateFormatCombo->lineEdit()->setText(chatConfig->chatDateFormat());
  myChatVertSpacingCheck->setChecked(chatConfig->chatVertSpacing());
  myChatLineBreakCheck->setChecked(chatConfig->chatAppendLineBreak());
  myChatStyleCombo->setCurrentIndex(chatConfig->chatMsgStyle());
  myHistStyleCombo->setCurrentIndex(chatConfig->histMsgStyle());
  myHistVertSpacingCheck->setChecked(chatConfig->histVertSpacing());
  myHistReverseCheck->setChecked(chatConfig->reverseHistory());
  myHistDateFormatCombo->lineEdit()->setText(chatConfig->histDateFormat());
  myColorRcvButton->setColor(chatConfig->recvColor());
  myColorSntButton->setColor(chatConfig->sentColor());
  myColorRcvHistoryButton->setColor(chatConfig->recvHistoryColor());
  myColorSntHistoryButton->setColor(chatConfig->sentHistoryColor());
  myColorNoticeButton->setColor(chatConfig->noticeColor());
  myColorTypingLabelButton->setColor(chatConfig->tabTypingColor());
  myColorChatBkgButton->setColor(chatConfig->chatBackColor());
  myAutoPosReplyWinCheck->setChecked(chatConfig->autoPosReplyWin());
  myAutoSendThroughServerCheck->setChecked(chatConfig->autoSendThroughServer());
  myShowSendCloseCheck->setChecked(chatConfig->showDlgButtons());
#if defined USE_KDE or defined HAVE_HUNSPELL
  myCheckSpelling->setChecked(chatConfig->checkSpelling());
#endif
#ifdef HAVE_HUNSPELL
  myDictionaryEdit->setFileName(chatConfig->spellingDictionary());
#endif
  myMsgWinStickyCheck->setChecked(chatConfig->msgWinSticky());
  mySingleLineChatModeCheck->setChecked(chatConfig->singleLineChatMode());
  myTabbedChattingCheck->setChecked(chatConfig->tabbedChatting());
  myShowHistoryCheck->setChecked(chatConfig->showHistory());
  myShowNoticesCheck->setChecked(chatConfig->showNotices());
  myShowUserPicCheck->setChecked(chatConfig->showUserPic());
  myShowUserPicHiddenCheck->setChecked(chatConfig->showUserPicHidden());
  myPopupAutoResponseCheck->setChecked(chatConfig->popupAutoResponse());

  if (!chatConfig->msgChatView())
  {
    myTabbedChattingCheck->setEnabled(false);
    myShowHistoryCheck->setEnabled(false);
    myShowNoticesCheck->setChecked(false);
  }

  mySendTNCheck->setChecked(Licq::gDaemon.sendTypingNotification());

  QByteArray defaultEncoding = Licq::gUserManager.defaultUserEncoding().c_str();
  if (defaultEncoding.isEmpty())
    myDefaultEncodingCombo->setCurrentIndex(0);
  else
  {
    for (int i = 1; i < myDefaultEncodingCombo->count(); i++)
    {
      if (UserCodec::encodingForName(myDefaultEncodingCombo->itemText(i)) == defaultEncoding)
      {
        myDefaultEncodingCombo->setCurrentIndex(i);
        break;
      }
    }
  }
  myShowAllEncodingsCheck->setChecked(chatConfig->showAllEncodings());

  myTerminalEdit->setText(Licq::gDaemon.terminal().empty() ?
      tr("none") : QString(Licq::gDaemon.terminal().c_str()));

  updatePreviews();
}

void Settings::Chat::apply()
{
  Config::Chat* chatConfig = Config::Chat::instance();
  Config::General* generalConfig = Config::General::instance();
  chatConfig->blockUpdates(true);
  generalConfig->blockUpdates(true);

  generalConfig->setUseDoubleReturn(myUseDoubleReturnCheck->isChecked());

  chatConfig->setAutoClose(myAutoCloseCheck->isChecked());
  chatConfig->setSendFromClipboard(mySendFromClipboardCheck->isChecked());
  chatConfig->setMsgChatView(myMsgChatViewCheck->isChecked());
  chatConfig->setChatVertSpacing(myChatVertSpacingCheck->isChecked());
  chatConfig->setChatAppendLineBreak(myChatLineBreakCheck->isChecked());
  chatConfig->setChatMsgStyle(myChatStyleCombo->currentIndex());
  chatConfig->setChatDateFormat(myChatDateFormatCombo->currentText());
  chatConfig->setHistMsgStyle(myHistStyleCombo->currentIndex());
  chatConfig->setHistVertSpacing(myHistVertSpacingCheck->isChecked());
  chatConfig->setReverseHistory(myHistReverseCheck->isChecked());
  chatConfig->setHistDateFormat(myHistDateFormatCombo->currentText());
  chatConfig->setRecvColor(myColorRcvButton->colorName());
  chatConfig->setSentColor(myColorSntButton->colorName());
  chatConfig->setRecvHistoryColor(myColorRcvHistoryButton->colorName());
  chatConfig->setSentHistoryColor(myColorSntHistoryButton->colorName());
  chatConfig->setNoticeColor(myColorNoticeButton->colorName());
  chatConfig->setTabTypingColor(myColorTypingLabelButton->colorName());
  chatConfig->setChatBackColor(myColorChatBkgButton->colorName());
  chatConfig->setTabbedChatting(myTabbedChattingCheck->isChecked());
  chatConfig->setShowHistory(myShowHistoryCheck->isChecked());
  chatConfig->setShowNotices(myShowNoticesCheck->isChecked());
  chatConfig->setAutoPosReplyWin(myAutoPosReplyWinCheck->isChecked());
  chatConfig->setAutoSendThroughServer(myAutoSendThroughServerCheck->isChecked());
  chatConfig->setShowDlgButtons(myShowSendCloseCheck->isChecked());
#if defined USE_KDE or defined HAVE_HUNSPELL
  chatConfig->setCheckSpelling(myCheckSpelling->isChecked());
#endif
#ifdef HAVE_HUNSPELL
  chatConfig->setSpellingDictionary(myDictionaryEdit->fileName());
#endif
  chatConfig->setMsgWinSticky(myMsgWinStickyCheck->isChecked());
  chatConfig->setSingleLineChatMode(mySingleLineChatModeCheck->isChecked());
  chatConfig->setShowUserPic(myShowUserPicCheck->isChecked());
  chatConfig->setShowUserPicHidden(myShowUserPicHiddenCheck->isChecked());
  chatConfig->setPopupAutoResponse(myPopupAutoResponseCheck->isChecked());

  Licq::gDaemon.setSendTypingNotification(mySendTNCheck->isChecked());

  Licq::gDaemon.setTerminal(myTerminalEdit->text().toLocal8Bit().data());

  if (myDefaultEncodingCombo->currentIndex() > 0)
    Licq::gUserManager.setDefaultUserEncoding(UserCodec::encodingForName(myDefaultEncodingCombo->currentText()).data());
  else
    Licq::gUserManager.setDefaultUserEncoding("");
  chatConfig->setShowAllEncodings(myShowAllEncodingsCheck->isChecked());

  chatConfig->blockUpdates(false);
  generalConfig->blockUpdates(false);
}
