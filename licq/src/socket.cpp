// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998 - 2009 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

/* Socket routine descriptions */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <arpa/inet.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef HAVE_ERRNO_H
#include <errno.h>
#else
extern int errno;
extern int h_errno;
#endif

// Localization
#include "gettext.h"

#include "licq_socket.h"
#include "licq_icq.h"
#include "licq_log.h"
#include "licq_user.h"
#include "support.h"
#include "licq_icqd.h"
#include "licq_proxy.h"

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

using namespace std;

char *ip_ntoa(unsigned long in, char *buf)
{
  inet_ntop(AF_INET, &in, buf, 32);
  return buf;
}


//=====Constants================================================================
const unsigned long MAX_RECV_SIZE = 4096;


CSocketManager gSocketManager;


//=====CFdSet===================================================================
CSocketSet::CSocketSet()
{
  FD_ZERO(&sFd);
  // Initialise the mutex
  pthread_mutex_init(&mutex, NULL);
}

CSocketSet::~CSocketSet()
{
  // Empty
}

void CSocketSet::Set(int _nSD)
{
  Lock();
  FD_SET(_nSD, &sFd);
  list<int>::iterator i = lFd.begin();
  while (i != lFd.end() && _nSD < *i) ++i;
  lFd.insert(i, _nSD);
  Unlock();
}

void CSocketSet::Clear (int _nSD)
{
  Lock();
  FD_CLR(_nSD, &sFd);
  list<int>::iterator i = lFd.begin();
  while (i != lFd.end() && *i != _nSD) ++i;
  if (i != lFd.end()) lFd.erase(i);
  Unlock();
}

unsigned short CSocketSet::Num()
{
  Lock();
  unsigned short n = lFd.size();
  Unlock();
  return n;
}

int CSocketSet::Largest()
{
  if (Num() == 0)
    return 0;
  else
  {
    Lock();
    unsigned short l = *lFd.begin();
    Unlock();
    return l;
  }
}

fd_set CSocketSet::SocketSet()
{
  Lock();
  fd_set f = sFd;
  Unlock();
  return f;
}

void CSocketSet::Lock()
{
  pthread_mutex_lock(&mutex);
}

void CSocketSet::Unlock()
{
  pthread_mutex_unlock(&mutex);
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

char *INetSocket::LocalIpStr(char *buf) const
{
  string str = getLocalIpString();
  strcpy(buf, str.c_str());
  return buf;
}

char *INetSocket::RemoteIpStr(char *buf) const
{
  string str = getRemoteIpString();
  strcpy(buf, str.c_str());
  return buf;
}

//-----INetSocket::Error------------------------------------------------------
int INetSocket::Error()
{
  switch (m_nErrorType)
  {
    case SOCK_ERROR_errno: return errno;
    case SOCK_ERROR_h_errno: return h_errno;
    case SOCK_ERROR_desx: return -1;
    case SOCK_ERROR_none: return 0;
    case SOCK_ERROR_internal: return -2;
    case SOCK_ERROR_proxy: if (m_xProxy != NULL) return m_xProxy->Error();
  }
  return 0;
}


//-----INetSocket::ErrorStr---------------------------------------------------
char *INetSocket::ErrorStr(char *buf, int buflen)
{
  switch (m_nErrorType)
  {
    case SOCK_ERROR_errno:
      strncpy(buf, strerror(errno), buflen);
      buf[buflen - 1] = '\0';
      break;

    case SOCK_ERROR_h_errno:
#ifndef HAVE_HSTRERROR
      sprintf(buf, tr("hostname resolution failure (%d)"), h_errno);
#else
      strncpy(buf, hstrerror(h_errno), buflen);
      buf[buflen - 1] = '\0';
#endif
      break;

    case SOCK_ERROR_desx:
      strncpy(buf, tr("DesX encryption/decryption failure"), buflen);
      buf[buflen - 1] = '\0';
      break;

    case SOCK_ERROR_none:
      strncpy(buf, tr("No error detected"), buflen);
      buf[buflen - 1] = '\0';
      break;

    case SOCK_ERROR_internal:
      strncpy(buf, tr("Internal error"), buflen);
      buf[buflen - 1] = '\0';
      break;
    case SOCK_ERROR_proxy:
      if (m_xProxy != NULL)
	return m_xProxy->ErrorStr(buf, buflen);
      break;
  }

  return buf;
}


INetSocket::INetSocket(const UserId& userId)
{
  m_nDescriptor = -1;
  myUserId = userId;
  m_nVersion = 0;
  m_nErrorType = SOCK_ERROR_none;
  memset(&myRemoteAddr, 0, sizeof(myRemoteAddrStorage));
  memset(&myLocalAddr, 0, sizeof(myLocalAddrStorage));
  m_xProxy = NULL;
  m_nChannel = ICQ_CHNxNONE;
  
  // Initialize the mutex
  pthread_mutex_init(&mutex, NULL);
}

INetSocket::~INetSocket()
{
  CloseConnection();
  // Destroy the mutex
  int nResult = 0;
  do
  {
    pthread_mutex_lock(&mutex);
    pthread_mutex_unlock(&mutex);
    nResult = pthread_mutex_destroy(&mutex);
  } while (nResult != 0);
}

//-----INetSocket::dumpPacket---------------------------------------------------
void INetSocket::DumpPacket(CBuffer *b, direction d)
{
  char *szPacket;

  // This speeds things up if no one is logging packets
  if (!gLog.LoggingPackets()) return;

  switch(d)
  {
  case D_SENDER:
    gLog.Packet("%sPacket (%sv%lu, %lu bytes) sent:\n%s(%s:%d -> %s:%d)\n%s\n",
     L_PACKETxSTR, m_szID, Version(), b->getDataSize(), L_BLANKxSTR,
          getLocalIpString().c_str(), getLocalPort(),
          getRemoteIpString().c_str(), getRemotePort(),
          b->print(szPacket));
    break;
  case D_RECEIVER:
     gLog.Packet("%sPacket (%sv%lu, %lu bytes) received:\n%s(%s:%d <- %s:%d)\n%s\n",
      L_PACKETxSTR, m_szID, Version(), b->getDataSize(), L_BLANKxSTR,
          getLocalIpString().c_str(), getLocalPort(),
          getRemoteIpString().c_str(), getRemotePort(),
          b->print(szPacket));
     break;
  }
  delete[] szPacket;
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

  if (getsockname(m_nDescriptor, (struct sockaddr*)&myLocalAddr, &sizeofSockaddr) < 0)
  {
    m_nErrorType = SOCK_ERROR_errno;
    return (false);
  }

  return (true);
}

bool INetSocket::connectTo(const string& remoteName, uint16_t remotePort, ProxyServer* proxy)
{
  myRemoteName = remoteName;
  m_xProxy = proxy;

  // If we're using a proxy, let the proxy class handle this
  if (m_xProxy != NULL)
  {
    if (!m_xProxy->OpenConnection())
    {
      m_nErrorType = SOCK_ERROR_proxy;
      return(false);
    }

    bool ret = m_xProxy->OpenProxyConnection(remoteName.c_str(), remotePort);
    if (!ret)
    {
      m_nErrorType = SOCK_ERROR_proxy;
      return(false);
    }

    memcpy(&myRemoteAddr, m_xProxy->ProxyAddr(), sizeof(myRemoteAddrStorage));
    m_nDescriptor = m_xProxy->Descriptor();
    return SetLocalAddress();
  }

  // No proxy, let's do this ourselves

  // If already connected, close the old connection first
  if (m_nDescriptor != -1)
    CloseConnection();

  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
#ifdef LICQ_DISABLE_IPV6
  hints.ai_family = AF_INET;
#else
  hints.ai_family = AF_UNSPEC;
#endif
  hints.ai_socktype = m_nSockType;
#ifdef AI_ADDRCONFIG
  // AI_ADDRCONFIG = Don't return IPvX address if host has no IPvX address configured
  hints.ai_flags = AI_ADDRCONFIG;
#endif

  // If anything happens here, the error will be in errno
  m_nErrorType = SOCK_ERROR_errno;

  struct addrinfo* addrs;
  int s = getaddrinfo(remoteName.c_str(), NULL, &hints, &addrs);
  if(s != 0)
  {
    gLog.Warn(tr("%sError when trying to resolve %s. getaddrinfo() returned %d\n."),
        L_WARNxSTR, remoteName.c_str(), s);
    return false;
  }

  // getaddrinfo() returns a list of addresses, we'll try them one by one until
  //   we manage to make a connection. The list is already be sorted with
  //   preferred address first.
  struct addrinfo* ai;
  for (ai = addrs; ai != NULL; ai = ai->ai_next)
  {
    memcpy(&myRemoteAddr, ai->ai_addr, sizeof(myRemoteAddrStorage));

    // We didn't use getaddrinfo to lookup port so set in manually
    if (myRemoteAddr.sa_family == AF_INET)
      ((struct sockaddr_in*)&myRemoteAddr)->sin_port = htons(remotePort);
    else if (myRemoteAddr.sa_family == AF_INET6)
      ((struct sockaddr_in6*)&myRemoteAddr)->sin6_port = htons(remotePort);

    gLog.Info(tr("%sConnecting to %s:%i...\n"), L_SRVxSTR,
        addrToString(&myRemoteAddr).c_str(), remotePort);

    // Create socket of the returned type
    m_nDescriptor = socket(myRemoteAddr.sa_family, m_nSockType, 0);
    if (m_nDescriptor == -1)
      continue;

#ifdef IP_PORTRANGE
    int i=IP_PORTRANGE_HIGH;
    if (setsockopt(m_nDescriptor, IPPROTO_IP, IP_PORTRANGE, &i, sizeof(i))<0)
    {
      close(m_nDescriptor);
      m_nDescriptor = -1;
      gLog.Warn(tr("%sFailed to set port range for socket.\n"), L_WARNxSTR);
      continue;
    }
#endif

    // Try to connect, exit loop if successful
    if (connect(m_nDescriptor, (struct sockaddr*)&myRemoteAddr, sizeof(myRemoteAddrStorage)) != -1)
      break;

    // Failed to connect, close socket and try next
    close(m_nDescriptor);
    m_nDescriptor = -1;
  }

  // Return the memory allocated by getaddrinfo
  freeaddrinfo(addrs);

  // If we reached the end of the address list we didn't find anything that could connect
  if (ai == NULL)
    return false;

#ifdef USE_SOCKS5
    if (m_nSockType != SOCK_STREAM) return true;
#endif

  return SetLocalAddress();
}

bool INetSocket::connectTo(uint32_t remoteAddr, uint16_t remotePort, ProxyServer* proxy)
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
  m_nDescriptor = socket(AF_INET6, m_nSockType, 0);
  if (m_nDescriptor != -1)
  {
    // IPv6 socket created

#ifdef IPV6_PORTRANGE
    int i = IPV6_PORTRANGE_HIGH;
    if (setsockopt(m_nDescriptor, IPPROTO_IPV6, IPV6_PORTRANGE, &i, sizeof(i)) < 0)
    {
      m_nErrorType = SOCK_ERROR_errno;
      ::close(m_nDescriptor);
      m_nDescriptor = -1;
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
    gLog.Warn(tr("%sFailed to start local server using IPv6 socket (falling back to IPv4):\n%s%s\n"),
        L_WARNxSTR, L_BLANKxSTR, strerror(errno));

#endif
    m_nDescriptor = socket(AF_INET, m_nSockType, 0);
    if (m_nDescriptor == -1)
    {
      m_nErrorType = SOCK_ERROR_errno;
      return (false);
    }

#ifdef IP_PORTRANGE
    int i = IP_PORTRANGE_HIGH;
    if (setsockopt(m_nDescriptor, IPPROTO_IP, IP_PORTRANGE, &i, sizeof(i)) < 0)
    {
      m_nErrorType = SOCK_ERROR_errno;
      ::close(m_nDescriptor);
      m_nDescriptor = -1;
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

  if (bind(m_nDescriptor, (struct sockaddr*)&myLocalAddr, addrlen) == -1)
  {
    m_nErrorType = SOCK_ERROR_errno;
    ::close(m_nDescriptor);
    m_nDescriptor = -1;
    return (false);
  }

  if (!SetLocalAddress(false)) return (false);

  if (m_nSockType == SOCK_STREAM)
  {
    // Allow 10 unprocessed connections
    if (listen(m_nDescriptor, 10) != 0)
    {
      m_nErrorType = SOCK_ERROR_errno;
      ::close(m_nDescriptor);
      m_nDescriptor = -1;
      return false;
    }
  }

  return(true);
}


//-----INetSocket::CloseConnection-------------------------------------------
void INetSocket::CloseConnection()
{
  m_xRecvBuffer.Clear();
  if (m_nDescriptor != -1)
  {
    ::shutdown(m_nDescriptor, 2);
    ::close (m_nDescriptor);
    m_nDescriptor = -1;
  }
}

//-----INetSocket::SendRaw------------------------------------------------------
bool INetSocket::SendRaw(CBuffer *b)
{
  // send the packet
  int nBytesSent;
  unsigned long nTotalBytesSent = 0;
  while (nTotalBytesSent < b->getDataSize())
  {
    nBytesSent = send(m_nDescriptor, b->getDataStart() + nTotalBytesSent,
                      b->getDataSize() - nTotalBytesSent, 0);
    if (nBytesSent < 0)
    {
      m_nErrorType = SOCK_ERROR_errno;
      return(false);
    }
    nTotalBytesSent += nBytesSent;
  }

  // Print the packet
  DumpPacket(b, D_SENDER);
  return (true);
}


/*-----INetSocket::RecvRaw---------------------------------------------------
 * Receive data on the socket.
 *---------------------------------------------------------------------------*/
bool INetSocket::RecvRaw()
{
  char *buffer = new char[MAX_RECV_SIZE];
  errno = 0;
  int nBytesReceived = recv(m_nDescriptor, buffer, MAX_RECV_SIZE, 0);
  if (nBytesReceived <= 0)
  {
    delete[] buffer;
    m_nErrorType = SOCK_ERROR_errno;
    return (false);
  }
  m_xRecvBuffer.Create(nBytesReceived);
  m_xRecvBuffer.Pack(buffer, nBytesReceived);
  delete[] buffer;

  // Print the packet
  DumpPacket(&m_xRecvBuffer, D_RECEIVER);

  return (true);
}

//=====SrvSocket===============================================================

SrvSocket::SrvSocket(const UserId& userId)
  : INetSocket(userId)
{
  strcpy(m_szID, "SRV");
  m_nSockType = SOCK_STREAM;
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

bool SrvSocket::SendPacket(CBuffer *b_in)
{
  CBuffer *b = b_in;

  unsigned long nTotalBytesSent = 0;
  int nBytesSent = 0;

  // send the packet
  nTotalBytesSent = 0;
  errno = 0;
  while (nTotalBytesSent < b->getDataSize())
  {
    nBytesSent = send(m_nDescriptor, b->getDataStart() + nTotalBytesSent,
                      b->getDataSize() - nTotalBytesSent, 0);
    if (nBytesSent <= 0)
    {
      if (nBytesSent < 0 && errno == EINTR) continue;
      m_nErrorType = SOCK_ERROR_errno;
      if (b != b_in) delete b;
      return(false);
    }
    nTotalBytesSent += nBytesSent;
  }

  // Print the packet
  DumpPacket(b, D_SENDER);

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
  if (!m_xRecvBuffer.Empty())
  {
    gLog.Error("%sInternal error: SrvSocket::RecvPacket(): Called with full buffer (%lu bytes).\n",
              L_WARNxSTR, m_xRecvBuffer.getDataSize());
    return (true);
  }

  int nBytesReceived = 0;
  errno = 0;

  // Check if the buffer is empty
  char *buffer = new char[6];
  int nSixBytes = 0;
  while (nSixBytes != 6)
  {
    nBytesReceived = read(m_nDescriptor, buffer + nSixBytes, 6 - nSixBytes);
    if (nBytesReceived <= 0)
    {
      if (nBytesReceived == 0)
        gLog.Warn(tr("server socket was closed!!!\n"));
      else {
        char buf[128];
        m_nErrorType = SOCK_ERROR_errno;
        gLog.Warn(tr("%serror during receiving from server socket :-((\n%s%s\n"),
                  L_WARNxSTR, L_BLANKxSTR, ErrorStr(buf, sizeof(buf)));
      }
      delete[] buffer;
      return (false);
    }
    nSixBytes += nBytesReceived;
  }

  // now we start to verify the FLAP header
  if (buffer[0] != 0x2a) {
    gLog.Warn("%sServer send bad packet start code: %d.\n", L_WARNxSTR, buffer[0]);
    gLog.Warn("%sSixbyte: %02x %02x %02x %02x %02x %02x\n", L_WARNxSTR,
              buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
    m_nErrorType = SOCK_ERROR_errno;
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
    gLog.Warn("%sServer send bad packet with suspiciously large size: %d.\n", L_WARNxSTR, nLen);
    m_nErrorType = SOCK_ERROR_errno;
    delete[] buffer;
    return false;
  }
#endif
  // push the 6 bytes at the beginning of the packet again..
  m_xRecvBuffer.Create(nLen + 6);
  m_xRecvBuffer.Pack(buffer, 6);
  delete[] buffer;

  while ( !m_xRecvBuffer.Full()) {
    // Determine the number of bytes left to be read into the buffer
    unsigned long nBytesLeft = m_xRecvBuffer.getDataStart() +
                               m_xRecvBuffer.getDataMaxSize() -
                               m_xRecvBuffer.getDataPosWrite();

    nBytesReceived = read(m_nDescriptor, m_xRecvBuffer.getDataPosWrite(), nBytesLeft);
    if (nBytesReceived == 0 ||
        (nBytesReceived < 0 && errno != EINTR) )
    {
      m_nErrorType = SOCK_ERROR_errno;
      return (false);
    }
    m_xRecvBuffer.incDataPosWrite(nBytesReceived);
  }

  DumpPacket(&m_xRecvBuffer, D_RECEIVER);

  return (true);
}


//=====TCPSocket===============================================================
TCPSocket::TCPSocket(const UserId& userId)
  : INetSocket(userId)
{
  strcpy(m_szID, "TCP");
  m_nSockType = SOCK_STREAM;
  m_p_SSL = NULL;
}

TCPSocket::TCPSocket()
  : INetSocket(USERID_NONE)
{
  strcpy(m_szID, "TCP");
  m_nSockType = SOCK_STREAM;
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
  int newDesc = accept(m_nDescriptor, (struct sockaddr*)&newSocket.myRemoteAddr, &sizeofSockaddr);
  if (newDesc < 0)
  {
    // Something went wrong, probably indicates an error somewhere else
    gLog.Warn(tr("%sCannot accept new connection:\n%s%s\n"), L_WARNxSTR, L_BLANKxSTR, strerror(errno));
    return false;
  }
  if (newDesc < FD_SETSIZE)
  {
    newSocket.m_nDescriptor = newDesc;
    newSocket.SetLocalAddress();
    success = true;
  }
  else
  {
    gLog.Error(tr("%sCannot accept new connection, too many descriptors in use.\n"), L_ERRORxSTR);
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
  m_nDescriptor = from.m_nDescriptor;
  myLocalAddr = from.myLocalAddr;
  myRemoteAddr = from.myRemoteAddr;
  myUserId = from.myUserId;

  m_nVersion = from.m_nVersion;
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
  from.m_nDescriptor = -1;
  from.CloseConnection();
}

/*-----TCPSocket::SendPacket---------------------------------------------------
 * Sends a packet on a socket.  The socket is blocking, so we are guaranteed
 * that the entire packet will be sent, however, it may block if the tcp
 * buffer is full.  This should not be a problem unless we are sending a huge
 * packet.
 *---------------------------------------------------------------------------*/
bool TCPSocket::SendPacket(CBuffer *b_in)
{
  char *pcSize = new char[2];
  CBuffer *b = b_in;

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
          gLog.Error("%sSSL_write error = %lx, %s:%i\n", L_SSLxSTR, err, file, line);
          ERR_clear_error();
          break;
        default:
          gLog.Error("%sSSL_write error %d, SSL_%d\n", L_SSLxSTR, i, j);
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
          gLog.Error("%sSSL_write error = %lx, %s:%i\n", L_SSLxSTR, err, file, line);
          ERR_clear_error();
          break;
        default:
          gLog.Error("%sSSL_write error %d, SSL_%d\n", L_SSLxSTR, i, j);
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
    nBytesSent = send(m_nDescriptor, pcSize + nTotalBytesSent, 2 - nTotalBytesSent, 0);
    if (nBytesSent <= 0) {
      delete[] pcSize;
      if (b != b_in) delete b;
      m_nErrorType = SOCK_ERROR_errno;
      return (false);
    }
    nTotalBytesSent += nBytesSent;
  }
  delete[] pcSize;

  // send the rest of the packet
  nTotalBytesSent = 0;
  while (nTotalBytesSent < b->getDataSize())
  {
    nBytesSent = send(m_nDescriptor, b->getDataStart() + nTotalBytesSent,
                      b->getDataSize() - nTotalBytesSent, 0);
    if (nBytesSent <= 0)
    {
      m_nErrorType = SOCK_ERROR_errno;
      if (b != b_in) delete b;
      return(false);
    }
    nTotalBytesSent += nBytesSent;
  }

#ifdef USE_OPENSSL
  }
#endif

  // Print the packet
  DumpPacket(b, D_SENDER);

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
  if (m_xRecvBuffer.Full())
  {
    gLog.Warn("%sInternal error: TCPSocket::RecvPacket(): Called with full buffer (%lu bytes).\n",
              L_WARNxSTR, m_xRecvBuffer.getDataSize());
    return (true);
  }

  int nBytesReceived = 0;
  errno = 0;

  // Check if the buffer is empty
  if (m_xRecvBuffer.Empty())
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
            m_nErrorType = SOCK_ERROR_errno;
            errno = 0;
            delete[] buffer;
            return (false);
          case SSL_ERROR_SYSCALL:
            m_nErrorType = SOCK_ERROR_errno;
            delete[] buffer;
            return (false);
          case SSL_ERROR_SSL:
            m_nErrorType = SOCK_ERROR_internal;
            delete[] buffer;
            return (false);
        }
      }
      else
      {
#endif
      nBytesReceived = recv(m_nDescriptor, buffer + nTwoBytes, 2 - nTwoBytes, 0);
      if (nBytesReceived <= 0)
      {
        m_nErrorType = SOCK_ERROR_errno;
        delete[] buffer;
        return (false);
      }
#ifdef USE_OPENSSL
      }
#endif
      nTwoBytes += nBytesReceived;
    }
    m_xRecvBuffer.Create(((unsigned char)buffer[0]) +
                         (((unsigned char)buffer[1]) << 8 ));
    delete[] buffer;
  }

  // Determine the number of bytes left to be read into the buffer
  unsigned long nBytesLeft = m_xRecvBuffer.getDataStart() +
                             m_xRecvBuffer.getDataMaxSize() -
                             m_xRecvBuffer.getDataPosWrite();
#ifdef USE_OPENSSL
  if (m_pSSL != NULL)
  {
    pthread_mutex_lock(&mutex_ssl);
    nBytesReceived = SSL_read(m_pSSL, m_xRecvBuffer.getDataPosWrite(), nBytesLeft);
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
        m_nErrorType = SOCK_ERROR_errno;
        errno = 0;
        return (false);
      case SSL_ERROR_SYSCALL:
        m_nErrorType = SOCK_ERROR_errno;
        return (false);
      case SSL_ERROR_SSL:
        m_nErrorType = SOCK_ERROR_internal;
        return (false);
    }
  }
  else
  {
#endif
  int f = fcntl(m_nDescriptor, F_GETFL);
  fcntl(m_nDescriptor, F_SETFL, f | O_NONBLOCK);
  nBytesReceived = recv(m_nDescriptor, m_xRecvBuffer.getDataPosWrite(), nBytesLeft, 0);
  fcntl(m_nDescriptor, F_SETFL, f & ~O_NONBLOCK);
  if (nBytesReceived <= 0)
  {
    m_nErrorType = SOCK_ERROR_errno;
    if (errno == EAGAIN || errno == EWOULDBLOCK) return (true);
    return (false);
  }
#ifdef USE_OPENSSL
  }
#endif
  m_xRecvBuffer.incDataPosWrite(nBytesReceived);

  // Print the packet if it's full
  if (m_xRecvBuffer.Full())
    DumpPacket(&m_xRecvBuffer, D_RECEIVER);

  return (true);
}


bool TCPSocket::SSLSend(CBuffer *b_in)
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
 
  DumpPacket(b_in, D_SENDER);

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
      m_nErrorType = SOCK_ERROR_errno;
      errno = 0;
      return (false);
    case SSL_ERROR_SYSCALL:
      m_nErrorType = SOCK_ERROR_errno;
      return (false);
    case SSL_ERROR_SSL:
      m_nErrorType = SOCK_ERROR_internal;
      return (false);
  }
  if (nBytesReceived <= 0)
  {
    delete[] buffer;
    m_nErrorType = SOCK_ERROR_errno;
    return (false);
  }
  m_xRecvBuffer.Create(nBytesReceived);
  m_xRecvBuffer.Pack(buffer, nBytesReceived);
  delete[] buffer;

  // Print the packet
  DumpPacket(&m_xRecvBuffer, D_RECEIVER);

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
  if (LicqUser::getUserProtocolId(myUserId) == LICQ_PPID)
    m_p_SSL = SSL_new(gSSL_CTX);
  else
    m_p_SSL = SSL_new(gSSL_CTX_NONICQ);
#ifdef SSL_DEBUG
  m_pSSL->debug = 1;
#endif
  SSL_set_session(m_pSSL, NULL);
  SSL_set_fd(m_pSSL, m_nDescriptor);
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
        gLog.Warn("%sSSL_connect error = %lx, %s:%i\n", L_WARNxSTR, err, file, line);
        ERR_clear_error();
        break;
      default:
        gLog.Warn("%sSSL_connect error %d, SSL_%d\n", L_WARNxSTR, i, j);
        break;
    }
    return false;
  }
  return true;
}

bool TCPSocket::SecureListen()
{
  pthread_mutex_init(&mutex_ssl, NULL);

  if (LicqUser::getUserProtocolId(myUserId) == LICQ_PPID)
    m_p_SSL = SSL_new(gSSL_CTX);
  else
    m_p_SSL = SSL_new(gSSL_CTX_NONICQ);
  SSL_set_session(m_pSSL, NULL);
  SSL_set_fd(m_pSSL, m_nDescriptor);
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
        gLog.Warn("%sSSL_accept error = %lx, %s:%i\n", L_SSLxSTR, err, file, line);
        gLog.Warn("%s%s\n", L_SSLxSTR, ERR_error_string(err, 0));
        ERR_clear_error();
        break;
      default:
        err = ERR_get_error();
        gLog.Warn("%sSSL_accept error %d, SSL_%d\n", L_SSLxSTR, i, j);
        gLog.Warn("%s%s\n", L_SSLxSTR, ERR_error_string(err, 0));
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
  : INetSocket(userId)
{
  strcpy(m_szID, "UDP");
  m_nSockType = SOCK_DGRAM;
}

UDPSocket::~UDPSocket()
{
  // Empty
}

//=====Locking==================================================================
void INetSocket::Lock()
{
  pthread_mutex_lock (&mutex);
}

void INetSocket::Unlock()
{
  pthread_mutex_unlock (&mutex);
}


//=====CSocketHashTable=========================================================
CSocketHashTable::CSocketHashTable(unsigned short _nSize) : m_vlTable(_nSize)
{
  myMutex.setName(__func__);
}

CSocketHashTable::~CSocketHashTable()
{
}

INetSocket *CSocketHashTable::Retrieve(int _nSd)
{
  myMutex.lockRead();

  INetSocket *s = NULL;
  list <INetSocket *> &l = m_vlTable[HashValue(_nSd)];

  int nSd;
  list<INetSocket *>::iterator iter;
  for (iter = l.begin(); iter != l.end(); ++iter)
  {
    (*iter)->Lock();
    nSd = (*iter)->Descriptor();
    (*iter)->Unlock();
    if (nSd == _nSd)
    {
      s = (*iter);
      break;
    }
  }
  if (iter == l.end()) s = NULL;

  myMutex.unlockRead();
  return s;
}

void CSocketHashTable::Store(INetSocket *s, int _nSd)
{
  myMutex.lockWrite();
  list<INetSocket *> &l = m_vlTable[HashValue(_nSd)];
  l.push_front(s);
  myMutex.unlockWrite();
}

void CSocketHashTable::Remove(int _nSd)
{
  myMutex.lockWrite();
  list<INetSocket *> &l = m_vlTable[HashValue(_nSd)];
  int nSd;
  list<INetSocket *>::iterator iter;
  for (iter = l.begin(); iter != l.end(); ++iter)
  {
    (*iter)->Lock();
    nSd = (*iter)->Descriptor();
    (*iter)->Unlock();
    if (nSd == _nSd)
    {
      l.erase(iter);
      break;
    }
  }
  myMutex.unlockWrite();
}

unsigned short CSocketHashTable::HashValue(int _nSd)
{
  //return _nSd % m_vlTable.size();
  return _nSd & (unsigned long)(SOCKET_HASH_SIZE - 1);
}


//=====CSocketManager===========================================================
CSocketManager::CSocketManager() : m_hSockets(SOCKET_HASH_SIZE)
{
  pthread_mutex_init(&mutex, NULL);
}

CSocketManager::~CSocketManager()
{
  int nResult = 0;
  do
  {
    pthread_mutex_lock(&mutex);
    pthread_mutex_unlock(&mutex);
    nResult = pthread_mutex_destroy(&mutex);
  } while (nResult != 0);
}

INetSocket *CSocketManager::FetchSocket(int _nSd)
{
  pthread_mutex_lock(&mutex);
  INetSocket *s = m_hSockets.Retrieve(_nSd);
  if (s != NULL) s->Lock();
  pthread_mutex_unlock(&mutex);
  return s;
}


void CSocketManager::DropSocket(INetSocket *s)
{
  if (s != NULL) s->Unlock();
}


void CSocketManager::AddSocket(INetSocket *s)
{
  s->Lock();
  m_hSockets.Store(s, s->Descriptor());
  m_sSockets.Set(s->Descriptor());
}


void CSocketManager::CloseSocket (int nSd, bool bClearUser, bool bDelete)
{
  // Quick check that the socket is valid
  if (nSd == -1) return;

  pthread_mutex_lock(&mutex);

  // Clear from the socket list
  m_sSockets.Clear(nSd);

  // Fetch the actual socket
  INetSocket *s = m_hSockets.Retrieve(nSd);
  if (s == NULL)
  {
    pthread_mutex_unlock(&mutex);
    return;
  }

  UserId userId = s->userId();
  unsigned char nChannel = s->Channel();
  
  // First remove the socket from the hash table so it won't be fetched anymore
  m_hSockets.Remove(nSd);

  pthread_mutex_unlock(&mutex);

  // Now close the connection (we don't have to lock it first, because the
  // Remove function above guarantees that no one has a lock on the socket
  // before removing it from the hash table, and once removed from the has
  // table, no one can get a lock again.
  s->CloseConnection();
  if (bDelete) delete s;

  if (bClearUser)
  {
    ICQUser *u = NULL;
    if (USERID_ISVALID(userId))
      u = gUserManager.fetchUser(userId, LOCK_W);

    if (u != NULL)
    {
      u->ClearSocketDesc(nChannel);
      if (u->OfflineOnDisconnect())
        gLicqDaemon->ChangeUserStatus(u, ICQ_STATUS_OFFLINE);
      gUserManager.DropUser(u);
    }
  }
}

