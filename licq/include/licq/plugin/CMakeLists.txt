set(licq_HEADERS
  generalbase.h
  generalplugin.h
  generalplugininterface.h
  plugin.h
  plugininterface.h
  pluginmanager.h
  protocolbase.h
  protocolplugin.h
  protocolplugininterface.h
)

install(FILES ${licq_HEADERS} DESTINATION "${Licq_INCLUDE_DIR}/licq/plugin")
