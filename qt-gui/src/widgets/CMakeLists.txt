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
  ownercombobox.cpp
  protocombobox.cpp
  selectemoticon.cpp
  shortcutedit.cpp
  skinnablebutton.cpp
  skinnablecombobox.cpp
  skinnablelabel.cpp
  specialspinbox.cpp
  tabwidget.cpp
  timezoneedit.cpp
  treepager.cpp
)

if (HAVE_HUNSPELL)
  list(APPEND widgets_SRCS spellchecker.cpp)
endif (HAVE_HUNSPELL)

string(REGEX REPLACE ".cpp" ".h" widgets_MOC_HDRS "${widgets_SRCS}")
