/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2011 Licq developers
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

#ifndef LICQ_PROXY_H
#define LICQ_PROXY_H

#include <string>
#include <sys/socket.h>


namespace Licq
{

class Proxy
{
public:
  Proxy();
  virtual ~Proxy();

  int error() const;
  std::string errorStr() const;

  const struct sockaddr* proxyAddr() const { return &myProxyAddr; };
  void setProxyAddr(const std::string& proxyName, int proxyPort);
  void setProxyAuth(const std::string& proxyLogin, const std::string& proxyPasswd);

  virtual bool initProxy() = 0;

  /**
   * Open a connection through proxy
   *
   * @param remoteName Name of host to connect to
   * @param remotePort Port to connect to
   * @return Socket connected to remote host through proxy or -1 on failure
   */
  int openConnection(const std::string& remoteName, int remotePort);

protected:
  /**
   * Internal call to sub class when connecting
   *
   * @param sock A socket connected to the proxy
   * @param remoteName Name of host to connect to
   * @param remotePort Port to connect to
   * @return Socket connected through proxy or -1 on failure (with sock closed)
   */
  virtual int openProxyConnection(int sock, const std::string& remoteName, int remotePort) = 0;

  // sockaddr is too small to hold a sockaddr_in6 so use union to allocate the extra space
  union
  {
    struct sockaddr myProxyAddr;
    struct sockaddr_storage myProxyAddrStorage;
  };

  enum ErrorType
  {
    ErrorNone           = 0,
    ErrorErrno          = 1,
    ErrorInternal       = 2,
  };

  ErrorType myErrorType;
  std::string myProxyName;
  int myProxyPort;
  std::string myProxyLogin;
  std::string myProxyPasswd;
};


class HttpProxy : public Proxy
{
public:
  HttpProxy();
  virtual ~HttpProxy();

  bool initProxy();

protected:
  int openProxyConnection(int sock, const std::string& remoteName, int remotePort);
};

} // namespace Licq

#endif
