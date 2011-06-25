set(licq_HEADERS
  generalplugin.h
  plugin.h
  pluginbase.h
  pluginmanager.h
  protocolbase.h
  protocolplugin.h
)

install(FILES ${licq_HEADERS} DESTINATION "${Licq_INCLUDE_DIR}/licq/plugin")
