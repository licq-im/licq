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

#include "iconmanager.h"

#include "config.h"

#include <cctype>

#include <licq/daemon.h>
#include <licq/icqdefines.h>
#include <licq/inifile.h>
#include <licq/logging/log.h>
#include <licq/contactlist/user.h>

#include "config/contactlist.h"

#include "xpm/bar/beep.xpm"
#include "xpm/bar/bgcolor.xpm"
#include "xpm/bar/bold.xpm"
#include "xpm/bar/encoding.xpm"
#include "xpm/bar/history.xpm"
#include "xpm/bar/ignore.xpm"
#include "xpm/bar/info.xpm"
#include "xpm/bar/italic.xpm"
#include "xpm/bar/menu.xpm"
#include "xpm/bar/multiplerec.xpm"
#include "xpm/bar/secureoff.xpm"
#include "xpm/bar/secureon.xpm"
#include "xpm/bar/smile.xpm"
#include "xpm/bar/strikethrough.xpm"
#include "xpm/bar/textcolor.xpm"
#include "xpm/bar/throughserver.xpm"
#include "xpm/bar/underline.xpm"
#include "xpm/bar/urgent.xpm"

#include "xpm/exticons/ICQphoneActive.xpm"
#include "xpm/exticons/ICQphoneBusy.xpm"
#include "xpm/exticons/PFMActive.xpm"
#include "xpm/exticons/PFMBusy.xpm"
#include "xpm/exticons/autoresponse.xpm"
#include "xpm/exticons/birthday.xpm"
#include "xpm/exticons/cellular.xpm"
#include "xpm/exticons/collapsed.xpm"
#include "xpm/exticons/expanded.xpm"
#include "xpm/exticons/gpgoff.xpm"
#include "xpm/exticons/gpgon.xpm"
#include "xpm/exticons/invisible.xpm"
#include "xpm/exticons/phone.xpm"
#include "xpm/exticons/sharedfiles.xpm"
#include "xpm/exticons/typing.xpm"

#include "xpm/menu/remove.xpm"
#include "xpm/menu/search.xpm"

#include "xpm/phonebook/fax.xpm"
#include "xpm/phonebook/mobile.xpm"
#include "xpm/phonebook/pager.xpm"
#include "xpm/phonebook/pstn.xpm"
#include "xpm/phonebook/sms.xpm"

using Licq::User;
using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::IconManager */

IconManager* IconManager::myInstance = NULL;

void IconManager::createInstance(const QString& iconSet, const QString& extendedIconSet, QObject* parent)
{
  myInstance = new IconManager(iconSet, extendedIconSet, parent);
}

IconManager::IconManager(const QString& iconSet, const QString& extendedIconSet, QObject* parent)
  : QObject(parent)
{
  if (!loadIcons(iconSet))
    Licq::gLog.warning("Unable to load icons %s", iconSet.toLocal8Bit().data());

  if (!loadExtendedIcons(extendedIconSet))
    Licq::gLog.warning("Unable to load extended icons %s", extendedIconSet.toLocal8Bit().data());
}

bool IconManager::loadIcons(const QString& iconSet)
{
  QString iconListName = iconSet + ".icons";
  QString subdir = QString(QTGUI_DIR) + ICONS_DIR + iconSet + "/";
  QString iconPath = QString::fromLocal8Bit(Licq::gDaemon.baseDir().c_str()) + subdir;
  Licq::IniFile iconsConf((iconPath + iconListName).toLocal8Bit().data());
  if (!iconsConf.loadFile())
  {
    iconPath = QString::fromLocal8Bit(Licq::gDaemon.shareDir().c_str()) + subdir;
    iconsConf.setFilename((iconPath + iconListName).toLocal8Bit().data());
    if (!iconsConf.loadFile())
      return false;
  }

  iconsConf.setSection("icons");

  std::string filename;

  // Note: With Qt 4.6 the QPixmap cannot be reused without clearing it
  //       between loads or icons will be mixed up.
  QPixmap p;

#define LOAD_ICON(name, icon) \
    if (iconsConf.get(name, filename) && p.load(iconPath + QString::fromLocal8Bit(filename.c_str()))) \
    { \
      myIconMap.insert(icon, p); \
      p = QPixmap(); \
    } \
    else \
      myIconMap.remove(icon);

#define LOAD2_ICON(name, icon, deficon) \
    if (iconsConf.get(name, filename) && p.load(iconPath + QString::fromLocal8Bit(filename.c_str()))) \
    { \
      myIconMap.insert(icon, p); \
      p = QPixmap(); \
    } \
    else \
      myIconMap.insert(icon, deficon);

#define LOAD_STATUSICON(name, protocol, icon) \
    if (iconsConf.get(name, filename) && p.load(iconPath + QString::fromLocal8Bit(filename.c_str()))) \
    { \
      myStatusIconMap.insert(QPair<ProtocolType, unsigned>(protocol, icon), p); \
      p = QPixmap(); \
    } \
    else \
      myStatusIconMap.remove(QPair<ProtocolType, unsigned>(protocol, icon));

  // ICQ/Default status icons
  LOAD_STATUSICON("Online",     ProtocolIcq, User::OnlineStatus)
  LOAD_STATUSICON("FFC",        ProtocolIcq, User::FreeForChatStatus)
  LOAD_STATUSICON("Offline",    ProtocolIcq, User::OfflineStatus)
  LOAD_STATUSICON("Away",       ProtocolIcq, User::AwayStatus)
  LOAD_STATUSICON("NA",         ProtocolIcq, User::NotAvailableStatus)
  LOAD_STATUSICON("Occupied",   ProtocolIcq, User::OccupiedStatus)
  LOAD_STATUSICON("DND",        ProtocolIcq, User::DoNotDisturbStatus)
  LOAD_STATUSICON("Private",    ProtocolIcq, User::InvisibleStatus)
  LOAD_STATUSICON("Idle",       ProtocolIcq, User::IdleStatus)

  // AIM status icons
  LOAD_STATUSICON("AIMOnline",  ProtocolAim, User::OnlineStatus)
  LOAD_STATUSICON("AIMOffline", ProtocolAim, User::OfflineStatus)
  LOAD_STATUSICON("AIMAway",    ProtocolAim, User::AwayStatus)

  // MSN status icons
  LOAD_STATUSICON("MSNOnline",  ProtocolMsn, User::OnlineStatus)
  LOAD_STATUSICON("MSNOffline", ProtocolMsn, User::OfflineStatus)
  LOAD_STATUSICON("MSNAway",    ProtocolMsn, User::AwayStatus)
  LOAD_STATUSICON("MSNOccupied",ProtocolMsn, User::OccupiedStatus)
  LOAD_STATUSICON("MSNPrivate", ProtocolMsn, User::InvisibleStatus)
  LOAD_STATUSICON("MSNIdle",    ProtocolMsn, User::IdleStatus)

  // XMPP status icons
  LOAD_STATUSICON("XMPPOnline",	ProtocolXmpp, User::OnlineStatus)
  LOAD_STATUSICON("XMPPFFC",	ProtocolXmpp, User::FreeForChatStatus)
  LOAD_STATUSICON("XMPPOffline",ProtocolXmpp, User::OfflineStatus)
  LOAD_STATUSICON("XMPPAway",	ProtocolXmpp, User::AwayStatus)
  LOAD_STATUSICON("XMPPNA",	ProtocolXmpp, User::NotAvailableStatus)
  LOAD_STATUSICON("XMPPDND",	ProtocolXmpp, User::DoNotDisturbStatus)

  // Message icons
  LOAD_ICON("Message",          StandardMessageIcon);
  LOAD2_ICON("Url",             UrlMessageIcon, myIconMap.value(StandardMessageIcon));
  LOAD2_ICON("Chat",            ChatMessageIcon, myIconMap.value(StandardMessageIcon));
  LOAD2_ICON("File",            FileMessageIcon, myIconMap.value(StandardMessageIcon));
  LOAD2_ICON("Contact",         ContactMessageIcon, myIconMap.value(StandardMessageIcon));
  LOAD2_ICON("Authorize",       AuthorizeMessageIcon, myIconMap.value(StandardMessageIcon));
  LOAD2_ICON("ReqAuthorize",    ReqAuthorizeMessageIcon, myIconMap.value(AuthorizeMessageIcon));
  LOAD2_ICON("SMS",             SmsMessageIcon, myIconMap.value(StandardMessageIcon));

  // Menu icons
  LOAD2_ICON("Remove",          RemoveIcon, remove_xpm);
  LOAD2_ICON("Search",          SearchIcon, search_xpm);
  LOAD2_ICON("GPGKey",          GpgKeyIcon, gpgon_xpm);

  // Toolbar icons
  LOAD2_ICON("BackColor",       BackColorIcon, bgcolor_xpm);
  LOAD2_ICON("Beep",            BeepIcon, beep_xpm);
  LOAD2_ICON("Bold",            BoldIcon, bold_xpm);
  LOAD2_ICON("Encoding",        EncodingIcon, encoding_xpm);
  LOAD2_ICON("History",         HistoryIcon, history_xpm);
  LOAD2_ICON("Ignore",          IgnoreIcon, ignore_xpm);
  LOAD2_ICON("Info",            InfoIcon, info_xpm);
  LOAD2_ICON("Italic",          ItalicIcon, italic_xpm);
  LOAD2_ICON("Menu",            MenuIcon, menu_xpm);
  LOAD2_ICON("MultipleRec",     MultipleRecIcon, multiplerec_xpm);
  LOAD2_ICON("SecureOff",       SecureOffIcon, secureoff_xpm);
  LOAD2_ICON("SecureOn",        SecureOnIcon, secureon_xpm);
  LOAD2_ICON("Smile",           SmileIcon, smile_xpm);
  LOAD2_ICON("Strikethrough",   StrikethroughIcon, strikethrough_xpm);
  LOAD2_ICON("TextColor",       TextColorIcon, textcolor_xpm);
  LOAD2_ICON("ThroughServer",   ThroughServerIcon, throughserver_xpm);
  LOAD2_ICON("Underline",       UnderlineIcon, underline_xpm);
  LOAD2_ICON("Urgent",          UrgentIcon, urgent_xpm);

  // Phonebook icons
  LOAD2_ICON("Mobile",          MobileIcon, mobile_xpm);
  LOAD2_ICON("MobileSMS",       SMSIcon, sms_xpm);
  LOAD2_ICON("Fax",             FaxIcon, fax_xpm);
  LOAD2_ICON("Pager",           PagerIcon, pager_xpm);
  LOAD2_ICON("PSTN",            PSTNIcon, pstn_xpm);

#undef LOAD_ICON
#undef LOAD_ICON2
#undef LOAD_STATUSICON

  myIconSet = iconSet;
  emit iconsChanged();
  emit generalIconsChanged();
  emit statusIconsChanged();

  return true;
}

bool IconManager::loadExtendedIcons(const QString& iconSet)
{
  QString iconListName = iconSet + ".icons";
  QString subdir = QString(QTGUI_DIR) + EXTICONS_DIR + iconSet + "/";
  QString iconPath = QString::fromLocal8Bit(Licq::gDaemon.baseDir().c_str()) + subdir;
  Licq::IniFile iconsConf((iconPath + iconListName).toLocal8Bit().data());
  if (!iconsConf.loadFile())
  {
    iconPath = QString::fromLocal8Bit(Licq::gDaemon.shareDir().c_str()) + subdir;
    iconsConf.setFilename((iconPath + iconListName).toLocal8Bit().data());
    if (!iconsConf.loadFile())
      return false;
  }

  iconsConf.setSection("icons");

  std::string filename;

  QPixmap p;

#define LOAD_ICON(name, icon, deficon) \
    if (iconsConf.get(name, filename) && p.load(iconPath + QString::fromLocal8Bit(filename.c_str()))) \
    { \
      myIconMap.insert(icon, p); \
      p = QPixmap(); \
    } \
    else \
      myIconMap.insert(icon, deficon);

  LOAD_ICON("Birthday",             BirthdayIcon, birthday_xpm);
  LOAD_ICON("Cellular",             CellularIcon, cellular_xpm);
  LOAD_ICON("Collapsed",            CollapsedIcon, collapsed_xpm);
  LOAD_ICON("CustomAR",             CustomArIcon, autoresponse_xpm);
  LOAD_ICON("Expanded",             ExpandedIcon, expanded_xpm);
  LOAD_ICON("GPGKeyDisabled",       GpgKeyDisabledIcon, gpgoff_xpm);
  LOAD_ICON("GPGKeyEnabled",        GpgKeyEnabledIcon, gpgon_xpm);
  LOAD_ICON("ICQphoneActive",       IcqPhoneActiveIcon, ICQphoneActive_xpm);
  LOAD_ICON("ICQphoneBusy",         IcqPhoneBusyIcon, ICQphoneBusy_xpm);
  LOAD_ICON("Invisible",            InvisibleIcon, invisible_xpm);
  LOAD_ICON("Phone",                PhoneIcon, phone_xpm);
  LOAD_ICON("PhoneFollowMeActive",  PfmActiveIcon, PFMActive_xpm);
  LOAD_ICON("PhoneFollowMeBusy",    PfmBusyIcon, PFMBusy_xpm);
  LOAD_ICON("SharedFiles",          SharedFilesIcon, sharedfiles_xpm);
  LOAD_ICON("Typing",               TypingIcon, typing_xpm);

#undef LOAD_ICON

  myExtendedIconSet = iconSet;
  emit iconsChanged();
  emit extendedIconsChanged();

  return true;
}

const QPixmap& IconManager::getIcon(IconType icon)
{
  if (myIconMap.contains(icon))
    return myIconMap[icon];

  qWarning("IconManager::getIcon - returning empty icon for IconType=%i", icon);
  return myEmptyIcon;
}

const QPixmap& IconManager::iconForStatus(unsigned status, const Licq::UserId& userId, bool allowInvisible)
{
  ProtocolType protocol = static_cast<ProtocolType>(userId.protocolId());
  if (protocol == ProtocolIcq && userId.accountId().size() > 0 && !isdigit(userId.accountId()[0]))
    protocol = ProtocolAim;

  if (Config::ContactList::instance()->showExtendedIcons() && !allowInvisible)
    // Extended icons shows if user is invisible so skip it here
    status &= ~User::InvisibleStatus;

  // Make sure we don't have multiple flags in status word
  status = User::singleStatus(status);

  if (myStatusIconMap.contains(QPair<ProtocolType, unsigned>(protocol, status)))
    return myStatusIconMap[QPair<ProtocolType, unsigned>(protocol, status)];

  // Use Occupied icon if Do Not Disturb icon is missing
  if (status & User::DoNotDisturbStatus && myStatusIconMap.contains(QPair<ProtocolType, unsigned>(protocol, User::OccupiedStatus)))
    return myStatusIconMap[QPair<ProtocolType, unsigned>(protocol, User::OccupiedStatus)];

  // Use Away icon if other away type icons are missing
  if (status & User::AwayStatuses && myStatusIconMap.contains(QPair<ProtocolType, unsigned>(protocol, User::AwayStatus)))
    return myStatusIconMap[QPair<ProtocolType, unsigned>(protocol, User::AwayStatus)];

  // Use Online icon if nothing else exist
  if (myStatusIconMap.contains(QPair<ProtocolType, unsigned>(protocol, User::OnlineStatus)))
    return myStatusIconMap[QPair<ProtocolType, unsigned>(protocol, User::OnlineStatus)];

  // No protocol specific icon existed so use default (same as ICQ)
  if (myStatusIconMap.contains(QPair<ProtocolType, unsigned>(ProtocolIcq, status)))
    return myStatusIconMap[QPair<ProtocolType, unsigned>(ProtocolIcq, status)];

  // Icon is missing in default as well, use default online
  return myStatusIconMap[QPair<ProtocolType, unsigned>(ProtocolIcq, User::OnlineStatus)];
}

const QPixmap& IconManager::iconForEvent(unsigned short subCommand)
{
  IconType icon;
  switch(subCommand)
  {
    case ICQ_CMDxSUB_URL:
      icon = UrlMessageIcon;
      break;
    case ICQ_CMDxSUB_CHAT:
      icon = ChatMessageIcon;
      break;
    case ICQ_CMDxSUB_FILE:
      icon = FileMessageIcon;
      break;
    case ICQ_CMDxSUB_CONTACTxLIST:
      icon = ContactMessageIcon;
      break;
    case ICQ_CMDxSUB_AUTHxREQUEST:
      icon = ReqAuthorizeMessageIcon;
      break;
    case ICQ_CMDxSUB_AUTHxREFUSED:
    case ICQ_CMDxSUB_AUTHxGRANTED:
      icon = AuthorizeMessageIcon;
      break;
    case ICQ_CMDxSUB_MSG:
    default:
      icon = StandardMessageIcon;
  }

  return myIconMap[icon];
}
