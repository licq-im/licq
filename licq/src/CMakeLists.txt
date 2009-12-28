set(licq_SRCS
  backgroundcodes.c
  buffer.cpp
  countrycodes.c
  fifo.cpp
  file.cpp
  gpg.cpp
  hebrev.c
  history.cpp
  homepagecodes.c
  icqcolor.cpp
  icqd-chat.cpp
  icqd.cpp
  icqd-filetransfer.cpp
  icqd-srv.cpp
  icqd-tcp.cpp
  icqd-threads.cpp
  icqevent.cpp
  icqpacket.cpp
  interestcodes.c
  languagecodes.c
  licq.cpp
  log.cpp
  main.cpp
  md5.cpp
  message.cpp
  mutex.cpp
  occupationcodes.c
  onevent.cpp
  organizationcodes.c
  oscarservice.cpp
  plugind.cpp
  protoplugind.cpp
  providers.c
  proxy.cpp
  rtf.cc
  sar.cpp
  sighandler.c
  socket.cpp
  support.c
  translate.cpp
  user.cpp
  utility.cpp
)

add_executable(licq ${licq_SRCS})

# Add linker and compiler flags for thread support
set_target_properties(licq PROPERTIES LINK_FLAGS "${CMAKE_THREAD_LIBS_INIT}")
add_definitions(-D_REENTRANT)

# To find public headers
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/include)

# To find config.h and licqversion.h
include_directories(BEFORE ${PROJECT_BINARY_DIR}/include)

include_directories(${GETTEXT_INCLUDE_DIR})
include_directories(${GPGME_INCLUDES})
include_directories(${ICONV_INCLUDE_DIR})
include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(${SOCKS5_INCLUDE_DIR})

target_link_libraries(licq ${DLOPEN_LIBRARIES})
target_link_libraries(licq ${GETTEXT_LIBRARIES})
target_link_libraries(licq ${GPGME_VANILLA_LIBRARIES})
target_link_libraries(licq ${ICONV_LIBRARIES})
target_link_libraries(licq ${OPENSSL_LIBRARIES})
target_link_libraries(licq ${SOCKS5_LIBRARIES})

install(TARGETS licq RUNTIME DESTINATION bin)
