set(forwarder_SRCS
  default_config.cpp
  forwarder.cpp
  main.cpp
)

# Read default configuration, escape quotes and generate a cpp file
# TODO: Make it rebuild when licq_forwarder.conf changes
file(READ ${PROJECT_SOURCE_DIR}/licq_forwarder.conf config_raw)
string(REPLACE "\"" "\\\"" config_escaped ${config_raw})
string(REGEX REPLACE "\n" "\\\\n\"\n\"" config_quoted ${config_escaped})
file(WRITE ${PROJECT_BINARY_DIR}/src/default_config.cpp
    "extern const char* const FORWARDER_CONF =\n\"${config_quoted}\";\n")

licq_add_plugin(licq_forwarder ${forwarder_SRCS})
