set(tested_SRCS
  logging/log.cpp
  logging/logdistributor.cpp
  logging/logsink.cpp
  logging/pluginlogsink.cpp

  plugins/generalplugin.cpp
  plugins/plugin.cpp
  plugins/pluginthread.cpp
  plugins/protocolplugin.cpp

  thread/condition.cpp
  thread/mutexlocker.cpp
  thread/readwritemutex.cpp

  utils/dynamiclibrary.cpp
  utils/pipe.cpp
)

set(licq_SRCS
  backgroundcodes.c
  buffer.cpp
  countrycodes.c
  fifo.cpp
  file.cpp
  gpghelper.cpp
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
  occupationcodes.c
  onevent.cpp
  organizationcodes.c
  oscarservice.cpp
  providers.c
  proxy.cpp
  rtf.cc
  sar.cpp
  sighandler.c
  socket.cpp
  support.c
  translate.cpp
  utility.cpp

  contactlist/group.cpp
  contactlist/owner.cpp
  contactlist/user.cpp
  contactlist/usermanager.cpp

  logging/logservice.cpp
  logging/streamlogsink.cpp
  logging/threadlog.cpp

  plugins/icqprotocolplugin.cpp
  plugins/plugineventhandler.cpp
  plugins/pluginmanager.cpp

  ${tested_SRCS}
)

set(test_SRCS
  logging/tests/logdistributortest.cpp
  logging/tests/logsinktest.cpp
  logging/tests/logtest.cpp
  logging/tests/pluginlogsinktest.cpp

  plugins/tests/generalplugintest.cpp
  plugins/tests/plugintest.cpp
  plugins/tests/pluginthreadtest.cpp
  plugins/tests/protocolplugintest.cpp

  thread/tests/conditiontest.cpp
  thread/tests/mutextest.cpp
  thread/tests/mutexlockertest.cpp
  thread/tests/readwritemutextest.cpp
  thread/tests/threadspecificdatatest.cpp

  utils/tests/dynamiclibrarytest.cpp
  utils/tests/pipetest.cpp

  ${tested_SRCS}
)

add_executable(licq ${licq_SRCS})

# Add linker flags for thread support
set_target_properties(licq PROPERTIES LINK_FLAGS "${CMAKE_THREAD_LIBS_INIT}")

# Must export symbols from daemon so that plugins can use them
list(APPEND CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_EXPORTS_CXX_FLAG})

# To find public headers
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/include)

# To find config.h and licqversion.h
include_directories(BEFORE ${PROJECT_BINARY_DIR}/include)

# To find local headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

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

if (GTEST_FOUND)
  include_directories(${GTEST_INCLUDE_DIRS})
  include_directories(${GMOCK_INCLUDE_DIRS})

  add_executable(unittests ${test_SRCS})

  target_link_libraries(unittests ${GTEST_LIBRARIES})
  target_link_libraries(unittests ${GMOCK_LIBRARIES})
  target_link_libraries(unittests ${DLOPEN_LIBRARIES})

  # Add linker flags for thread support
  set_target_properties(unittests PROPERTIES
    LINK_FLAGS "${CMAKE_THREAD_LIBS_INIT}")

  add_test(licq unittests)
  add_dependencies(tests unittests)
endif (GTEST_FOUND)
