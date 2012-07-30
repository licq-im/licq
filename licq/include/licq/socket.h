/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2012 Licq developers <licq-dev@googlegroups.com>
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

#ifndef LICQ_SOCKET_H
#define LICQ_SOCKET_H

#include <stdint.h>
#include <string>
#include <sys/socket.h> // AF_UNSPEC, struct sockaddr

#include "thread/mutex.h"
#include "userid.h"


// IPv6 is implemented in socket classes but Licq support must be considered experimental for now
// Uncomment the following line to disable IPv6 socket usage
//#define LICQ_DISABLE_IPV6


namespace Licq
{
class Buffer;
class Proxy;


/**
 * Convert an IPv4 address to readable text
 *
 * @param in IP address
 * @param buf Buffer for write text to, assumed to be able to hold at least 32 bytes
 * @return buf
 */
char *ip_ntoa(unsigned long in, char *buf);

//=====INetSocket==================================================================================
class INetSocket
{
public:
  static const size_t MAX_RECV_SIZE = 4096;

  INetSocket(int sockType, const std::string& logId, const UserId& userId);
  virtual ~INetSocket();

  bool Connected() const { return (myDescriptor > 0);  }
  int Descriptor() const { return myDescriptor; }
  bool DestinationSet()     { return myRemoteAddr.sa_family != AF_UNSPEC; }
  const UserId& userId() const { return myUserId; }
  void setUserId(const UserId& userId) { myUserId = userId; }

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

  /**
   * Connect to a remote host
   *
   * @param remoteAddr Address of remote host
   * @param remotePort Port to connect to
   * @param proxy Proxy connection to use or NULL for direct connect
   * @return True if connection was opened successfully
   */
  bool connectTo(const std::string& remoteAddr, uint16_t remotePort,
      Proxy* proxy = NULL);

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
      Proxy* proxy = NULL);

  void CloseConnection();
  bool StartServer(unsigned int _nPort);

  /**
   * Send one "packet"
   * Writes the contents of a buffer to the socket
   *
   * @param b Buffer with packet to send
   * @return False on any failure
   */
  virtual bool send(Buffer& b);

  /**
   * Receive one "packet"
   * Makes a single read from the socket
   *
   * @param b Buffer to store data in (will be created if empty)
   * @param maxlength Maximum packet length to read
   * @param dump True to dump packet for debugging if buffer was filled
   * @return False on any failure otherwise true regardless of data length
   */
  virtual bool receive(Buffer& b, size_t maxlength = MAX_RECV_SIZE, bool dump = true);

  void Lock();
  void Unlock();

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
   * Get the IPv4 address as an unsigned int from an IP address
   * Note: This function is not usable with IPv6 addresses
   *
   * @param ip A string with the IPv4 IP to extract
   * @return The ip from the string in network endian form
   */
  static uint32_t ipToInt(const std::string& ip);

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

protected:
  enum ErrorType
  {
    ErrorNone           = 0,
    ErrorErrno          = 1,
    ErrorInternal       = 2,
    ErrorProxy          = 3,
  };

  bool SetLocalAddress(bool bIp = true);
  void DumpPacket(Buffer* b, bool isReceiver);

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

  int myDescriptor;
  std::string myRemoteName;
  std::string myLogId;
  int mySockType;
  ErrorType myErrorType;
  Proxy* myProxy;
  UserId myUserId;
  Mutex myMutex;
};


class TCPSocket : public INetSocket
{
public:
  TCPSocket(const UserId& userId);
  TCPSocket();
  virtual ~TCPSocket();

  bool RecvConnection(TCPSocket &newSocket);
  virtual void TransferConnectionFrom(TCPSocket &from);

  /// Overloaded to add SSL support
  bool send(Buffer& b);

  /// Overloaded to add SSL support
  bool receive(Buffer& b, size_t maxlength = MAX_RECV_SIZE, bool dump = true);

  bool Secure() { return m_p_SSL != NULL; }
  bool SSL_Pending();

  bool SecureConnect();
  bool SecureListen();
  void SecureStop();

protected:
  void* m_p_SSL;
  pthread_mutex_t mutex_ssl;
};


//=====UDPSocket================================================================
class UDPSocket : public INetSocket
{
public:
  UDPSocket(const UserId& userId);
  virtual ~UDPSocket();
};

} // namespace Licq

#endif
