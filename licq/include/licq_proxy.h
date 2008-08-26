#ifndef PROXY_H
#define PROXY_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <netinet/in.h>

// proxy types
#define PROXY_TYPE_HTTP 1

typedef enum ProxyError_et_
{
  PROXY_ERROR_none,
  PROXY_ERROR_errno,
  PROXY_ERROR_h_errno,
  PROXY_ERROR_internal
} ProxyError_et;

//=====ProxyServer==============================================================
class ProxyServer
{
public:
  ProxyServer();
  virtual ~ProxyServer();

  int Descriptor()  { return (m_nDescriptor); }

  int Error();
  char *ErrorStr(char *buf, int buflen);

  struct sockaddr_in *ProxyAddr()  { return (&m_sProxyAddr); };
  bool SetProxyAddr(const char *_szProxyName, unsigned short _nProxyPort);
  void SetProxyAuth(const char *_szProxyLogin, const char *_szProxyPasswd);

  virtual bool InitProxy() = 0;
  bool OpenConnection();
  void CloseConnection();
  virtual bool OpenProxyConnection(const char *_szRemoteName, unsigned short _nRemotePort) = 0;

  static unsigned long GetIpByName(const char *_szHostName);

protected:
  int m_nDescriptor;
  ProxyError_et m_nErrorType;
  struct sockaddr_in m_sProxyAddr;
  char *m_szProxyLogin, *m_szProxyPasswd;
};

//=====HTTPProxyServer==========================================================
class HTTPProxyServer : public ProxyServer
{
public:
  HTTPProxyServer();
  virtual ~HTTPProxyServer();

  // Abstract base class overload
  virtual bool InitProxy()
    { return HTTPInitProxy(); }
  virtual bool OpenProxyConnection(const char *_szRemoteName, unsigned short _nRemotePort)
    { return HTTPOpenProxyConnection(_szRemoteName, _nRemotePort); }

  // Functions specific to HTTP Proxy
  bool HTTPInitProxy();
  bool HTTPOpenProxyConnection(const char *_szRemoteName, unsigned short _nRemotePort);
};

#endif
