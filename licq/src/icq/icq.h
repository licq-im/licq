/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2012 Licq developers <licq-dev@googlegroups.com>
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

#ifndef LICQICQ_ICQ_H
#define LICQICQ_ICQ_H

#include <licq/icq/icq.h>

#include <boost/shared_array.hpp>
#include <list>
#include <map>
#include <vector>
#include <pthread.h>

#include <licq/event.h>
#include <licq/oneventmanager.h>
#include <licq/pipe.h>
#include <licq/socketmanager.h>
#include <licq/userid.h>

#include "buffer.h"

namespace Licq
{
class EventContactList;
class EventServerMessage;
class EventSms;
class EventUrl;
class INetSocket;
class IniFile;
class Packet;
class ProtoRefuseAuthSignal;
class ProtoRemoveGroupSignal;
class ProtoRenameGroupSignal;
class ProtoSendEventReplySignal;
class ProtoSendFileSignal;
class ProtoSendMessageSignal;
class ProtoSendUrlSignal;
class ProtocolSignal;
class Proxy;
class TCPSocket;
class User;
class UserEvent;
}

class CChatManager;
class CFileTransferManager;

namespace LicqIcq
{
class COscarService;
class CPacketTcp;
class CSrvPacketTcp;
class DcSocket;
class User;

// To keep old code working
typedef std::map<int, std::string> GroupNameMap;

void* Ping_tep(void* p);
void* UpdateUsers_tep(void* p);
void* MonitorSockets_func();
void *ProcessRunningEvent_Client_tep(void *p);
void *ProcessRunningEvent_Server_tep(void *p);
void *ReverseConnectToUser_tep(void *p);

struct PluginList
{
  const char* const name;
  const uint8_t* const guid;
  const char* const description;
};

/**
 * Internal template class for storing and processing received contact list.
 */
class CUserProperties
{
public:
  CUserProperties();

private:
  std::string newAlias;
  std::string newCellular;

  unsigned short normalSid;
  unsigned short groupId;

  unsigned short visibleSid;
  unsigned short invisibleSid;
  bool inIgnoreList;

  bool awaitingAuth;

  TlvList tlvs;

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

enum EDaemonStatus {STATUS_ONLINE, STATUS_OFFLINE_MANUAL, STATUS_OFFLINE_FORCED };

class IcqProtocol : public CICQDaemon
{
public:
  static const struct PluginList info_plugins[2];
  static const struct PluginList status_plugins[3];

  IcqProtocol();
  ~IcqProtocol();

  void initialize();
  bool start();
  void save(Licq::IniFile& licqConf);

  /// Tell ICQ main thread to shut down
  void shutdown()
  { myNewSocketPipe.putChar('X'); }

  /// Process a protocol signal from daemon
  void processSignal(Licq::ProtocolSignal* s);

  bool UseServerSideBuddyIcons() const { return m_bUseBART; }
  void SetUseServerSideBuddyIcons(bool b);

  unsigned long icqSendContactList(const Licq::UserId& userId, const Licq::StringList& users,
      unsigned flags = 0, const Licq::Color* pColor = NULL);

  unsigned long icqFetchAutoResponse(const Licq::UserId& userId);
  unsigned long icqChatRequest(const Licq::UserId& userId, const std::string& reason,
      unsigned flags);
  unsigned long icqMultiPartyChatRequest(const Licq::UserId& userId,
     const std::string& reason, const std::string& chatUsers, unsigned short nPort,
      unsigned flags = 0);
  void icqChatRequestRefuse(const Licq::UserId& userId, const std::string& reason,
      unsigned short nSequence, const unsigned long nMsgID[], bool bDirect);
  void icqChatRequestAccept(const Licq::UserId& userId, unsigned short nPort,
      const std::string& clients, unsigned short nSequence,
      const unsigned long nMsgID[], bool bDirect);
  void icqChatRequestCancel(const Licq::UserId& userId, unsigned short nSequence);
  unsigned long icqRequestInfoPluginList(const Licq::UserId& userId, bool bServer = false);
  unsigned long icqRequestPhoneBook(const Licq::UserId& userId);
  void icqRequestPicture(const Licq::ProtocolSignal* ps);
  unsigned long icqRequestStatusPluginList(const Licq::UserId& userId, bool bServer = false);
  unsigned long icqRequestSharedFiles(const Licq::UserId& userId, bool bServer = false);
  unsigned long icqRequestPhoneFollowMe(const Licq::UserId& userId, bool bServer = false);
  unsigned long icqRequestICQphone(const Licq::UserId& userId, bool bServer = false);
  void icqRegister(const std::string& passwd);
  void icqVerifyRegistration();
  void icqVerify(const std::string& verification);
  unsigned long icqSetWorkInfo(const std::string& city, const std::string& state,
      const std::string& phone, const std::string& fax, const std::string& address,
      const std::string& zip, unsigned short companyCountry, const std::string& name,
      const std::string& department, const std::string& position, unsigned short companyOccupation,
      const std::string& homepage);
  void icqSetGeneralInfo(const Licq::ProtocolSignal* ps);
  unsigned long icqSetEmailInfo(const std::string& emailSecondary, const std::string& emailOld);
  unsigned long icqSetMoreInfo(unsigned short age, char gender,
      const std::string& homepage, unsigned short birthYear, char birthMonth,
      char birthDay, char language1, char language2, char language3);
  unsigned long icqSetSecurityInfo(bool bAuthorize, bool bWebAware);
  unsigned long icqSetInterestsInfo(const Licq::UserCategoryMap& interests);
  unsigned long icqSetOrgBackInfo(const Licq::UserCategoryMap& orgs,
      const Licq::UserCategoryMap& background);
  unsigned long icqSetAbout(const std::string& about);
  unsigned long icqSetPassword(const std::string& password);
  unsigned long setRandomChatGroup(unsigned chatGroup);
  unsigned long randomChatSearch(unsigned chatGroup);
  unsigned long icqSearchWhitePages(const std::string& firstName, const std::string& lastName,
      const std::string& alias, const std::string& email, unsigned short minAge, unsigned short maxAge,
      char gender, char language, const std::string& city, const std::string& state,
      unsigned short countryCode, const std::string& coName, const std::string& coDept,
      const std::string& coPos, const std::string& keyword, bool onlineOnly);
  unsigned long icqSearchByUin(unsigned long);
  void icqAuthorizeGrant(const Licq::ProtocolSignal* ps);
  void icqAuthorizeRefuse(const Licq::ProtoRefuseAuthSignal* ps);
  void icqRequestAuth(const Licq::UserId& userId, const std::string& message);
  void icqAlertUser(const Licq::UserId& userId);
  void icqUpdatePhoneBookTimestamp();
  void icqUpdatePictureTimestamp();
  void icqSetPhoneFollowMeStatus(unsigned newStatus);
  void icqUpdateContactList();
  void icqCheckInvisible(const Licq::UserId& userId);
  unsigned long icqSendSms(const Licq::UserId& userId, const std::string& number, const std::string& message);

  void icqSendMessage(const Licq::ProtoSendMessageSignal* ps);
  void icqSendUrl(const Licq::ProtoSendUrlSignal* ps);
  void icqFileTransfer(const Licq::ProtoSendFileSignal* ps);
  void icqFileTransferRefuse(const Licq::ProtoSendEventReplySignal* ps);
  void icqFileTransferCancel(const Licq::UserId& userId, unsigned short nSequence);
  void icqFileTransferAccept(const Licq::ProtoSendEventReplySignal* ps);
  void icqOpenSecureChannel(const Licq::ProtocolSignal* ps);
  void icqCloseSecureChannel(const Licq::ProtocolSignal* ps);
  void icqOpenSecureChannelCancel(const Licq::UserId& userId, unsigned short nSequence);
  unsigned long icqFetchAutoResponseServer(const Licq::UserId& userId);
  unsigned long logon(unsigned logonStatus);
  unsigned long icqRequestLogonSalt();
  unsigned long icqUserBasicInfo(const Licq::UserId& userId);
  void icqRequestMetaInfo(const Licq::UserId& userId, const Licq::ProtocolSignal* ps = NULL);
  unsigned long setStatus(unsigned newStatus);
  void icqLogoff();
  void postLogoff(int nSD, Licq::Event* cancelledEvent);
  void icqRelogon();
  void icqAddUser(const Licq::UserId& userId, bool _bAuthReq = false);
  void icqAddUserServer(const Licq::UserId& userId, bool _bAuthReq, unsigned short groupId = 0);
  void icqRemoveUser(const Licq::UserId& userId, bool ignored = false);
  void icqRemoveGroup(const Licq::ProtoRemoveGroupSignal* ps);
  void icqChangeGroup(const Licq::UserId& userId);
  void icqRenameGroup(const Licq::ProtoRenameGroupSignal* ps);
  void icqRenameUser(const Licq::UserId& userId);
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
  Licq::Event* icqSendThroughServer(pthread_t caller, unsigned long eventId,
      const Licq::UserId& userId, unsigned char format, const std::string& message,
      Licq::UserEvent*, unsigned short = 0);

  void CheckExport();
  bool openConnectionToUser(const Licq::UserId& userId, Licq::TCPSocket* sock,
     unsigned short nPort);
  bool OpenConnectionToUser(const std::string& name, unsigned long nIp,
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

  bool processPluginMessage(Licq::Buffer& packet, User* user, int channel,
     bool bIsAck, unsigned long nMsgID1,
     unsigned long nMsgID2, unsigned short nSequence,
     Licq::TCPSocket* pSock);
  void ProcessDoneEvent(Licq::Event*);
  bool ProcessSrvPacket(Buffer& packet);

  //--- Channels ---------
  bool ProcessCloseChannel(Buffer& packet);
  void ProcessDataChannel(Buffer& packet);

  //--- Families ---------
  void ProcessServiceFam(Buffer& packet, unsigned short);
  void ProcessLocationFam(Buffer& packet, unsigned short);
  void ProcessBuddyFam(Buffer& packet, unsigned short);
  void ProcessMessageFam(Buffer& packet, unsigned short);
  void ProcessVariousFam(Buffer& packet, unsigned short);
  void ProcessBOSFam(Buffer& packet, unsigned short);
  void processStatsFam(Buffer& packet, int subType);
  void ProcessListFam(Buffer& packet, unsigned short);
  void ProcessAuthFam(Buffer& packet, unsigned short);

  void ProcessUserList();

  void ProcessSystemMessage(Licq::Buffer &packet, unsigned long checkUin, unsigned short newCommand, time_t timeSent);
  void ProcessMetaCommand(Licq::Buffer &packet, unsigned short nMetaCommand, Licq::Event* e);
  bool ProcessTcpPacket(DcSocket*);
  bool ProcessTcpHandshake(DcSocket*);

  unsigned long icqRequestInfoPlugin(User* user, bool, const uint8_t*,
      const Licq::ProtocolSignal* ps = NULL);
  unsigned long icqRequestStatusPlugin(User* user, bool, const uint8_t*);
  void icqUpdateInfoTimestamp(const uint8_t*);

  static bool handshake_Send(DcSocket* s, const Licq::UserId& userId, unsigned short,
                             unsigned short, bool = true, unsigned long = 0);
  static bool Handshake_SendConfirm_v7(DcSocket*);
  static bool Handshake_Recv(DcSocket*, unsigned short, bool = true, bool = false);
  static bool Handshake_RecvConfirm_v7(DcSocket*);

  int ConnectToServer(const std::string& server, unsigned short port);
  int ConnectToLoginServer();
  int connectToUser(const Licq::UserId& userId, int channel);
  int reverseConnectToUser(const Licq::UserId& userId, unsigned long nIp,
     unsigned short nPort, unsigned short nVersion, unsigned short nFailedPort,
     unsigned long nId, unsigned long nMsgID1, unsigned long nMsgID2);
  int requestReverseConnection(const Licq::UserId& userId, unsigned long, unsigned long,
                               unsigned short, unsigned short);
  bool waitForReverseConnection(unsigned short id, const Licq::UserId& userId);

  void PushEvent(Licq::Event*);
  void PushExtendedEvent(Licq::Event*);

  bool UseServerContactList() const;

  EDaemonStatus Status() const                  { return m_eStatus; }

  static unsigned short dcVersionToUse(unsigned short v_in);

  // Proxy options
  void InitProxy();
  Licq::Proxy* GetProxy() {  return m_xProxy;  }

  bool directMode() const;

  static unsigned short icqStatusFromStatus(unsigned status);
  static unsigned statusFromIcqStatus(unsigned short icqStatus);
  static unsigned long addStatusFlags(unsigned long nStatus, const User* u);

  static int getGroupFromId(unsigned short gsid);

  static unsigned short generateSid();

  static unsigned long icqOwnerUin();

  static std::string parseDigits(const std::string& number);
  static std::string parseRtf(const std::string& rtf);
  static std::string pipeInput(const std::string& message);

  static std::string getXmlTag(const std::string& xmlSource, const std::string& tagName);

private:
  static const int PingFrequency = 60;
  static const int UpdateFrequency = 60;
  static const int LogonAttemptDelay = 300;
  static const int MaxPingTimeouts = 3;

  bool SendEvent(int nSD, Licq::Packet &, bool);
  bool SendEvent(Licq::INetSocket *, Licq::Packet &, bool);
  void SendEvent_Server(Licq::Packet *packet, const Licq::ProtocolSignal* ps = NULL);
  Licq::Event* SendExpectEvent_Server(const Licq::ProtocolSignal* ps, const Licq::UserId& userId,
      CSrvPacketTcp*, Licq::UserEvent*, bool = false);

  Licq::Event* SendExpectEvent_Server(const Licq::UserId& userId, CSrvPacketTcp* packet, Licq::UserEvent* ue, bool extendedEvent = false)
  { return SendExpectEvent_Server(NULL, userId, packet, ue, extendedEvent); }

  Licq::Event* SendExpectEvent_Server(const Licq::ProtocolSignal* ps, CSrvPacketTcp* packet,
      Licq::UserEvent* ue, bool extendedEvent = false)
  { return SendExpectEvent_Server(ps, Licq::UserId(), packet, ue, extendedEvent); }

  Licq::Event* SendExpectEvent_Server(CSrvPacketTcp* packet, Licq::UserEvent* ue, bool extendedEvent = false)
  { return SendExpectEvent_Server(NULL, Licq::UserId(), packet, ue, extendedEvent); }

  Licq::Event* SendExpectEvent_Client(const Licq::ProtocolSignal* ps,
      const User* user, CPacketTcp* packet, Licq::UserEvent* ue);

  Licq::Event* SendExpectEvent_Client(const User* user, CPacketTcp* packet, Licq::UserEvent* ue)
  { return SendExpectEvent_Client(NULL, user, packet, ue); }

  Licq::Event* SendExpectEvent(Licq::Event*, void *(*fcn)(void *));
  unsigned eventCommandFromPacket(Licq::Packet* p);

  void AckTCP(CPacketTcp &, int);
  void AckTCP(CPacketTcp &, Licq::TCPSocket*);

  static std::string getUserEncoding(const Licq::UserId& userId);

  void setEventThread(unsigned long eventId, pthread_t plugin_thread);

  /**
   * Split a string into parts delimited by 0xFE
   *
   * @param ret List to return substrings in
   * @param s String to split
   * @param count Number of substrings to find or zero to get all
   * @param userEncoding Encoding to convert resulting strings from
   */
  static void splitFE(std::vector<std::string>& ret, const std::string& s,
      int count, const std::string& userEncoding);

  static Licq::EventUrl* parseUrlEvent(const std::string& s, time_t timeSent,
      unsigned long flags, const std::string& userEncoding);
  static Licq::EventContactList* parseContactEvent(const std::string& s,
      time_t timeSent, unsigned long flags, const std::string& userEncoding);

  // Common message handler
  void ProcessMessage(Licq::User* user, Licq::Buffer& packet,
      const std::string& message, unsigned short nMsgType, unsigned long nMask,
      const unsigned long nMsgID[], unsigned short nSequence, bool bIsAck,
      bool &bNewUser);

  /**
   * Parse a message into a Licq user event object
   *
   * @param type Message type (ICQ protocol constant)
   * @param packet Packet to log on failure
   * @param userId Id of user associated with message
   * @param message Message block to parse
   * @param timeSent Timestamp of message
   * @param flags User event flags
   */
  void processServerMessage(int type, Licq::Buffer& packet,
      const Licq::UserId& userId, std::string& message, time_t timeSent,
      unsigned long flags);

  void processIconHash(User* u, Buffer& packet);

  void ChangeUserStatus(User* u, unsigned long s, time_t onlineSince = 0);
  std::string findUserByCellular(const std::string& cellular);
  bool hasServerEvent(unsigned long subSequence) const;
  void StupidChatLinkageFix();
  void addToModifyUsers(unsigned long unique_id, const std::string data);

  unsigned long icqLogon();
  unsigned long icqSetStatus(unsigned short newStatus);

  /**
   * Identify a user client based on available data
   *
   * @param caps Raw capabilities data
   * @param numCaps Size of capabilities data
   * @param userClass User class
   * @param tcpVersion Protocol version for direct contact
   * @param ts1 Last info update time
   * @param ts2 Last ext info update time
   * @param ts3 Last ext status update time
   * @param onlineSince Time user went online
   * @param webPort Web front port
   * @return Empty string if detection failed, otherwise name and (if possible) version of user client
   */
  std::string detectUserClient(const char* caps, int capSize, int userClass,
      int tcpVersion, unsigned ts1, unsigned ts2, unsigned ts3,
      time_t onlineSince, int webPort);

  Licq::Pipe myNewSocketPipe;
  unsigned long m_nDesiredStatus;
  unsigned short m_nServerSequence;
  unsigned myMaxUsersPerPacket;
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
  std::string myRegisterPasswd;
  pthread_t m_nRegisterThreadId;
  bool myDirectMode;
  EDaemonStatus m_eStatus;
  Licq::Proxy* m_xProxy;

  // Services
  COscarService *m_xBARTService;
  bool m_bUseBART; // server side buddy icons

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
  pthread_t thread_ping,
            thread_updateusers,
            thread_ssbiservice;

  pthread_cond_t cond_serverack;
  pthread_mutex_t mutex_serverack;
  unsigned short m_nServerAck;

  friend void *Ping_tep(void *p);
  friend void *UpdateUsers_tep(void *p);
  friend void *MonitorSockets_func();
  friend void *ProcessRunningEvent_Client_tep(void *p);
  friend void *ProcessRunningEvent_Server_tep(void *p);
  friend class COscarService;
  friend class ::CChatManager;
  friend class ::CFileTransferManager;
};

extern IcqProtocol gIcqProtocol;

} // namespace LicqIcq

extern Licq::SocketManager gSocketManager;

#endif
