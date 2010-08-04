// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998-2010 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

/*
    Proxy code written by Sergey Kononenko <sergk@sergk.org.ua>
    using code from gaim (Copyright (C) 1998-1999, Mark Spencer <markster@marko.net>)
    and lftp (Copyright (c) 1999-2001 by Alexander V. Lukyanov (lav@yars.free.net))
*/

#include "config.h"

#include <licq/proxy.h>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <licq/logging/log.h>
#include <licq/socket.h>

#include "gettext.h"

static int  base64_length(int len);
static void base64_encode(const char *s, char *store, int length);

using namespace std;
using Licq::Proxy;
using Licq::HttpProxy;


Proxy::Proxy()
{
  memset(&myProxyAddr, 0, sizeof(myProxyAddrStorage));
  myErrorType = ErrorNone;
}

Proxy::~Proxy()
{
  // Empty
}

int Proxy::error() const
{
  switch (myErrorType)
  {
    case ErrorErrno:
      return errno;
    case ErrorNone:
      return 0;
    case ErrorInternal:
      return -2;
  }
  return 0;
}

string Proxy::errorStr() const
{
  switch (myErrorType)
  {
    case ErrorErrno:
      return strerror(errno);

    case ErrorNone:
      return tr("No proxy error detected");

    case ErrorInternal:
      return tr("Internal proxy error");

    default:
      return tr("Unknown proxy error");
  }
}

void Proxy::setProxyAddr(const string& proxyName, int proxyPort)
{
  myProxyName = proxyName;
  myProxyPort = proxyPort;
}

void Proxy::setProxyAuth(const string& proxyLogin, const string& proxyPasswd)
{
  myProxyLogin = proxyLogin;
  myProxyPasswd = proxyPasswd;
}

int Proxy::openConnection(const string& remoteName, int remotePort)
{
  if (myProxyName.empty())
  {
    myErrorType = ErrorInternal;
    return -1;
  }

  int sock = INetSocket::connectDirect(myProxyName, myProxyPort, SOCK_STREAM, &myProxyAddr);

  if (sock == -1)
  {
    myErrorType = ErrorErrno;
    return -1;
  }

  // Connection to proxy established, invoke sub class to instruct proxy what to do
  return openProxyConnection(sock, remoteName, remotePort);
}


//-----HTTPProxyServer----------------------------------------------------------
#define	HTTP_INPUT_LINE_MAX	8192
#define	HTTP_CMD_LINE_MAX	384
#define HTTP_STATUS_OK		200

//----- base64 -----------------------------------------------------------------
static int base64_length(int len)
{
  return (4 * (((len) + 2) / 3));
}

static void base64_encode (const char *s, char *store, int length)
{
  /* Conversion table.  */
  static char tbl[64] = {
    'A','B','C','D','E','F','G','H',
    'I','J','K','L','M','N','O','P',
    'Q','R','S','T','U','V','W','X',
    'Y','Z','a','b','c','d','e','f',
    'g','h','i','j','k','l','m','n',
    'o','p','q','r','s','t','u','v',
    'w','x','y','z','0','1','2','3',
    '4','5','6','7','8','9','+','/'
  };
  int i;
  unsigned char *p = (unsigned char *)store;

  /* Transform the 3x8 bits to 4x6 bits, as required by base64.  */
  for (i = 0; i < length; i += 3)
  {
    *p++ = tbl[s[0] >> 2];
    *p++ = tbl[((s[0] & 3) << 4) + (s[1] >> 4)];
    *p++ = tbl[((s[1] & 0xf) << 2) + (s[2] >> 6)];
    *p++ = tbl[s[2] & 0x3f];
    s += 3;
  }
  /* Pad the result if necessary...  */
  if (i == length + 1)
    *(p - 1) = '=';
  else if (i == length + 2)
    *(p - 1) = *(p - 2) = '=';
  /* ...and zero-terminate it.  */
  *p = '\0';
}


HttpProxy::HttpProxy()
  : Proxy()
{
  // Empty
}

HttpProxy::~HttpProxy()
{
  // Empty
}

bool HttpProxy::initProxy()
{
  return true;
}

int HttpProxy::openProxyConnection(int sock, const string& remoteName, int remotePort)
{
  int nlc = 0;
  int pos = 0;
  int ver_major, ver_minor, status_consumed;
  int status_code = 0;
  char input_line[HTTP_INPUT_LINE_MAX];
  char cmd[HTTP_CMD_LINE_MAX];

  cmd[sizeof(cmd) - 1] = '\0';

  snprintf(cmd, sizeof(cmd) - 1, "CONNECT %s:%d HTTP/1.1\r\nHost: %s:%d\r\n", 
      remoteName.c_str(), remotePort, remoteName.c_str(), remotePort);
  if (send(sock, cmd, strlen(cmd), 0) < 0)
  {
    myErrorType = ErrorErrno;
    close(sock);
    return -1;
  }

  if (!myProxyLogin.empty() && !myProxyPasswd.empty())
  {
    string auth = myProxyLogin + ":" + myProxyPasswd;
    char* cmd_b64 = new char[base64_length(auth.size()) + 1];
    base64_encode(auth.c_str(), cmd_b64, auth.size());
    snprintf(cmd, sizeof(cmd) - 1, "Proxy-Authorization: Basic %s\r\n", cmd_b64);
    delete [] cmd_b64;
    if (send(sock, cmd, strlen(cmd), 0) < 0)
    {
      myErrorType = ErrorErrno;
      close(sock);
      return -1;
    }
  }
  
  snprintf(cmd, sizeof(cmd) - 1, "\r\n");
  if (send(sock, cmd, strlen(cmd), 0) < 0)
  {
    myErrorType = ErrorErrno;
    close(sock);
    return -1;
  }

  while ((nlc != 2) && (read(sock, &input_line[pos++], 1) == 1))
  {
    if (input_line[pos - 1] == '\n')
      nlc++;
    else if (input_line[pos - 1] != '\r')
      nlc = 0;
  }
  input_line[pos] = '\0';
  
  if (sscanf(input_line, "HTTP/%d.%d %n%d", &ver_major, &ver_minor,
	     &status_consumed, &status_code) != 3)
  {
    gLog.warning(tr("%sCould not parse HTTP status line from proxy\n"), L_ERRORxSTR);
    myErrorType = ErrorInternal;
    close(sock);
    return -1;
  }
  if (status_code == HTTP_STATUS_OK)
    return sock;

  gLog.warning(tr("%sHTTPS proxy return error code: %d, error string:\n%s\n"),
	      L_ERRORxSTR, status_code, input_line);
  myErrorType = ErrorInternal;
  close(sock);
  return -1;
}
