# - Try to find Licq
#
# Variables defined:
#   Licq_FOUND        - TRUE if Licq was found
#   Licq_INCLUDE_DIRS - Licq's include directories, not cached
#
#   Licq_LOCALE_DIR   - Directory where Licq's locales are installed
#   Licq_PLUGIN_DIR   - Directory where plugins should be installed
#   Licq_SHARE_DIR    - Directory where sounds, icons etc. are installed
#
#   Licq_CMAKE_DIR    - Directory where Licq's cmake files can be found
#
# Macros defined:
#   LICQ_ADD_PLUGIN(plugin_name sources...)
#

# Directory this file is in
get_filename_component(Licq_CMAKE_DIR ${CMAKE_CURRENT_LIST_FILE} PATH)

# Check if Licq is installed
include(${Licq_CMAKE_DIR}/LicqPaths.cmake OPTIONAL RESULT_VARIABLE _licq_installed)

if (_licq_installed)
  if (NOT Licq_FIND_QUIETLY)
    message(STATUS "Found Licq headers: ${Licq_INCLUDE_DIR}")
  endif (NOT Licq_FIND_QUIETLY)

  set(Licq_FOUND TRUE)
  set(Licq_INCLUDE_DIRS
      "${Licq_INCLUDE_DIR}"
      "${Licq_INCLUDE_DIR}/licq"
  )

else (_licq_installed)
  # Licq not installed, check if this is the source
  set(LICQ_VERSION_SOURCE_DIR "${Licq_CMAKE_DIR}/..")
  include(${Licq_CMAKE_DIR}/LicqVersion.cmake OPTIONAL RESULT_VARIABLE _licq_version)

  if (_licq_version)
    include(${Licq_CMAKE_DIR}/LicqDefaultPaths.cmake)
    get_filename_component(_licq_src_dir "${Licq_CMAKE_DIR}/.." ABSOLUTE)

    if (NOT Licq_FIND_QUIETLY)
      message(STATUS "Found Licq source: ${_licq_src_dir}")
    endif (NOT Licq_FIND_QUIETLY)

    set(Licq_FOUND TRUE)
    set(Licq_INCLUDE_DIRS
        "${_licq_src_dir}/include"
        "${CMAKE_CURRENT_BINARY_DIR}/include")
  endif (_licq_version)
endif (_licq_installed)

if (NOT Licq_FOUND)
  if (Licq_FIND_REQUIRED)
    message(FATAL_ERROR "Licq NOT FOUND")
  endif (Licq_FIND_REQUIRED)
endif (NOT Licq_FOUND)

# Add common (for daemon and plugin) compiler flags
include(${Licq_CMAKE_DIR}/LicqCommonCompilerFlags.cmake)

# make uninstall
if (NOT licq_target_prefix)
  configure_file(
    "${Licq_CMAKE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_BINARY_DIR}/cmake_uninstall.cmake"
    @ONLY)

  add_custom_target(uninstall
    "${CMAKE_COMMAND}" -P "${CMAKE_BINARY_DIR}/cmake_uninstall.cmake"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endif (NOT licq_target_prefix)

# These symbols must be exported from the plugin
set(_licq_plugin_symbols
  LP_Name LP_Version LP_Status LP_Description
  LP_BuildDate LP_BuildTime LP_Usage LP_Id
  LP_Init LP_Main_tep LP_ConfigFile
  LProto_Name LProto_Version LProto_PPID
  LProto_Init LProto_Main_tep LProto_SendFuncs)

macro (LICQ_ADD_PLUGIN _licq_plugin_name)
  add_library(${_licq_plugin_name} MODULE ${ARGN})
  set_target_properties(${_licq_plugin_name} PROPERTIES PREFIX "")

  # Add linker flags for thread support
  set(_link_flags "${CMAKE_THREAD_LIBS_INIT}")

  if (APPLE)
    set(_link_flags "${_link_flags} -flat_namespace -undefined suppress")

    # Write the list of symbols that should be exported from the plugin to a
    # file and tell the linker about it. One symbol per line with a '_' prefix.
    string(REPLACE ";" "\n_" _symbols "${_licq_plugin_symbols}")
    set(_symbols_list "${CMAKE_CURRENT_BINARY_DIR}/symbols.list")
    file(WRITE ${_symbols_list} "_${_symbols}\n")
    set(_link_flags
      "${_link_flags} -Wl,-exported_symbols_list,'${_symbols_list}'")

  elseif (CMAKE_COMPILER_IS_GNUCXX)
    # Create a version script exporting the symbols that should be exported
    # from the plugin and tell the linker about it.
    set(_symbols "{ global: ${_licq_plugin_symbols}; local: *; };")
    set(_version_script "${CMAKE_CURRENT_BINARY_DIR}/version.script")
    file(WRITE ${_version_script} "${_symbols}\n")

    # Check if the linker supports version script (i.e. is GNU ld)
    check_cxx_accepts_flag("-Wl,--version-script,${_version_script}"
      LD_ACCEPTS_VERSION_SCRIPT)
    if (LD_ACCEPTS_VERSION_SCRIPT)
      set(_link_flags
	"${_link_flags} -Wl,--version-script,'${_version_script}'")
    endif (LD_ACCEPTS_VERSION_SCRIPT)
  endif (APPLE)

  set_target_properties(${_licq_plugin_name} PROPERTIES
    LINK_FLAGS ${_link_flags})

  install(TARGETS ${_licq_plugin_name} DESTINATION ${Licq_PLUGIN_DIR})
endmacro (LICQ_ADD_PLUGIN)

macro (_licq_plugin_version_helper type)
  if (DEFINED PLUGIN_VERSION_${type})
    set(_PLUGIN_VERSION_${type} ${ARGV1}${PLUGIN_VERSION_${type}}${ARGV2})
  else (DEFINED PLUGIN_VERSION_${type})
    set(_PLUGIN_VERSION_${type} LICQ_VERSION_${type})
  endif (DEFINED PLUGIN_VERSION_${type})
endmacro (_licq_plugin_version_helper)

macro (LICQ_CREATE_PLUGIN_VERSION_FILE dir)
  # Use PLUGIN_VERSION_x if set, otherwise use LICQ_VERSION_x
  _licq_plugin_version_helper(MAJOR)
  _licq_plugin_version_helper(MINOR)
  _licq_plugin_version_helper(RELEASE)
  _licq_plugin_version_helper(EXTRA \" \")

  # When building from a svn working copy, set the extra version to the current
  # revision, replacing any existing value.
  find_package(Subversion QUIET)
  if (Subversion_FOUND)
    # The subversion_wc_info macro prints an error if the dir isn't a working
    # copy. To avoid this, check if it is one before executing the macro.
    execute_process(
      COMMAND ${Subversion_SVN_EXECUTABLE} info ${PROJECT_SOURCE_DIR}
      RESULT_VARIABLE _licq_plugin_svn_result
      OUTPUT_QUIET ERROR_QUIET)

    if (${_licq_plugin_svn_result} EQUAL 0)
      subversion_wc_info(${PROJECT_SOURCE_DIR} Plugin)
      set(_PLUGIN_VERSION_EXTRA "\"-r${Plugin_WC_LAST_CHANGED_REV}\"")
    endif (${_licq_plugin_svn_result} EQUAL 0)
  endif (Subversion_FOUND)

  # pluginversion.h content
  set(_plugin_version_file "${dir}/pluginversion.h")
  set(_plugin_old_version "")
  set(_plugin_new_version 
    "// Autogenerated by CMake. Do not edit, changes will be lost.
#ifndef LICQ_PLUGINVERSION_H
#define LICQ_PLUGINVERSION_H

#include <licq/licqversion.h>

#define _LICQ_PLUGIN_STR_(x) #x
#define _LICQ_PLUGIN_STR(x) _LICQ_PLUGIN_STR_(x)

#define PLUGIN_VERSION_MAJOR ${_PLUGIN_VERSION_MAJOR}
#define PLUGIN_VERSION_MINOR ${_PLUGIN_VERSION_MINOR}
#define PLUGIN_VERSION_RELEASE ${_PLUGIN_VERSION_RELEASE}
#define PLUGIN_VERSION_EXTRA ${_PLUGIN_VERSION_EXTRA}
#define PLUGIN_VERSION_STRING \\
  _LICQ_PLUGIN_STR(${_PLUGIN_VERSION_MAJOR}) \".\" \\
  _LICQ_PLUGIN_STR(${_PLUGIN_VERSION_MINOR}) \".\" \\
  _LICQ_PLUGIN_STR(${_PLUGIN_VERSION_RELEASE}) \\
  ${_PLUGIN_VERSION_EXTRA}

#endif
")

  # Read in the old file (if it exists)
  if (EXISTS ${_plugin_version_file})
    file(READ ${_plugin_version_file} _plugin_old_version)
  endif (EXISTS ${_plugin_version_file})

  # Only write the file if the content has changed
  string(COMPARE NOTEQUAL
    "${_plugin_old_version}" "${_plugin_new_version}"
    _plugin_update_version_file)

  if (${_plugin_update_version_file})
    message(STATUS "Creating ${_plugin_version_file}")
    file(WRITE ${_plugin_version_file} ${_plugin_new_version})
  endif (${_plugin_update_version_file})
endmacro (LICQ_CREATE_PLUGIN_VERSION_FILE)
