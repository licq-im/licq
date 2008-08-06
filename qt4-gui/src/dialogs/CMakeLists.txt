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
  skinbrowser.cpp
  statsdlg.cpp
  userselectdlg.cpp
  utilitydlg.cpp
)

# GPG specific stuff
if (GPGME_FOUND)
  list(APPEND dialogs_SRCS gpgkeymanager.cpp gpgkeyselect.cpp)
endif (GPGME_FOUND)

string(REGEX REPLACE ".cpp" ".h" dialogs_HDRS "${dialogs_SRCS}")

qt4_wrap_cpp(dialogs_MOC_SRCS ${dialogs_HDRS})

set(dialogs ${dialogs_SRCS} ${dialogs_MOC_SRCS})
add_library(dialogs STATIC ${dialogs})
