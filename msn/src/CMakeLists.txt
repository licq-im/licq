set(msn_SRCS
  factory.cpp
  msn-sb.cpp
  msn-srv.cpp
  msn-ssl.cpp
  msn.cpp
  msnbuffer.cpp
  msnevent.cpp
  msnpacket.cpp
  owner.cpp
  user.cpp
)

licq_add_plugin(protocol_msn ${msn_SRCS})
