/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Erik Johansson <erijo@licq.org>
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
#include <string>

class CUserEvent;
class LicqEvent;
class LicqProtoSignal;
class LicqSignal;
class ProxyServer;
class TCPSocket;

namespace Licq
{

class LogService;
class User;
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

  virtual LogService& getLogService() = 0;

  // Firewall options
  bool tcpEnabled() const                       { return myTcpEnabled; }
  void setTcpEnabled(bool b);
  bool behindFirewall() const                   { return myBehindFirewall; }
  void setBehindFirewall(bool b);
  unsigned tcpPortsLow() const                  { return myTcpPortsLow; }
  unsigned tcpPortsHigh() const                 { return myTcpPortsHigh; }
  void setTcpPorts(unsigned lowPort, unsigned highPort);

  // Proxy options
  ProxyServer* createProxy();
  bool proxyEnabled() const                     { return myProxyEnabled; }
  void setProxyEnabled(bool b)                  { myProxyEnabled = b; }
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

  int StartTCPServer(TCPSocket *);

  virtual bool addUserEvent(User* u, CUserEvent* e) = 0;
  virtual void rejectEvent(const UserId& userId, CUserEvent* e) = 0;

  /**
   * Add a signal to the signal queues of all plugins.
   *
   * @param signal Signal to send
   */
  void pushPluginSignal(LicqSignal* signal);

  void PushPluginEvent(LicqEvent *);
  void PushProtoSignal(LicqProtoSignal* s, unsigned long ppid);

  void pluginUIViewEvent(const Licq::UserId& userId);

  void pluginUIMessage(const Licq::UserId& userId);

  /**
   * Get the next queued signal for a plugin
   * Checks calling thread to determine which plugin queue to pop
   *
   * @return The next queued signal or NULL if the queue is empty
   */
  LicqSignal* popPluginSignal();
  LicqEvent *PopPluginEvent();
  LicqProtoSignal* PopProtoSignal();

protected:
  ~Daemon() { /* Empty */ }

  bool myShuttingDown;
  std::string myTerminal;

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
};

extern Daemon& gDaemon;

} // namespace Licq

#endif
