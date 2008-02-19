set(userevents_SRCS
  emoticonlabel.cpp
  selectemoticon.cpp
  usereventcommon.cpp
  usereventtabdlg.cpp
  usersendchatevent.cpp
  usersendcommon.cpp
  usersendcontactevent.cpp
  usersendfileevent.cpp
  usersendmsgevent.cpp
  usersendsmsevent.cpp
  usersendurlevent.cpp
  userviewevent.cpp
)

string(REGEX REPLACE ".cpp" ".h" userevents_HDRS "${userevents_SRCS}")

qt4_wrap_cpp(userevents_MOC_SRCS ${userevents_HDRS})

set(userevents ${userevents_SRCS} ${userevents_MOC_SRCS})
add_library(userevents STATIC ${userevents})
