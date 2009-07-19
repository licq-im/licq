# ContactList Model
set(contactlist_SRCS
  contactbar.cpp
  contactgroup.cpp
  contactitem.cpp
  contactlist.cpp
  contactuser.cpp
  contactuserdata.cpp
  maincontactlistproxy.cpp
  mode2contactlistproxy.cpp
  multicontactproxy.cpp
  singlecontactproxy.cpp
  sortedcontactlistproxy.cpp
)

string(REGEX REPLACE ".cpp" ".h" contactlist_HDRS "${contactlist_SRCS}")

qt4_wrap_cpp(contactlist_MOC_SRCS ${contactlist_HDRS})

set(contactlist ${contactlist_SRCS} ${contactlist_MOC_SRCS})
add_library(contactlist STATIC ${contactlist})
