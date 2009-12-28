project(osd)
cmake_minimum_required(VERSION 2.6.0)
cmake_policy(VERSION 2.6)

# Licq
find_package(Licq REQUIRED)
include_directories(${Licq_INCLUDE_DIRS})

# libxosd
find_path(XOSD_INCLUDE_DIR xosd.h)
if (XOSD_INCLUDE_DIR)
  find_library(XOSD_LIBRARY xosd)
endif (XOSD_INCLUDE_DIR)

if (NOT XOSD_LIBRARY)
  message(FATAL_ERROR "Could NOT find libxosd")
endif (NOT XOSD_LIBRARY)

add_subdirectory(po)
add_subdirectory(src)
