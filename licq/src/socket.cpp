/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998-2011 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

/* Socket routine descriptions */

#include "config.h"

#include <licq/socket.h>

#include <arpa/inet.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#ifdef USE_OPENSSL
#include <openssl/ssl.h>
#include <openssl/err.h>

SSL_CTX *gSSL_CTX;
SSL_CTX *gSSL_CTX_NONICQ;
#endif // OpenSSL

#ifdef USE_SOCKS5

#define SOCKS
#define INCLUDE_PROTOTYPES
extern "C" {
#include <socks.h>
}
#endif // SOCKS5

#ifdef SOCKS5_OPTLEN
  #ifdef socklen_t
    #undef socklen_t
  #endif

  #define socklen_t SOCKS5_OPTLEN
#endif

#include <licq/proxy.h>
#include <licq/logging/log.h>

#include "gettext.h"


using namespace std;
using Licq::Buffer;
using Licq::INetSocket;
using Licq::SrvSocket;
using Licq::TCPSocket;
using Licq::UDPSocket;
using Licq::UserId;

char* Licq::ip_ntoa(unsigned long in, char *buf)
{
  inet_ntop(AF_INET, &in, buf, 32);
  return buf;
}


//=====Constants================================================================
const unsigned long MAX_RECV_SIZE = 4096;



//=====INetSocket===============================================================

string INetSocket::addrToString(const struct sockaddr* addr)
{
  switch (addr->sa_family)
  {
    case AF_INET:
    {
      char buf[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &((struct sockaddr_in*)addr)->sin_addr.s_addr, buf, sizeof(buf));
      return buf;
    }

    case AF_INET6:
    {
      char buf[INET6_ADDRSTRLEN];
      inet_ntop(AF_INET6, &((struct sockaddr_in6*)addr)->sin6_addr.s6_addr, buf, sizeof(buf));
      return buf;
    }

    default:
      return string();
  }
}

uint32_t INetSocket::addrToInt(const struct sockaddr* addr)
{
  if (addr->sa_family == AF_INET)
    return ((struct sockaddr_in*)addr)->sin_addr.s_addr;
  return 0;
}

uint32_t INetSocket::ipToInt(const std::string& ip)
{
  struct in_addr addr;
  if (::inet_aton(ip.c_str(), &addr))
    return addr.s_addr;
  return 0;
}

uint16_t INetSocket::getAddrPort(const struct sockaddr* addr)
{
  switch (addr->sa_family)
  {
    case AF_INET:
      return ntohs(((struct sockaddr_in*)addr)->sin_port);

    case AF_INET6:
      return ntohs(((struct sockaddr_in6*)addr)->sin6_port);

    default:
      return 0;
  }
}

//-----INetSocket::Error------------------------------------------------------
int INetSocket::Error()
{
  switch (myErrorType)
  {
    case ErrorErrno:
      return errno;
    case ErrorNone:
      return 0;
    case ErrorInternal:
      return -2;
    case ErrorProxy:
      if (myProxy != NULL)
        return myProxy->error();
  }
  return 0;
}

string INetSocket::errorStr() const
{
  switch (myErrorType)
  {
    case ErrorErrno:
      return strerror(errno);

    case ErrorNone:
      return tr("No error detected");

    case ErrorProxy:
      if (myProxy != NULL)
        return myProxy->errorStr();

    case ErrorInternal:
    default:
      return tr("Internal error");
  }
}


INetSocket::INetSocket(int sockType, const string& logId, const UserId& userId)
  : myDescriptor(-1),
    myLogId(logId),
    mySockType(sockType),
    myVersion(0),
    myErrorType(ErrorNone),
    myProxy(NULL),
    myUserId(userId)
{
  memset(&myRemoteAddr, 0, sizeof(myRemoteAddrStorage));
  memset(&myLocalAddr, 0, sizeof(myLocalAddrStorage));
}

INetSocket::~INetSocket()
{
  CloseConnection();
}

//-----INetSocket::dumpPacket---------------------------------------------------
void INetSocket::DumpPacket(Buffer *b, bool isReceiver)
{
  if (!isReceiver)
  {
    b->log(Log::Debug, "Packet (%sv%lu, %lu bytes) sent:\n(%s:%d -> %s:%d)",
           myLogId.c_str(), Version(), b->getDataSize(),
           getLocalIpString().c_str(), getLocalPort(),
           getRemoteIpString().c_str(), getRemotePort());
  }
  else
  {
    b->log(Log::Debug, "Packet (%sv%lu, %lu bytes) received:\n(%s:%d <- %s:%d)",
           myLogId.c_str(), Version(), b->getDataSize(),
           getLocalIpString().c_str(), getLocalPort(),
           getRemoteIpString().c_str(), getRemotePort());
  }
}


//-----INetSocket::ResetSocket-------------------------------------------------
void INetSocket::ResetSocket()
{
  CloseConnection();
  memset(&myRemoteAddr, 0, sizeof(myRemoteAddrStorage));
  memset(&myLocalAddr, 0, sizeof(myLocalAddrStorage));
}


/*-----INetSocket::SetLocalAddress------------------------------------------
 * Sets the sockaddr_in structures using data from the connected socket
 *---------------------------------------------------------------------------*/
bool INetSocket::SetLocalAddress(bool /* bIp */)
{
  // Setup the local structure
  socklen_t sizeofSockaddr = sizeof(myLocalAddrStorage);

  if (getsockname(myDescriptor, (struct sockaddr*)&myLocalAddr, &sizeofSockaddr) < 0)
  {
    myErrorType = ErrorNone;
    return (false);
  }

  return (true);
}

bool INetSocket::connectTo(const string& remoteName, uint16_t remotePort, Licq::Proxy* proxy)
{
  myRemoteName = remoteName;
  myProxy = proxy;

  // If we're using a proxy, let the proxy class handle this
  if (myProxy != NULL)
  {
    myDescriptor = myProxy->openConnection(remoteName, remotePort);
    if (myDescriptor == -1)
    {
      myErrorType = ErrorProxy;
      return(false);
    }

    memcpy(&myRemoteAddr, myProxy->proxyAddr(), sizeof(myRemoteAddrStorage));
    return SetLocalAddress();
  }

  // No proxy, let's do this ourselves

  // If already connected, close the old connection first
  if (myDescriptor != -1)
    CloseConnection();

  // If anything happens here, the error will be in errno
  myErrorType = ErrorErrno;

  myDescriptor = connectDirect(remoteName, remotePort, mySockType, &myRemoteAddr);

#ifdef USE_SOCKS5
    if (mySockType != SOCK_STREAM)
      return true;
#endif

  if (myDescriptor == -1)
    return false;

  return SetLocalAddress();
}

int INetSocket::connectDirect(const string& remoteName, uint16_t remotePort, uint16_t sockType, struct sockaddr* remoteAddr)
{
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
#ifdef LICQ_DISABLE_IPV6
  hints.ai_family = AF_INET;
#else
  hints.ai_family = AF_UNSPEC;
#endif
  hints.ai_socktype = sockType;
#ifdef AI_ADDRCONFIG
  // AI_ADDRCONFIG = Don't return IPvX address if host has no IPvX address configured
  hints.ai_flags = AI_ADDRCONFIG;
#endif

  struct addrinfo* addrs;
  int s = getaddrinfo(remoteName.c_str(), NULL, &hints, &addrs);
  if(s != 0)
  {
    gLog.warning(tr("Error when trying to resolve %s. getaddrinfo() returned %d."),
        remoteName.c_str(), s);
    return false;
  }

  int sock = -1;

  // getaddrinfo() returns a list of addresses, we'll try them one by one until
  //   we manage to make a connection. The list is already be sorted with
  //   preferred address first.
  struct addrinfo* ai;
  for (ai = addrs; ai != NULL; ai = ai->ai_next)
  {
    memcpy(remoteAddr, ai->ai_addr, ai->ai_addrlen);

    // We didn't use getaddrinfo to lookup port so set in manually
    if (remoteAddr->sa_family == AF_INET)
      ((struct sockaddr_in*)remoteAddr)->sin_port = htons(remotePort);
    else if (remoteAddr->sa_family == AF_INET6)
      ((struct sockaddr_in6*)remoteAddr)->sin6_port = htons(remotePort);

    gLog.info(tr("Connecting to %s:%i..."),
        addrToString(remoteAddr).c_str(), remotePort);

    // Create socket of the returned type
    sock = socket(remoteAddr->sa_family, sockType, 0);
    if (sock == -1)
      continue;

#ifdef IP_PORTRANGE
    int i=IP_PORTRANGE_HIGH;
    if (setsockopt(sock, IPPROTO_IP, IP_PORTRANGE, &i, sizeof(i))<0)
    {
      close(sock);
      sock = -1;
      gLog.warning(tr("Failed to set port range for socket."));
      continue;
    }
#endif

    // Try to connect, exit loop if successful
    if (connect(sock, (struct sockaddr*)remoteAddr, ai->ai_addrlen) != -1)
      break;

    // Failed to connect, close socket and try next
    close(sock);
    sock = -1;
  }

  // Return the memory allocated by getaddrinfo
  freeaddrinfo(addrs);

  // If we reached the end of the address list we didn't find anything that could connect
  if (ai == NULL)
    return -1;

  return sock;
}

bool INetSocket::connectTo(uint32_t remoteAddr, uint16_t remotePort, Licq::Proxy* proxy)
{
  char buf[INET_ADDRSTRLEN];
  return connectTo(string(inet_ntop(AF_INET, &remoteAddr, buf, sizeof(buf))), remotePort, proxy);
}

//-----INetSocket::StartServer--------------------------------------------------
bool INetSocket::StartServer(unsigned int _nPort)
{
  socklen_t addrlen;
  memset(&myLocalAddr, 0, sizeof(myLocalAddrStorage));

#ifndef LICQ_DISABLE_IPV6
  // Try to create an IPv6 socket
  myDescriptor = socket(AF_INET6, mySockType, 0);
  if (myDescriptor != -1)
  {
    // IPv6 socket created

    // Make sure we can accept connections for IPv4 as well
    int i = 0;
    if (setsockopt(myDescriptor, IPPROTO_IPV6, IPV6_V6ONLY, &i, sizeof(i)) < 0)
    {
      myErrorType = ErrorErrno;
      ::close(myDescriptor);
      myDescriptor = -1;
      return false;
    }

#ifdef IPV6_PORTRANGE
    i = IPV6_PORTRANGE_HIGH;
    if (setsockopt(myDescriptor, IPPROTO_IPV6, IPV6_PORTRANGE, &i, sizeof(i)) < 0)
    {
      myErrorType = ErrorErrno;
      ::close(myDescriptor);
      myDescriptor = -1;
      return false;
    }
#endif

    addrlen = sizeof(sockaddr_in6);
    myLocalAddr.sa_family = AF_INET6;
    ((struct sockaddr_in6*)&myLocalAddr)->sin6_port = htons(_nPort);
    ((struct sockaddr_in6*)&myLocalAddr)->sin6_addr = in6addr_any;
  }
  else
  {
    // Unable to create an IPv6 socket, try with IPv4 instead
    gLog.warning(tr("Failed to start local server using IPv6 socket "
                    "(falling back to IPv4):\n%s"), strerror(errno));

#endif
    myDescriptor = socket(AF_INET, mySockType, 0);
    if (myDescriptor == -1)
    {
      myErrorType = ErrorErrno;
      return (false);
    }

#ifdef IP_PORTRANGE
    int i = IP_PORTRANGE_HIGH;
    if (setsockopt(myDescriptor, IPPROTO_IP, IP_PORTRANGE, &i, sizeof(i)) < 0)
    {
      myErrorType = ErrorErrno;
      ::close(myDescriptor);
      myDescriptor = -1;
      return false;
    }
#endif

    addrlen = sizeof(sockaddr_in);
    myLocalAddr.sa_family = AF_INET;
    ((struct sockaddr_in*)&myLocalAddr)->sin_port = htons(_nPort);
    ((struct sockaddr_in*)&myLocalAddr)->sin_addr.s_addr = INADDR_ANY;
#ifndef LICQ_DISABLE_IPV6
  }
#endif

  if (bind(myDescriptor, (struct sockaddr*)&myLocalAddr, addrlen) == -1)
  {
    myErrorType = ErrorErrno;
    ::close(myDescriptor);
    myDescriptor = -1;
    return (false);
  }

  if (!SetLocalAddress(false)) return (false);

  if (mySockType == SOCK_STREAM)
  {
    // Allow 10 unprocessed connections
    if (listen(myDescriptor, 10) != 0)
    {
      myErrorType = ErrorErrno;
      ::close(myDescriptor);
      myDescriptor = -1;
      return false;
    }
  }

  return(true);
}


//-----INetSocket::CloseConnection-------------------------------------------
void INetSocket::CloseConnection()
{
  myRecvBuffer.Clear();
  if (myDescriptor != -1)
  {
    ::shutdown(myDescriptor, 2);
    ::close (myDescriptor);
    myDescriptor = -1;
  }
}

//-----INetSocket::SendRaw------------------------------------------------------
bool INetSocket::SendRaw(Buffer *b)
{
  // send the packet
  int nBytesSent;
  unsigned long nTotalBytesSent = 0;
  while (nTotalBytesSent < b->getDataSize())
  {
    nBytesSent = send(myDescriptor, b->getDataStart() + nTotalBytesSent,
                      b->getDataSize() - nTotalBytesSent, 0);
    if (nBytesSent < 0)
    {
      myErrorType = ErrorErrno;
      return(false);
    }
    nTotalBytesSent += nBytesSent;
  }

  // Print the packet
  DumpPacket(b, false);
  return (true);
}


/*-----INetSocket::RecvRaw---------------------------------------------------
 * Receive data on the socket.
 *---------------------------------------------------------------------------*/
bool INetSocket::RecvRaw()
{
  char *buffer = new char[MAX_RECV_SIZE];
  errno = 0;
  int nBytesReceived = recv(myDescriptor, buffer, MAX_RECV_SIZE, 0);
  if (nBytesReceived <= 0)
  {
    delete[] buffer;
    myErrorType = ErrorErrno;
    return (false);
  }
  myRecvBuffer.Create(nBytesReceived);
  myRecvBuffer.Pack(buffer, nBytesReceived);
  delete[] buffer;

  // Print the packet
  DumpPacket(&myRecvBuffer, true);

  return (true);
}

//=====SrvSocket===============================================================

SrvSocket::SrvSocket(const UserId& userId)
  : INetSocket(SOCK_STREAM, "SRV", userId)
{
  // Empty
}

SrvSocket::~SrvSocket()
{
  // Empty
}


/*-----SrvSocket::SendPacket---------------------------------------------------
 * Sends a packet on a socket.  The socket is blocking, so we are guaranteed
 * that the entire packet will be sent, however, it may block if the tcp
 * buffer is full.  This should not be a problem unless we are sending a huge
 * packet.
 *---------------------------------------------------------------------------*/

bool SrvSocket::SendPacket(Buffer *b_in)
{
  Buffer *b = b_in;

  unsigned long nTotalBytesSent = 0;
  int nBytesSent = 0;

  // send the packet
  nTotalBytesSent = 0;
  errno = 0;
  while (nTotalBytesSent < b->getDataSize())
  {
    nBytesSent = send(myDescriptor, b->getDataStart() + nTotalBytesSent,
                      b->getDataSize() - nTotalBytesSent, 0);
    if (nBytesSent <= 0)
    {
      if (nBytesSent < 0 && errno == EINTR) continue;
      myErrorType = ErrorErrno;
      if (b != b_in) delete b;
      return(false);
    }
    nTotalBytesSent += nBytesSent;
  }

  // Print the packet
  DumpPacket(b, false);

  if (b != b_in) delete b;
  return (true);
}

/*-----SrvSocket::ReceivePacket------------------------------------------------
 * Receive data on the socket.  Checks the buffer to see if it is empty, if
 * so, then it will create it using either the size read in from the socket
 * (the first two bytes available) or the given size.
 *---------------------------------------------------------------------------*/
bool SrvSocket::RecvPacket()
{
  if (!myRecvBuffer.Empty())
  {
    gLog.error("Internal error: SrvSocket::RecvPacket(): Called with full buffer (%lu bytes).",
        myRecvBuffer.getDataSize());
    return (true);
  }

  int nBytesReceived = 0;
  errno = 0;

  // Check if the buffer is empty
  char *buffer = new char[6];
  int nSixBytes = 0;
  while (nSixBytes != 6)
  {
    nBytesReceived = read(myDescriptor, buffer + nSixBytes, 6 - nSixBytes);
    if (nBytesReceived <= 0)
    {
      if (nBytesReceived == 0)
        gLog.warning(tr("server socket was closed!!!\n"));
      else {
        myErrorType = ErrorErrno;
        gLog.warning(tr("Error during receiving from server socket:\n%s"),
                     errorStr().c_str());
      }
      delete[] buffer;
      return (false);
    }
    nSixBytes += nBytesReceived;
  }

  // now we start to verify the FLAP header
  if (buffer[0] != 0x2a) {
    gLog.warning("Server send bad packet start code: %d.", buffer[0]);
    gLog.warning("Sixbyte: %02x %02x %02x %02x %02x %02x",
              buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
    myErrorType = ErrorErrno;
    delete[] buffer;
    return false;
  }

  // DAW maybe verify sequence number ?

  unsigned short nLen = ((unsigned char)buffer[5]) + (((unsigned char)buffer[4]) << 8);
#if 0
  // JON Recv size does not matter here i believe, m_xRevBuffer can create
  // a large enough packet, if there is enough memory.  It is not a static
  // buffer like RecvRaw
  if (nLen >= MAX_RECV_SIZE) {
    gLog.warning(tr("Server send bad packet with suspiciously large size: %d."), nLen);
    myErrorType = ErrorErrno;
    delete[] buffer;
    return false;
  }
#endif
  // push the 6 bytes at the beginning of the packet again..
  myRecvBuffer.Create(nLen + 6);
  myRecvBuffer.Pack(buffer, 6);
  delete[] buffer;

  while (!myRecvBuffer.Full())
  {
    // Determine the number of bytes left to be read into the buffer
    unsigned long nBytesLeft = myRecvBuffer.getDataStart() +
        myRecvBuffer.getDataMaxSize() - myRecvBuffer.getDataPosWrite();

    nBytesReceived = read(myDescriptor, myRecvBuffer.getDataPosWrite(), nBytesLeft);
    if (nBytesReceived == 0 ||
        (nBytesReceived < 0 && errno != EINTR) )
    {
      myErrorType = ErrorErrno;
      return (false);
    }
    myRecvBuffer.incDataPosWrite(nBytesReceived);
  }

  DumpPacket(&myRecvBuffer, true);

  return (true);
}


//=====TCPSocket===============================================================
TCPSocket::TCPSocket(const UserId& userId)
  : INetSocket(SOCK_STREAM, "TCP", userId),
    myChannel(ChannelNormal)
{
  m_p_SSL = NULL;
}

TCPSocket::TCPSocket()
  : INetSocket(SOCK_STREAM, "TCP", UserId()),
    myChannel(ChannelNormal)
{
  m_p_SSL = NULL;
}

TCPSocket::~TCPSocket()
{
  SecureStop();
}


/*-----TCPSocket::ReceiveConnection--------------------------------------------
 * Called to set up a given TCPSocket from an incoming connection on the
 * current TCPSocket
 *---------------------------------------------------------------------------*/
bool TCPSocket::RecvConnection(TCPSocket &newSocket)
{
  socklen_t sizeofSockaddr = sizeof(myRemoteAddrStorage);
  bool success = false;

  // Make sure we stay under FD_SETSIZE
  // See:
  // * http://www.securityfocus.com/archive/1/490711
  // * http://securityvulns.com/docs7669.html
  // for more details
  // This probably has no affect, since we are using multiple threads, but keep it here 
  // to be used as a sanity check.
  int newDesc = accept(myDescriptor, (struct sockaddr*)&newSocket.myRemoteAddr, &sizeofSockaddr);
  if (newDesc < 0)
  {
    // Something went wrong, probably indicates an error somewhere else
    gLog.warning(tr("Cannot accept new connection:\n%s"), strerror(errno));
    return false;
  }
  if (newDesc < static_cast<int>(FD_SETSIZE))
  {
    newSocket.myDescriptor = newDesc;
    newSocket.SetLocalAddress();
    success = true;
  }
  else
  {
    gLog.error(tr("Cannot accept new connection, too many descriptors in use."));
    close(newDesc);
  }

  return success;
}

#define m_pSSL ((SSL *) m_p_SSL)

/*-----TCPSocket::TransferConnectionFrom---------------------------------------
 * Transfers a connection from the given socket to the current one and closes
 * and resets the given socket
 *---------------------------------------------------------------------------*/
void TCPSocket::TransferConnectionFrom(TCPSocket &from)
{
  myDescriptor = from.myDescriptor;
  myLocalAddr = from.myLocalAddr;
  myRemoteAddr = from.myRemoteAddr;
  myUserId = from.myUserId;

  myVersion = from.myVersion;
  if (from.m_p_SSL)
  {
    pthread_mutex_lock(&from.mutex_ssl);

    pthread_mutex_init(&mutex_ssl, NULL);
    pthread_mutex_lock(&mutex_ssl);
    m_p_SSL = from.m_p_SSL;
    from.SecureStop();

    pthread_mutex_unlock(&mutex_ssl);
  }
  else
    m_p_SSL = NULL;
  ClearRecvBuffer();
  from.myDescriptor = -1;
  from.CloseConnection();
}

/*-----TCPSocket::SendPacket---------------------------------------------------
 * Sends a packet on a socket.  The socket is blocking, so we are guaranteed
 * that the entire packet will be sent, however, it may block if the tcp
 * buffer is full.  This should not be a problem unless we are sending a huge
 * packet.
 *---------------------------------------------------------------------------*/
bool TCPSocket::SendPacket(Buffer *b_in)
{
  char *pcSize = new char[2];
  Buffer *b = b_in;

  pcSize[0] = (b->getDataSize()) & 0xFF;
  pcSize[1] = (b->getDataSize() >> 8) & 0xFF;

#ifdef USE_OPENSSL
  if (m_pSSL != NULL)
  {
    int i, j;
    ERR_clear_error();
    pthread_mutex_lock(&mutex_ssl);
    i = SSL_write(m_pSSL, pcSize, 2);
    j = SSL_get_error(m_pSSL, i);
    pthread_mutex_unlock(&mutex_ssl);
    if (j != SSL_ERROR_NONE)
    {
      const char *file; int line;
      unsigned long err;
      switch (j)
      {
        case SSL_ERROR_SSL:
          err = ERR_get_error_line(&file, &line);
          gLog.error("SSL_write error = %lx, %s:%i", err, file, line);
          ERR_clear_error();
          break;
        default:
          gLog.error("SSL_write error %d, SSL_%d", i, j);
          break;
      }
    }

    ERR_clear_error();
    pthread_mutex_lock(&mutex_ssl);
    i = SSL_write(m_pSSL, b->getDataStart(), b->getDataSize());
    j = SSL_get_error(m_pSSL, i);
    pthread_mutex_unlock(&mutex_ssl);
    if (j != SSL_ERROR_NONE)
    {
      const char *file; int line;
      unsigned long err;
      switch (j)
      {
        case SSL_ERROR_SSL:
          err = ERR_get_error_line(&file, &line);
          gLog.error("SSL_write error = %lx, %s:%i", err, file, line);
          ERR_clear_error();
          break;
        default:
          gLog.error("SSL_write error %d, SSL_%d\n", i, j);
          break;
      }
    }
  }
  else
  {
#endif

  unsigned long nTotalBytesSent = 0;
  int nBytesSent = 0;

  //  send the length of the packet, close the connection and return false if unable to send
  while (nTotalBytesSent < 2) {
    nBytesSent = send(myDescriptor, pcSize + nTotalBytesSent, 2 - nTotalBytesSent, 0);
    if (nBytesSent <= 0) {
      delete[] pcSize;
      if (b != b_in) delete b;
      myErrorType = ErrorErrno;
      return (false);
    }
    nTotalBytesSent += nBytesSent;
  }
  delete[] pcSize;

  // send the rest of the packet
  nTotalBytesSent = 0;
  while (nTotalBytesSent < b->getDataSize())
  {
    nBytesSent = send(myDescriptor, b->getDataStart() + nTotalBytesSent,
                      b->getDataSize() - nTotalBytesSent, 0);
    if (nBytesSent <= 0)
    {
      myErrorType = ErrorErrno;
      if (b != b_in) delete b;
      return(false);
    }
    nTotalBytesSent += nBytesSent;
  }

#ifdef USE_OPENSSL
  }
#endif

  // Print the packet
  DumpPacket(b, false);

  if (b != b_in) delete b;
  return (true);
}


/*-----TCPSocket::ReceivePacket------------------------------------------------
 * Receive data on the socket.  Checks the buffer to see if it is empty, if
 * so, then it will create it using either the size read in from the socket
 * (the first two bytes available) or the given size.  Then determine the
 * number of bytes needed to fill the buffer and call recv with this amount.
 * Note if the buffer is not filled, then the next call to this function will
 * append more bytes until the buffer is full.  If more bytes are available
 * then we take, then the socket will still be readable after we end.  Also
 * note that it is the responsibility of the calling procedure to reset the
 * RecvBuffer if it is full as this is not done here.
 *---------------------------------------------------------------------------*/
bool TCPSocket::RecvPacket()
{
  if (myRecvBuffer.Full())
  {
    gLog.warning(tr("Internal error: TCPSocket::RecvPacket(): Called with full buffer (%lu bytes)."),
        myRecvBuffer.getDataSize());
    return (true);
  }

  int nBytesReceived = 0;
  errno = 0;

  // Check if the buffer is empty
  if (myRecvBuffer.Empty())
  {
    char *buffer = new char[2];
    int nTwoBytes = 0;
    while (nTwoBytes != 2)
    {
#ifdef USE_OPENSSL
      if (m_pSSL)
      {
        pthread_mutex_lock(&mutex_ssl);
        nBytesReceived = SSL_read(m_pSSL, buffer, 2);
        int tmp = SSL_get_error(m_pSSL, nBytesReceived);
        pthread_mutex_unlock(&mutex_ssl);
        switch (tmp)
        {
          case SSL_ERROR_NONE:
            break;
          case SSL_ERROR_WANT_READ:
          case SSL_ERROR_WANT_WRITE:
          case SSL_ERROR_WANT_X509_LOOKUP:
            break;
          case SSL_ERROR_ZERO_RETURN:
            myErrorType = ErrorErrno;
            errno = 0;
            delete[] buffer;
            return (false);
          case SSL_ERROR_SYSCALL:
            myErrorType = ErrorErrno;
            delete[] buffer;
            return (false);
          case SSL_ERROR_SSL:
            myErrorType = ErrorInternal;
            delete[] buffer;
            return (false);
        }
      }
      else
      {
#endif
      nBytesReceived = recv(myDescriptor, buffer + nTwoBytes, 2 - nTwoBytes, 0);
      if (nBytesReceived <= 0)
      {
        myErrorType = ErrorErrno;
        delete[] buffer;
        return (false);
      }
#ifdef USE_OPENSSL
      }
#endif
      nTwoBytes += nBytesReceived;
    }
    myRecvBuffer.Create(((unsigned char)buffer[0]) +
                         (((unsigned char)buffer[1]) << 8 ));
    delete[] buffer;
  }

  // Determine the number of bytes left to be read into the buffer
  unsigned long nBytesLeft = myRecvBuffer.getDataStart() +
      myRecvBuffer.getDataMaxSize() - myRecvBuffer.getDataPosWrite();
#ifdef USE_OPENSSL
  if (m_pSSL != NULL)
  {
    pthread_mutex_lock(&mutex_ssl);
    nBytesReceived = SSL_read(m_pSSL, myRecvBuffer.getDataPosWrite(), nBytesLeft);
    int tmp = SSL_get_error(m_pSSL, nBytesReceived);
    pthread_mutex_unlock(&mutex_ssl);
    switch (tmp)
    {
      case SSL_ERROR_NONE:
        break;
      case SSL_ERROR_WANT_READ:
      case SSL_ERROR_WANT_WRITE:
      case SSL_ERROR_WANT_X509_LOOKUP:
        return (true);
      case SSL_ERROR_ZERO_RETURN:
        myErrorType = ErrorErrno;
        errno = 0;
        return (false);
      case SSL_ERROR_SYSCALL:
        myErrorType = ErrorErrno;
        return (false);
      case SSL_ERROR_SSL:
        myErrorType = ErrorInternal;
        return (false);
    }
  }
  else
  {
#endif
  int f = fcntl(myDescriptor, F_GETFL);
  fcntl(myDescriptor, F_SETFL, f | O_NONBLOCK);
  nBytesReceived = recv(myDescriptor, myRecvBuffer.getDataPosWrite(), nBytesLeft, 0);
  fcntl(myDescriptor, F_SETFL, f & ~O_NONBLOCK);
  if (nBytesReceived <= 0)
  {
    myErrorType = ErrorErrno;
    if (errno == EAGAIN || errno == EWOULDBLOCK) return (true);
    return (false);
  }
#ifdef USE_OPENSSL
  }
#endif
  myRecvBuffer.incDataPosWrite(nBytesReceived);

  // Print the packet if it's full
  if (myRecvBuffer.Full())
    DumpPacket(&myRecvBuffer, true);

  return (true);
}


bool TCPSocket::SSLSend(Buffer *b_in)
{
#ifdef USE_OPENSSL
  if (m_pSSL == 0) return false;

  int i, j;
  ERR_clear_error();
  pthread_mutex_lock(&mutex_ssl);
  i = SSL_write(m_pSSL, b_in->getDataStart(), b_in->getDataSize());
  j = SSL_get_error(m_pSSL, i);
  pthread_mutex_unlock(&mutex_ssl);
  if (j != SSL_ERROR_NONE)
  {
    const char *file; int line;
    unsigned long err;
    switch (j)
    {
      case SSL_ERROR_SSL:
        err = ERR_get_error_line(&file, &line);
        printf("SSL_write error = %lx, %s:%i\n", err, file, line);
        ERR_clear_error();
        break;
      default:
        printf("SSL_write error %d, SSL_%d\n", i, j);
        break;
    }
  }

  DumpPacket(b_in, false);

  return true;
#else
  return false;
#endif
}


bool TCPSocket::SSLRecv()
{
#ifdef USE_OPENSSL
  if (m_pSSL == 0) return false;

  char *buffer = new char[MAX_RECV_SIZE];
  errno = 0;
  pthread_mutex_lock(&mutex_ssl);
  int nBytesReceived = SSL_read(m_pSSL, buffer, MAX_RECV_SIZE);
  int tmp = SSL_get_error(m_pSSL, nBytesReceived);
  pthread_mutex_unlock(&mutex_ssl);
  switch (tmp)
  {
    case SSL_ERROR_NONE:
      break;
    case SSL_ERROR_WANT_READ:
    case SSL_ERROR_WANT_WRITE:
    case SSL_ERROR_WANT_X509_LOOKUP:
      return (true);
    case SSL_ERROR_ZERO_RETURN:
      myErrorType = ErrorErrno;
      errno = 0;
      return (false);
    case SSL_ERROR_SYSCALL:
      myErrorType = ErrorErrno;
      return (false);
    case SSL_ERROR_SSL:
      myErrorType = ErrorInternal;
      return (false);
  }
  if (nBytesReceived <= 0)
  {
    delete[] buffer;
    myErrorType = ErrorErrno;
    return (false);
  }
  myRecvBuffer.Create(nBytesReceived);
  myRecvBuffer.Pack(buffer, nBytesReceived);
  delete[] buffer;

  // Print the packet
  DumpPacket(&myRecvBuffer, true);

  return (true);
#else
  return false;
#endif
}


bool TCPSocket::SSL_Pending()
{
#ifdef USE_OPENSSL
  return (m_pSSL && SSL_pending(m_pSSL));
#else
  return false;
#endif
}



#ifdef USE_OPENSSL /*-----Start of OpenSSL code----------------------------*/

bool TCPSocket::SecureConnect()
{
  pthread_mutex_init(&mutex_ssl, NULL);
  if (myUserId.protocolId() == LICQ_PPID)
    m_p_SSL = SSL_new(gSSL_CTX);
  else
    m_p_SSL = SSL_new(gSSL_CTX_NONICQ);
#ifdef SSL_DEBUG
  m_pSSL->debug = 1;
#endif
  SSL_set_session(m_pSSL, NULL);
  SSL_set_fd(m_pSSL, myDescriptor);
  int i = SSL_connect(m_pSSL);
  int j = SSL_get_error(m_pSSL, i);
  if (j != SSL_ERROR_NONE)
  {
    const char *file;
    int line;
    unsigned long err;
    switch (j)
    {
      case SSL_ERROR_SSL:
        err = ERR_get_error_line(&file, &line);
        gLog.warning(tr("SSL_connect error = %lx, %s:%i"), err, file, line);
        ERR_clear_error();
        break;
      default:
        gLog.warning(tr("SSL_connect error %d, SSL_%d"), i, j);
        break;
    }
    return false;
  }
  return true;
}

bool TCPSocket::SecureListen()
{
  pthread_mutex_init(&mutex_ssl, NULL);

  if (myUserId.protocolId() == LICQ_PPID)
    m_p_SSL = SSL_new(gSSL_CTX);
  else
    m_p_SSL = SSL_new(gSSL_CTX_NONICQ);
  SSL_set_session(m_pSSL, NULL);
  SSL_set_fd(m_pSSL, myDescriptor);
  int i = SSL_accept(m_pSSL);
  int j = SSL_get_error(m_pSSL, i);
  if (j != SSL_ERROR_NONE)
  {
    const char *file;
    int line;
    unsigned long err;
    switch (j)
    {
      case SSL_ERROR_SSL:
        err = ERR_get_error_line(&file, &line);
        gLog.warning("SSL_accept error = %lx, %s:%i\n%s", err, file, line,
                     ERR_error_string(err, 0));
        ERR_clear_error();
        break;
      default:
        err = ERR_get_error();
        gLog.warning("SSL_accept error %d, SSL_%d\n%s", i, j,
                     ERR_error_string(err, 0));
        break;
    }
    return false;
  }
  return true;
}

void TCPSocket::SecureStop()
{
  if(m_pSSL)
  {
    pthread_mutex_destroy(&mutex_ssl);
    SSL_free(m_pSSL);
    m_p_SSL = NULL;
  }
}

#else

bool TCPSocket::SecureConnect()
{
  return false;
}

bool TCPSocket::SecureListen()
{
  return false;
}

void TCPSocket::SecureStop()
{
  m_p_SSL = NULL;
}


#endif /*-----End of OpenSSL code------------------------------------------*/

UDPSocket::UDPSocket(const UserId& userId)
  : INetSocket(SOCK_DGRAM, "UDP", userId)
{
  // Empty
}

UDPSocket::~UDPSocket()
{
  // Empty
}

//=====Locking==================================================================
void INetSocket::Lock()
{
  myMutex.lock();
}

void INetSocket::Unlock()
{
  myMutex.unlock();
}
