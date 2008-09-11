project(Qt4-GUI)
cmake_minimum_required(VERSION 2.4.2)
include(CheckIncludeFile)
include(TestCXXAcceptsFlag)
include(CheckLibraryExists)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

# Qt-GUI version
set(VERSION_MAJOR 1)
set(VERSION_MINOR 3)
set(VERSION_RELEASE 6)

# Use KDE support
option(WITH_KDE "Compile with KDE support" OFF)


# Install dirs
set(INSTALL_LIB_DIR lib CACHE PATH
  "Directory to install the plugin to. A relative path is relative to CMAKE_INSTALL_PREFIX.")
mark_as_advanced(INSTALL_LIB_DIR)

set(INSTALL_DATA_DIR share CACHE PATH
  "Directory to install shared data to. A relative path is relative to CMAKE_INSTALL_PREFIX.")
mark_as_advanced(INSTALL_DATA_DIR)

set(INSTALL_QTGUI_DIR qt4-gui CACHE PATH
  "Icons etc. will be installed to INSTALL_DATA_DIR/licq/INSTALL_QTGUI_DIR.")
mark_as_advanced(INSTALL_QTGUI_DIR)


# Configure compiler flags
check_cxx_accepts_flag(-Wall CXX_ACCEPTS_WALL)
if (CXX_ACCEPTS_WALL)
  add_definitions(-Wall)
endif (CXX_ACCEPTS_WALL)

check_cxx_accepts_flag(-Wextra CXX_ACCEPTS_WEXTRA)
if (CXX_ACCEPTS_WEXTRA)
  add_definitions(-Wextra)
endif (CXX_ACCEPTS_WEXTRA)

check_cxx_accepts_flag(-fPIC CXX_ACCEPTS_FPIC)
if (CXX_ACCEPTS_FPIC)
  add_definitions(-fPIC)
endif (CXX_ACCEPTS_FPIC)


# Holds all libraries we should link to
set(LIBRARIES)

# Check for X11 and (optional) screensaver support
if (NOT APPLE)
  find_package(X11 REQUIRED)
  include_directories(${X11_INCLUDE_DIR})

  set(CMAKE_REQUIRED_LIBRARIES ${X11_LIBRARIES})
  check_library_exists(Xss XScreenSaverRegister "" HAVE_LIBXSS)
  check_include_file(X11/extensions/scrnsaver.h HAVE_SCRNSAVER_H)
  set(CMAKE_REQUIRED_LIBRARIES)

  if (HAVE_SCRNSAVER_H AND HAVE_LIBXSS)
    set(X11_LIBRARIES ${X11_LIBRARIES} Xss)
    set(USE_SCRNSAVER 1)
  endif (HAVE_SCRNSAVER_H AND HAVE_LIBXSS)
  set(LIBRARIES ${LIBRARIES} ${X11_LIBRARIES})
endif (NOT APPLE)


# Check for Qt4
set(QT_MIN_VERSION "4.3.0")

# Check for Qt unless WITH_KDE is requested
if (NOT WITH_KDE)
  find_package(Qt4 REQUIRED)
else (NOT WITH_KDE)
  find_package(KDE4 REQUIRED)
  set(USE_KDE ${KDE4_FOUND})

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

  add_definitions(${KDE4_DEFINITIONS})
  link_directories(${KDE4_LIB_DIR})

  include_directories(${KDE4_INCLUDE_DIR})
  set(LIBRARIES ${LIBRARIES}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS})
endif (NOT WITH_KDE)

include_directories(${QT_INCLUDE_DIR}
  ${QT_QTCORE_INCLUDE_DIR}
  ${QT_QTGUI_INCLUDE_DIR}
  ${QT_QTXML_INCLUDE_DIR})
set(LIBRARIES ${LIBRARIES}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTXML_LIBRARY})


# Optional check for gpgme
find_package(gpgme)
if (GPGME_FOUND)
  set(HAVE_LIBGPGME 1)
  add_definitions(${GPGME_DEFINITIONS})
  include_directories(${GPGME_INCLUDE_DIRS})
  set(LIBRARIES ${LIBRARIES} ${GPGME_LIBRARIES})
endif (GPGME_FOUND)

# Check for boost, used in Licq includes so we must have it
find_package(Boost 1.31.1 COMPONENTS smart_ptr)
link_directories ( ${Boost_LIBRARY_DIRS} )
include_directories ( ${Boost_INCLUDE_DIRS} )
set(LIBRARIES ${LIBRARIES} ${Boost_LIBRARIES})

# Licq headers (see cmake --help-command find_path
# for the reason we call find_path twice)
find_path(LICQ_INCLUDE_DIR NAMES licq_icqd.h
          PATHS ${CMAKE_SOURCE_DIR}/../../include
                ${CMAKE_SOURCE_DIR}/../licq/include
          NO_DEFAULT_PATHS)
find_path(LICQ_INCLUDE_DIR NAMES licq_icqd.h)

if (NOT LICQ_INCLUDE_DIR)
  message(FATAL_ERROR "Licq header files not found. Please specify the location with -DLICQ_INCLUDE_DIR")
endif (NOT LICQ_INCLUDE_DIR)

message(STATUS "Found Licq include dir: ${LICQ_INCLUDE_DIR}")
include_directories(${LICQ_INCLUDE_DIR})

# Local includes
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Misc headers
check_include_file(locale.h HAVE_LOCALE_H)


# To find config.h
include_directories(${CMAKE_BINARY_DIR})

# Generate config.h
configure_file(${CMAKE_SOURCE_DIR}/config.h.cmake ${CMAKE_BINARY_DIR}/config.h)

add_subdirectory(doc)
add_subdirectory(po)
add_subdirectory(share)
add_subdirectory(src)
