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
#include "translate.h"
#include "log.h"

//-----ConnectToServer----------------------------------------------------------
int CICQDaemon::ConnectToServer(void)
{
  // no servers!
  if (icqServers.current() == NULL)
  {
    gLog.Error("%sInternal error: Attempt to connect to server when no servers are set.\n",
               L_ERRORxSTR);
    return (-1);
  }

  // try and set the destination
  gLog.Info("%sResolving %s...\n", L_UDPxSTR, icqServers.current()->name());
  UDPSocket *s = new UDPSocket(0);
  if (!s->SetRemoteAddr(icqServers.current()->name(), icqServers.current()->port()))
  {
    char buf[128];
    gLog.Warn("%sUnable to resolve %s:\n%s%s.\n", L_ERRORxSTR,
              icqServers.current()->name(), L_BLANKxSTR, s->ErrorStr(buf, 128));
    delete s;
    return (-1);  // no route to host (not connected)
  }
  char ipbuf[32];
  gLog.Info("%sICQ server found at %s:%d.\n", L_UDPxSTR,
             s->RemoteIpStr(ipbuf), s->RemotePort());

  gLog.Info("%sCreating local server.\n", L_UDPxSTR);
  if (!s->StartServer(0))
  {
    char buf[128];
    gLog.Warn("%sUnable to start udp server:\n%s%s.\n", L_ERRORxSTR,
              L_BLANKxSTR, s->ErrorStr(buf, 128));
    delete s;
    return -1;
  }
  gLog.Info("%sOpening socket to server.\n", L_UDPxSTR);
  if (!s->OpenConnection())
  {
    char buf[128];
    gLog.Warn("%sUnable to connect to %s:%d:\n%s%s.\n", L_ERRORxSTR,
              s->RemoteIpStr(ipbuf), s->RemotePort(), L_BLANKxSTR,
              s->ErrorStr(buf, 128));
    delete s;
    return -1;
  }

  gSocketManager.AddSocket(s);
  m_nUDPSocketDesc = s->Descriptor();
  gSocketManager.DropSocket(s);
  write(pipe_newsocket[PIPE_WRITE], "S", 1);

  return m_nUDPSocketDesc;
}


//-----icqAddUser----------------------------------------------------------
void CICQDaemon::icqAddUser(unsigned long _nUin)
{
  CPU_ContactList *p = new CPU_ContactList(_nUin);
  gLog.Info("%sAlerting server to new user (#%d)...\n", L_UDPxSTR,
            p->getSequence());
  SendExpectEvent(m_nUDPSocketDesc, p, CONNECT_NONE);

  // update the users info from the server
  icqUserBasicInfo(_nUin);
}


//-----icqAlertUser-------------------------------------------------------------
void CICQDaemon::icqAlertUser(unsigned long _nUin)
{
  CPU_AddUser *p = new CPU_AddUser(_nUin);
  gLog.Info("%sAlerting user they were added (#%d)...\n", L_UDPxSTR, p->getSequence());
  SendExpectEvent(m_nUDPSocketDesc, p, CONNECT_NONE);
}



//-----NextServer---------------------------------------------------------------
void CICQDaemon::SwitchServer(void)
{
  icqLogoff();
  gSocketManager.CloseSocket(m_nUDPSocketDesc);
  m_nUDPSocketDesc = -1;
  icqServers.next();
}


void CICQDaemon::icqRegister(const char *_szPasswd)
{
  ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
  o->SetPassword(_szPasswd);
  gUserManager.DropOwner();
  CPU_Register *p = new CPU_Register(_szPasswd);
  gLog.Info("%sRegistering a new user (#%d)...\n", L_UDPxSTR, p->getSequence());
  SendExpectEvent(m_nUDPSocketDesc, p, CONNECT_SERVER);
}


//-----ICQ::Logon---------------------------------------------------------------
ICQEvent *CICQDaemon::icqLogon(unsigned long logonStatus)
{
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  if (o->Uin() == 0)
  {
    gUserManager.DropOwner();
    gLog.Error("%sNo registered user, unable to process logon attempt.\n", L_ERRORxSTR);
    return NULL;
  }
  char *passwd = strdup(o->Password());
  gUserManager.DropOwner();
  INetSocket *s = gSocketManager.FetchSocket(m_nTCPSocketDesc);
  if (s == NULL) return NULL;
  CPU_Logon *p = new CPU_Logon(s, passwd, logonStatus);
  gSocketManager.DropSocket(s);
  free (passwd);
  gLog.Info("%sRequesting logon (#%d)...\n", L_UDPxSTR, p->getSequence());
  m_nServerAck = p->getSequence() - 1;
  m_nDesiredStatus = logonStatus;
  m_tLogonTime = time(NULL);
  return SendExpectEvent(m_nUDPSocketDesc, p, CONNECT_SERVER);
}


//-----ICQ::icqRelogon-------------------------------------------------------
void CICQDaemon::icqRelogon(void)
{
  unsigned long status;

  if (m_eStatus == STATUS_ONLINE)
  {
    ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
    status = o->StatusFull();
    gUserManager.DropOwner();
  }
  else
  {
    status = m_nDesiredStatus;
  }
  icqLogoff();
  m_eStatus = STATUS_OFFLINE_FORCED;

  icqLogon(status);
}


//-----ICQ::icqLogoff-----------------------------------------------------------
void CICQDaemon::icqLogoff(void)
{
  // Kill the udp socket asap to avoid race conditions
  int nSD = m_nUDPSocketDesc;
  m_nUDPSocketDesc = -1;
  UDPSocket *s = (UDPSocket *)gSocketManager.FetchSocket(nSD);
  // if not connected then don't both logging off
  if (s != NULL)
  {
    gLog.Info("%sLogging off.\n", L_UDPxSTR);
    CPU_Logoff packet;
    s->SendRaw(packet.getBuffer());
    gSocketManager.DropSocket(s);
    gSocketManager.CloseSocket(nSD);
  }
  m_eStatus = STATUS_OFFLINE_MANUAL;

  pthread_mutex_lock(&mutex_runningevents);
  list<ICQEvent *>::iterator iter = m_lxRunningEvents.begin();
  while (iter != m_lxRunningEvents.end())
  {
    if ((*iter)->m_nSocketDesc == nSD)
    {
      if (!pthread_equal((*iter)->thread_send, pthread_self()))
        pthread_cancel((*iter)->thread_send);
      delete (*iter);
      iter = m_lxRunningEvents.erase(iter);
    }
    else
      iter++;
  }
  pthread_mutex_unlock(&mutex_runningevents);
  // wipe out extended events too...
  pthread_mutex_lock(&mutex_extendedevents);
  m_lxExtendedEvents.erase(m_lxExtendedEvents.begin(), m_lxExtendedEvents.end());
  pthread_mutex_unlock(&mutex_extendedevents);

  ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
  ChangeUserStatus(o, ICQ_STATUS_OFFLINE);
  gUserManager.DropOwner();
}


//-----icqUpdateContactList-----------------------------------------------------
void CICQDaemon::icqUpdateContactList(void)
{
  m_nAllowUpdateUsers = 0;
  unsigned short n = 0;
  UinList uins;
  FOR_EACH_USER_START(LOCK_W)
  {
    n++;
    uins.push_back(pUser->Uin());
    if (n == m_nMaxUsersPerPacket)
    {
      CPU_ContactList *p = new CPU_ContactList(uins);
      gLog.Info("%sUpdating contact list (#%d)...\n", L_UDPxSTR, p->getSequence());
      SendExpectEvent(m_nUDPSocketDesc, p, CONNECT_NONE);
      m_nAllowUpdateUsers++;
      uins.clear();
      n = 0;
    }
    // Reset all users to offline
    if (!pUser->StatusOffline()) ChangeUserStatus(pUser, ICQ_STATUS_OFFLINE);
  }
  FOR_EACH_USER_END
  if (n != 0)
  {
    gLog.Info("%sUpdating contact list (#%d)...\n", L_UDPxSTR, p->getSequence());
    CPU_ContactList *p = new CPU_ContactList(uins);
    SendExpectEvent(m_nUDPSocketDesc, p, CONNECT_NONE);
    m_nAllowUpdateUsers++;
  }

  PushPluginSignal(new CICQSignal(SIGNAL_UPDATExLIST, LIST_ALL, 0));
}


//-----icqSendVisibleList-------------------------------------------------------
void CICQDaemon::icqSendVisibleList(bool _bSendIfEmpty = false)
{
  // send user info packet
  // Go through the entire list of users, checking if each one is on
  // the visible list
  UinList uins;
  FOR_EACH_USER_START(LOCK_R)
  {
    if (pUser->GetInGroup(GROUPS_SYSTEM, GROUP_VISIBLE_LIST) )
      uins.push_back(pUser->Uin());
  }
  FOR_EACH_USER_END
  if (uins.size() == 0 && !_bSendIfEmpty) return;

  gLog.Info("%sSending visible list (#%d)...\n", L_UDPxSTR, p->getSequence());
  CPU_VisibleList *p = new CPU_VisibleList(uins);
  SendExpectEvent(m_nUDPSocketDesc, p, CONNECT_NONE);
}


//-----icqSendInvisibleList-----------------------------------------------------
void CICQDaemon::icqSendInvisibleList(bool _bSendIfEmpty = false)
{
  UinList uins;
  FOR_EACH_USER_START(LOCK_R)
  {
    if (pUser->GetInGroup(GROUPS_SYSTEM, GROUP_INVISIBLE_LIST) )
      uins.push_back(pUser->Uin());
  }
  FOR_EACH_USER_END
  if (uins.size() == 0 && !_bSendIfEmpty) return;

  gLog.Info("%sSending invisible list (#%d)...\n", L_UDPxSTR, p->getSequence());
  CPU_InvisibleList *p = new CPU_InvisibleList(uins);
  SendExpectEvent(m_nUDPSocketDesc, p, CONNECT_NONE);
}


//-----icqStartSearch-----------------------------------------------------------
unsigned short CICQDaemon::icqStartSearch(const char *nick, const char *first,
                                          const char *last, const char *email)
{
  CPU_StartSearch *p = new CPU_StartSearch(nick, first, last, email);
  gLog.Info("%sStarting search for user (#%d/#%d)...\n", L_UDPxSTR,
            p->getSequence(), p->SubSequence());
  SendExpectEvent(m_nUDPSocketDesc, p, CONNECT_NONE);
  return p->SubSequence();
}


//-----icqPing------------------------------------------------------------------
void CICQDaemon::icqPing()
{
  CPU_Ping *p = new CPU_Ping;
  gLog.Info("%sPinging Mirabilis (#%d)...\n", L_UDPxSTR, p->getSequence());
  SendExpectEvent(m_nUDPSocketDesc, p, CONNECT_NONE);
}


//-----icqSetStatus-------------------------------------------------------------
ICQEvent *CICQDaemon::icqSetStatus(unsigned long newStatus)
{
  CPU_SetStatus *p = new CPU_SetStatus(newStatus);
  gLog.Info("%sChanging status (#%d)...\n", L_UDPxSTR, p->getSequence());
  m_nDesiredStatus = newStatus;
  return SendExpectEvent(m_nUDPSocketDesc, p, CONNECT_NONE);
}


//-----icqGetUserBasicInfo------------------------------------------------------
ICQEvent *CICQDaemon::icqUserBasicInfo(unsigned long _nUin)
{
  CPU_GetUserBasicInfo *p = new CPU_GetUserBasicInfo(_nUin);
  gLog.Info("%sRequesting user info (#%d/#%d)...\n", L_UDPxSTR,
            p->getSequence(), p->SubSequence());
  return (SendExpectEvent(m_nUDPSocketDesc, p, CONNECT_NONE));
}


//-----icqGetUserExtInfo--------------------------------------------------------
ICQEvent *CICQDaemon::icqUserExtendedInfo(unsigned long _nUin)
{
  CPU_GetUserExtInfo *p = new CPU_GetUserExtInfo(_nUin);
  gLog.Info("%sRequesting extended user info (#%d/#%d)...\n", L_UDPxSTR,
            p->getSequence(), p->SubSequence());
  return (SendExpectEvent(m_nUDPSocketDesc, p, CONNECT_NONE));
}


//-----icqUpdatePersonalBasicInfo-----------------------------------------------
ICQEvent *CICQDaemon::icqUpdateBasicInfo(const char *_sAlias, const char *_sFirstName,
                                     const char *_sLastName, const char *_sEmail,
                                     bool _bAuthorization)
{
  CPU_UpdatePersonalBasicInfo *p =
    new CPU_UpdatePersonalBasicInfo(_sAlias, _sFirstName, _sLastName, _sEmail,
                                    (char)_bAuthorization);
  gLog.Info("%sUpdating personal information (#%d/#%d)...\n", L_UDPxSTR,
            p->getSequence(), p->SubSequence());
  return (SendExpectEvent(m_nUDPSocketDesc, p, CONNECT_NONE));
}


//-----icqUpdatePersonalExtInfo-------------------------------------------------
ICQEvent *CICQDaemon::icqUpdateExtendedInfo(const char *_sCity, unsigned short _nCountry,
                                   const char *_sState, unsigned short _nAge,
                                   char _cSex, const char *_sPhone,
                                   const char *_sHomepage, const char *_sAbout,
                                   unsigned long _nZipcode)
{
  CPU_UpdatePersonalExtInfo *p =
    new CPU_UpdatePersonalExtInfo(_sCity, _nCountry, _sState, _nAge, _cSex,
                                  _sPhone, _sHomepage, _sAbout, _nZipcode);
   gLog.Info("%sUpdating personal extended info (#%d/#%d)...\n", L_UDPxSTR,
             p->getSequence(), p->SubSequence());
   return (SendExpectEvent(m_nUDPSocketDesc, p, CONNECT_NONE));
}


//-----icqSetWorkInfo--------------------------------------------------------
ICQEvent *CICQDaemon::icqSetWorkInfo(const char *_szCity, const char *_szState,
                                     const char *_szFax, const char *_szAddress,
                                     const char *_szName, const char *_szDepartment,
                                     const char *_szPosition, const char *_szHomepage)
{
  CPU_Meta_SetWorkInfo *p =
    new CPU_Meta_SetWorkInfo(_szCity, _szState, _szFax, _szAddress,
                             _szName, _szDepartment, _szPosition, _szHomepage);
  gLog.Info("%sUpdating personal work info (#%d/#%d)...\n", L_UDPxSTR,
            p->getSequence(), p->SubSequence());
  return (SendExpectEvent(m_nUDPSocketDesc, p, CONNECT_NONE));
}


//-----icqSetSecurityInfo----------------------------------------------------
ICQEvent *CICQDaemon::icqSetSecurityInfo(bool bAuthorize, bool bHideIp, bool bWebAware)
{
  CPU_Meta_SetSecurityInfo *p =
    new CPU_Meta_SetSecurityInfo(bAuthorize, bHideIp, bWebAware);
  gLog.Info("%sUpdating security info (#%d/#%d)...\n", L_UDPxSTR,
            p->getSequence(), p->SubSequence());
  return (SendExpectEvent(m_nUDPSocketDesc, p, CONNECT_NONE));
}


//-----icqRequestMetaInfo----------------------------------------------------
ICQEvent *CICQDaemon::icqRequestMetaInfo(unsigned long nUin)
{
  CPU_Meta_RequestInfo *p = new CPU_Meta_RequestInfo(nUin);
  gLog.Info("%sRequesting meta info for %ld (#%d/#%d)...\n", L_UDPxSTR, nUin,
            p->getSequence(), p->SubSequence());
  return (SendExpectEvent(m_nUDPSocketDesc, p, CONNECT_NONE));
}


//-----icqAuthorize-------------------------------------------------------------
void CICQDaemon::icqAuthorize(unsigned long uinToAuthorize)
// authorize a user to add you to their contact list
{
  CPU_Authorize *p = new CPU_Authorize(uinToAuthorize);
  gLog.Info("%sAuthorizing user %ld (#%d)...\n", L_UDPxSTR,
             uinToAuthorize, p->getSequence());
  SendExpectEvent(m_nUDPSocketDesc, p, CONNECT_NONE);
}


//-----icqRequestSystemMsg------------------------------------------------------
// request offline system messages
void CICQDaemon::icqRequestSystemMsg(void)
{
  CPU_RequestSysMsg *p = new CPU_RequestSysMsg;
  gLog.Info("%sSending offline message request (#%d)...\n", L_UDPxSTR,
            p->getSequence());
  SendExpectEvent(m_nUDPSocketDesc, p, CONNECT_NONE);
}



//-----ProcessUdpPacket---------------------------------------------------------
unsigned short CICQDaemon::ProcessUdpPacket(CBuffer &packet, bool bMultiPacket = false)
{
  unsigned short version, nCommand, nSequence, nSubSequence,
                 junkShort;
  unsigned long nUin, junkLong, nOwnerUin;
  char junkChar;

  // read in the standard UDP header info
  packet >> version;

#if ICQ_VERSION == 2
  if (version != 0x02)
#elif ICQ_VERSION == 4
  if (version != 0x03)
#elif ICQ_VERSION == 5
  if (version != 0x05)
#endif
  {
    gLog.Warn("%sServer send bad version number: %d.\n", L_WARNxSTR, version);
    return(0xFFFF);
  }

#if ICQ_VERSION == 2
  packet >> nCommand
         >> nSequence;
#elif ICQ_VERSION == 4
  unsigned long nCheckSum;
  packet >> nCommand
         >> nSequence
         >> nSubSequence
         >> nOwnerUin
         >> nCheckSum;
#elif ICQ_VERSION == 5
  unsigned long nSessionId, nCheckSum;
  packet >> junkChar
         >> nSessionId
         >> nCommand
         >> nSequence
         >> nSubSequence
         >> nOwnerUin
         >> nCheckSum;
#endif

  switch (nCommand)
  {
  case ICQ_CMDxRCV_MULTIxPACKET:  // Multi-packet
  {
    AckUDP(nSequence, nSubSequence);
    unsigned char nPackets;
    unsigned short nLen;
    char *buf;
    packet >> nPackets;

    gLog.Info("%sMultiPacket (%d sub-packets).\n", L_UDPxSTR, nPackets);

    for (unsigned short i = 0; i < nPackets; i++)
    {
      packet >> nLen;
      buf = packet.getDataPosRead() + nLen;
      ProcessUdpPacket(packet, true);
      packet.setDataPosRead(buf);
    }
    break;
  }
  case ICQ_CMDxRCV_USERxONLINE:   // initial user status packet
  {
    /* 02 00 6E 00 0B 00 8F 76 20 00 CD CD 77 90 3F 50 00 00 7F 00 00 01 04 00
       00 00 00 03 00 00 00 */
    if (!bMultiPacket) AckUDP(nSequence, nSubSequence);
    packet >> nUin;

    // find which user it is, verify we have them on our list
    ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
    if (u == NULL)
    {
       gLog.Warn("%sUnknown user (%ld) is online.\n", L_WARNxSTR, nUin);
       break;
    }
    gLog.Info("%s%s (%ld) went online.\n", L_UDPxSTR, u->GetAlias(), nUin);

    // read in the relevant user information
    unsigned short userPort, newStatus;
    unsigned long userIP;
    packet >> userIP
           >> userPort
           >> junkLong >> junkShort >> junkChar  // 7 bytes of junk
           >> newStatus  // initial status of user
    ;

    // The packet class will spit out an ip in network order on a little
    // endian machine and in little-endian on a big endian machine
    userIP = PacketIpToNetworkIp(userIP);
    u->SetIpPort(userIP, userPort);
    ChangeUserStatus(u, newStatus);
    u->SetAutoResponse(NULL);
    if (u->OnlineNotify()) m_xOnEventManager.Do(ON_EVENT_NOTIFY, u);
    gUserManager.DropUser(u);
    u = gUserManager.FetchUser(nUin, LOCK_R);
    gUserManager.Reorder(u);  // put the user at the top of the list
    gUserManager.DropUser(u);
    if (m_nAllowUpdateUsers <= 0)
      PushPluginSignal(new CICQSignal(SIGNAL_UPDATExLIST, LIST_REORDER, nUin));
    break;
  }

  case ICQ_CMDxRCV_USERxOFFLINE:  // user just went offline packet
  {
    /* 02 00 78 00 06 00 ED 21 4E 00 */
    if (!bMultiPacket) AckUDP(nSequence, nSubSequence);
    packet >> nUin;

    // find which user it is, verify we have them on our list
    ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
    if (u == NULL)
    {
       gLog.Warn("%sUnknown user (%ld) has gone offline.\n", L_WARNxSTR, nUin);
       break;
    }
    gLog.Info("%s%s (%ld) went offline.\n", L_UDPxSTR, u->GetAlias(),
              nUin);
    ChangeUserStatus(u, ICQ_STATUS_OFFLINE);
    gUserManager.DropUser(u);
    u = gUserManager.FetchUser(nUin, LOCK_R);
    gUserManager.Reorder(u);
    gUserManager.DropUser(u);
    PushPluginSignal(new CICQSignal(SIGNAL_UPDATExLIST, LIST_REORDER, nUin));
    break;
  }

  case ICQ_CMDxRCV_USERxINFO:   // user info packet
  {
    /* 02 00 18 01 6C 00 10 00 50 A5 82 00 08 00 48 61 63 6B 49 43 51 00 04 00
       46 6F 6F 00 04 00 42 61 72 00 15 00 68 61 63 6B 65 72 73 40 75 77 61 74
       65 72 6C 6F 6F 2E 63 61 00 00 00 00  */
    if (!bMultiPacket) AckUDP(nSequence, nSubSequence);
#if ICQ_VERSION == 2
    packet >> nSubSequence;  // corresponds to the sequence number from the user information request packet...totally irrelevant.
#endif
    packet >> nUin;

    // find which user it is, verify we have them on our list
    ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
    if (u == NULL)
    {
       gLog.Warn("%sInformation on unknown user (%d).\n", L_WARNxSTR, nUin);
       break;
    }
    gLog.Info("%sReceived information for %s (%ld).\n", L_UDPxSTR,
              u->GetAlias(), nUin);

    // read in the four data fields; alias, first name, last name, and email address
    u->SetEnableSave(false);
    char temp[MAX_DATA_LEN], cAuthorization;
    u->SetAlias(packet.UnpackString(temp));
    u->SetFirstName(packet.UnpackString(temp));
    u->SetLastName(packet.UnpackString(temp));
    u->SetEmail1(packet.UnpackString(temp));
    packet >> cAuthorization;
    u->SetAuthorization(cAuthorization == 0 ? true : false);

    // translating string with Translation Table
    gTranslator.ServerToClient(u->GetAlias());
    gTranslator.ServerToClient(u->GetFirstName());
    gTranslator.ServerToClient(u->GetLastName());

    // print out the user information
    /*gLog.Info("%s%s (%s %s), %s.\n", L_SBLANKxSTR , u->GetAlias(),
             u->getFirstName(), u->getLastName(), u->getEmail());*/

    // save the user infomation
    u->SetEnableSave(true);
    u->SaveBasicInfo();

    ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_USERxGETINFO, nSubSequence, EVENT_SUCCESS);
    if (e != NULL)
      ProcessDoneEvent(e);
    else
      gLog.Warn("%sResponse to unknown user info request for %s (%ld).\n",
                L_WARNxSTR, u->GetAlias(), nUin);
    PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_BASIC, u->Uin()));
    gUserManager.DropUser(u);
    break;
  }

  case ICQ_CMDxRCV_USERxDETAILS:   // user details packet
  {
    /* 02 00 22 01 09 00 0A 00 96 3D 44 00 04 00 3F 3F 3F 00 2E 00 FE 01 00 00
       FF FF 01 01 00 00 1C 00 68 74 74 70 3A 2F 2F 68 65 6D 2E 70 61 73 73 61
       67 65 6E 2E 73 65 2F 67 72 72 2F 00 1E 00 49 27 6D 20 6A 75 73 74 20 61
       20 67 69 72 6C 20 69 6E 20 61 20 77 6F 72 6C 64 2E 2E 2E 00 FF FF FF FF */

    if (!bMultiPacket) AckUDP(nSequence, nSubSequence);
#if ICQ_VERSION == 2
    packet >> nSubSequence;  // corresponds to the sequence number from the user information request packet
#endif
    packet >> nUin;
    // find which user it is, verify we have them on our list
    ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
    if (u == NULL)
    {
       gLog.Warn("%sExtended information on unknown user (%d).\n", L_WARNxSTR, nUin);
       break;
    }
    gLog.Info("%sReceived extended information for %s (%ld).\n", L_UDPxSTR,
              u->GetAlias(), nUin);

    u->SetEnableSave(false);
    char sTemp[MAX_MESSAGE_SIZE];

    u->SetCity(packet.UnpackString(sTemp));
    u->SetCountryCode(packet.UnpackUnsignedShort());
    u->SetTimezone(packet.UnpackChar());
    u->SetState(packet.UnpackString(sTemp));
    u->SetAge(packet.UnpackUnsignedShort());
    u->SetGender(packet.UnpackChar());
    u->SetPhoneNumber(packet.UnpackString(sTemp));
    u->SetHomepage(packet.UnpackString(sTemp));
    u->SetAbout(packet.UnpackString(sTemp));
    u->SetZipCode(packet.UnpackUnsignedLong());

    // translating string with Translation Table
    gTranslator.ServerToClient(u->GetCity());
    gTranslator.ServerToClient(u->GetState());
    gTranslator.ServerToClient(u->GetPhoneNumber());
    gTranslator.ServerToClient(u->GetHomepage());
    gTranslator.ServerToClient(u->GetAbout());

    // print out the user information
    /*gLog.Info("%s%s, %s, %s. %d years old, %s. %s, %s, %s.\n", L_SBLANKxSTR,
              u->getCity(), u->getState(), u->getCountry(sTemp), u->getAge(),
              u->getSex(buf), u->getPhoneNumber(), u->getHomepage(), u->getAbout());*/

    // save the user infomation
    u->SetEnableSave(true);
    u->SaveExtInfo();

    ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_USERxGETDETAILS, nSubSequence, EVENT_SUCCESS);
    if (e != NULL) ProcessDoneEvent(e);
    PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER,
                                    USER_EXT, u->Uin()));
    gUserManager.DropUser(u);
    break;
  }

  case ICQ_CMDxRCV_UPDATEDxBASIC:
  {
    /* 02 00 B4 00 28 00 01 00 */
    if (!bMultiPacket) AckUDP(nSequence, nSubSequence);
#if ICQ_VERSION == 2
    packet >> nSubSequence;
#endif
    gLog.Info("%sSuccessfully updated basic info.\n", L_UDPxSTR);
    ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_UPDATExBASIC, nSubSequence, EVENT_SUCCESS);
    CPU_UpdatePersonalBasicInfo *p = (CPU_UpdatePersonalBasicInfo *)e->m_xPacket;
    ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
    o->SetAlias(p->Alias());
    o->SetFirstName(p->FirstName());
    o->SetLastName(p->LastName());
    o->SetEmail1(p->Email());
    o->SetAuthorization(p->Authorization());

    // translating string with Translation Table
    gTranslator.ServerToClient(o->GetAlias());
    gTranslator.ServerToClient(o->GetFirstName());
    gTranslator.ServerToClient(o->GetLastName());

    PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER,
                                    USER_BASIC, o->Uin()));
    gUserManager.DropOwner();
    if (e != NULL) ProcessDoneEvent(e);
    break;
  }

  case ICQ_CMDxRCV_UPDATExBASICxFAIL:
  {
    if (!bMultiPacket) AckUDP(nSequence, nSubSequence);
    gLog.Info("%sFailed to update basic info.\n", L_UDPxSTR);
#if ICQ_VERSION == 2
    packet >> nSubSequence;
#endif
    ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_UPDATExBASIC, nSubSequence, EVENT_FAILED);
    if (e != NULL) ProcessDoneEvent(e);
    break;
  }

  case ICQ_CMDxRCV_UPDATEDxDETAIL:
  {
    if (!bMultiPacket) AckUDP(nSequence, nSubSequence);
    gLog.Info("%sSuccessfully updated detail info.\n", L_UDPxSTR);
#if ICQ_VERSION == 2
    packet >> nSubSequence;
#endif
    ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_UPDATExDETAIL, nSubSequence, EVENT_SUCCESS);
    CPU_UpdatePersonalExtInfo *p = (CPU_UpdatePersonalExtInfo *)e->m_xPacket;
    ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
    o->SetCity(p->City());
    o->SetCountryCode(p->Country());
    o->SetState(p->State());
    o->SetAge(p->Age());
    o->SetGender(p->Sex());
    o->SetPhoneNumber(p->PhoneNumber());
    o->SetHomepage(p->Homepage());
    o->SetAbout(p->About());
    o->SetZipCode(p->Zipcode());
    PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER,
                                    USER_EXT, o->Uin()));
    gUserManager.DropOwner();
    if (e != NULL) ProcessDoneEvent(e);
    break;
  }

  case ICQ_CMDxRCV_UPDATExDETAILxFAIL:
  {
    if (!bMultiPacket) AckUDP(nSequence, nSubSequence);
    gLog.Info("%sFailed to update detail info.\n", L_UDPxSTR);
#if ICQ_VERSION == 2
    packet >> nSubSequence;
#endif
    ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_UPDATExDETAIL, nSubSequence, EVENT_FAILED);
    if (e != NULL) ProcessDoneEvent(e);
    break;
  }

  case ICQ_CMDxRCV_USERxINVALIDxUIN:  // not a good uin
    if (!bMultiPacket) AckUDP(nSequence, nSubSequence);
    packet >> nUin;
    gLog.Info("%sInvalid UIN: %d.\n", L_UDPxSTR, nUin);
    // we need to do something here, but I bet the command is included in the packet
    //emit signal_doneUserBasicInfo(false, nUin);
    break;

  case ICQ_CMDxRCV_USERxSTATUS:  // user changed status packet
  {
    if (!bMultiPacket) AckUDP(nSequence, nSubSequence);
    packet >> nUin;

    // find which user it is, verify we have them on our list
    ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
    char s[32];
    unsigned long nNewStatus;
    packet >> nNewStatus;
    ICQUser::StatusToStatusStr(nNewStatus, false, s);
    gLog.Info("%s%s (%ld) is now %s.\n", L_UDPxSTR,
              (u ? u->GetAlias() : "Unknown user"), nUin, s);
    ChangeUserStatus(u, nNewStatus);
    gUserManager.DropUser(u);
    u = gUserManager.FetchUser(nUin, LOCK_R);
    gUserManager.Reorder(u);
    gUserManager.DropUser(u);
    PushPluginSignal(new CICQSignal(SIGNAL_UPDATExLIST, LIST_REORDER, nUin));
    break;
  }

  case ICQ_CMDxRCV_USERxLISTxDONE:  // end of user list
    /* 02 00 1C 02 05 00 8F 76 20 00 */
    if (!bMultiPacket) AckUDP(nSequence, nSubSequence);
    gLog.Info("%sEnd of online users list.\n", L_UDPxSTR);
    // Possible race condition here...
    m_nAllowUpdateUsers--;
    PushPluginSignal(new CICQSignal(SIGNAL_UPDATExLIST, LIST_ALL, 0));
    break;

  case ICQ_CMDxRCV_SEARCHxFOUND:  // user found in search
  {
    /* 02 00 8C 00 03 00 05 00 8F 76 20 00 0B 00 41 70 6F 74 68 65 6F 73 69 73
       00 07 00 47 72 61 68 61 6D 00 05 00 52 6F 66 66 00 13 00 67 72 6F 66 66 40 75
       77 61 74 65 72 6C 6F 6F 2E 63 61 00 01 02 */
    if (!bMultiPacket) AckUDP(nSequence, nSubSequence);
    gLog.Info("%sSearch found user:\n", L_UDPxSTR);

    unsigned short i, aliasLen, firstNameLen, lastNameLen, emailLen;
    char auth;
    char szTemp[64];
#if ICQ_VERSION == 2
    packet >> nSubSequence;
#endif
    packet >> nUin;
    CSearchAck *s = new CSearchAck(nUin);
    // Alias
    packet >> aliasLen;
    for (i = 0; i < aliasLen; i++) packet >> szTemp[i];
    s->szAlias = strdup(szTemp);
    // First name
    packet >> firstNameLen;
    for (i = 0; i < firstNameLen; i++) packet >> szTemp[i];
    s->szFirstName = strdup(szTemp);
    // Last name
    packet >> lastNameLen;
    for (i = 0; i < lastNameLen; i++) packet >> szTemp[i];
    s->szLastName = strdup(szTemp);
    // Email
    packet >> emailLen;
    for (i = 0; i < emailLen; i++) packet >> szTemp[i];
    s->szEmail = strdup(szTemp);

    // translating string with Translation Table
    gTranslator.ServerToClient(s->szAlias);
    gTranslator.ServerToClient(s->szFirstName);
    gTranslator.ServerToClient(s->szLastName);

    packet >> auth;
    gLog.Info("%s%s (%ld) <%s %s, %s>\n", L_BLANKxSTR, s->szAlias, nUin,
              s->szFirstName, s->szLastName, s->szEmail);

    ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_SEARCHxSTART, nSubSequence, EVENT_ACKED);
    // We make as copy as each plugin will delete the events it gets
    if (e == NULL)
    {
      gLog.Warn("%sReceived search result when no search in progress.\n", L_WARNxSTR);
      delete s;
      break;
    }
    ICQEvent *e2 = new ICQEvent(e);
    e2->m_sSearchAck = s;
    e2->m_sSearchAck->cMore = 0;
    PushPluginEvent(e2);
    PushExtendedEvent(e);
    break;
  }

  case ICQ_CMDxRCV_SEARCHxDONE:  // user found in search
  {
    /* 02 00 A0 00 04 00 05 00 00*/
    if (!bMultiPacket) AckUDP(nSequence, nSubSequence);

    char more;
#if ICQ_VERSION == 2
    packet >> nSubSequence;
#endif
    packet >> more;
    gLog.Info("%sSearch finished.\n", L_UDPxSTR);
    ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_SEARCHxSTART, nSubSequence, EVENT_SUCCESS);
    if (e == NULL)
    {
      gLog.Warn("%sReceived end of search when no search in progress.\n", L_WARNxSTR);
      break;
    }
    e->m_sSearchAck = new CSearchAck(0);
    e->m_sSearchAck->cMore = more;
    ProcessDoneEvent(e);
    break;
  }

  case ICQ_CMDxRCV_SYSxMSGxDONE:  // end of system messages
  {
     /* 02 00 E6 00 04 00 50 A5 82 00 */
     if (!bMultiPacket) AckUDP(nSequence, nSubSequence);
     gLog.Info("%sEnd of system messages.\n", L_UDPxSTR);

#if ICQ_VERSION == 2
     CPU_SysMsgDoneAck *p = new CPU_SysMsgDoneAck(nSequence);
#elif ICQ_VERSION == 4
     CPU_SysMsgDoneAck *p = new CPU_SysMsgDoneAck(nSequence, nSubSequence);
#elif ICQ_VERSION == 5
     CPU_SysMsgDoneAck *p = new CPU_SysMsgDoneAck;
#endif
     gLog.Info("%sAcknowledging system messages (#%d)...\n", L_UDPxSTR, p->getSequence());
     SendExpectEvent(m_nUDPSocketDesc, p, CONNECT_NONE);
     break;
  }

  case ICQ_CMDxRCV_SYSxMSGxOFFLINE:  // offline system message
  {
    /* 02 00 DC 00 0A 00 EC C9 45 00 CE 07 04 17 04 21 01 00 3F 00 6E 6F 2C 20
       73 74 69 6C 6C 20 68 6F 70 69 6E 67 20 66 6F 72 20 74 68 65 20 72 65 63
       6F 72 64 20 63 6F 6D 70 61 6E 79 2C 20 62 75 74 20 79 6F 75 20 6E 65 76
       65 72 20 6B 6E 6F 77 2E 2E 2E 00 */
    if (!bMultiPacket) AckUDP(nSequence, nSubSequence);
    gLog.Info("%sOffline system message.\n", L_UDPxSTR);

    unsigned short yearSent, newCommand;
    char monthSent, daySent, hourSent, minSent;
    packet >> nUin
           >> yearSent
           >> monthSent
           >> daySent
           >> hourSent
           >> minSent
           >> newCommand
    ;
    // prepare a structure containing the relevant time information
    struct tm sentTime;
    sentTime.tm_sec  = 0;
    sentTime.tm_min  = minSent;
    sentTime.tm_hour = hourSent;
    sentTime.tm_mday = daySent;
    sentTime.tm_mon  = monthSent - 1;
    sentTime.tm_year = yearSent - 1900;

    // process the system message, sending the time it occured converted to a time_t structure
    ProcessSystemMessage(packet, nUin, newCommand, mktime(&sentTime));
    break;
  }

  case ICQ_CMDxRCV_SYSxMSGxONLINE:  // online system message
  {
    if (!bMultiPacket) AckUDP(nSequence, nSubSequence);
    gLog.Info("%sOnline system message.\n", L_UDPxSTR);
    unsigned short newCommand;
    packet >> nUin
           >> newCommand;
    // process the system message, sending the current time as a time_t structure
    ProcessSystemMessage(packet, nUin, newCommand, time(NULL));
    break;
  }

  case ICQ_CMDxRCV_META: // meta command
  {
    if (!bMultiPacket) AckUDP(nSequence, nSubSequence);
    unsigned short nMetaCommand;
    char nMetaResult;
    packet >> nMetaCommand
           >> nMetaResult;
    gLog.Info("%sMeta command (%d) response (#%d) - %s.\n", L_UDPxSTR,
              nMetaCommand, nSubSequence,
              nMetaResult == META_SUCCESS ? "success" : "failed");
    ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_META, nSubSequence,
                                    nMetaResult == META_SUCCESS ? EVENT_SUCCESS : EVENT_FAILED);
    if (e == NULL)
    {
      gLog.Warn("%sReceived meta result for unknown meta command.\n", L_WARNxSTR);
      break;
    }
    if (nMetaResult != META_SUCCESS)
      ProcessDoneEvent(e);
    else
    {
      ProcessMetaCommand(packet, nMetaCommand, e);
      if (e->m_nSubResult == META_DONE)
        ProcessDoneEvent(e);
      else
        PushExtendedEvent(e);
    }
    break;
  }

  case ICQ_CMDxRCV_SETxOFFLINE:  // we got put offline by mirabilis for some reason
    gLog.Info("%sKicked offline by server.\n", L_UDPxSTR);
    icqRelogon();
    break;

  case ICQ_CMDxRCV_ACK:  // icq acknowledgement
  {
    /* 02 00 0A 00 12 00 */
    gLog.Info("%sAck (#%d).\n", L_UDPxSTR, nSequence);
#if ICQ_VERSION == 5
    pthread_mutex_lock(&mutex_serverack);
    m_nServerAck = nSequence;
    pthread_cond_broadcast(&cond_serverack);
    pthread_mutex_unlock(&mutex_serverack);
#endif
    ICQEvent *e = DoneEvent(m_nUDPSocketDesc, nSequence, EVENT_ACKED);
    if (e != NULL) ProcessDoneEvent(e);
    break;
  }

  case ICQ_CMDxRCV_ERROR:  // icq says go away
  {
    gLog.Info("%sServer says you are not logged on (#%d).\n", L_UDPxSTR, nSequence);
    ICQEvent *e = DoneEvent(m_nUDPSocketDesc, nSequence, EVENT_FAILED);
    if (e != NULL) ProcessDoneEvent(e);
    icqRelogon();
    break;
  }

  case ICQ_CMDxRCV_HELLO: // hello packet from mirabilis received on logon
  {
    /* 02 00 5A 00 00 00 8F 76 20 00 CD CD 76 10 02 00 01 00 05 00 00 00 00 00
       8C 00 00 00 F0 00 0A 00 0A 00 05 00 0A 00 01 */
    if (!bMultiPacket) AckUDP(nSequence, nSubSequence);
    gLog.Info("%sMirabilis says hello.\n", L_UDPxSTR);

    ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
    ChangeUserStatus(o, m_nDesiredStatus);
    gUserManager.DropOwner();

    m_eStatus = STATUS_ONLINE;
    ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_LOGON, 0, EVENT_SUCCESS);
    if (e != NULL) ProcessDoneEvent(e);
    PushPluginSignal(new CICQSignal(SIGNAL_LOGON, 0, 0));

    icqUpdateContactList();
    icqSendInvisibleList();
    icqSendVisibleList();
#if ICQ_VERSION != 5
    icqRequestSystemMsg();
#endif

    // Send an update status packet to force hideip/webpresence
    if (m_nDesiredStatus & ICQ_STATUS_FxFLAGS) icqSetStatus(m_nDesiredStatus);
    break;
  }

  case ICQ_CMDxRCV_WRONGxPASSWD:  // incorrect password sent in logon
  {
    /* 02 00 64 00 00 00 02 00 8F 76 20 00 */
    gLog.Error("%sIncorrect password.\n", L_ERRORxSTR);
    m_eStatus = STATUS_OFFLINE_FORCED;
    ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_LOGON, 0, EVENT_FAILED);
    if (e != NULL) ProcessDoneEvent(e);
    break;
  }

  case ICQ_CMDxRCV_BUSY:  // server too busy to respond
  {
    gLog.Info("%sServer busy, try again in a few minutes.\n", L_UDPxSTR);
    m_eStatus = STATUS_OFFLINE_FORCED;
    ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_LOGON, 0, EVENT_FAILED);
    if (e != NULL) ProcessDoneEvent(e);
    break;
  }

  case ICQ_CMDxRCV_NEWxUIN:  // received a new uin
  {
#if ICQ_VERSION == 2
    unsigned short nTemp;
    packet >> nTemp >> nOwnerUin;
#endif
    gLog.Info("%sReceived new uin: %d\n", L_UDPxSTR, nOwnerUin);
    gUserManager.SetOwnerUin(nOwnerUin);
    ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_REGISTERxUSER, 0, EVENT_SUCCESS);
    if (e != NULL) ProcessDoneEvent(e);
    // Logon as an ack
    icqLogon(ICQ_STATUS_ONLINE);
    break;
  }

  default:  // what the heck is this packet?  print it out
    if (!bMultiPacket) AckUDP(nSequence, nSubSequence);
    char *buf;
    gLog.Unknown("%sUnknown UDP packet:\n%s\n", L_UNKNOWNxSTR, packet.print(buf));
    delete buf;
    break;
  }

  return(nCommand);
}


//-----CICQDaemon::ProcessSystemMessage-----------------------------------------
void CICQDaemon::ProcessSystemMessage(CBuffer &packet, unsigned long nUin,
                                      unsigned short newCommand, time_t timeSent)
{
  ICQUser *u;
  int i, j;
  char c;

  if (nUin == 0)
  {
    char *buf;
    gLog.Unknown("%sInvalid system message (UIN = 0?):\n%s\n", L_UNKNOWNxSTR, packet.print(buf));
    delete buf;
  }

  // Check if uin is backwards, what the fuck is with icq99b?
  if (nUin > 0x07FFFFFF)
  {
    nUin = ((nUin & 0x000000FF) << 24) + ((nUin & 0x0000FF00) << 8) +
           ((nUin & 0x00FF0000) >> 8)  + ((nUin & 0xFF000000) >> 24);
  }
  // Swap high and low bytes for strange new icq99
  if ((newCommand > 0x00FF) && !(newCommand & ICQ_CMDxSUB_FxMULTIREC))
  {
    newCommand = ((newCommand & 0xFF00) >> 8) + ((newCommand & 0x00FF) << 8);
  }

  unsigned long nMask = ((newCommand & ICQ_CMDxSUB_FxMULTIREC) ? E_MULTIxREC : 0);
  newCommand &= ~ICQ_CMDxSUB_FxMULTIREC;

  // Read out the data
  unsigned short nLen;
  packet >> nLen;

  // read in the user data from the packet
  char szMessage[nLen + 1];
  for (i = 0, j = 0; i < nLen; i++)
  {
    packet >> c;
    if (c != 0x0D) szMessage[j++] = c;
  }

  switch(newCommand)
  {
  case ICQ_CMDxSUB_MSG:  // system message: message through the server
  {
    // translating string with Translation Table
    gTranslator.ServerToClient (szMessage);
    CEventMsg *e = new CEventMsg(szMessage, ICQ_CMDxRCV_SYSxMSGxONLINE,
                                 timeSent, nMask);

    // Lock the user to add the message to their queue
    u = gUserManager.FetchUser(nUin, LOCK_W);
    if (u == NULL)
    {
      if (Ignore(IGNORE_NEWUSERS))
      {
        gLog.Info("%sMessage from new user (%ld), ignoring.\n", L_SBLANKxSTR, nUin);
        RejectEvent(nUin, e);
        break;
      }
      gLog.Info("%sMessage from new user (%ld).\n",
                L_SBLANKxSTR, nUin);
      AddUserToList(nUin);
      u = gUserManager.FetchUser(nUin, LOCK_W);
    }
    else
      gLog.Info("%sMessage through server from %s (%ld).\n", L_SBLANKxSTR,
                u->GetAlias(), nUin);

    if (AddUserEvent(u, e))
    {
      m_xOnEventManager.Do(ON_EVENT_MSG, u);
      u->Unlock();
      // We only want a read lock because Reorder takes a write lock
      // on the group, so there would be a potential race condition
      // if we have a group and user write locked at the same time
      u->Lock(LOCK_R);
      gUserManager.Reorder(u);
      gUserManager.DropUser(u);
      PushPluginSignal(new CICQSignal(SIGNAL_UPDATExLIST, LIST_REORDER, nUin));
    }
    else
      gUserManager.DropUser(u);
    break;
  }
  case ICQ_CMDxSUB_URL:  // system message: url through the server
  {
    // parse the message into url and url description
    char **szUrl = new char*[2];  // description, url
    ParseFE(szMessage, &szUrl, 2);

    // translating string with Translation Table
    gTranslator.ServerToClient (szUrl[0]);
    CEventUrl *e = new CEventUrl(szUrl[1], szUrl[0],
                                 ICQ_CMDxRCV_SYSxMSGxONLINE, timeSent, nMask);

    u = gUserManager.FetchUser(nUin, LOCK_W);
    if (u == NULL)
    {
      if (Ignore(IGNORE_NEWUSERS))
      {
        gLog.Info("%sURL from new user (%ld), ignoring.\n", L_SBLANKxSTR, nUin);
        RejectEvent(nUin, e);
        delete []szUrl;
        break;
      }
      gLog.Info("%sURL from new user (%ld).\n",
                L_SBLANKxSTR, nUin);
      AddUserToList(nUin);
      u = gUserManager.FetchUser(nUin, LOCK_W);
    }
    else
      gLog.Info("%sURL through server from %s (%ld).\n", L_SBLANKxSTR,
                u->GetAlias(), nUin);

    if (AddUserEvent(u, e))
    {
      m_xOnEventManager.Do(ON_EVENT_URL, u);
      u->Unlock();
      u->Lock(LOCK_R);
      gUserManager.Reorder(u);
      gUserManager.DropUser(u);
      PushPluginSignal(new CICQSignal(SIGNAL_UPDATExLIST, LIST_REORDER, nUin));
    }
    else
      gUserManager.DropUser(u);
    delete[] szUrl;
    break;
  }
  case ICQ_CMDxSUB_REQxAUTH:  // system message: authorisation request
  {
     /* 02 00 04 01 08 00 8F 76 20 00 06 00 41 00 41 70 6F 74 68 65 6F 73 69 73
        FE 47 72 61 68 61 6D FE 52 6F 66 66 FE 67 72 6F 66 66 40 75 77 61 74 65
        72 6C 6F 6F 2E 63 61 FE 31 FE 50 6C 65 61 73 65 20 61 75 74 68 6F 72 69
        7A 65 20 6D 65 2E 00 */

     gLog.Info("%sAuthorization request from %ld.\n", L_SBLANKxSTR, nUin);

     char **szFields = new char*[6];  // alias, first name, last name, email, auth, reason
     ParseFE(szMessage, &szFields, 6);

     // translating string with Translation Table
     gTranslator.ServerToClient (szFields[0]);
     gTranslator.ServerToClient (szFields[1]);
     gTranslator.ServerToClient (szFields[2]);
     gTranslator.ServerToClient (szFields[5]);

     CEventAuthReq *e = new CEventAuthReq(nUin, szFields[0], szFields[1],
                                          szFields[2], szFields[3], szFields[5],
                                          ICQ_CMDxRCV_SYSxMSGxONLINE, timeSent, 0);
     ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
     AddUserEvent(o, e);
     gUserManager.DropOwner();
     e->AddToHistory(NULL, D_RECEIVER);
     m_xOnEventManager.Do(ON_EVENT_SYSMSG, NULL);
     delete[] szFields;
     break;
  }
  case ICQ_CMDxSUB_AUTHORIZED:  // system message: authorized
  {
     gLog.Info("%sAuthorization from %ld.\n", L_SBLANKxSTR, nUin);

     // translating string with Translation Table
     gTranslator.ServerToClient (szMessage);

     CEventAuth *e = new CEventAuth(nUin, szMessage, ICQ_CMDxRCV_SYSxMSGxONLINE,
                                    timeSent, 0);
     ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
     AddUserEvent(o, e);
     gUserManager.DropOwner();
     e->AddToHistory(NULL, D_RECEIVER);
     m_xOnEventManager.Do(ON_EVENT_SYSMSG, NULL);
     break;
  }
  case ICQ_CMDxSUB_ADDEDxTOxLIST:  // system message: added to a contact list
  {
     gLog.Info("%sUser %ld added you to their contact list.\n", L_SBLANKxSTR,
               nUin);

     char **szFields = new char*[5]; // alias, first name, last name, email, auth
     ParseFE(szMessage, &szFields, 5);

     // translating string with Translation Table
     gTranslator.ServerToClient(szFields[0]);  // alias
     gTranslator.ServerToClient(szFields[1]);  // first name
     gTranslator.ServerToClient(szFields[2]);  // last name
     gLog.Info("%s%s (%s %s), %s\n", L_UDPxSTR, szFields[0], szFields[1],
              szFields[2], szFields[3]);

     CEventAdded *e = new CEventAdded(nUin, szFields[0], szFields[1],
                                      szFields[2], szFields[3],
                                      ICQ_CMDxRCV_SYSxMSGxONLINE, timeSent, 0);
     ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
     AddUserEvent(o, e);
     gUserManager.DropOwner();
     e->AddToHistory(NULL, D_RECEIVER);
     m_xOnEventManager.Do(ON_EVENT_SYSMSG, NULL);
     delete[] szFields;
     break;
  }
  case ICQ_CMDxSUB_WEBxPANEL:
  {
    /* 02 00 04 01 28 00 0A 00 00 00 0D 00 49 00 6D 79 20 6E 61 6D 65 FE FE FE
       65 6D 61 69 6C FE 33 FE 53 65 6E 64 65 72 20 49 50 3A 20 32 30 39 2E 32
       33 39 2E 36 2E 31 33 0D 0A 53 75 62 6A 65 63 74 3A 20 73 75 62 6A 65 63
       74 0D 0A 74 68 65 20 6D 65 73 73 61 67 65 00 */
    gLog.Info("%sMessage through web panel.\n", L_SBLANKxSTR);

    char **szFields = new char*[6]; // name, ?, ?, email, ?, message
    ParseFE(szMessage, &szFields, 6);

    gTranslator.ServerToClient(szFields[0]);
    gTranslator.ServerToClient(szFields[5]);

    gLog.Info("%sFrom %s (%s).\n", L_SBLANKxSTR, szFields[0], szFields[3]);
    CEventWebPanel *e = new CEventWebPanel(szFields[0], szFields[3], szFields[5],
                                           ICQ_CMDxRCV_SYSxMSGxONLINE, timeSent, 0);
    ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
    if (AddUserEvent(o, e))
    {
      gUserManager.DropOwner();
      e->AddToHistory(NULL, D_RECEIVER);
      m_xOnEventManager.Do(ON_EVENT_SYSMSG, NULL);
    }
    else
      gUserManager.DropOwner();
    delete[] szFields;
    break;
  }
  case ICQ_CMDxSUB_EMAILxPAGER:
  {
    /* 02 00 04 01 07 00 0A 00 00 00 0E 00 5C 00 73 61 6D 40 65 75 2E 6F 72 67
       FE FE FE 73 61 6D 40 65 75 2E 6F 72 67 FE 33 FE 53 75 62 6A 65 63 74 3A
       20 5B 53 4C 41 53 48 44 4F 54 5D 0D 0A 5B 54 68 65 20 49 6E 74 65 72 6E
       65 74 5D 20 45 6E 6F 72 6D 6F 75 73 20 38 30 73 20 54 65 78 74 66 69 6C
       65 20 41 72 63 68 69 76 65 00 */
    gLog.Info("%sEmail pager message.\n", L_SBLANKxSTR);

    char **szFields = new char*[6]; // name, ?, ?, email, ?, message
    ParseFE(szMessage, &szFields, 6);

    gTranslator.ServerToClient(szFields[0]);
    gTranslator.ServerToClient(szFields[5]);

    gLog.Info("%sFrom %s (%s).\n", L_SBLANKxSTR, szFields[0], szFields[3]);
    CEventEmailPager *e = new CEventEmailPager(szFields[0], szFields[3], szFields[5],
                                               ICQ_CMDxRCV_SYSxMSGxONLINE, timeSent, 0);
    ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
    if (AddUserEvent(o, e))
    {
      gUserManager.DropOwner();
      e->AddToHistory(NULL, D_RECEIVER);
      m_xOnEventManager.Do(ON_EVENT_SYSMSG, NULL);
    }
    else
      gUserManager.DropOwner();
    delete[] szFields;
    break;
  }
  case ICQ_CMDxSUB_CONTACTxLIST:
  {
    /* 02 00 DC 00 1A 00 23 64 84 00 CF 07 06 15 13 2B 13 00 1E 00 32 FE 37 33
       39 37 38 35 33 FE 46 6C 75 6E 6B 69 FE 37 33 35 37 32 31 39 FE 55 68 75
       FE 00 */
    gLog.Info("%sContact list.\n", L_SBLANKxSTR);

    i = 0;
    while ((unsigned char)szMessage[i++] != 0xFE);
    szMessage[--i] = '\0';
    int nNumContacts = atoi(szMessage);
    char **szFields = new char*[nNumContacts * 2 + 1];
    ParseFE(&szMessage[++i], &szFields, nNumContacts * 2 + 1);

    // Translate the aliases
    vector <char *> vszFields;
    for (i = 0; i < nNumContacts * 2; i += 2)
    {
      vszFields.push_back(szFields[i]);  // uin
      gTranslator.ServerToClient(szFields[i + 1]); // alias
      vszFields.push_back(szFields[i + 1]);
    }

    gLog.Info("%s%s contacts.\n", L_SBLANKxSTR, szMessage);
    CEventContactList *e = new CEventContactList(vszFields, ICQ_CMDxRCV_SYSxMSGxONLINE, timeSent, 0);
    ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
    AddUserEvent(o, e);
    gUserManager.DropOwner();
    e->AddToHistory(NULL, D_RECEIVER);
    m_xOnEventManager.Do(ON_EVENT_SYSMSG, NULL);
    delete[] szFields;
    break;
  }
  default:
  {
    char *szFE;
    while((szFE = strchr(szMessage, 0xFE)) != NULL) *szFE = '\n';

    char *buf;
    gLog.Unknown("%sUnknown system message (0x%04x):\n%s\n", L_UNKNOWNxSTR,
                 newCommand, packet.print(buf));
    delete [] buf;
    CEventUnknownSysMsg *e =
      new CEventUnknownSysMsg(newCommand, ICQ_CMDxRCV_SYSxMSGxONLINE,
                              nUin, szMessage, timeSent, 0);
    ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
    AddUserEvent(o, e);
    gUserManager.DropOwner();
  }
  } // switch
}



//-----CICQDaemon::ProcessMetaCommand-----------------------------------------
void CICQDaemon::ProcessMetaCommand(CBuffer &packet,
                                    unsigned short nMetaCommand,
                                    ICQEvent *e)
{
  ICQUser *u = NULL;
  char szTemp[MAX_DATA_LEN];
  unsigned long nUin;

  switch(nMetaCommand)
  {
    case ICQ_CMDxMETA_MORExINFO:
    case ICQ_CMDxMETA_WORKxINFO:
    case ICQ_CMDxMETA_GENERALxINFO:
    case ICQ_CMDxMETA_ABOUT:
    case ICQ_CMDxMETA_UNKNOWNx240:
    case ICQ_CMDxMETA_UNKNOWNx250:
    case ICQ_CMDxMETA_UNKNOWNx270:
    {
      e->m_nSubResult += nMetaCommand;
      nUin = ((CPU_Meta_RequestInfo *)e->m_xPacket)->Uin();
      u = gUserManager.FetchUser(nUin, LOCK_W);
      if (u == NULL)
      {
        gLog.Warn("%sReceived meta information on deleted user (%ld).\n",
                  L_WARNxSTR, nUin);
        break;
      }
      switch (nMetaCommand)
      {
        case ICQ_CMDxMETA_GENERALxINFO:
        {
          gLog.Info("%sGeneral info on %s (%ld).\n", L_SBLANKxSTR, u->GetAlias(), u->Uin());
          u->SetEnableSave(false);
          u->SetAlias(packet.UnpackString(szTemp));
          u->SetFirstName(packet.UnpackString(szTemp));
          u->SetLastName(packet.UnpackString(szTemp));
          u->SetEmail1(packet.UnpackString(szTemp));
          u->SetEmail2(packet.UnpackString(szTemp));
          // Old email address
          packet.UnpackString(szTemp);
          u->SetCity(packet.UnpackString(szTemp));
          u->SetState(packet.UnpackString(szTemp));
          u->SetPhoneNumber(packet.UnpackString(szTemp));
          u->SetFaxNumber(packet.UnpackString(szTemp));
          u->SetAddress(packet.UnpackString(szTemp));
          u->SetCellularNumber(packet.UnpackString(szTemp));
          u->SetZipCode(packet.UnpackUnsignedLong());
          u->SetCountryCode(packet.UnpackUnsignedShort());
          u->SetTimezone(packet.UnpackChar());
          u->SetAuthorization(!packet.UnpackChar());
          // Unknown (web panel ?)
          packet.UnpackChar();
          u->SetHideEmail(packet.UnpackChar());


         // translating string with Translation Table
         gTranslator.ServerToClient(u->GetAlias());
         gTranslator.ServerToClient(u->GetFirstName());
         gTranslator.ServerToClient(u->GetLastName());
         gTranslator.ServerToClient(u->GetCity());
         gTranslator.ServerToClient(u->GetState());
         gTranslator.ServerToClient(u->GetPhoneNumber());
         gTranslator.ServerToClient(u->GetFaxNumber());
         gTranslator.ServerToClient(u->GetCellularNumber());
         gTranslator.ServerToClient(u->GetAddress());

          u->SetEnableSave(true);
          u->SaveGeneralInfo();
          PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER,
                                          USER_GENERAL, nUin));
          break;
        }
        case ICQ_CMDxMETA_WORKxINFO:
        {
          gLog.Info("%sWork info on %s (%ld).\n", L_SBLANKxSTR, u->GetAlias(), u->Uin());
          u->SetEnableSave(false);
          u->SetCompanyCity(packet.UnpackString(szTemp));
          u->SetCompanyState(packet.UnpackString(szTemp));
          u->SetCompanyPhoneNumber(packet.UnpackString(szTemp));
          u->SetCompanyFaxNumber(packet.UnpackString(szTemp));
          u->SetCompanyAddress(packet.UnpackString(szTemp));
          packet.UnpackUnsignedLong();
          packet.UnpackUnsignedShort();
          u->SetCompanyName(packet.UnpackString(szTemp));
          u->SetCompanyDepartment(packet.UnpackString(szTemp));
          u->SetCompanyPosition(packet.UnpackString(szTemp));
          packet.UnpackUnsignedShort();
          u->SetCompanyHomepage(packet.UnpackString(szTemp));

          // translating string with Translation Table
          gTranslator.ServerToClient(u->GetCompanyCity());
          gTranslator.ServerToClient(u->GetCompanyState());
          gTranslator.ServerToClient(u->GetCompanyAddress());
          gTranslator.ServerToClient(u->GetCompanyPhoneNumber());
          gTranslator.ServerToClient(u->GetFaxNumber());
          gTranslator.ServerToClient(u->GetCompanyName());
          gTranslator.ServerToClient(u->GetCompanyDepartment());
          gTranslator.ServerToClient(u->GetCompanyPosition());


          u->SetEnableSave(true);
          u->SaveWorkInfo();
          PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER,
                                          USER_WORK, nUin));
          break;
        }
        case ICQ_CMDxMETA_MORExINFO:
        {
          gLog.Info("%sMore info on %s (%ld).\n", L_SBLANKxSTR, u->GetAlias(), u->Uin());
          u->SetEnableSave(false);
          u->SetAge(packet.UnpackUnsignedShort());
          u->SetGender(packet.UnpackChar());
          u->SetHomepage(packet.UnpackString(szTemp));
          u->SetBirthYear(packet.UnpackChar());
          u->SetBirthMonth(packet.UnpackChar());
          u->SetBirthDay(packet.UnpackChar());
          u->SetLanguage1(packet.UnpackChar());
          u->SetLanguage2(packet.UnpackChar());
          u->SetLanguage3(packet.UnpackChar());
          u->SetEnableSave(true);
          u->SaveWorkInfo();
          PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER,
                                          USER_MORE, nUin));
          break;
        }
        case ICQ_CMDxMETA_ABOUT:
        {
          gLog.Info("%sAbout info on %s (%ld).\n", L_SBLANKxSTR, u->GetAlias(), u->Uin());
          u->SetAbout(packet.UnpackString(szTemp));
          PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER,
                                          USER_ABOUT, nUin));
          break;
        }
      }
      gUserManager.DropUser(u);
      break;
    }

    case ICQ_CMDxMETA_GENERALxINFOxRSP:
    case ICQ_CMDxMETA_MORExINFOxRSP:
    case ICQ_CMDxMETA_WORKxINFOxRSP:
    case ICQ_CMDxMETA_ABOUTxRSP:
      e->m_nSubResult = META_DONE;
      break;
    case ICQ_CMDxMETA_SECURITYxRSP:
    {
      /*CPU_Meta_SetSecurityInfo *p = (CPU_Meta_SetSecurityInfo *)e->m_xPacket;
      u->SetEnableSave(false);
      u->SetAuthorization(p->Authorization());
      p->SetWebAware(p->WebAware());
      p->SetHideIp(p->HideIp());
      u->SetEnableSave(true);
      u->SaveLicqInfo();
      */
      e->m_nSubResult = META_DONE;
      break;
    }
    case ICQ_CMDxMETA_PASSWORDxRSP:
    {
      /*u->SetEnableSave(false);
      u->SetPassword( ((CPU_Meta_SetPassword)e->m_xPacket)->Password());
      u->SetEnableSave(true);
      u->SaveLicqInfo();
      e->m_nSubResult = META_DONE;
      */
      break;
    }
    default:
    {
      char *buf;
      gLog.Unknown("%sUnknown meta command (0x%04x):\n%s\n", L_UNKNOWNxSTR,
                   nMetaCommand, packet.print(buf));
      delete [] buf;
      break;
    }
  }

}


//-----CICQDaemon::AckUDP--------------------------------------------------------------
void CICQDaemon::AckUDP(unsigned short _nSequence, unsigned short _nSubSequence)
// acknowledge whatever packet we received using the relevant sequence number
{
#if ICQ_VERSION == 2
   CPU_Ack p(_nSequence);
#elif ICQ_VERSION == 4 || ICQ_VERSION == 5
   CPU_Ack p(_nSequence, _nSubSequence);
#endif
   SendEvent(m_nUDPSocketDesc, p);
}


