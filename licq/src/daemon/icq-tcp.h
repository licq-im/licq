#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream.h>

#include "time-fix.h"

#include "icqd.h"

//-----ICQ::sendMessage----------------------------------------------------------------------------
ICQEvent *CICQDaemon::icqSendMessage(unsigned long _nUin, const char *m, bool online, bool _bUrgent, unsigned long _nSourceUin = 0)
{
  ICQEvent *result = NULL;
  char *mDos = NULL;
  if (m != NULL)
  {
    mDos = gTranslator.NToRN(m);
    gTranslator.ClientToServer(mDos);
  }
  CEventMsg *e = NULL;

  if (!online) // send offline
  {
     e = new CEventMsg(m, ICQ_CMDxSND_THRUxSERVER, TIME_NOW, INT_VERSION);
     CPU_ThroughServer *p = new CPU_ThroughServer(_nSourceUin, _nUin, ICQ_CMDxSUB_MSG, mDos);
     gLog.Info("%sSending message through server (#%d).\n", L_UDPxSTR, p->getSequence());
     result = SendExpectEvent(m_nUDPSocketDesc, p, CONNECT_NONE, _nUin, e);
  }
  else        // send direct
  {
    e = new CEventMsg(m, ICQ_CMDxTCP_START, TIME_NOW, E_DIRECT | INT_VERSION);
    ICQUser *u = gUserManager.FetchUser(_nUin, LOCK_W);
    CPT_Message *p = new CPT_Message(_nSourceUin, mDos, _bUrgent, u);
    gLog.Info("%sSending %smessage to %s (#%d).\n", L_TCPxSTR, _bUrgent ? "urgent " : "", u->GetAlias(), p->getSequence());
    result = SendExpectEvent(u->SocketDesc(), p, CONNECT_USER, _nUin, e);
    gUserManager.DropUser(u);
  }

  delete mDos;
  return (result);
}


//-----CICQDaemon::sendReadAwayMsg------------------------------------------------------------------------
ICQEvent *CICQDaemon::icqFetchAutoResponse(unsigned long _nUin, unsigned long _nSourceUin = 0)
{
  ICQUser *u = gUserManager.FetchUser(_nUin, LOCK_W);
  CPT_ReadAwayMessage *p = new CPT_ReadAwayMessage(_nSourceUin, u);
  gLog.Info("%sRequesting away message from %s (#%d).\n", L_TCPxSTR,
            u->GetAlias(), p->getSequence());
  ICQEvent *result = SendExpectEvent(u->SocketDesc(), p, CONNECT_USER, _nUin, NULL);
  gUserManager.DropUser(u);

  return (result);
}


//-----CICQDaemon::sendUrl--------------------------------------------------------------------------------
ICQEvent *CICQDaemon::icqSendUrl(unsigned long _nUin, const char *url, const char *description, bool online, bool _bUrgent, unsigned long _nSourceUin = 0)
{
   // make the URL info string
  char *szDescDos = NULL;
  CEventUrl *e = NULL;
  szDescDos = gTranslator.NToRN(description);
  gTranslator.ClientToServer(szDescDos);
  char m[ (url == NULL ? 0 : strlen(url)) + (szDescDos == NULL ? 0 : strlen(szDescDos)) + 2];
  strcpy(m, szDescDos == NULL ? "" : szDescDos);
  int nLen = strlen(m);
  m[nLen] = (char)0xFE;
  m[nLen + 1] = '\0';
  strcat(m, url == NULL ? "" : url);

  ICQEvent *result = NULL;

  if (!online) // send offline
  {
    e = new CEventUrl(url, description, ICQ_CMDxSND_THRUxSERVER, TIME_NOW, INT_VERSION);
    CPU_ThroughServer *p = new CPU_ThroughServer(_nSourceUin, _nUin, ICQ_CMDxSUB_URL, m);
    gLog.Info("%sSending url through server (#%d).\n", L_UDPxSTR, p->getSequence());
    result = SendExpectEvent(m_nUDPSocketDesc, p, CONNECT_NONE, _nUin, e);
  }
  else
  {
    e = new CEventUrl(url, description, ICQ_CMDxTCP_START, TIME_NOW, E_DIRECT | INT_VERSION);
    ICQUser *u = gUserManager.FetchUser(_nUin, LOCK_W);
    CPT_Url *p = new CPT_Url(_nSourceUin, m, _bUrgent, u);
    gLog.Info("%sSending %sURL to %s (#%d).\n", L_TCPxSTR, _bUrgent ? "urgent " : "", u->GetAlias(), p->getSequence());
    result = SendExpectEvent(u->SocketDesc(), p, CONNECT_USER, _nUin, e);
    gUserManager.DropUser(u);
  }

  delete szDescDos;
  return (result);
}


//-----CICQDaemon::sendFile------------------------------------------------------------
ICQEvent *CICQDaemon::icqFileTransfer(unsigned long _nUin, const char *_szFilename,
                        const char *_szDescription, bool online, bool _bUrgent,
                        unsigned long _nSourceUin = 0)
{
  ICQEvent *result = NULL;
  char *szDosDesc = NULL;
  if (_szDescription != NULL)
  {
    szDosDesc = gTranslator.NToRN(_szDescription);
    gTranslator.ClientToServer(szDosDesc);
  }
  CEventFile *e = NULL;
  online = true;

  ICQUser *u = gUserManager.FetchUser(_nUin, LOCK_W);
  CPT_FileTransfer *p = new CPT_FileTransfer(_nSourceUin, _szFilename, szDosDesc,
                                             _bUrgent, u);
  if (!p->IsValid())
  {
    delete p;
    result = NULL;
  }
  else
  {
    e = new CEventFile(_szFilename, p->GetDescription(), p->GetFileSize(),
                       p->getSequence(), TIME_NOW, E_DIRECT | INT_VERSION);
    gLog.Info("%sSending %sfile transfer to %s (#%d).\n", L_TCPxSTR, _bUrgent ? "urgent " : "",
             u->GetAlias(), p->getSequence());
    result = SendExpectEvent(u->SocketDesc(), p, CONNECT_USER, _nUin, e);
  }
  gUserManager.DropUser(u);


  delete szDosDesc;
  return (result);
}


//-----CICQDaemon::fileCancel-------------------------------------------------------------------------
void CICQDaemon::icqFileTransferCancel(unsigned long _nUin, unsigned long _nSequence)
{
  // add to history ??
  ICQUser *u = gUserManager.FetchUser(_nUin, LOCK_R);
  if (u == NULL) return;
  gLog.Info("%sCancelling file transfer to %s (#%lu).\n", L_TCPxSTR, u->GetAlias(), _nSequence);
  CPT_CancelFile p(_nSequence, u);
  AckTCP(p, u->SocketDesc());
  gUserManager.DropUser(u);
}


//-----CICQDaemon::fileAccept-----------------------------------------------------------------------------
void CICQDaemon::icqFileTransferAccept(unsigned long _nUin, unsigned short _nPort, unsigned long _nSequence)
{
   // basically a fancy tcp ack packet which is sent late
   // add to history ??
  ICQUser *u = gUserManager.FetchUser(_nUin, LOCK_R);
  if (u == NULL) return;
  gLog.Info("%sAccepting file transfer from %s (#%lu).\n", L_TCPxSTR, u->GetAlias(), _nSequence);
  CPT_AckFileAccept p(_nPort, _nSequence, u);
  AckTCP(p, u->SocketDesc());
  gUserManager.DropUser(u);
}



//-----CICQDaemon::chatRefuse-----------------------------------------------------------------------------
void CICQDaemon::icqFileTransferRefuse(unsigned long _nUin, const char *_sReason, unsigned long _nSequence)
{
   // add to history ??
  char *szReasonDos = gTranslator.NToRN(_sReason);
  gTranslator.ClientToServer(szReasonDos);
  ICQUser *u = gUserManager.FetchUser(_nUin, LOCK_R);
  if (u == NULL) return;
  gLog.Info("%sRefusing file transfer from %s (#%lu).\n", L_TCPxSTR, u->GetAlias(), _nSequence);
  CPT_AckFileRefuse p(szReasonDos, _nSequence, u);
  AckTCP(p, u->SocketDesc());
  gUserManager.DropUser(u);
  delete szReasonDos;
}


//-----CICQDaemon::sendChat------------------------------------------------------------
ICQEvent *CICQDaemon::icqChatRequest(unsigned long _nUin, const char *reason, bool online, bool _bUrgent, unsigned long _nSourceUin = 0)
{
  online = true;

  ICQUser *u = gUserManager.FetchUser(_nUin, LOCK_W);
  if (u == NULL) return NULL;
  char *szReasonDos = gTranslator.NToRN(reason);
  gTranslator.ClientToServer(szReasonDos);
  CPT_ChatRequest *p = new CPT_ChatRequest(_nSourceUin, szReasonDos, _bUrgent, u);
  CEventChat *e = new CEventChat(reason, p->getSequence(), TIME_NOW, E_DIRECT | INT_VERSION);
  gLog.Info("%sSending %schat request to %s (#%d).\n", L_TCPxSTR, _bUrgent ? "urgent " : "",
            u->GetAlias(), p->getSequence());
  ICQEvent *result = SendExpectEvent(u->SocketDesc(), p, CONNECT_USER, _nUin, e);
  gUserManager.DropUser(u);

  delete szReasonDos;
  return (result);
}


//-----CICQDaemon::chatCancel----------------------------------------------------------
void CICQDaemon::icqChatRequestCancel(unsigned long _nUin, unsigned long _nSequence)
{
   // add to history ??
  ICQUser *u = gUserManager.FetchUser(_nUin, LOCK_R);
  if (u == NULL) return;
  gLog.Info("%sCancelling chat request with %s (#%d).\n", L_TCPxSTR, u->GetAlias(), _nSequence);
  CPT_CancelChat p(_nSequence, u);
  AckTCP(p, u->SocketDesc());
  gUserManager.DropUser(u);
}


//-----CICQDaemon::chatRefuse-----------------------------------------------------------------------------
void CICQDaemon::icqChatRequestRefuse(unsigned long _nUin, const char *_sReason, unsigned long _nSequence)
{
  // add to history ??
  ICQUser *u = gUserManager.FetchUser(_nUin, LOCK_R);
  if (u == NULL) return;
  gLog.Info("%sRefusing chat request with %s (#%d).\n", L_TCPxSTR, u->GetAlias(), _nSequence);
  char *szReasonDos = gTranslator.NToRN(_sReason);
  gTranslator.ClientToServer(szReasonDos);

  CPT_AckChatRefuse p(szReasonDos, _nSequence, u);
  AckTCP(p, u->SocketDesc());
  gUserManager.DropUser(u);
  delete szReasonDos;
}


//-----CICQDaemon::chatAccept-----------------------------------------------------------------------------
void CICQDaemon::icqChatRequestAccept(unsigned long _nUin, unsigned short _nPort, unsigned long _nSequence)
{
  // basically a fancy tcp ack packet which is sent late
  // add to history ??
  ICQUser *u = gUserManager.FetchUser(_nUin, LOCK_R);
  if (u == NULL) return;
  gLog.Info("%sAccepting chat request with %s (#%d).\n", L_TCPxSTR, u->GetAlias(), _nSequence);
  CPT_AckChatAccept p(_nPort, _nSequence, u);
  AckTCP(p, u->SocketDesc());
  gUserManager.DropUser(u);
}


/*------------------------------------------------------------------------------
 * ConnectToUser
 *
 * Creates a new TCPSocket and connects it to a given user.  Adds the socket
 * to the global socket manager and to the user.
 *----------------------------------------------------------------------------*/
int CICQDaemon::ConnectToUser(unsigned long _nUin)
{
  TCPSocket *s = new TCPSocket(_nUin);
  ICQUser *u = gUserManager.FetchUser(_nUin, LOCK_R);
  if (u == NULL)
  {
    delete s;
    return -1;
  }
  char *szAlias = strdup(u->GetAlias());

  gLog.Info("%sConnecting to %s (%d) on port %d.\n", L_TCPxSTR, szAlias, _nUin, u->Port());

  // If we fail to set the remote address, the ip must be 0
  bool b = s->SetRemoteAddr(u->Ip(), u->Port());
  gUserManager.DropUser(u);
  if (!b)
  {
    delete s; free(szAlias);
    return -1;
  }

  if (!s->OpenConnection())
  {
    char buf[128];
    gLog.Warn("%sConnect to %s (%d) failed:\n%s%s.\n", L_WARNxSTR, szAlias,
              _nUin, L_BLANKxSTR, s->ErrorStr(buf, 128));
    delete s; free(szAlias);
    return -1;
  }

  gLog.Info("%sShaking hands with %s (%d).\n", L_TCPxSTR, szAlias, _nUin);
  CPacketTcp_Handshake p(s->LocalPort());
  if (!s->SendPacket(p.getBuffer()))
  {
    char buf[128];
    gLog.Warn("%sHandshake failed:\n%s%s.\n", L_WARNxSTR, L_BLANKxSTR, s->ErrorStr(buf, 128)); 
    delete s; free (szAlias);
    return -1;
  }

  // Add the new socket to the socket manager
  gSocketManager.AddSocket(s);
  int nSD = s->Descriptor();
  gSocketManager.DropSocket(s);

  // Set the socket descriptor in the user
  u = gUserManager.FetchUser(_nUin, LOCK_W);
  u->SetSocketDesc(nSD);
  gUserManager.DropUser(u);
  free (szAlias);

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
  // THIS IS SHIT AND NEEDS TO BE CHANGED
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
  if (checkUin == 0 || command == 0 || newCommand == 0)
  {
    char *buf;
    gLog.Unknown("%sInvalid TCP packet:\n%s\n", L_UNKNOWNxSTR, packet.print(buf));
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

  unsigned long nMask = E_DIRECT
                        | ((newCommand & ICQ_CMDxSUB_FxMULTIREC) ? E_MULTIxREC : 0)
                        | ((msgFlags & ICQ_TCPxMSG_URGENT) ? E_URGENT : 0);
  newCommand &= ~ICQ_CMDxSUB_FxMULTIREC;
  bool bUrgent = msgFlags & ICQ_TCPxMSG_URGENT || msgFlags & ICQ_TCPxMSG_LIST;

  switch(command)
  {

  //-----START------------------------------------------------------------------
  case ICQ_CMDxTCP_START:
    switch(newCommand)
    {
    case ICQ_CMDxSUB_MSG:  // straight message from a user
    {
      gLog.Info("%sMessage from %s (%d).\n", L_TCPxSTR, u->GetAlias(), checkUin);
      packet >> theSequence >> licqChar >> licqVersion;
      gTranslator.ServerToClient (message);
      CEventMsg *e = new CEventMsg(message, ICQ_CMDxTCP_START, TIME_NOW,
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

      CPT_AckMessage p(theSequence, true, bUrgent, u);
      AckTCP(p, u->SocketDesc());

      // If we are in DND or Occupied and message isn't urgent then we ignore it
      if (!bUrgent)
      {
        ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
        unsigned long s = o->getStatus();
        gUserManager.DropOwner();
        if (s == ICQ_STATUS_OCCUPIED || s == ICQ_STATUS_DND)
        {
          delete e;
          break;
        }
      }
      if (!AddUserEvent(u, e)) break;
      m_xOnEventManager.Do(ON_EVENT_MSG, u);
      gUserManager.DropUser(u);
      u = gUserManager.FetchUser(checkUin, LOCK_R);
      gUserManager.Reorder(u);
      PushPluginSignal(new CICQSignal(SIGNAL_UPDATExLIST, LIST_REORDER, checkUin));
      break;
    }
    case ICQ_CMDxTCP_READxNAxMSG:
    case ICQ_CMDxTCP_READxDNDxMSG:
    case ICQ_CMDxTCP_READxOCCUPIEDxMSG:
    case ICQ_CMDxTCP_READxAWAYxMSG:  // read away message
    {
      gLog.Info("%s%s (%d) requested auto response.\n", L_TCPxSTR, u->GetAlias(), checkUin);
      packet >> theSequence >> licqChar >> licqVersion;
      CPT_AckReadAwayMessage p(newCommand, theSequence, true, u);
      AckTCP(p, u->SocketDesc());
      break;
    }

    case ICQ_CMDxSUB_URL:  // url sent
    {
      gLog.Info("%sURL from %s (%d).\n", L_TCPxSTR, u->GetAlias(), checkUin);
      packet >> theSequence >> licqChar >> licqVersion;
      // parse the message into url and url description
      char **szUrl = new char*[2]; // desc, url
      ParseFE(message, &szUrl, 2);

      // translating string with Translation Table
      gTranslator.ServerToClient(szUrl[0]);
      CEventUrl *e = new CEventUrl(szUrl[1], szUrl[0], ICQ_CMDxTCP_START, TIME_NOW, nMask | licqVersion);
      delete []szUrl;

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

      CPT_AckUrl p(theSequence, true, bUrgent, u);
      AckTCP(p, u->SocketDesc());
      // If we are in DND or Occupied and message isn't urgent then we ignore it
      if (!bUrgent)
      {
        ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
        unsigned long s = o->getStatus();
        gUserManager.DropOwner();
        if (s == ICQ_STATUS_OCCUPIED || s == ICQ_STATUS_DND)
        {
          delete e;
          break;
        }
      }

      // format the url and url description into a message and add it to the users list
      if (!AddUserEvent(u, e)) break;
      m_xOnEventManager.Do(ON_EVENT_URL, u);
      gUserManager.DropUser(u);
      u = gUserManager.FetchUser(checkUin, LOCK_R);
      gUserManager.Reorder(u);
      PushPluginSignal(new CICQSignal(SIGNAL_UPDATExLIST, LIST_REORDER, checkUin));

      break;
    }

    // Chat Request
    case ICQ_CMDxSUB_CHAT:
    {
      gLog.Info("%sChat request from %s (%d).\n", L_TCPxSTR, u->GetAlias(), checkUin);
      packet >> junkLong >> junkLong >> junkShort >> junkChar >> theSequence
             >> licqChar >> licqVersion;
      // translating string with translation table
      gTranslator.ServerToClient (message);
      CEventChat *e = new CEventChat(message, theSequence, TIME_NOW, nMask | licqVersion);

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
      gUserManager.DropUser(u);
      u = gUserManager.FetchUser(checkUin, LOCK_R);
      gUserManager.Reorder(u);
      PushPluginSignal(new CICQSignal(SIGNAL_UPDATExLIST, LIST_REORDER, checkUin));

      break;
    }
    case ICQ_CMDxSUB_FILE:
    {
      gLog.Info("%sFile transfer request from %s (%d).\n", L_TCPxSTR, u->GetAlias(), checkUin);

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
      gUserManager.DropUser(u);
      u = gUserManager.FetchUser(checkUin, LOCK_R);
      gUserManager.Reorder(u);
      PushPluginSignal(new CICQSignal(SIGNAL_UPDATExLIST, LIST_REORDER, checkUin));
      break;
    }

    // Contact List
    case ICQ_CMDxSUB_CONTACTxLIST:
    {
      gLog.Info("%sContact list from %s (%d).\n", L_BLANKxSTR, u->GetAlias(), checkUin);
      packet >> theSequence >> licqChar >> licqVersion;

      unsigned short i = 0;
      while ((unsigned char)message[i++] != 0xFE);
      message[--i] = '\0';
      int nNumContacts = atoi(message);
      char **szFields = new char*[nNumContacts * 2 + 1];
      ParseFE(&message[++i], &szFields, nNumContacts * 2 + 1);

      // Translate the aliases
      vector <char *> vszFields;
      for (i = 0; i < nNumContacts * 2; i += 2)
      {
        vszFields.push_back(szFields[i]);  // uin
        gTranslator.ServerToClient(szFields[i + 1]); // alias
        vszFields.push_back(szFields[i + 1]);
      }
      CEventContactList *e = new CEventContactList(vszFields, ICQ_CMDxTCP_START,
                                                   TIME_NOW, nMask | licqVersion);
      delete[] szFields;

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

      CPT_AckContactList p(theSequence, true, bUrgent, u);
      AckTCP(p, u->SocketDesc());
      // If we are in DND or Occupied and message isn't urgent then we ignore it
      if (!bUrgent)
      {
        ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
        unsigned long s = o->getStatus();
        gUserManager.DropOwner();
        if (s == ICQ_STATUS_OCCUPIED || s == ICQ_STATUS_DND)
        {
          delete e;
          break;
        }
      }

      if (!AddUserEvent(u, e)) break;
      m_xOnEventManager.Do(ON_EVENT_MSG, u);
      gUserManager.DropUser(u);
      u = gUserManager.FetchUser(checkUin, LOCK_R);
      gUserManager.Reorder(u);
      PushPluginSignal(new CICQSignal(SIGNAL_UPDATExLIST, LIST_REORDER, checkUin));

      break;
    }
    default:
       break;
    }
    break;

  //-----ACK--------------------------------------------------------------------
  case ICQ_CMDxTCP_ACK:  // message received packet
  {
    // If this is not from a user on our list then ignore it
    if (bNewUser) break;

    struct SExtendedAck *sExtendedAck = NULL;

    switch (newCommand)
    {
    case ICQ_CMDxSUB_MSG:
    case ICQ_CMDxTCP_READxNAxMSG:
    case ICQ_CMDxTCP_READxDNDxMSG:
    case ICQ_CMDxTCP_READxOCCUPIEDxMSG:
    case ICQ_CMDxTCP_READxAWAYxMSG:
    case ICQ_CMDxSUB_URL:
    case ICQ_CMDxSUB_CONTACTxLIST:
      packet >> theSequence;
      break;

    case ICQ_CMDxSUB_CHAT:
      packet >> junkShort
             >> junkChar
             >> nPortReversed   // port backwards
             >> nPort    // port to connect to for chat
             >> theSequence;
      if (nPort == 0) nPort = (nPortReversed >> 8) | ((nPortReversed & 0xFF) << 8);
      // only if this is the first chat ack packet do we do anything
      if (s_nChatSequence == theSequence && s_nChatUin == checkUin)
      {
        gUserManager.DropUser(u);
        return true;
      }
      s_nChatSequence = theSequence;
      s_nChatUin = checkUin;

      sExtendedAck = new SExtendedAck;
      sExtendedAck->nPort = nPort;
      sExtendedAck->bAccepted = (ackFlags != ICQ_TCPxACK_REFUSE);
      sExtendedAck->szResponse = strdup(message);
      break;

    case ICQ_CMDxSUB_FILE:
       /* 50 A5 82 00 03 00 DA 07 00 00 50 A5 82 00 03 00 0A 00 6E 6F 20 74 68 61
          6E 6B 73 00 D1 EF 04 9F 7F 00 00 01 4A 1F 00 00 04 01 00 00 00 00 00 00
          00 01 00 00 00 00 00 00 00 00 00 00 03 00 00 00 */
       packet >> nPortReversed
              >> junkShort;
       for (int i = 0; i < junkShort; i++) packet >> junkChar;
       packet >> junkLong
              >> nPort
              >> theSequence;
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

       sExtendedAck = new SExtendedAck;
       sExtendedAck->nPort = nPort;
       sExtendedAck->bAccepted = (ackFlags != ICQ_TCPxACK_REFUSE);
       sExtendedAck->szResponse = strdup(message);
       break;

    default:
      break;
    }

    // translating string with translation table
    gTranslator.ServerToClient (message);
    // output the away message if there is one (ie if user status is not online)
    int nSubResult;
    if (ackFlags == ICQ_TCPxACK_ACCEPT)
    {
      gLog.Info("%sAck from %s (#%d).\n", L_TCPxSTR, u->GetAlias(), theSequence);
      nSubResult = ICQ_TCPxACK_ACCEPT;
    }
    else if (ackFlags == ICQ_TCPxACK_REFUSE)
    {
      gLog.Info("%sRefusal from %s (#%d).\n", L_TCPxSTR, u->GetAlias(), theSequence);
      nSubResult = (newCommand == ICQ_CMDxSUB_FILE || newCommand == ICQ_CMDxSUB_CHAT) ? ICQ_TCPxACK_ACCEPT : ICQ_TCPxACK_REFUSE;
    }
    else if (ackFlags == ICQ_TCPxACK_DND || ackFlags == ICQ_TCPxACK_OCCUPIED)
    {
      gLog.Info("%sReturned from %s (#%d).\n", L_TCPxSTR, u->GetAlias(), theSequence);
      nSubResult = ICQ_TCPxACK_RETURN;
    }
    else
    {
      // Update the away message if it's changed
      if (strcmp(u->AutoResponse(), message) != 0)
      {
        u->SetAutoResponse(message);
        u->setShowAwayMsg(true);
      }
      gLog.Info("%sAuto response from %s (#%d).\n", L_TCPxSTR, u->GetAlias(), theSequence);
      nSubResult = ICQ_TCPxACK_ACCEPT;
    }

    ICQEvent *e = DoneEvent(sockfd, theSequence, EVENT_ACKED);
    if (e != NULL)
    {
      e->m_sExtendedAck = sExtendedAck;
      e->m_nSubResult = nSubResult;
      gUserManager.DropUser(u);
      ProcessDoneEvent(e);
      return true;
    }
    else if (sExtendedAck != NULL)
      delete sExtendedAck;
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
      CEventChatCancel *e = new CEventChatCancel(0, TIME_NOW, E_DIRECT);
      AddUserEvent(u, e);
      gUserManager.DropUser(u);
      u = gUserManager.FetchUser(checkUin, LOCK_R);
      gUserManager.Reorder(u);
      PushPluginSignal(new CICQSignal(SIGNAL_UPDATExLIST, LIST_REORDER, checkUin));
      break;
    }
    case ICQ_CMDxSUB_FILE:
    {
      gLog.Info("%sFile transfer request from %s (%ld) cancelled.\n",
               L_TCPxSTR, u->GetAlias(), checkUin);
      CEventFileCancel *e = new CEventFileCancel(0, TIME_NOW, E_DIRECT);
      AddUserEvent(u, e);
      gUserManager.DropUser(u);
      u = gUserManager.FetchUser(checkUin, LOCK_R);
      gUserManager.Reorder(u);
      PushPluginSignal(new CICQSignal(SIGNAL_UPDATExLIST, LIST_REORDER, checkUin));
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
    gLog.Info("%sConnection from %s (%d).\n", L_TCPxSTR, u->GetAlias(), nUin);
    if (u->SocketDesc() != -1)
      gLog.Warn("%sUser %s (%d) already has an associated socket.\n",
                L_WARNxSTR, u->GetAlias(), nUin);
    u->SetSocketDesc(s->Descriptor());
    gUserManager.DropUser(u);
  }
  else
  {
    gLog.Info("%sConnection from new user (%d).\n", L_TCPxSTR, nUin);
  }
  s->SetOwner(nUin);

  return true;
}




