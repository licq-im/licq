project(msn)
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

# strndup
check_function_exists(strndup have_strndup)
if (have_strndup)
  add_definitions(-DHAVE_STRNDUP=1)
endif (have_strndup)

add_subdirectory(src)
