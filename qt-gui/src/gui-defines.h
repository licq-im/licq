#ifndef GUIDEFINES_H
#define GUIDEFINES_H

#define QTGUI_DIR "qt-gui/"
#define EMOTICONS_DIR "emoticons/"

#define NUM_MSG_PER_HISTORY 40
#define COLOR_SENT "blue"
#define COLOR_RECEIVED "red"


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
  mnuUserCheckResponse,
  mnuUserCustomAutoResponse,
  mnuUserGeneral,
  mnuUserHistory,
  mnuUserFloaty,
  mnuUserRemoveFromList,
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
  OwnerMenuRandomChat
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
  DockThemed = 2
};



#endif
