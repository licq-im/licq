set(userevents_SRCS
  emoticonlabel.cpp
  selectemoticon.cpp
  usereventcommon.cpp
  usereventtabdlg.cpp
  usersendevent.cpp
  userviewevent.cpp
)

string(REGEX REPLACE ".cpp" ".h" userevents_MOC_HDRS "${userevents_SRCS}")
