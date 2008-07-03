// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998 - 2003 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

/* Socket routine descriptions */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef HAVE_INET_ATON
#include <arpa/inet.h>
#endif

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
  in_addr _in = { in };
  return inet_ntoa_r(_in, buf);
}

char *inet_ntoa_r(struct in_addr in, char *buf)
{
  register char *p;
  p = (char *)&in;
#define UC(b)   (((int)b)&0xff)
  sprintf(buf, "%d.%d.%d.%d", UC(p[0]), UC(p[1]), UC(p[2]), UC(p[3]));
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

char *INetSocket::LocalIpStr(char *buf)
{
  return (inet_ntoa_r(*(struct in_addr *)&m_sLocalAddr.sin_addr.s_addr, buf));
}

char *INetSocket::RemoteIpStr(char *buf)
{
  return (inet_ntoa_r(*(struct in_addr *)&m_sRemoteAddr.sin_addr.s_addr, buf));
}

void INetSocket::SetOwner(const char *_szOwnerId, unsigned long _nOwnerPPID)
{
  m_szOwnerId = strdup(_szOwnerId);
  m_nOwnerPPID = _nOwnerPPID;
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


INetSocket::INetSocket(const char *_szOwnerId, unsigned long _nOwnerPPID)
{
  m_nDescriptor = -1;
  if (_szOwnerId)
    m_szOwnerId = strdup(_szOwnerId);
  else
    m_szOwnerId = 0;
  m_nOwnerPPID = _nOwnerPPID;
  m_nVersion = 0;
  m_nErrorType = SOCK_ERROR_none;
  memset(&m_sRemoteAddr, 0, sizeof(struct sockaddr_in));
  memset(&m_sLocalAddr, 0, sizeof(struct sockaddr_in));
  m_szRemoteName = NULL;
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

  if (m_szOwnerId)
    free(m_szOwnerId);

  if (m_szRemoteName != NULL) free (m_szRemoteName);
}

//-----INetSocket::dumpPacket---------------------------------------------------
void INetSocket::DumpPacket(CBuffer *b, direction d)
{
  char *szPacket;
  char szIpR[32], szIpL[32];

  // This speeds things up if no one is logging packets
  if (!gLog.LoggingPackets()) return;

  switch(d)
  {
  case D_SENDER:
    gLog.Packet("%sPacket (%sv%lu, %lu bytes) sent:\n%s(%s:%d -> %s:%d)\n%s\n",
     L_PACKETxSTR, m_szID, Version(), b->getDataSize(), L_BLANKxSTR,
     LocalIpStr(szIpL),
     LocalPort(), RemoteIpStr(szIpR), RemotePort(), b->print(szPacket));
    break;
  case D_RECEIVER:
     gLog.Packet("%sPacket (%sv%lu, %lu bytes) received:\n%s(%s:%d <- %s:%d)\n%s\n",
      L_PACKETxSTR, m_szID, Version(), b->getDataSize(), L_BLANKxSTR,
      LocalIpStr(szIpL),
      LocalPort(), RemoteIpStr(szIpR), RemotePort(), b->print(szPacket));
     break;
  }
  delete[] szPacket;
}


/*-----INetSocket::setDestination----------------------------------------------
 * Takes an ip in network order and a port in host byte order and sets the
 * remote ip and port to those values
 *---------------------------------------------------------------------------*/
bool INetSocket::SetRemoteAddr(unsigned long _nRemoteIp, unsigned short _nRemotePort)
{
  if (_nRemoteIp == 0 || _nRemotePort == 0)
  {
    m_nErrorType = SOCK_ERROR_h_errno;
    // The Remote IP could be 0 if a proxy could not resolve it.  So let's do
    // this to fix that.
    if (m_xProxy)
      m_sRemoteAddr.sin_port = htons(_nRemotePort);
    return(false);  // if the rIp is invalid, exit
  }

  m_sRemoteAddr.sin_port = htons(_nRemotePort);
  m_sRemoteAddr.sin_addr.s_addr = _nRemoteIp;
  return(true);
}


//-----INetSocket::ResetSocket-------------------------------------------------
void INetSocket::ResetSocket()
{
  CloseConnection();
  memset(&m_sRemoteAddr, 0, sizeof(struct sockaddr_in));
  memset(&m_sLocalAddr, 0, sizeof(struct sockaddr_in));
}


/*-----INetSocket::SetLocalAddress------------------------------------------
 * Sets the sockaddr_in structures using data from the connected socket
 *---------------------------------------------------------------------------*/
bool INetSocket::SetLocalAddress(bool bIp)
{
  // Setup the local structure
  socklen_t sizeofSockaddr = sizeof(struct sockaddr_in);

  if (getsockname(m_nDescriptor, (struct sockaddr *)&m_sLocalAddr, &sizeofSockaddr) < 0)
  {
    m_nErrorType = SOCK_ERROR_errno;
    return (false);
  }

  // This should never happen unless the IP stack is fucked
  if (m_sLocalAddr.sin_addr.s_addr == INADDR_ANY && bIp)
  {
    gLog.Warn(tr("%sYour IP stack or SOCKS client is a piece of crap.\n"
                 "%sAttempting to guess local IP.\n"), L_WARNxSTR, L_BLANKxSTR);
    char szHostName[256];
    if (gethostname(szHostName, 256) == -1)
    {
      strcpy(szHostName, "localhost");
    }
    struct hostent sLocalHost;
    char temp[1024];
    h_errno = gethostbyname_r_portable(szHostName, &sLocalHost, temp, sizeof(temp));
    if (h_errno != 0)
    {
      m_nErrorType = SOCK_ERROR_h_errno;
      return false;
    }
    m_sLocalAddr.sin_addr.s_addr = ((struct in_addr *)sLocalHost.h_addr)->s_addr;
  }
  return (true);
}


//-----INetSocket::GetIpByName-------------------------------------------------
unsigned long INetSocket::GetIpByName(const char *_szHostName)
{
  // check if the hostname is in dot and number notation
  struct in_addr ina;
  if (inet_aton(_szHostName, &ina))
     return(ina.s_addr);

  // try and resolve hostname
  struct hostent host;
  char temp[1024];
  h_errno = gethostbyname_r_portable(_szHostName, &host, temp, sizeof(temp));
  if (h_errno == -1) // Couldn't resolve hostname/ip
  {
    return (0);
  }
  else if (h_errno > 0)
  {
    return (0);
  }
  // return the ip
  return ((struct in_addr *)(host.h_addr))->s_addr;
}


//-----INetSocket::OpenConnection-----------------------------------------------
bool INetSocket::OpenConnection()
{
  if (m_xProxy != NULL)
  {
    if (!m_xProxy->OpenConnection())
    {
      m_nErrorType = SOCK_ERROR_proxy;
      return(false);
    }

    bool ret;
    if (m_szRemoteName)
      ret = m_xProxy->OpenProxyConnection(m_szRemoteName, RemotePort());
    else
    {
      char szIpR[32];
      ret = m_xProxy->OpenProxyConnection(RemoteIpStr(szIpR), RemotePort());
    }
    if (!ret)
    {
      m_nErrorType = SOCK_ERROR_proxy;
      return(false);
    }
    
    m_nDescriptor = m_xProxy->Descriptor();
  }
  else
  {
    // If no destination set then someone screwed up
    if(m_sRemoteAddr.sin_addr.s_addr == 0 || ntohs(m_sRemoteAddr.sin_port) == 0)
    {
      m_nErrorType = SOCK_ERROR_internal;
      return(false);
    }

    if (m_nDescriptor == -1)
      m_nDescriptor = socket(AF_INET, m_nSockType, 0);
    if (m_nDescriptor == -1)
    {
      m_nErrorType = SOCK_ERROR_errno;
      return(false);
    }

#ifdef IP_PORTRANGE
    int i=IP_PORTRANGE_HIGH;
    if (setsockopt(m_nDescriptor, IPPROTO_IP, IP_PORTRANGE, &i, sizeof(i))<0)
    {
      m_nErrorType = SOCK_ERROR_errno;
      return(false);
    }
#endif

    m_sRemoteAddr.sin_family = AF_INET;

    // if connect fails then call CloseConnection to clean up before returning
    socklen_t sizeofSockaddr = sizeof(struct sockaddr);
    if (connect(m_nDescriptor, (struct sockaddr *)&m_sRemoteAddr, sizeofSockaddr) < 0)
    {
      // errno has been set
      m_nErrorType = SOCK_ERROR_errno;
      CloseConnection();
      return(false);
    }

#ifdef USE_SOCKS5
    if (m_nSockType != SOCK_STREAM) return true;
#endif
  }
  
  return SetLocalAddress();
}


//-----INetSocket::StartServer--------------------------------------------------
bool INetSocket::StartServer(unsigned int _nPort)
{
  m_nDescriptor = socket(AF_INET, m_nSockType, 0);
  if (m_nDescriptor == -1)
  {
    m_nErrorType = SOCK_ERROR_errno;
    return (false);
  }

#ifdef IP_PORTRANGE
  int i=IP_PORTRANGE_HIGH;
  if (setsockopt(m_nDescriptor, IPPROTO_IP, IP_PORTRANGE, &i, sizeof(i))<0)
  {
    m_nErrorType = SOCK_ERROR_errno;
    ::close(m_nDescriptor);
    m_nDescriptor = -1;
    return(false);
  }
#endif

  memset(&m_sLocalAddr.sin_zero, 0, 8);
  m_sLocalAddr.sin_family = AF_INET;
  m_sLocalAddr.sin_port = htons(_nPort);
  m_sLocalAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(m_nDescriptor, (struct sockaddr *)&m_sLocalAddr, sizeof(sockaddr_in)) == -1)
  {
    m_nErrorType = SOCK_ERROR_errno;
    ::close(m_nDescriptor);
    m_nDescriptor = -1;
    return (false);
  }

  if (!SetLocalAddress(false)) return (false);

  if (m_nSockType == SOCK_STREAM)
    listen(m_nDescriptor, 10); // Allow 10 unprocessed connections
  return(true);
}


//-----INetSocket::SetRemoteAddr-----------------------------------------------
bool INetSocket::SetRemoteAddr(const char *_szRemoteName, unsigned short _nRemotePort)
{
  if (m_szRemoteName != NULL) free (m_szRemoteName);
  if (_szRemoteName != NULL)
    m_szRemoteName = strdup(_szRemoteName);
  else
    m_szRemoteName = NULL;

  return(SetRemoteAddr(GetIpByName(_szRemoteName), _nRemotePort));
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

SrvSocket::SrvSocket(const char *s, unsigned long n) : INetSocket(s, n)
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

/*-----SrvSocket::ConnectTo--------------------------------------------------
* Establish connection to server (through proxy or not)
*---------------------------------------------------------------------------*/
bool SrvSocket::ConnectTo(const char* server, unsigned short port,
                          ProxyServer *xProxy)
{
  char ipbuf[32];

  if (xProxy == NULL)
  {
    gLog.Info(tr("%sResolving %s port %d...\n"), L_SRVxSTR, server, port);
    if (!SetRemoteAddr(server, port)) {
      char buf[128];
      gLog.Warn(tr("%sUnable to resolve %s:\n%s%s.\n"), L_ERRORxSTR,
                server, L_BLANKxSTR, ErrorStr(buf, 128));
      return false;
    }
    gLog.Info(tr("%sICQ server found at %s:%d.\n"), L_SRVxSTR,
              RemoteIpStr(ipbuf), RemotePort());
  }
  else
  {
    // It doesn't matter if it resolves or not, the proxy should do it then
    SetProxy(xProxy);
    SetRemoteAddr(server, port);
  }

  if (xProxy == NULL)
    gLog.Info(tr("%sOpening socket to server.\n"), L_SRVxSTR);
  else
    gLog.Info(tr("%sOpening socket to server %s:%d via proxy.\n"),
              L_SRVxSTR, server, port);
  if (!OpenConnection())
  {
    char buf[128];
    gLog.Warn(tr("%sUnable to connect to %s:%d:\n%s%s.\n"), L_ERRORxSTR,
              RemoteIpStr(ipbuf), RemotePort(), L_BLANKxSTR,
              ErrorStr(buf, 128));
    return false;
  }

  return true;
}

//=====TCPSocket===============================================================
TCPSocket::TCPSocket(const char *s, unsigned long n) : INetSocket(s, n)
{
  strcpy(m_szID, "TCP");
  m_nSockType = SOCK_STREAM;
  m_p_SSL = NULL;
}

TCPSocket::TCPSocket() : INetSocket(0, 0)
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
  socklen_t sizeofSockaddr = sizeof(struct sockaddr_in);
  bool success = false;

  // Make sure we stay under FD_SETSIZE
  // See:
  // * http://www.securityfocus.com/archive/1/490711
  // * http://securityvulns.com/docs7669.html
  // for more details
  // This probably has no affect, since we are using multiple threads, but keep it here 
  // to be used as a sanity check.
  int newDesc = accept(m_nDescriptor, (struct sockaddr *)&newSocket.m_sRemoteAddr, &sizeofSockaddr);
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
  m_sLocalAddr = from.m_sLocalAddr;
  m_sRemoteAddr = from.m_sRemoteAddr;

  if (m_szOwnerId)
    free (m_szOwnerId);
  if (from.m_szOwnerId)
    m_szOwnerId = strdup(from.m_szOwnerId);
  else
    m_szOwnerId = 0;
  m_nOwnerPPID = from.m_nOwnerPPID;

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
  if (m_nOwnerPPID == LICQ_PPID)
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

  if (m_nOwnerPPID == LICQ_PPID)
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

UDPSocket::UDPSocket(const char* ownerId, unsigned long ownerPpid)
    : INetSocket(ownerId, ownerPpid)
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
  pthread_rdwr_init_np(&mutex_rw, NULL);
  pthread_rdwr_set_name(&mutex_rw, __func__);
}

CSocketHashTable::~CSocketHashTable()
{
  pthread_rdwr_destroy_np(&mutex_rw);
}

void CSocketHashTable::Lock(unsigned short _nLockType)
{
  switch (_nLockType)
  {
  case LOCK_R:
    pthread_rdwr_rlock_np (&mutex_rw);
    break;
  case LOCK_W:
    pthread_rdwr_wlock_np(&mutex_rw);
    break;
  default:
    break;
  }
  m_nLockType = _nLockType;
}

void CSocketHashTable::Unlock()
{
  unsigned short nLockType = m_nLockType;
  m_nLockType = LOCK_R;
  switch (nLockType)
  {
  case LOCK_R:
    pthread_rdwr_runlock_np(&mutex_rw);
    break;
  case LOCK_W:
    pthread_rdwr_wunlock_np(&mutex_rw);
    break;
  default:
    break;
  }
}

INetSocket *CSocketHashTable::Retrieve(int _nSd)
{
  Lock(LOCK_R);

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

  Unlock();
  return s;
}

void CSocketHashTable::Store(INetSocket *s, int _nSd)
{
  Lock(LOCK_W);
  list<INetSocket *> &l = m_vlTable[HashValue(_nSd)];
  l.push_front(s);
  Unlock();
}

void CSocketHashTable::Remove(int _nSd)
{
  Lock(LOCK_W);
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
  Unlock();
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

  char *szOwner = s->OwnerId() ? strdup(s->OwnerId()) : 0;
  unsigned long nPPID = s->OwnerPPID();
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
    if (szOwner)
    {
      u = gUserManager.FetchUser(szOwner, nPPID, LOCK_W);
      free(szOwner);
    }
    
    if (u != NULL)
    {
      u->ClearSocketDesc(nChannel);
      if (u->OfflineOnDisconnect())
        gLicqDaemon->ChangeUserStatus(u, ICQ_STATUS_OFFLINE);
      gUserManager.DropUser(u);
    }
  }
}

