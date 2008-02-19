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

#include "iconmanager.h"

#include "config.h"

#include <licq_file.h>
#include <licq_log.h>

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
#ifdef HAVE_LIBGPGME
#include "xpm/exticons/gpgoff.xpm"
#include "xpm/exticons/gpgon.xpm"
#endif
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

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::IconManager */

IconManager* IconManager::myInstance = NULL;

void IconManager::createInstance(QString iconSet, QString extendedIconSet, QObject* parent)
{
  myInstance = new IconManager(iconSet, extendedIconSet, parent);
}

IconManager::IconManager(QString iconSet, QString extendedIconSet, QObject* parent)
  : QObject(parent)
{
  if (!loadIcons(iconSet))
    gLog.Warn("%sUnable to load icons %s.\n", L_WARNxSTR, iconSet.toLocal8Bit().data());

  if (!loadExtendedIcons(extendedIconSet))
    gLog.Warn("%sUnable to load extended icons %s.\n", L_WARNxSTR, extendedIconSet.toLocal8Bit().data());
}

bool IconManager::loadIcons(QString iconSet)
{
  CIniFile fIconsConf;

  QString iconListName = iconSet + ".icons";
  QString subdir = QString(QTGUI_DIR) + ICONS_DIR + iconSet + "/";
  QString iconPath = QString::fromLocal8Bit(BASE_DIR) + subdir;
  if (!fIconsConf.LoadFile((iconPath + iconListName).toLocal8Bit()))
  {
    iconPath = QString::fromLocal8Bit(SHARE_DIR) + subdir;
    if (!fIconsConf.LoadFile((iconPath + iconListName).toLocal8Bit()))
      return false;
  }

  fIconsConf.SetSection("icons");

  char filename[MAX_FILENAME_LEN];

  QPixmap p;

#define LOAD_ICON(name, icon) \
    if (fIconsConf.ReadStr(name, filename) && p.load(iconPath + QString::fromLocal8Bit(filename))) \
      myIconMap.insert(icon, p); \
    else \
      myIconMap.remove(icon);

#define LOAD2_ICON(name, icon, deficon) \
    if (fIconsConf.ReadStr(name, filename) && p.load(iconPath + QString::fromLocal8Bit(filename))) \
      myIconMap.insert(icon, p); \
    else \
      myIconMap.insert(icon, deficon);

#define LOAD_STATUSICON(name, protocol, icon) \
    if (fIconsConf.ReadStr(name, filename) && p.load(iconPath + QString::fromLocal8Bit(filename))) \
      myStatusIconMap.insert(QPair<ProtocolType, StatusIconType>(protocol, icon), p); \
    else \
      myStatusIconMap.remove(QPair<ProtocolType, StatusIconType>(protocol, icon));

  // ICQ/Default status icons
  LOAD_STATUSICON("Online",     ProtocolIcq, OnlineStatusIcon)
  LOAD_STATUSICON("FFC",        ProtocolIcq, FreeForChatStatusIcon)
  LOAD_STATUSICON("Offline",    ProtocolIcq, OfflineStatusIcon)
  LOAD_STATUSICON("Away",       ProtocolIcq, AwayStatusIcon)
  LOAD_STATUSICON("NA",         ProtocolIcq, NotAvailableStatusIcon)
  LOAD_STATUSICON("Occupied",   ProtocolIcq, OccupiedStatusIcon)
  LOAD_STATUSICON("DND",        ProtocolIcq, DoNotDisturbStatusIcon)
  LOAD_STATUSICON("Private",    ProtocolIcq, PrivateStatusIcon)

  // AIM status icons
  LOAD_STATUSICON("AIMOnline",  ProtocolAim, OnlineStatusIcon)
  LOAD_STATUSICON("AIMOffline", ProtocolAim, OfflineStatusIcon)
  LOAD_STATUSICON("AIMAway",    ProtocolAim, AwayStatusIcon)

  // MSN status icons
  LOAD_STATUSICON("MSNOnline",  ProtocolMsn, OnlineStatusIcon)
  LOAD_STATUSICON("MSNOffline", ProtocolMsn, OfflineStatusIcon)
  LOAD_STATUSICON("MSNAway",    ProtocolMsn, AwayStatusIcon)
  LOAD_STATUSICON("MSNOccupied",ProtocolMsn, OccupiedStatusIcon)
  LOAD_STATUSICON("MSNPrivate", ProtocolMsn, PrivateStatusIcon)

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
#ifdef HAVE_LIBGPGME
  LOAD2_ICON("GPGKey",          GpgKeyIcon, gpgon_xpm);
#endif

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

bool IconManager::loadExtendedIcons(QString iconSet)
{
  CIniFile fIconsConf;

  QString iconListName = iconSet + ".icons";
  QString subdir = QString(QTGUI_DIR) + EXTICONS_DIR + iconSet + "/";
  QString iconPath = QString::fromLocal8Bit(BASE_DIR) + subdir;
  if (!fIconsConf.LoadFile((iconPath + iconListName).toLocal8Bit()))
  {
    iconPath = QString::fromLocal8Bit(SHARE_DIR) + subdir;
    if (!fIconsConf.LoadFile((iconPath + iconListName).toLocal8Bit()))
      return false;
  }

  fIconsConf.SetSection("icons");

  char filename[MAX_FILENAME_LEN];

  QPixmap p;

#define LOAD_ICON(name, icon, deficon) \
    if (fIconsConf.ReadStr(name, filename) && p.load(iconPath + QString::fromLocal8Bit(filename))) \
      myIconMap.insert(icon, p); \
    else \
      myIconMap.insert(icon, deficon);

  LOAD_ICON("Birthday",             BirthdayIcon, birthday_xpm);
  LOAD_ICON("Cellular",             CellularIcon, cellular_xpm);
  LOAD_ICON("Collapsed",            CollapsedIcon, collapsed_xpm);
  LOAD_ICON("CustomAR",             CustomArIcon, autoresponse_xpm);
  LOAD_ICON("Expanded",             ExpandedIcon, expanded_xpm);
#ifdef HAVE_LIBGPGME
  LOAD_ICON("GPGKeyDisabled",       GpgKeyDisabledIcon, gpgoff_xpm);
  LOAD_ICON("GPGKeyEnabled",        GpgKeyEnabledIcon, gpgon_xpm);
#endif
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

const QPixmap& IconManager::iconForStatus(unsigned long fullStatus, QString id, unsigned long ppid)
{
  bool isAim = (ppid == LICQ_PPID) && (!id[0].isDigit());

  ProtocolType protocol = (isAim ? ProtocolAim : static_cast<ProtocolType>(ppid));
  StatusIconType statusIcon = OnlineStatusIcon;

  // cut off the flags, since we should not mind about them
  fullStatus &= ~ICQ_STATUS_FxFLAGS;

  if (fullStatus != ICQ_STATUS_OFFLINE &&
      (fullStatus & ICQ_STATUS_FxPRIVATE) &&
      !Config::ContactList::instance()->showExtendedIcons())
    statusIcon = PrivateStatusIcon;

  else if (fullStatus == ICQ_STATUS_OFFLINE)
    statusIcon = OfflineStatusIcon;

  else
  {
    if (protocol == ProtocolMsn)
    {
      if (fullStatus & (ICQ_STATUS_DND | ICQ_STATUS_OCCUPIED))
        statusIcon = OccupiedStatusIcon;

      else if (fullStatus & (ICQ_STATUS_NA | ICQ_STATUS_AWAY))
        statusIcon = AwayStatusIcon;
    }
    else if (protocol == ProtocolAim)
    {
      if (fullStatus & (ICQ_STATUS_DND | ICQ_STATUS_OCCUPIED | ICQ_STATUS_NA | ICQ_STATUS_AWAY))
        statusIcon = AwayStatusIcon;
    }
    else
    {
      if (fullStatus & ICQ_STATUS_DND)
        statusIcon = DoNotDisturbStatusIcon;

      else if (fullStatus & ICQ_STATUS_OCCUPIED)
        statusIcon = OccupiedStatusIcon;

      else if (fullStatus & ICQ_STATUS_NA)
        statusIcon = NotAvailableStatusIcon;

      else if (fullStatus & ICQ_STATUS_AWAY)
        statusIcon = AwayStatusIcon;

      else if ((fullStatus & ICQ_STATUS_FREEFORCHAT) &&
          myStatusIconMap.contains(
            QPair<ProtocolType, StatusIconType>(ProtocolIcq, FreeForChatStatusIcon)))
        statusIcon = FreeForChatStatusIcon;
    }
  }

  if (myStatusIconMap.contains(QPair<ProtocolType, StatusIconType>(protocol, statusIcon)))
    return myStatusIconMap[QPair<ProtocolType, StatusIconType>(protocol, statusIcon)];

  // No protocol specific icon existed so use default (same as ICQ)
  return myStatusIconMap[QPair<ProtocolType, StatusIconType>(ProtocolIcq, statusIcon)];
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
