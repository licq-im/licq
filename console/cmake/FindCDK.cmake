# Written August 2011 by Martin von Gagern for Licq on Gentoo Linux.

find_program(_CDKCONFIG_EXECUTABLE NAMES cdk5-config)

message(STATUS "Searching for Curses Development Kit (CDK)")
if(_CDKCONFIG_EXECUTABLE)

  message(STATUS " Found cdk5-config: ${_CDKCONFIG_EXECUTABLE}")
  exec_program(${_CDKCONFIG_EXECUTABLE} ARGS --version OUTPUT_VARIABLE CDK_VERSION)
  exec_program(${_CDKCONFIG_EXECUTABLE} ARGS --abi-version OUTPUT_VARIABLE CDK_ABIVERSION)
  message(STATUS " CDK version: ${CDK_VERSION}")

  exec_program(${_CDKCONFIG_EXECUTABLE} ARGS --cflags OUTPUT_VARIABLE CDK_CFLAGS)
  set(CDK_INCLUDE_DIR)
  string(REGEX REPLACE "[ \r\n]+" ";" _flags ${CDK_CFLAGS})
  foreach(_flag ${_flags})
    if( "${_flag}" MATCHES "^-I" )
      string(SUBSTRING "${_flag}" 2 -1 _dir)
      file(TO_CMAKE_PATH "${_dir}" _dir)
      set(CDK_INCLUDE_DIR ${CDK_INCLUDE_DIR} ${_dir})
    endif()
  endforeach()

  exec_program( ${_CDKCONFIG_EXECUTABLE} ARGS --libs OUTPUT_VARIABLE CDK_LIBS )
  set(CDK_LIBRARY)
  string(REGEX REPLACE "[ \r\n]+" ";" _flags ${CDK_LIBS})
  foreach(_flag ${_flags})
    if( "${_flag}" MATCHES "^-l" )
      string(SUBSTRING "${_flag}" 2 -1 _name)
      set(CDK_LIBRARY ${CDK_LIBRARY} ${_name})
    endif()
  endforeach()

else()

  message(STATUS " No cdk5-config found")
  find_path(CDK_INCLUDE_DIR cdk.h PATH_SUFFIXES cdk)
  if(CDK_INCLUDE_DIR)
    find_library(CDK_LIBRARY NAMES cdkw cdk)
  endif(CDK_INCLUDE_DIR)

endif()

if(CDK_LIBRARY)
  message(STATUS " CDK library: ${CDK_LIBRARY}")
  message(STATUS " CDK include directory: ${CDK_INCLUDE_DIR}")
else()
  if(CDK_FIND_REQUIRED)
    message(FATAL_ERROR "Could NOT find Curses Development Kit (CDK)")
  else()
    message(STATUS "Could NOT find Curses Development Kit (CDK)")
  endif()
endif()
