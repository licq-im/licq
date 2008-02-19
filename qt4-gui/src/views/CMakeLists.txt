set(views_SRCS
  contactdelegate.cpp
  floatyview.cpp
  mmuserview.cpp
  userview.cpp
  userviewbase.cpp
)

string(REGEX REPLACE ".cpp" ".h" views_HDRS "${views_SRCS}")

qt4_wrap_cpp(views_MOC_SRCS ${views_HDRS})

set(views ${views_SRCS} ${views_MOC_SRCS})
add_library(views STATIC ${views})
