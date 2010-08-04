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

#ifndef CONFIG_GENERAL_H
#define CONFIG_GENERAL_H

#include "config.h"

#include <QFont>
#include <QObject>
#include <QRect>

namespace Licq
{
class IniFile;
}

namespace LicqQtGui
{
namespace Config
{
/**
 * General configuration, including main window and dock icon
 */
class General : public QObject
{
  Q_OBJECT

public:
  enum DockMode
  {
    DockNone = 0,
    DockDefault = 1,
    DockThemed = 2,
    DockTray = 3
  };

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
  static General* instance()
  { return myInstance; }


  /**
   * Constuctor
   */
  General(QObject* parent = 0);

  ~General() {}

  void blockUpdates(bool block);

  // Get functions
  bool useDoubleReturn() const { return myUseDoubleReturn; }
  const QString& msgPopupKey() const { return myMsgPopupKey; }
  const QFont& defaultFont() const { return myDefaultFont; }
  const QFont& defaultFixedFont() const { return myDefaultFixedFont; }
  QFont normalFont() const;
  const QFont& editFont() const { return myEditFont; }
  const QFont& historyFont() const { return myHistoryFont; }
  const QFont& fixedFont() const { return myFixedFont; }

#ifndef USE_KDE
  QString guiStyle() const;
#endif

  bool miniMode() const { return myMiniMode; }
  bool showGroupIfNoMsg() const { return myShowGroupIfNoMsg; }
  bool boldOnMsg() const { return myBoldOnMsg; }
  bool mainwinDraggable() const { return myMainwinDraggable; }
  bool mainwinSticky() const { return myMainwinSticky; }
  bool autoRaiseMainwin() const { return myAutoRaiseMainwin; }
  bool mainwinStartHidden() const { return myMainwinStartHidden; }
  const QRect& mainwinRect() const { return myMainwinRect; }

  DockMode dockMode() const { return myDockMode; }
#ifndef USE_KDE
  bool defaultIconFortyEight() const { return myDefaultIconFortyEight; }
  const QString& themedIconTheme() const { return myThemedIconTheme; }
#endif
  bool trayBlink() const { return myTrayBlink; }
  bool trayMsgOnlineNotify() const { return myTrayMsgOnlineNotify; }

  int autoAwayTime() const { return myAutoAwayTime; }
  int autoNaTime() const { return myAutoNaTime; }
  int autoOfflineTime() const { return myAutoOfflineTime; }
  int autoAwayMess() const { return myAutoAwayMess; }
  int autoNaMess() const { return myAutoNaMess; }

public slots:
  /**
   * Load configuration from file
   */
  void loadConfiguration(Licq::IniFile& iniFile);

  /**
   * Save configuration to file
   */
  void saveConfiguration(Licq::IniFile& iniFile) const;

  // Set functions
  void setUseDoubleReturn(bool useDoubleReturn);
  void setMsgPopupKey(const QString& msgPopupKey);
  void setNormalFont(const QString& normalFont);
  void setEditFont(const QString& editFont);
  void setHistoryFont(const QString& historyFont);
  void setFixedFont(const QString& fixedFont);

  void setMiniMode(bool miniMode);
  void setShowGroupIfNoMsg(bool showGroupIfNoMsg);
  void setBoldOnMsg(bool boldOnMsg);
  void setMainwinDraggable(bool mainwinDraggable);
  void setMainwinSticky(bool mainwinSticky);
  void setAutoRaiseMainwin(bool autoRaiseMainwin);
  void setMainwinStartHidden(bool mainwinStartHidden);
  void setMainwinRect(const QRect& geometry);

#ifndef USE_KDE
  void setGuiStyle(const QString& guiStyle);
#endif

  void setDockMode(DockMode dockMode);
#ifndef USE_KDE
  void setDefaultIconFortyEight(bool defaultIconFortyEight);
  void setThemedIconTheme(const QString& themedIconTheme);
#endif
  void setTrayBlink(bool trayBlink);
  void setTrayMsgOnlineNotify(bool trayMsgOnlineNotify);

  void setAutoAwayTime(int autoAwayTime);
  void setAutoNaTime(int autoNaTime);
  void setAutoOfflineTime(int autoOfflineTime);
  void setAutoAwayMess(int autoAwayMess);
  void setAutoNaMess(int autoNaMess);

  // Toggle functions for convenience
  void toggleMiniMode();

signals:
  /**
   * Configuration affecting main window has changed
   */
  void mainwinChanged();

  /**
   * Dock icon type has changed
   */
  void dockModeChanged();

  /**
   * Configuration affecting dock icon has changed
   */
  void dockChanged();

  /**
   * Font configuration has changed (not emitted for normal font)
   */
  void fontChanged();

  /**
   * GUI Style has changed
   */
  void styleChanged();

  /**
   * Popup key has changed
   */
  void msgPopupKeyChanged(const QString& newKey);

private:
  static General* myInstance;

  // Changes have been made that should trigger changed() signal
  bool myMainwinHasChanged;
  bool myDockHasChanged;
  bool myDockModeHasChanged;
  bool myFontHasChanged;
  bool myStyleHasChanged;
  bool myBlockUpdates;

  // General configuration
  bool myUseDoubleReturn;
  QString myMsgPopupKey;
  QFont myDefaultFont;
  QFont myDefaultFixedFont;
  QFont myEditFont;
  QFont myHistoryFont;
  QFont myFixedFont;
#ifndef USE_KDE
  QString myDefaultStyle;
#endif

  // Mainwin configuration
  bool myMiniMode;
  bool myShowGroupIfNoMsg;
  bool myBoldOnMsg;
  bool myMainwinDraggable;
  bool myMainwinSticky;
  bool myAutoRaiseMainwin;
  bool myMainwinStartHidden;
  bool myFrameTransparent;
  QRect myMainwinRect;

  // Dock configuration
  DockMode myDockMode;
#ifndef USE_KDE
  bool myDefaultIconFortyEight;
  QString myThemedIconTheme;
#endif
  bool myTrayBlink;
  bool myTrayMsgOnlineNotify;

  // Auto status configuration
  int myAutoAwayTime;
  int myAutoNaTime;
  int myAutoOfflineTime;
  int myAutoAwayMess;
  int myAutoNaMess;
};

} // namespace Config
} // namespace LicqQtGui

#endif
