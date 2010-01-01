project(console)
cmake_minimum_required(VERSION 2.6.0)
cmake_policy(VERSION 2.6)

# Licq
find_package(Licq REQUIRED)
include_directories(${Licq_INCLUDE_DIRS})

# Generate pluginversion.h
include(version.cmake)
licq_create_plugin_version_file(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# NCruses
set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)

# CDK
find_path(CDK_INCLUDE_DIR cdk.h PATH_SUFFIXES cdk)
if (CDK_INCLUDE_DIR)
  find_library(CDK_LIBRARY cdk)
endif (CDK_INCLUDE_DIR)

if (NOT CDK_LIBRARY)
  message(FATAL_ERROR "Could NOT find Curses Development Kit (CDK)")
endif (NOT CDK_LIBRARY)

add_subdirectory(src)
