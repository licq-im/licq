set(licq_HEADERS
  generalbase.h
  generalplugin.h
  plugin.h
  pluginmanager.h
  protocolbase.h
  protocolplugin.h
)

install(FILES ${licq_HEADERS} DESTINATION "${Licq_INCLUDE_DIR}/licq/plugin")
