set(msn_SRCS
  msn-sb.cpp
  msn-srv.cpp
  msn-ssl.cpp
  msn.cpp
  msnbuffer.cpp
  msnevent.cpp
  msnpacket.cpp
  owner.cpp
  plugin.cpp
  user.cpp
)

licq_add_plugin(protocol_msn ${msn_SRCS})
