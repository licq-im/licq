#ifndef socket_h
#define socket_h

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <vector.h>
#include <list.h>

#include "buffer.h"
#include "pthread_rdwr.h"
#include "support.h"
#include "constants.h"


//=====INetSocket==================================================================================
class INetSocket
{
public:
  INetSocket(unsigned long _nOwner);
  virtual ~INetSocket(void);

  bool Connected(void)          { return(m_nDescriptor > 0);  }
  int Descriptor(void)          { return(m_nDescriptor);      }
  bool DestinationSet(void)     { return (RemoteIp() != 0); }
  unsigned long Owner(void)     { return (m_nOwner); }
  void SetOwner(unsigned long _nOwner)  { m_nOwner = _nOwner; }

  int Error(void);
  char *ErrorStr(char *, int);

  unsigned long  LocalIp(void)     { return (m_sLocalAddr.sin_addr.s_addr);  }
  char *LocalIpStr(char *buf)      { return (inet_ntoa_r(*(struct in_addr *)&m_sLocalAddr.sin_addr.s_addr, buf)); }
  unsigned long  RemoteIp(void)    { return (m_sRemoteAddr.sin_addr.s_addr); };
  char *RemoteIpStr(char *buf)     { return (inet_ntoa_r(*(struct in_addr *)&m_sRemoteAddr.sin_addr.s_addr, buf)); }
  unsigned short LocalPort(void)   { return (ntohs(m_sLocalAddr.sin_port));  };
  unsigned short RemotePort(void)  { return (ntohs(m_sRemoteAddr.sin_port)); };

  bool SetRemoteAddr(unsigned long _nRemoteIp, unsigned short _nRemotePort);
  bool SetRemoteAddr(char *_szRemoteName, unsigned short _nRemotePort);

  void ResetSocket(void);
  void ClearRecvBuffer(void)  { m_xRecvBuffer.Clear(); };
  bool RecvBufferFull(void)   { return m_xRecvBuffer.Full(); };
  CBuffer &RecvBuffer(void)   { return m_xRecvBuffer; };

  bool OpenConnection(void);
  void CloseConnection(void)  {  CloseSocket(); }
  bool StartServer(unsigned int _nPort);
  bool SendRaw(CBuffer *b);
  bool RecvRaw(void);

  virtual bool Send(CBuffer *b) = 0;
  virtual bool Recv(void) = 0;

  pthread_mutex_t mutex;

protected:
  void OpenSocket(void);
  void CloseSocket(void);
  const char *GetIDStr(void)  { return (m_szID); }
  bool SetAddrsFromSocket(unsigned short _nFlags);
  void DumpPacket(CBuffer *b, direction d);
  unsigned long GetIpByName(char *_szHostName);

  int m_nDescriptor;
  struct sockaddr_in m_sRemoteAddr, m_sLocalAddr;
  CBuffer m_xRecvBuffer;
  char m_szID[4];
  int m_nSockType;
  unsigned long m_nOwner;
};


//=====TCPSocket===============================================================
class TCPSocket : public INetSocket
{
public:
  TCPSocket(unsigned long _nOwner) : INetSocket(_nOwner)
    { strcpy(m_szID, "TCP"); m_nSockType = SOCK_STREAM; }
  TCPSocket(void) : INetSocket(0)
    { strcpy(m_szID, "TCP"); m_nSockType = SOCK_STREAM; }

  // Abstract base class overloads
  virtual bool Send(CBuffer *b)
    { return SendPacket(b); }
  virtual bool Recv(void)
    { return RecvPacket(); }

  // Functions specific to TCP
  bool SendPacket(CBuffer *b);
  bool RecvPacket(void);
  void RecvConnection(TCPSocket &newSocket);
  void TransferConnectionFrom(TCPSocket &from);
};


//=====UDPSocket================================================================
class UDPSocket : public INetSocket
{
public:
  UDPSocket(unsigned long _nOwner) : INetSocket(_nOwner)
    { strcpy(m_szID, "UDP"); m_nSockType = SOCK_DGRAM; }

  // Abstract base class overloads
  virtual bool Send(CBuffer *b)
    { return SendRaw(b); }
  virtual bool Recv(void)
    { return RecvRaw(); }
};


//=====CSocketHashTable=========================================================
class CSocketHashTable
{
public:
  CSocketHashTable(unsigned short _nSize);
  INetSocket *Retrieve(int _nSd);
  void Store(INetSocket *s, int _nSd);
  void Remove(int _nSd);

protected:
  unsigned short HashValue(int _nSd);
  vector < list<INetSocket *> > m_vlTable;

  void Lock(unsigned short _nLockType);
  void Unlock(void);

  pthread_rdwr_t mutex_rw;
  unsigned short m_nLockType;
};


/*=====CSocketSet===============================================================
 *
 * This class encapsulates an fd_set used for a call to select().  It is
 * thread-safe as long as the caller calls Lock() and Unlock() properly.
 *----------------------------------------------------------------------------*/
class CSocketSet
{
friend class CSocketManager;

public:
  CSocketSet (void);

  unsigned short Num(void);
  int Largest(void);
  fd_set SocketSet(void);

protected:
  fd_set sFd;
  list <int> lFd;
  void Set (int _nSD);
  void Clear (int _nSD);
  void Lock(void);
  void Unlock(void);

  pthread_mutex_t mutex;
};


//=====CSocketManager===========================================================
class CSocketManager
{
public:
  CSocketManager(void);

  INetSocket *FetchSocket (int _nSd);
  void DropSocket (INetSocket *s);
  void AddSocket(INetSocket *s);
  void CloseSocket (int _nSd, bool _bClearUser = true);

  fd_set SocketSet(void)   {  return m_sSockets.SocketSet(); }
  int LargestSocket(void)  {  return m_sSockets.Largest(); }

protected:
  CSocketSet m_sSockets;
  CSocketHashTable m_hSockets;
};

extern CSocketManager gSocketManager;

#endif
