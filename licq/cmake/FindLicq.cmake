# - Try to find Licq
#
# Variables defined:
#   Licq_FOUND        - TRUE if Licq was found
#   Licq_INCLUDE_DIRS - Licq include directories, not cached
#
#   Licq_LOCALE_DIR   - Directory where Licq's locales are installed
#   Licq_PLUGIN_DIR   - Directory where plugins should be installed
#   Licq_SHARE_DIR    - Directory where sounds, icons etc. are installed
#
# Macros defined:
#   LICQ_ADD_PLUGIN(plugin_name sources...)
#

# Directory this file is in
get_filename_component(_licq_cmake_dir ${CMAKE_CURRENT_LIST_FILE} PATH)

# Check if Licq is installed
include(${_licq_cmake_dir}/LicqPaths.cmake OPTIONAL RESULT_VARIABLE _licq_installed)

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
  include(${_licq_cmake_dir}/LicqVersion.cmake OPTIONAL RESULT_VARIABLE _licq_version)

  if (_licq_version)
    include(${_licq_cmake_dir}/LicqDefaultPaths.cmake)
    get_filename_component(_licq_src_dir "${_licq_cmake_dir}/.." ABSOLUTE)

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
include(${_licq_cmake_dir}/LicqCommonCompilerFlags.cmake)

# These symbols must be exported from the plugin
set(_licq_plugin_symbols
  LP_Name LP_Version LP_Status LP_Description
  LP_BuildDate LP_BuildTime LP_Usage LP_Id
  LP_Init LP_Main LP_Main_tep LP_ConfigFile
  LProto_Name LProto_Version LProto_PPID
  LProto_Init LProto_Main LProto_Main_tep LProto_SendFuncs)

macro (LICQ_ADD_PLUGIN _licq_plugin_name)
  add_library(${_licq_plugin_name} MODULE ${ARGN})
  set_target_properties(${_licq_plugin_name} PROPERTIES PREFIX "")

  if (APPLE)
    string(REPLACE ";" "\n_" _symbols "${_licq_plugin_symbols}")
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/symbols.list" "_${_symbols}\n")

    set_target_properties(${_licq_plugin_name} PROPERTIES LINK_FLAGS
      "-flat_namespace -undefined suppress -Wl,-exported_symbols_list -Wl,'${CMAKE_CURRENT_BINARY_DIR}/symbols.list'")
  elseif (CMAKE_COMPILER_IS_GNUCXX)
    set(_symbols "{ global: ${_licq_plugin_symbols}; local: *; };")
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/symbols.script" "${_symbols}\n")

    set_target_properties(${_licq_plugin_name} PROPERTIES LINK_FLAGS
      "-Wl,--version-script -Wl,'${CMAKE_CURRENT_BINARY_DIR}/symbols.script'")
  endif (APPLE)
  install(TARGETS ${_licq_plugin_name} DESTINATION ${Licq_PLUGIN_DIR})
endmacro (LICQ_ADD_PLUGIN)
