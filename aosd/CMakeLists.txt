project(aosd)
cmake_minimum_required(VERSION 2.6.0)
cmake_policy(VERSION 2.6)

# Licq
find_package(Licq REQUIRED)
include_directories(${Licq_INCLUDE_DIRS})

# Generate pluginversion.h
include(version.cmake)
licq_create_plugin_version_file(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Atheme OSD text
find_package(PkgConfig REQUIRED)
pkg_check_modules(AOSD REQUIRED libaosd-text>=0.2.7)
add_definitions(${AOSD_CFLAGS})
include_directories(${AOSD_INCLUDE_DIRS})
link_directories(${AOSD_LIBRARY_DIRS})

add_subdirectory(src)
