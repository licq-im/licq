set(licq_HEADERS
  log.h
  logservice.h
  logsink.h
  logutils.h
  pluginlogsink.h
)

install(FILES ${licq_HEADERS} DESTINATION "${Licq_INCLUDE_DIR}/licq/logging")
