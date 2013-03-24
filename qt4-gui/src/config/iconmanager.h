/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2013 Licq developers <licq-dev@googlegroups.com>
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

#include <licq/contactlist/user.h>
#include <licq/userid.h>

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

  enum ProtocolType
  {
    ProtocolIcq = ICQ_PPID,
    ProtocolAim,
    ProtocolMsn = MSN_PPID,
    ProtocolXmpp = JABBER_PPID,
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
   * @param status Status to get icon for
   * @param userId Contact id, used to get protocol to get icon for
   * @param allowInvisible True to allow invisible regardles if extended icons are used
   * @return The requested icon if loaded, otherwise a null pixmap
   */
  const QPixmap& iconForStatus(unsigned status, const Licq::UserId& userId = Licq::UserId(),
      bool allowInvisible = false);

  /**
   * Get icon for a protocol
   * Note: Will always return ICQ icon for oscar protocol, never AIM
   *
   * @param protocolId Protocol to get icon for
   * @param status Status to get icon for
   * @return Status icon for the requested protocol
   */
  const QPixmap& iconForProtocol(unsigned long protocolId, unsigned status = Licq::User::OnlineStatus);

  /**
   * Get icon for a user
   *
   * @param user An already locked object of LicqUser type
   * @return The requested icon if loaded, otherwise a null pixmap
   */
  const QPixmap& iconForUser(const Licq::User* user)
  { return iconForStatus(user->status(), user->id()); }

  /**
   * Get icon for an event type
   *
   * @param eventType Type of event to get message for
   * @return The requested icon if loaded, otherwise a null pixmap
   */
  const QPixmap& iconForEvent(unsigned eventType);

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

  const QPixmap& iconForProtocol(unsigned status, ProtocolType protocol, bool allowInvisible);

  QString myIconSet;
  QString myExtendedIconSet;

  // Map of current icons
  QMap<IconType, QPixmap> myIconMap;

  // Map of status icons for different protocols
  QMap<QPair<ProtocolType, unsigned>, QPixmap> myStatusIconMap;

  // Null icon that can be returned as default
  QPixmap myEmptyIcon;
};

} // namespace LicqQtGui

#endif
