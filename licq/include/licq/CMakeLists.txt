set(licq_HEADERS
  ${CMAKE_CURRENT_BINARY_DIR}/licqversion.h

  byteorder.h
  daemon.h
  gpghelper.h
  log.h
  logservice.h
  logsink.h
  md5.h
  plugin.h
  pluginmanager.h
  types.h
  version.h
)

install(FILES ${licq_HEADERS} DESTINATION "${Licq_INCLUDE_DIR}/licq")

add_subdirectory(contactlist)
add_subdirectory(exceptions)
add_subdirectory(thread)
