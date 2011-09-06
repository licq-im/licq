project(console)
cmake_minimum_required(VERSION 2.6.0)
cmake_policy(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

# Licq
find_package(Licq REQUIRED)
include_directories(${Licq_INCLUDE_DIRS})

# Generate pluginversion.h
include(version.cmake)
licq_create_plugin_version_file(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# NCurses
set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)

# CDK
find_package(CDK REQUIRED)

add_subdirectory(src)
