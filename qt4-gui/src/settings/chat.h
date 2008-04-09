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
  void useMsgChatViewChanged(bool);
  void updatePreviews();

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
  QVBoxLayout* myPageChatLayout;
  QGroupBox* myChatBox;
  QGroupBox* myExtensionsBox;
  QGridLayout* myChatLayout;
  QGridLayout* myExtensionsLayout;
  QCheckBox* mySendFromClipboardCheck;
  QCheckBox* myAutoPosReplyWinCheck;
  QCheckBox* myAutoSendThroughServerCheck;
  QCheckBox* mySingleLineChatModeCheck;
  QCheckBox* myUseDoubleReturnCheck;
  QCheckBox* myMsgChatViewCheck;
  QCheckBox* myTabbedChattingCheck;
  QCheckBox* mySendTNCheck;
  QCheckBox* myMsgWinStickyCheck;
  QCheckBox* myAutoCloseCheck;
  QCheckBox* myShowSendCloseCheck;
  QCheckBox* myCheckSpelling;
  QCheckBox* myShowUserPicCheck;
  QCheckBox* myShowUserPicHiddenCheck;
  QCheckBox* myPopupAutoResponseCheck;

  QGroupBox* myLocaleBox;
  QVBoxLayout* myLocaleLayout;
  QCheckBox* myShowAllEncodingsCheck;
  QLabel* myDefaultEncodingLabel;
  QComboBox* myDefaultEncodingCombo;

  QLabel* myUrlViewerLabel;
  QLabel* myTerminalLabel;
  QComboBox* myUrlViewerCombo;
  QLineEdit* myTerminalEdit;

  // Widget for chat display settings
  QGridLayout* myPageChatDispLayout;
  QGroupBox* myChatDispBox;
  QGroupBox* myChatColorsBox;
  QVBoxLayout* myChatDispLayout;
  QGridLayout* myChatColorsLayout;
  QLabel* myChatStyleLabel;
  QLabel* myChatDateFormatLabel;
  QComboBox* myChatStyleCombo;
  QComboBox* myChatDateFormatCombo;
  QCheckBox* myChatVertSpacingCheck;
  QCheckBox* myChatLineBreakCheck;
  QCheckBox* myShowHistoryCheck;
  QCheckBox* myShowNoticesCheck;
  QLabel* myColorRcvLabel;
  QLabel* myColorSntLabel;
  QLabel* myColorRcvHistoryLabel;
  QLabel* myColorSntHistoryLabel;
  QLabel* myColorNoticeLabel;
  QLabel* myColorTabLabelLabel;
  QLabel* myColorTypingLabelLabel;
  QLabel* myColorChatBkgLabel;
  ColorButton* myColorRcvButton;
  ColorButton* myColorSntButton;
  ColorButton* myColorRcvHistoryButton;
  ColorButton* myColorSntHistoryButton;
  ColorButton* myColorNoticeButton;
  ColorButton* myColorTabLabelButton;
  ColorButton* myColorTypingLabelButton;
  ColorButton* myColorChatBkgButton;
  TabWidget* myChatTabs;
  HistoryView* myChatView;

  // Widget for history display settings
  QVBoxLayout* myPageHistDispLayout;
  QGroupBox* myHistDispBox;
  QGroupBox* myHistPreviewBox;
  QVBoxLayout* myHistDispLayout;
  QVBoxLayout* myHistPreviewLayout;
  QLabel* myHistStyleLabel;
  QLabel* myHistDateFormatLabel;
  QComboBox* myHistStyleCombo;
  QCheckBox* myHistVertSpacingCheck;
  QCheckBox* myHistReverseCheck;
  QComboBox* myHistDateFormatCombo;
  HistoryView* myHistoryView;
};

} // namespace Settings
} // namespace LicqQtGui

#endif
