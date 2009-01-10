# Settings dialog
set(settings_SRCS
  chat.cpp
  contactlist.cpp
  events.cpp
  general.cpp
  network.cpp
  settingsdlg.cpp
  shortcuts.cpp
  skin.cpp
  status.cpp
)

string(REGEX REPLACE ".cpp" ".h" settings_HDRS "${settings_SRCS}")

qt4_wrap_cpp(settings_MOC_SRCS ${settings_HDRS})

set(settings ${settings_SRCS} ${settings_MOC_SRCS})
add_library(settings STATIC ${settings})
