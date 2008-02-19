# Helper classes
set(helpers_SRCS
  eventdesc.cpp
  licqstrings.cpp
  support.cpp
  usercodec.cpp
)

set(helpers ${helpers_SRCS})
add_library(helpers STATIC ${helpers})
