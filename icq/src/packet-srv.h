/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2012 Licq developers <licq-dev@googlegroups.com>
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

#ifndef LICQICQ_PACKET_SRV_H
#define LICQICQ_PACKET_SRV_H

#include <cstring>
#include <list>
#include <map>
#include <pthread.h>
#include <stdint.h>
#include <string>

#include <licq/userid.h>
#include <licq/packet.h>

#include "buffer.h"
#include "defines.h"

namespace Licq
{
typedef std::list<std::string> StringList;
typedef std::map<unsigned int, std::string> UserCategoryMap;
class Buffer;
class INetSocket;
class User;
}

namespace LicqIcq
{
class User;

typedef Licq::Buffer CBuffer;
typedef Licq::Packet CPacket;

typedef std::map<int, std::string> GroupNameMap;

// values of extra info to identify plugin request
const unsigned short ServerInfoPluginRequest     = 3;
const unsigned short ServerStatusPluginRequest   = 4;


//=====ServerTCP===============================================================

//-----SrvPacketTcp------------------------------------------------------------

class CSrvPacketTcp : public CPacket
{
public:
  virtual ~CSrvPacketTcp();

  virtual Licq::Buffer* getBuffer() { return buffer; }

  // Packet details
  unsigned char icqChannel() const { return myIcqChannel; }
  virtual unsigned short Sequence()    { return m_nSequence; }
  virtual unsigned short SubSequence() { return m_nSubSequence; }
  virtual unsigned long  SNAC() { return ((m_nFamily << 16) | (m_nSubType)); }
  virtual unsigned short SubCommand()  { return m_nSubCommand; }

  // Misc.
  virtual unsigned short ExtraInfo() { return m_nExtraInfo; }
  virtual CBuffer *Finalize(Licq::INetSocket*);
  void SetExtraInfo(unsigned short e)  { m_nExtraInfo = e; }

protected:
  CSrvPacketTcp(unsigned char);
  void InitBuffer();
  static void initSequence(int service);

  static bool s_bRegistered;
  static unsigned short s_nSequence[32];
  static unsigned short s_nSubSequence;
  static pthread_mutex_t s_xMutex;

  unsigned char myIcqChannel;
  unsigned short m_nSequence;
  unsigned short m_nSubSequence;
  unsigned short m_nFamily;
  unsigned short m_nSubType;
  unsigned short m_nSubCommand;
  unsigned short m_nExtraInfo;
  unsigned short m_nService;

  char *m_szSequenceOffset;

  Buffer* buffer;
};

//=====UDP======================================================================

//-----PacketUdp----------------------------------------------------------------
class CPacketUdp : public CPacket
{
public:
   virtual ~CPacketUdp();

  virtual Licq::Buffer* getBuffer() { return buffer; }

   virtual CBuffer *Finalize(Licq::INetSocket*);
   virtual unsigned short Sequence() { return m_nSequence; }
   virtual unsigned short SubSequence() { return m_nSubSequence; }
   virtual unsigned short SubCommand()  { return 0; }
protected:
   CPacketUdp(unsigned short _nCommand);
   void InitBuffer();

   static bool s_bRegistered;

#if ICQ_VERSION == 2
   unsigned short m_nVersion;
   unsigned short m_nCommand;
   unsigned short m_nSequence;
   unsigned short m_nSubSequence;
#elif ICQ_VERSION == 4
   unsigned short m_nVersion;
   unsigned short m_nRandom;
   unsigned short m_nZero;
   unsigned short m_nCommand;
   unsigned short m_nSequence;
   unsigned short m_nSubSequence;
   unsigned long  m_nCheckSum;
#else
   unsigned short m_nVersion;
   unsigned long  m_nZero;
   unsigned long  m_nSessionId;
   unsigned short m_nCommand;
   unsigned short m_nSequence;
   unsigned short m_nSubSequence;
   unsigned long  m_nCheckSum;
#endif

   static unsigned short s_nSequence;
   static unsigned short s_nSubSequence;
   static unsigned long  s_nSessionId;

  Licq::Buffer* buffer;
};

class CPU_ConnectStart : public CSrvPacketTcp
{
public:
  CPU_ConnectStart();
};

//-----Logon--------------------------------------------------------------------
class CPU_Logon : public CSrvPacketTcp
{
public:
  CPU_Logon(const std::string& password, const std::string& accountId, unsigned short _nLogonStatus);
  virtual ~CPU_Logon();
protected:
  unsigned long  m_nLogonStatus;
  unsigned long  m_nTcpVersion;
};

//-----Logoff-------------------------------------------------------------------
class CPU_Logoff : public CSrvPacketTcp
{
public:
   CPU_Logoff();
   virtual ~CPU_Logoff();
};

//-----SendCookie---------------------------------------------------------------
class CPU_SendCookie : public CSrvPacketTcp
{
public:
  CPU_SendCookie(const std::string& cookie, unsigned short nService = 0);
  virtual ~CPU_SendCookie();
};

//-----CommonFamily----------------------------------------------------------
class CPU_CommonFamily : public CSrvPacketTcp
{
public:
  CPU_CommonFamily(unsigned short Family, unsigned short SubType);
  virtual ~CPU_CommonFamily();

protected:
  void InitBuffer();
};

class CPU_GenericFamily : public CPU_CommonFamily
{
public:
  CPU_GenericFamily(unsigned short Family, unsigned short SubType,
                    unsigned short nService = 0);
  virtual ~CPU_GenericFamily();
};

class CPU_RequestLogonSalt : public CPU_CommonFamily
{
public:
  CPU_RequestLogonSalt(const std::string &);
};

class CPU_NewLogon : public CPU_CommonFamily
{
public:
  CPU_NewLogon(const std::string& password, const std::string& accountId, const std::string& md5Salt);
};

#if ICQ_VERSION == 2 || ICQ_VERSION == 6
//-----Register----------------------------------------------------------------
// Doesn't actually descend from CPacketUdp in version 2 but we keep the
// name the same for simplicity
class CPU_Register : public CPacketUdp
{
public:
  CPU_Register(const std::string& password);
  virtual ~CPU_Register();

  virtual unsigned short Sequence() { return m_nSequence; }
  virtual unsigned short SubSequence() { return 0; }
  virtual unsigned short SubCommand() { return 0; }
protected:
  virtual unsigned long getSize()       { return 1; }

  /* 02 00 FC 03 01 00 02 00 04 00 65 66 67 00 72 00 00 00 00 00 00 00 */
  unsigned short m_nVersion;
  unsigned short m_nCommand;
  unsigned short m_nSequence;
  unsigned short m_nUnknown1;
  unsigned short m_nPasswdLen;
  std::string myPassword;
  unsigned long  m_nUnknown2;
  unsigned long  m_nUnknown3;
};
#elif ICQ_VERSION == 4 || ICQ_VERSION == 5
//-----Register----------------------------------------------------------------
class CPU_Register : public CPacketUdp
{
public:
  CPU_Register(const std::string& password);
  virtual ~CPU_Register();
};
#elif ICQ_VERSION > 6
class CPU_RegisterFirst : public CSrvPacketTcp
{
public:
  CPU_RegisterFirst();
  virtual ~CPU_RegisterFirst();
};

class CPU_Register : public CPU_CommonFamily
{
public:
  CPU_Register(const std::string& password);
  virtual ~CPU_Register();
};
#endif

//-----VerifyRegistration------------------------------------------------------
class CPU_VerifyRegistration : public CPU_CommonFamily
{
public:
  CPU_VerifyRegistration();
  virtual ~CPU_VerifyRegistration();
};

//-----SendVerification--------------------------------------------------------
class CPU_SendVerification : public CPU_CommonFamily
{
public:
  CPU_SendVerification(const std::string& password, const std::string& verification);
  virtual ~CPU_SendVerification();
};

//-----ListRequestRights-----------------------------------------------------
class CPU_ListRequestRights : public CPU_CommonFamily
{
public:
  CPU_ListRequestRights();
};

//-----ImICQ------------------------------------------------------------------
class CPU_ImICQ : public CPU_CommonFamily
{
public:
  CPU_ImICQ();
  CPU_ImICQ(unsigned short VerArray[][2], unsigned short NumVer,
            unsigned short nService);
};

//-----ICQMode------------------------------------------------------------------
class CPU_ICQMode : public CPU_CommonFamily
{
public:
    CPU_ICQMode(unsigned short nChannel, unsigned long nFlags);
};

//-----CapabilitySettings-------------------------------------------------------------
class CPU_CapabilitySettings : public CPU_CommonFamily
{
public:
  CPU_CapabilitySettings();
};

//-----RateAck-----------------------------------------------------------------
class CPU_RateAck : public CPU_CommonFamily
{
public:
  CPU_RateAck(unsigned short nService = 0);
};

//-----GenericUinList------------------------------------------------------------
class CPU_GenericUinList : public CPU_CommonFamily
{
public:
   CPU_GenericUinList(const std::string& accountId, unsigned short Family, unsigned short Subtype);
   CPU_GenericUinList(const Licq::StringList& users, unsigned short, unsigned short);
   CPU_GenericUinList(unsigned long _nUin, unsigned short Family, unsigned short Subtype);
};

//-----RequestList--------------------------------------------------------------
class CPU_RequestList : public CPU_CommonFamily
{
public:
  CPU_RequestList();

protected:
  time_t         m_nSavedTime;
  unsigned short m_nRecords;
};

//-----ExportContactStart-------------------------------------------------------
class CPU_ExportContactStart : public CPU_CommonFamily
{
public:
  CPU_ExportContactStart();
};

//-----ExportToServerList-------------------------------------------------------
class CPU_ExportToServerList : public CPU_CommonFamily
{
public:
  CPU_ExportToServerList(const std::list<Licq::UserId>& users, unsigned short type);
};

//-----ExportGroupsToServerList-------------------------------------------------
class CPU_ExportGroupsToServerList : public CPU_CommonFamily
{
public:
  CPU_ExportGroupsToServerList(const GroupNameMap& groups);
};

//-----AddPrivacyInfo-----------------------------------------------------------
class CPU_AddPDINFOToServerList : public CPU_CommonFamily
{
public:
  CPU_AddPDINFOToServerList();

  unsigned short GetSID()   { return m_nSID; }
  unsigned short GetGSID()  { return m_nGSID; }

protected:
  unsigned short m_nSID,
                 m_nGSID;
};

//-----AddToServerList----------------------------------------------------------
class CPU_AddToServerList : public CPU_CommonFamily
{
public:
  CPU_AddToServerList(const Licq::UserId& userId, unsigned short _nType,
      unsigned short _nGroup = 0, bool _bAuthReq = false);

  CPU_AddToServerList(const std::string& groupName,
                      bool _bAuthReq = false, bool _bTopLevel = false);

  unsigned short GetSID()   { return m_nSID; }
  unsigned short GetGSID()  { return m_nGSID; }

protected:
  void init(const std::string& name, unsigned short _nType, bool _bAuthReq, bool _bTopLevel);

  unsigned short m_nSID,
                 m_nGSID;
  Buffer tlvBuffer;
};

//-----RemoveFromServerList-----------------------------------------------------
class CPU_RemoveFromServerList : public CPU_CommonFamily
{
public:
  CPU_RemoveFromServerList(const Licq::UserId& userId, unsigned short _nGSID,
      unsigned short _nSID, unsigned short _nType);
  CPU_RemoveFromServerList(const std::string& name, unsigned short _nGSID);

protected:
  void init(const std::string& name, unsigned short _nGSID,
      unsigned short _nSID, unsigned short _nType);

  Buffer tlvBuffer;
};

//-----ClearServerList----------------------------------------------------------
class CPU_ClearServerList : public CPU_CommonFamily
{
public:
  CPU_ClearServerList(const std::list<Licq::UserId>& userIds, unsigned short type);
};

//-----UpdateToServerList-------------------------------------------------------
class CPU_UpdateToServerList : public CPU_CommonFamily
{
public:
  CPU_UpdateToServerList(const Licq::UserId& userId, unsigned short _nType, bool _bAuthReq = false);
  CPU_UpdateToServerList(const std::string& name, unsigned short _nSID = 0);
protected:
  void init(const std::string& name, unsigned short _nType, bool _bAuthReq,
      unsigned short nGSID, unsigned short nSID, unsigned short nExtraLen);

  Buffer tlvBuffer;
  std::list<unsigned long> groupIds;
};

//-----SetPrivacy---------------------------------------------------------------
class CPU_SetPrivacy : public CPU_CommonFamily
{
public:
  CPU_SetPrivacy(unsigned char _nPrivacy);
};

//-----SetStatus----------------------------------------------------------------
class CPU_SetStatus : public CPU_CommonFamily
{
public:
  CPU_SetStatus(unsigned long _nNewStatus);

private:
  unsigned long m_nNewStatus;
};

class CPU_SetStatusFamily : public CPU_CommonFamily
{
public:
  CPU_SetStatusFamily();

protected:
  void InitBuffer();
  unsigned long m_nNewStatus;
};

class CPU_SetLogonStatus : public CPU_SetStatusFamily
{
public:
  CPU_SetLogonStatus(unsigned long _nNewStatus);
};

class CPU_UpdateInfoTimestamp : public CPU_SetStatusFamily
{
public:
  CPU_UpdateInfoTimestamp(const uint8_t* GUID);
};

class CPU_UpdateStatusTimestamp : public CPU_SetStatusFamily
{
public:
  CPU_UpdateStatusTimestamp(const uint8_t* GUID, unsigned long nState,
    unsigned long nStatus = ICQ_STATUS_OFFLINE);
};

class CPU_UpdateTimestamp : public CPU_SetStatusFamily
{
public:
  CPU_UpdateTimestamp();
};

//-----ClientReady--------------------------------------------------------------
class CPU_ClientReady : public CPU_CommonFamily
{
public:
   CPU_ClientReady();
   CPU_ClientReady(unsigned short VerArray[][4], unsigned short NumVer,
                   unsigned short nService);
};

//-----ClientAckNameInfo--------------------------------------------------------
class CPU_AckNameInfo : public CPU_CommonFamily
{
public:
   CPU_AckNameInfo();
};

//-----RequestSysMsg------------------------------------------------------------
class CPU_RequestSysMsg : public CPU_CommonFamily
{
public:
   CPU_RequestSysMsg();
   virtual unsigned short SubCommand()   { return m_nMetaCommand; }

protected:
   unsigned short m_nMetaCommand;
};

//-----SysMsgDoneAck------------------------------------------------------------
class CPU_SysMsgDoneAck : public CPU_CommonFamily
{
public:
  CPU_SysMsgDoneAck(unsigned short nId);
  virtual unsigned short SubCommand()   { return m_nMetaCommand; }

protected:
  unsigned short m_nMetaCommand;
};

//-----TypingNotification------------------------------------------------------
class CPU_TypingNotification : public CPU_CommonFamily
{
public:
  CPU_TypingNotification(const std::string& accountId, bool bActive);
};

//-----CheckInvisible----------------------------------------------------------
class CPU_CheckInvisible : public CPU_CommonFamily
{
public:
  CPU_CheckInvisible(const std::string& accountId);
};

//-----Ack---------------------------------------------------------------------
class CPU_Ack : public CPacketUdp
{
public:
#if ICQ_VERSION == 2
   CPU_Ack(unsigned short _nSequence);
#else
   CPU_Ack(unsigned short _nSequence, unsigned short _nSubSequence);
#endif
};


//-----AddUser------------------------------------------------------------------
class CPU_AddUser : public CPacketUdp
{
public:
   CPU_AddUser(unsigned long _nAddedUin);
protected:
   /* 02 00 3C 05 06 00 50 A5 82 00 8F 76 20 00 */
   unsigned long  m_nAddedUin;
};


//-----SearchWhitePages---------------------------------------------------------
class CPU_SearchWhitePages : public CPU_CommonFamily
{
public:
  CPU_SearchWhitePages(const std::string& firstName, const std::string& lastName,
      const std::string& alias, const std::string& email, unsigned short nMinAge,
      unsigned short nMaxAge, char nGender, char nLanguage, const std::string& city,
      const std::string& state, unsigned short nCountryCode, const std::string& coName,
      const std::string& coDept, const std::string& coPos, const std::string& keyword,
      bool bOnlineOnly);
   virtual unsigned short SubCommand()   { return m_nMetaCommand; }
   unsigned long Uin()  { return 0; }
  void packSearch(unsigned short nCmd, const std::string& field);

protected:
   unsigned long m_nMetaCommand;
};


//-----SearchByUin--------------------------------------------------------------
class CPU_SearchByUin : public CPU_CommonFamily
{
public:
   CPU_SearchByUin(unsigned long nUin);
   virtual unsigned short SubCommand()   { return m_nMetaCommand; }
protected:
   unsigned long m_nMetaCommand;
};


//-----UpdatePersonalBasicInfo--------------------------------------------------
class CPU_UpdatePersonalBasicInfo : public CPacketUdp
{
public:
  CPU_UpdatePersonalBasicInfo(const std::string& alias, const std::string& firstName,
      const std::string& lastName, const std::string& email, bool _bAuthorization);

  const std::string& alias() const { return myAlias; }
  const std::string& firstName() const { return myFirstName; }
  const std::string& lastName() const { return myLastName; }
  const std::string& email() const { return myEmail; }
   bool Authorization()  { return m_nAuthorization == 0; }
protected:
  std::string myAlias;
  std::string myFirstName;
  std::string myLastName;
  std::string myEmail;
  char m_nAuthorization;
};


//-----UpdatePersonalExtInfo-------------------------------------------------------
class CPU_UpdatePersonalExtInfo : public CPacketUdp
{
public:
  CPU_UpdatePersonalExtInfo(const std::string& city, unsigned short _nCountry,
      const std::string& state, unsigned short _nAge, char _cSex, const std::string& phone,
      const std::string& homepage, const std::string& about, unsigned long _nZipcode);

  const std::string& city() const { return myCity; }
   unsigned short Country()  { return m_nCountry; }
  const std::string& state() const { return myState; }
   unsigned short Age()  { return m_nAge; }
   char Sex()  { return m_cSex; }
  const std::string& phoneNumber() const { return myPhone; }
  const std::string& homepage() const { return myHomepage; }
  const std::string& about() const { return myAbout; }
   unsigned long Zipcode() { return m_nZipcode; }
protected:
  std::string myCity;
   unsigned short m_nCountry;
   char           m_cTimezone;
  std::string myState;
   unsigned short m_nAge;
   char           m_cSex;
  std::string myPhone;
  std::string myHomepage;
  std::string myAbout;
   unsigned long  m_nZipcode;
};


//-----Ping---------------------------------------------------------------------
class CPU_Ping : public CSrvPacketTcp
{
public:
   CPU_Ping();
};


//-----ThroughServer-----------------------------------------------------------
class CPU_ThroughServer : public CPU_CommonFamily
{
public:
  CPU_ThroughServer(const std::string& accountId, unsigned char format, const std::string& message,
      unsigned short _nCharset = 0, bool bOffline = true);
protected:
  unsigned char  m_nMsgType;
};

//-----Type2Message-------------------------------------------------------------
class CPU_Type2Message : public CPU_CommonFamily
{
public:
  CPU_Type2Message(const User* u, bool _bAck, bool _bDirectInfo, const uint8_t* cap,
                   unsigned long nMsgID1 = 0,
                   unsigned long nMsgID2 = 0);
protected:
  void InitBuffer();

  const User* m_pUser;
  bool m_bAck;
  bool m_bDirectInfo;
  unsigned long m_nMsgID[2];
  uint8_t m_cap[CAP_LENGTH];
  unsigned long m_nExtraLen; // length of data following 0x2711 tlv
};

//-----ReverseConnect-----------------------------------------------------------
class CPU_ReverseConnect : public CPU_Type2Message
{
public:
  CPU_ReverseConnect(const User* u, unsigned long nLocalIP,
                     unsigned short nLocalPort, unsigned short nRemotePort);
};

//-----ReverseConnectFailed-----------------------------------------------------
class CPU_ReverseConnectFailed : public CPU_CommonFamily
{
public:
  CPU_ReverseConnectFailed(const std::string& accountId, unsigned long nMsgID1,
                           unsigned long nMsgID2, unsigned short nFailedPort,
                           unsigned short nOurPort, unsigned long nConnectID);
};

//-----PluginMessage-----------------------------------------------------------
class CPU_PluginMessage : public CPU_Type2Message
{
public:
  CPU_PluginMessage(const User* u, bool bAck, const uint8_t* PluginGUID,
                    unsigned long nMsgID1 = 0, unsigned long nMsgID2 = 0);

protected:
  void InitBuffer();

  uint8_t m_PluginGUID[GUID_LENGTH];
};

//-----InfoPluginRequest-------------------------------------------------------
class CPU_InfoPluginReq : public CPU_PluginMessage
{
public:
  CPU_InfoPluginReq(const User* u, const uint8_t* GUID, unsigned long nTime);
  virtual const char *RequestGUID() { return m_ReqGUID; }
  virtual unsigned short ExtraInfo() { return ServerInfoPluginRequest; }

protected:
  char m_ReqGUID[GUID_LENGTH];
};

//-----StatusPluginRequest-----------------------------------------------------
class CPU_StatusPluginReq : public CPU_PluginMessage
{
public:
  CPU_StatusPluginReq(const User* u, const uint8_t* GUID, unsigned long nTime);
  virtual unsigned short ExtraInfo() { return ServerStatusPluginRequest; }
  virtual const char *RequestGUID() { return m_ReqGUID; }

protected:
  char m_ReqGUID[GUID_LENGTH];
};

//-----AdvancedMessage---------------------------------------------------------
class CPU_AdvancedMessage : public CPU_Type2Message
{
public:
  CPU_AdvancedMessage(const User* u, unsigned short _nMsgType,
                      unsigned short _nMsgFlags, bool _bAck,
                      unsigned short _nSequence,
                      unsigned long nID1 = 0,
                      unsigned long nID2 = 0);
protected:
  void InitBuffer();

  unsigned short m_nMsgType;
  unsigned short m_nMsgFlags;
  unsigned short m_nSequence;
};

//-----ChatRequest-------------------------------------------------------------
class CPU_ChatRequest : public CPU_AdvancedMessage
{
public:
  CPU_ChatRequest(const std::string& message, const std::string& chatUsers, unsigned short nPort,
      unsigned short nLevel, const User* pUser, bool bICBM);
};

//-----FileTransfer------------------------------------------------------------
class CPU_FileTransfer : public CPU_AdvancedMessage
{
public:
  CPU_FileTransfer(const User* u, const std::list<std::string>& lFileList,
      const std::string& file, const std::string& desc, unsigned short nLevel, bool bICBM);

  bool IsValid()                { return m_bValid; }
  const std::list<std::string>& GetFileList() const { return m_lFileList; }
  const std::string& filename() const { return myFilename; }
  const std::string& description() const { return myDesc; }
  unsigned long GetFileSize()   { return m_nFileSize; }

private:
  bool          m_bValid;
  std::string myDesc;
  std::string myFilename;
  std::list<std::string> m_lFileList;
  unsigned long m_nFileSize;
};

//-----NoManager--------------------------------------------------------
class CPU_NoManager : public CPU_CommonFamily
{
public:
  CPU_NoManager(const Licq::User* u, unsigned long nMsgID1, unsigned long nMsgID2);
};

//-----AckThroughServer--------------------------------------------------------
class CPU_AckThroughServer : public CPU_CommonFamily
{
public:
  CPU_AckThroughServer(const Licq::User* u, unsigned long msgid1,
                         unsigned long msgid2, unsigned short sequence,
                         unsigned short msgType, bool bAccept,
                         unsigned short nLevel, const uint8_t* GUID);
protected:
  void InitBuffer();

  unsigned long m_nMsgID[2];
  unsigned short m_nSequence, m_nMsgType, m_nStatus, m_nLevel;
  std::string myAccountId;
  std::string myMessage;
  uint8_t m_GUID[GUID_LENGTH];
};

//-----AckGeneral--------------------------------------------------------------
class CPU_AckGeneral : public CPU_AckThroughServer
{
public:
  CPU_AckGeneral(const Licq::User* u, unsigned long nMsgID1,
                 unsigned long nMsgID2, unsigned short nSequence,
                 unsigned short nMsgType, bool bAccept, unsigned short nLevel);
};

//-----AckFileAccept-----------------------------------------------------------
class CPU_AckFileAccept : public CPU_AdvancedMessage
{
public:
  CPU_AckFileAccept(const User* u, const unsigned long nMsgID[],
      unsigned short nSequence, unsigned short nPort, const std::string& desc,
      const std::string& file, unsigned long nFileSize);
};

//-----AckFileRefuse-----------------------------------------------------------
class CPU_AckFileRefuse : public CPU_AckThroughServer
{
public:
  CPU_AckFileRefuse(const User* u, const unsigned long nMsgID[],
      unsigned short nSequence, const std::string& message);
};

//-----AckChatAccept-----------------------------------------------------------
class CPU_AckChatAccept : public CPU_AdvancedMessage
{
public:
  CPU_AckChatAccept(const User* u, const std::string& clients,
      const unsigned long nMsgID[], unsigned short nSequence, unsigned short nPort);
};

//-----AckChatRefuse-----------------------------------------------------------
class CPU_AckChatRefuse : public CPU_AckThroughServer
{
public:
  CPU_AckChatRefuse(const User* u, const unsigned long nMsgID[],
      unsigned short nSequence, const std::string& message);
};

//-----PluginError-------------------------------------------------------------
class CPU_PluginError : public CPU_AckThroughServer
{
public:
  CPU_PluginError(const Licq::User* u, unsigned long nMsgID1, unsigned long nMsgID2,
                  unsigned short nSequence, const uint8_t* GUID);
};

//-----InfoPluginListResp------------------------------------------------------
class CPU_InfoPluginListResp : public CPU_AckThroughServer
{
public:
  CPU_InfoPluginListResp(const Licq::User* u, unsigned long nMsgID1,
                         unsigned long nMsgID2, unsigned short nSequence);
};

//-----InfoPhoneBookResp-------------------------------------------------------
class CPU_InfoPhoneBookResp : public CPU_AckThroughServer
{
public:
  CPU_InfoPhoneBookResp(const Licq::User* u, unsigned long nMsgID1,
                        unsigned long nMsgID2, unsigned short nSequence);
};

//-----InfoPictureResp---------------------------------------------------------
class CPU_InfoPictureResp : public CPU_AckThroughServer
{
public:
  CPU_InfoPictureResp(const Licq::User* u, unsigned long nMsgID1,
                      unsigned long nMsgID2, unsigned short nSequence);
};

//-----StatusPluginListResp----------------------------------------------------
class CPU_StatusPluginListResp : public CPU_AckThroughServer
{
public:
  CPU_StatusPluginListResp(const Licq::User* u, unsigned long nMsgID1,
                           unsigned long nMsgID2, unsigned short nSequence);
};

//-----StatusPluginResp-----------------------------------------------------
class CPU_StatusPluginResp : public CPU_AckThroughServer
{
public:
  CPU_StatusPluginResp(const Licq::User* u, unsigned long nMsgID1,
                       unsigned long nMsgID2, unsigned short nSequence,
                       unsigned long nStatus);
};

//-----SendSms-----------------------------------------------------------
class CPU_SendSms : public CPU_CommonFamily
{
public:
  CPU_SendSms(const std::string& number, const std::string& message);
   virtual unsigned short SubCommand() { return m_nMetaCommand; }
protected:
   unsigned long m_nMetaCommand;
};

//-----ReverseTCPRequest--------------------------------------------------------

class CPU_ReverseTCPRequest : public CPacketUdp
{
public:
   CPU_ReverseTCPRequest(unsigned long _nDestinationUin, unsigned long _nIP,
                         unsigned short _nPort, unsigned short _nPort2);
protected:
   unsigned long  m_nDestinationUin;
};

//-----RequestAuth--------------------------------------------------------------
class CPU_RequestAuth : public CPU_CommonFamily
{
public:
  CPU_RequestAuth(const std::string& accountId, const std::string& message);
};

//-----Authorize----------------------------------------------------------------
class CPU_Authorize : public CPU_CommonFamily
{
public:
   CPU_Authorize(const std::string& accountId);
};




//-----SetRandomChatGroup----------------------------------------------------
class CPU_SetRandomChatGroup : public CPU_CommonFamily
{
public:
  CPU_SetRandomChatGroup(unsigned long nGroup);
  unsigned long Group() { return m_nGroup; }
  virtual unsigned short SubCommand() { return m_nMetaCommand; }
protected:
  unsigned long m_nGroup;
  unsigned long m_nMetaCommand;
};



//-----RandomChatSearch----------------------------------------------------
class CPU_RandomChatSearch : public CPU_CommonFamily
{
public:
  CPU_RandomChatSearch(unsigned long nGroup);
  virtual unsigned short SubCommand() { return m_nMetaCommand; }
protected:
  unsigned long m_nMetaCommand;
};



//-----Meta_SetGeneralInfo-----------------------------------------------------
class CPU_Meta_SetGeneralInfo : public CPU_CommonFamily
{
public:
  CPU_Meta_SetGeneralInfo(const std::string& alias, const std::string& firstName,
      const std::string& lastName, const std::string& emailPrimary,
      const std::string& city, const std::string& state, const std::string& phoneNumber,
      const std::string& faxNumber, const std::string& address,
      const std::string& cellularNumber, const std::string& zipCode,
      unsigned short nCountryCode, bool bHideEmail);
  virtual unsigned short SubCommand()  { return m_nMetaCommand; }
protected:
  unsigned short m_nMetaCommand;

  std::string myAlias;
  std::string myFirstName;
  std::string myLastName;
  std::string myEmailPrimary;
  std::string myCity;
  std::string myState;
  std::string myPhoneNumber;
  std::string myFaxNumber;
  std::string myAddress;
  std::string myCellularNumber;
  std::string myZipCode;
  unsigned short m_nCountryCode;
  char m_nTimezone;
  char m_nHideEmail;

  friend class IcqProtocol;
};


//-----Meta_SetMoreEmailInfo---------------------------------------------------
class CPU_Meta_SetEmailInfo : public CPU_CommonFamily
{
public:
  CPU_Meta_SetEmailInfo(const std::string& emailSecondary, const std::string& emailOld);
  virtual unsigned short SubCommand()  { return m_nMetaCommand; }
protected:
  unsigned short m_nMetaCommand;

  std::string myEmailSecondary;
  std::string myEmailOld;

  friend class IcqProtocol;
};


//-----Meta_SetMoreInfo------------------------------------------------------
class CPU_Meta_SetMoreInfo : public CPU_CommonFamily
{
public:
  CPU_Meta_SetMoreInfo(unsigned short nAge, char nGender, const std::string& homepage,
      unsigned short nBirthYear, char nBirthMonth, char nBirthDay, char nLanguage1,
      char nLanguage2, char nLanguage3);
  virtual unsigned short SubCommand()  { return m_nMetaCommand; }
protected:
  unsigned short m_nMetaCommand;

  unsigned short m_nAge;
  char m_nGender;
  std::string myHomepage;
  unsigned short m_nBirthYear;
  char m_nBirthMonth;
  char m_nBirthDay;
  char m_nLanguage1;
  char m_nLanguage2;
  char m_nLanguage3;

  friend class IcqProtocol;
};

//-----Meta_SetInterestsInfo----------------------------------------------------
class CPU_Meta_SetInterestsInfo : public CPU_CommonFamily
{
public:
  CPU_Meta_SetInterestsInfo(const Licq::UserCategoryMap& interests);
  virtual unsigned short SubCommand()  { return m_nMetaCommand; }
protected:
  unsigned short m_nMetaCommand;

  Licq::UserCategoryMap myInterests;

  friend class IcqProtocol;
};


//-----Meta_SetOrgBackInfo------------------------------------------------------
class CPU_Meta_SetOrgBackInfo : public CPU_CommonFamily
{
public:
  CPU_Meta_SetOrgBackInfo(const Licq::UserCategoryMap& orgs,
      const Licq::UserCategoryMap& background);
  virtual unsigned short SubCommand()  { return m_nMetaCommand; }
protected:
  unsigned short m_nMetaCommand;

  Licq::UserCategoryMap myOrganizations;
  Licq::UserCategoryMap myBackgrounds;

  friend class IcqProtocol;
};

//-----Meta_SetWorkInfo------------------------------------------------------
class CPU_Meta_SetWorkInfo : public CPU_CommonFamily
{
public:
  CPU_Meta_SetWorkInfo(const std::string& city, const std::string& state,
      const std::string& phoneNumber, const std::string& faxNumber,
      const std::string& address, const std::string& zip,
      unsigned short nCompanyCountry, const std::string& name,
      const std::string& department, const std::string& position,
      unsigned short nCompanyOccupation, const std::string& homepage);
  virtual unsigned short SubCommand()  { return m_nMetaCommand; }
protected:
  unsigned short m_nMetaCommand;

  std::string myCity;
  std::string myState;
  std::string myPhoneNumber;
  std::string myFaxNumber;
  std::string myAddress;
  std::string myZip;
  unsigned short m_nCompanyCountry;
  std::string myName;
  std::string myDepartment;
  std::string myPosition;
  unsigned short m_nCompanyOccupation;
  std::string myHomepage;

  friend class IcqProtocol;
};


//-----Meta_SetAbout---------------------------------------------------------
class CPU_Meta_SetAbout : public CPU_CommonFamily
{
public:
  CPU_Meta_SetAbout(const std::string& about);
  virtual unsigned short SubCommand()  { return m_nMetaCommand; }
protected:
  unsigned short m_nMetaCommand;

  std::string myAbout;

  friend class IcqProtocol;
};

//-----RequestInfo-----------------------------------------------------------
class CPU_RequestInfo : public CPU_CommonFamily
{
public:
  CPU_RequestInfo(const std::string& accountId);
};

//-----RequestBuddyIcon------------------------------------------------------
class CPU_RequestBuddyIcon : public CPU_CommonFamily
{
public:
  CPU_RequestBuddyIcon(const std::string& accountId, unsigned short _nBuddyIconType,
      char _nBuddyIconHashType, const std::string& buddyIconHash, unsigned short nService);
};

class CPU_RequestService : public CPU_CommonFamily
{
public:
  CPU_RequestService(unsigned short nFam);
};

//-----AIMFetchAwayMessage--------------------------------------------------
class CPU_AIMFetchAwayMessage : public CPU_CommonFamily
{
public:
  CPU_AIMFetchAwayMessage(const std::string& accountId);
};

//-----SetPassword---------------------------------------------------------
class CPU_SetPassword : public CPU_CommonFamily
{
public:
  CPU_SetPassword(const std::string& password);
  virtual unsigned short SubCommand()  { return m_nMetaCommand; }
  unsigned long Uin()           { return 0; }
protected:
  unsigned short m_nMetaCommand;
  std::string myPassword;

  friend class IcqProtocol;
};


//-----Meta_SetSecurityInfo--------------------------------------------------
class CPU_Meta_SetSecurityInfo : public CPU_CommonFamily
{
public:
  CPU_Meta_SetSecurityInfo(bool bAuthorization,
                           bool bWebAware);
  virtual unsigned short SubCommand()  { return m_nMetaCommand; }

  bool Authorization()  { return m_nAuthorization == 0; }
  bool WebAware()       { return m_nWebAware == 1; }
protected:
  unsigned short m_nMetaCommand;
  char m_nAuthorization;
  char m_nWebAware;
};


//-----Meta_RequestAllInfo------------------------------------------------------
class CPU_Meta_RequestAllInfo : public CPU_CommonFamily
{
public:
  CPU_Meta_RequestAllInfo(const Licq::UserId& userId);
  virtual unsigned short SubCommand()  { return m_nMetaCommand; }
protected:
  unsigned short m_nMetaCommand;
};


//-----Meta_RequestBasicInfo------------------------------------------------------
class CPU_Meta_RequestBasicInfo : public CPU_CommonFamily
{
public:
  CPU_Meta_RequestBasicInfo(const std::string& accountId);
  virtual unsigned short SubCommand()  { return m_nMetaCommand; }
  const std::string& accountId() const {  return myAccountId; }
protected:
  unsigned short m_nMetaCommand;
  std::string myAccountId;
};

} // namespace LicqIcq

#endif
