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

string(REGEX REPLACE ".cpp" ".h" userevents_MOC_HDRS "${userevents_SRCS}")
