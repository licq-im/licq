set(licq_HEADERS
  ${CMAKE_CURRENT_BINARY_DIR}/licqversion.h

  daemon.h
  log.h
  logservice.h
  logsink.h
  plugin.h
  pluginmanager.h
  version.h
)

install(FILES ${licq_HEADERS} DESTINATION "${Licq_INCLUDE_DIR}/licq")

add_subdirectory(contactlist)
add_subdirectory(exceptions)
add_subdirectory(thread)
