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

#include "pthread_rdwr.h"

#include "licq_buffer.h"
#include "licq_constants.h"

char *inet_ntoa_r(struct in_addr in, char *buf);
char *ip_ntoa(unsigned long in, char *buf);

//=====INetSocket==================================================================================
class INetSocket
{
public:
  INetSocket(unsigned long _nOwner);
  virtual ~INetSocket();

  bool Connected()          { return(m_nDescriptor > 0);  }
  int Descriptor()          { return(m_nDescriptor);      }
  bool DestinationSet()     { return (RemoteIp() != 0); }
  unsigned long Owner()     { return (m_nOwner); }
  void SetOwner(unsigned long _nOwner)  { m_nOwner = _nOwner; }
  unsigned long Version()     { return (m_nVersion); }
  void SetVersion(unsigned long _nVersion)  { m_nVersion = _nVersion; }

  int Error();
  char *ErrorStr(char *, int);

  unsigned long  LocalIp()     { return (m_sLocalAddr.sin_addr.s_addr);  }
  char *LocalIpStr(char *buf);
  unsigned long  RemoteIp()    { return (m_sRemoteAddr.sin_addr.s_addr); };
  char *RemoteIpStr(char *buf);
  unsigned short LocalPort()   { return (ntohs(m_sLocalAddr.sin_port));  };
  unsigned short RemotePort()  { return (ntohs(m_sRemoteAddr.sin_port)); };

  bool SetRemoteAddr(unsigned long _nRemoteIp, unsigned short _nRemotePort);
  bool SetRemoteAddr(char *_szRemoteName, unsigned short _nRemotePort);

  void ResetSocket();
  void ClearRecvBuffer()  { m_xRecvBuffer.Clear(); };
  bool RecvBufferFull()   { return m_xRecvBuffer.Full(); };
  CBuffer &RecvBuffer()   { return m_xRecvBuffer; };

  bool OpenConnection();
  void CloseConnection()  {  CloseSocket(); }
  bool StartServer(unsigned int _nPort);
  bool SendRaw(CBuffer *b);
  bool RecvRaw();

  virtual bool Send(CBuffer *b) = 0;
  virtual bool Recv() = 0;

  void Lock();
  void Unlock();
  pthread_mutex_t mutex;

  static unsigned long GetIpByName(const char *_szHostName);

protected:
  void OpenSocket();
  void CloseSocket();
  const char *GetIDStr()  { return (m_szID); }
  bool SetAddrsFromSocket(unsigned short _nFlags);
  void DumpPacket(CBuffer *b, direction d);

  int m_nDescriptor;
  struct sockaddr_in m_sRemoteAddr, m_sLocalAddr;
  CBuffer m_xRecvBuffer;
  char m_szID[4];
  int m_nSockType;
  unsigned long m_nOwner;
  unsigned short m_nVersion;
};


//=====TCPSocket===============================================================
class TCPSocket : public INetSocket
{
public:
  TCPSocket(unsigned long _nOwner) : INetSocket(_nOwner)
    { strcpy(m_szID, "TCP"); m_nSockType = SOCK_STREAM; }
  TCPSocket() : INetSocket(0)
    { strcpy(m_szID, "TCP"); m_nSockType = SOCK_STREAM; }

  // Abstract base class overloads
  virtual bool Send(CBuffer *b)
    { return SendPacket(b); }
  virtual bool Recv()
    { return RecvPacket(); }

  // Functions specific to TCP
  bool SendPacket(CBuffer *b);
  bool RecvPacket();
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
  virtual bool Recv()
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
  void Unlock();

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
  CSocketSet ();

  unsigned short Num();
  int Largest();
  fd_set SocketSet();

protected:
  fd_set sFd;
  list <int> lFd;
  void Set (int _nSD);
  void Clear (int _nSD);
  void Lock();
  void Unlock();

  pthread_mutex_t mutex;
};


//=====CSocketManager===========================================================
class CSocketManager
{
public:
  CSocketManager();

  INetSocket *FetchSocket (int _nSd);
  void DropSocket (INetSocket *s);
  void AddSocket(INetSocket *s);
  void CloseSocket (int nSd, bool bClearUser = true, bool bDelete = true);

  fd_set SocketSet()   {  return m_sSockets.SocketSet(); }
  int LargestSocket()  {  return m_sSockets.Largest(); }

protected:
  CSocketSet m_sSockets;
  CSocketHashTable m_hSockets;
};

extern CSocketManager gSocketManager;

#endif
