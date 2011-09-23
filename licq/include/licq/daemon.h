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

#ifndef LICQ_DAEMON_H
#define LICQ_DAEMON_H

#include <boost/noncopyable.hpp>
#include <pthread.h>
#include <string>

namespace Licq
{

class Event;
class PluginSignal;
class ProtocolSignal;
class Proxy;
class TCPSocket;
class User;
class UserEvent;
class UserId;


class Daemon : private boost::noncopyable
{
public:
  virtual pthread_t* Shutdown() = 0;
  virtual const char* Version() const = 0;
  virtual void SaveConf() = 0;

  bool shuttingDown() const                     { return myShuttingDown; }

  /**
   * Check if GPG support is enabled
   * This function allows plugins to check at runtime if GPG options are available
   *
   * @return True if GPG support is available in daemon
   */
  bool haveGpgSupport() const;

  /**
   * Check if SSL support is enabled
   * This function allows plugins to check at runtime if encryption is available
   *
   * @return True if SSL support is available in daemon
   */
  bool haveCryptoSupport() const;

  // Firewall options
  bool tcpEnabled() const                       { return myTcpEnabled; }
  void setTcpEnabled(bool b);
  bool behindFirewall() const                   { return myBehindFirewall; }
  void setBehindFirewall(bool b);
  unsigned tcpPortsLow() const                  { return myTcpPortsLow; }
  unsigned tcpPortsHigh() const                 { return myTcpPortsHigh; }
  void setTcpPorts(unsigned lowPort, unsigned highPort);

  // Proxy options
  Proxy* createProxy();
  bool proxyEnabled() const                     { return myProxyEnabled; }
  void setProxyEnabled(bool b)                  { myProxyEnabled = b; }
  enum ProxyTypes
  {
    ProxyTypeHttp = 1,
  };
  unsigned proxyType() const                    { return myProxyType; }
  void setProxyType(unsigned t)                 { myProxyType = t; }
  const std::string& proxyHost() const          { return myProxyHost; }
  void setProxyHost(const std::string& s)       { myProxyHost = s; }
  unsigned proxyPort() const                    { return myProxyPort; }
  void setProxyPort(unsigned short p)           { myProxyPort = p; }
  bool proxyAuthEnabled() const                 { return myProxyAuthEnabled; }
  void setProxyAuthEnabled(bool b)              { myProxyAuthEnabled = b; }
  const std::string& proxyLogin() const         { return myProxyLogin; }
  void setProxyLogin(const std::string& s)      { myProxyLogin = s; }
  const std::string& proxyPasswd() const        { return myProxyPasswd; }
  void setProxyPasswd(const std::string& s)     { myProxyPasswd = s; }

  const std::string& terminal() const           { return myTerminal; }
  void setTerminal(const std::string& s)        { myTerminal = s; }
  bool sendTypingNotification() const           { return mySendTypingNotification; }
  void setSendTypingNotification(bool b)        { mySendTypingNotification = b; }

  int StartTCPServer(TCPSocket *);

  virtual bool addUserEvent(User* u, UserEvent* e) = 0;
  virtual void rejectEvent(const UserId& userId, UserEvent* e) = 0;

  void cancelEvent(unsigned long eventId);
  void cancelEvent(Event* event);

  enum IgnoreTypes
  {
    IgnoreMassMsg = 1,
    IgnoreNewUsers = 2,
    IgnoreEmailPager = 4,
    IgnoreWebPanel = 8,
  };
  bool ignoreType(unsigned type) const          { return (myIgnoreTypes & type); }
  void setIgnoreType(unsigned type, bool ignore);

  void pluginUIViewEvent(const UserId& userId);

  void pluginUIMessage(const UserId& userId);

  /**
   * Get path for the base dir (e.g. /home/fred/.licq/)
   *
   * @return Full path for base dir, ending with a slash
   */
  const std::string& baseDir() const { return myBaseDir; }

  /**
   * Get path for plugin libraries (e.g. /usr/local/lib/licq/)
   *
   * @return Full path for library dir, ending with a slash
   */
  const std::string& libDir() const { return myLibDir; }

  /**
   * Get path for data files (e.g. /usr/local/share/licq/)
   *
   * @return Full path for share dir, ending with a slash
   */
  const std::string& shareDir() const { return myShareDir; }

protected:
  virtual ~Daemon() { /* Empty */ }

  bool myShuttingDown;
  std::string myTerminal;
  bool mySendTypingNotification;
  unsigned myIgnoreTypes;

  // Firewall
  bool myTcpEnabled;
  bool myBehindFirewall;
  unsigned myTcpPortsLow;
  unsigned myTcpPortsHigh;

  // Proxy
  bool myProxyEnabled;
  unsigned myProxyType;
  std::string myProxyHost;
  unsigned myProxyPort;
  bool myProxyAuthEnabled;
  std::string myProxyLogin;
  std::string myProxyPasswd;

  // Paths
  std::string myBaseDir;
  std::string myLibDir;
  std::string myShareDir;
};

extern Daemon& gDaemon;

} // namespace Licq

#endif
