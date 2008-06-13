set(widgets_SRCS
  calendar.cpp
  colorbutton.cpp
  filenameedit.cpp
  fontedit.cpp
  groupcombobox.cpp
  historyview.cpp
  infofield.cpp
  messagelist.cpp
  mledit.cpp
  mlview.cpp
  protocombobox.cpp
  skinnablebutton.cpp
  skinnablecombobox.cpp
  skinnablelabel.cpp
  tabwidget.cpp
  timezoneedit.cpp
  treepager.cpp
)

string(REGEX REPLACE ".cpp" ".h" widgets_HDRS "${widgets_SRCS}")

qt4_wrap_cpp(widget_MOC_SRCS ${widgets_HDRS})

set(widgets ${widgets_SRCS} ${widget_MOC_SRCS})
add_library(widgets STATIC ${widgets})
