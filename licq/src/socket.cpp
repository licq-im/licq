/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1998-2013 Licq developers <licq-dev@googlegroups.com>
 *
 * Licq is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Licq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Licq; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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
#define socket_send Rsend
#undef send
#else
#define socket_send send
#endif // SOCKS5

#ifdef SOCKS5_OPTLEN
  #ifdef socklen_t
    #undef socklen_t
  #endif

  #define socklen_t SOCKS5_OPTLEN
#endif

#include <licq/buffer.h>
#include <licq/proxy.h>
#include <licq/logging/log.h>

#include "gettext.h"

using Licq::Buffer;
using Licq::INetSocket;
using Licq::TCPSocket;
using Licq::UDPSocket;
using Licq::UserId;
using std::string;

char* Licq::ip_ntoa(unsigned long in, char *buf)
{
  inet_ntop(AF_INET, &in, buf, 32);
  return buf;
}


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
    b->log(Log::Debug, "Packet (%s, %lu bytes) sent:\n(%s:%d -> %s:%d)",
           myLogId.c_str(), b->getDataSize(),
           getLocalIpString().c_str(), getLocalPort(),
           getRemoteIpString().c_str(), getRemotePort());
  }
  else
  {
    b->log(Log::Debug, "Packet (%s, %lu bytes) received:\n(%s:%d <- %s:%d)",
           myLogId.c_str(), b->getDataSize(),
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

  if (::bind(myDescriptor, (struct sockaddr*)&myLocalAddr, addrlen) == -1)
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
  if (myDescriptor != -1)
  {
    ::shutdown(myDescriptor, 2);
    ::close (myDescriptor);
    myDescriptor = -1;
  }
}

bool INetSocket::send(Buffer& buf)
{
  // send the packet
  int bytesLeft = buf.getDataSize();
  char* dataPos = buf.getDataStart();
  while (bytesLeft > 0)
  {
    ssize_t bytesSent = ::socket_send(myDescriptor, dataPos, bytesLeft, 0);
    if (bytesSent < 0)
    {
      if (errno == EINTR)
        continue;
      myErrorType = ErrorErrno;
      return(false);
    }
    bytesLeft -= bytesSent;
    dataPos += bytesSent;
  }

  // Print the packet
  DumpPacket(&buf, false);
  return (true);
}

bool INetSocket::receive(Buffer& buf, size_t maxlength, bool dump)
{
  // Don't try to read more than the buffer has room for
  if (buf.Full())
    return true;
  if (!buf.Empty() && maxlength > buf.remainingDataToWrite())
    maxlength = buf.remainingDataToWrite();

  char* buffer = new char[maxlength];
  errno = 0;
  int f = fcntl(myDescriptor, F_GETFL);
  fcntl(myDescriptor, F_SETFL, f | O_NONBLOCK);
  ssize_t bytesReceived = recv(myDescriptor, buffer, maxlength, 0);
  fcntl(myDescriptor, F_SETFL, f & ~O_NONBLOCK);
  if (bytesReceived <= 0)
  {
    delete[] buffer;
    myErrorType = ErrorErrno;
    if (errno == EAGAIN || errno == EWOULDBLOCK)
      return true;
    return (false);
  }
  if (buf.Empty())
    buf.Create(bytesReceived);
  buf.packRaw(buffer, bytesReceived);
  delete[] buffer;

  // Print the packet
  if (dump)
    DumpPacket(&buf, true);

  return (true);
}


//=====TCPSocket===============================================================
TCPSocket::TCPSocket(const UserId& userId)
  : INetSocket(SOCK_STREAM, "TCP", userId)
{
  m_p_SSL = NULL;
}

TCPSocket::TCPSocket()
  : INetSocket(SOCK_STREAM, "TCP", UserId())
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

#ifdef USE_OPENSSL
#define m_pSSL ((SSL *) m_p_SSL)
#else
#define m_pSSL m_p_SSL
#endif

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
  from.myDescriptor = -1;
  from.CloseConnection();
}

/*-----TCPSocket::SendPacket---------------------------------------------------
 * Sends a packet on a socket.  The socket is blocking, so we are guaranteed
 * that the entire packet will be sent, however, it may block if the tcp
 * buffer is full.  This should not be a problem unless we are sending a huge
 * packet.
 *---------------------------------------------------------------------------*/
bool TCPSocket::send(Buffer& buf)
{
  if (m_pSSL == NULL)
    return INetSocket::send(buf);

#ifdef USE_OPENSSL
  int i, j;
  ERR_clear_error();
  pthread_mutex_lock(&mutex_ssl);
  i = SSL_write(m_pSSL, buf.getDataStart(), buf.getDataSize());
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

  DumpPacket(&buf, false);

  return true;
#else
  return false;
#endif
}

bool TCPSocket::receive(Buffer& buf, size_t maxlength, bool dump)
{
  // If SSL not enabled for this socket, use normal receive
  if (m_pSSL == NULL)
    return INetSocket::receive(buf, maxlength, dump);

#ifdef USE_OPENSSL
  if (buf.Full())
    return true;
  if (!buf.Empty() && maxlength > buf.remainingDataToWrite())
    maxlength = buf.remainingDataToWrite();

  char* buffer = new char[maxlength];
  errno = 0;
  pthread_mutex_lock(&mutex_ssl);
  int nBytesReceived = SSL_read(m_pSSL, buffer, maxlength);
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
  if (buf.Empty())
    buf.Create(nBytesReceived);
  buf.packRaw(buffer, nBytesReceived);
  delete[] buffer;

  // Print the packet
  if (dump)
    DumpPacket(&buf, true);

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
  if (myUserId.protocolId() == ICQ_PPID)
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

  if (myUserId.protocolId() == ICQ_PPID)
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
