#include "msn.h"
#include "msnpacket.h"

#include <string>
#include <openssl/md5.h>

//Global socket manager
CSocketManager gSocketMan;

CMSN::CMSN(CICQDaemon *_pDaemon, int _nPipe)
{
  m_pDaemon = _pDaemon;
  m_nPipe = _nPipe;
  m_nSSLSocket = m_nServerSocket = -1;
  m_pPacketBuf = 0;
  m_szUserName = 0;
  m_szPassword = 0;
}

CMSN::~CMSN()
{
  if (m_pPacketBuf)
    delete m_pPacketBuf;
  if (m_szUserName)
    free(m_szUserName);
  if (m_szPassword)
    free(m_szPassword);
}

void CMSN::Run()
{
  int nResult;
  int nNumDesc;
  int nCurrent; 
  fd_set f;

  while (1)
  {
    f = gSocketMan.SocketSet();
    nNumDesc = gSocketMan.LargestSocket() + 1;
 
    if (m_nPipe != -1)
    {
      FD_SET(m_nPipe, &f);
      if (m_nPipe >= nNumDesc)
        nNumDesc = m_nPipe + 1;
    }

    nResult = select(nNumDesc, &f, NULL, NULL, NULL);
  
    nCurrent = 0;
    while (nResult > 0 && nCurrent < nNumDesc)
    {
      if (FD_ISSET(nCurrent, &f))
      {
        if (nCurrent == m_nPipe)
        {
          ProcessPipe();
        }
        
        if (nCurrent == m_nServerSocket)
        {
          INetSocket *s = gSocketMan.FetchSocket(m_nServerSocket);
          TCPSocket *sock = static_cast<TCPSocket *>(s);
          if (sock->RecvRaw())
          {
            CMSNBuffer packet(sock->RecvBuffer());
            sock->ClearRecvBuffer();
            gSocketMan.DropSocket(sock);
            ProcessServerPacket(packet);
          }
        }
        
        if (nCurrent == m_nSSLSocket)
        {
          INetSocket *s = gSocketMan.FetchSocket(m_nSSLSocket);
          TCPSocket *sock = static_cast<TCPSocket *>(s);
          if (sock->SSLRecv())
          {
            CMSNBuffer packet(sock->RecvBuffer());
            sock->ClearRecvBuffer();
            gSocketMan.DropSocket(sock);
            ProcessSSLServerPacket(packet);
          }
        }
      }

      nCurrent++;
    }

  }
}

void CMSN::ProcessPipe()
{
  char buf[16];
  read(m_nPipe, buf, 1);
  switch (buf[0])
  {
  case 'S':  // A signal is pending
    {
      CSignal *s = m_pDaemon->PopProtoSignal();
      ProcessSignal(s);
      break;
    }
  }
}

void CMSN::ProcessSignal(CSignal *s)
{
  switch (s->Type())
  {
    case PROTOxADD_USER:
    {
      MSNLogon("messenger.hotmail.com", 1863);
      break;
    }
  }
}

void CMSN::ProcessSSLServerPacket(CMSNBuffer &packet)
{
  // Did we receive the entire packet?
  // I don't like doing this, is there a better way to see
  // if we get the entire packet at the socket level?
  // I couldn't find anything in the HTTP RFC about this packet
  // being broken up without any special HTTP headers
  static CMSNBuffer sSSLPacket = packet;
  
  char *ptr = packet.getDataStart() + packet.getDataSize() - 4;
  int x = memcmp(ptr, "\x0D\x0A\x0D\x0A", 4);
  if (sSSLPacket.getDataSize() != packet.getDataSize())
    sSSLPacket += packet;
  
  if (x)  return;
  
  // Now process the packet
  char cTmp = 0;
  std::string strFirstLine = "";
  
  sSSLPacket >> cTmp;
  while (cTmp != '\r')
  {
    strFirstLine += cTmp;
    sSSLPacket >> cTmp;
  }
  
  sSSLPacket >> cTmp; // skip \n as well
  
  // Authenticated
  if (strFirstLine == "HTTP/1.1 200 OK")
  {
    sSSLPacket.ParseHeaders();
    char *fromPP = strstr(sSSLPacket.GetValue("Authentication-Info").c_str(), "from-PP=");
    fromPP+= 9; // skip to the tag
    char *endTag = strchr(fromPP, '\'');
    char *tag = strndup(fromPP, endTag - fromPP); // Thanks, this is all we need
    
    gSocketMan.CloseSocket(m_nSSLSocket, false, true);
    m_nSSLSocket = -1;
    
    CMSNPacket *pReply = new CPS_MSNSendTicket(tag);
    SendPacket(pReply);
    free(tag);
  }
}

void CMSN::ProcessServerPacket(CMSNBuffer &packet)
{
  char szCommand[4];
  unsigned short nSequence;
  CMSNPacket *pReply = 0;
  
  // Build the entire packet
  if (!m_pPacketBuf)
    m_pPacketBuf = new CMSNBuffer(packet);
  else
    *m_pPacketBuf += packet;
    
  if (memcmp((void *)(&(m_pPacketBuf->getDataStart())[m_pPacketBuf->getDataSize() - 2]), "\x0D\x0A", 2))
    return;
  
  while (!m_pPacketBuf->End())
  {
    m_pPacketBuf->UnpackRaw(szCommand, 3);
  
    if (strcmp(szCommand, "VER") == 0)
    {
      // Don't really care about this packet's data.
      pReply = new CPS_MSNClientVersion(m_szUserName);
    }
    else if (strcmp(szCommand, "CVR") == 0)
    {
      // Don't really care about this packet's data.
      pReply = new CPS_MSNUser(m_szUserName);
    }
    else if (strcmp(szCommand, "XFR") == 0)
    {
      //Time to transfer to a new server (assuming notification server always should be safe)
      m_pPacketBuf->SkipParameter(); // Seq
      m_pPacketBuf->SkipParameter(); // 'NS'
      const char *szParam = (m_pPacketBuf->GetParameter()).c_str();
      char szNewServer[16];
      char *szPort;
      if ((szPort = strchr(szParam, ':')))
      {
        strncpy(szNewServer, szParam, szPort - szParam);
        szNewServer[szPort - szParam] = '\0';
      *szPort++ = '\0';
    }
    
      gSocketMan.CloseSocket(m_nServerSocket, false, true);
  
      // Make the new connection
      MSNLogon(szNewServer, atoi(szPort));
    }
    else if (strcmp(szCommand, "USR") == 0)
    {
      m_pPacketBuf->SkipParameter(); // Seq
      std::string strType = m_pPacketBuf->GetParameter();
      
      if (strType == "OK")
      {
        m_pPacketBuf->SkipParameter(); // email account
        std::string strNick = m_pPacketBuf->GetParameter();
        printf("[MSN] %s logged in.\n", strNick.c_str());
        
        pReply = new CPS_MSNSync();
      }
      else
      {
        m_pPacketBuf->SkipParameter(); // "S"
        std::string strParam = m_pPacketBuf->GetParameter();
      
        // Make an SSL connection to authenticate
        MSNAuthenticate(strdup(strParam.c_str()));
      }
    }
    else if (strcmp(szCommand, "CHL") == 0)
    {
      m_pPacketBuf->SkipParameter(); // Seq
      std::string strHash = m_pPacketBuf->GetParameter();
      
      pReply = new CPS_MSNChallenge(strHash.c_str());
    }
    else if (strcmp(szCommand, "SYN") == 0)
    {
      m_pPacketBuf->SkipParameter();
      std::string strVersion = m_pPacketBuf->GetParameter();
      
      pReply = new CPS_MSNChangeStatus();
    }
    else if (strcmp(szCommand, "LST") == 0)
    {
      // Add user
      std::string strUser = m_pPacketBuf->GetParameter();
      m_pDaemon->AddUserToList(strUser.c_str(), MSN_PPID);
    }
    else if (strcmp(szCommand, "LSG") == 0)
    {
      // Add group
    }
    else if (strcmp(szCommand, "CHG") == 0)
    {
      // Send our local list now
      FOR_EACH_PROTO_USER_START(MSN_PPID, LOCK_R)
      {
        pReply = new CPS_MSNAddUser(pUser->IdString());
        SendPacket(pReply);
      }
      FOR_EACH_PROTO_USER_END
      
      pReply = 0;
    }
    else if (strcmp(szCommand, "ILN") == 0)
    {
      m_pPacketBuf->SkipParameter(); // seq
      m_pPacketBuf->SkipParameter(); //status
      std::string strUser = m_pPacketBuf->GetParameter();
      
      ICQUser *u = gUserManager.FetchUser(strUser.c_str(), MSN_PPID, LOCK_W);
      if (u) m_pDaemon->ChangeUserStatus(u, ICQ_STATUS_ONLINE);
      gUserManager.DropUser(u);
    }
    else if (strcmp(szCommand, "NLN") == 0)
    {
      m_pPacketBuf->SkipParameter(); //status
      std::string strUser = m_pPacketBuf->GetParameter();
      
      ICQUser *u = gUserManager.FetchUser(strUser.c_str(), MSN_PPID, LOCK_W);
      if (u) m_pDaemon->ChangeUserStatus(u, ICQ_STATUS_ONLINE);
      gUserManager.DropUser(u);
    }
    else if (strcmp(szCommand, "FLN") == 0)
    {
      std::string strUser = m_pPacketBuf->GetParameter();
      
      ICQUser *u = gUserManager.FetchUser(strUser.c_str(), MSN_PPID, LOCK_W);
      m_pDaemon->ChangeUserStatus(u, ICQ_STATUS_OFFLINE);
      gUserManager.DropUser(u);
    }
    
    // Get the next packet
    m_pPacketBuf->SkipPacket();
    
    if (pReply)
      SendPacket(pReply);
  }
  
  // Clear it out
  delete m_pPacketBuf;
  m_pPacketBuf = 0;
}

void CMSN::SendPacket(CMSNPacket *p)
{
  INetSocket *s = gSocketMan.FetchSocket(m_nServerSocket);
  SrvSocket *sock = static_cast<SrvSocket *>(s);
  sock->SendRaw(p->getBuffer());
  gSocketMan.DropSocket(sock);
  
  delete p;
}

void CMSN::MSNLogon(const char *_szServer, int _nPort)
{
  ICQOwner *o = gUserManager.FetchOwner(MSN_PPID, LOCK_R);
  if (!o)
  {
    printf("No MSN owner set!\n");
    return;
  }
  m_szUserName = strdup(o->IdString());
  m_szPassword = strdup(o->Password());
  gUserManager.DropOwner(MSN_PPID);
  
  SrvSocket *sock = new SrvSocket(m_szUserName, MSN_PPID);
  sock->SetRemoteAddr(_szServer, _nPort);
  char ipbuf[32];
  printf("Server found at %s:%d.\n", sock->RemoteIpStr(ipbuf), sock->RemotePort());
  
  if (!sock->OpenConnection())
  {
    printf("Connect failed!\n");
    delete sock;
    return;
  }
  
  gSocketMan.AddSocket(sock);
  m_nServerSocket = sock->Descriptor();
  gSocketMan.DropSocket(sock);
  
  CMSNPacket *pHello = new CPS_MSNVersion();
  SendPacket(pHello);
}

void CMSN::MSNAuthenticate(char *szCookie)
{
   TCPSocket *sock = new TCPSocket(m_szUserName, MSN_PPID);
  sock->SetRemoteAddr("loginnet.passport.com", 443);
  char ipbuf[32];
  printf("Authenticating to %s:%d\n", sock->RemoteIpStr(ipbuf), sock->RemotePort());
  
  if (!sock->OpenConnection())
  {
    printf("Connect failed!\n");
    delete sock;
    free(szCookie);
    return;
  }
  
  printf("Connected\n");
  
  if (!sock->SecureConnect())
  {
    printf("fuck\n");
    delete sock;
    return;
  }
  
  gSocketMan.AddSocket(sock);
  m_nSSLSocket = sock->Descriptor();
  CMSNPacket *pHello = new CPS_MSNAuthenticate(m_szUserName, m_szPassword, szCookie);
  sock->SSLSend(pHello->getBuffer());
  gSocketMan.DropSocket(sock);

  free(szCookie);
}