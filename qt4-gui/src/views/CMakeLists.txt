set(views_SRCS
  contactdelegate.cpp
  floatyview.cpp
  mmuserview.cpp
  userview.cpp
  userviewbase.cpp
)

string(REGEX REPLACE ".cpp" ".h" views_MOC_HDRS "${views_SRCS}")
