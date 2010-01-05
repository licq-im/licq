# User dialog
set(userdlg_SRCS
  info.cpp
  settings.cpp
  userdlg.cpp
)

string(REGEX REPLACE ".cpp" ".h" userdlg_MOC_HDRS "${userdlg_SRCS}")
