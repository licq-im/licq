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

string(REGEX REPLACE ".cpp" ".h" config_HDRS "${config_SRCS}")

qt4_wrap_cpp(config_MOC_SRCS ${config_HDRS})

set(config ${config_SRCS} ${config_MOC_SRCS})
add_library(config STATIC ${config})
