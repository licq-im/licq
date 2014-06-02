project(jabber)
cmake_minimum_required(VERSION 2.6.0)
cmake_policy(VERSION 2.6)

include(CheckFunctionExists)

# Licq
find_package(Licq REQUIRED)
include_directories(${Licq_INCLUDE_DIRS})

# Generate pluginversion.h
include(version.cmake)
licq_create_plugin_version_file(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Find gloox
find_package(PkgConfig REQUIRED)
pkg_check_modules(gloox REQUIRED gloox>=1.0)
add_definitions(${gloox_CFLAGS})
include_directories(${gloox_INCLUDE_DIRS})
link_directories(${gloox_LIBRARY_DIRS})

# See if timegm(3) exists
check_function_exists(timegm HAVE_TIMEGM)
if (HAVE_TIMEGM)
  add_definitions("-DHAVE_TIMEGM=1")
endif (HAVE_TIMEGM)

add_subdirectory(src)
