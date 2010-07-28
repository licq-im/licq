project(osd)
cmake_minimum_required(VERSION 2.6.0)
cmake_policy(VERSION 2.6)

# Licq
find_package(Licq REQUIRED)
include_directories(${Licq_INCLUDE_DIRS})

# Licq cmake modules
set(CMAKE_MODULE_PATH ${Licq_CMAKE_DIR}/Modules)

# Generate pluginversion.h
include(version.cmake)
licq_create_plugin_version_file(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Xosd
find_path(XOSD_INCLUDE_DIR xosd.h)
if (XOSD_INCLUDE_DIR)
  find_library(XOSD_LIBRARY xosd)
endif (XOSD_INCLUDE_DIR)

if (NOT XOSD_LIBRARY)
  message(FATAL_ERROR "Could NOT find Xosd")
endif (NOT XOSD_LIBRARY)

# iconv
find_package(Iconv REQUIRED)

add_subdirectory(po)
add_subdirectory(src)
