set(licq_HEADERS
  ${CMAKE_CURRENT_BINARY_DIR}/licqversion.h

  buffer.h
  byteorder.h
  conversation.h
  daemon.h
  gpghelper.h
  icqcodes.h
  inifile.h
  log.h
  logservice.h
  logsink.h
  macro.h
  md5.h
  oneventmanager.h
  plugin.h
  pluginlogsink.h
  pluginmanager.h
  protocolmanager.h
  sarmanager.h
  userid.h
  version.h
)

install(FILES ${licq_HEADERS} DESTINATION "${Licq_INCLUDE_DIR}/licq")

add_subdirectory(contactlist)
add_subdirectory(exceptions)
add_subdirectory(thread)
