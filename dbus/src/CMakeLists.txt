set(dbus_SRCS
  dbusinterface.cpp
  factory.cpp
  plugin.cpp
)

licq_add_plugin(licq_dbus ${dbus_SRCS})
target_link_libraries(licq_dbus ${dbus_LIBRARIES})
