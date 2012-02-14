set(licq_HEADERS
  ${CMAKE_CURRENT_BINARY_DIR}/licqversion.h

  buffer.h
  byteorder.h
  color.h
  conversation.h
  daemon.h
  event.h
  filter.h
  gpghelper.h
  inifile.h
  macro.h
  md5.h
  oneventmanager.h
  packet.h
  pipe.h
  pluginsignal.h
  protocolmanager.h
  protocolsignal.h
  proxy.h
  sarmanager.h
  socket.h
  socketmanager.h
  statistics.h
  translator.h
  userevents.h
  userid.h
  utility.h
  version.h
)

install(FILES ${licq_HEADERS} DESTINATION "${Licq_INCLUDE_DIR}/licq")

add_subdirectory(contactlist)
add_subdirectory(exceptions)
add_subdirectory(icq)
add_subdirectory(logging)
add_subdirectory(plugin)
add_subdirectory(thread)
