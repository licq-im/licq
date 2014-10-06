set(userevents_SRCS
  usereventcommon.cpp
  usereventtabdlg.cpp
  usersendevent.cpp
  userviewevent.cpp
)

string(REGEX REPLACE ".cpp" ".h" userevents_MOC_HDRS "${userevents_SRCS}")
