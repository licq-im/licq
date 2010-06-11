set(licq_HEADERS
  licq_color.h
  licq_events.h
  licq_log.h
  licq_message.h
  licq_plugin.h
  licq_protoplugin.h
)

install(FILES ${licq_HEADERS} DESTINATION "${Licq_INCLUDE_DIR}/licq")

add_subdirectory(licq)
