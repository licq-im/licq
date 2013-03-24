set(jabber_SRCS
  client.cpp
  factory.cpp
  handler.cpp
  owner.cpp
  plugin.cpp
  sessionmanager.cpp
  user.cpp
  vcard.cpp
)

licq_add_plugin(protocol_jabber ${jabber_SRCS})
target_link_libraries(protocol_jabber ${gloox_LIBRARIES})
