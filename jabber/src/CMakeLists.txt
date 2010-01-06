set(jabber_SRCS
  client.cpp
  jabber.cpp
  plugin.cpp
)

licq_add_plugin(protocol_jabber ${jabber_SRCS})
target_link_libraries(protocol_jabber ${gloox_LIBRARIES})
