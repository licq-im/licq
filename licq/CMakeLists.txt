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

set(DLOPEN_POLICY RTLD_NOW CACHE STRING "2nd dlopen parameter")
mark_as_advanced(DLOPEN_POLICY)

include(cmake/LicqCommonCompilerFlags.cmake)

# Functions
check_function_exists(readdir_r HAVE_READDIR_R)

# dlopen
find_package(Dlopen REQUIRED QUIET)

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

# Google Test is optional
find_package(GTest QUIET)
if (GTEST_FOUND)
  enable_testing()
  add_custom_target(tests ALL COMMAND ${CMAKE_CTEST_COMMAND} $(ARGS))
else (GTEST_FOUND)
  message(STATUS "Google test NOT found; unit tests will not be built")
endif (GTEST_FOUND)

# Set up installation paths
include(cmake/LicqDefaultPaths.cmake)

# Generate config.h
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/include/config.h
)

# Generate licqversion.h
include(cmake/LicqVersion.cmake)

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
add_subdirectory(po)
add_subdirectory(share)
add_subdirectory(src)

if (BUILD_PLUGINS)
  add_subdirectory(plugins)
endif (BUILD_PLUGINS)
