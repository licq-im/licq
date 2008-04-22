/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2006 Licq developers
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

#ifndef GUIDEFINES_H
#define GUIDEFINES_H

#ifdef USE_KDE
# define PLUGIN_NAME "kde-gui"
#else
# define PLUGIN_NAME "qt-gui"
#endif

#define QTGUI_DIR "qt-gui/"
#define EMOTICONS_DIR "emoticons/"
#define QTGUI_CONFIGFILE "licq_qt-gui.conf"

#define NUM_MSG_PER_HISTORY 40
#define COLOR_SENT "blue"
#define COLOR_RECEIVED "red"

// Defined here until there is a way to get constant from daemon
#define MSN_PPID 0x4D534E5F

enum UserMenu {
  mnuUserView,
  mnuUserSendMsg,
  mnuUserSendUrl,
  mnuUserSendChat,
  mnuUserSendFile,
  mnuUserSendContact,
  mnuUserSendSms,
  mnuUserSendKey,
  mnuUserAuthorize,
  mnuUserAuthorizeRequest,
  mnuUserCheckIfInvisible,
  mnuUserCheckResponse,
  mnuUserCustomAutoResponse,
  mnuUserGeneral,
  mnuUserHistory,
  mnuUserFloaty,
  mnuUserRemoveFromList,
  mnuUserSelectGPGKey,
  mnuUserSendInfoPluginListRequest,
  mnuUserSendStatusPluginListRequest,
  mnuUserSendPhoneFollowMeRequest,
  mnuUserSendICQphoneRequest,
  mnuUserSendFileServerRequest
};

enum OwnerMenu_Identifiers {
  OwnerMenuView = mnuUserView,
  OwnerMenuGeneral = mnuUserGeneral,
  OwnerMenuHistory = mnuUserHistory,
  OwnerMenuSecurity,
  OwnerMenuRandomChat,
  OwnerMenuManager
};

const unsigned short MNUxITEM_MINIxMODE            = 7;
const unsigned short MNUxITEM_SHOWxOFFLINE         = 8;
const unsigned short MNUxITEM_THREADxVIEW          = 9;

const unsigned short MNUxITEM_STATUSxINVISIBLE     = 8;

const unsigned short MNUxITEM_DEBUGxALL            = 6;
const unsigned short MNUxITEM_DEBUGxNONE           = 7;

const int CHANGE_STATUS_ONLINE          = 0x0000;
const int CHANGE_STATUS_FFC             = 0x0001;
const int CHANGE_STATUS_AWAY            = 0x0002;
const int CHANGE_STATUS_NA              = 0x0004;
const int CHANGE_STATUS_DND             = 0x0008;
const int CHANGE_STATUS_OCC             = 0x0010;
const int CHANGE_STATUS_OFFLINE         = 0x0020;
const int CHANGE_STATUS_PRV             = 0x0040;

enum DockMode
{
  DockNone = 0,
  DockDefault = 1,
  DockThemed = 2,
  DockSmall = 3
};



#endif
