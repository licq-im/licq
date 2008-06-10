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

#ifndef CONFIG_CHAT_H
#define CONFIG_CHAT_H

#include "config.h"

#include <QObject>
#include <QRect>

class CIniFile;

namespace LicqQtGui
{
namespace Config
{
/**
 * Chat and history configuration
 */
class Chat : public QObject
{
  Q_OBJECT

public:
  /**
   * Create the singleton instance
   *
   * @param parent Parent object
   */
  static void createInstance(QObject* parent = NULL);

  /**
   * Get the singleton instance
   *
   * @return The instance
   */
  static Chat* instance()
  { return myInstance; }

  /**
   * Constuctor
   */
  Chat(QObject* parent = 0);

  ~Chat() {}

  void blockUpdates(bool block);

  // Get functions
  bool autoClose() const { return myAutoClose; }
  unsigned short autoPopup() const { return myAutoPopup; }
  bool autoFocus() const { return myAutoFocus; }
  bool manualNewUser() const { return myManualNewUser; }
  bool sendFromClipboard() const { return mySendFromClipboard; }
  bool msgChatView() const { return myMsgChatView; }
  bool autoPosReplyWin() const { return myAutoPosReplyWin; }
  bool autoSendThroughServer() const { return myAutoSendThroughServer; }
  bool showDlgButtons() const { return myShowDlgButtons; }
  bool showAllEncodings() const { return myShowAllEncodings; }
  bool tabbedChatting() const { return myTabbedChatting; }
  bool showHistory() const { return myShowHistory; }
  bool showNotices() const { return myShowNotices; }
  bool showUserPic() const { return myShowUserPic; }
  bool showUserPicHidden() const { return myShowUserPicHidden; }
  bool popupAutoResponse() const { return myPopupAutoResponse; }
  bool chatVertSpacing() const { return myChatVertSpacing; }
  bool chatAppendLineBreak() const { return myChatAppendLineBreak; }
  bool flashTaskbar() const { return myFlashTaskbar; }
  bool msgWinSticky() const { return myMsgWinSticky; }
  bool singleLineChatMode() const { return mySingleLineChatMode; }
  bool checkSpelling() const { return myCheckSpelling; }
  bool histVertSpacing() const { return myHistVertSpacing; }
  bool reverseHistory() const { return myReverseHistory; }
  unsigned short chatMsgStyle() const { return myChatMsgStyle; }
  unsigned short histMsgStyle() const { return myHistMsgStyle; }
  QString chatDateFormat() const { return myChatDateFormat; }
  QString histDateFormat() const { return myHistDateFormat; }
  QString recvHistoryColor() const { return myRecvHistoryColor; }
  QString sentHistoryColor() const { return mySentHistoryColor; }
  QString recvColor() const { return myRecvColor; }
  QString sentColor() const { return mySentColor; }
  QString noticeColor() const { return myNoticeColor; }
  QString tabTypingColor() const { return myTabTypingColor; }
  QString chatBackColor() const { return myChatBackColor; }
  QRect dialogRect() const { return myDialogRect; }

public slots:
  /**
   * Load configuration from file
   */
  void loadConfiguration(CIniFile& iniFile);

  /**
   * Save configuration to file
   */
  void saveConfiguration(CIniFile& iniFile) const;

  // Set functions
  void setAutoClose(bool autoClose);
  void setAutoPopup(unsigned short autoPopup);
  void setAutoFocus(bool autoFocus);
  void setManualNewUser(bool manualNewUser);
  void setSendFromClipboard(bool sendFromClipboard);
  void setMsgChatView(bool msgChatView);
  void setAutoPosReplyWin(bool autoPosReplyWin);
  void setAutoSendThroughServer(bool autoSendThroughServer);
  void setShowDlgButtons(bool showDlgButtons);
  void setShowAllEncodings(bool showAllEncodings);
  void setTabbedChatting(bool tabbedChatting);
  void setShowHistory(bool showHistory);
  void setShowNotices(bool showNotices);
  void setShowUserPic(bool showUserPic);
  void setShowUserPicHidden(bool showUserPicHidden);
  void setPopupAutoResponse(bool popupAutoResponse);
  void setChatVertSpacing(bool chatVertSpacing);
  void setChatAppendLineBreak(bool chatAppendLineBreak);
  void setFlashTaskbar(bool flashTaskbar);
  void setMsgWinSticky(bool msgWinSticky);
  void setSingleLineChatMode(bool singleLineChatMode);
  void setCheckSpelling(bool checkSpelling);
  void setHistVertSpacing(bool histVertSpacing);
  void setReverseHistory(bool reverseHistory);
  void setChatMsgStyle(unsigned short chatMsgStyle);
  void setHistMsgStyle(unsigned short histMsgStyle);
  void setChatDateFormat(QString chatDateFormat);
  void setHistDateFormat(QString histDateFormat);
  void setRecvHistoryColor(QString recvHistoryColor);
  void setSentHistoryColor(QString sentHistoryColor);
  void setRecvColor(QString recvColor);
  void setSentColor(QString sentColor);
  void setNoticeColor(QString noticeColor);
  void setTabTypingColor(QString tabTypingColor);
  void setChatBackColor(QString chatBackColor);
  void setDialogRect(const QRect& geometry);

signals:
  void chatColorsChanged();

private:
  static Chat* myInstance;

  // Changes have been made that should trigger changed() signal
  bool myBlockUpdates;
  bool myColorsHaveChanged;

  bool myAutoFocus;
  unsigned short myAutoPopup;
  bool myAutoClose;
  bool myManualNewUser;
  bool mySendFromClipboard;
  bool myMsgChatView;
  bool myAutoPosReplyWin;
  bool myAutoSendThroughServer;
  bool myShowDlgButtons;
  bool myShowAllEncodings;
  bool myTabbedChatting;
  bool myShowHistory;
  bool myShowNotices;
  bool myChatVertSpacing;
  bool myChatAppendLineBreak;
  bool myMsgWinSticky;
  bool mySingleLineChatMode;
  bool myFlashTaskbar;
  bool myCheckSpelling;
  bool myHistVertSpacing;
  bool myReverseHistory;
  bool myShowUserPic;
  bool myShowUserPicHidden;
  bool myPopupAutoResponse;

  unsigned short myChatMsgStyle;
  unsigned short myHistMsgStyle;

  QString myChatDateFormat;
  QString myHistDateFormat;

  QString myRecvHistoryColor;
  QString mySentHistoryColor;
  QString myRecvColor;
  QString mySentColor;
  QString myNoticeColor;
  QString myTabTypingColor;
  QString myChatBackColor;

  QRect myDialogRect;

  void changeChatColors();
};

} // namespace Config
} // namespace LicqQtGui

#endif
