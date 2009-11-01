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

#ifndef ICONMANAGER_H
#define ICONMANAGER_H

#include "config.h"

#include <QMap>
#include <QObject>
#include <QPair>
#include <QPixmap>

#include <licq_icq.h>
#include <licq_user.h>

#include "core/gui-defines.h"

namespace LicqQtGui
{
/**
 * Manager for icon sets used in gui
 */
class IconManager : public QObject
{
  Q_OBJECT

public:
  enum IconType
  {
    // Message icons
    StandardMessageIcon,
    UrlMessageIcon,
    ChatMessageIcon,
    FileMessageIcon,
    ContactMessageIcon,
    SmsMessageIcon,
    AuthorizeMessageIcon,
    ReqAuthorizeMessageIcon,

    // Extended icons
    BirthdayIcon,
    CellularIcon,
    CollapsedIcon,
    CustomArIcon,
    ExpandedIcon,
    GpgKeyDisabledIcon,
    GpgKeyEnabledIcon,
    IcqPhoneActiveIcon,
    IcqPhoneBusyIcon,
    InvisibleIcon,
    PfmActiveIcon,
    PfmBusyIcon,
    PhoneIcon,
    SharedFilesIcon,
    TypingIcon,

    // Menu icons
    RemoveIcon,
    SearchIcon,
    GpgKeyIcon,

    // Toolbar icons
    BackColorIcon,
    BeepIcon,
    BoldIcon,
    EncodingIcon,
    HistoryIcon,
    IgnoreIcon,
    InfoIcon,
    ItalicIcon,
    MenuIcon,
    MultipleRecIcon,
    SecureOffIcon,
    SecureOnIcon,
    SmileIcon,
    StrikethroughIcon,
    TextColorIcon,
    ThroughServerIcon,
    UnderlineIcon,
    UrgentIcon,

    // Phonebook icons
    MobileIcon,
    SMSIcon,
    FaxIcon,
    PagerIcon,
    PSTNIcon,
  };

  enum StatusIconType
  {
    OnlineStatusIcon = ICQ_STATUS_ONLINE,
    OfflineStatusIcon = ICQ_STATUS_OFFLINE,
    AwayStatusIcon = ICQ_STATUS_AWAY,
    DoNotDisturbStatusIcon = ICQ_STATUS_DND,
    OccupiedStatusIcon = ICQ_STATUS_OCCUPIED,
    NotAvailableStatusIcon = ICQ_STATUS_NA,
    FreeForChatStatusIcon = ICQ_STATUS_FREEFORCHAT,
    PrivateStatusIcon = ICQ_STATUS_FxPRIVATE,
  };

  enum ProtocolType
  {
    ProtocolIcq = LICQ_PPID,
    ProtocolMsn = MSN_PPID,
    ProtocolAim,
  };

  /**
   * Create the Icon Manager instance
   *
   * @param iconSet Initial icon set to load
   * @param extendedIconSet Initial extended icon set to load
   * @param parent Parent object
   */
  static void createInstance(const QString& iconSet,
      const QString& extendedIconSet, QObject* parent = NULL);

  /**
   * Get the Icon Manager
   *
   * @return The icon manager singleton
   */
  static IconManager* instance()
  { return myInstance; }

  /**
   * Load a set of icons
   *
   * @param iconSet Name of icon set
   * @return True if icon set was found and index files could be read
   */
  bool loadIcons(const QString& iconSet);

  /**
   * Load a set of extended icons
   *
   * @param iconSet Name of extended icon set
   * @return True if icon set was found and index files could be read
   */
  bool loadExtendedIcons(const QString& iconSet);

  /**
   * Get an icon
   *
   * @param icon The icon type to get
   * @return The requested icon if loaded, otherwise a null pixmap
   */
  const QPixmap& getIcon(IconType icon);

  /**
   * Get icon for a protocol status
   *
   * @param fullStatus Status to get icon for, should be full to include invisible flag
   * @param id Contact id, used to differentiate between ICQ and AIM
   * @param ppid Id of protocol to use icon set for
   * @return The requested icon if loaded, otherwise a null pixmap
   */
  const QPixmap& iconForStatus(unsigned long fullStatus, const QString& id = "0", unsigned long ppid = LICQ_PPID);

  /**
   * Get icon for an event type
   *
   * @param subCommand Message sub command to get icon for
   * @return The requested icon if loaded, otherwise a null pixmap
   */
  const QPixmap& iconForEvent(unsigned short subCommand);

  const QString& iconSet() const { return myIconSet; }
  const QString& extendedIconSet() const { return myExtendedIconSet; }

signals:
  /**
   * The icon set has changed, emitted for all icon sets
   */
  void iconsChanged();

  /**
   * The general icons has changed
   */
  void generalIconsChanged();

  /**
   * The status icons has changed
   */
  void statusIconsChanged();

  /**
   * The extended icons has changed
   */
  void extendedIconsChanged();


private:
  // Singleton instance
  static IconManager* myInstance;

  /**
   * Constructor, private so only createInstance() can call it
   *
   * @param iconSet Initial icon set to load
   * @param extendedIconSet Initial extended icon set to load
   * @param parent Parent object
   */
  IconManager(const QString& iconSet, const QString& extendedIconSet, QObject* parent = NULL);

  /**
   * Destructor
   */
  virtual ~IconManager() {}

  QString myIconSet;
  QString myExtendedIconSet;

  // Map of current icons
  QMap<IconType, QPixmap> myIconMap;

  // Map of status icons for different protocols
  QMap<QPair<ProtocolType, StatusIconType>, QPixmap> myStatusIconMap;

  // Null icon that can be returned as default
  QPixmap myEmptyIcon;
};

} // namespace LicqQtGui

#endif
