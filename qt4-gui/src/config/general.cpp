// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2009 Licq developers
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

#include "general.h"

#include "config.h"

#include <QApplication>
#include <QDesktopWidget>

#ifdef USE_KDE
#include <KDE/KGlobalSettings>
#else
#include <QStyle>
#endif

#include <licq_file.h>

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::Config::General */

Config::General* Config::General::myInstance = NULL;

void Config::General::createInstance(QObject* parent)
{
  myInstance = new Config::General(parent);
}

Config::General::General(QObject* parent)
  : QObject(parent),
    myMainwinHasChanged(false),
    myDockHasChanged(false),
    myDockModeHasChanged(false),
    myFontHasChanged(false),
    myStyleHasChanged(false),
    myBlockUpdates(false)
{
#ifdef USE_KDE
  myDefaultFont = KGlobalSettings::generalFont();
#else
  myDefaultFont = qApp->font();
  myDefaultStyle = qApp->style()->objectName();
#endif

  myDefaultFixedFont = QFont(myDefaultFont);
  myDefaultFixedFont.setFamily("Monospace");
}

void Config::General::loadConfiguration(CIniFile& iniFile)
{
  char szTemp[255];

  iniFile.SetSection("functions");
  iniFile.ReadStr("MsgPopupKey", szTemp, "none");
  myMsgPopupKey = (strcmp(szTemp, "none") != 0 ? QString::fromLatin1(szTemp) : QString());
  iniFile.ReadBool("DelayStatusChange", myDelayStatusChange, false);

  iniFile.SetSection("appearance");
  iniFile.ReadBool("UseDoubleReturn", myUseDoubleReturn, false);

#ifndef USE_KDE
  iniFile.ReadStr("QtStyle", szTemp, "default");
  if (strcmp(szTemp, "default") != 0)
    setGuiStyle(szTemp);
#endif

  iniFile.ReadStr("Font", szTemp, "default");
  if (strcmp(szTemp, "default") == 0)
    szTemp[0] = '\0';
  setNormalFont(szTemp);
  iniFile.ReadStr("EditFont", szTemp, "default");
  if (strcmp(szTemp, "default") == 0)
    szTemp[0] = '\0';
  setEditFont(szTemp);
  iniFile.ReadStr("HistoryFont", szTemp, "default");
  if (strcmp(szTemp, "default") == 0)
    szTemp[0] = '\0';
  setHistoryFont(szTemp);
  iniFile.ReadStr("FixedFont", szTemp, "default");
  if (strcmp(szTemp, "default") == 0)
    szTemp[0] = '\0';
  setFixedFont(szTemp);

  iniFile.ReadBool("InMiniMode", myMiniMode, false);
  iniFile.ReadBool("ShowGroupIfNoMsg", myShowGroupIfNoMsg, true);
  iniFile.ReadBool("BoldOnMsg", myBoldOnMsg, true);
  iniFile.ReadBool("EnableMainwinMouseMovement", myMainwinDraggable, true);
  iniFile.ReadBool("MainWinSticky", myMainwinSticky, false);
  iniFile.ReadBool("AutoRaise", myAutoRaiseMainwin, true);
  iniFile.ReadBool("Hidden", myMainwinStartHidden, false);

  unsigned short dockMode;
  iniFile.ReadNum("UseDock", dockMode, DockTray);
  myDockMode = static_cast<DockMode>(dockMode);
#ifndef USE_KDE
  iniFile.ReadBool("Dock64x48", myDefaultIconFortyEight, false);
  char szDockTheme[64];
  iniFile.ReadStr("DockTheme", szDockTheme, "");
  myThemedIconTheme = szDockTheme;
#endif
  iniFile.ReadBool("TrayBlink", myTrayBlink, true);
  iniFile.ReadBool("TrayMsgOnlineNotify", myTrayMsgOnlineNotify, true);

  iniFile.SetSection("startup");
  iniFile.ReadNum("Logon", myAutoLogon, 0);
  if (myAutoLogon > 16)
    myAutoLogon = 0;
  iniFile.ReadNum("AutoAway", myAutoAwayTime, 5);
  iniFile.ReadNum("AutoNA", myAutoNaTime, 10);
  iniFile.ReadNum("AutoOffline", myAutoOfflineTime, 0);
  iniFile.ReadNum("AutoAwayMess", myAutoAwayMess, 0);
  iniFile.ReadNum("AutoNAMess", myAutoNaMess, 0);

  iniFile.SetSection("geometry");
  short xPos, yPos, wVal, hVal;
  iniFile.ReadNum("MainWindow.X", xPos, 0);
  iniFile.ReadNum("MainWindow.Y", yPos, 0);
  iniFile.ReadNum("MainWindow.W", wVal, 0);
  iniFile.ReadNum("MainWindow.H", hVal, 0);
  if (xPos > QApplication::desktop()->width() - 16)
    xPos = 0;
  if (yPos > QApplication::desktop()->height() - 16)
    yPos = 0;
  myMainwinRect.setRect(xPos, yPos, wVal, hVal);

  emit msgPopupKeyChanged(myMsgPopupKey);
  emit mainwinChanged();
  emit dockModeChanged();
  emit fontChanged();
  emit styleChanged();
}

void Config::General::saveConfiguration(CIniFile& iniFile) const
{
  iniFile.SetSection("functions");
  iniFile.WriteStr("MsgPopupKey", myMsgPopupKey.isEmpty() ? "none" : myMsgPopupKey.toLatin1());
  iniFile.WriteBool("DelayStatusChange", myDelayStatusChange);

  iniFile.SetSection("appearance");
  iniFile.WriteBool("UseDoubleReturn", myUseDoubleReturn);

#ifndef USE_KDE
  QString currentStyle = qApp->style()->objectName();
  iniFile.WriteStr("QtStyle", currentStyle.isEmpty() ||
      currentStyle == myDefaultStyle ? "default" : currentStyle.toLatin1());
#endif

  iniFile.WriteStr("Font", qApp->font() == myDefaultFont ?
      "default" : qApp->font().toString().toLatin1());
  iniFile.WriteStr("EditFont", myEditFont == myDefaultFont ?
      "default" : myEditFont.toString().toLatin1());
  iniFile.WriteStr("HistoryFont", myHistoryFont == myDefaultFont ?
      "default" : myHistoryFont.toString().toLatin1());
  iniFile.WriteStr("FixedFont", myFixedFont == myDefaultFixedFont ?
      "default" : myFixedFont.toString().toLatin1());

  iniFile.WriteBool("InMiniMode", myMiniMode);
  iniFile.WriteBool("ShowGroupIfNoMsg", myShowGroupIfNoMsg);
  iniFile.WriteBool("BoldOnMsg", myBoldOnMsg);
  iniFile.WriteBool("EnableMainwinMouseMovement", myMainwinDraggable);
  iniFile.WriteBool("MainWinSticky", myMainwinSticky);
  iniFile.WriteBool("AutoRaise", myAutoRaiseMainwin);
  iniFile.WriteBool("Hidden", myMainwinStartHidden);

  iniFile.WriteNum("UseDock", static_cast<unsigned short>(myDockMode));
#ifndef USE_KDE
  iniFile.WriteBool("Dock64x48", myDefaultIconFortyEight);
  iniFile.WriteStr("DockTheme", myThemedIconTheme.toLatin1());
#endif
  iniFile.WriteBool("TrayBlink", myTrayBlink);
  iniFile.WriteBool("TrayMsgOnlineNotify", myTrayMsgOnlineNotify);

  iniFile.SetSection("startup");
  iniFile.WriteNum("Logon", myAutoLogon);
  iniFile.WriteNum("AutoAway", myAutoAwayTime);
  iniFile.WriteNum("AutoNA", myAutoNaTime);
  iniFile.WriteNum("AutoOffline", myAutoOfflineTime);
  iniFile.WriteNum("AutoAwayMess", myAutoAwayMess);
  iniFile.WriteNum("AutoNAMess", myAutoNaMess);

  iniFile.SetSection("geometry");
  iniFile.WriteNum("MainWindow.X", static_cast<short>(myMainwinRect.x()));
  iniFile.WriteNum("MainWindow.Y", static_cast<short>(myMainwinRect.y()));
  iniFile.WriteNum("MainWindow.W", static_cast<short>(myMainwinRect.width()));
  iniFile.WriteNum("MainWindow.H", static_cast<short>(myMainwinRect.height()));
}

void Config::General::blockUpdates(bool block)
{
  if ((myBlockUpdates = block))
    return;

  if (myMainwinHasChanged)
  {
    myMainwinHasChanged = false;
    emit mainwinChanged();
  }
  if (myDockModeHasChanged)
  {
    myDockModeHasChanged = false;
    myDockHasChanged = false;
    emit dockModeChanged();
  }
  if (myDockHasChanged)
  {
    myDockHasChanged = false;
    emit dockChanged();
  }
  if (myFontHasChanged)
  {
    myFontHasChanged = false;
    emit fontChanged();
  }
  if (myStyleHasChanged)
  {
    myStyleHasChanged = false;
    emit styleChanged();
  }
}

void Config::General::setUseDoubleReturn(bool useDoubleReturn)
{
  if (useDoubleReturn == myUseDoubleReturn)
    return;

  myUseDoubleReturn = useDoubleReturn;
}

void Config::General::setDelayStatusChange(bool delayStatusChange)
{
  if (delayStatusChange == myDelayStatusChange)
    return;

  myDelayStatusChange = delayStatusChange;
}

void Config::General::setMsgPopupKey(const QString& msgPopupKey)
{
  if (msgPopupKey == myMsgPopupKey)
    return;

  myMsgPopupKey = msgPopupKey;
  emit msgPopupKeyChanged(myMsgPopupKey);
}

QFont Config::General::normalFont() const
{
  return qApp->font();
}

void Config::General::setNormalFont(const QString& normalFont)
{
  QFont f;
  if (normalFont.isEmpty())
    f = myDefaultFont;
  else
    f.fromString(normalFont);

  if (f == qApp->font())
    return;

  qApp->setFont(f);
  // No need to emit fontChanged for normal font, qt will handle this for us
}

void Config::General::setEditFont(const QString& editFont)
{
  QFont f;
  if (editFont.isEmpty())
    f = myDefaultFont;
  else
    f.fromString(editFont);

  if (f == myEditFont)
    return;

  myEditFont = f;
  if (myBlockUpdates)
    myFontHasChanged = true;
  else
    emit fontChanged();
}

void Config::General::setHistoryFont(const QString& historyFont)
{
  QFont f;
  if (historyFont.isEmpty())
    f = myDefaultFont;
  else
    f.fromString(historyFont);

  if (f == myHistoryFont)
    return;

  myHistoryFont = f;
  if (myBlockUpdates)
    myFontHasChanged = true;
  else
    emit fontChanged();
}

void Config::General::setFixedFont(const QString& fixedFont)
{
  QFont f;
  if (fixedFont.isEmpty())
    f = myDefaultFixedFont;
  else
    f.fromString(fixedFont);

  if (f == myFixedFont)
    return;

  myFixedFont = f;
  if (myBlockUpdates)
    myFontHasChanged = true;
  else
    emit fontChanged();
}

#ifndef USE_KDE
QString Config::General::guiStyle() const
{
  return qApp->style()->objectName();
}

void Config::General::setGuiStyle(const QString& guiStyle)
{
  if (guiStyle.compare(QApplication::style()->objectName(), Qt::CaseInsensitive) == 0)
    return;

  qApp->setStyle(guiStyle);

  // Since Licq daemon blocks SIGCHLD and Qt never receives it,
  // QProcess hangs. By this we avoid Qt's attempts to be
  // conformant to desktop settings in Cleanlooks style.
  if (guiStyle.compare("Cleanlooks", Qt::CaseInsensitive) == 0)
    qApp->setDesktopSettingsAware(false);
  else
    qApp->setDesktopSettingsAware(true);

  if (myBlockUpdates)
    myStyleHasChanged = true;
  else
    emit styleChanged();
}
#endif

void Config::General::setMiniMode(bool miniMode)
{
  if (miniMode == myMiniMode)
    return;

  myMiniMode = miniMode;
  if (myBlockUpdates)
    myMainwinHasChanged = true;
  else
    emit mainwinChanged();
}

void Config::General::toggleMiniMode()
{
  setMiniMode(!myMiniMode);
}

void Config::General::setShowGroupIfNoMsg(bool showGroupIfNoMsg)
{
  if (showGroupIfNoMsg == myShowGroupIfNoMsg)
    return;

  myShowGroupIfNoMsg = showGroupIfNoMsg;
  if (myBlockUpdates)
    myMainwinHasChanged = true;
  else
    emit mainwinChanged();
}

void Config::General::setBoldOnMsg(bool boldOnMsg)
{
  if (boldOnMsg == myBoldOnMsg)
    return;

  myBoldOnMsg = boldOnMsg;
  if (myBlockUpdates)
    myMainwinHasChanged = true;
  else
    emit mainwinChanged();
}

void Config::General::setMainwinDraggable(bool mainwinDraggable)
{
  if (mainwinDraggable == myMainwinDraggable)
    return;

  myMainwinDraggable = mainwinDraggable;
}

void Config::General::setMainwinSticky(bool mainwinSticky)
{
  if (mainwinSticky == myMainwinSticky)
    return;

  myMainwinSticky = mainwinSticky;
  if (myBlockUpdates)
    myMainwinHasChanged = true;
  else
    emit mainwinChanged();
}

void Config::General::setAutoRaiseMainwin(bool autoRaiseMainwin)
{
  if (autoRaiseMainwin == myAutoRaiseMainwin)
    return;

  myAutoRaiseMainwin = autoRaiseMainwin;
}

void Config::General::setMainwinStartHidden(bool mainwinStartHidden)
{
  if (mainwinStartHidden == myMainwinStartHidden)
    return;

  myMainwinStartHidden = mainwinStartHidden;
}

void Config::General::setMainwinRect(const QRect& geometry)
{
  if (geometry.isValid())
    myMainwinRect = geometry;
}

void Config::General::setDockMode(DockMode dockMode)
{
  if (dockMode == myDockMode)
    return;

  myDockMode = dockMode;
  if (myBlockUpdates)
    myDockModeHasChanged = true;
  else
    emit dockModeChanged();
}

#ifndef USE_KDE
void Config::General::setDefaultIconFortyEight(bool defaultIconFortyEight)
{
  if (defaultIconFortyEight == myDefaultIconFortyEight)
    return;

  myDefaultIconFortyEight = defaultIconFortyEight;
  if (myDockMode != DockDefault)
    return;

  if (myBlockUpdates)
    myDockHasChanged = true;
  else
    emit dockChanged();
}

void Config::General::setThemedIconTheme(const QString& themedIconTheme)
{
  if (themedIconTheme == myThemedIconTheme)
    return;

  myThemedIconTheme = themedIconTheme;
  if (myDockMode != DockThemed)
    return;

  if (myBlockUpdates)
    myDockHasChanged = true;
  else
    emit dockChanged();
}
#endif

void Config::General::setTrayBlink(bool trayBlink)
{
  if (trayBlink == myTrayBlink)
    return;

  myTrayBlink = trayBlink;
  if (myDockMode != DockTray)
    return;

  if (myBlockUpdates)
    myDockHasChanged = true;
  else
    emit dockChanged();
}

void Config::General::setTrayMsgOnlineNotify(bool trayMsgOnlineNotify)
{
  if (trayMsgOnlineNotify == myTrayMsgOnlineNotify)
    return;

  myTrayMsgOnlineNotify = trayMsgOnlineNotify;
}

void Config::General::setAutoLogon(unsigned short autoLogon)
{
  if (autoLogon == myAutoLogon)
    return;

  myAutoLogon = autoLogon;
}

void Config::General::setAutoAwayTime(unsigned short autoAwayTime)
{
  if (autoAwayTime == myAutoAwayTime)
    return;

  myAutoAwayTime = autoAwayTime;
}

void Config::General::setAutoNaTime(unsigned short autoNaTime)
{
  if (autoNaTime == myAutoNaTime)
    return;

  myAutoNaTime = autoNaTime;
}

void Config::General::setAutoOfflineTime(unsigned short autoOfflineTime)
{
  if (autoOfflineTime == myAutoOfflineTime)
    return;

  myAutoOfflineTime = autoOfflineTime;
}

void Config::General::setAutoAwayMess(unsigned short autoAwayMess)
{
  if (autoAwayMess == myAutoAwayMess)
    return;

  myAutoAwayMess = autoAwayMess;
}

void Config::General::setAutoNaMess(unsigned short autoNaMess)
{
  if (autoNaMess == myAutoNaMess)
    return;

  myAutoNaMess = autoNaMess;
}
