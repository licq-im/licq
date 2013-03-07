project(Licq)
cmake_minimum_required(VERSION 2.6.0)
cmake_policy(VERSION 2.6)

# Internal cmake modules
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules)

# Includes
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckFileOffsetBits)

# Options
option(ENABLE_NLS "Enable translation of program messages" ON)
option(USE_FIFO "Use Licq FIFO" ON)
option(USE_HEBREW "Include support for hebrew reverse string" OFF)
option(USE_OPENSSL "Enable secure communication channels" ON)
option(USE_SOCKS5 "Enable socks5 support" OFF)
option(BUILD_PLUGINS "Build all plugins" OFF)
option(BUILD_TESTS "Build all unit tests" ON)

set(DLOPEN_POLICY RTLD_NOW CACHE STRING "2nd dlopen parameter")
mark_as_advanced(DLOPEN_POLICY)

include(cmake/LicqCommonCompilerFlags.cmake)

# Network libraries (needed on Solaris)
check_library_exists(socket getaddrinfo "" HAVE_LIBSOCKET)
if (HAVE_LIBSOCKET)
  set(SOCKET_LIBRARIES ${SOCKET_LIBRARIES} socket)
endif (HAVE_LIBSOCKET)
check_library_exists(nsl gethostbyname "" HAVE_LIBNSL)
if (HAVE_LIBNSL)
  set(SOCKET_LIBRARIES ${SOCKET_LIBRARIES} nsl)
endif (HAVE_LIBNSL)

# Functions
check_function_exists(readdir_r HAVE_READDIR_R)
check_function_exists(backtrace HAVE_BACKTRACE)
check_function_exists(prctl HAVE_PRCTL)

if(CMAKE_SYSTEM MATCHES "SunOS.*")
  # Make readdir_r on Solaris behave normally
  add_definitions(-D_POSIX_PTHREAD_SEMANTICS)
endif(CMAKE_SYSTEM MATCHES "SunOS.*")

# iconv
find_package(Iconv REQUIRED)
if (ICONV_SECOND_ARGUMENT_IS_CONST)
  set(ICONV_CONST "const")
endif (ICONV_SECOND_ARGUMENT_IS_CONST)

# gettext
if (ENABLE_NLS)
  set(GETTEXT_FOUND FALSE)

  find_path(GETTEXT_INCLUDE_DIR libintl.h)
  if (GETTEXT_INCLUDE_DIR)
    set(CMAKE_REQUIRED_LIBRARIES ${ICONV_LIBRARIES})
    check_function_exists(dgettext LIBC_HAS_DGETTEXT)
    if (LIBC_HAS_DGETTEXT)
      set(GETTEXT_FOUND TRUE)
    else (LIBC_HAS_DGETTEXT)
      find_library(LIBINTL_LIBRARY NAMES intl libintl)
      check_library_exists(${LIBINTL_LIBRARY} "dgettext" "" LIBINTL_HAS_DGETTEXT)
      if (LIBINTL_HAS_DGETTEXT)
        set(GETTEXT_LIBRARIES ${LIBINTL_LIBRARY})
        set(GETTEXT_FOUND TRUE)
      endif (LIBINTL_HAS_DGETTEXT)
    endif (LIBC_HAS_DGETTEXT)
    set(CMAKE_REQUIRED_LIBRARIES)
  endif (GETTEXT_INCLUDE_DIR)

  if (NOT GETTEXT_FOUND)
    message(FATAL_ERROR "Gettext NOT found")
  endif (NOT GETTEXT_FOUND)
endif (ENABLE_NLS)

# OpenSSL
if (USE_OPENSSL)
  check_library_exists(crypto DH_free "" have_openssl_crypto)
  if (have_openssl_crypto)
    find_package(OpenSSL)
  endif (have_openssl_crypto)

  if (OPENSSL_FOUND)
    list(APPEND OPENSSL_LIBRARIES crypto)
  else (OPENSSL_FOUND)
    message(STATUS "OpenSSL NOT found; secure communication channels will not be available")
    set(USE_OPENSSL OFF)
  endif (OPENSSL_FOUND)
endif (USE_OPENSSL)

# GPGME
find_package(Gpgme)
if (GPGME_VANILLA_FOUND)
  set(HAVE_LIBGPGME 1)
  check_file_offset_bits()
  if (_FILE_OFFSET_BITS)
      add_definitions(-D_FILE_OFFSET_BITS=${_FILE_OFFSET_BITS})
  endif (_FILE_OFFSET_BITS)
endif (GPGME_VANILLA_FOUND)

# SOCKS5
if (USE_SOCKS5)
  find_path(SOCKS5_INCLUDE_DIR socks.h)
  find_library(SOCKS5_LIBRARY NAMES socks5 socks)
  if (NOT SOCKS5_INCLUDE_DIR OR NOT SOCKS5_LIBRARY)
    message(FATAL_ERROR "SOCKS5 NOT found")
  endif (NOT SOCKS5_INCLUDE_DIR OR NOT SOCKS5_LIBRARY)
  set(SOCKS5_LIBRARIES ${SOCKS5_LIBRARY})
endif (USE_SOCKS5)

if (BUILD_TESTS)
  enable_testing()

  # Google Test
  add_subdirectory(3rdparty/gtest)
  set(GTEST_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/gtest/include")
  set(GTEST_LIBRARIES gtest)

  # Google Mock
  add_subdirectory(3rdparty/gmock)
  set(GMOCK_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/gmock/include")
  set(GMOCK_LIBRARIES gmock gmock_main)
else (BUILD_TESTS)
  message(STATUS "Unit tests disabled")
endif (BUILD_TESTS)

# Set up installation paths
include(cmake/LicqDefaultPaths.cmake)

# Generate config.h
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/include/config.h
)

# Generate licqversion.h
include(cmake/LicqVersion.cmake)

# Doxygen is option
find_package(Doxygen)

if (DOXYGEN_FOUND)
  if (NOT DOXYGEN_DOT_FOUND)
    message(SEND_ERROR "Need the dot program to build documentation")
  endif (NOT DOXYGEN_DOT_FOUND)

  set(DOXY_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

  message(STATUS "Generating ${DOXY_CONFIG}")
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in"
    ${DOXY_CONFIG} @ONLY)

  add_custom_target(docs
    ${DOXYGEN_EXECUTABLE} ${DOXY_CONFIG}
    DEPENDS ${DOXY_CONFIG}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating doxygen documentation" VERBATIM)
endif (DOXYGEN_FOUND)

# make uninstall (file added to top binary dir to work with build_all)
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
  "${CMAKE_BINARY_DIR}/cmake_uninstall.cmake"
  @ONLY)

add_custom_target(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_BINARY_DIR}/cmake_uninstall.cmake"
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(cmake)
add_subdirectory(include)
add_subdirectory(share)
add_subdirectory(src)

if (ENABLE_NLS)
  add_subdirectory(po)
endif (ENABLE_NLS)

if (BUILD_PLUGINS)
  add_subdirectory(plugins)
endif (BUILD_PLUGINS)
