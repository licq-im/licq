# Settings dialog
set(settings_SRCS
  chat.cpp
  contactlist.cpp
  events.cpp
  general.cpp
  network.cpp
  oneventbox.cpp
  plugins.cpp
  settingsdlg.cpp
  shortcuts.cpp
  skin.cpp
  status.cpp
)

string(REGEX REPLACE ".cpp" ".h" settings_MOC_HDRS "${settings_SRCS}")
