set(jabber_SRCS
  client.cpp
  config.cpp
  handler.cpp
  jabberplugin.cpp
  plugin.cpp
  sessionmanager.cpp
  vcard.cpp
)

licq_add_plugin(protocol_jabber ${jabber_SRCS})
target_link_libraries(protocol_jabber ${gloox_LIBRARIES})
