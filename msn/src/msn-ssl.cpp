/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// written by Jon Keating <jon@licq.org>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "msn.h"
#include "msnpacket.h"
#include "licq_log.h"
#include "licq_message.h"

#include <openssl/md5.h>

#include <cstring>
#include <string>
#include <list>
#include <vector>

using namespace std;

void CMSN::ProcessSSLServerPacket(CMSNBuffer &packet)
{
  // Did we receive the entire packet?
  // I don't like doing this, is there a better way to see
  // if we get the entire packet at the socket level?
  // I couldn't find anything in the HTTP RFC about this packet
  // being broken up without any special HTTP headers
  if (m_pSSLPacket == 0)
    m_pSSLPacket = new CMSNBuffer(packet);

  char *ptr = packet.getDataStart() + packet.getDataSize() - 4;
  int x = memcmp(ptr, "\x0D\x0A\x0D\x0A", 4);
  if (m_pSSLPacket->getDataSize() != packet.getDataSize())
    *m_pSSLPacket += packet;
  
  if (x)  return;
  
  // Now process the packet
  char cTmp = 0;
  string strFirstLine = "";
  
  *m_pSSLPacket >> cTmp;
  while (cTmp != '\r')
  {
    strFirstLine += cTmp;
    *m_pSSLPacket >> cTmp;
  }
  
  *m_pSSLPacket >> cTmp; // skip \n as well
  
  // Authenticated
  if (strFirstLine == "HTTP/1.1 200 OK")
  {
    m_pSSLPacket->ParseHeaders();
    char *fromPP = strstr(m_pSSLPacket->GetValue("Authentication-Info").c_str(), "from-PP=");
    char *tag;

    if (fromPP == 0)
      tag = m_szCookie;
    else
    {
      fromPP+= 9; // skip to the tag
      char *endTag = strchr(fromPP, '\'');
      tag = strndup(fromPP, endTag - fromPP); // Thanks, this is all we need
    }

    CMSNPacket *pReply = new CPS_MSNSendTicket(tag);
    SendPacket(pReply);
    free(tag);
    m_szCookie = 0;
  }
  else if (strFirstLine == "HTTP/1.1 302 Found")
  {
    m_pSSLPacket->ParseHeaders();
    string strAuthHeader = m_pSSLPacket->GetValue("WWW-Authenticate");
    string strToSend = strAuthHeader.substr(strAuthHeader.find(" ") + 1, strAuthHeader.size() - strAuthHeader.find(" "));

    string strLocation = m_pSSLPacket->GetValue("Location");
    string::size_type pos = strLocation.find("/", 9);
    if (pos != string::npos)
    {
      string strHost = strLocation.substr(8, pos - 8);
      string strParam = strLocation.substr(pos, strLocation.size() - pos);
      gSocketMan.CloseSocket(m_nSSLSocket, false, true);
      m_nSSLSocket = -1;
      delete m_pSSLPacket;
      m_pSSLPacket = 0;

      gLog.Info("%sRedirecting to %s:443\n", L_MSNxSTR, strHost.c_str());
      MSNAuthenticateRedirect(strHost, strToSend);
      return;
    }
    else
      gLog.Error("%sMalformed location header.\n", L_MSNxSTR);
  }
  else if (strFirstLine == "HTTP/1.1 401 Unauthorized")
  {
    gLog.Error("%sInvalid password.\n", L_MSNxSTR);
  }
  else
  {
    gLog.Error("%sUnknown sign in error.\n", L_MSNxSTR);
  }
  
  gSocketMan.CloseSocket(m_nSSLSocket, false, true);
  m_nSSLSocket = -1;
  delete m_pSSLPacket;
  m_pSSLPacket = 0;
}

void CMSN::ProcessNexusPacket(CMSNBuffer &packet)
{
  bool bNew = false;
  if (m_pNexusBuff == 0)
  {
    m_pNexusBuff = new CMSNBuffer(packet);
    bNew = true;
  }

  char *ptr = packet.getDataStart() + packet.getDataSize() - 4;
  int x = memcmp(ptr, "\x0D\x0A\x0D\x0A", 4);
  if (x) return;
  else if (!bNew) *m_pNexusBuff += packet;

  char cTmp = 0;

  while (cTmp != '\r')
    *m_pNexusBuff >> cTmp;
  *m_pNexusBuff >> cTmp; // skip the \n as well

  m_pNexusBuff->ParseHeaders();
  
  char *szLogin = strstr(m_pNexusBuff->GetValue("PassportURLs").c_str(), "DALogin=");
  szLogin += 8; // skip to the tag
  //char *szEndURL = strchr(szLogin, '/');
  //char *szServer = strndup(szLogin, szEndURL - szLogin); // this is all we need
  //char *szEnd = strchr(szLogin, ',');
  //char *szURL = strndup(szEndURL, szEnd - szEndURL);

  MSNAuthenticate(m_szCookie);
}

void CMSN::MSNGetServer()
{
  TCPSocket *sock = new TCPSocket(m_szUserName, MSN_PPID);
  sock->SetRemoteAddr("nexus.passport.com", 443);
//  char ipbuf[32];
  if (!sock->OpenConnection())
  {
    delete sock;
    return;
  }
  
  if (!sock->SecureConnect())
  {
    delete sock;
    return;
  }

  gSocketMan.AddSocket(sock);
  m_nNexusSocket = sock->Descriptor();
  CMSNPacket *pHello = new CPS_MSNGetServer();
  sock->SSLSend(pHello->getBuffer());
  gSocketMan.DropSocket(sock);
}

void CMSN::MSNAuthenticateRedirect(const string &strHost, const string& /* strParam */)
{
  TCPSocket *sock = new TCPSocket(m_szUserName, MSN_PPID);
  sock->SetRemoteAddr(strHost.c_str(), 443);
  char ipbuf[32];
  gLog.Info("%sAuthenticating to %s:%d\n", L_MSNxSTR, sock->RemoteIpStr(ipbuf),
sock->RemotePort());

  if (!sock->OpenConnection())
  {
    gLog.Error("%sConnection to %s failed.\n", L_MSNxSTR, sock->RemoteIpStr(ipbuf));
    delete sock;
    return;
  }

  if (!sock->SecureConnect())
  {
    gLog.Error("%sSSL connection failed.\n", L_MSNxSTR);
    delete sock;
    return;
  }

  gSocketMan.AddSocket(sock);
  m_nSSLSocket = sock->Descriptor();
  CMSNPacket *pHello = new CPS_MSNAuthenticate(m_szUserName, m_szPassword, m_szCookie);
  sock->SSLSend(pHello->getBuffer());
  gSocketMan.DropSocket(sock);
}

void CMSN::MSNAuthenticate(char *szCookie)
{
  TCPSocket *sock = new TCPSocket(m_szUserName, MSN_PPID);
  sock->SetRemoteAddr("loginnet.passport.com", 443);
  char ipbuf[32];
  gLog.Info("%sAuthenticating to %s:%d\n", L_MSNxSTR, sock->RemoteIpStr(ipbuf), sock->RemotePort());
  
  if (!sock->OpenConnection())
  {
    gLog.Error("%sConnection to %s failed.\n", L_MSNxSTR, sock->RemoteIpStr(ipbuf));
    delete sock;
    free(szCookie);
    szCookie = 0;
    return;
  }
  
  if (!sock->SecureConnect())
  {
    gLog.Error("%sSSL connection failed.\n", L_MSNxSTR);   
    free(szCookie);
    szCookie = 0;
    delete sock;
    return;
  }
  
  gSocketMan.AddSocket(sock);
  m_nSSLSocket = sock->Descriptor();
  CMSNPacket *pHello = new CPS_MSNAuthenticate(m_szUserName, m_szPassword, szCookie);
  sock->SSLSend(pHello->getBuffer());
  gSocketMan.DropSocket(sock);
}

