set(core_SRCS
  groupmenu.cpp
  licqgui.cpp
  mainwin.cpp
  messagebox.cpp
  plugin.cpp
  signalmanager.cpp
  systemmenu.cpp
  usermenu.cpp
)

string(REGEX REPLACE ".cpp" ".h" core_MOC_HDRS "${core_SRCS}")
