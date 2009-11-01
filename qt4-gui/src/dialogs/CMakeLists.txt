# Various Dialogs
set(dialogs_SRCS
  aboutdlg.cpp
  adduserdlg.cpp
  authuserdlg.cpp
  awaymsgdlg.cpp
  chatdlg.cpp
  customautorespdlg.cpp
  editcategorydlg.cpp
  editfiledlg.cpp
  editfilelistdlg.cpp
  editgrpdlg.cpp
  filedlg.cpp
  forwarddlg.cpp
  gpgkeymanager.cpp
  gpgkeyselect.cpp
  hintsdlg.cpp
  historydlg.cpp
  joinchatdlg.cpp
  keyrequestdlg.cpp
  logwindow.cpp
  mmsenddlg.cpp
  ownereditdlg.cpp
  ownermanagerdlg.cpp
  phonedlg.cpp
  plugindlg.cpp
  randomchatdlg.cpp
  refusedlg.cpp
  registeruser.cpp
  reqauthdlg.cpp
  searchuserdlg.cpp
  securitydlg.cpp
  showawaymsgdlg.cpp
  statsdlg.cpp
  userselectdlg.cpp
  utilitydlg.cpp
)

string(REGEX REPLACE ".cpp" ".h" dialogs_HDRS "${dialogs_SRCS}")

qt4_wrap_cpp(dialogs_MOC_SRCS ${dialogs_HDRS})

set(dialogs ${dialogs_SRCS} ${dialogs_MOC_SRCS})
add_library(dialogs STATIC ${dialogs})
