set(licq_HEADERS
  ${CMAKE_CURRENT_BINARY_DIR}/licqversion.h

  buffer.h
  byteorder.h
  color.h
  conversation.h
  daemon.h
  event.h
  gpghelper.h
  icq.h
  icqchat.h
  icqcodes.h
  icqdefines.h
  icqfiletransfer.h
  inifile.h
  macro.h
  md5.h
  oneventmanager.h
  packet.h
  pipe.h
  plugin.h
  pluginbase.h
  pluginmanager.h
  pluginsignal.h
  protocolbase.h
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
add_subdirectory(logging)
add_subdirectory(thread)
