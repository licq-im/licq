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
  icqLogoff(false);
  gSocketManager.CloseSocket(m_nUDPSocketDesc);
  m_nUDPSocketDesc = -1;
  icqServers.next();
}


void CICQDaemon::icqRegister(const char *_szPasswd)
{
  ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
  o->setPassword(_szPasswd);
  gUserManager.DropOwner();
  CPacketRegister *p = new CPacketRegister(_szPasswd);
  gLog.Info("%sRegistering a new user (#%d)...\n", L_UDPxSTR, p->getSequence());
  SendExpectEvent(m_nUDPSocketDesc, p, CONNECT_SERVER);
}


//-----ICQ::Logon---------------------------------------------------------------
ICQEvent *CICQDaemon::icqLogon(unsigned long logonStatus)
{
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  if (o->getUin() == 0)
  {
    gUserManager.DropOwner();
    gLog.Error("%sNo registered user, unable to process logon attempt.\n", L_ERRORxSTR);
    return NULL;
  }
  char *passwd = strdup(o->getPassword());
  gUserManager.DropOwner();
  INetSocket *s = gSocketManager.FetchSocket(m_nTCPSocketDesc);
  if (s == NULL) return NULL;
  CPU_Logon *p = new CPU_Logon(s, passwd, logonStatus);
  gSocketManager.DropSocket(s);
  free (passwd);
  gLog.Info("%sRequesting logon (#%d)...\n", L_UDPxSTR, p->getSequence());
  m_nDesiredStatus = logonStatus;
  return SendExpectEvent(m_nUDPSocketDesc, p, CONNECT_SERVER);
}


//-----ICQ::icqLogoff-----------------------------------------------------------
void CICQDaemon::icqLogoff(bool reconnect)
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

  // Cancel all open events
  list<ICQEvent *>::iterator iter;
  pthread_mutex_lock(&mutex_pendingevents);
  iter = m_lxPendingEvents.begin();
  while (iter != m_lxPendingEvents.end())
  {
    if ((*iter)->m_nSocketDesc == nSD)
    {
      delete *iter;
      iter = m_lxPendingEvents.erase(iter);
    }
    else
      iter++;
  }
  pthread_mutex_unlock(&mutex_pendingevents);
  pthread_mutex_lock(&mutex_runningevents);
  iter = m_lxRunningEvents.begin();
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

  if (reconnect)
  {
    ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
    unsigned long status = o->getStatusFull();
    gUserManager.DropOwner();
    icqLogon(status);
  }
/*  else
  {
    // reset all the users statuses
    ICQUser *u;
    CUserGroup *g = gUserManager.FetchGroup(0, LOCK_R);
    for (unsigned short i = 0; i < g->NumUsers(); i++)
    {
      u = g->FetchUser(i, LOCK_W);
      ChangeUserStatus(u, ICQ_STATUS_OFFLINE);
      g->DropUser(u);
    }
    gUserManager.DropGroup(g);
    PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSERS, NULL, NULL));
  }*/
  ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
  ChangeUserStatus(o, ICQ_STATUS_OFFLINE);
  gUserManager.DropOwner();

  /*PushPluginSignal(new CICQSignal(SIGNAL_UPDATExOWNER,
                                  new unsigned short(UPDATE_STATUS),
                                  NULL));*/
}


//-----icqUpdateContactList-----------------------------------------------------
void CICQDaemon::icqUpdateContactList(void)
{
  m_nAllowUpdateUsers = 0;
  CUserGroup *g = gUserManager.FetchGroup(0, LOCK_R);
  unsigned short numPackets = 1 + (g->NumUsers() / m_nMaxUsersPerPacket);
  for (unsigned short i = 0; i < numPackets; i++)
  {
    CPU_ContactList *p = new CPU_ContactList(g, i * m_nMaxUsersPerPacket, m_nMaxUsersPerPacket);
    gLog.Info("%sUpdating contact list (#%d)...\n", L_UDPxSTR, p->getSequence());
    SendExpectEvent(m_nUDPSocketDesc, p, CONNECT_NONE);
    m_nAllowUpdateUsers++;
  }

  // reset all the users statuses
  ICQUser *u;
  int nNumUsers = g->NumUsers();
  for (unsigned short i = 0; i < nNumUsers; i++)
  {
    u = g->FetchUser(i, LOCK_W);
    if (!u->getStatusOffline()) ChangeUserStatus(u, ICQ_STATUS_OFFLINE);
    g->DropUser(u);
  }

  gUserManager.DropGroup(g);
  PushPluginSignal(new CICQSignal(SIGNAL_UPDATExLIST, LIST_ALL, 0));
}


//-----icqSendVisibleList-------------------------------------------------------
void CICQDaemon::icqSendVisibleList(bool _bSendIfEmpty = false)
{
  // send user info packet
  CUserGroup *g = gUserManager.FetchGroup(0, LOCK_R);
  CPU_VisibleList *p = new CPU_VisibleList(g);
  gUserManager.DropGroup(g);
  if (!p->empty() || _bSendIfEmpty)
  {
    gLog.Info("%sSending visible list (#%d)...\n", L_UDPxSTR, p->getSequence());
    SendExpectEvent(m_nUDPSocketDesc, p, CONNECT_NONE);
  }
  else
    delete p;
}


//-----icqSendInvisibleList-----------------------------------------------------
void CICQDaemon::icqSendInvisibleList(bool _bSendIfEmpty = false)
{
  CUserGroup *g = gUserManager.FetchGroup(0, LOCK_R);
  CPU_InvisibleList *p = new CPU_InvisibleList(g);
  gUserManager.DropGroup(g);
  if (!p->empty() || _bSendIfEmpty)
  {
    gLog.Info("%sSending invisible list (#%d)...\n", L_UDPxSTR, p->getSequence());
    SendExpectEvent(m_nUDPSocketDesc, p, CONNECT_NONE);
  }
  else
    delete p;
}


//-----icqStartSearch-----------------------------------------------------------
unsigned short CICQDaemon::icqStartSearch(const char *nick, const char *first,
                                          const char *last, const char *email)
{
  CPU_StartSearch *p = new CPU_StartSearch(nick, first, last, email);
  gLog.Info("%sStarting search for user (#%d)...\n", L_UDPxSTR,
            p->getSequence());
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
void CICQDaemon::icqSetStatus(unsigned long newStatus)
{
  CPU_SetStatus *p = new CPU_SetStatus(newStatus);
  gLog.Info("%sChanging status (#%d)...\n", L_UDPxSTR, p->getSequence());
  SendExpectEvent(m_nUDPSocketDesc, p, CONNECT_NONE);
  m_nDesiredStatus = newStatus;
}


//-----icqGetUserBasicInfo------------------------------------------------------
ICQEvent *CICQDaemon::icqUserBasicInfo(unsigned long _nUin)
{
  CPU_GetUserBasicInfo *p = new CPU_GetUserBasicInfo(_nUin);
  gLog.Info("%sRequesting user info (#%d)...\n", L_UDPxSTR, p->getSequence());
  return (SendExpectEvent(m_nUDPSocketDesc, p, CONNECT_NONE));
}


//-----icqGetUserExtInfo--------------------------------------------------------
ICQEvent *CICQDaemon::icqUserExtendedInfo(unsigned long _nUin)
{
  CPU_GetUserExtInfo *p = new CPU_GetUserExtInfo(_nUin);
  gLog.Info("%sRequesting extended user info (#%d)...\n", L_UDPxSTR,
            p->getSequence());
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
  gLog.Info("%sUpdating personal information (#%d)...\n", L_UDPxSTR,
            p->getSequence());
  return (SendExpectEvent(m_nUDPSocketDesc, p, CONNECT_NONE));
}


//-----icqUpdatePersonalExtInfo-------------------------------------------------
ICQEvent *CICQDaemon::icqUpdateExtendedInfo(const char *_sCity, unsigned short _nCountry,
                                   const char *_sState, unsigned short _nAge,
                                   char _cSex, const char *_sPhone,
                                   const char *_sHomepage, const char *_sAbout)
{
  CPU_UpdatePersonalExtInfo *p =
    new CPU_UpdatePersonalExtInfo(_sCity, _nCountry, _sState, _nAge, _cSex,
                                  _sPhone, _sHomepage, _sAbout);
   gLog.Info("%sUpdating personal extended info (#%d)...\n", L_UDPxSTR,
             p->getSequence());
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
unsigned short CICQDaemon::ProcessUdpPacket(CBuffer &packet)
{
  unsigned short version, command, theSequence, messageLen, junkShort;
  unsigned long nUin, junkLong;
  char junkChar;
  //CBuffer newPacket;

  // read in the standard UDP header info
  packet >> version
         >> command
         >> theSequence
  ;

  if (version != ICQ_VERSION)
  {
    gLog.Warn("%sServer send bad version number: %d.\n", L_WARNxSTR, version);
    return(0xFFFF);
  }

  switch (command)
  {
  case ICQ_CMDxRCV_USERxONLINE:   // initial user status packet
  {
    /* 02 00 6E 00 0B 00 8F 76 20 00 CD CD 77 90 3F 50 00 00 7F 00 00 01 04 00
       00 00 00 03 00 00 00 */
    AckUDP(theSequence);
    packet >> nUin;

    // find which user it is, verify we have them on our list
    ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
    if (u == NULL)
    {
       gLog.Warn("%sUnknown user (%ld) is online.\n", L_WARNxSTR, nUin);
       break;
    }
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    gLog.Info("%s%s (%ld) went online at %d:%02d.\n", L_UDPxSTR, u->getAlias(),
              nUin, t->tm_hour, t->tm_min);;

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
    u->setAwayMessage(NULL);
    if (u->getOnlineNotify()) m_xOnEventManager.Do(ON_EVENT_NOTIFY, u);
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
    AckUDP(theSequence);
    packet >> nUin;

    // find which user it is, verify we have them on our list
    ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
    if (u == NULL)
    {
       gLog.Warn("%sUnknown user (%ld) has gone offline.\n", L_WARNxSTR, nUin);
       break;
    }
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    gLog.Info("%s%s (%ld) went offline at %d:%02d.\n", L_UDPxSTR, u->getAlias(),
              nUin, t->tm_hour, t->tm_min);
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
    AckUDP(theSequence);
    unsigned short checkSequence;
    packet >> checkSequence  // corresponds to the sequence number from the user information request packet...totally irrelevant.
           >> nUin;

    // find which user it is, verify we have them on our list
    ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
    if (u == NULL)
    {
       gLog.Warn("%sInformation on unknown user (%d).\n", L_WARNxSTR, nUin);
       break;
    }
    gLog.Info("%sReceived information for %s (%ld):\n", L_UDPxSTR,
              u->getAlias(), nUin);

    // read in the four data fields; alias, first name, last name, and email address
    u->setEnableSave(false);
    char temp[MAX_DATA_LEN], cAuthorization;
    int i;
    packet >> messageLen;
    for (i = 0; i < messageLen; i++) packet >> temp[i];
    u->setAlias(temp);
    packet >> messageLen;
    for (i = 0; i < messageLen; i++) packet >> temp[i];
    u->setFirstName(temp);
    packet >> messageLen;
    for (i = 0; i < messageLen; i++) packet >> temp[i];
    u->setLastName(temp);
    packet >> messageLen;
    for (i = 0; i < messageLen; i++) packet >> temp[i];
    u->setEmail(temp);
    packet >> cAuthorization;
    u->setAuthorization(cAuthorization == 0 ? true : false);

    // translating string with Translation Table
    gTranslator.ServerToClient(u->getAlias());
    gTranslator.ServerToClient(u->getFirstName());
    gTranslator.ServerToClient(u->getLastName());

    // print out the user information
    gLog.Info("%s%s (%s %s), %s.\n", L_BLANKxSTR , u->getAlias(),
             u->getFirstName(), u->getLastName(), u->getEmail());

    // save the user infomation
    u->setEnableSave(true);
    u->saveBasicInfo();

    ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_USERxGETINFO, checkSequence, EVENT_SUCCESS);
    if (e != NULL)
      PushDoneEvent(e);
    else
      gLog.Warn("%sResponse to unknown event.\n", L_WARNxSTR);
    //PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSERS, 0, 0));
    PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_BASIC, u->getUin()));
    gUserManager.DropUser(u);
    break;
  }

  case ICQ_CMDxRCV_USERxDETAILS:   // user details packet
  {
    /* 02 00 22 01 09 00 0A 00 96 3D 44 00 04 00 3F 3F 3F 00 2E 00 FE 01 00 00
       FF FF 01 01 00 00 1C 00 68 74 74 70 3A 2F 2F 68 65 6D 2E 70 61 73 73 61
       67 65 6E 2E 73 65 2F 67 72 72 2F 00 1E 00 49 27 6D 20 6A 75 73 74 20 61
       20 67 69 72 6C 20 69 6E 20 61 20 77 6F 72 6C 64 2E 2E 2E 00 FF FF FF FF */
    unsigned short country_code, nAge, checkSequence, i;
    char timezone, cSex;

    AckUDP(theSequence);
    packet >> checkSequence  // corresponds to the sequence number from the user information request packet
           >> nUin;
    // find which user it is, verify we have them on our list
    ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
    if (u == NULL)
    {
       gLog.Warn("%sExtended information on unknown user (%d).\n", L_WARNxSTR, nUin);
       break;
    }
    gLog.Info("%sReceived extended information for %s (%ld):\n", L_UDPxSTR,
              u->getAlias(), nUin);

    u->setEnableSave(false);
    char sTemp[MAX_MESSAGE_SIZE], buf[32];

    // City
    packet >> messageLen;
    for (i = 0; i < messageLen; i++) packet >> sTemp[i];
    u->setCity(sTemp);
    // Country
    packet >> country_code;
    packet >> timezone;
    u->setCountry(country_code);
    u->setTimezone(timezone);
    // State
    packet >> messageLen;
    for (i = 0; i < messageLen; i++) packet >> sTemp[i];
    u->setState(sTemp);
    // Age
    packet >> nAge;
    u->setAge(nAge);
    // Sex
    packet >> cSex;
    u->setSex(cSex);
    // Phone Number
    packet >> messageLen;
    for (i = 0; i < messageLen; i++) packet >> sTemp[i];
    u->setPhoneNumber(sTemp);
    // Homepage
    packet >> messageLen;
    for (i = 0; i < messageLen; i++) packet >> sTemp[i];
    u->setHomepage(sTemp);
    // About
    packet >> messageLen;
    for (i = 0; i < messageLen; i++) packet >> sTemp[i];
    u->setAbout(sTemp);

    // translating string with Translation Table
    gTranslator.ServerToClient(u->getCity());
    gTranslator.ServerToClient(u->getState());
    gTranslator.ServerToClient(u->getHomepage());
    gTranslator.ServerToClient(u->getAbout());

    // print out the user information
    gLog.Info("%s%s, %s, %s. %d years old, %s. %s, %s, %s.\n", L_BLANKxSTR,
              u->getCity(), u->getState(), u->getCountry(sTemp), u->getAge(),
              u->getSex(buf), u->getPhoneNumber(), u->getHomepage(), u->getAbout());

    // save the user infomation
    u->setEnableSave(true);
    u->saveExtInfo();

    ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_USERxGETDETAILS, checkSequence, EVENT_SUCCESS);
    if (e != NULL) PushDoneEvent(e);
    PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER,
                                    USER_EXT, u->getUin()));
    gUserManager.DropUser(u);
    break;
  }

  case ICQ_CMDxRCV_UPDATEDxBASIC:
  {
    /* 02 00 B4 00 28 00 01 00 */
    AckUDP(theSequence);
    unsigned short checkSequence;
    packet >> checkSequence;
    gLog.Info("%sSuccessfully updated basic info.\n", L_UDPxSTR);
    ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_UPDATExBASIC, checkSequence, EVENT_SUCCESS);
    CPU_UpdatePersonalBasicInfo *p = (CPU_UpdatePersonalBasicInfo *)e->m_xPacket;
    ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
    o->setAlias(p->Alias());
    o->setFirstName(p->FirstName());
    o->setLastName(p->LastName());
    o->setEmail(p->Email());
    o->setAuthorization(p->Authorization());
    PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER,
                                    USER_BASIC, o->getUin()));
    gUserManager.DropOwner();
    if (e != NULL) PushDoneEvent(e);
    break;
  }

  case ICQ_CMDxRCV_UPDATExBASICxFAIL:
  {
    AckUDP(theSequence);
    gLog.Info("%sFailed to update basic info.\n", L_UDPxSTR);
    unsigned short checkSequence;
    packet >> checkSequence;
    ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_UPDATExBASIC, checkSequence, EVENT_FAILED);
    if (e != NULL) PushDoneEvent(e);
    break;
  }

  case ICQ_CMDxRCV_UPDATEDxDETAIL:
  {
    AckUDP(theSequence);
    gLog.Info("%sSuccessfully updated detail info.\n", L_UDPxSTR);
    unsigned short checkSequence;
    packet >> checkSequence;
    ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_UPDATExDETAIL, checkSequence, EVENT_SUCCESS);
    CPU_UpdatePersonalExtInfo *p = (CPU_UpdatePersonalExtInfo *)e->m_xPacket;
    ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
    o->setCity(p->City());
    o->setCountry(p->Country());
    o->setState(p->State());
    o->setAge(p->Age());
    o->setSex(p->Sex());
    o->setPhoneNumber(p->PhoneNumber());
    o->setHomepage(p->Homepage());
    o->setAbout(p->About());
    PushPluginSignal(new CICQSignal(SIGNAL_UPDATExUSER,
                                    USER_EXT, o->getUin()));
    gUserManager.DropOwner();
    if (e != NULL) PushDoneEvent(e);
    break;
  }

  case ICQ_CMDxRCV_UPDATExDETAILxFAIL:
  {
    AckUDP(theSequence);
    gLog.Info("%sFailed to update detail info.\n", L_UDPxSTR);
    unsigned short checkSequence;
    packet >> checkSequence;
    ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_UPDATExDETAIL, checkSequence, EVENT_FAILED);
    if (e != NULL) PushDoneEvent(e);
    break;
  }

  case ICQ_CMDxRCV_USERxINVALIDxUIN:  // not a good uin
    AckUDP(theSequence);
    packet >> nUin;
    gLog.Info("%sInvalid UIN: %d.\n", L_UDPxSTR, nUin);
    // we need to do something here, but I bet the command is included in the packet
    //emit signal_doneUserBasicInfo(false, nUin);
    break;

  case ICQ_CMDxRCV_USERxSTATUS:  // user changed status packet
  {
    AckUDP(theSequence);
    packet >> nUin;

    // find which user it is, verify we have them on our list
    ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
    if (u == NULL)
    {
       gLog.Warn("%sUnknown user (%d) changed status.\n", L_WARNxSTR, nUin);
       break;
    }
    gLog.Info("%s%s (%ld) changed status.\n", L_UDPxSTR, u->getAlias(), nUin);

    unsigned long nNewStatus;
    packet >> nNewStatus;
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
    AckUDP(theSequence);
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
    AckUDP(theSequence);
    gLog.Info("%sSearch found user:\n", L_UDPxSTR);

    unsigned short i, aliasLen, firstNameLen, lastNameLen, emailLen,
                   searchSequence;
    char auth;
    struct UserBasicInfo *us = new UserBasicInfo;

    packet >> searchSequence
           >> nUin;
    sprintf(us->uin, "%ld", nUin);
    // Alias
    packet >> aliasLen;
    for (i = 0; i < aliasLen; i++) packet >> us->alias[i];
    // First name
    packet >> firstNameLen;
    for (i = 0; i < firstNameLen; i++) packet >> us->firstname[i];
    // Last name
    packet >> lastNameLen;
    for (i = 0; i < lastNameLen; i++) packet >> us->lastname[i];
    // Email
    packet >> emailLen;
    for (i = 0; i < emailLen; i++) packet >> us->email[i];

    // translating string with Translation Table
    gTranslator.ServerToClient(us->alias);
    gTranslator.ServerToClient(us->firstname);
    gTranslator.ServerToClient(us->lastname);

    packet >> auth;
    sprintf(us->name, "%s %s", us->firstname, us->lastname);
    gLog.Info("%s%s (%ld) <%s %s, %s>\n", L_BLANKxSTR, us->alias, nUin,
             us->firstname, us->lastname, us->email);

    ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_SEARCHxSTART, searchSequence, EVENT_ACKED);
    // We make as copy as each plugin will delete the events it gets
    if (e == NULL)
    {
      gLog.Warn("%sReceived search result when no search in progress.\n", L_WARNxSTR);
      break;
    }
    ICQEvent *e2 = new ICQEvent(e);
    e2->m_sSearchAck = new SSearchAck;
    e2->m_sSearchAck->sBasicInfo = us;
    e2->m_sSearchAck->cMore = 0;
    PushPluginEvent(e2);
    PushExtendedEvent(e);
    break;
  }

  case ICQ_CMDxRCV_SEARCHxDONE:  // user found in search
  {
    /* 02 00 A0 00 04 00 05 00 00*/
    AckUDP(theSequence);

    char more;
    unsigned short searchSequence;
    packet >> searchSequence >> more;
    gLog.Info("%sSearch finished.\n", L_UDPxSTR);
    ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_SEARCHxSTART, searchSequence, EVENT_SUCCESS);
    if (e == NULL)
    {
      gLog.Warn("%sReceived end of search when no search in progress.\n", L_WARNxSTR);
      break;
    }
    e->m_sSearchAck = new SSearchAck;
    e->m_sSearchAck->sBasicInfo = NULL;
    e->m_sSearchAck->cMore = more;
    PushDoneEvent(e);
    break;
  }

  case ICQ_CMDxRCV_SYSxMSGxDONE:  // end of system messages
  {
     /* 02 00 E6 00 04 00 50 A5 82 00 */
     AckUDP(theSequence);
     gLog.Info("%sEnd of system messages.\n", L_UDPxSTR);

     // send special ack for this one as well as the usual ack
     gLog.Info("%sSending ok to erase system messages (#%d)...\n", L_UDPxSTR, theSequence);
     CPU_SysMsgDoneAck *p = new CPU_SysMsgDoneAck(theSequence);
     SendExpectEvent(m_nUDPSocketDesc, p, CONNECT_NONE);
     break;
  }

  case ICQ_CMDxRCV_SYSxMSGxOFFLINE:  // offline system message
  {
    /* 02 00 DC 00 0A 00 EC C9 45 00 CE 07 04 17 04 21 01 00 3F 00 6E 6F 2C 20
       73 74 69 6C 6C 20 68 6F 70 69 6E 67 20 66 6F 72 20 74 68 65 20 72 65 63
       6F 72 64 20 63 6F 6D 70 61 6E 79 2C 20 62 75 74 20 79 6F 75 20 6E 65 76
       65 72 20 6B 6E 6F 77 2E 2E 2E 00 */
    AckUDP(theSequence);
    gLog.Info("%sOffline system message:\n", L_UDPxSTR);

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
    AckUDP(theSequence);
    gLog.Info("%sOnline system message:\n", L_UDPxSTR);
    unsigned short newCommand;
    packet >> nUin
           >> newCommand;
    // process the system message, sending the current time as a time_t structure
    ProcessSystemMessage(packet, nUin, newCommand, time(NULL));
    break;
  }

  case ICQ_CMDxRCV_SETxOFFLINE:  // we got put offline by mirabilis for some reason
    gLog.Info("%sKicked offline by server.\n", L_UDPxSTR);
    icqLogoff(true);
    break;

  case ICQ_CMDxRCV_ACK:  // icq acknowledgement
  {
    /* 02 00 0A 00 12 00 */
    gLog.Info("%sAck (#%d).\n", L_UDPxSTR, theSequence);
    ICQEvent *e = DoneEvent(m_nUDPSocketDesc, theSequence, EVENT_ACKED);
    if (e != NULL) PushDoneEvent(e);
    break;
  }

  case ICQ_CMDxRCV_ERROR:  // icq says go away
  {
    gLog.Info("%sServer says you are not logged on (%d).\n", L_UDPxSTR, theSequence);
    ICQEvent *e = DoneEvent(m_nUDPSocketDesc, theSequence, EVENT_FAILED);
    if (e != NULL) PushDoneEvent(e);
    icqLogoff(true);
    break;
  }

  case ICQ_CMDxRCV_HELLO: // hello packet from mirabilis received on logon
  {
    /* 02 00 5A 00 00 00 8F 76 20 00 CD CD 76 10 02 00 01 00 05 00 00 00 00 00
       8C 00 00 00 F0 00 0A 00 0A 00 05 00 0A 00 01 */
    AckUDP(theSequence);
    gLog.Info("%sMirabilis says hello.\n", L_UDPxSTR);

    ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
    ChangeUserStatus(o, m_nDesiredStatus);
    gUserManager.DropOwner();

    m_eStatus = STATUS_ONLINE;
    ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_LOGON, 0, EVENT_SUCCESS);
    if (e != NULL) PushDoneEvent(e);
    PushPluginSignal(new CICQSignal(SIGNAL_LOGON, 0, 0));

    icqUpdateContactList();
    icqSendInvisibleList();
    icqSendVisibleList();
    icqRequestSystemMsg();

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
    if (e != NULL) PushDoneEvent(e);
    break;
  }

  case ICQ_CMDxRCV_BUSY:  // server too busy to respond
  {
    gLog.Info("%sServer busy, try again in a few minutes.\n", L_UDPxSTR);
    m_eStatus = STATUS_OFFLINE_FORCED;
    ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_LOGON, 0, EVENT_FAILED);
    if (e != NULL) PushDoneEvent(e);
    break;
  }

  case ICQ_CMDxRCV_NEWxUIN:  // received a new uin
  {
    unsigned long nUin;
    unsigned short nTemp;
    packet >> nTemp >> nUin;
    gLog.Info("%sReceived new uin: %d\n", L_UDPxSTR, nUin);
    gUserManager.SetOwnerUin(nUin);
    ICQEvent *e = DoneExtendedEvent(ICQ_CMDxSND_REGISTERxUSER, 0, EVENT_SUCCESS);
    if (e != NULL) PushDoneEvent(e);
    // Logon as an ack
    icqLogon(ICQ_STATUS_ONLINE);
    break;
  }

  default:  // what the heck is this packet?  print it out
    AckUDP(theSequence);
    char *buf;
    gLog.Unknown("%sUnknown UDP packet:\n%s\n", L_UNKNOWNxSTR, packet.print(buf));
    delete buf;
    break;
  }

  return(command);
}


//-----CICQDaemon::ProcessSystemMessage-----------------------------------------
void CICQDaemon::ProcessSystemMessage(CBuffer &packet, unsigned long nUin,
                                      unsigned short newCommand, time_t timeSent)
{
  unsigned short messageLen;
  ICQUser *u;
  int i;

  if (nUin == 0)
  {
    char *buf;
    gLog.Unknown("%sInvalid system message (UIN = 0?):\n%s\n", L_UNKNOWNxSTR, packet.print(buf));
    delete buf;
  }

  // Check if uin is backwards, what the fuck is with icq99b?
  if (nUin > 0x07FFFFFF)
  {
    nUin = (nUin & 0x000000FF) << 24 + (nUin & 0x0000FF00) << 8 +
           (nUin & 0x00FF0000) >> 8  + (nUin & 0xFF000000) >> 24;
  }
  // Swap high and low bytes for strange new icq99
  if ((newCommand > 0x00FF) && !(newCommand & ICQ_CMDxSUB_FxMULTIREC))
  {
    newCommand = (newCommand & 0xFF00) >> 8 + (newCommand & 0x00FF) << 8;
  }

  unsigned long nMask = ((newCommand & ICQ_CMDxSUB_FxMULTIREC) ? E_MULTIxREC : 0);
  newCommand &= ~ICQ_CMDxSUB_FxMULTIREC;

  switch(newCommand)
  {
  case ICQ_CMDxSUB_MSG:  // system message: message through the server
  {
    // Read in the message
    packet >> messageLen;
    char message[messageLen];
    for (i = 0; i < messageLen; i++) packet >> message[i];

    // translating string with Translation Table
    gTranslator.ServerToClient (message);
    CEventMsg *e = new CEventMsg(message, ICQ_CMDxRCV_SYSxMSGxONLINE,
                                 timeSent, nMask);

    // Lock the user to add the message to their queue
    bool bNewUser = false;
    u = gUserManager.FetchUser(nUin, LOCK_W);
    if (u == NULL)
    {
      gLog.Info("%sMessage from unknown user (%ld) adding them to your list.\n",
                L_BLANKxSTR, nUin);
      AddUserToList(nUin);
      u = gUserManager.FetchUser(nUin, LOCK_W);
      bNewUser = true;
    }
    else
      gLog.Info("%sMessage through server from %s (%ld).\n", L_BLANKxSTR,
                u->getAlias(), nUin);

    m_xOnEventManager.Do(ON_EVENT_MSG, u);
    AddUserEvent(u, e);
    gUserManager.DropUser(u);
    // We only want a read lock because Reorder takes a write lock
    // on the group, so there would be a potential race condition
    // if we have a group and user write locked at the same time
    gUserManager.DropUser(u);
    u = gUserManager.FetchUser(nUin, LOCK_R);
    gUserManager.Reorder(u);
    gUserManager.DropUser(u);
    PushPluginSignal(new CICQSignal(SIGNAL_UPDATExLIST, LIST_REORDER, nUin));
    break;
  }
  case ICQ_CMDxSUB_URL:  // system message: url through the server
  {
    packet >> messageLen;
    char message[messageLen];
    for (i = 0; i < messageLen; i++) packet >> message[i];

    // parse the message into url and url description
    char **szUrl = new char*[2];  // description, url
    ParseFE(message, &szUrl, 2);

    // translating string with Translation Table
    gTranslator.ServerToClient (szUrl[0]);
    CEventUrl *e = new CEventUrl(szUrl[1], szUrl[0],
                                 ICQ_CMDxRCV_SYSxMSGxONLINE, timeSent, nMask);

    u = gUserManager.FetchUser(nUin, LOCK_W);
    if (u == NULL)
    {
      gLog.Info("%sURL from unknown user (%ld) adding them to your list.\n",
                L_BLANKxSTR, nUin);
      AddUserToList(nUin);
      u = gUserManager.FetchUser(nUin, LOCK_W);
    }
    else
      gLog.Info("%sURL through server from %s (%ld).\n", L_BLANKxSTR,
                u->getAlias(), nUin);

    m_xOnEventManager.Do(ON_EVENT_URL, u);
    AddUserEvent(u, e);
    gUserManager.DropUser(u);
    u = gUserManager.FetchUser(nUin, LOCK_R);
    gUserManager.Reorder(u);
    gUserManager.DropUser(u);
    PushPluginSignal(new CICQSignal(SIGNAL_UPDATExLIST, LIST_REORDER, nUin));
    delete[] szUrl;
    break;
  }
  case ICQ_CMDxSUB_REQxAUTH:  // system message: authorisation request
  {
     /* 02 00 04 01 08 00 8F 76 20 00 06 00 41 00 41 70 6F 74 68 65 6F 73 69 73
        FE 47 72 61 68 61 6D FE 52 6F 66 66 FE 67 72 6F 66 66 40 75 77 61 74 65
        72 6C 6F 6F 2E 63 61 FE 31 FE 50 6C 65 61 73 65 20 61 75 74 68 6F 72 69
        7A 65 20 6D 65 2E 00 */

     gLog.Info("%sAuthorization request from %ld.\n", L_BLANKxSTR, nUin);
     unsigned short infoLen;
     packet >> infoLen;
     char message[infoLen + 1];
     for (i = 0; i < infoLen; i++) packet >> message[i];

     char **szFields = new char*[6];  // alias, first name, last name, email, auth, reason
     ParseFE(message, &szFields, 6);

     // translating string with Translation Table
     gTranslator.ServerToClient (szFields[0]);
     gTranslator.ServerToClient (szFields[1]);
     gTranslator.ServerToClient (szFields[2]);
     gTranslator.ServerToClient (szFields[5]);

     CEventAuth *e = new CEventAuth(nUin, szFields[0], szFields[1],
                                    szFields[2], szFields[3], szFields[5],
                                    ICQ_CMDxRCV_SYSxMSGxONLINE, timeSent, 0);
     ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
     AddUserEvent(o, e);
     gUserManager.DropOwner();
     e->AddToHistory(NULL, D_RECEIVER);
     m_xOnEventManager.Do(ON_EVENT_SYSMSG, NULL);
     PushPluginSignal(new CICQSignal(SIGNAL_UPDATExLIST, LIST_REORDER, nUin));
     delete[] szFields;
     break;
  }
  case ICQ_CMDxSUB_ADDEDxTOxLIST:  // system message: added to a contact list
  {
     gLog.Info("%sUser %ld added you to their contact list.\n", L_BLANKxSTR,
               nUin);

     unsigned short infoLen;
     packet >> infoLen;

     // read in the user data from the packet
     char userInfo[infoLen + 1];
     for (i = 0; i < infoLen; i++) packet >> userInfo[i];
     if (userInfo[i - 1] != '\0')
     {
       char *buf;
       gLog.Warn("%sError in packet (no terminating NULL):\n%s\n", L_WARNxSTR,
                 packet.print(buf));
       delete [] buf;
       userInfo[i - 1] = '\0';
     }
     char **szFields = new char*[5]; // alias, first name, last name, email, auth
     ParseFE(userInfo, &szFields, 5);

     // translating string with Translation Table
     gTranslator.ServerToClient(szFields[0]);  // alias
     gTranslator.ServerToClient(szFields[1]);  // first name
     gTranslator.ServerToClient(szFields[2]);  // last name
     gLog.Info("%s%s (%s %s), %s\n", L_BLANKxSTR, szFields[0], szFields[1],
              szFields[2], szFields[3]);

     CEventAdded *e = new CEventAdded(nUin, szFields[0], szFields[1],
                                      szFields[2], szFields[3],
                                      ICQ_CMDxRCV_SYSxMSGxONLINE, timeSent, 0);
     ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
     AddUserEvent(o, e);
     gUserManager.DropOwner();
     e->AddToHistory(NULL, D_RECEIVER);
     m_xOnEventManager.Do(ON_EVENT_SYSMSG, NULL);
     PushPluginSignal(new CICQSignal(SIGNAL_UPDATExLIST, LIST_REORDER, nUin));
     delete[] szFields;
     break;
  }
  case ICQ_CMDxSUB_WEBxPANEL:
  {
    /* 02 00 04 01 28 00 0A 00 00 00 0D 00 49 00 6D 79 20 6E 61 6D 65 FE FE FE
       65 6D 61 69 6C FE 33 FE 53 65 6E 64 65 72 20 49 50 3A 20 32 30 39 2E 32
       33 39 2E 36 2E 31 33 0D 0A 53 75 62 6A 65 63 74 3A 20 73 75 62 6A 65 63
       74 0D 0A 74 68 65 20 6D 65 73 73 61 67 65 00 */
    gLog.Info("%sMessage through web panel.\n", L_BLANKxSTR);

    unsigned short nLen;
    packet >> nLen;

    // read in the user data from the packet
    char szMessage[nLen];
    for (i = 0; i < nLen; i++) packet >> szMessage[i];
    char **szFields = new char*[6]; // name, ?, ?, email, ?, message
    ParseFE(szMessage, &szFields, 6);

    gTranslator.ServerToClient(szFields[0]);
    gTranslator.ServerToClient(szFields[5]);

    gLog.Info("%sFrom %s (%s).\n", L_BLANKxSTR, szFields[0], szFields[3]);
    CEventWebPanel *e = new CEventWebPanel(szFields[0], szFields[3], szFields[5],
                                           ICQ_CMDxRCV_SYSxMSGxONLINE, timeSent, 0);
    ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
    AddUserEvent(o, e);
    gUserManager.DropOwner();
    e->AddToHistory(NULL, D_RECEIVER);
    m_xOnEventManager.Do(ON_EVENT_SYSMSG, NULL);
    PushPluginSignal(new CICQSignal(SIGNAL_UPDATExLIST, LIST_REORDER, nUin));
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
    gLog.Info("%sEmail pager message.\n", L_BLANKxSTR);

    unsigned short nLen;
    packet >> nLen;

    // read in the user data from the packet
    char szMessage[nLen];
    for (i = 0; i < nLen; i++) packet >> szMessage[i];
    char **szFields = new char*[6]; // name, ?, ?, email, ?, message
    ParseFE(szMessage, &szFields, 6);

    gTranslator.ServerToClient(szFields[0]);
    gTranslator.ServerToClient(szFields[5]);

    gLog.Info("%sFrom %s (%s).\n", L_BLANKxSTR, szFields[0], szFields[3]);
    CEventEmailPager *e = new CEventEmailPager(szFields[0], szFields[3], szFields[5],
                                               ICQ_CMDxRCV_SYSxMSGxONLINE, timeSent, 0);
    ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
    AddUserEvent(o, e);
    gUserManager.DropOwner();
    e->AddToHistory(NULL, D_RECEIVER);
    m_xOnEventManager.Do(ON_EVENT_SYSMSG, NULL);
    PushPluginSignal(new CICQSignal(SIGNAL_UPDATExLIST, LIST_REORDER, nUin));
    delete[] szFields;
    break;
  }
  case ICQ_CMDxSUB_CONTACTxLIST:
  {
    /* 02 00 DC 00 1A 00 23 64 84 00 CF 07 06 15 13 2B 13 00 1E 00 32 FE 37 33
       39 37 38 35 33 FE 46 6C 75 6E 6B 69 FE 37 33 35 37 32 31 39 FE 55 68 75
       FE 00 */
    gLog.Info("%sContact list.\n", L_BLANKxSTR);

    unsigned short nLen;
    packet >> nLen;

    // read in the user data from the packet
    char szMessage[nLen];
    for (i = 0; i < nLen; i++) packet >> szMessage[i];
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

    gLog.Info("%s%s contacts.\n", L_BLANKxSTR, szMessage);
    CEventContactList *e = new CEventContactList(vszFields, ICQ_CMDxRCV_SYSxMSGxONLINE, timeSent, 0);
    ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
    AddUserEvent(o, e);
    gUserManager.DropOwner();
    e->AddToHistory(NULL, D_RECEIVER);
    m_xOnEventManager.Do(ON_EVENT_SYSMSG, NULL);
    PushPluginSignal(new CICQSignal(SIGNAL_UPDATExLIST, LIST_REORDER, nUin));
    delete[] szFields;
    break;
  }
  default:
  {
    unsigned short nLen;
    packet >> nLen;
    char szMessage[nLen];
    for (i = 0; i < nLen; i++) packet >> szMessage[i];
    char *szFE;
    while((szFE = strchr(szMessage, 0xFE)) != NULL) *szFE = '\n';

    char *buf;
    gLog.Unknown("%sUnknown system message (0x%04x):\n%s\n", L_UNKNOWNxSTR,
                 newCommand, packet.print(buf));
    delete [] buf;
    CEventUnknownSysMsg *e =
      new CEventUnknownSysMsg(newCommand, ICQ_CMDxRCV_SYSxMSGxONLINE,
                              nUin, szMessage, 0, timeSent, 0);
    ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
    AddUserEvent(o, e);
    gUserManager.DropOwner();
    e->AddToHistory(NULL, D_RECEIVER);
  }
  } // switch
}


//-----CICQDaemon::AckUDP--------------------------------------------------------------
void CICQDaemon::AckUDP(unsigned short theSequence)
// acknowledge whatever packet we received using the relevant sequence number
{
   CPU_Ack p(theSequence);
   SendEvent(m_nUDPSocketDesc, p);
}


