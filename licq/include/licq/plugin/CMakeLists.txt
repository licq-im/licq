set(licq_HEADERS
  generalplugin.h
  generalpluginhelper.h
  generalplugininterface.h
  plugin.h
  plugininterface.h
  pluginmanager.h
  protocolplugin.h
  protocolpluginhelper.h
  protocolplugininterface.h
)

install(FILES ${licq_HEADERS} DESTINATION "${Licq_INCLUDE_DIR}/licq/plugin")
