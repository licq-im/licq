#ifndef socket_h
#define socket_h

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <string>

#include <licq/buffer.h>
#include "licq_constants.h"
#include "licq/userid.h"

namespace Licq
{
class Proxy;
}

// Temporary to keep old code working until this header has also been ported
typedef Licq::Buffer CBuffer;


// IPv6 is implemented in socket classes but Licq support must be considered experimental for now
// Uncomment the following line to disable IPv6 socket usage
//#define LICQ_DISABLE_IPV6


/**
 * Convert an IPv4 address to readable text
 *
 * @param in IP address
 * @param buf Buffer for write text to, assumed to be able to hold at least 32 bytes
 * @return buf
 */
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
  INetSocket(const Licq::UserId& userId);
  virtual ~INetSocket();

  bool Connected()          { return(m_nDescriptor > 0);  }
  int Descriptor()          { return(m_nDescriptor);      }
  bool DestinationSet()     { return myRemoteAddr.sa_family != AF_UNSPEC; }
  const Licq::UserId& userId() const { return myUserId; }
  void setUserId(const Licq::UserId& userId) { myUserId = userId; }
  unsigned long Version()     { return (m_nVersion); }
  void SetVersion(unsigned long _nVersion)  { m_nVersion = _nVersion; }

  int Error();
  std::string errorStr() const;

  /**
   * Get IP address of local endpoint as a readable string
   *
   * @return Local IP address
   */
  std::string getLocalIpString() const  { return addrToString(&myLocalAddr); }

  /**
   * Get IP address of remote endpoint as a readable string
   *
   * @return Remote IP address
   */
  std::string getRemoteIpString() const { return addrToString(&myRemoteAddr); }

  /**
   * Get IP address of local endpoint as an unsigned int
   * Note: This function is not usable with IPv6 sockets
   *
   * @return Local IP address in network endian form
   */
  uint32_t getLocalIpInt() const        { return addrToInt(&myLocalAddr); }

  /**
   * Get IP address of remote endpoint as an unsigned int
   * Note: This function is not usable with IPv6 sockets
   *
   * @return Remote IP address in network endian form
   */
  uint32_t getRemoteIpInt() const       { return addrToInt(&myRemoteAddr); }

  /**
   * Get local port number
   *
   * @return Local port number in host endian form
   */
  uint16_t getLocalPort() const         { return getAddrPort(&myLocalAddr); }

  /**
   * Get remote port number
   *
   * @return Remote port number in host endian form
   */
  uint16_t getRemotePort() const        { return getAddrPort(&myRemoteAddr); };

  void ResetSocket();
  void ClearRecvBuffer()  { m_xRecvBuffer.Clear(); };
  bool RecvBufferFull()   { return m_xRecvBuffer.Full(); };
  CBuffer &RecvBuffer()   { return m_xRecvBuffer; };

  /**
   * Connect to a remote host
   *
   * @param remoteAddr Address of remote host
   * @param remotePort Port to connect to
   * @param proxy Proxy connection to use or NULL for direct connect
   * @return True if connection was opened successfully
   */
  bool connectTo(const std::string& remoteAddr, uint16_t remotePort,
      Licq::Proxy* proxy = NULL);

  /**
   * Connect to a remote IP as unsigned int
   * Note: This function is not usable with IPv6
   *
   * @param remoteAddr Address of remote host
   * @param remotePort Port to connect to
   * @param proxy Proxy connection to use or NULL for direct connect
   * @return True if connection was opened successfully
   */
  bool connectTo(uint32_t remoteAddr, uint16_t remotePort,
      Licq::Proxy* proxy = NULL);

  void CloseConnection();
  bool StartServer(unsigned int _nPort);
  bool SendRaw(CBuffer *b);
  bool RecvRaw();

  virtual bool Send(CBuffer *b) = 0;
  virtual bool Recv() = 0;

  void Lock();
  void Unlock();
  pthread_mutex_t mutex;

  /**
   * Convert an address to readable string form
   *
   * @param addr A sockaddr_in or sockaddr_in6 with the address to convert
   * @return The address in text form
   */
  static std::string addrToString(const struct sockaddr* addr);

  /**
   * Get the IPv4 address as an unsigned int from an address
   * Note: This function is not usable with IPv6 addresses
   *
   * @param addr A sockaddr_in with the address to extract
   * @return The address from the struct in network endian form
   */
  static uint32_t addrToInt(const struct sockaddr* addr);

  /**
   * Get the port from an address
   *
   * @param addr A sockaddr_in or sockaddr_in6 with the port to extract
   * @return The port from the struct
   */
  static uint16_t getAddrPort(const struct sockaddr* addr);

  /**
   * Connect to a remote host without actually using a INetSocket object
   *
   * @param remoteName Host to connect to
   * @param prometPort Port to connect to
   * @param sockType Socket type
   * @param remoteAddr Area to store remote address after resolving
   * @return A socket descriptor if successfull, -1 if failed
   */
  static int connectDirect(const std::string& remoteName, uint16_t remotePort, uint16_t sockType, struct sockaddr* remoteAddr);

  void SetChannel(unsigned char nChannel) { m_nChannel = nChannel; }
  unsigned char Channel()                 { return m_nChannel; }

protected:
  const char *GetIDStr()  { return (m_szID); }
  bool SetLocalAddress(bool bIp = true);
  void DumpPacket(CBuffer *b, direction d);

  // sockaddr is too small to hold a sockaddr_in6 so use union to allocate the extra space
  union
  {
    struct sockaddr myLocalAddr;
    struct sockaddr_storage myLocalAddrStorage;
  };
  union
  {
    struct sockaddr myRemoteAddr;
    struct sockaddr_storage myRemoteAddrStorage;
  };

  int m_nDescriptor;
  std::string myRemoteName;
  CBuffer m_xRecvBuffer;
  char m_szID[4];
  int m_nSockType;
  unsigned short m_nVersion;
  SocketError_et m_nErrorType;
  Licq::Proxy* myProxy;
  Licq::UserId myUserId;
  unsigned char m_nChannel;
};


//=====TCPSocket===============================================================
class TCPSocket : public INetSocket
{
public:
  TCPSocket(const Licq::UserId& userId);
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
  SrvSocket(const Licq::UserId& userId);
  virtual ~SrvSocket();

  // Abstract base class overloads
  virtual bool Send(CBuffer *b)
    { return SendPacket(b); }
  virtual bool Recv()
    { return RecvPacket(); }

  // Functions specific to Server TCP communication
  bool SendPacket(CBuffer *b);
  bool RecvPacket();
};


//=====UDPSocket================================================================
class UDPSocket : public INetSocket
{
public:
  UDPSocket(const Licq::UserId& userId);
  virtual ~UDPSocket();

  // Abstract base class overloads
  virtual bool Send(CBuffer *b)
    { return SendRaw(b); }
  virtual bool Recv()
    { return RecvRaw(); }
};

#endif
