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

# KDE specific stuff
# if (WITH_KDE)
#   list(APPEND core_SRCS licqkimiface.cpp wrap_kde_malloc.cpp)

#   kde4_add_dcop_skels(core_SRCS kimiface.h)
#   kde4_add_dcop_stubs(core_SRCS kimiface.h)
# endif (WITH_KDE)

string(REGEX REPLACE ".cpp" ".h" core_HDRS "${core_SRCS}")
string(REGEX REPLACE "(wrap_kde_malloc.h|plugin.h)" "" core_MOC_HDRS "${core_HDRS}")

qt4_wrap_cpp(core_MOC_SRCS ${core_MOC_HDRS})

set(core ${core_SRCS} ${core_MOC_SRCS})
add_library(core STATIC ${core})
