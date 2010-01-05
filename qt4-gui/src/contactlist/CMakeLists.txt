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

string(REGEX REPLACE ".cpp" ".h" contactlist_MOC_HDRS "${contactlist_SRCS}")
