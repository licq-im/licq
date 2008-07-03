#ifndef socket_h
#define socket_h

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <arpa/inet.h>
#include <netinet/in.h>
#include <vector>
#include <list>

#include "pthread_rdwr.h"

#include "licq_buffer.h"
#include "licq_constants.h"

class ProxyServer;

char *inet_ntoa_r(struct in_addr in, char *buf);
char *ip_ntoa(unsigned long in, char *buf);

typedef enum SocketError_et_
{
  SOCK_ERROR_none,
  SOCK_ERROR_errno,
  SOCK_ERROR_h_errno,
  SOCK_ERROR_desx,
  SOCK_ERROR_internal,
  SOCK_ERROR_proxy
} SocketError_et;


//=====INetSocket==================================================================================
class INetSocket
{
public:
  INetSocket(const char *_szOwnerId, unsigned long _nOwnerPPID);
  virtual ~INetSocket();

  bool Connected()          { return(m_nDescriptor > 0);  }
  int Descriptor()          { return(m_nDescriptor);      }
  bool DestinationSet()     { return (RemoteIp() != 0); }
  char *OwnerId()           { return m_szOwnerId; }
  unsigned long OwnerPPID() { return m_nOwnerPPID; }
  void SetOwner(const char *s, unsigned long n);
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
  bool SetRemoteAddr(const char *_szRemoteName, unsigned short _nRemotePort);

  void SetProxy(ProxyServer *_xProxy) { m_xProxy = _xProxy; };

  void ResetSocket();
  void ClearRecvBuffer()  { m_xRecvBuffer.Clear(); };
  bool RecvBufferFull()   { return m_xRecvBuffer.Full(); };
  CBuffer &RecvBuffer()   { return m_xRecvBuffer; };

  bool OpenConnection();
  void CloseConnection();
  bool StartServer(unsigned int _nPort);
  bool SendRaw(CBuffer *b);
  bool RecvRaw();

  virtual bool Send(CBuffer *b) = 0;
  virtual bool Recv() = 0;

  void Lock();
  void Unlock();
  pthread_mutex_t mutex;

  static unsigned long GetIpByName(const char *_szHostName);

  void SetChannel(unsigned char nChannel) { m_nChannel = nChannel; }
  unsigned char Channel()                 { return m_nChannel; }

protected:
  const char *GetIDStr()  { return (m_szID); }
  bool SetLocalAddress(bool bIp = true);
  void DumpPacket(CBuffer *b, direction d);

  int m_nDescriptor;
  struct sockaddr_in m_sRemoteAddr, m_sLocalAddr;
  char *m_szRemoteName;
  CBuffer m_xRecvBuffer;
  char m_szID[4];
  int m_nSockType;
  unsigned short m_nVersion;
  SocketError_et m_nErrorType;
  ProxyServer *m_xProxy;
  char *m_szOwnerId;
  unsigned long m_nOwnerPPID;
  unsigned char m_nChannel;
};


//=====TCPSocket===============================================================
class TCPSocket : public INetSocket
{
public:
  TCPSocket(const char *s, unsigned long n);
  TCPSocket();
  virtual ~TCPSocket();

  // Abstract base class overloads
  virtual bool Send(CBuffer *b)
    { return SendPacket(b); }
  virtual bool Recv()
    { return RecvPacket(); }

  // Functions specific to TCP
  bool SendPacket(CBuffer *b);
  bool RecvPacket();
  bool RecvConnection(TCPSocket &newSocket);
  void TransferConnectionFrom(TCPSocket &from);

  bool SSLSend(CBuffer *b);
  bool SSLRecv();

  bool Secure() { return m_p_SSL != NULL; }
  bool SSL_Pending();

  bool SecureConnect();
  bool SecureListen();
  void SecureStop();

protected:
  void* m_p_SSL;
  pthread_mutex_t mutex_ssl;
};


//=====SrvSocket===============================================================
class SrvSocket : public INetSocket
{
public:
  SrvSocket(const char *s, unsigned long n);
  virtual ~SrvSocket();

  // Abstract base class overloads
  virtual bool Send(CBuffer *b)
    { return SendPacket(b); }
  virtual bool Recv()
    { return RecvPacket(); }

  // Functions specific to Server TCP communication
  bool SendPacket(CBuffer *b);
  bool RecvPacket();
  bool ConnectTo(const char* server, unsigned short port, ProxyServer *xProxy);
};


//=====UDPSocket================================================================
class UDPSocket : public INetSocket
{
public:
  UDPSocket(const char* ownerId, unsigned long ownerPpid);
  virtual ~UDPSocket();

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
  ~CSocketHashTable();

  INetSocket *Retrieve(int _nSd);
  void Store(INetSocket *s, int _nSd);
  void Remove(int _nSd);

protected:
  unsigned short HashValue(int _nSd);
  std::vector < std::list<INetSocket *> > m_vlTable;

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
  CSocketSet();
  ~CSocketSet();

  unsigned short Num();
  int Largest();
  fd_set SocketSet();

protected:
  fd_set sFd;
  std::list <int> lFd;
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
  virtual ~CSocketManager();

  INetSocket *FetchSocket (int _nSd);
  void DropSocket (INetSocket *s);
  void AddSocket(INetSocket *s);
  void CloseSocket (int nSd, bool bClearUser = true, bool bDelete = true);

  fd_set SocketSet()   {  return m_sSockets.SocketSet(); }
  int LargestSocket()  {  return m_sSockets.Largest(); }
  unsigned short Num() {  return m_sSockets.Num(); }

protected:
  CSocketSet m_sSockets;
  CSocketHashTable m_hSockets;
  pthread_mutex_t mutex;
};

extern CSocketManager gSocketManager;

#endif
