set(licq_HEADERS
  ${CMAKE_CURRENT_BINARY_DIR}/licqversion.h

  buffer.h
  byteorder.h
  conversation.h
  daemon.h
  gpghelper.h
  icq.h
  icqchat.h
  icqcodes.h
  icqdefines.h
  icqfiletransfer.h
  inifile.h
  log.h
  logservice.h
  logsink.h
  macro.h
  md5.h
  oneventmanager.h
  packet.h
  plugin.h
  pluginlogsink.h
  pluginmanager.h
  protocolmanager.h
  proxy.h
  sarmanager.h
  statistics.h
  userid.h
  utility.h
  version.h
)

install(FILES ${licq_HEADERS} DESTINATION "${Licq_INCLUDE_DIR}/licq")

add_subdirectory(contactlist)
add_subdirectory(exceptions)
add_subdirectory(thread)
