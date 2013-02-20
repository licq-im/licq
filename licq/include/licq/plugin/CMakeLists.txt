set(licq_HEADERS
  generalplugin.h
  generalpluginfactory.h
  generalpluginhelper.h
  generalplugininterface.h
  plugin.h
  pluginfactory.h
  plugininterface.h
  pluginmanager.h
  protocolplugin.h
  protocolpluginfactory.h
  protocolpluginhelper.h
  protocolplugininterface.h
)

install(FILES ${licq_HEADERS} DESTINATION "${Licq_INCLUDE_DIR}/licq/plugin")
