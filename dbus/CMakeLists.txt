project(dbus)
cmake_minimum_required(VERSION 2.6.0)
cmake_policy(VERSION 2.6)

# Licq
find_package(Licq REQUIRED)
include_directories(${Licq_INCLUDE_DIRS})

# Generate pluginversion.h
include(version.cmake)
licq_create_plugin_version_file(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Find libdbus
find_package(PkgConfig REQUIRED)
pkg_check_modules(dbus REQUIRED dbus-1>=1.0.0)
add_definitions(${dbus_CFLAGS})
include_directories(${dbus_INCLUDE_DIRS})
link_directories(${dbus_LIBRARY_DIRS})

add_subdirectory(src)
