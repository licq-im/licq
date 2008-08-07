# User dialog
set(userdlg_SRCS
  info.cpp
  settings.cpp
  userdlg.cpp
)

string(REGEX REPLACE ".cpp" ".h" userdlg_HDRS "${userdlg_SRCS}")

qt4_wrap_cpp(userdlg_MOC_SRCS ${userdlg_HDRS})

set(userdlg ${userdlg_SRCS} ${userdlg_MOC_SRCS})
add_library(userdlg STATIC ${userdlg})
