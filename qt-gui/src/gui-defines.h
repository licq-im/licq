#ifndef GUIDEFINES_H
#define GUIDEFINES_H

enum UserMenu {
  mnuUserView,
  mnuUserSendMsg,
  mnuUserSendUrl,
  mnuUserSendChat,
  mnuUserSendFile,
  mnuUserAuthorize,
  mnuUserCheckResponse,
  mnuUserGeneral,
  mnuUserMore,
  mnuUserWork,
  mnuUserAbout,
  mnuUserHistory,
  mnuUserOnlineNotify,
  mnuUserInvisibleList,
  mnuUserVisibleList,
  mnuUserIgnoreList
};

enum OwnerMenu_Identifiers {
  OwnerMenuView = mnuUserView,
  OwnerMenuGeneral = mnuUserGeneral,
  OwnerMenuMore = mnuUserMore,
  OwnerMenuWork = mnuUserWork,
  OwnerMenuAbout = mnuUserAbout,
  OwnerMenuHistory = mnuUserHistory,
  OwnerMenuSecurity,
  OwnerMenuPassword
};

const unsigned short MNUxITEM_MINIxMODE            = 7;
const unsigned short MNUxITEM_SHOWxOFFLINE         = 8;

const unsigned short MNUxITEM_STATUSxINVISIBLE     = 8;

const unsigned short MNUxITEM_DEBUGxALL            = 6;
const unsigned short MNUxITEM_DEBUGxNONE           = 7;


#endif
