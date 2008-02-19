# Dock Icons
set(dockicons_SRCS
  dockicon.cpp
  systemtrayicon.cpp
)

# Other dock icons not available with kde
if (NOT WITH_KDE)
  list(APPEND dockicons_SRCS dockiconwidget.cpp defaultdockicon.cpp themeddockicon.cpp)
endif (NOT WITH_KDE)

string(REGEX REPLACE ".cpp" ".h" dockicons_HDRS "${dockicons_SRCS}")

qt4_wrap_cpp(dockicons_MOC_SRCS ${dockicons_HDRS})

set(dockicons ${dockicons_SRCS} ${dockicons_MOC_SRCS})
add_library(dockicons STATIC ${dockicons})
