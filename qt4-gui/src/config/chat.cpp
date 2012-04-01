/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2012 Licq developers <licq-dev@googlegroups.com>
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

#include <QApplication>
#include <QDesktopWidget>

#include <licq/inifile.h>

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::Config::Chat */

Config::Chat* Config::Chat::myInstance = NULL;

void Config::Chat::createInstance(QObject* parent)
{
  myInstance = new Config::Chat(parent);
}

Config::Chat::Chat(QObject* parent)
  : QObject(parent),
    myBlockUpdates(false),
    myColorsHaveChanged(false)
{
  // Empty
}

void Config::Chat::loadConfiguration(Licq::IniFile& iniFile)
{
  std::string s;
#define GET_QSTRING(param, var, def) \
    iniFile.get(param, s, def); \
    var = QString::fromLatin1(s.c_str());

  iniFile.setSection("appearance");
  iniFile.get("ManualNewUser", myManualNewUser, false);
  iniFile.get("SendFromClipboard", mySendFromClipboard, true);
  iniFile.get("MsgChatView", myMsgChatView, true );
  iniFile.get("TabbedChatting", myTabbedChatting, true);
  bool oldShowHistory;
  iniFile.get("ShowHistory", oldShowHistory, true);
  iniFile.get("ShowHistoryCount", myShowHistoryCount, (oldShowHistory ? 5 : 0));
  iniFile.get("ShowHistoryTime", myShowHistoryTime, 0);
  iniFile.get("ShowNotices", myShowNotices, true);
  iniFile.get("AutoPosReplyWin", myAutoPosReplyWin, true);
  iniFile.get("AutoSendThroughServer", myAutoSendThroughServer, false);
  iniFile.get("ShowChatDlgButtons", myShowDlgButtons, true);
  iniFile.get("ChatMessageStyle", myChatMsgStyle, 0);
  iniFile.get("ChatVerticalSpacing", myChatVertSpacing, true);
  iniFile.get("ChatAppendLinebreak", myChatAppendLineBreak, false);
  iniFile.get("FlashTaskbar", myFlashTaskbar, true);
  iniFile.get("MsgWinSticky", myMsgWinSticky, false);
  iniFile.get("SingleLineChatMode", mySingleLineChatMode, false);
  iniFile.get("CheckSpellingEnabled", myCheckSpelling, false);
#ifdef HAVE_HUNSPELL
  GET_QSTRING("SpellingDictionary", mySpellingDictionary, "");
#endif
  iniFile.get("ShowUserPic", myShowUserPic, false);
  iniFile.get("ShowUserPicHidden", myShowUserPicHidden, false);
  iniFile.get("NoSoundInActiveChat", myNoSoundInActiveChat, false);
  iniFile.get("ChatDateHeader", myChatDateHeader, true);
  GET_QSTRING("DateFormat", myChatDateFormat, "hh:mm:ss");
  iniFile.get("HistoryMessageStyle", myHistMsgStyle, 0);
  iniFile.get("HistoryVerticalSpacing", myHistVertSpacing, true);
  iniFile.get("HistoryReverse", myReverseHistory, false);
  GET_QSTRING("HistoryDateFormat", myHistDateFormat, "hh:mm:ss");

  GET_QSTRING("ReceiveMessageColor", myRecvColor, "red");
  GET_QSTRING("ReceiveHistoryColor", myRecvHistoryColor, "lightpink");
  GET_QSTRING("SentMessageColor", mySentColor, "blue");
  GET_QSTRING("SentHistoryColor", mySentHistoryColor, "lightblue");
  GET_QSTRING("NoticeColor", myNoticeColor, "darkgreen");
  GET_QSTRING("TabOnTypingColor", myTabTypingColor, "yellow");
  GET_QSTRING("ChatBackground", myChatBackColor, "white");

  iniFile.setSection("functions");
  iniFile.get("AutoClose", myAutoClose, true);
  iniFile.get("AutoPopup", myAutoPopup, 0);
  iniFile.get("AutoPopupUrgentOnly", myAutoPopupUrgentOnly, false);
  iniFile.get("AutoFocus", myAutoFocus, true);
  iniFile.get("PopupAutoResponse", myPopupAutoResponse, true);

  iniFile.setSection("locale");
  iniFile.get("ShowAllEncodings", myShowAllEncodings, false);

  iniFile.setSection("geometry");
  int xPos, yPos, wVal, hVal;
  iniFile.get("EventDialog.X", xPos, 0);
  iniFile.get("EventDialog.Y", yPos, 0);
  iniFile.get("EventDialog.W", wVal, 0);
  iniFile.get("EventDialog.H", hVal, 0);
  if (xPos > QApplication::desktop()->width() - 16)
    xPos = 0;
  if (yPos > QApplication::desktop()->height() - 16)
    yPos = 0;
  myTabDialogRect.setRect(xPos, yPos, wVal, hVal);
  iniFile.get("ViewEventDialog.W", wVal, -1);
  iniFile.get("ViewEventDialog.H", hVal, -1);
  myViewDialogSize = QSize(wVal, hVal);
  iniFile.get("SendEventDialog.W", wVal, -1);
  iniFile.get("SendEventDialog.H", hVal, -1);
  mySendDialogSize = QSize(wVal, hVal);

#undef GET_QSTRING
}

void Config::Chat::saveConfiguration(Licq::IniFile& iniFile) const
{
  iniFile.setSection("appearance");
  iniFile.set("ManualNewUser", myManualNewUser);
  iniFile.set("SendFromClipboard", mySendFromClipboard);
  iniFile.set("MsgChatView", myMsgChatView);
  iniFile.set("TabbedChatting", myTabbedChatting);
  iniFile.set("ShowHistoryCount", myShowHistoryCount);
  iniFile.set("ShowHistoryTime", myShowHistoryTime);
  iniFile.set("ShowNotices", myShowNotices);
  iniFile.set("AutoPosReplyWin", myAutoPosReplyWin);
  iniFile.set("AutoSendThroughServer", myAutoSendThroughServer);
  iniFile.set("ShowChatDlgButtons", myShowDlgButtons);
  iniFile.set("FlashTaskbar", myFlashTaskbar);
  iniFile.set("MsgWinSticky", myMsgWinSticky);
  iniFile.set("SingleLineChatMode", mySingleLineChatMode);
  iniFile.set("CheckSpellingEnabled", myCheckSpelling);
#ifdef HAVE_HUNSPELL
  iniFile.set("SpellingDictionary", mySpellingDictionary.toLatin1());
#endif
  iniFile.set("ShowUserPic", myShowUserPic);
  iniFile.set("ShowUserPicHidden", myShowUserPicHidden);
  iniFile.set("NoSoundInActiveChat", myNoSoundInActiveChat);
  iniFile.set("ChatDateHeader", myChatDateHeader);

  iniFile.set("ChatMessageStyle", myChatMsgStyle);
  iniFile.set("ChatVerticalSpacing", myChatVertSpacing);
  iniFile.set("ChatAppendLinebreak", myChatAppendLineBreak);
  iniFile.set("ReceiveMessageColor", myRecvColor.toLatin1());
  iniFile.set("ReceiveHistoryColor", myRecvHistoryColor.toLatin1());
  iniFile.set("SentMessageColor", mySentColor.toLatin1());
  iniFile.set("SentHistoryColor", mySentHistoryColor.toLatin1());
  iniFile.set("NoticeColor", myNoticeColor.toLatin1());
  iniFile.set("TabOnTypingColor", myTabTypingColor.toLatin1());
  iniFile.set("ChatBackground", myChatBackColor.toLatin1());
  iniFile.set("DateFormat", myChatDateFormat.toLatin1());
  iniFile.set("HistoryMessageStyle", myHistMsgStyle);
  iniFile.set("HistoryVerticalSpacing", myHistVertSpacing);
  iniFile.set("HistoryReverse", myReverseHistory);
  iniFile.set("HistoryDateFormat", myHistDateFormat.toLatin1());

  iniFile.setSection("functions");
  iniFile.set("AutoClose", myAutoClose);
  iniFile.set("AutoPopup", myAutoPopup);
  iniFile.set("AutoPopupUrgentOnly", myAutoPopupUrgentOnly);
  iniFile.set("AutoFocus", myAutoFocus);
  iniFile.set("PopupAutoResponse", myPopupAutoResponse);

  iniFile.setSection("locale");
  iniFile.set("ShowAllEncodings", myShowAllEncodings);

  iniFile.setSection("geometry");
  iniFile.set("EventDialog.X", myTabDialogRect.x());
  iniFile.set("EventDialog.Y", myTabDialogRect.y());
  iniFile.set("EventDialog.W", myTabDialogRect.width());
  iniFile.set("EventDialog.H", myTabDialogRect.height());
  iniFile.set("ViewEventDialog.W", myViewDialogSize.width());
  iniFile.set("ViewEventDialog.H", myViewDialogSize.height());
  iniFile.set("SendEventDialog.W", mySendDialogSize.width());
  iniFile.set("SendEventDialog.H", mySendDialogSize.height());
}

void Config::Chat::blockUpdates(bool block)
{
  myBlockUpdates = block;

  if (block)
    return;

  if (myColorsHaveChanged)
  {
    myColorsHaveChanged = false;
    emit chatColorsChanged();
  }
}

void Config::Chat::setAutoClose(bool autoClose)
{
  if (autoClose == myAutoClose)
    return;

  myAutoClose = autoClose;
}

void Config::Chat::setAutoPopup(int autoPopup)
{
  if (autoPopup == myAutoPopup)
    return;

  myAutoPopup = autoPopup;
}

void Config::Chat::setAutoPopupUrgentOnly(bool autoPopupUrgentOnly)
{
  if (autoPopupUrgentOnly == myAutoPopupUrgentOnly)
    return;

  myAutoPopupUrgentOnly = autoPopupUrgentOnly;
}

void Config::Chat::setAutoFocus(bool autoFocus)
{
  if (autoFocus == myAutoFocus)
    return;

  myAutoFocus = autoFocus;
}

void Config::Chat::setManualNewUser(bool manualNewUser)
{
  if (manualNewUser == myManualNewUser)
    return;

  myManualNewUser = manualNewUser;
}

void Config::Chat::setSendFromClipboard(bool sendFromClipboard)
{
  if (sendFromClipboard == mySendFromClipboard)
    return;

  mySendFromClipboard = sendFromClipboard;
}

void Config::Chat::setMsgChatView(bool msgChatView)
{
  if (msgChatView == myMsgChatView)
    return;

  myMsgChatView = msgChatView;
}

void Config::Chat::setShowAllEncodings(bool showAllEncodings)
{
  if (showAllEncodings == myShowAllEncodings)
    return;

  myShowAllEncodings = showAllEncodings;
}

void Config::Chat::setTabbedChatting(bool tabbedChatting)
{
  if (tabbedChatting == myTabbedChatting)
    return;

  myTabbedChatting = tabbedChatting;
}

void Config::Chat::setShowHistoryCount(int showHistoryCount)
{
  if (showHistoryCount == myShowHistoryCount)
    return;

  myShowHistoryCount = showHistoryCount;
}

void Config::Chat::setShowHistoryTime(int showHistoryTime)
{
  if (showHistoryTime == myShowHistoryTime)
    return;

  myShowHistoryTime = showHistoryTime;
}

void Config::Chat::setShowNotices(bool showNotices)
{
  if (showNotices == myShowNotices)
    return;

  myShowNotices = showNotices;
}

void Config::Chat::setShowUserPic(bool showUserPic)
{
  if (showUserPic == myShowUserPic)
    return;

  myShowUserPic = showUserPic;
}

void Config::Chat::setShowUserPicHidden(bool showUserPicHidden)
{
  if (showUserPicHidden == myShowUserPicHidden)
    return;

  myShowUserPicHidden = showUserPicHidden;
}

void Config::Chat::setPopupAutoResponse(bool popupAutoResponse)
{
  if (popupAutoResponse == myPopupAutoResponse)
    return;

  myPopupAutoResponse = popupAutoResponse;
}

void Config::Chat::setAutoPosReplyWin(bool autoPosReplyWin)
{
  if (autoPosReplyWin == myAutoPosReplyWin)
    return;

  myAutoPosReplyWin = autoPosReplyWin;
}

void Config::Chat::setAutoSendThroughServer(bool autoSendThroughServer)
{
  if (autoSendThroughServer == myAutoSendThroughServer)
    return;

  myAutoSendThroughServer = autoSendThroughServer;
}

void Config::Chat::setShowDlgButtons(bool showDlgButtons)
{
  if (showDlgButtons == myShowDlgButtons)
    return;

  myShowDlgButtons = showDlgButtons;
}

void Config::Chat::setChatVertSpacing(bool chatVertSpacing)
{
  if (chatVertSpacing == myChatVertSpacing)
    return;

  myChatVertSpacing = chatVertSpacing;
}

void Config::Chat::setChatAppendLineBreak(bool chatAppendLineBreak)
{
  if (chatAppendLineBreak == myChatAppendLineBreak)
    return;

  myChatAppendLineBreak = chatAppendLineBreak;
}

void Config::Chat::setFlashTaskbar(bool flashTaskbar)
{
  if (flashTaskbar == myFlashTaskbar)
    return;

  myFlashTaskbar = flashTaskbar;
}

void Config::Chat::setMsgWinSticky(bool msgWinSticky)
{
  if (msgWinSticky == myMsgWinSticky)
    return;

  myMsgWinSticky = msgWinSticky;
}

void Config::Chat::setSingleLineChatMode(bool singleLineChatMode)
{
  if (singleLineChatMode == mySingleLineChatMode)
    return;

  mySingleLineChatMode = singleLineChatMode;
}

void Config::Chat::setCheckSpelling(bool checkSpelling)
{
  if (checkSpelling == myCheckSpelling)
    return;

  myCheckSpelling = checkSpelling;
}

#ifdef HAVE_HUNSPELL
void Config::Chat::setSpellingDictionary(const QString& spellingDictionary)
{
  if (spellingDictionary == mySpellingDictionary)
    return;

  mySpellingDictionary = spellingDictionary;
}
#endif

void Config::Chat::setHistVertSpacing(bool histVertSpacing)
{
  if (histVertSpacing == myHistVertSpacing)
    return;

  myHistVertSpacing = histVertSpacing;
}

void Config::Chat::setReverseHistory(bool reverseHistory)
{
  if (reverseHistory == myReverseHistory)
    return;

  myReverseHistory = reverseHistory;
}

void Config::Chat::setNoSoundInActiveChat(bool noSoundInActiveChat)
{
  if (noSoundInActiveChat == myNoSoundInActiveChat)
    return;

  myNoSoundInActiveChat = noSoundInActiveChat;
}

void Config::Chat::setChatDateHeader(bool b)
{
  if (b == myChatDateHeader)
    return;

  myChatDateHeader = b;
}

void Config::Chat::setChatMsgStyle(int chatMsgStyle)
{
  if (chatMsgStyle == myChatMsgStyle)
    return;

  myChatMsgStyle = chatMsgStyle;
}

void Config::Chat::setHistMsgStyle(int histMsgStyle)
{
  if (histMsgStyle == myHistMsgStyle)
    return;

  myHistMsgStyle = histMsgStyle;
}

void Config::Chat::setChatDateFormat(const QString& chatDateFormat)
{
  if (chatDateFormat == myChatDateFormat)
    return;

  myChatDateFormat = chatDateFormat;
}

void Config::Chat::setHistDateFormat(const QString& histDateFormat)
{
  if (histDateFormat == myHistDateFormat)
    return;

  myHistDateFormat = histDateFormat;
}

void Config::Chat::setRecvHistoryColor(const QString& recvHistoryColor)
{
  if (recvHistoryColor == myRecvHistoryColor)
    return;

  myRecvHistoryColor = recvHistoryColor;
  changeChatColors();
}

void Config::Chat::setSentHistoryColor(const QString& sentHistoryColor)
{
  if (sentHistoryColor == mySentHistoryColor)
    return;

  mySentHistoryColor = sentHistoryColor;
  changeChatColors();
}

void Config::Chat::setRecvColor(const QString& recvColor)
{
  if (recvColor == myRecvColor)
    return;

  myRecvColor = recvColor;
  changeChatColors();
}

void Config::Chat::setSentColor(const QString& sentColor)
{
  if (sentColor == mySentColor)
    return;

  mySentColor = sentColor;
  changeChatColors();
}

void Config::Chat::setNoticeColor(const QString& noticeColor)
{
  if (noticeColor == myNoticeColor)
    return;

  myNoticeColor = noticeColor;
  changeChatColors();
}

void Config::Chat::setTabTypingColor(const QString& tabTypingColor)
{
  if (tabTypingColor == myTabTypingColor)
    return;

  myTabTypingColor = tabTypingColor;
  changeChatColors();
}

void Config::Chat::setChatBackColor(const QString& chatBackColor)
{
  if (chatBackColor == myChatBackColor)
    return;

  myChatBackColor = chatBackColor;
  changeChatColors();
}

void Config::Chat::setTabDialogRect(const QRect& geometry)
{
  if (geometry.isValid())
    myTabDialogRect = geometry;
}

void Config::Chat::setSendDialogSize(const QSize& size)
{
  if (size.isValid())
    mySendDialogSize = size;
}

void Config::Chat::setViewDialogSize(const QSize& size)
{
  if (size.isValid())
    myViewDialogSize = size;
}

void Config::Chat::changeChatColors()
{
  if (myBlockUpdates)
    myColorsHaveChanged = true;
  else
    emit chatColorsChanged();
}
