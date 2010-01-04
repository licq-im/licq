set(console_SRCS
  console.cpp
  console_menu.cpp
  console_print.cpp
  console_tab.cpp
  main.cpp
  window.cpp
)

licq_add_plugin(licq_console ${console_SRCS})

include_directories(${CURSES_INCLUDE_DIR})
include_directories(${CDK_INCLUDE_DIR})

target_link_libraries(licq_console ${CURSES_LIBRARIES})
target_link_libraries(licq_console ${CDK_LIBRARY})
