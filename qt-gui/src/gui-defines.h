#ifndef GUIDEFINES_H
#define GUIDEFINES_H

#define QTGUI_DIR "qt-gui/"

#define NUM_MSG_PER_HISTORY 20
#define COLOR_SENT "blue"
#define COLOR_RECEIVED "red"


enum UserMenu {
  mnuUserView,
  mnuUserSendMsg,
  mnuUserSendUrl,
  mnuUserSendChat,
  mnuUserSendFile,
  mnuUserSendContact,
  mnuUserAuthorize,
  mnuUserCheckResponse,
  mnuUserGeneral,
  mnuUserMore,
  mnuUserWork,
  mnuUserAbout,
  mnuUserHistory,
  mnuUserNewUser,
  mnuUserOnlineNotify,
  mnuUserInvisibleList,
  mnuUserVisibleList,
  mnuUserIgnoreList,
  mnuUserFloaty
};

enum OwnerMenu_Identifiers {
  OwnerMenuView = mnuUserView,
  OwnerMenuGeneral = mnuUserGeneral,
  OwnerMenuMore = mnuUserMore,
  OwnerMenuWork = mnuUserWork,
  OwnerMenuAbout = mnuUserAbout,
  OwnerMenuHistory = mnuUserHistory,
  OwnerMenuSecurity,
  OwnerMenuPassword,
  OwnerMenuRandomChat
};

const unsigned short MNUxITEM_MINIxMODE            = 7;
const unsigned short MNUxITEM_SHOWxOFFLINE         = 8;

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
