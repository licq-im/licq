#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream.h>
#include <errno.h>

#include "time-fix.h"

#include "licq_icqd.h"
#include "licq_translate.h"
#include "licq_packets.h"
#include "licq_socket.h"
#include "licq_user.h"
#include "licq_events.h"
#include "licq_log.h"
#include "licq_chat.h"
#include "licq_filetransfer.h"
#include "support.h"


//-----ICQ::sendMessage----------------------------------------------------------------------------
CICQEventTag *CICQDaemon::icqSendMessage(unsigned long _nUin, const char *m, bool online, unsigned short nLevel, unsigned long _nSourceUin)
{
  ICQEvent *result = NULL;
  char *mDos = NULL;
  if (m != NULL)
  {
    mDos = gTranslator.NToRN(m);
    gTranslator.ClientToServer(mDos);
  }
  CEventMsg *e = NULL;

  ICQUser *u = gUserManager.FetchUser(_nUin, LOCK_W);
  if (!online) // send offline
  {
     e = new CEventMsg(m, ICQ_CMDxSND_THRUxSERVER, TIME_NOW, INT_VERSION);
     if (strlen(mDos) > MAX_MESSAGE_SIZE)
     {
       gLog.Warn("%sTruncating message to %d characters to send through server.\n",
                 L_WARNxSTR, MAX_MESSAGE_SIZE);
       mDos[MAX_MESSAGE_SIZE] = '\0';
     }
     if (_nSourceUin != 0 && _nSourceUin != gUserManager.OwnerUin())
     {
       gLog.Error("%sSpoofing does not work through the server, send aborted.\n", L_ERRORxSTR);
       gUserManager.DropUser(u);
       return NULL;
     }
     CPU_ThroughServer *p = new CPU_ThroughServer(0, _nUin, ICQ_CMDxSUB_MSG, mDos);
     gLog.Info("%sSending message through server (#%ld).\n", L_UDPxSTR, p->getSequence());
     result = SendExpectEvent(m_nUDPSocketDesc, p, CONNECT_NONE, _nUin, e);
  }
  else        // send direct
  {
    if (u == NULL) return NULL;
    unsigned long f = E_DIRECT | INT_VERSION;
    if (nLevel == ICQ_TCPxMSG_URGENT) f |= E_URGENT;
    e = new CEventMsg(m, ICQ_CMDxTCP_START, TIME_NOW, f);
    CPT_Message *p = new CPT_Message(_nSourceUin, mDos, nLevel, u);
    gLog.Info("%sSending %smessage to %s (#%ld).\n", L_TCPxSTR,
       nLevel == ICQ_TCPxMSG_URGENT ? "urgent " : "",
       u->GetAlias(), -p->getSequence());
    result = SendExpectEvent(u->SocketDesc(), p, CONNECT_USER, _nUin, e);
  }

  if (u != NULL)
  {
    u->SetSendServer(!online);
    u->SetSendLevel(nLevel);
  }
  gUserManager.DropUser(u);

  delete mDos;
  CICQEventTag *t = NULL;
  if (result != NULL)
    t = new CICQEventTag(result);
  return (t);
}


//-----CICQDaemon::sendReadAwayMsg------------------------------------------------------------------------
CICQEventTag *CICQDaemon::icqFetchAutoResponse(unsigned long nUin)
{
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
  CPT_ReadAwayMessage *p = new CPT_ReadAwayMessage(0, u);
  gLog.Info("%sRequesting away message from %s (#%ld).\n", L_TCPxSTR,
            u->GetAlias(), -p->getSequence());
  ICQEvent *result = SendExpectEvent(u->SocketDesc(), p, CONNECT_USER, nUin, NULL);
  gUserManager.DropUser(u);

  CICQEventTag *t = NULL;
  if (result != NULL)
    t =  new CICQEventTag(result);
  return (t);
}


//-----CICQDaemon::sendUrl--------------------------------------------------------------------------------
CICQEventTag *CICQDaemon::icqSendUrl(unsigned long _nUin, const char *url, const char *description, bool online, unsigned short nLevel, unsigned long _nSourceUin)
{
   // make the URL info string
  char *szDescDos = NULL;
  CEventUrl *e = NULL;
  szDescDos = gTranslator.NToRN(description);
  gTranslator.ClientToServer(szDescDos);
  int n = strlen_safe(url) + strlen_safe(szDescDos) + 2;
  char m[n];
  if (!online && n > MAX_MESSAGE_SIZE && szDescDos != NULL)
    szDescDos[MAX_MESSAGE_SIZE - strlen_safe(url) - 2] = '\0';
  sprintf(m, "%s%c%s", szDescDos == NULL ? "" : szDescDos, char(0xFE), url == NULL ? "" : url);

  ICQEvent *result = NULL;

  ICQUser *u = gUserManager.FetchUser(_nUin, LOCK_W);
  if (!online) // send offline
  {
    e = new CEventUrl(url, description, ICQ_CMDxSND_THRUxSERVER, TIME_NOW, INT_VERSION);
    if (_nSourceUin != 0 && _nSourceUin != gUserManager.OwnerUin())
    {
      gLog.Error("%sSpoofing does not work through the server, send aborted.\n", L_ERRORxSTR);
      gUserManager.DropUser(u);
      return NULL;
    }
    CPU_ThroughServer *p = new CPU_ThroughServer(0, _nUin, ICQ_CMDxSUB_URL, m);
    gLog.Info("%sSending url through server (#%ld).\n", L_UDPxSTR, p->getSequence());
    result = SendExpectEvent(m_nUDPSocketDesc, p, CONNECT_NONE, _nUin, e);
  }
  else
  {
    if (u == NULL) return NULL;
    unsigned long f = E_DIRECT | INT_VERSION;
    if (nLevel == ICQ_TCPxMSG_URGENT) f |= E_URGENT;
    e = new CEventUrl(url, description, ICQ_CMDxTCP_START, TIME_NOW, f);
    CPT_Url *p = new CPT_Url(_nSourceUin, m, nLevel, u);
    gLog.Info("%sSending %sURL to %s (#%ld).\n", L_TCPxSTR,
       nLevel == ICQ_TCPxMSG_URGENT ? "urgent " : "",
       u->GetAlias(), -p->getSequence());
    result = SendExpectEvent(u->SocketDesc(), p, CONNECT_USER, _nUin, e);
  }
  if (u != NULL)
  {
    u->SetSendServer(!online);
    u->SetSendLevel(nLevel);
  }
  gUserManager.DropUser(u);

  delete szDescDos;
  CICQEventTag *t = NULL;
  if (result != NULL)
    t =  new CICQEventTag(result);
  return (t);
}


//-----CICQDaemon::sendFile------------------------------------------------------------
CICQEventTag *CICQDaemon::icqFileTransfer(unsigned long nUin, const char *szFilename,
                        const char *szDescription, unsigned short nLevel)
{
  ICQEvent *result = NULL;
  char *szDosDesc = NULL;
  if (szDescription != NULL)
  {
    szDosDesc = gTranslator.NToRN(szDescription);
    gTranslator.ClientToServer(szDosDesc);
  }
  CEventFile *e = NULL;

  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
  if (u == NULL) return NULL;

  CPT_FileTransfer *p = new CPT_FileTransfer(0, szFilename, szDosDesc, nLevel, u);
  if (!p->IsValid())
  {
    delete p;
    result = NULL;
  }
  else
  {
    unsigned long f = E_DIRECT | INT_VERSION;
    if (nLevel == ICQ_TCPxMSG_URGENT) f |= E_URGENT;
    e = new CEventFile(szFilename, p->GetDescription(), p->GetFileSize(),
                       p->getSequence(), TIME_NOW, f);
    gLog.Info("%sSending %sfile transfer to %s (#%ld).\n", L_TCPxSTR,
       nLevel == ICQ_TCPxMSG_URGENT ? "urgent " : "",
       u->GetAlias(), -p->getSequence());
    result = SendExpectEvent(u->SocketDesc(), p, CONNECT_USER, nUin, e);
  }
  u->SetSendServer(false);
  u->SetSendLevel(nLevel);
  gUserManager.DropUser(u);

  delete szDosDesc;
  CICQEventTag *t = NULL;
  if (result != NULL)
    t =  new CICQEventTag(result);
  return (t);
}


//-----CICQDaemon::sendContactList-------------------------------------------
CICQEventTag *CICQDaemon::icqSendContactList(unsigned long nUin,
   UinList &uins, bool online, unsigned short nLevel)
{
  char *m = new char[2 + uins.size() * 80];
  int p = sprintf(m, "%c%c", (char)uins.size(), char(0xFE));
  ContactList vc;

  ICQUser *u = NULL;
  UinList::iterator iter;
  for (iter = uins.begin(); iter != uins.end(); iter++)
  {
    u = gUserManager.FetchUser(*iter, LOCK_R);
    p += sprintf(&m[p], "%ld%c%s%c", *iter, char(0xFE),
       u == NULL ? "" : u->GetAlias(), char(0xFE));
    vc.push_back(new CContact(*iter, u == NULL ? "" : u->GetAlias()));
    gUserManager.DropUser(u);
  }

  if (!online && p > MAX_MESSAGE_SIZE)
  {
    gLog.Warn("%sContact list too large to send through server.\n", L_WARNxSTR);
    delete []m;
    return NULL;
  }

  CEventContactList *e = NULL;
  ICQEvent *result = NULL;

  u = gUserManager.FetchUser(nUin, LOCK_W);
  if (!online) // send offline
  {
    e = new CEventContactList(vc, ICQ_CMDxSND_THRUxSERVER, TIME_NOW, INT_VERSION);
    CPU_ThroughServer *p = new CPU_ThroughServer(0, nUin, ICQ_CMDxSUB_URL, m);
    gLog.Info("%sSending contact list through server (#%ld).\n", L_UDPxSTR, p->getSequence());
    result = SendExpectEvent(m_nUDPSocketDesc, p, CONNECT_NONE, nUin, e);
  }
  else
  {
    if (u == NULL) return NULL;
    unsigned long f = E_DIRECT | INT_VERSION;
    if (nLevel == ICQ_TCPxMSG_URGENT) f |= E_URGENT;
    e = new CEventContactList(vc, ICQ_CMDxTCP_START, TIME_NOW, f);
    CPT_ContactList *p = new CPT_ContactList(m, nLevel, u);
    gLog.Info("%sSending %scontact list to %s (#%ld).\n", L_TCPxSTR,
       nLevel == ICQ_TCPxMSG_URGENT ? "urgent " : "",
       u->GetAlias(), -p->getSequence());
    result = SendExpectEvent(u->SocketDesc(), p, CONNECT_USER, nUin, e);
  }
  if (u != NULL)
  {
    u->SetSendServer(!online);
    u->SetSendLevel(nLevel);
  }
  gUserManager.DropUser(u);

  delete []m;
  CICQEventTag *t = NULL;
  if (result != NULL)
    t =  new CICQEventTag(result);
  return (t);
}



//-----CICQDaemon::fileCancel-------------------------------------------------------------------------
void CICQDaemon::icqFileTransferCancel(unsigned long nUin, unsigned long nSequence)
{
  // add to history ??
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
  if (u == NULL) return;
  gLog.Info("%sCancelling file transfer to %s (#%ld).\n", L_TCPxSTR,
     u->GetAlias(), -nSequence);
  CPT_CancelFile p(nSequence, u);
  AckTCP(p, u->SocketDesc());
  gUserManager.DropUser(u);
}


//-----CICQDaemon::fileAccept-----------------------------------------------------------------------------
void CICQDaemon::icqFileTransferAccept(unsigned long nUin, unsigned short nPort,
   unsigned long nSequence)
{
   // basically a fancy tcp ack packet which is sent late
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
  if (u == NULL) return;
  gLog.Info("%sAccepting file transfer from %s (#%ld).\n", L_TCPxSTR,
     u->GetAlias(), -nSequence);
  CPT_AckFileAccept p(nPort, nSequence, u);
  AckTCP(p, u->SocketDesc());
  gUserManager.DropUser(u);
}



//-----CICQDaemon::chatRefuse-----------------------------------------------------------------------------
void CICQDaemon::icqFileTransferRefuse(unsigned long nUin, const char *szReason,
   unsigned long nSequence)
{
   // add to history ??
  char *szReasonDos = gTranslator.NToRN(szReason);
  gTranslator.ClientToServer(szReasonDos);
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
  if (u == NULL) return;
  gLog.Info("%sRefusing file transfer from %s (#%ld).\n", L_TCPxSTR,
     u->GetAlias(), -nSequence);
  CPT_AckFileRefuse p(szReasonDos, nSequence, u);
  AckTCP(p, u->SocketDesc());
  gUserManager.DropUser(u);
  delete szReasonDos;
}


//-----CICQDaemon::sendChat------------------------------------------------------------
CICQEventTag *CICQDaemon::icqChatRequest(unsigned long nUin, const char *szReason,
   unsigned short nLevel)
{
  return icqMultiPartyChatRequest(nUin, szReason, NULL, 0, nLevel);
}


CICQEventTag *CICQDaemon::icqMultiPartyChatRequest(unsigned long nUin,
   const char *reason, const char *szChatUsers, unsigned short nPort,
   unsigned short nLevel)
{
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
  if (u == NULL) return NULL;
  char *szReasonDos = gTranslator.NToRN(reason);
  gTranslator.ClientToServer(szReasonDos);
  CPT_ChatRequest *p = new CPT_ChatRequest(0, szReasonDos,
     szChatUsers, nPort, nLevel, u);

  unsigned long f = E_DIRECT | INT_VERSION;
  if (nLevel == ICQ_TCPxMSG_URGENT) f |= E_URGENT;
  CEventChat *e = new CEventChat(reason, szChatUsers, nPort, p->getSequence(),
     TIME_NOW, f);
  gLog.Info("%sSending %schat request to %s (#%ld).\n", L_TCPxSTR,
     nLevel == ICQ_TCPxMSG_URGENT ? "urgent " : "",
     u->GetAlias(), -p->getSequence());
  ICQEvent *result = SendExpectEvent(u->SocketDesc(), p, CONNECT_USER, nUin, e);
  u->SetSendServer(false);
  u->SetSendLevel(nLevel);
  gUserManager.DropUser(u);

  delete szReasonDos;
  CICQEventTag *t = NULL;
  if (result != NULL)
    t =  new CICQEventTag(result);
  return (t);
}


//-----CICQDaemon::chatCancel----------------------------------------------------------
void CICQDaemon::icqChatRequestCancel(unsigned long nUin, unsigned long nSequence)
{
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
  if (u == NULL) return;
  gLog.Info("%sCancelling chat request with %s (#%ld).\n", L_TCPxSTR,
     u->GetAlias(), -nSequence);
  CPT_CancelChat p(nSequence, u);
  AckTCP(p, u->SocketDesc());
  gUserManager.DropUser(u);
}


//-----CICQDaemon::chatRefuse-----------------------------------------------------------------------------
void CICQDaemon::icqChatRequestRefuse(unsigned long nUin, const char *szReason,
   unsigned long nSequence)
{
  // add to history ??
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
  if (u == NULL) return;
  gLog.Info("%sRefusing chat request with %s (#%ld).\n", L_TCPxSTR,
     u->GetAlias(), -nSequence);
  char *szReasonDos = gTranslator.NToRN(szReason);
  gTranslator.ClientToServer(szReasonDos);

  CPT_AckChatRefuse p(szReasonDos, nSequence, u);
  AckTCP(p, u->SocketDesc());
  gUserManager.DropUser(u);
  delete szReasonDos;
}


//-----CICQDaemon::chatAccept-----------------------------------------------------------------------------
void CICQDaemon::icqChatRequestAccept(unsigned long nUin, unsigned short nPort,
   unsigned long nSequence)
{
  // basically a fancy tcp ack packet which is sent late
  // add to history ??
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
  if (u == NULL) return;
  gLog.Info("%sAccepting chat request with %s (#%ld).\n", L_TCPxSTR,
     u->GetAlias(), -nSequence);
  CPT_AckChatAccept p(nPort, nSequence, u);
  AckTCP(p, u->SocketDesc());
  gUserManager.DropUser(u);
}


/*------------------------------------------------------------------------------
 * ConnectToUser
 *
 * Creates a new TCPSocket and connects it to a given user.  Adds the socket
 * to the global socket manager and to the user.
 *----------------------------------------------------------------------------*/
int CICQDaemon::ConnectToUser(unsigned long nUin)
{
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
  if (u == NULL) return -1;

  char szAlias[64];
  strcpy(szAlias, u->GetAlias());
  unsigned short nPort = u->Port();
  int sd = u->SocketDesc();
  gUserManager.DropUser(u);

  if (sd != -1)
  {
    gLog.Warn("%sConnection attempted to already connected user (%ld).\n",
       L_WARNxSTR, nUin);
    return sd;
  }

  TCPSocket *s = new TCPSocket(nUin);
  if (!OpenConnectionToUser(nUin, s, nPort))
  {
    delete s;
    return -1;
  }

  gLog.Info("%sShaking hands with %s (%ld).\n", L_TCPxSTR, szAlias, nUin);
  CPacketTcp_Handshake p(s->LocalPort());
  if (!s->SendPacket(p.getBuffer()))
  {
    char buf[128];
    gLog.Warn("%sHandshake failed:\n%s%s.\n", L_WARNxSTR, L_BLANKxSTR, s->ErrorStr(buf, 128));
    delete s;
    return -1;
  }

  // Add the new socket to the socket manager
  gSocketManager.AddSocket(s);
  int nSD = s->Descriptor();
  gSocketManager.DropSocket(s);

  // Set the socket descriptor in the user
  u = gUserManager.FetchUser(nUin, LOCK_W);
  u->SetSocketDesc(nSD);
  gUserManager.DropUser(u);

  // Alert the select thread that there is a new socket
  write(pipe_newsocket[PIPE_WRITE], "S", 1);

  return nSD;
}



/*------------------------------------------------------------------------------
 * OpenConnectionToUser
 *
 * Connects a socket to a given user on a given port.
 *----------------------------------------------------------------------------*/
bool CICQDaemon::OpenConnectionToUser(unsigned long nUin, TCPSocket *sock,
                                      unsigned short nPort)
{
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
  if (u == NULL) return false;

  char szAlias[64];
  sprintf(szAlias, "%s (%ld)", u->GetAlias(), u->Uin());
  unsigned long ip = u->Ip();
  unsigned long realip = u->RealIp();
  bool bSendRealIp = u->SendRealIp();

  gUserManager.DropUser(u);

  return OpenConnectionToUser(szAlias, ip, realip, sock, nPort, bSendRealIp);
}


bool CICQDaemon::OpenConnectionToUser(const char *szAlias, unsigned long nIp,
   unsigned long nRealIp, TCPSocket *sock, unsigned short nPort, bool bSendRealIp)
{
  char buf[128];

  // Sending to internet ip
  if (!bSendRealIp)
  {
    gLog.Info("%sConnecting to %s at %s:%d.\n", L_TCPxSTR, szAlias,
	     ip_ntoa(nIp, buf), nPort);
    // If we fail to set the remote address, the ip must be 0
    if (!sock->SetRemoteAddr(nIp, nPort)) return false;

    if (!sock->OpenConnection())
    {
      gLog.Warn("%sConnect to %s failed:\n%s%s.\n", L_WARNxSTR, szAlias,
                L_BLANKxSTR, sock->ErrorStr(buf, 128));

      // Now try the real ip if it is different from this one and we are behind a firewall
      if (sock->Error() != EINTR && nRealIp != nIp &&
          nRealIp != 0 && m_szFirewallHost[0] != '\0')
      {
        gLog.Info("%sConnecting to %s at %s:%d.\n", L_TCPxSTR, szAlias,
                  ip_ntoa(nRealIp, buf), nPort);
        sock->SetRemoteAddr(nRealIp, nPort);

        if (!sock->OpenConnection())
        {
          char buf[128];
          gLog.Warn("%sConnect to %s real ip failed:\n%s%s.\n", L_WARNxSTR, szAlias,
                    L_BLANKxSTR, sock->ErrorStr(buf, 128));
          return false;
        }
      }
      else
      {
        return false;
      }
    }
  }

  // Sending to Real IP
  else
  {
    gLog.Info("%sConnecting to %s at %s:%d.\n", L_TCPxSTR, szAlias,
       ip_ntoa(nRealIp, buf), nPort);
    if (!sock->SetRemoteAddr(nRealIp, nPort)) return false;

    if (!sock->OpenConnection())
    {
      gLog.Warn("%sConnect to %s real ip failed:\n%s%s.\n", L_WARNxSTR, szAlias,
         L_BLANKxSTR, sock->ErrorStr(buf, 128));
      return false;
    }
  }


  return true;
}



/*------------------------------------------------------------------------------
 * ReverseConnectToUser
 *
 * Creates a new TCPSocket and connects it to a given user.  Adds the socket
 * to the global socket manager and to the user.
 *----------------------------------------------------------------------------*/
int CICQDaemon::ReverseConnectToUser(unsigned long nUin, unsigned long nIp,
                                     unsigned short nPort)
{
  TCPSocket *s = new TCPSocket(nUin);
  char buf[32];

  gLog.Info("%sReverse connecting to %ld at %s:%d.\n", L_TCPxSTR, nUin,
            ip_ntoa(nIp, buf), nPort);

  // If we fail to set the remote address, the ip must be 0
  s->SetRemoteAddr(nIp, nPort);

  if (!s->OpenConnection())
  {
    char buf[128];
    gLog.Warn("%sReverse connect to %ld failed:\n%s%s.\n", L_WARNxSTR,
              nUin, L_BLANKxSTR, s->ErrorStr(buf, 128));
    return -1;
  }

  gLog.Info("%sReverse shaking hands with %ld.\n", L_TCPxSTR, nUin);
  CPacketTcp_Handshake p(s->LocalPort());
  if (!s->SendPacket(p.getBuffer()))
  {
    char buf[128];
    gLog.Warn("%sReverse handshake failed:\n%s%s.\n", L_WARNxSTR, L_BLANKxSTR, s->ErrorStr(buf, 128));
    delete s;
    return -1;
  }

  // Add the new socket to the socket manager
  gSocketManager.AddSocket(s);
  int nSD = s->Descriptor();
  gSocketManager.DropSocket(s);

  // Set the socket descriptor in the user if this user is on our list
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
  if (u != NULL)
  {
    u->SetSocketDesc(nSD);
    gUserManager.DropUser(u);
  }

  // Alert the select thread that there is a new socket
  write(pipe_newsocket[PIPE_WRITE], "S", 1);

  return nSD;
}


//-----CICQDaemon::ProcessTcpPacket----------------------------------------------------
bool CICQDaemon::ProcessTcpPacket(CBuffer &packet, int sockfd)
{
  unsigned long checkUin, theSequence, senderIp, localIp,
                senderPort, junkLong, nPort, nPortReversed;
  unsigned short version, command, junkShort, newCommand, messageLen,
                 ackFlags, msgFlags, licqVersion;
  char licqChar, junkChar;

  // Static variables to keep track of repeating chat/file ack packets
  // THIS IS SHIT AND NEEDS TO BE CHANGED FIXME
  static unsigned long s_nChatUin, s_nChatSequence, s_nFileUin, s_nFileSequence;

  packet >> checkUin
         >> version
         >> command      // main tcp command (start, cancel, ack)
         >> junkShort    // 00 00 to fill in the MSB of the command long int which is read in as a short
         >> checkUin
         >> newCommand   // sub command (message/chat/read away message/...)
         >> messageLen   // length of incoming message
  ;

  // Some simple validation of the packet
  if (version != ICQ_VERSION_TCP)
  {
    char *buf;
    gLog.Unknown("%sInvalid TCP version (%d):\n%s\n", L_UNKNOWNxSTR, version,
       packet.print(buf));
    delete buf;
    return false;
  }
  else if (checkUin == 0 || command == 0 || newCommand == 0)
  {
    char *buf;
    gLog.Unknown("%sInvalid TCP packet:\n%s\n", L_UNKNOWNxSTR, packet.print(buf));
    delete buf;
    return false;
  }

  if (checkUin == gUserManager.OwnerUin())
  {
    char *buf;
    gLog.Warn("%sTCP message from self (probable spoof):\n%s\n", L_WARNxSTR, packet.print(buf));
    delete buf;
    return false;
  }

  // read in the message minus any stupid DOS \r's
  char message[messageLen + 1];
  unsigned short j = 0;
  for (unsigned short i = 0; i < messageLen; i++)
  {
    packet >> junkChar;
    if (junkChar != 0x0D) message[j++] = junkChar;
  }

  // read in some more stuff common to all tcp packets
  packet >> senderIp
         >> localIp
         >> senderPort
         >> junkChar      // whether use can receive tcp packets directly
         >> ackFlags
         >> msgFlags
  ;

  senderIp = PacketIpToNetworkIp(senderIp);
  localIp = PacketIpToNetworkIp(localIp);

  // Store our status for later use
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  unsigned short nOwnerStatus = o->Status();
  gUserManager.DropOwner();

  // find which user was sent
  bool bNewUser = false;
  ICQUser *u = gUserManager.FetchUser(checkUin, LOCK_W);
  if (u == NULL)
  {
    u = new ICQUser(checkUin);
    u->Lock(LOCK_W);
    u->SetSocketDesc(sockfd);
    bNewUser = true;
  }

  // Check for spoofing
  if (u->SocketDesc() != sockfd)
  {
    gLog.Warn("%sUser %s (%ld) socket (%d) does not match incoming message (%d).\n",
              L_TCPxSTR, u->GetAlias(), u->Uin(), u->SocketDesc(), sockfd);
  }

  unsigned long nMask = E_DIRECT
                        | ((newCommand & ICQ_CMDxSUB_FxMULTIREC) ? E_MULTIxREC : 0)
                        | ((msgFlags & ICQ_TCPxMSG_URGENT) ? E_URGENT : 0);
  newCommand &= ~ICQ_CMDxSUB_FxMULTIREC;
  bool bAccept = msgFlags & ICQ_TCPxMSG_URGENT || msgFlags & ICQ_TCPxMSG_LIST;
  // Flag as sent urgent as well if we are in occ or dnd and auto-accept is on
  if ( ((nOwnerStatus == ICQ_STATUS_OCCUPIED || u->StatusToUser() == ICQ_STATUS_OCCUPIED)
         && u->AcceptInOccupied() ) ||
       ((nOwnerStatus == ICQ_STATUS_DND || u->StatusToUser() == ICQ_STATUS_DND)
         && u->AcceptInDND() ) ||
       (u->StatusToUser() != ICQ_STATUS_OFFLINE && u->StatusToUser() != ICQ_STATUS_OCCUPIED
         && u->StatusToUser() != ICQ_STATUS_DND) )
    bAccept = true;

  //printf("status: %04X (%04X)  msgtype: %04X\n", ackFlags, u->Status(), msgFlags);

  switch(command)
  {

  //-----START------------------------------------------------------------------
  case ICQ_CMDxTCP_START:
  {
    // Process the status bits
    unsigned short s = msgFlags & 0xFF80, ns = 0;
    if (s & ICQ_TCPxMSG_FxINVISIBLE)
    {
      s &= ~ICQ_TCPxMSG_FxINVISIBLE;
      ns |= ICQ_STATUS_FxPRIVATE;
    }
    switch(s)
    {
      case ICQ_TCPxMSG_FxONLINE: ns |= ICQ_STATUS_ONLINE; break;
      case ICQ_TCPxMSG_FxAWAY: ns |= ICQ_STATUS_AWAY; break;
      case ICQ_TCPxMSG_FxOCCUPIED: ns |= ICQ_STATUS_OCCUPIED; break;
      case ICQ_TCPxMSG_FxNA: ns |= ICQ_STATUS_NA; break;
      case ICQ_TCPxMSG_FxDND: ns |= ICQ_STATUS_DND; break;
      default:
        ns = ICQ_STATUS_OFFLINE;
        gLog.Warn("%sUnknown TCP status: %04X\n", L_WARNxSTR, msgFlags);
        break;
    }
    //printf("%08lX\n", (u->StatusFull() & ICQ_STATUS_FxFLAGS) | ns);
    /*if (!bNewUser && ns != ICQ_STATUS_OFFLINE &&
        !((ns & ICQ_STATUS_FxPRIVATE) && u->StatusOffline()))*/
    if (!bNewUser && ns != ICQ_STATUS_OFFLINE &&
        ns != u->Status() | (u->StatusInvisible() ? ICQ_STATUS_FxPRIVATE : 0))
    {
      ChangeUserStatus(u, (u->StatusFull() & ICQ_STATUS_FxFLAGS) | ns);
      gLog.Info("%s%s (%ld) is %s to us.\n", L_TCPxSTR, u->GetAlias(),
         u->Uin(), u->StatusStr());
    }

    // Process the command
    switch(newCommand)
    {
      case ICQ_CMDxSUB_MSG:  // straight message from a user
      {
        packet >> theSequence >> licqChar >> licqVersion;
        nMask |= licqVersion;
        if (licqChar == 'L')
          gLog.Info("%sMessage from %s (%ld) [Licq v0.%d].\n", L_TCPxSTR, u->GetAlias(),
             checkUin, licqVersion);
        else
          gLog.Info("%sMessage from %s (%ld).\n", L_TCPxSTR, u->GetAlias(), checkUin);

        CEventMsg *e = CEventMsg::Parse(message, ICQ_CMDxTCP_START, TIME_NOW, nMask);

        CPT_AckGeneral p(newCommand, theSequence, true, bAccept, u);
        AckTCP(p, u->SocketDesc());

        // If we are in DND or Occupied and message isn't urgent then we ignore it
        if (!bAccept)
        {
          if (nOwnerStatus == ICQ_STATUS_OCCUPIED || nOwnerStatus == ICQ_STATUS_DND)
          {
            delete e;
            break;
          }
        }
        // Add the user to our list if they are new
        if (bNewUser)
        {
          if (Ignore(IGNORE_NEWUSERS))
          {
            // FIXME should log a message here or in reject event
            // FIXME should either refuse the event or have a special auto response
            // for rejected events instead of pretending to accept the user
            RejectEvent(checkUin, e);
            break;
          }
          u->Unlock();
          AddUserToList(u);
          u->Lock(LOCK_W);
          bNewUser = false;
        }

        if (!AddUserEvent(u, e)) break;
        m_xOnEventManager.Do(ON_EVENT_MSG, u);
        break;
      }
      case ICQ_CMDxTCP_READxNAxMSG:
      case ICQ_CMDxTCP_READxDNDxMSG:
      case ICQ_CMDxTCP_READxOCCUPIEDxMSG:
      case ICQ_CMDxTCP_READxFFCxMSG:
      case ICQ_CMDxTCP_READxAWAYxMSG:  // read away message
      {
        packet >> theSequence >> licqChar >> licqVersion;
        if (licqChar == 'L')
          gLog.Info("%s%s (%ld) requested auto response [Licq v0.%d].\n", L_TCPxSTR,
             u->GetAlias(), checkUin, licqVersion);
        else
          gLog.Info("%s%s (%ld) requested auto response.\n", L_TCPxSTR, u->GetAlias(), checkUin);
        m_sStats[STATS_AutoResponseChecked].Inc();

        //CPT_AckReadAwayMsg p(newCommand, theSequence, true, u);
        CPT_AckGeneral p(newCommand, theSequence, true, false, u);
        AckTCP(p, u->SocketDesc());
        break;
      }

      case ICQ_CMDxSUB_URL:  // url sent
      {
        packet >> theSequence >> licqChar >> licqVersion;
        nMask |= licqVersion;
        if (licqChar == 'L')
          gLog.Info("%sURL from %s (%ld) [Licq v0.%d].\n", L_TCPxSTR, u->GetAlias(),
            checkUin, licqVersion);
        else
          gLog.Info("%sURL from %s (%ld).\n", L_TCPxSTR, u->GetAlias(), checkUin);

        CEventUrl *e = CEventUrl::Parse(message, ICQ_CMDxTCP_START, TIME_NOW, nMask);
        if (e == NULL)
        {
          char *buf;
          gLog.Warn("%sInvalid URL message:\n%s\n", L_WARNxSTR, packet.print(buf));
          delete []buf;
          break;
        }

        CPT_AckGeneral p(newCommand, theSequence, true, bAccept, u);
        AckTCP(p, u->SocketDesc());

        // If we are in DND or Occupied and message isn't urgent then we ignore it
        if (!bAccept)
        {
          if (nOwnerStatus == ICQ_STATUS_OCCUPIED || nOwnerStatus == ICQ_STATUS_DND)
          {
            delete e;
            break;
          }
        }
        // Add the user to our list if they are new
        if (bNewUser)
        {
          if (Ignore(IGNORE_NEWUSERS))
          {
            RejectEvent(checkUin, e);
            break;
          }
          u->Unlock();
          AddUserToList(u);
          u->Lock(LOCK_W);
          bNewUser = false;
        }

        if (!AddUserEvent(u, e)) break;
        m_xOnEventManager.Do(ON_EVENT_URL, u);
        break;
      }

      // Contact List
      case ICQ_CMDxSUB_CONTACTxLIST:
      {
        packet >> theSequence >> licqChar >> licqVersion;
        nMask |= licqVersion;
        if (licqChar == 'L')
          gLog.Info("%sContact list from %s (%ld) [Licq v0.%d].\n", L_BLANKxSTR,
             u->GetAlias(), checkUin, licqVersion);
        else
          gLog.Info("%sContact list from %s (%ld).\n", L_BLANKxSTR, u->GetAlias(),
             checkUin);

        CEventContactList *e = CEventContactList::Parse(message, ICQ_CMDxTCP_START, TIME_NOW, nMask);
        if (e == NULL)
        {
          char *buf;
          gLog.Warn("%sInvalid contact list message:\n%s\n", L_WARNxSTR, packet.print(buf));
          delete []buf;
          break;
        }

        CPT_AckGeneral p(newCommand, theSequence, true, bAccept, u);
        AckTCP(p, u->SocketDesc());

        // If we are in DND or Occupied and message isn't urgent then we ignore it
        if (!bAccept)
        {
          if (nOwnerStatus == ICQ_STATUS_OCCUPIED || nOwnerStatus == ICQ_STATUS_DND)
          {
            delete e;
            break;
          }
        }
        // Add the user to our list if they are new
        if (bNewUser)
        {
          if (Ignore(IGNORE_NEWUSERS))
          {
            RejectEvent(checkUin, e);
            break;
          }
          u->Unlock();
          AddUserToList(u);
          u->Lock(LOCK_W);
          bNewUser = false;
        }

        if (!AddUserEvent(u, e)) break;
        m_xOnEventManager.Do(ON_EVENT_MSG, u);

        break;
      }

      // Chat Request
      case ICQ_CMDxSUB_CHAT:
      {
        char szChatClients[1024];
        packet.UnpackString(szChatClients);
        packet.UnpackUnsignedLong(); // reversed port
        unsigned short nPort = packet.UnpackUnsignedLong();
        packet >> theSequence >> licqChar >> licqVersion;

        if (licqChar == 'L')
          gLog.Info("%sChat request from %s (%ld) [Licq v0.%d].\n", L_TCPxSTR,
             u->GetAlias(), checkUin, licqVersion);
        else
          gLog.Info("%sChat request from %s (%ld).\n", L_TCPxSTR, u->GetAlias(),
             checkUin);

        // translating string with translation table
        gTranslator.ServerToClient (message);
        CEventChat *e = new CEventChat(message, szChatClients, nPort, theSequence,
           TIME_NOW, nMask | licqVersion);

        // Add the user to our list if they are new
        if (bNewUser)
        {
          if (Ignore(IGNORE_NEWUSERS))
          {
            RejectEvent(checkUin, e);
            break;
          }
          u->Unlock();
          AddUserToList(u);
          u->Lock(LOCK_W);
          bNewUser = false;
        }

        if (!AddUserEvent(u, e)) break;
        m_xOnEventManager.Do(ON_EVENT_CHAT, u);
        break;
      }
      case ICQ_CMDxSUB_FILE:
      {
        unsigned short nLenFilename;
        unsigned long nFileLength;
        packet >> junkLong
               >> nLenFilename;
        char szFilename[nLenFilename];
        for (unsigned short i = 0; i < nLenFilename; i++)
           packet >> szFilename[i];
        packet >> nFileLength
               >> junkLong
               >> theSequence >> licqChar >> licqVersion;

        if (licqChar == 'L')
          gLog.Info("%sFile transfer request from %s (%ld) [Licq v0.%d].\n",
             L_TCPxSTR, u->GetAlias(), checkUin, licqVersion);
        else
          gLog.Info("%sFile transfer request from %s (%ld).\n", L_TCPxSTR,
             u->GetAlias(), checkUin);

        // translating string with translation table
        gTranslator.ServerToClient (message);
        CEventFile *e = new CEventFile(szFilename, message, nFileLength,
                                       theSequence, TIME_NOW,
                                       nMask | licqVersion);
        // Add the user to our list if they are new
        if (bNewUser)
        {
          if (Ignore(IGNORE_NEWUSERS))
          {
            RejectEvent(checkUin, e);
            break;
          }
          u->Unlock();
          AddUserToList(u);
          u->Lock(LOCK_W);
          bNewUser = false;
        }

        if (!AddUserEvent(u, e)) break;
        m_xOnEventManager.Do(ON_EVENT_FILE, u);
        break;
      }

      default:
      {
        char *buf;
        gLog.Unknown("%sUnknown TCP message type (%d):\n%s\n", L_UNKNOWNxSTR,
          newCommand, packet.print(buf));
        delete []buf;
        break;
      }
    }
    break;
  }

  //-----ACK--------------------------------------------------------------------
  case ICQ_CMDxTCP_ACK:  // message received packet
  {
    // If this is not from a user on our list then ignore it
    if (bNewUser) break;

    CExtendedAck *pExtendedAck = NULL;

    switch (newCommand)
    {
    case ICQ_CMDxSUB_MSG:
    case ICQ_CMDxTCP_READxNAxMSG:
    case ICQ_CMDxTCP_READxDNDxMSG:
    case ICQ_CMDxTCP_READxOCCUPIEDxMSG:
    case ICQ_CMDxTCP_READxAWAYxMSG:
    case ICQ_CMDxSUB_URL:
    case ICQ_CMDxSUB_CONTACTxLIST:
      packet >> theSequence >> licqChar >> licqVersion;
      break;

    case ICQ_CMDxSUB_CHAT:
    {
      char ul[1024];
      packet.UnpackString(ul);
      packet >> nPortReversed   // port backwards
             >> nPort    // port to connect to for chat
             >> theSequence >> licqChar >> licqVersion;
      if (nPort == 0) nPort = (nPortReversed >> 8) | ((nPortReversed & 0xFF) << 8);
      // only if this is the first chat ack packet do we do anything
      if (s_nChatSequence == theSequence && s_nChatUin == checkUin)
      {
        gUserManager.DropUser(u);
        return true;
      }
      s_nChatSequence = theSequence;
      s_nChatUin = checkUin;

      //pExtendedAck = new CExtendedAck (ackFlags != ICQ_TCPxACK_REFUSE, nPort, message);
      pExtendedAck = new CExtendedAck (nPort != 0, nPort, message);
      break;
    }

    case ICQ_CMDxSUB_FILE:
       /* 50 A5 82 00 03 00 DA 07 00 00 50 A5 82 00 03 00 0A 00 6E 6F 20 74 68 61
          6E 6B 73 00 D1 EF 04 9F 7F 00 00 01 4A 1F 00 00 04 01 00 00 00 00 00 00
          00 01 00 00 00 00 00 00 00 00 00 00 03 00 00 00 */
       packet >> nPortReversed
              >> junkShort;
       for (int i = 0; i < junkShort; i++) packet >> junkChar;
       packet >> junkLong
              >> nPort
              >> theSequence >> licqChar >> licqVersion;
       // Some clients only send the first port (reversed)
       if (nPort == 0) nPort = (nPortReversed >> 8) | ((nPortReversed & 0xFF) << 8);
       // only if this is the first file ack packet do we do anything
       if (s_nFileSequence == theSequence && s_nFileUin == checkUin)
       {
         gUserManager.DropUser(u);
         return true;
       }
       s_nFileSequence = theSequence;
       s_nFileUin = checkUin;

       //pExtendedAck = new CExtendedAck(ackFlags != ICQ_TCPxACK_REFUSE, nPort, message);
       pExtendedAck = new CExtendedAck(nPort != 0, nPort, message);
       break;

    default:
      break;
    }

    char l[32] = "";
    if (licqChar == 'L') sprintf(l, " [Licq v0.%d]", licqVersion);

    // translating string with translation table
    gTranslator.ServerToClient (message);
    // output the away message if there is one (ie if user status is not online)
    int nSubResult;
    if (ackFlags == ICQ_TCPxACK_ACCEPT)
    {
      gLog.Info("%sAck from %s (#%ld)%s.\n", L_TCPxSTR, u->GetAlias(), -theSequence, l);
      nSubResult = ICQ_TCPxACK_ACCEPT;
    }
    else if (ackFlags == ICQ_TCPxACK_REFUSE)
    {
      gLog.Info("%sRefusal from %s (#%ld)%s.\n", L_TCPxSTR, u->GetAlias(), -theSequence, l);
      nSubResult = (newCommand == ICQ_CMDxSUB_FILE || newCommand == ICQ_CMDxSUB_CHAT) ? ICQ_TCPxACK_ACCEPT : ICQ_TCPxACK_REFUSE;
    }
    else
    {
      // Update the away message if it's changed
      if (strcmp(u->AutoResponse(), message) != 0)
      {
        u->SetAutoResponse(message);
        u->SetShowAwayMsg(true);
      }
      if (ackFlags == ICQ_TCPxACK_DND || ackFlags == ICQ_TCPxACK_OCCUPIED)
      {
        gLog.Info("%sReturned from %s (#%ld)%s.\n", L_TCPxSTR, u->GetAlias(),
           -theSequence, l);
        nSubResult = ICQ_TCPxACK_RETURN;
      }
      else
      {
        gLog.Info("%sAuto response from %s (#%ld)%s.\n", L_TCPxSTR,
           u->GetAlias(), -theSequence, l);
        nSubResult = ICQ_TCPxACK_ACCEPT;
      }
    }

    ICQEvent *e = DoneEvent(sockfd, theSequence, EVENT_ACKED);
    if (e != NULL)
    {
      e->m_pExtendedAck = pExtendedAck;
      e->m_nSubResult = nSubResult;
      gUserManager.DropUser(u);
      ProcessDoneEvent(e);
      return true;
    }
    else
    {
      gLog.Warn("%sAck for unknown event.\n", L_TCPxSTR);
      delete pExtendedAck;
    }
    break;
  }

  //-----CANCEL-----------------------------------------------------------------
  case ICQ_CMDxTCP_CANCEL:
    // If from a new user, ignore it
    if (bNewUser) break;

    switch (newCommand)
    {
      case ICQ_CMDxSUB_CHAT:
      {
        gLog.Info("%sChat request from %s (%ld) cancelled.\n", L_TCPxSTR,
                 u->GetAlias(), checkUin);
        //CEventChatCancel *e = new CEventChatCancel(0, TIME_NOW, E_DIRECT);
        //AddUserEvent(u, e);
        packet >> junkLong >> junkLong >> junkShort >> junkChar >> theSequence;
        for (unsigned short i = 0; i < u->NewMessages(); i++)
        {
          if (u->EventPeek(i)->Sequence() == theSequence)
          {
            u->EventPeek(i)->Cancel();
            break;
          }
        }
        break;
      }
      case ICQ_CMDxSUB_FILE:
      {
        gLog.Info("%sFile transfer request from %s (%ld) cancelled.\n",
                 L_TCPxSTR, u->GetAlias(), checkUin);
        //CEventFileCancel *e = new CEventFileCancel(0, TIME_NOW, E_DIRECT);
        //AddUserEvent(u, e);
        packet >> junkLong >> junkShort >> junkChar >> junkLong >> junkLong >> theSequence;
        for (unsigned short i = 0; i < u->NewMessages(); i++)
        {
          if (u->EventPeek(i)->Sequence() == theSequence)
          {
            u->EventPeek(i)->Cancel();
            break;
          }
        }
        break;
      }

      default:
         break;
    }

    break;

  default:
    char *buf;
    gLog.Unknown("%sUnknown TCP packet (command %d):\n%s\n", L_UNKNOWNxSTR, command, packet.print(buf));
    delete [] buf;
    break;
  }

  if (bNewUser)
  {
    delete u;
    return false;
  }
  gUserManager.DropUser(u);
  return true;
}



//-----CICQDaemon::AckTCP--------------------------------------------------------------
void CICQDaemon::AckTCP(CPacketTcp &p, int _nSD)
{
  SendEvent(_nSD, p);
}


/*------------------------------------------------------------------------------
 * ProcessTcpHandshake
 *
 * Takes the first buffer from a socket and parses it as a icq handshake.
 * Does not check that the given user already has a socket or not.
 *----------------------------------------------------------------------------*/
bool CICQDaemon::ProcessTcpHandshake(TCPSocket *s)
{
  char cHandshake;
  unsigned long nVersion;
  CBuffer &b = s->RecvBuffer();
  b >> cHandshake >> nVersion;

  if ((unsigned char)cHandshake != ICQ_CMDxTCP_HANDSHAKE)
  {
    char *buf;
    gLog.Unknown("%sUnknown TCP handshake packet (command = 0x%02X):\n%s\n",
                 L_UNKNOWNxSTR, cHandshake, b.print(buf));
    delete buf;
    return false;
  }

  unsigned long ulJunk, nUin;//, localHost;
  b >> ulJunk >> nUin;
                              //>> localHost >> localHost;
                              //>> ulJunk >> ucJunk;

  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
  if (u != NULL)
  {
    gLog.Info("%sConnection from %s (%ld).\n", L_TCPxSTR, u->GetAlias(), nUin);
    if (u->SocketDesc() != s->Descriptor())
    {
      if (u->SocketDesc() != -1)
      {
        gLog.Warn("%sUser %s (%ld) already has an associated socket.\n",
                  L_WARNxSTR, u->GetAlias(), nUin);
        gSocketManager.CloseSocket(u->SocketDesc(), false);
        u->ClearSocketDesc();
      }
      u->SetSocketDesc(s->Descriptor());
    }
    gUserManager.DropUser(u);
  }
  else
  {
    gLog.Info("%sConnection from new user (%ld).\n", L_TCPxSTR, nUin);
  }
  s->SetOwner(nUin);

  return true;
}


void CICQDaemon::StupidChatLinkageFix()
{
  CChatManager *chatman = new CChatManager(NULL, 0);
  delete chatman;
  CFileTransferManager *ftman = new CFileTransferManager(NULL, 0);
  delete ftman;
}


