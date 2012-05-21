# User dialog
set(userdlg_SRCS
  info.cpp
  owner.cpp
  settings.cpp
  userdlg.cpp
)

string(REGEX REPLACE ".cpp" ".h" userdlg_MOC_HDRS "${userdlg_SRCS}")
