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
  mnuUserMore,
  mnuUserWork,
  mnuUserAbout,
  mnuUserLast,
  mnuUserHistory,
  mnuUserFloaty,
  mnuUserRemoveFromList
};

enum OwnerMenu_Identifiers {
  OwnerMenuView = mnuUserView,
  OwnerMenuGeneral = mnuUserGeneral,
  OwnerMenuMore = mnuUserMore,
  OwnerMenuWork = mnuUserWork,
  OwnerMenuAbout = mnuUserAbout,
  OwnerMenuLast = mnuUserLast,
  OwnerMenuHistory = mnuUserHistory,
  OwnerMenuSecurity,
  OwnerMenuPassword,
  OwnerMenuSavedPassword,
  OwnerMenuRandomChat
};

const unsigned short MNUxITEM_MINIxMODE            = 7;
const unsigned short MNUxITEM_SHOWxOFFLINE         = 8;
const unsigned short MNUxITEM_THREADxVIEW          = 9;

const unsigned short MNUxITEM_STATUSxINVISIBLE     = 8;

const unsigned short MNUxITEM_DEBUGxALL            = 6;
const unsigned short MNUxITEM_DEBUGxNONE           = 7;

enum DockMode
{
  DockNone = 0,
  DockDefault = 1,
  DockThemed = 2
};



#endif
