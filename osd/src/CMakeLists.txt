set(osd_SRCS
  licq-osd.cpp
  my_xosd.cpp
)

licq_add_plugin(licq_osd ${osd_SRCS})

add_definitions(-DPACKAGE="licq_osd_plugin")
add_definitions(-DLOCALEDIR="${Licq_LOCALE_DIR}")

include_directories(${XOSD_INCLUDE_DIR})
target_link_libraries(licq_osd xosd)
