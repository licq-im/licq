# Configuration data
set(config_SRCS
  chat.cpp
  contactlist.cpp
  emoticons.cpp
  general.cpp
  iconmanager.cpp
  shortcuts.cpp
  skin.cpp
)

string(REGEX REPLACE ".cpp" ".h" config_MOC_HDRS "${config_SRCS}")
