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

#ifndef MSG_DONTWAIT
#define MSG_DONTWAIT 0
#endif

#ifdef HAVE_INET_ATON
#include <arpa/inet.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#else
extern int errno;
extern int h_errno;
#endif

#include "socket.h"
#include "icq-defines.h"
#include "log.h"
#include "support.h"
#include "user.h"

#ifdef USE_SOCKS5
#define SOCKS
#define INCLUDE_PROTOTYPES
extern "C" {
#include <socks.h>
}
#endif


//=====Constants================================================================
const unsigned short ADDR_LOCAL = 0x01;
const unsigned short ADDR_REMOTE = 0x02;
const unsigned long MAX_RECV_SIZE = 4096;


CSocketManager gSocketManager;


//=====CFdSet===================================================================
CSocketSet::CSocketSet ()
{
  FD_ZERO(&sFd);
  // Initialise the mutex
}

void CSocketSet::Set(int _nSD)
{
  Lock();
  FD_SET(_nSD, &sFd);
  list<int>::iterator i = lFd.begin();
  while (i != lFd.end() && _nSD < *i) i++;
  lFd.insert(i, _nSD);
  Unlock();
}

void CSocketSet::Clear (int _nSD)
{
  Lock();
  FD_CLR(_nSD, &sFd);
  list<int>::iterator i = lFd.begin();
  while (i != lFd.end() && *i != _nSD) i++;
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

//-----INetSocket::OpenSocket---------------------------------------------------
void INetSocket::OpenSocket()
{
}


//-----INetSocket::Error------------------------------------------------------
int INetSocket::Error()
{
  return (errno == -1 ? (h_errno == -1 ? 0 : h_errno) : errno);
}


//-----INetSocket::ErrorStr---------------------------------------------------
char *INetSocket::ErrorStr(char *buf, int buflen)
{
  if (errno <= 0 && h_errno <= 0)
    strcpy(buf, "No error detected!");
  else if (errno > 0)
    strncpy(buf, strerror(errno), buflen);
  else if (h_errno > 0)
#ifndef HAVE_HSTRERROR
    sprintf(buf, "hostname resolution failure (%d)", h_errno);
#else
    strncpy(buf, hstrerror(h_errno), buflen);
#endif
  return buf;
}


//-----INetSocket::constructor--------------------------------------------------
INetSocket::INetSocket(unsigned long _nOwner)
{
  m_nDescriptor = -1;
  m_nOwner = _nOwner;
  memset(&m_sRemoteAddr, 0, sizeof(struct sockaddr_in));
  memset(&m_sLocalAddr, 0, sizeof(struct sockaddr_in));

  // Initialise the mutex
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
  char szIpR[32], szIpL[32];

  // This speeds things up if no one is logging packets
  if (!gLog.LoggingPackets()) return;

  switch(d)
  {
  case D_SENDER:
    gLog.Packet("%sPacket (%s, %d bytes) sent (%s:%d -> %s:%d):\n%s\n",
                L_PACKETxSTR, m_szID, b->getDataSize(), LocalIpStr(szIpL),
                LocalPort(), RemoteIpStr(szIpR), RemotePort(), b->print(szPacket));
     break;
  case D_RECEIVER:
     gLog.Packet("%sPacket (%s, %d bytes) received (%s:%d <- %s:%d):\n%s\n",
                L_PACKETxSTR, m_szID, b->getDataSize(), LocalIpStr(szIpL),
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
  if (_nRemoteIp == 0) return(false);  // if the rIp is invalid, exit

  m_sRemoteAddr.sin_port = htons(_nRemotePort);
  m_sRemoteAddr.sin_addr.s_addr = _nRemoteIp;
  return(true);
}


//-----INetSocket::ResetSocket-------------------------------------------------
void INetSocket::ResetSocket()
{
  CloseSocket();
  memset(&m_sRemoteAddr, 0, sizeof(struct sockaddr_in));
  memset(&m_sLocalAddr, 0, sizeof(struct sockaddr_in));
}


/*-----INetSocket::SetAddrsFromSocket------------------------------------------
 * Sets the sockaddr_in structures using data from the connected socket
 *---------------------------------------------------------------------------*/
bool INetSocket::SetAddrsFromSocket(unsigned short _nFlags)
{
  if (_nFlags & ADDR_LOCAL)
  {
    // Setup the local structure
    socklen_t sizeofSockaddr = sizeof(struct sockaddr_in);
    if (getsockname(m_nDescriptor, (struct sockaddr *)&m_sLocalAddr, &sizeofSockaddr) < 0)
    {
      // errno has been set
      h_errno = -1;
      return (false);
    }
    if (m_sLocalAddr.sin_addr.s_addr == INADDR_ANY)
    {
      char szHostName[256];
      if (gethostname(szHostName, 256) == -1)
      {
        strcpy(szHostName, "localhost");
        //h_errno = -1;
        //return false;
      }
      struct hostent sLocalHost;
      h_errno = gethostbyname_r_portable(szHostName, &sLocalHost);
      if (h_errno != 0)
      {
        errno = -1;
        return false;
      }
      m_sLocalAddr.sin_addr.s_addr = *((unsigned long *)sLocalHost.h_addr);
    }
  }
/*
  if (_nFlags & ADDR_REMOTE)
  {
    // Set up the remote structure
    sizeofSockaddr = sizeof(struct sockaddr_in);
    if (getpeername(m_nDescriptor, (struct sockaddr *)&m_sRemoteAddr, &sizeofSockaddr) < 0)
    {
      // errno has been set
      h_errno = -1;
      return (false);
    }
  }
*/
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
  h_errno = gethostbyname_r_portable(_szHostName, &host);
  if (h_errno == -1) // Couldn't resolve hostname/ip
  {
     // errno has been set
     return (0);
  }
  else if (h_errno > 0)
  {
    // h_errno has been set
    errno = -1;
    return (0);
  }
  // return the ip
  return *((unsigned long *)(host.h_addr));
}


//-----INetSocket::OpenConnection-----------------------------------------------
bool INetSocket::OpenConnection()
{
  // If no destination set then someone screwed up
  if(m_sRemoteAddr.sin_addr.s_addr == 0) return(false);

  if (m_nDescriptor < 0)
    m_nDescriptor = socket(AF_INET, m_nSockType, 0);
  if (m_nDescriptor < 0)
  {
    // errno has been set
    h_errno = -1;
    return(false);
  }

#ifdef IP_PORTRANGE
  int i=IP_PORTRANGE_HIGH;
  if (setsockopt(m_nDescriptor, IPPROTO_IP, IP_PORTRANGE, &i, sizeof(i))<0)
  {
    h_errno = -1;
    return(false);
  }
#endif

  OpenSocket();
  m_sRemoteAddr.sin_family = AF_INET;

  // if connect fails then call CloseSocket to clean up before returning
  socklen_t sizeofSockaddr = sizeof(struct sockaddr);
  if (connect(m_nDescriptor, (struct sockaddr *)&m_sRemoteAddr, sizeofSockaddr) < 0)
  {
    // errno has been set
    h_errno = -1;
    CloseSocket();
    return(false);
  }

#ifdef USE_SOCKS5
  if (m_nSockType != SOCK_STREAM) return true;
#endif

  if (!SetAddrsFromSocket(ADDR_LOCAL | ADDR_REMOTE))
    return (false);
  return (true);
}


//-----INetSocket::StartServer--------------------------------------------------
bool INetSocket::StartServer(unsigned int _nPort)
{
  m_nDescriptor = socket(AF_INET, m_nSockType, 0);
  if (m_nDescriptor == -1)
  {
    // errno has been set
    h_errno = -1;
    return (false);
  }

#ifdef IP_PORTRANGE
  int i=IP_PORTRANGE_HIGH;
  if (setsockopt(m_nDescriptor, IPPROTO_IP, IP_PORTRANGE, &i, sizeof(i))<0)
  {
    h_errno = -1;
    return(false);
  }
#endif

  OpenSocket();
  memset(&m_sLocalAddr.sin_zero, 0, 8);
  m_sLocalAddr.sin_family = AF_INET;
  m_sLocalAddr.sin_port = htons(_nPort);
  m_sLocalAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(m_nDescriptor, (struct sockaddr *)&m_sLocalAddr, sizeof(sockaddr_in)) == -1)
  {
    h_errno = -1;
    return (false);
  }

  if (!SetAddrsFromSocket(ADDR_LOCAL)) return (false);

  if (m_nSockType == SOCK_STREAM)
    listen(m_nDescriptor, 10); // Allow 10 unprocessed connections
  return(true);
}


//-----INetSocket::SetRemoteAddr-----------------------------------------------
bool INetSocket::SetRemoteAddr(char *_szRemoteName, unsigned short _nRemotePort)
{
  return(SetRemoteAddr(GetIpByName(_szRemoteName), _nRemotePort));
}


//-----INetSocket::CloseSocket--------------------------------------------------
void INetSocket::CloseSocket()
{
  m_xRecvBuffer.Clear();
  if (m_nDescriptor != -1)
  {
    close (m_nDescriptor);
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
      // errno has been set
      h_errno = -1;
      //CloseSocket();
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
    // errno has been set
    h_errno = -1;
    //CloseSocket();
    return (false);
  }
  m_xRecvBuffer.Create(nBytesReceived);
  m_xRecvBuffer.Pack(buffer, nBytesReceived);
  delete[] buffer;

  // Print the packet
  DumpPacket(&m_xRecvBuffer, D_RECEIVER);

  return (true);
}


//=====TCPSocket===============================================================


/*-----TCPSocket::ReceiveConnection--------------------------------------------
 * Called to set up a given TCPSocket from an incoming connection on the
 * current TCPSocket
 *---------------------------------------------------------------------------*/
void TCPSocket::RecvConnection(TCPSocket &newSocket)
{
  socklen_t sizeofSockaddr = sizeof(struct sockaddr_in);
  newSocket.m_nDescriptor = accept(m_nDescriptor, (struct sockaddr *)&newSocket.m_sRemoteAddr, &sizeofSockaddr);
  newSocket.OpenSocket();
  newSocket.SetAddrsFromSocket(ADDR_LOCAL | ADDR_REMOTE);
}


/*-----TCPSocket::TransferConnectionFrom---------------------------------------
 * Transfers a connection from the given socket to the current one and closes
 * and resets the given socket (not used anymore)
 *---------------------------------------------------------------------------*/
void TCPSocket::TransferConnectionFrom(TCPSocket &from)
{
  m_nDescriptor = from.m_nDescriptor;
  m_sLocalAddr = from.m_sLocalAddr;
  m_sRemoteAddr = from.m_sRemoteAddr;
  ClearRecvBuffer();
//---ACK IS THIS OK???---
  from.m_nDescriptor = -1;
  from.CloseSocket();
//-----------------------
  OpenSocket();
}



/*-----TCPSocket::SendPacket---------------------------------------------------
 * Sends a packet on a socket.  The socket is blocking, so we are guaranteed
 * that the entire packet will be sent, however, it may block if the tcp
 * buffer is full.  This should not be a problem unless we are sending a huge
 * packet.
 *---------------------------------------------------------------------------*/
bool TCPSocket::SendPacket(CBuffer *b)
{
  char *pcSize = new char[2];
  pcSize[0] = (b->getDataSize()) & 0xFF;
  pcSize[1] = (b->getDataSize() >> 8) & 0xFF;

  // send the length of the packet, close the connection and return false if unable to send
  int nBytesSent = send(m_nDescriptor, pcSize, 2, 0);
  if (nBytesSent <= 0)
  {
    delete[] pcSize;
    // errno has been set
    h_errno = -1;
    //CloseSocket();
    return (false);
  }
  delete[] pcSize;

  // send the rest of the packet
  unsigned long nTotalBytesSent = 0;
  while (nTotalBytesSent < b->getDataSize())
  {
    nBytesSent = send(m_nDescriptor, b->getDataStart() + nTotalBytesSent,
                      b->getDataSize() - nTotalBytesSent, 0);
    if (nBytesSent < 0)
    {
      // errno has been set
      h_errno = -1;
      //CloseSocket();
      return(false);
    }
    nTotalBytesSent += nBytesSent;
  }

  // Print the packet
  DumpPacket(b, D_SENDER);

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
    gLog.Warn("%sInternal error: TCPSocket::RecvPacket(): Called with full buffer (%d bytes).\n", 
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
      nBytesReceived = recv(m_nDescriptor, buffer + nTwoBytes, 2 - nTwoBytes, 0);
      if (nBytesReceived <= 0)
      {
        // errno has been set
        h_errno = -1;
        delete[] buffer;
        //CloseSocket();
        return (false);
      }
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
  nBytesReceived = recv(m_nDescriptor, m_xRecvBuffer.getDataPosWrite(), nBytesLeft, MSG_DONTWAIT);
  if (nBytesReceived <= 0)
  {
    // errno has been set
    h_errno = -1;
    if (errno == EAGAIN || errno == EWOULDBLOCK) return (true);
    //CloseSocket();
    return (false);
  }
  m_xRecvBuffer.incDataPosWrite(nBytesReceived);

  // Print the packet if it's full
  if (m_xRecvBuffer.Full()) DumpPacket(&m_xRecvBuffer, D_RECEIVER);

  return (true);
}


//=====Locking==================================================================
static void LockSocket(INetSocket *s)
{
  if (s == NULL) return;
  pthread_mutex_lock (&s->mutex);
}

static void UnlockSocket(INetSocket *s)
{
  if (s == NULL) return;
  pthread_mutex_unlock (&s->mutex);
}


//=====CSocketHashTable=========================================================
CSocketHashTable::CSocketHashTable(unsigned short _nSize) : m_vlTable(_nSize) 
{
  pthread_rdwr_init_np(&mutex_rw, NULL);
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
  // If no sockets, this is bad
  if (l.size() == 0)
    s = NULL;

  // if only one, assume it's the right one (saves having to lock the socket
  // and checking)
  else if (l.size() == 1)
    s = *(l.begin());

  // Otherwise, start iterating and comparing uins
  else
  {
    int nSd;
    list<INetSocket *>::iterator iter;
    for (iter = l.begin(); iter != l.end(); iter++)
    {
      LockSocket(*iter);
      nSd = (*iter)->Descriptor();
      UnlockSocket(*iter);
      if (nSd == _nSd) 
      {
        s = (*iter);
        break;
      }
    }
    if (iter == l.end()) s = NULL;
  }

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
  for (iter = l.begin(); iter != l.end(); iter++)
  {
    LockSocket(*iter);
    nSd = (*iter)->Descriptor();
    UnlockSocket(*iter);
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
}

INetSocket *CSocketManager::FetchSocket(int _nSd)
{
  INetSocket *s = m_hSockets.Retrieve(_nSd);
  if (s != NULL) LockSocket(s);
  return s;
}


void CSocketManager::DropSocket(INetSocket *s)
{
  if (s != NULL) UnlockSocket(s);
}


void CSocketManager::AddSocket(INetSocket *s)
{
  LockSocket(s);
  m_hSockets.Store(s, s->Descriptor());
  m_sSockets.Set(s->Descriptor());
}


void CSocketManager::CloseSocket (int _nSd, bool _bClearUser)
{
  INetSocket *s = FetchSocket(_nSd);
  if (s == NULL) return;
  unsigned long nOwner = s->Owner();
  DropSocket(s);

  // First remove the socket from the hash table so it won't be fetched anymore
  m_hSockets.Remove(_nSd);
  m_sSockets.Clear(_nSd);

  // Now close the connection (we don't have to lock it first, because the
  // Remove function above guarantees that no one has a lock on the socket
  // before removing it from the hash table, and once removed from the has
  // table, no one can get a lock again.
  s->CloseConnection();
  delete s;

  if (_bClearUser)
  {
    ICQUser *u = gUserManager.FetchUser(nOwner, LOCK_W);
    if (u != NULL)
    {
      u->ClearSocketDesc();
      gUserManager.DropUser(u);
    }
  }
}

