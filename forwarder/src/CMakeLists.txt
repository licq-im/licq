set(forwarder_SRCS
  default_config.cpp
  forwarder.cpp
  main.cpp
)

# Read default configuration, escape quotes and generate a cpp file
add_custom_command(
  OUTPUT ${PROJECT_BINARY_DIR}/src/default_config.cpp
  COMMAND ${CMAKE_COMMAND}
    -DINPUT="${PROJECT_SOURCE_DIR}/licq_forwarder.conf"
    -DOUTPUT="${PROJECT_BINARY_DIR}/src/default_config.cpp"
    -P "${PROJECT_SOURCE_DIR}/create_config.cmake"
  DEPENDS ${PROJECT_SOURCE_DIR}/licq_forwarder.conf
)

licq_add_plugin(licq_forwarder ${forwarder_SRCS})
