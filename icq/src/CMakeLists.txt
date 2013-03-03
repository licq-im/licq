set(icq_SRCS
  buffer.cpp
  chat.cpp
  codes.cpp
  factory.cpp
  filetransfer.cpp
  icq-srv.cpp
  icq-tcp.cpp
  icq.cpp
  icqprotocolplugin.cpp
  oscarservice.cpp
  owner.cpp
  packet-srv.cpp
  packet-tcp.cpp
  protocolsignal.cpp
  rtf.cc
  socket.cpp
  threads.cpp
  user.cpp
  userclients.cpp
)

licq_add_plugin(protocol_icq ${icq_SRCS})
