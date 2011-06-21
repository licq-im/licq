set(osd_SRCS
  licq-osd.cpp
  my_xosd.cpp
  plugin.cpp
)

licq_add_plugin(licq_osd ${osd_SRCS})

add_definitions(-DPACKAGE="licq_osd_plugin")
add_definitions(-DLOCALEDIR="${Licq_LOCALE_DIR}")

if (ICONV_SECOND_ARGUMENT_IS_CONST)
  add_definitions(-DICONV_CONST=const)
else (ICONV_SECOND_ARGUMENT_IS_CONST)
  add_definitions(-DICONV_CONST=)
endif (ICONV_SECOND_ARGUMENT_IS_CONST)

include_directories(${XOSD_INCLUDE_DIR})
include_directories(${ICONV_INCLUDE_DIR})

target_link_libraries(licq_osd ${XOSD_LIBRARY})
target_link_libraries(licq_osd ${ICONV_LIBRARIES})
