// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998 - 2003 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

/*
    Proxy code written by Sergey Kononenko <sergk@sergk.org.ua>
    using code from gaim (Copyright (C) 1998-1999, Mark Spencer <markster@marko.net>)
    and lftp (Copyright (c) 1999-2001 by Alexander V. Lukyanov (lav@yars.free.net))
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef HAVE_INET_ATON
#include <arpa/inet.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#else
extern int errno;
extern int h_errno;
#endif

// Localization
#include "gettext.h"

#include "licq_proxy.h"
#include "licq_log.h"
#include "support.h"

static int  base64_length(int len);
static void base64_encode(const char *s, char *store, int length);

using namespace std;

//-----ProxyServer::ProxyServer-------------------------------------------------
ProxyServer::ProxyServer()
{
  m_nDescriptor = -1;
  memset(&m_sProxyAddr, 0, sizeof(struct sockaddr_in));
  m_szProxyLogin = NULL;
  m_szProxyPasswd = NULL;
}


//-----ProxyServer::~ProxyServer------------------------------------------------
ProxyServer::~ProxyServer()
{
  CloseConnection();
  free (m_szProxyLogin);
  free (m_szProxyPasswd);
}


//-----ProxyServer::Error-------------------------------------------------------
int ProxyServer::Error()
{
  switch (m_nErrorType)
  {
    case PROXY_ERROR_errno: return errno;
    case PROXY_ERROR_h_errno: return h_errno;
    case PROXY_ERROR_none: return 0;
    case PROXY_ERROR_internal: return -2;
  }
  return 0;
}


//-----ProxyServer::ErrorStr----------------------------------------------------
char *ProxyServer::ErrorStr(char *buf, int buflen)
{
  switch (m_nErrorType)
  {
    case PROXY_ERROR_errno:
      strncpy(buf, strerror(errno), buflen);
      buf[buflen - 1] = '\0';
      break;

    case PROXY_ERROR_h_errno:
#ifndef HAVE_HSTRERROR
      sprintf(buf, tr("proxy hostname resolution failure (%d)"), h_errno);
#else
      strncpy(buf, hstrerror(h_errno), buflen);
      buf[buflen - 1] = '\0';
#endif
    case PROXY_ERROR_none:
      strncpy(buf, tr("No proxy error detected"), buflen);
      buf[buflen - 1] = '\0';
      break;

    case PROXY_ERROR_internal:
      strncpy(buf, tr("Internal proxy error"), buflen);
      buf[buflen - 1] = '\0';
      break;

    default:
      strncpy(buf, tr("Unknown proxy error"), buflen);
      buf[buflen - 1] = '\0';
  }
  
  return buf;
}

		       
//-----ProxyServer::GetIpByName-------------------------------------------------
unsigned long ProxyServer::GetIpByName(const char *_szHostName)
{
  // check if the hostname is in dot and number notation
  struct in_addr ina;
  if (inet_aton(_szHostName, &ina))
     return(ina.s_addr);

  // try and resolve hostname
  struct hostent host;
  char temp[1024];
  h_errno = gethostbyname_r_portable(_szHostName, &host, temp, sizeof(temp));
  if (h_errno == -1) // Couldn't resolve hostname/ip
  {
    return (0);
  }
  else if (h_errno > 0)
  {
    return (0);
  }
  // return the ip
  return ((struct in_addr *)(host.h_addr))->s_addr;
}


//-----ProxyServer::SetProxyAddr------------------------------------------------
bool ProxyServer::SetProxyAddr(const char *_szProxyName, unsigned short _nProxyPort)
{
  unsigned long nProxyIp;
  
  nProxyIp = GetIpByName(_szProxyName);
  if (nProxyIp == 0)
    return(false);

  m_sProxyAddr.sin_port = htons(_nProxyPort);
  m_sProxyAddr.sin_addr.s_addr = nProxyIp;
  return(true);
}


//-----ProxyServer::SetProxyAuth------------------------------------------------
void ProxyServer::SetProxyAuth(const char *_szProxyLogin, const char *_szProxyPasswd)
{
  if (m_szProxyLogin != NULL) free (m_szProxyLogin);
  if (_szProxyLogin != NULL)
    m_szProxyLogin = strdup(_szProxyLogin);
  else
    m_szProxyLogin = NULL;

  if (m_szProxyPasswd != NULL) free (m_szProxyPasswd);
  if (_szProxyPasswd != NULL)
    m_szProxyPasswd = strdup(_szProxyPasswd);
  else
    m_szProxyPasswd = NULL;
}


//-----ProxyServer::OpenConnection----------------------------------------------
bool ProxyServer::OpenConnection()
{
  if(m_sProxyAddr.sin_addr.s_addr == 0)
  {
    m_nErrorType = PROXY_ERROR_internal;
    return(false);
  }

  if (m_nDescriptor != -1) CloseConnection();
  m_nDescriptor = socket(AF_INET, SOCK_STREAM, 0);
  if (m_nDescriptor == -1)
  {
    m_nErrorType = PROXY_ERROR_errno;
    return(false);
  }

#ifdef IP_PORTRANGE
  int i = IP_PORTRANGE_HIGH;
  if (setsockopt(m_nDescriptor, IPPROTO_IP, IP_PORTRANGE, &i, sizeof(i)) < 0)
  {
    m_nErrorType = PROXY_ERROR_errno;
    return(false);
  }
#endif

  m_sProxyAddr.sin_family = AF_INET;

  socklen_t sizeofSockaddr = sizeof(struct sockaddr);
  if (connect(m_nDescriptor, (struct sockaddr *)&m_sProxyAddr, sizeofSockaddr) < 0)
  {
    // errno has been set
    m_nErrorType = PROXY_ERROR_errno;
    CloseConnection();
    return(false);
  }
  return(true);
}


//-----ProxyServer::CloseConnection---------------------------------------------
void ProxyServer::CloseConnection()
{
  if (m_nDescriptor != -1)
  {
    ::shutdown(m_nDescriptor, 2);
    ::close (m_nDescriptor);
    m_nDescriptor = -1;
  }
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


HTTPProxyServer::HTTPProxyServer() : ProxyServer()
{
  // Empty
}

//-----HTTPProxyServer::~HTTPProxyServer----------------------------------------
HTTPProxyServer::~HTTPProxyServer()
{
  // Empty
}


//-----HTTPProxyServer::HTTPInitProxy-------------------------------------------
bool HTTPProxyServer::HTTPInitProxy()
{
  return(true);
}


//-----HTTPProxyServer::HTTPOpenProxyConnection---------------------------------
bool HTTPProxyServer::HTTPOpenProxyConnection(const char *_szRemoteName, unsigned short _nRemotePort)
{
  int nlc = 0;
  int pos = 0;
  int ver_major, ver_minor, status_consumed;
  int status_code = 0;
  char input_line[HTTP_INPUT_LINE_MAX];
  char cmd[HTTP_CMD_LINE_MAX];

  cmd[sizeof(cmd) - 1] = '\0';

  snprintf(cmd, sizeof(cmd) - 1, "CONNECT %s:%d HTTP/1.1\r\nHost: %s:%d\r\n", 
		_szRemoteName, _nRemotePort, _szRemoteName, _nRemotePort);
  if (send(m_nDescriptor, cmd, strlen(cmd), 0) < 0)
  {
    m_nErrorType = PROXY_ERROR_errno;
    CloseConnection();
    return(false);
  }
  
  if (m_szProxyLogin && m_szProxyPasswd)
  {
    char *cmd_b = new char[strlen(m_szProxyLogin) + 1 + strlen(m_szProxyPasswd) + 1];
    sprintf(cmd_b, "%s:%s", m_szProxyLogin, m_szProxyPasswd);
    char *cmd_b64 = new char[base64_length(strlen(cmd_b)) + 1];
    base64_encode(cmd_b, cmd_b64, strlen(cmd_b));
    snprintf(cmd, sizeof(cmd) - 1, "Proxy-Authorization: Basic %s\r\n", cmd_b64);
    delete cmd_b;
    delete cmd_b64;
    if (send(m_nDescriptor, cmd, strlen(cmd), 0) < 0)
    {
      m_nErrorType = PROXY_ERROR_errno;
      CloseConnection();
      return(false);
    }
  }
  
  snprintf(cmd, sizeof(cmd) - 1, "\r\n");
  if (send(m_nDescriptor, cmd, strlen(cmd), 0) < 0)
  {
    m_nErrorType = PROXY_ERROR_errno;
    CloseConnection();
    return(false);
  }

  while ((nlc != 2) && (read(m_nDescriptor, &input_line[pos++], 1) == 1))
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
    gLog.Warn(tr("%sCould not parse HTTP status line from proxy\n"), L_ERRORxSTR);
    m_nErrorType = PROXY_ERROR_internal;
    CloseConnection();
    return(false);
  }
  if (status_code == HTTP_STATUS_OK)
    return (true);

  gLog.Warn(tr("%sHTTPS proxy return error code: %d, error string:\n%s\n"),
	      L_ERRORxSTR, status_code, input_line);
  m_nErrorType = PROXY_ERROR_internal;
  CloseConnection();
  return(false);
}
