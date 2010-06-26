/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Licq developers
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

#ifndef LICQDAEMON_ICQ_H
#define LICQDAEMON_ICQ_H

#include <licq/icq.h>

#include <boost/shared_array.hpp>
#include <list>
#include <map>
#include <pthread.h>

#include <licq/buffer.h>
#include <licq/event.h>
#include <licq/pipe.h>
#include <licq/socketmanager.h>
#include <licq/userid.h>

class COscarService;
class CPacketTcp;

namespace Licq
{
class INetSocket;
class IniFile;
class Packet;
class TCPSocket;
class UserEvent;
}

// To keep old code working
typedef std::map<int, std::string> GroupNameMap;

void *ProcessRunningEvent_Client_tep(void *p);
void *ProcessRunningEvent_Server_tep(void *p);
void *ReverseConnectToUser_tep(void *p);
void *Shutdown_tep(void *p);

/**
 * Internal template class for storing and processing received contact list.
 */
class CUserProperties
{
public:
  CUserProperties();

private:
  boost::shared_array<char> newAlias;
  boost::shared_array<char> newCellular;

  unsigned short normalSid;
  unsigned short groupId;

  unsigned short visibleSid;
  unsigned short invisibleSid;
  bool inIgnoreList;

  bool awaitingAuth;

  Licq::TlvList tlvs;

friend class CICQDaemon;
  friend class IcqProtocol;
};

typedef std::map<std::string, CUserProperties*> ContactUserList;
typedef ContactUserList::iterator ContactUserListIter;

// Data structure for passing information to the reverse connection thread
class CReverseConnectToUserData
{
public:
  CReverseConnectToUserData(const char* idString, unsigned long id,
      unsigned long data, unsigned long ip, unsigned short port,
      unsigned short version, unsigned short failedport, unsigned long msgid1,
      unsigned long msgid2);
  ~CReverseConnectToUserData();

  std::string myIdString;
  unsigned long nId;
  unsigned long nData;
  unsigned long nIp;
  unsigned short nPort;
  unsigned short nFailedPort;
  unsigned short nVersion;
  unsigned long nMsgID1;
  unsigned long nMsgID2;
  bool bSuccess;
  bool bFinished;
};


class IcqProtocol : public CICQDaemon
{
public:
  IcqProtocol();
  ~IcqProtocol();

  void initialize();
  bool start();
  void save(Licq::IniFile& licqConf);

  void SetUseServerSideBuddyIcons(bool b);

  unsigned long icqSendContactList(const Licq::UserId& userId, const Licq::StringList& users,
     bool bOnline, unsigned short nLevel, bool bMultipleRecipients = false,
     const Licq::Color* pColor = NULL);

  unsigned long icqFetchAutoResponse(const Licq::UserId& userId, bool bServer = false);
  unsigned long icqChatRequest(const Licq::UserId& userId, const char *szReason,
     unsigned short nLevel, bool bServer);
  unsigned long icqMultiPartyChatRequest(const Licq::UserId& userId,
     const char *szReason, const char *szChatUsers, unsigned short nPort,
     unsigned short nLevel, bool bServer);
  void icqChatRequestRefuse(const Licq::UserId& userId, const char* szReason,
      unsigned short nSequence, const unsigned long nMsgID[], bool bDirect);
  void icqChatRequestAccept(const Licq::UserId& userId, unsigned short nPort,
      const char* szClients, unsigned short nSequence,
      const unsigned long nMsgID[], bool bDirect);
  void icqChatRequestCancel(const Licq::UserId& userId, unsigned short nSequence);
  unsigned long icqRequestInfoPluginList(const Licq::UserId& userId, bool bServer = false);
  unsigned long icqRequestPhoneBook(const Licq::UserId& userId, bool bServer = false);
  unsigned long icqRequestPicture(const Licq::UserId& userId, bool bServer, size_t iconHashSize);
  unsigned long icqRequestStatusPluginList(const Licq::UserId& userId, bool bServer = false);
  unsigned long icqRequestSharedFiles(const Licq::UserId& userId, bool bServer = false);
  unsigned long icqRequestPhoneFollowMe(const Licq::UserId& userId, bool bServer = false);
  unsigned long icqRequestICQphone(const Licq::UserId& userId, bool bServer = false);
  void icqRegister(const char *_szPasswd);
  void icqVerifyRegistration();
  void icqVerify(const char *);
  unsigned long icqSetWorkInfo(const char *_szCity, const char *_szState,
                           const char *_szPhone,
                           const char *_szFax, const char *_szAddress,
                           const char *_szZip, unsigned short _nCompanyCountry,
                           const char *_szName, const char *_szDepartment,
                           const char *_szPosition, unsigned short _nCompanyOccupation,
                           const char *_szHomepage);
  unsigned long icqSetGeneralInfo(const char *szAlias, const char *szFirstName,
                              const char *szLastName, const char *szEmailPrimary,
                              const char *szCity,
                              const char *szState, const char *szPhoneNumber,
                              const char *szFaxNumber, const char *szAddress,
                              const char *szCellularNumber, const char *szZipCode,
                              unsigned short nCountryCode, bool bHideEmail);
  unsigned long icqSetEmailInfo(const char *szEmailSecondary, const char *szEmailOld);
  unsigned long icqSetMoreInfo(unsigned short nAge,
                           char nGender, const char *szHomepage,
                           unsigned short nBirthYear, char nBirthMonth,
                           char nBirthDay, char nLanguage1,
                           char nLanguage2, char nLanguage3);
  unsigned long icqSetSecurityInfo(bool bAuthorize, bool bHideIp, bool bWebAware);
  unsigned long icqSetInterestsInfo(const Licq::UserCategoryMap& interests);
  unsigned long icqSetOrgBackInfo(const Licq::UserCategoryMap& orgs,
      const Licq::UserCategoryMap& background);
  unsigned long icqSetAbout(const char *szAbout);
  unsigned long icqSetPassword(const char *szPassword);
  unsigned long icqSetRandomChatGroup(unsigned long nGroup);
  unsigned long icqRandomChatSearch(unsigned long nGroup);
  unsigned long icqSearchWhitePages(const char *szFirstName,
                            const char *szLastName, const char *szAlias,
                            const char *szEmail, unsigned short nMinAge,
                            unsigned short nMaxAge, char nGender,
                            char nLanguage, const char *szCity,
                            const char *szState, unsigned short nCountryCode,
                            const char *szCoName, const char *szCoDept,
                            const char *szCoPos, const char *szKeyword,
                            bool bOnlineOnly);
  unsigned long icqSearchByUin(unsigned long);
  unsigned long icqAuthorizeGrant(const Licq::UserId& userId, const std::string& message);
  unsigned long icqAuthorizeRefuse(const Licq::UserId& userId, const std::string& message);
  void icqRequestAuth(const Licq::UserId& userId, const char *_szMessage);
  void icqAlertUser(const Licq::UserId& userId);
  void icqUpdatePhoneBookTimestamp();
  void icqUpdatePictureTimestamp();
  void icqSetPhoneFollowMeStatus(unsigned long nNewStatus);
  void icqUpdateContactList();
  void icqCheckInvisible(const Licq::UserId& userId);
  unsigned long icqSendSms(const Licq::UserId& userId, const char* number, const char* message);

  void icqSendMessage(unsigned long eventId, const Licq::UserId& userId, const std::string& message,
      bool viaServer, unsigned short nLevel, bool bMultipleRecipients = false,
      const Licq::Color* pColor = NULL);
  void icqSendUrl(unsigned long eventId, const Licq::UserId& userId, const std::string& url,
      const std::string& message, bool viaServer, unsigned short nLevel,
      bool bMultipleRecipients = false, const Licq::Color* pColor = NULL);
  void icqFileTransfer(unsigned long eventId, const Licq::UserId& userId, const std::string& filename,
      const std::string& message, const std::list<std::string>& fileList,
     unsigned short nLevel, bool bServer);
  void icqFileTransferRefuse(const Licq::UserId& userId, const std::string& message,
      unsigned short nSequence, const unsigned long nMsgID[], bool viaServer);
  void icqFileTransferCancel(const Licq::UserId& userId, unsigned short nSequence);
  void icqFileTransferAccept(const Licq::UserId& userId, unsigned short nPort,
      unsigned short nSequence, const unsigned long nMsgID[], bool viaServer,
      const std::string& message, const std::string& filename, unsigned long nFileSize);
  void icqOpenSecureChannel(unsigned long eventId, const Licq::UserId& userId);
  void icqCloseSecureChannel(unsigned long eventId, const Licq::UserId& userId);
  void icqOpenSecureChannelCancel(const Licq::UserId& userId, unsigned short nSequence);
  void icqFetchAutoResponseServer(unsigned long eventId, const Licq::UserId& userId);
  unsigned long icqLogon(unsigned short logonStatus);
  unsigned long icqRequestLogonSalt();
  unsigned long icqUserBasicInfo(const Licq::UserId& userId);
  unsigned long icqRequestMetaInfo(const Licq::UserId& userId);
  unsigned long icqSetStatus(unsigned short newStatus);
  void icqLogoff();
  void postLogoff(int nSD, Licq::Event* cancelledEvent);
  void icqRelogon();
  void icqAddUser(const Licq::UserId& userId, bool _bAuthReq = false, unsigned short groupId = 0);
  void icqAddUserServer(const Licq::UserId& userId, bool _bAuthReq, unsigned short groupId = 0);
  void icqAddGroup(const char *);
  void icqRemoveUser(const Licq::UserId& userId, bool ignored = false);
  void icqRemoveGroup(const char *);
  void icqChangeGroup(const Licq::UserId& userId, unsigned short _nNewGroup, unsigned short _nOldGSID,
                      unsigned short _nNewType, unsigned short _nOldType);
  void icqRenameGroup(const char *_szNewName, unsigned short _nGSID);
  void icqRenameUser(const Licq::UserId& userId, const std::string& newAlias);
  void icqExportUsers(const std::list<Licq::UserId>& users, unsigned short);
  void icqExportGroups(const GroupNameMap& groups);
  void icqUpdateServerGroups();
  void icqTypingNotification(const Licq::UserId& userId, bool _bActive);
  void icqRequestService(unsigned short nFam);
  void icqAddToVisibleList(const Licq::UserId& userId);
  void icqRemoveFromVisibleList(const Licq::UserId& userId);
  void icqAddToInvisibleList(const Licq::UserId& userId);
  void icqRemoveFromInvisibleList(const Licq::UserId& userId);
  void icqAddToIgnoreList(const Licq::UserId& userId);
  void icqRemoveFromIgnoreList(const Licq::UserId& userId);
  void icqClearServerList();

  void icqRegisterFinish();
  void icqPing();
  void icqSendVisibleList();
  void icqSendInvisibleList();
  void icqCreatePDINFO();
  void icqRequestSystemMsg();
  Licq::Event* icqSendThroughServer(unsigned long eventId, const Licq::UserId& userId,
      unsigned char format, const std::string& message, Licq::UserEvent*,
      unsigned short = 0, size_t = 0);

  void CheckExport();
  bool openConnectionToUser(const Licq::UserId& userId, Licq::TCPSocket* sock,
     unsigned short nPort);
  bool OpenConnectionToUser(const char *szAlias, unsigned long nIp,
     unsigned long nIntIp, Licq::TCPSocket* sock, unsigned short nPort,
     bool bSendIntIp);

  void UpdateAllUsers();
  void updateAllUsersInGroup(int groupId);
  void CancelEvent(unsigned long );
  void CancelEvent(Licq::Event*);

  void FailEvents(int sd, int err);
  Licq::Event* DoneServerEvent(unsigned long, Licq::Event::ResultType);
  Licq::Event* DoneEvent(Licq::Event* e, Licq::Event::ResultType _eResult);
  Licq::Event* DoneEvent(int _nSD, unsigned short _nSequence, Licq::Event::ResultType _eResult);
  Licq::Event* DoneEvent(unsigned long tag, Licq::Event::ResultType _eResult);
  Licq::Event* DoneExtendedServerEvent(const unsigned short, Licq::Event::ResultType);
  Licq::Event* DoneExtendedEvent(Licq::Event*, Licq::Event::ResultType);
  Licq::Event* DoneExtendedEvent(unsigned long tag, Licq::Event::ResultType _eResult);

  // Common message handler
  void ProcessMessage(Licq::User* user, Licq::Buffer& packet, char* message,
     unsigned short nMsgType, unsigned long nMask,
      const unsigned long nMsgID[], unsigned short nSequence,
     bool bIsAck, bool &bNewUser);

  bool ProcessPluginMessage(Licq::Buffer& packet, Licq::User* user, unsigned char nChannel,
     bool bIsAck, unsigned long nMsgID1,
     unsigned long nMsgID2, unsigned short nSequence,
     Licq::TCPSocket* pSock);
  void ProcessDoneEvent(Licq::Event*);
  bool ProcessSrvPacket(Licq::Buffer&);

  //--- Channels ---------
  bool ProcessCloseChannel(Licq::Buffer&);
  void ProcessDataChannel(Licq::Buffer&);

  //--- Families ---------
  void ProcessServiceFam(Licq::Buffer&, unsigned short);
  void ProcessLocationFam(Licq::Buffer&, unsigned short);
  void ProcessBuddyFam(Licq::Buffer&, unsigned short);
  void ProcessMessageFam(Licq::Buffer&, unsigned short);
  void ProcessVariousFam(Licq::Buffer&, unsigned short);
  void ProcessBOSFam(Licq::Buffer&, unsigned short);
  void ProcessListFam(Licq::Buffer&, unsigned short);
  void ProcessAuthFam(Licq::Buffer&, unsigned short);

  void ProcessUserList();

  void ProcessSystemMessage(Licq::Buffer &packet, unsigned long checkUin, unsigned short newCommand, time_t timeSent);
  void ProcessMetaCommand(Licq::Buffer &packet, unsigned short nMetaCommand, Licq::Event* e);
  bool ProcessTcpPacket(Licq::TCPSocket*);
  bool ProcessTcpHandshake(Licq::TCPSocket*);

  unsigned long icqRequestInfoPlugin(Licq::User* user, bool, const char *);
  unsigned long icqRequestStatusPlugin(Licq::User* user, bool, const char *);
  void icqUpdateInfoTimestamp(const char *);

  static bool handshake_Send(Licq::TCPSocket*, const Licq::UserId& userId, unsigned short,
                             unsigned short, bool = true, unsigned long = 0);
  static bool Handshake_SendConfirm_v7(Licq::TCPSocket*);
  static bool Handshake_Recv(Licq::TCPSocket*, unsigned short, bool = true, bool = false);
  static bool Handshake_RecvConfirm_v7(Licq::TCPSocket*);

  int ConnectToServer(const char* server, unsigned short port);
  int ConnectToLoginServer();
  int connectToUser(const Licq::UserId& userId, unsigned char channel);
  int reverseConnectToUser(const Licq::UserId& userId, unsigned long nIp,
     unsigned short nPort, unsigned short nVersion, unsigned short nFailedPort,
     unsigned long nId, unsigned long nMsgID1, unsigned long nMsgID2);
  int requestReverseConnection(const Licq::UserId& userId, unsigned long, unsigned long,
                               unsigned short, unsigned short);
  bool waitForReverseConnection(unsigned short id, const Licq::UserId& userId);

  void PushEvent(Licq::Event*);
  void PushExtendedEvent(Licq::Event*);

private:
  static const int PingFrequency = 60;
  static const int UpdateFrequency = 60;
  static const int LogonAttemptDelay = 300;
  static const int MaxPingTimeouts = 3;

  bool SendEvent(int nSD, Licq::Packet &, bool);
  bool SendEvent(Licq::INetSocket *, Licq::Packet &, bool);
  void SendEvent_Server(Licq::Packet *packet);
  Licq::Event* SendExpectEvent_Server(unsigned long eventId, const Licq::UserId& userId, Licq::Packet*, Licq::UserEvent*, bool = false);
  Licq::Event* SendExpectEvent_Server(const Licq::UserId& userId, Licq::Packet* packet, Licq::UserEvent* ue, bool extendedEvent = false);

  Licq::Event* SendExpectEvent_Server(unsigned long eventId, Licq::Packet* packet, Licq::UserEvent* ue, bool extendedEvent = false)
  { return SendExpectEvent_Server(eventId, Licq::UserId(), packet, ue, extendedEvent); }

  Licq::Event* SendExpectEvent_Server(Licq::Packet* packet, Licq::UserEvent* ue, bool extendedEvent = false);
  Licq::Event* SendExpectEvent_Client(unsigned long eventId, const Licq::User* user, Licq::Packet* packet, Licq::UserEvent* ue);
  Licq::Event* SendExpectEvent_Client(const Licq::User* user, Licq::Packet* packet, Licq::UserEvent* ue);
  Licq::Event* SendExpectEvent(Licq::Event*, void *(*fcn)(void *));

  void AckTCP(CPacketTcp &, int);
  void AckTCP(CPacketTcp &, Licq::TCPSocket*);

  void ChangeUserStatus(Licq::User* u, unsigned long s);
  Licq::User* FindUserForInfoUpdate(const Licq::UserId& userId, Licq::Event* e, const char*);
  std::string findUserByCellular(const std::string& cellular);
  bool hasServerEvent(unsigned long subSequence) const;
  void StupidChatLinkageFix();
  void addToModifyUsers(unsigned long unique_id, const std::string data);


  Licq::Pipe myNewSocketPipe;
  unsigned long m_nDesiredStatus;
  unsigned short m_nServerSequence;
  unsigned myMaxUsersPerPacket;
  unsigned myErrorTypes;
  std::string myErrorFile;
  int m_nTCPSrvSocketDesc,
      m_nTCPSocketDesc;
  bool m_bLoggingOn,
       m_bRegistering,
       m_bOnlineNotifies,
       m_bVerify,
       // NeedSalt is to let the daemon know when to make a salt request, which
       // should only happen when we first log on. After we get the credentials, we
       // do another logon, but it doesn't need to get a salt.
       m_bNeedSalt;
  time_t m_tLogonTime;
  char *m_szRegisterPasswd;
  pthread_t m_nRegisterThreadId;

  // Services
  COscarService *m_xBARTService;

  static std::list <CReverseConnectToUserData *> m_lReverseConnect;
  static pthread_mutex_t mutex_reverseconnect;
  static pthread_cond_t  cond_reverseconnect_done;

  ContactUserList receivedUserList;

  std::list<Licq::Event*> m_lxRunningEvents;
  mutable pthread_mutex_t mutex_runningevents;
  std::list<Licq::Event*> m_lxExtendedEvents;
  pthread_mutex_t mutex_extendedevents;
  std::list<Licq::Event*> m_lxSendQueue_Server;
  pthread_mutex_t mutex_sendqueue_server;
  std::map <unsigned long, std::string> m_lszModifyServerUsers;
  pthread_mutex_t mutex_modifyserverusers;
  pthread_mutex_t mutex_cancelthread;
  pthread_t thread_monitorsockets,
            thread_ping,
            thread_updateusers,
            thread_ssbiservice;

  pthread_cond_t cond_serverack;
  pthread_mutex_t mutex_serverack;
  unsigned short m_nServerAck;

  friend void *Ping_tep(void *p);
  friend void *UpdateUsers_tep(void *p);
  friend void *MonitorSockets_tep(void *p);
  friend void *ProcessRunningEvent_Client_tep(void *p);
  friend void *ProcessRunningEvent_Server_tep(void *p);
  friend void *Shutdown_tep(void *p);
  friend class COscarService;
  friend class CChatManager;
  friend class CFileTransferManager;
};

extern IcqProtocol gIcqProtocol;

extern Licq::SocketManager gSocketManager;

#endif
