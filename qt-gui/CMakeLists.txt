project(Qt4-GUI)
cmake_minimum_required(VERSION 2.4.2)
include(CheckIncludeFile)
include(TestCXXAcceptsFlag)
include(CheckLibraryExists)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

# Use KDE support
option(WITH_KDE "Compile with KDE support" OFF)

# Licq
find_package(Licq REQUIRED)
include_directories(${Licq_INCLUDE_DIRS})

# Generate pluginversion.h
include(version.cmake)
licq_create_plugin_version_file(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(INSTALL_QTGUI_DIR qt4-gui CACHE STRING
  "Icons etc. will be installed to ${Licq_SHARE_DIR}/INSTALL_QTGUI_DIR.")
mark_as_advanced(INSTALL_QTGUI_DIR)

# Holds all libraries we should link to
set(LIBRARIES)

# Check for X11 and (optional) screensaver support
if (NOT APPLE)
  find_package(X11 REQUIRED)

  if (X11_Xscreensaver_FOUND)
    set(X11_LIBRARIES ${X11_LIBRARIES} ${X11_Xscreensaver_LIB})
    set(USE_SCRNSAVER 1)
  endif (X11_Xscreensaver_FOUND)

  include_directories(${X11_INCLUDE_DIR})
  set(LIBRARIES ${LIBRARIES} ${X11_LIBRARIES})
endif (NOT APPLE)


# Check for Qt4
set(QT_MIN_VERSION "4.3.0")

# Check for Qt unless WITH_KDE is requested
if (NOT WITH_KDE)
  find_package(Qt4 REQUIRED)

  # Check for HunSpell
  find_package(Hunspell)
  if (HUNSPELL_FOUND)
    set(HAVE_HUNSPELL 1)
    add_definitions(${HUNSPELL_DEFINITIONS})
    include_directories(${HUNSPELL_INCLUDE_DIRS})
    set(LIBRARIES ${LIBRARIES} ${HUNSPELL_LIBRARIES})
  endif (HUNSPELL_FOUND)

else (NOT WITH_KDE)
  # Stop KDE4 from adding an uninstall target
  set(_kde4_uninstall_rule_created TRUE)

  find_package(KDE4 REQUIRED)
  set(USE_KDE ${KDE4_FOUND})

  # Unset variables set by FindKDE4Internal
  set(LIBRARY_OUTPUT_PATH)

  # FindKDE4Internal adds "-Wl,--no-undefined" to the linker command
  # line. This doesn't work for use, since all symbols from the daemon
  # are undefined. So we remove it.
  string(REPLACE "-Wl,--no-undefined" ""
    CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS}")

  # FindKDE4Internal adds "-fvisibility=hidden" to the compiler
  # command line. This hides symbols in the plugin that the daemon
  # looks for. So we remove it.
  string(REPLACE "-fvisibility=hidden" ""
    CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

  # FindKDE4Internal disables exceptions, but the daemon is built with
  # exceptions so we must have them enabled here as well.
  string(REPLACE "-fno-exceptions" ""
    CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  string(REPLACE "-DQT_NO_EXCEPTIONS" ""
    CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

  add_definitions(${KDE4_DEFINITIONS})
  link_directories(${KDE4_LIB_DIR})

  include_directories(${KDE4_INCLUDE_DIR})
  set(LIBRARIES ${LIBRARIES}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS})
endif (NOT WITH_KDE)

set(QT_USE_QTXML 1)
include(${QT_USE_FILE})
set(LIBRARIES ${LIBRARIES} ${QT_LIBRARIES})

# Local includes
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Misc headers
check_include_file(locale.h HAVE_LOCALE_H)


# To find config.h
include_directories(BEFORE ${CMAKE_CURRENT_BINARY_DIR})

# Generate config.h
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_subdirectory(doc)
add_subdirectory(po)
add_subdirectory(share)
add_subdirectory(src)
