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

#ifndef SETTINGS_CHAT_H
#define SETTINGS_CHAT_H

#include <config.h>

#include <QObject>

class QCheckBox;
class QComboBox;
class QGridLayout;
class QGroupBox;
class QLabel;
class QLineEdit;
class QSpinBox;
class QVBoxLayout;
class QWidget;

namespace LicqQtGui
{
class ColorButton;
class HistoryView;
class SettingsDlg;
class TabWidget;

namespace Settings
{
class Chat : public QObject
{
  Q_OBJECT

public:
  Chat(SettingsDlg* parent);
  virtual ~Chat() {}

  void load();
  void apply();

private slots:
  void slot_useMsgChatView(bool);
  void slot_refresh_msgViewer();

private:
  /**
   * Setup the chat page.
   *
   * @return a widget with the chat settings
   */
  QWidget* createPageChat(QWidget* parent);

  /**
   * Setup the chat display page.
   *
   * @return a widget with the chat display settings
   */
  QWidget* createPageChatDisp(QWidget* parent);

  /**
   * Setup the history display page.
   *
   * @return a widget with the history display settings
   */
  QWidget* createPageHistDisp(QWidget* parent);

  // Widget for chat settings
  QVBoxLayout* layPageChat;
  QGroupBox* boxChat;
  QGroupBox* boxExtensions;
  QGridLayout* layChat;
  QGridLayout* layExtensions;
  QCheckBox* chkSendFromClipboard;
  QCheckBox* chkAutoPosReplyWin;
  QCheckBox* chkAutoSendThroughServer;
  QCheckBox* chkSingleLineChatMode;
  QCheckBox* chkUseDoubleReturn;
  QCheckBox* chkMsgChatView;
  QCheckBox* chkTabbedChatting;
  QCheckBox* chkSendTN;
  QCheckBox* chkMsgWinSticky;
  QCheckBox* chkAutoClose;
  QCheckBox* chkShowSendClose;
  QCheckBox* chkShowUserPic;
  QCheckBox* chkShowUserPicHidden;

  QGroupBox* myLocaleBox;
  QVBoxLayout* myLocaleLayout;
  QCheckBox* myShowAllEncodingsCheck;
  QLabel* myDefaultEncodingLabel;
  QComboBox* myDefaultEncodingCombo;

  QLabel* lblUrlViewer;
  QLabel* lblTerminal;
  QComboBox* cmbUrlViewer;
  QLineEdit* edtTerminal;

  // Widget for chat display settings
  QGridLayout* layPageChatDisp;
  QGroupBox* boxChatDisp;
  QGroupBox* boxChatColors;
  QVBoxLayout* layChatDisp;
  QGridLayout* layChatColors;
  QLabel* lblChatStyle;
  QLabel* lblChatDateFormat;
  QComboBox* cmbChatStyle;
  QComboBox* cmbChatDateFormat;
  QCheckBox* chkChatVertSpacing;
  QCheckBox* chkChatLineBreak;
  QCheckBox* chkShowHistory;
  QCheckBox* chkShowNotices;
  QLabel* lblColorRcv;
  QLabel* lblColorSnt;
  QLabel* lblColorRcvHistory;
  QLabel* lblColorSntHistory;
  QLabel* lblColorNotice;
  QLabel* lblColorTabLabel;
  QLabel* lblColorTypingLabel;
  QLabel* lblColorChatBkg;
  ColorButton* btnColorRcv;
  ColorButton* btnColorSnt;
  ColorButton* btnColorRcvHistory;
  ColorButton* btnColorSntHistory;
  ColorButton* btnColorNotice;
  ColorButton* btnColorTabLabel;
  ColorButton* btnColorTypingLabel;
  ColorButton* btnColorChatBkg;
  TabWidget* tabViewer;
  HistoryView* msgChatViewer;

  // Widget for history display settings
  QVBoxLayout* layPageHistDisp;
  QGroupBox* boxHistDisp;
  QGroupBox* boxHistPreview;
  QVBoxLayout* layHistDisp;
  QVBoxLayout* layHistPreview;
  QLabel* lblHistStyle;
  QLabel* lblHistDateFormat;
  QComboBox* cmbHistStyle;
  QCheckBox* chkHistVertSpacing;
  QCheckBox* chkHistReverse;
  QComboBox* cmbHistDateFormat;
  HistoryView* msgHistViewer;
};

} // namespace Settings
} // namespace LicqQtGui

#endif
