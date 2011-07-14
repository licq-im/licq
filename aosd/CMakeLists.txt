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
pkg_check_modules(PANGO REQUIRED pangocairo)
pkg_check_modules(AOSD REQUIRED libaosd-text>=0.2.7)
add_definitions(${AOSD_CFLAGS})
pkg_check_modules(GLIB REQUIRED glib-2.0>=2.0)
add_definitions(${GLIB_CFLAGS})
include_directories(${AOSD_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS})
link_directories(${AOSD_LIBRARY_DIRS} ${GLIB_LIBRARY_DIRS})

add_subdirectory(src)
