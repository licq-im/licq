# Various Dialogs
set(dialogs_SRCS
  aboutdlg.cpp
  addgroupdlg.cpp
  adduserdlg.cpp
  authdlg.cpp
  awaymsgdlg.cpp
  chatdlg.cpp
  customautorespdlg.cpp
  editcategorydlg.cpp
  editfiledlg.cpp
  editfilelistdlg.cpp
  editgrpdlg.cpp
  filedlg.cpp
  filterruledlg.cpp
  forwarddlg.cpp
  gpgkeymanager.cpp
  gpgkeyselect.cpp
  groupdlg.cpp
  hintsdlg.cpp
  historydlg.cpp
  joinchatdlg.cpp
  keyrequestdlg.cpp
  logwindow.cpp
  mmsenddlg.cpp
  ownereditdlg.cpp
  ownermanagerdlg.cpp
  phonedlg.cpp
  randomchatdlg.cpp
  refusedlg.cpp
  searchuserdlg.cpp
  showawaymsgdlg.cpp
  statsdlg.cpp
  utilitydlg.cpp
)

string(REGEX REPLACE ".cpp" ".h" dialogs_MOC_HDRS "${dialogs_SRCS}")
