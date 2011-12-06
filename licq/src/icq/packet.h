/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2011 Licq developers
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

#ifndef LICQDAEMON_ICQ_PACKET_H
#define LICQDAEMON_ICQ_PACKET_H

#include <cstring>
#include <list>
#include <map>
#include <pthread.h>
#include <string>

#include <licq/contactlist/user.h>
#include <licq/icq/chat.h> // ChatClientList
#include <licq/userid.h>
#include <licq/packet.h>

#include "defines.h"

namespace Licq
{
typedef std::list<std::string> StringList;
typedef std::map<unsigned int, std::string> UserCategoryMap;
class Buffer;
class Color;
class INetSocket;
}

typedef Licq::Buffer CBuffer;
typedef Licq::Packet CPacket;

typedef std::map<int, std::string> GroupNameMap;

// values of extra info to identify plugin request
const unsigned short DirectInfoPluginRequest     = 1;
const unsigned short DirectStatusPluginRequest   = 2;
const unsigned short ServerInfoPluginRequest     = 3;
const unsigned short ServerStatusPluginRequest   = 4;

const unsigned short GUID_LENGTH                 = 18;
const unsigned short CAP_LENGTH                  = 16;

// list of plugins we currently support
struct PluginList
{
  const char* const name;
  const char* const guid;
  const char* const description;
};

extern const struct PluginList info_plugins[];
extern const struct PluginList status_plugins[];

unsigned short ReversePort(unsigned short p);
size_t lengthField(const std::string& field);
std::string pipeInput(const std::string& message);


//
// These classes, CPX_*, are general classes for different packets that do the
// same function that may be sent through the server or directly to the client.
// This gives the direct and server packets a multiple inheritence.
//

//-----FileTransfer------------------------------------------------------------
class CPX_FileTransfer
{
public:
  CPX_FileTransfer(const std::list<std::string>& lFileList, const std::string& fileName);
  virtual ~CPX_FileTransfer();

  bool IsValid()                { return m_bValid; }
  const std::list<std::string>& GetFileList() const { return m_lFileList; }
  const std::string& filename() const { return myFilename; }
  const std::string& description() const { return myDesc; }
  unsigned long GetFileSize()   { return m_nFileSize; }

protected:
  CPX_FileTransfer();

  bool          m_bValid;
  std::string myDesc;
  std::string myFilename;
  std::list<std::string> m_lFileList;
  unsigned long m_nFileSize;
};



//=====ServerTCP===============================================================

//-----SrvPacketTcp------------------------------------------------------------

class CSrvPacketTcp : public CPacket
{
public:
  virtual ~CSrvPacketTcp();

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
};

//=====UDP======================================================================

//-----PacketUdp----------------------------------------------------------------
class CPacketUdp : public CPacket
{
public:
   virtual ~CPacketUdp();

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
  CPU_AddToServerList(const std::string& name, unsigned short _nType,
                      unsigned short _nGroup = 0,
                      bool _bAuthReq = false, bool _bTopLevel = false);

  unsigned short GetSID()   { return m_nSID; }
  unsigned short GetGSID()  { return m_nGSID; }

protected:
  unsigned short m_nSID,
                 m_nGSID;
};

//-----RemoveFromServerList-----------------------------------------------------
class CPU_RemoveFromServerList : public CPU_CommonFamily
{
public:
  CPU_RemoveFromServerList(const std::string& name, unsigned short _nGSID,
                           unsigned short _nSID, unsigned short _nType);
};

//-----ClearServerList----------------------------------------------------------
class CPU_ClearServerList : public CPU_CommonFamily
{
public:
  CPU_ClearServerList(const Licq::StringList& users, unsigned short);
};

//-----UpdateToServerList-------------------------------------------------------
class CPU_UpdateToServerList : public CPU_CommonFamily
{
public:
  CPU_UpdateToServerList(const std::string& name, unsigned short _nType,
                         unsigned short _nSID = 0, bool _bAuthReq = false);
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
  CPU_UpdateInfoTimestamp(const char *GUID);
};

class CPU_UpdateStatusTimestamp : public CPU_SetStatusFamily
{
public:
  CPU_UpdateStatusTimestamp(const char *GUID, unsigned long nState,
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
  CPU_Type2Message(const Licq::User* u, bool _bAck, bool _bDirectInfo, const char* cap,
                   unsigned long nMsgID1 = 0,
                   unsigned long nMsgID2 = 0);
protected:
  void InitBuffer();

  const Licq::User* m_pUser;
  bool m_bAck;
  bool m_bDirectInfo;
  unsigned long m_nMsgID[2];
  char m_cap[CAP_LENGTH];
  unsigned long m_nExtraLen; // length of data following 0x2711 tlv
};

//-----ReverseConnect-----------------------------------------------------------
class CPU_ReverseConnect : public CPU_Type2Message
{
public:
  CPU_ReverseConnect(const Licq::User* u, unsigned long nLocalIP,
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
  CPU_PluginMessage(const Licq::User* u, bool bAck, const char *PluginGUID,
                    unsigned long nMsgID1 = 0, unsigned long nMsgID2 = 0);

protected:
  void InitBuffer();

  char m_PluginGUID[GUID_LENGTH];
};

//-----InfoPluginRequest-------------------------------------------------------
class CPU_InfoPluginReq : public CPU_PluginMessage
{
public:
  CPU_InfoPluginReq(const Licq::User* u, const char *GUID, unsigned long nTime);
  virtual const char *RequestGUID() { return m_ReqGUID; }
  virtual unsigned short ExtraInfo() { return ServerInfoPluginRequest; }

protected:
  char m_ReqGUID[GUID_LENGTH];
};

//-----StatusPluginRequest-----------------------------------------------------
class CPU_StatusPluginReq : public CPU_PluginMessage
{
public:
  CPU_StatusPluginReq(const Licq::User* u, const char *GUID, unsigned long nTime);
  virtual unsigned short ExtraInfo() { return ServerStatusPluginRequest; }
  virtual const char *RequestGUID() { return m_ReqGUID; }

protected:
  char m_ReqGUID[GUID_LENGTH];
};

//-----AdvancedMessage---------------------------------------------------------
class CPU_AdvancedMessage : public CPU_Type2Message
{
public:
  CPU_AdvancedMessage(const Licq::User* u, unsigned short _nMsgType,
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
      unsigned short nLevel, const Licq::User* pUser, bool bICBM);
};

//-----FileTransfer------------------------------------------------------------
class CPU_FileTransfer : public CPU_AdvancedMessage, public CPX_FileTransfer
{
public:
  CPU_FileTransfer(const Licq::User* u, const std::list<std::string>& lFileList,
      const std::string& file, const std::string& desc, unsigned short nLevel, bool bICBM);
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
                         unsigned short nLevel, const char *GUID);
protected:
  void InitBuffer();

  unsigned long m_nMsgID[2];
  unsigned short m_nSequence, m_nMsgType, m_nStatus, m_nLevel;
  std::string myAccountId;
  std::string myMessage;
  char m_GUID[GUID_LENGTH];
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
  CPU_AckFileAccept(const Licq::User* u, const unsigned long nMsgID[],
      unsigned short nSequence, unsigned short nPort, const std::string& desc,
      const std::string& file, unsigned long nFileSize);
};

//-----AckFileRefuse-----------------------------------------------------------
class CPU_AckFileRefuse : public CPU_AckThroughServer
{
public:
  CPU_AckFileRefuse(const Licq::User* u, const unsigned long nMsgID[],
      unsigned short nSequence, const std::string& message);
};

//-----AckChatAccept-----------------------------------------------------------
class CPU_AckChatAccept : public CPU_AdvancedMessage
{
public:
  CPU_AckChatAccept(const Licq::User* u, const std::string& clients,
      const unsigned long nMsgID[], unsigned short nSequence, unsigned short nPort);
};

//-----AckChatRefuse-----------------------------------------------------------
class CPU_AckChatRefuse : public CPU_AckThroughServer
{
public:
  CPU_AckChatRefuse(const Licq::User* u, const unsigned long nMsgID[],
      unsigned short nSequence, const std::string& message);
};

//-----PluginError-------------------------------------------------------------
class CPU_PluginError : public CPU_AckThroughServer
{
public:
  CPU_PluginError(const Licq::User* u, unsigned long nMsgID1, unsigned long nMsgID2,
                  unsigned short nSequence, const char *GUID);
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
                           bool bHideIp,
                           bool bWebAware);
  virtual unsigned short SubCommand()  { return m_nMetaCommand; }

  bool Authorization()  { return m_nAuthorization == 0; }
  bool HideIp()         { return m_nHideIp == 1; }
  bool WebAware()       { return m_nWebAware == 1; }
protected:
  unsigned short m_nMetaCommand;
  char m_nAuthorization;
  char m_nHideIp;
  char m_nWebAware;
};


//-----Meta_RequestAllInfo------------------------------------------------------
class CPU_Meta_RequestAllInfo : public CPU_CommonFamily
{
public:
  CPU_Meta_RequestAllInfo(const std::string& accountId);
  virtual unsigned short SubCommand()  { return m_nMetaCommand; }
  const std::string& accountId() const {  return myAccountId; }
protected:
  unsigned short m_nMetaCommand;
  std::string myAccountId;
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


//=====TCP======================================================================
bool Decrypt_Client(CBuffer *pkt, unsigned long version);
void Encrypt_Client(CBuffer *pkt, unsigned long version);

//-----PacketTcp_Handshake------------------------------------------------------
class CPacketTcp_Handshake : public CPacket
{
public:
  virtual unsigned short Sequence()   { return 0; }
  virtual unsigned short SubSequence()   { return 0; }
  virtual unsigned short SubCommand() { return 0; }
};

//-----PacketTcp_Handshake------------------------------------------------------
/* FF 03 00 00 00 3D 62 00 00 50 A5 82 00 CF 60 AD 95 CF 60 AD 95 04 3D 62
   00 00 */
class CPacketTcp_Handshake_v2 : public CPacketTcp_Handshake
{
public:
  CPacketTcp_Handshake_v2(unsigned long _nLocalPort);
protected:
  unsigned long  m_nLocalPort;
  unsigned long  m_nLocalHost;
};


class CPacketTcp_Handshake_v4 : public CPacketTcp_Handshake
{
public:
  CPacketTcp_Handshake_v4(unsigned long _nLocalPort);
protected:
  unsigned long  m_nLocalPort;
  unsigned long  m_nLocalHost;
};


class CPacketTcp_Handshake_v6 : public CPacketTcp_Handshake
{
public:
  CPacketTcp_Handshake_v6(unsigned long nDestinationUin,
     unsigned long nSessionId, unsigned short nLocalPort = 0);
  CPacketTcp_Handshake_v6(CBuffer *);

  char Handshake() { return m_nHandshake; }
  unsigned short VersionMajor() { return m_nVersionMajor; }
  unsigned short VersionMinor() { return m_nVersionMinor; }
  unsigned long DestinationUin() { return m_nDestinationUin; }
  unsigned short LocalPort() { return m_nLocalPort; }
  unsigned long SourceUin() { return m_nSourceUin; }
  unsigned long LocalIp()  { return m_nLocalIp; }
  unsigned long RealIp()  { return m_nRealIp; }
  char Mode()  { return m_nMode; }
  unsigned long SessionId() { return m_nSessionId; }

protected:
  char m_nHandshake;
  unsigned short m_nVersionMajor;
  unsigned short m_nVersionMinor;
  unsigned long m_nDestinationUin;
  unsigned long m_nSourceUin;
  unsigned short m_nLocalPort;
  unsigned long m_nLocalIp;
  unsigned long m_nRealIp;
  char m_nMode;
  unsigned long m_nSessionId;
};

class CPacketTcp_Handshake_v7 : public CPacketTcp_Handshake
{
public:
  CPacketTcp_Handshake_v7(unsigned long nDestinationUin,
     unsigned long nSessionId, unsigned short nLocalPort = 0,
     unsigned long nId = 0);
  CPacketTcp_Handshake_v7(CBuffer *);

  char Handshake() { return m_nHandshake; }
  unsigned short VersionMajor() { return m_nVersionMajor; }
  unsigned short VersionMinor() { return m_nVersionMinor; }
  unsigned long DestinationUin() { return m_nDestinationUin; }
  unsigned short LocalPort() { return m_nLocalPort; }
  unsigned long SourceUin() { return m_nSourceUin; }
  unsigned long LocalIp()  { return m_nLocalIp; }
  unsigned long RealIp()  { return m_nRealIp; }
  char Mode()  { return m_nMode; }
  unsigned long SessionId() { return m_nSessionId; }
  unsigned long Id() { return m_nId; }

protected:
  char m_nHandshake;
  unsigned short m_nVersionMajor;
  unsigned short m_nVersionMinor;
  unsigned long m_nDestinationUin;
  unsigned long m_nSourceUin;
  unsigned short m_nLocalPort;
  unsigned long m_nLocalIp;
  unsigned long m_nRealIp;
  char m_nMode;
  unsigned long m_nSessionId;
  unsigned long m_nId;
};


class CPacketTcp_Handshake_Ack : public CPacketTcp_Handshake
{
public:
  CPacketTcp_Handshake_Ack();
};

class CPacketTcp_Handshake_Confirm : public CPacketTcp_Handshake
{
public:
  CPacketTcp_Handshake_Confirm(int channel, unsigned short nSequence);
  CPacketTcp_Handshake_Confirm(CBuffer *inbuf);

  int channel() const { return myChannel; }
  unsigned long Id() { return m_nId; }
protected:
  int myChannel;
  unsigned long m_nId;
};


//-----CPacketTcp---------------------------------------------------------------
class CPacketTcp : public CPacket
{
public:
   virtual ~CPacketTcp();

   virtual CBuffer *Finalize(Licq::INetSocket*);
  int channel() const { return myChannel; }
   virtual unsigned short Sequence()   { return m_nSequence; }
   virtual unsigned short SubSequence()   { return 0; }
   virtual unsigned short SubCommand() { return m_nSubCommand; }

   char *LocalPortOffset()  {  return m_szLocalPortOffset; }
   unsigned short Level()  { return m_nLevel; }
   unsigned short Version()  { return m_nVersion; }
protected:
  CPacketTcp(unsigned long _nCommand, unsigned short _nSubCommand, int channel,
      const std::string& message, bool _bAccept, unsigned short nLevel,
      Licq::User* _cUser);
   void InitBuffer();
   void PostBuffer();
   void InitBuffer_v2();
   void PostBuffer_v2();
   void InitBuffer_v4();
   void PostBuffer_v4();
   void InitBuffer_v6();
   void PostBuffer_v6();
   void InitBuffer_v7();
   void PostBuffer_v7();

   unsigned long  m_nSourceUin;
   unsigned long  m_nCommand;
   unsigned short m_nSubCommand;
  int myChannel;
   std::string myMessage;
   unsigned long  m_nLocalPort;
   unsigned short m_nStatus;
   unsigned short m_nMsgType;
   unsigned short m_nSequence;
   bool           m_bPluginReq;
   size_t         m_nMsgLen;

   char *m_szLocalPortOffset;
   unsigned short m_nLevel;
   unsigned short m_nVersion;
};


//-----Message------------------------------------------------------------------
class CPT_Message : public CPacketTcp
{
public:
   CPT_Message(const std::string& message, unsigned short nLevel, bool bMR,
      const Licq::Color* pColor, Licq::User* pUser, bool isUtf8);
};


//-----Url----------------------------------------------------------------------
/* BA 95 47 00 03 00 EE 07 00 00 BA 95 47 00 04 00 24 00 67 6F 6F 64 20 70 6F
   72 6E 20 73 69 74 65 FE 68 74 74 70 3A 2F 2F 63 6F 6F 6C 70 6F 72 74 6E 2E
   63 6F 6D 00 81 61 1D 9E 7F 00 00 01 3F 07 00 00 04 00 00 10 00 03 00 00 00 */
class CPT_Url : public CPacketTcp
{
public:
  CPT_Url(const std::string& message, unsigned short nLevel, bool bMR,
      const Licq::Color* pColor, Licq::User *pUser);
};


class CPT_ContactList : public CPacketTcp
{
public:
  CPT_ContactList(const std::string& message, unsigned short nLevel, bool bMR,
      const Licq::Color* pColor, Licq::User* pUser);
};




//-----ReadAwayMessage----------------------------------------------------------
class CPT_ReadAwayMessage : public CPacketTcp
{
public:
  CPT_ReadAwayMessage(Licq::User* _cUser);
   /* 76 1E 3F 00 03 00 EE 07 00 00 76 1E 3F 00 E8 03 01 00 00 81 61 1D 9D 81 61
      1D 9D C9 05 00 00 04 00 00 10 00 FE FF FF FF */
};


//-----ChatRequest--------------------------------------------------------------
  /* 50 A5 82 00 03 00 EE 07 00 00 50 A5 82 00 02 00 0D 00 63 68 61 74 20 72
     65 71 75 65 73 74 00 CF 60 AD D3 CF 60 AD D3 28 12 00 00 04 00 00 10 00
     01 00 00 00 00 00 00 00 00 00 00 06 00 00 00 */
class CPT_ChatRequest : public CPacketTcp
{
public:
  CPT_ChatRequest(const std::string& message, const std::string& chatUsers, unsigned short nPort,
      unsigned short nLevel, Licq::User* pUser, bool bICBM);
};


//-----FileTransfer-------------------------------------------------------------
class CPT_FileTransfer : public CPacketTcp, public CPX_FileTransfer
{
public:
  CPT_FileTransfer(const std::list<std::string>& lFileList, const std::string& filename,
      const std::string& description, unsigned short nLevel, Licq::User* pUser);
  const std::string& description() { return myMessage; }
protected:
   /* 50 A5 82 00 03 00 EE 07 00 00 50 A5 82 00 03 00 0F 00 74 68 69 73 20 69
      73 20 61 20 66 69 6C 65 00 CF 60 AD D3 CF 60 AD D3 60 12 00 00 04 00 00
      10 00 00 00 00 00 09 00 4D 61 6B 65 66 69 6C 65 00 55 0C 00 00 00 00 00
      00 04 00 00 00 */
};


//-----OpenSecureChannel------------------------------------------------------------
class CPT_OpenSecureChannel : public CPacketTcp
{
public:
  CPT_OpenSecureChannel(Licq::User* pUser);
};


class CPT_CloseSecureChannel : public CPacketTcp
{
public:
  CPT_CloseSecureChannel(Licq::User* pUser);
};




//++++Ack++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/* 50 A5 82 00 03 00 DA 07 00 00 50 A5 82 00 01 00 01 00 00 CF 60 AD D3 CF
   60 AD D3 60 12 00 00 04 00 00 00 00 02 00 00 00 */
class CPT_Ack : public CPacketTcp
{
protected:
  CPT_Ack(unsigned short _nSubCommand, unsigned short _nSequence,
      bool _bAccept, bool _bUrgent, Licq::User* _cUser);
  virtual ~CPT_Ack();
};


//-----AckGeneral------------------------------------------------------------
class CPT_AckGeneral : public CPT_Ack
{
public:
  CPT_AckGeneral(unsigned short nSubCommand, unsigned short nSequence,
      bool bAccept, bool bUrgent, Licq::User* pUser);
};


//-----AckKey------------------------------------------------------------
class CPT_AckOldSecureChannel : public CPT_Ack
{
public:
  CPT_AckOldSecureChannel(unsigned short nSequence, Licq::User* pUser);
};


//-----AckKey------------------------------------------------------------
class CPT_AckOpenSecureChannel : public CPT_Ack
{
public:
  CPT_AckOpenSecureChannel(unsigned short nSequence, bool ok, Licq::User* pUser);
};


//-----AckKey------------------------------------------------------------
class CPT_AckCloseSecureChannel : public CPT_Ack
{
public:
  CPT_AckCloseSecureChannel(unsigned short nSequence, Licq::User* pUser);
};


#if 0
//-----AckMessage------------------------------------------------------------
class CPT_AckMessage : public CPT_Ack
{
public:
  CPT_AckMessage(unsigned short _nSequence, bool _bAccept, bool _bUrgent, Licq::User* _cUser);
};


//-----AckReadAwayMessage-------------------------------------------------------
class CPT_AckReadAwayMessage : public CPT_Ack
{
public:
   CPT_AckReadAwayMessage(unsigned short _nSubCommand, unsigned short _nSequence,
      bool _bAccept, Licq::User* _cUser);
};


//-----AckUrl-------------------------------------------------------------------
class CPT_AckUrl : public CPT_Ack
{
public:
  CPT_AckUrl(unsigned short _nSequence, bool _bAccept, bool _bUrgent, Licq::User* _cUser);
};


//-----AckContactList----------------------------------------------------------
class CPT_AckContactList : public CPT_Ack
{
public:
  CPT_AckContactList(unsigned short _nSequence, bool _bAccept, bool _bUrgent,
      Licq::User* _cUser);
};
#endif

//-----AckChatRefuse------------------------------------------------------------
class CPT_AckChatRefuse : public CPT_Ack
{
public:
  CPT_AckChatRefuse(const std::string& reason, unsigned short _nSequence, Licq::User* _cUser);
   /* 50 A5 82 00 03 00 DA 07 00 00 50 A5 82 00 02 00 03 00 6E 6F 00 CF 60 AD
      95 CF 60 AD 95 1E 3C 00 00 04 01 00 00 00 01 00 00 00 00 00 00 00 00 00
      00 01 00 00 00 */
};


//-----AckChatAccept------------------------------------------------------------
class CPT_AckChatAccept : public CPT_Ack
{
public:
  CPT_AckChatAccept(unsigned short _nPort, const std::string& clients,
      unsigned short _nSequence, Licq::User* _cUser, bool bICBM);
   /* 50 A5 82 00 03 00 DA 07 00 00 50 A5 82 00 02 00 01 00 00 CF 60 AD 95 CF
      60 AD 95 1E 3C 00 00 04 00 00 00 00 01 00 00 40 78 00 00 78 40 00 00 02
      00 00 00 */
   unsigned long m_nPort;
};



//-----AckFileAccept------------------------------------------------------------
class CPT_AckFileAccept : public CPT_Ack
{
public:
  CPT_AckFileAccept(unsigned short _nPort, unsigned short _nSequence,
      Licq::User* _cUser);
protected:
   /* 50 A5 82 00 03 00 DA 07 00 00 50 A5 82 00 03 00 01 00 00 D1 EF 04 9F 7F
      00 00 01 4A 1F 00 00 04 00 00 00 00 20 3A 00 00 01 00 00 00 00 00 00 3A
      20 00 00 05 00 00 00 */

   unsigned long  m_nFileSize;  // not used in the ack
   unsigned long  m_nPort;

};


//-----AckFileRefuse------------------------------------------------------------
class CPT_AckFileRefuse : public CPT_Ack
{
public:
  CPT_AckFileRefuse(const std::string& reason, unsigned short _nSequence, Licq::User* _cUser);
protected:
   /* 50 A5 82 00 03 00 DA 07 00 00 50 A5 82 00 03 00 0A 00 6E 6F 20 74 68 61
      6E 6B 73 00 D1 EF 04 9F 7F 00 00 01 4A 1F 00 00 04 01 00 00 00 00 00 00
      00 01 00 00 00 00 00 00 00 00 00 00 03 00 00 00 */
};


//++++Cancel++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CPT_Cancel : public CPacketTcp
{
protected:
   CPT_Cancel(unsigned short _nSubCommand, unsigned short _nSequence,
      Licq::User* _cUser);
};


//-----ChatCancel---------------------------------------------------------------
class CPT_CancelChat : public CPT_Cancel
{
public:
  CPT_CancelChat(unsigned short _nSequence, Licq::User* _cUser);
   /* 50 A5 82 00 03 00 D0 07 00 00 50 A5 82 00 02 00 01 00 00 CF 60 AD D3 CF
      60 AD D3 28 12 00 00 04 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 06
      00 00 00 */
};


//-----FileCancel---------------------------------------------------------------
class CPT_CancelFile : public CPT_Cancel
{
public:
  CPT_CancelFile(unsigned short _nSequence, Licq::User* _cUser);
   /* 50 A5 82 00 03 00 D0 07 00 00 50 A5 82 00 02 00 01 00 00 CF 60 AD D3 CF
      60 AD D3 28 12 00 00 04 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 06
      00 00 00 */
};

//----Send error reply----------------------------------------------------------
class CPT_PluginError : public CPacketTcp
{
public:
  CPT_PluginError(Licq::User* _cUser, unsigned short nSequence,
      int channel);
};

//----Request info plugin list--------------------------------------------------
class CPT_InfoPluginReq : public CPacketTcp
{
public:
  CPT_InfoPluginReq(Licq::User* _cUser, const char* GUID, unsigned long int nTime);
   virtual const char *RequestGUID()        { return m_ReqGUID; }
   virtual unsigned short ExtraInfo() { return DirectInfoPluginRequest; }

protected:
   char m_ReqGUID[GUID_LENGTH];
};

//----Response to phone book request--------------------------------------------
class CPT_InfoPhoneBookResp : public CPacketTcp
{
public:
  CPT_InfoPhoneBookResp(Licq::User* _cUser, unsigned short nSequence);
};

//-----Response to picture request----------------------------------------------
class CPT_InfoPictureResp : public CPacketTcp
{
public:
  CPT_InfoPictureResp(Licq::User* _cUser, unsigned short nSequence);
};

//----Response to info plugin list request--------------------------------------
class CPT_InfoPluginListResp : public CPacketTcp
{
public:
  CPT_InfoPluginListResp(Licq::User* _cUser, unsigned short nSequence);
};

//----Send status plugin request------------------------------------------------
class CPT_StatusPluginReq : public CPacketTcp
{
public:
  CPT_StatusPluginReq(Licq::User* _cUser, const char* GUID, unsigned long nTime);
   virtual unsigned short ExtraInfo() { return DirectStatusPluginRequest;}
   virtual const char *RequestGUID() { return m_ReqGUID; }

protected:
  char m_ReqGUID[GUID_LENGTH];
};

//----Response to status plugin list request------------------------------------
class CPT_StatusPluginListResp : public CPacketTcp
{
public:
  CPT_StatusPluginListResp(Licq::User* _cUser, unsigned short nSequence);
};

//----Response to status request------------------------------------------------
class CPT_StatusPluginResp : public CPacketTcp
{
public:
  CPT_StatusPluginResp(Licq::User* _cUser, unsigned short nSequence,
                       unsigned long nStatus);
};


//=====File=====================================================================
class CPacketFile : public Licq::Packet
{
public:
  CPacketFile();
  virtual ~CPacketFile();

  virtual unsigned short Sequence()    { return 0; };
  virtual unsigned short SubSequence() { return 0; };
  virtual unsigned short SubCommand()  { return 0; };
protected:
   void InitBuffer()   { buffer = new CBuffer(m_nSize); };
};

//-----File_InitClient----------------------------------------------------------
/* 00 00 00 00 00 01 00 00 00 45 78 00 00 64 00 00 00 08 00 38 35 36 32 30
   30 30 00 */
class CPFile_InitClient : public CPacketFile
{
public:
  CPFile_InitClient(const std::string& localName, unsigned long _nNumFiles,
                    unsigned long _nTotalSize);
  virtual ~CPFile_InitClient();
};

//-----File_InitServer----------------------------------------------------------
/* 01 64 00 00 00 08 00 38 35 36 32 30 30 30 00 */
class CPFile_InitServer : public CPacketFile
{
public:
  CPFile_InitServer(const std::string& localName);
  virtual ~CPFile_InitServer();
};

//-----File_Info---------------------------------------------------------------
/* 02 00 0D 00 63 75 72 72 65 6E 74 2E 64 69 66 66 00 01 00 00 45 78 00 00
   00 00 00 00 64 00 00 00 */
class CPFile_Info : public CPacketFile
{
public:
  CPFile_Info(const std::string& fileName);
  virtual ~CPFile_Info();

  bool IsValid()  { return m_bValid; };
  unsigned long GetFileSize()
    { return m_nFileSize; };
  const std::string& fileName() const
    { return myFileName; }
  const char *ErrorStr()
    { return strerror(m_nError); }
protected:
  bool m_bValid;
  int m_nError;
  std::string myFileName;
  unsigned long m_nFileSize;
};

//-----File_Start---------------------------------------------------------------
/* 03 00 00 00 00 00 00 00 00 64 00 00 00 */
class CPFile_Start : public CPacketFile
{
public:
  CPFile_Start(unsigned long nFilePos, unsigned long nFile);
  virtual ~CPFile_Start();
};

//-----File_SetSpeed---------------------------------------------------------------
/* 03 00 00 00 00 00 00 00 00 64 00 00 00 */
class CPFile_SetSpeed : public CPacketFile
{
public:
  CPFile_SetSpeed(unsigned long nSpeed);
  virtual ~CPFile_SetSpeed();
};


//=====Chat=====================================================================
class CPacketChat : public Licq::Packet
{
public:
  virtual unsigned short Sequence()   { return 0; };
  virtual unsigned short SubSequence()   { return 0; };
  virtual unsigned short SubCommand() { return 0; };
protected:
   void InitBuffer();
};


//-----ChatColor----------------------------------------------------------------
  /* 64 00 00 00 FD FF FF FF 50 A5 82 00 08 00 38 35 36 32 30 30 30 00 62 3D
     FF FF FF 00 00 00 00 00 00 */
class CPChat_Color : public CPacketChat  // First info packet after handshake
{
public:
  CPChat_Color(const std::string& localName, unsigned short nLocalPort,
               int nColorForeRed, int nColorForeGreen, int nColorForeBlue,
               int nColorBackRed, int nColorBackBlue, int nColorBackGreen);
  CPChat_Color(CBuffer &);

  // Accessors
  const std::string& name() const { return myName; }
  const Licq::UserId& userId() const { return myUserId; }
  unsigned short Port() { return m_nPort; }
  int ColorForeRed() { return m_nColorForeRed; }
  int ColorForeGreen() { return m_nColorForeGreen; }
  int ColorForeBlue() { return m_nColorForeBlue; }
  int ColorBackRed() { return m_nColorBackRed; }
  int ColorBackGreen() { return m_nColorBackGreen; }
  int ColorBackBlue() { return m_nColorBackBlue; }

  virtual ~CPChat_Color();

protected:
  Licq::UserId myUserId;
  std::string myName;
  unsigned short m_nPort;
  int m_nColorForeRed;
  int m_nColorForeGreen;
  int m_nColorForeBlue;
  int m_nColorBackRed;
  int m_nColorBackGreen;
  int m_nColorBackBlue;
};


/* 64 00 00 00 50 A5 82 00 08 00 38 35 36 32 30 30 30 00 FF FF FF 00 00 00
   00 00 03 00 00 00 DB 64 00 00 CF 60 AD 95 CF 60 AD 95 04 75 5A 0C 00 00
   00 00 00 00 00 08 00 43 6F 75 72 69 65 72 00 00 00 00 */
class CPChat_ColorFont : public CPacketChat  // Second info packet after handshake
{
public:
  CPChat_ColorFont(const std::string& localName, unsigned short nLocalPort,
     unsigned short nSession,
     int nColorForeRed, int nColorForeGreen, int nColorForeBlue,
     int nColorBackRed, int nColorBackBlue, int nColorBackGreen,
     unsigned long nFontSize,
     bool bFontBold, bool bFontItalic, bool bFontUnderline, bool bFontStrikeOut,
      const std::string& fontFamily, unsigned char nFontEncoding,
     unsigned char nFontStyle, ChatClientPList &clientList);

  CPChat_ColorFont(CBuffer &);

  virtual ~CPChat_ColorFont();

  // Accessors
  const std::string& name() const { return myName; }
  const Licq::UserId& userId() const { return myUserId; }
  unsigned short Session() { return m_nSession; }
  int ColorForeRed() { return m_nColorForeRed; }
  int ColorForeGreen() { return m_nColorForeGreen; }
  int ColorForeBlue() { return m_nColorForeBlue; }
  int ColorBackRed() { return m_nColorBackRed; }
  int ColorBackGreen() { return m_nColorBackGreen; }
  int ColorBackBlue() { return m_nColorBackBlue; }
  unsigned short Port() { return m_nPort; }
  unsigned long FontSize() { return m_nFontSize; }
  bool FontBold() { return m_nFontFace & FONT_BOLD; }
  bool FontItalic() { return m_nFontFace & FONT_ITALIC; }
  bool FontUnderline() { return m_nFontFace & FONT_UNDERLINE; }
  bool FontStrikeOut() { return m_nFontFace & FONT_STRIKEOUT; }
  unsigned long FontFace() { return m_nFontFace; }
  const std::string& fontFamily() const { return myFontFamily; }
  unsigned char FontEncoding() { return m_nFontEncoding; }
  unsigned char FontStyle() { return m_nFontStyle; }
  ChatClientList &ChatClients()  { return chatClients; }

protected:
  Licq::UserId myUserId;
  unsigned short m_nSession;
  std::string myName;
  int m_nColorForeRed;
  int m_nColorForeGreen;
  int m_nColorForeBlue;
  int m_nColorBackRed;
  int m_nColorBackGreen;
  int m_nColorBackBlue;
  unsigned short m_nPort;
  unsigned long m_nFontSize;
  unsigned long m_nFontFace;
  std::string myFontFamily;
  unsigned char m_nFontEncoding, m_nFontStyle;
  ChatClientList chatClients;
};


//-----ChatFont-----------------------------------------------------------------
/* 03 00 00 00 83 72 00 00 CF 60 AD 95 CF 60 AD 95 04 54 72 0C 00 00 00 00
   00 00 00 08 00 43 6F 75 72 69 65 72 00 00 00 */
class CPChat_Font : public CPacketChat
{
public:
   CPChat_Font(unsigned short nLocalPort, unsigned short nSession,
               unsigned long nFontSize,
               bool bFontBold, bool bFontItalic, bool bFontUnderline,
      bool bFontStrikeOut, const std::string& fontFamily,
               unsigned char nFontEncoding, unsigned char nFontStyle);
   CPChat_Font(CBuffer &);
   virtual ~CPChat_Font();

  unsigned short Port() { return m_nPort; }
  unsigned short Session() { return m_nSession; }
  unsigned long FontSize() { return m_nFontSize; }
  bool FontBold() { return m_nFontFace & FONT_BOLD; }
  bool FontItalic() { return m_nFontFace & FONT_ITALIC; }
  bool FontUnderline() { return m_nFontFace & FONT_UNDERLINE; }
  bool FontStrikeOut() { return m_nFontFace & FONT_STRIKEOUT; }
  unsigned long FontFace() { return m_nFontFace; }
  const std::string& fontFamily() const { return myFontFamily; }
  unsigned char FontEncoding() { return m_nFontEncoding; }
  unsigned char FontStyle() { return m_nFontStyle; }

protected:
  unsigned short m_nPort;
  unsigned short m_nSession;
  unsigned long m_nFontSize;
  unsigned long m_nFontFace;
  std::string myFontFamily;
  unsigned char m_nFontEncoding, m_nFontStyle;
};


/*
class CPChat_ChangeFontFamily : public CPacketChat
{
public:
  CPChat_ChangeFontFamily(const std::string& fontFamily);
  CPChat_ChangeFontFamily(CBuffer &);
  virtual ~CPChat_ChangeFontFamily() { }

  const std::string& fontFamily() const { return myFontFamily; }

protected:
  std::string myFontFamily;
};


class CPChat_ChangeFontSize : public CPacketChat
{
public:
  CPChat_ChangeFontSize(unsigned short);
  CPChat_ChangeFontSize(CBuffer &);

  unsigned short FontSize()  { return m_nFontSize; }

protected:
  unsigned short m_nFontSize;
};



class CPChat_ChangeFontFace : public CPacketChat
{
public:
  CPChat_ChangeFontFace(bool bBold, bool bItalic, bool bUnderline);
  CPChat_ChangeFontFace(CBuffer &);

  bool FontBold() { return m_nFontFace & FONT_BOLD; }
  bool FontItalic() { return m_nFontFace & FONT_ITALIC; }
  bool FontUnderline() { return m_nFontFace & FONT_UNDERLINE; }
  unsigned long FontFace()  { return m_nFontFace; }

protected:
  unsigned long m_nFontFace;
};


class CPChat_ChangeColorBg : public CPacketChat
{
public:
  CPChat_ChangeColorBg(int nRed, int nGreen, int nBlue);
  CPChat_ChangeColorBg(CBuffer &);

  int ColorBackRed() { return m_nColorBackRed; }
  int ColorBackGreen() { return m_nColorBackGreen; }
  int ColorBackBlue() { return m_nColorBackBlue; }

protected:
  int m_nColorBackRed;
  int m_nColorBackGreen;
  int m_nColorBackBlue;
};


class CPChat_ChangeColorFg : public CPacketChat
{
public:
  CPChat_ChangeColorFg(int nRed, int nGreen, int nBlue);
  CPChat_ChangeColorFg(CBuffer &);

  int ColorForeRed() { return m_nColorForeRed; }
  int ColorForeGreen() { return m_nColorForeGreen; }
  int ColorForeBlue() { return m_nColorForeBlue; }

protected:
  int m_nColorForeRed;
  int m_nColorForeGreen;
  int m_nColorForeBlue;
};



class CPChat_Beep : public CPacketChat
{
public:
  CPChat_Beep();
};
*/

#endif
