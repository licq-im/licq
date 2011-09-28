set(tested_SRCS
  conversation.cpp
  inifile.cpp

  logging/adjustablelogsink.cpp
  logging/log.cpp
  logging/logdistributor.cpp
  logging/logutils.cpp
  logging/pluginlogsink.cpp

  plugin/generalplugin.cpp
  plugin/plugin.cpp
  plugin/pluginthread.cpp
  plugin/protocolplugin.cpp

  thread/condition.cpp
  thread/mutexlocker.cpp
  thread/readwritemutex.cpp

  utils/dynamiclibrary.cpp
  utils/pipe.cpp
)

set(licq_SRCS
  buffer.cpp
  color.cpp
  daemon.cpp
  event.cpp
  fifo.cpp
  filter.cpp
  gpghelper.cpp
  hebrev.c
  licq.cpp
  main.cpp
  md5.cpp
  oneventmanager.cpp
  packet.cpp
  protocolmanager.cpp
  proxy.cpp
  sarmanager.cpp
  sighandler.cpp
  socket.cpp
  socketmanager.cpp
  statistics.cpp
  translator.cpp
  userevents.cpp
  utility.cpp

  contactlist/group.cpp
  contactlist/owner.cpp
  contactlist/user.cpp
  contactlist/userhistory.cpp
  contactlist/usermanager.cpp

  icq/chat.cpp
  icq/codes.c
  icq/filetransfer.cpp
  icq/icq-srv.cpp
  icq/icq-tcp.cpp
  icq/icq.cpp
  icq/icqprotocolplugin.cpp
  icq/oscarservice.cpp
  icq/packet.cpp
  icq/rtf.cc
  icq/threads.cpp
  icq/userclients.cpp

  logging/logservice.cpp
  logging/streamlogsink.cpp
  logging/threadlog.cpp

  plugin/pluginmanager.cpp

  ${tested_SRCS}
)

set(test_SRCS
  tests/conversationtest.cpp
  tests/inifiletest.cpp

  logging/tests/adjustablelogsinktest.cpp
  logging/tests/logdistributortest.cpp
  logging/tests/logtest.cpp
  logging/tests/logutilstest.cpp
  logging/tests/pluginlogsinktest.cpp

  plugin/tests/generalplugintest.cpp
  plugin/tests/plugintest.cpp
  plugin/tests/pluginthreadtest.cpp
  plugin/tests/protocolplugintest.cpp

  thread/tests/conditiontest.cpp
  thread/tests/mutextest.cpp
  thread/tests/mutexlockertest.cpp
  thread/tests/readwritemutextest.cpp
  thread/tests/threadspecificdatatest.cpp

  utils/tests/dynamiclibrarytest.cpp
  utils/tests/pipetest.cpp

  # Dummy global instances to make tests compile
  tests/daemon_dummy.cpp
  tests/log_dummy.cpp

  ${tested_SRCS}
)

add_executable(licq ${licq_SRCS})

# Link with thread library
target_link_libraries(licq ${CMAKE_THREAD_LIBS_INIT})

# Must export symbols from daemon so that plugins can use them
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_EXE_EXPORTS_CXX_FLAG}")

# Print a message to stderr (and to the file <basedir>/licq.debug_rw_mutex) and
# then abort if a (potential) deadlock is detected.
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DLICQDAEMON_DEBUG_RW_MUTEX=1")

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

target_link_libraries(licq ${CMAKE_DL_LIBS})
target_link_libraries(licq ${GETTEXT_LIBRARIES})
target_link_libraries(licq ${GPGME_VANILLA_LIBRARIES})
target_link_libraries(licq ${ICONV_LIBRARIES})
target_link_libraries(licq ${OPENSSL_LIBRARIES})
target_link_libraries(licq ${SOCKET_LIBRARIES})
target_link_libraries(licq ${SOCKS5_LIBRARIES})
target_link_libraries(licq ${Boost_LIBRARIES})

install(TARGETS licq RUNTIME DESTINATION bin)

if (BUILD_TESTS)
  include_directories(${GTEST_INCLUDE_DIRS})
  include_directories(${GMOCK_INCLUDE_DIRS})

  add_executable(unittest ${test_SRCS})

  target_link_libraries(unittest ${GTEST_LIBRARIES})
  target_link_libraries(unittest ${GMOCK_LIBRARIES})
  target_link_libraries(unittest ${CMAKE_DL_LIBS})
  target_link_libraries(unittest ${SOCKET_LIBRARIES})

  # Link with thread library
  target_link_libraries(unittest ${CMAKE_THREAD_LIBS_INIT})

  add_test(licq unittest)

  set(unittest_stamp
    "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/unittest.stamp")
  add_custom_command(OUTPUT "${unittest_stamp}"
    COMMAND ${CMAKE_CTEST_COMMAND} $(ARGS)
    COMMAND ${CMAKE_COMMAND} -E touch "${unittest_stamp}"
    COMMENT "Running unit test"
    DEPENDS unittest)
  add_custom_target(unittest_run ALL DEPENDS "${unittest_stamp}")
endif (BUILD_TESTS)
