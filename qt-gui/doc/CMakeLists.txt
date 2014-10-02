find_package(Doxygen)

if (DOXYGEN_EXECUTABLE)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/qt-gui.doxygen ${CMAKE_CURRENT_BINARY_DIR}/qt-gui.doxygen)

  add_custom_target(doxy
    command ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/qt-gui.doxygen
    working_directory ${CMAKE_SOURCE_DIR})

endif (DOXYGEN_EXECUTABLE)
