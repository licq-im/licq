#ifndef ICQPACKET_H
#define ICQPACKET_H

#include <list>
#include <map>
#include <pthread.h>
#include <string>

#include "licq_icq.h"
#include "licq_types.h"

class CICQColor;
class CBuffer;
class INetSocket;

typedef std::list<std::string> StringList;
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
  const char *name;
  const char *guid;
  const char *description;
};

extern struct PluginList info_plugins[];
extern struct PluginList status_plugins[];

unsigned short ReversePort(unsigned short p);
unsigned short LengthField(const char *szField);
char *PipeInput(char *m_szMessage);

//=====Packet===================================================================

class CPacket
{
public:
   virtual ~CPacket();

   CBuffer *getBuffer()  { return buffer; };
   virtual CBuffer *Finalize(INetSocket *) { return NULL; }

   virtual unsigned short Sequence() = 0;
   virtual unsigned short SubSequence() = 0;
   virtual unsigned short Command() = 0;
   virtual unsigned short SubCommand() = 0;

   virtual unsigned char  Channel()   { return ICQ_CHNxNONE; }
   virtual unsigned long  SNAC()      { return 0; }
   virtual unsigned short ExtraInfo() { return 0; }


   static void SetMode(char c) { s_nMode = c; }
   static char Mode()  { return s_nMode; }
   static void SetLocalIp(unsigned long n)  {  s_nLocalIp = n; }
   static void SetLocalPort(unsigned short n)  {  s_nLocalPort = n; }
   static void SetRealIp(unsigned long n)  {  s_nRealIp = n; }
   static bool Firewall() { return s_nLocalIp != s_nRealIp; }
   static unsigned long RealIp() { return s_nRealIp; }
   static unsigned long LocalIp() { return s_nLocalIp; }

protected:
   CPacket();

   CBuffer *buffer;
   unsigned short m_nSize;

   static unsigned long s_nLocalIp;
   static unsigned long s_nRealIp;
   static unsigned short s_nLocalPort;
   static char s_nMode;
};

// Order of inclusion is significant here!
#include "licq_filetransfer.h"

//
// These classes, CPX_*, are general classes for different packets that do the
// same function that may be sent through the server or directly to the client.
// This gives the direct and server packets a multiple inheritence.
//

//-----FileTransfer------------------------------------------------------------
class CPX_FileTransfer
{
public:
  CPX_FileTransfer(ConstFileList &lFileList, const char *szFileName);
  virtual ~CPX_FileTransfer();

  bool IsValid()                { return m_bValid; }
  ConstFileList GetFileList()   { return m_lFileList; }
  const char *GetFilename()     { return m_szFilename; }
  const char *GetDescription() { return m_szDesc; }
  unsigned long GetFileSize()   { return m_nFileSize; }

protected:
  CPX_FileTransfer();

  bool          m_bValid;
  char          *m_szDesc;
  char          *m_szFilename;
  ConstFileList m_lFileList;
  unsigned long m_nFileSize;
};



//=====ServerTCP===============================================================

//-----SrvPacketTcp------------------------------------------------------------

class CSrvPacketTcp : public CPacket
{
public:
  virtual ~CSrvPacketTcp();

  // Packet details
  virtual unsigned char  Channel()     { return m_nChannel; }
  virtual unsigned short Sequence()    { return m_nSequence; }
  virtual unsigned short SubSequence() { return m_nSubSequence; }
  virtual unsigned long  SNAC() { return ((m_nFamily << 16) | (m_nSubType)); }
  virtual unsigned short SubCommand()  { return m_nSubCommand; }

  // Not used anymore here, use SNAC instead.
  virtual unsigned short Command()     { return 0; }

  // Misc.
  virtual unsigned short ExtraInfo() { return m_nExtraInfo; }
  virtual CBuffer *Finalize(INetSocket *);
  void SetExtraInfo(unsigned short e)  { m_nExtraInfo = e; }

protected:
  CSrvPacketTcp(unsigned char);
  void InitBuffer();

  static bool s_bRegistered;
  static unsigned short s_nSequence[32];
  static unsigned short s_nSubSequence;
  static pthread_mutex_t s_xMutex;

  unsigned char  m_nChannel;
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

   virtual CBuffer *Finalize(INetSocket *);
   virtual unsigned short Sequence() { return m_nSequence; }
   virtual unsigned short SubSequence() { return m_nSubSequence; }
   virtual unsigned short Command()  { return m_nCommand; }
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
  CPU_Logon(const char *_szPassword, const char *_szUin, unsigned short _nLogonStatus);
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
  CPU_SendCookie(const char *, int len, unsigned short nService = 0);
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
  CPU_NewLogon(const char *_szPassword, const char *_szUin, const char *_szMD5Salt);
};

#if ICQ_VERSION == 2 || ICQ_VERSION == 6
//-----Register----------------------------------------------------------------
// Doesn't actually descend from CPacketUdp in version 2 but we keep the
// name the same for simplicity
class CPU_Register : public CPacketUdp
{
public:
  CPU_Register(const char *_szPasswd);
  virtual ~CPU_Register();

  virtual unsigned short Sequence() { return m_nSequence; }
  virtual unsigned short SubSequence() { return 0; }
  virtual unsigned short Command()  { return m_nCommand; }
  virtual unsigned short SubCommand() { return 0; }
protected:
  virtual unsigned long getSize()       { return 1; }

  /* 02 00 FC 03 01 00 02 00 04 00 65 66 67 00 72 00 00 00 00 00 00 00 */
  unsigned short m_nVersion;
  unsigned short m_nCommand;
  unsigned short m_nSequence;
  unsigned short m_nUnknown1;
  unsigned short m_nPasswdLen;
  char          *m_szPasswd;
  unsigned long  m_nUnknown2;
  unsigned long  m_nUnknown3;
};
#elif ICQ_VERSION == 4 || ICQ_VERSION == 5
//-----Register----------------------------------------------------------------
class CPU_Register : public CPacketUdp
{
public:
  CPU_Register(const char *_szPasswd);
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
  CPU_Register(const char *_szPasswd);
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
  CPU_SendVerification(const char *, const char *);
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
   CPU_GenericUinList(const char *szId, unsigned short Family, unsigned short Subtype);
   CPU_GenericUinList(const StringList& users, unsigned short, unsigned short);
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
  CPU_ExportToServerList(const std::list<UserId>& users, unsigned short type);
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
  CPU_AddToServerList(const char *_szName, unsigned short _nType,
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
  CPU_RemoveFromServerList(const char * _szName, unsigned short _nGSID,
                           unsigned short _nSID, unsigned short _nType);
};

//-----ClearServerList----------------------------------------------------------
class CPU_ClearServerList : public CPU_CommonFamily
{
public:
  CPU_ClearServerList(const StringList& users, unsigned short);
};

//-----UpdateToServerList-------------------------------------------------------
class CPU_UpdateToServerList : public CPU_CommonFamily
{
public:
  CPU_UpdateToServerList(const char *_szName, unsigned short _nType,
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
  CPU_TypingNotification(const char *szId, bool bActive);
};

//-----CheckInvisible----------------------------------------------------------
class CPU_CheckInvisible : public CPU_CommonFamily
{
public:
  CPU_CheckInvisible(const char *szId);
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
   CPU_SearchWhitePages(const char *szFirstName, const char *szLastName,
                   const char *szAlias, const char *szEmail,
                   unsigned short nMinAge, unsigned short nMaxAge,
                   char nGender, char nLanguage, const char *szCity,
                   const char *szState, unsigned short nCountryCode,
                   const char *szCoName, const char *szCoDept,
                   const char *szCoPos, const char *szKeyword, bool bOnlineOnly);
   virtual unsigned short SubCommand()   { return m_nMetaCommand; }
   unsigned long Uin()  { return 0; }
   void PackSearch(unsigned short nCmd, const char *szField);
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
   CPU_UpdatePersonalBasicInfo(const char *_sAlias, const char *_sFirstName,
                               const char *_sLastName, const char *_sEmail,
                               bool _bAuthorization);

   const char *Alias()  { return m_szAlias; }
   const char *FirstName()  { return m_szFirstName; }
   const char *LastName()  { return m_szLastName; }
   const char *Email()  { return m_szEmail; }
   bool Authorization()  { return m_nAuthorization == 0; }
protected:
   char *m_szAlias;
   char *m_szFirstName;
   char *m_szLastName;
   char *m_szEmail;
   char m_nAuthorization;
};


//-----UpdatePersonalExtInfo-------------------------------------------------------
class CPU_UpdatePersonalExtInfo : public CPacketUdp
{
public:
   CPU_UpdatePersonalExtInfo(const char *_sCity, unsigned short _nCountry,
                             const char *_sState, unsigned short _nAge,
                             char _cSex, const char *_sPhone,
                             const char *_sHomepage, const char *_sAbout,
                             unsigned long _nZipcode);

   const char *City()  { return m_szCity; }
   unsigned short Country()  { return m_nCountry; }
   const char *State()  { return m_szState; }
   unsigned short Age()  { return m_nAge; }
   char Sex()  { return m_cSex; }
   const char *PhoneNumber()  { return m_szPhone; }
   const char *Homepage()  { return m_szHomepage; }
   const char *About()  { return m_szAbout; }
   unsigned long Zipcode() { return m_nZipcode; }
protected:
   char           *m_szCity;
   unsigned short m_nCountry;
   char           m_cTimezone;
   char           *m_szState;
   unsigned short m_nAge;
   char           m_cSex;
   char           *m_szPhone;
   char           *m_szHomepage;
   char           *m_szAbout;
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
  CPU_ThroughServer(const char *szId, unsigned char format, const char* _sMessage,
                    unsigned short _nCharset = 0, bool bOffline = true,
                    size_t _nLen = 0);
  CPU_ThroughServer(unsigned long _nDestinationUin, unsigned char format,
                    char *_sMessage);
protected:
  unsigned char  m_nMsgType;
};

//-----Type2Message-------------------------------------------------------------
class CPU_Type2Message : public CPU_CommonFamily
{
public:
  CPU_Type2Message(const LicqUser* u, bool _bAck, bool _bDirectInfo, const char* cap,
                   unsigned long nMsgID1 = 0,
                   unsigned long nMsgID2 = 0);
protected:
  void InitBuffer();

  const LicqUser* m_pUser;
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
  CPU_ReverseConnect(const LicqUser* u, unsigned long nLocalIP,
                     unsigned short nLocalPort, unsigned short nRemotePort);
};

//-----ReverseConnectFailed-----------------------------------------------------
class CPU_ReverseConnectFailed : public CPU_CommonFamily
{
public:
  CPU_ReverseConnectFailed(const char* id, unsigned long nMsgID1,
                           unsigned long nMsgID2, unsigned short nFailedPort,
                           unsigned short nOurPort, unsigned long nConnectID);
};

//-----PluginMessage-----------------------------------------------------------
class CPU_PluginMessage : public CPU_Type2Message
{
public:
  CPU_PluginMessage(const LicqUser* u, bool bAck, const char *PluginGUID,
                    unsigned long nMsgID1 = 0, unsigned long nMsgID2 = 0);

protected:
  void InitBuffer();

  char m_PluginGUID[GUID_LENGTH];
};

//-----InfoPluginRequest-------------------------------------------------------
class CPU_InfoPluginReq : public CPU_PluginMessage
{
public:
  CPU_InfoPluginReq(const LicqUser* u, const char *GUID, unsigned long nTime);
  virtual const char *RequestGUID() { return m_ReqGUID; }
  virtual unsigned short ExtraInfo() { return ServerInfoPluginRequest; }

protected:
  char m_ReqGUID[GUID_LENGTH];
};

//-----StatusPluginRequest-----------------------------------------------------
class CPU_StatusPluginReq : public CPU_PluginMessage
{
public:
  CPU_StatusPluginReq(const LicqUser* u, const char *GUID, unsigned long nTime);
  virtual unsigned short ExtraInfo() { return ServerStatusPluginRequest; }
  virtual const char *RequestGUID() { return m_ReqGUID; }

protected:
  char m_ReqGUID[GUID_LENGTH];
};

//-----AdvancedMessage---------------------------------------------------------
class CPU_AdvancedMessage : public CPU_Type2Message
{
public:
  CPU_AdvancedMessage(const LicqUser* u, unsigned short _nMsgType,
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
  CPU_ChatRequest(char *szReason, const char *szChatUsers, unsigned short nPort,
      unsigned short nLevel, const LicqUser* pUser, bool bICBM);
};

//-----FileTransfer------------------------------------------------------------
class CPU_FileTransfer : public CPU_AdvancedMessage, public CPX_FileTransfer
{
public:
  CPU_FileTransfer(const LicqUser* u, ConstFileList &lFileList, const char *_szFile,
                   const char *_szDesc, unsigned short nLevel, bool bICBM);
};

//-----NoManager--------------------------------------------------------
class CPU_NoManager : public CPU_CommonFamily
{
public:
  CPU_NoManager(const LicqUser* u, unsigned long nMsgID1, unsigned long nMsgID2);
};

//-----AckThroughServer--------------------------------------------------------
class CPU_AckThroughServer : public CPU_CommonFamily
{
public:
  CPU_AckThroughServer(const LicqUser* u, unsigned long msgid1,
                         unsigned long msgid2, unsigned short sequence,
                         unsigned short msgType, bool bAccept,
                         unsigned short nLevel, const char *GUID);
protected:
  void InitBuffer();

  unsigned long m_nMsgID[2];
  unsigned short m_nSequence, m_nMsgType, m_nStatus, m_nUinLen, m_nLevel;
  char m_szUin[13];
  char *m_szMessage;
  char m_GUID[GUID_LENGTH];
};

//-----AckGeneral--------------------------------------------------------------
class CPU_AckGeneral : public CPU_AckThroughServer
{
public:
  CPU_AckGeneral(const LicqUser* u, unsigned long nMsgID1,
                 unsigned long nMsgID2, unsigned short nSequence,
                 unsigned short nMsgType, bool bAccept, unsigned short nLevel);
};

//-----AckFileAccept-----------------------------------------------------------
class CPU_AckFileAccept : public CPU_AdvancedMessage
{
public:
  CPU_AckFileAccept(const LicqUser* u, const unsigned long nMsgID[],
                    unsigned short nSequence, unsigned short nPort,
                    const char *szDesc, const char *szFile,
                    unsigned long nFileSize);
};

//-----AckFileRefuse-----------------------------------------------------------
class CPU_AckFileRefuse : public CPU_AckThroughServer
{
public:
  CPU_AckFileRefuse(const LicqUser* u, const unsigned long nMsgID[],
                    unsigned short nSequence, const char *msg);
};

//-----AckChatAccept-----------------------------------------------------------
class CPU_AckChatAccept : public CPU_AdvancedMessage
{
public:
  CPU_AckChatAccept(const LicqUser* u, const char *szClients, const unsigned long nMsgID[],
                    unsigned short nSequence, unsigned short nPort);
};

//-----AckChatRefuse-----------------------------------------------------------
class CPU_AckChatRefuse : public CPU_AckThroughServer
{
public:
  CPU_AckChatRefuse(const LicqUser* u, const unsigned long nMsgID[],
                    unsigned short nSequence, const char *msg);
};

//-----PluginError-------------------------------------------------------------
class CPU_PluginError : public CPU_AckThroughServer
{
public:
  CPU_PluginError(const LicqUser* u, unsigned long nMsgID1, unsigned long nMsgID2,
                  unsigned short nSequence, const char *GUID);
};

//-----InfoPluginListResp------------------------------------------------------
class CPU_InfoPluginListResp : public CPU_AckThroughServer
{
public:
  CPU_InfoPluginListResp(const LicqUser* u, unsigned long nMsgID1,
                         unsigned long nMsgID2, unsigned short nSequence);
};

//-----InfoPhoneBookResp-------------------------------------------------------
class CPU_InfoPhoneBookResp : public CPU_AckThroughServer
{
public:
  CPU_InfoPhoneBookResp(const LicqUser* u, unsigned long nMsgID1,
                        unsigned long nMsgID2, unsigned short nSequence);
};

//-----InfoPictureResp---------------------------------------------------------
class CPU_InfoPictureResp : public CPU_AckThroughServer
{
public:
  CPU_InfoPictureResp(const LicqUser* u, unsigned long nMsgID1,
                      unsigned long nMsgID2, unsigned short nSequence);
};

//-----StatusPluginListResp----------------------------------------------------
class CPU_StatusPluginListResp : public CPU_AckThroughServer
{
public:
  CPU_StatusPluginListResp(const LicqUser* u, unsigned long nMsgID1,
                           unsigned long nMsgID2, unsigned short nSequence);
};

//-----StatusPluginResp-----------------------------------------------------
class CPU_StatusPluginResp : public CPU_AckThroughServer
{
public:
  CPU_StatusPluginResp(const LicqUser* u, unsigned long nMsgID1,
                       unsigned long nMsgID2, unsigned short nSequence,
                       unsigned long nStatus);
};

//-----SendSms-----------------------------------------------------------
class CPU_SendSms : public CPU_CommonFamily
{
public:
   CPU_SendSms(const char *szNumber, const char *szMessage);
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
  CPU_RequestAuth(const char* id, const char *);
};

//-----Authorize----------------------------------------------------------------
class CPU_Authorize : public CPU_CommonFamily
{
public:
   CPU_Authorize(const char *szId);
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
  CPU_Meta_SetGeneralInfo(const char *szAlias,
                          const char *szFirstName,
                          const char *szLastName,
                          const char *szEmailPrimary,
                          const char *szCity,
                          const char *szState,
                          const char *szPhoneNumber,
                          const char *szFaxNumber,
                          const char *szAddress,
                          const char *szCellularNumber,
                          const char *szZipCode,
                          unsigned short nCountryCode,
                          bool bHideEmail);
  virtual unsigned short SubCommand()  { return m_nMetaCommand; }
protected:
  unsigned short m_nMetaCommand;

  char *m_szAlias;
  char *m_szFirstName;
  char *m_szLastName;
  char *m_szEmailPrimary;
  char *m_szCity;
  char *m_szState;
  char *m_szPhoneNumber;
  char *m_szFaxNumber;
  char *m_szAddress;
  char *m_szCellularNumber;
  char *m_szZipCode;
  unsigned short m_nCountryCode;
  char m_nTimezone;
  char m_nHideEmail;

friend class CICQDaemon;
};


//-----Meta_SetMoreEmailInfo---------------------------------------------------
class CPU_Meta_SetEmailInfo : public CPU_CommonFamily
{
public:
  CPU_Meta_SetEmailInfo(const char *szEmailSecondary,
                        const char *szEmailOld);
  virtual unsigned short SubCommand()  { return m_nMetaCommand; }
protected:
  unsigned short m_nMetaCommand;

  char *m_szEmailSecondary;
  char *m_szEmailOld;

friend class CICQDaemon;
};


//-----Meta_SetMoreInfo------------------------------------------------------
class CPU_Meta_SetMoreInfo : public CPU_CommonFamily
{
public:
  CPU_Meta_SetMoreInfo(unsigned short nAge,
                       char nGender,
                       const char *szHomepage,
                       unsigned short nBirthYear,
                       char nBirthMonth,
                       char nBirthDay,
                       char nLanguage1,
                       char nLanguage2,
                       char nLanguage3);
  virtual unsigned short SubCommand()  { return m_nMetaCommand; }
protected:
  unsigned short m_nMetaCommand;

  unsigned short m_nAge;
  char m_nGender;
  char *m_szHomepage;
  unsigned short m_nBirthYear;
  char m_nBirthMonth;
  char m_nBirthDay;
  char m_nLanguage1;
  char m_nLanguage2;
  char m_nLanguage3;

friend class CICQDaemon;
};

//-----Meta_SetInterestsInfo----------------------------------------------------
class CPU_Meta_SetInterestsInfo : public CPU_CommonFamily
{
public:
  CPU_Meta_SetInterestsInfo(const UserCategoryMap& interests);
  virtual unsigned short SubCommand()  { return m_nMetaCommand; }
protected:
  unsigned short m_nMetaCommand;

  UserCategoryMap myInterests;

friend class CICQDaemon;
};


//-----Meta_SetOrgBackInfo------------------------------------------------------
class CPU_Meta_SetOrgBackInfo : public CPU_CommonFamily
{
public:
  CPU_Meta_SetOrgBackInfo(const UserCategoryMap& orgs,
      const UserCategoryMap& background);
  virtual unsigned short SubCommand()  { return m_nMetaCommand; }
protected:
  unsigned short m_nMetaCommand;

  UserCategoryMap myOrganizations;
  UserCategoryMap myBackgrounds;

friend class CICQDaemon;
};

//-----Meta_SetWorkInfo------------------------------------------------------
class CPU_Meta_SetWorkInfo : public CPU_CommonFamily
{
public:
  CPU_Meta_SetWorkInfo(const char *szCity,
                       const char *szState,
                       const char *szPhoneNumber,
                       const char *szFaxNumber,
                       const char *szAddress,
                       const char *szZip,
                       unsigned short nCompanyCountry,
                       const char *szName,
                       const char *szDepartment,
                       const char *szPosition,
                       unsigned short nCompanyOccupation,
                       const char *szHomepage);
  virtual unsigned short SubCommand()  { return m_nMetaCommand; }
protected:
  unsigned short m_nMetaCommand;

  char *m_szCity;
  char *m_szState;
  char *m_szPhoneNumber;
  char *m_szFaxNumber;
  char *m_szAddress;
  char *m_szZip;
  unsigned short m_nCompanyCountry;
  char *m_szName;
  char *m_szDepartment;
  char *m_szPosition;
  unsigned short m_nCompanyOccupation;
  char *m_szHomepage;

friend class CICQDaemon;
};


//-----Meta_SetAbout---------------------------------------------------------
class CPU_Meta_SetAbout : public CPU_CommonFamily
{
public:
  CPU_Meta_SetAbout(const char *szAbout);
  virtual ~CPU_Meta_SetAbout();
  virtual unsigned short SubCommand()  { return m_nMetaCommand; }
protected:
  unsigned short m_nMetaCommand;

  char *m_szAbout;

friend class CICQDaemon;
};

//-----RequestInfo-----------------------------------------------------------
class CPU_RequestInfo : public CPU_CommonFamily
{
public:
  CPU_RequestInfo(const char *szId);
};

//-----RequestBuddyIcon------------------------------------------------------
class CPU_RequestBuddyIcon : public CPU_CommonFamily
{
public:
  CPU_RequestBuddyIcon(const char *szId, unsigned short _nBuddyIconType,
                       char _nBuddyIconHashType, const char *_szBuddyIconHash,
                       unsigned short nService);
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
  CPU_AIMFetchAwayMessage(const char *szId);
};

//-----SetPassword---------------------------------------------------------
class CPU_SetPassword : public CPU_CommonFamily
{
public:
  CPU_SetPassword(const char *szPassword);
  virtual unsigned short SubCommand()  { return m_nMetaCommand; }
  unsigned long Uin()           { return 0; }
protected:
  unsigned short m_nMetaCommand;
  char *m_szPassword;

friend class CICQDaemon;
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
  CPU_Meta_RequestAllInfo(const char *_szId);
  virtual ~CPU_Meta_RequestAllInfo();
  virtual unsigned short SubCommand()  { return m_nMetaCommand; }
  const char *Id()  {  return m_szId; }
protected:
  unsigned short m_nMetaCommand;
  char *m_szId;
};


//-----Meta_RequestBasicInfo------------------------------------------------------
class CPU_Meta_RequestBasicInfo : public CPU_CommonFamily
{
public:
  CPU_Meta_RequestBasicInfo(const char *_szId);
  virtual ~CPU_Meta_RequestBasicInfo();
  virtual unsigned short SubCommand()  { return m_nMetaCommand; }
  const char *Id()  {  return m_szId; }
protected:
  unsigned short m_nMetaCommand;
  char *m_szId;
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
  virtual unsigned short Command()    { return ICQ_CMDxTCP_HANDSHAKE; }
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
  CPacketTcp_Handshake_Confirm(unsigned char nChannel, unsigned short nSequence);
  CPacketTcp_Handshake_Confirm(CBuffer *inbuf);

  virtual unsigned char Channel() { return m_nChannel; }
  unsigned long Id() { return m_nId; }
protected:
  unsigned char m_nChannel;
  unsigned long m_nId;
};


//-----CPacketTcp---------------------------------------------------------------
class CPacketTcp : public CPacket
{
public:
   virtual ~CPacketTcp();

   virtual CBuffer *Finalize(INetSocket *);
   virtual unsigned short Sequence()   { return m_nSequence; }
   virtual unsigned short SubSequence()   { return 0; }
   virtual unsigned short Command()    { return m_nCommand; }
   virtual unsigned short SubCommand() { return m_nSubCommand; }

   char *LocalPortOffset()  {  return m_szLocalPortOffset; }
   unsigned short Level()  { return m_nLevel; }
   unsigned short Version()  { return m_nVersion; }
protected:
   CPacketTcp(unsigned long _nCommand, unsigned short _nSubCommand,
      const char *szMessage, bool _bAccept, unsigned short nLevel,
      LicqUser* _cUser, size_t _nLen = 0);
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
   char          *m_szMessage;
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
   CPT_Message(char *_sMessage, unsigned short nLevel, bool bMR,
      CICQColor* pColor, LicqUser* pUser, size_t nLen = 0);
};


//-----Url----------------------------------------------------------------------
/* BA 95 47 00 03 00 EE 07 00 00 BA 95 47 00 04 00 24 00 67 6F 6F 64 20 70 6F
   72 6E 20 73 69 74 65 FE 68 74 74 70 3A 2F 2F 63 6F 6F 6C 70 6F 72 74 6E 2E
   63 6F 6D 00 81 61 1D 9E 7F 00 00 01 3F 07 00 00 04 00 00 10 00 03 00 00 00 */
class CPT_Url : public CPacketTcp
{
public:
  CPT_Url(const char* _sMessage, unsigned short nLevel, bool bMR,
      CICQColor* pColor, LicqUser *pUser);
};


class CPT_ContactList : public CPacketTcp
{
public:
   CPT_ContactList(char *szMessage, unsigned short nLevel, bool bMR,
      CICQColor* pColor, LicqUser* pUser);
};




//-----ReadAwayMessage----------------------------------------------------------
class CPT_ReadAwayMessage : public CPacketTcp
{
public:
  CPT_ReadAwayMessage(LicqUser* _cUser);
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
  CPT_ChatRequest(char *_sMessage, const char *szChatUsers, unsigned short nPort,
      unsigned short nLevel, LicqUser* pUser, bool bICBM);
};


//-----FileTransfer-------------------------------------------------------------
class CPT_FileTransfer : public CPacketTcp, public CPX_FileTransfer
{
public:
   CPT_FileTransfer(ConstFileList &lFileList, const char *_szFilename,
      const char* _szDescription, unsigned short nLevel, LicqUser* pUser);
   const char *GetDescription() { return m_szMessage; }
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
  CPT_OpenSecureChannel(LicqUser* pUser);
};


class CPT_CloseSecureChannel : public CPacketTcp
{
public:
  CPT_CloseSecureChannel(LicqUser* pUser);
};




//++++Ack++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/* 50 A5 82 00 03 00 DA 07 00 00 50 A5 82 00 01 00 01 00 00 CF 60 AD D3 CF
   60 AD D3 60 12 00 00 04 00 00 00 00 02 00 00 00 */
class CPT_Ack : public CPacketTcp
{
protected:
  CPT_Ack(unsigned short _nSubCommand, unsigned short _nSequence,
      bool _bAccept, bool _bUrgent, LicqUser* _cUser);
  virtual ~CPT_Ack();
};


//-----AckGeneral------------------------------------------------------------
class CPT_AckGeneral : public CPT_Ack
{
public:
  CPT_AckGeneral(unsigned short nSubCommand, unsigned short nSequence,
      bool bAccept, bool bUrgent, LicqUser* pUser);
};


//-----AckKey------------------------------------------------------------
class CPT_AckOldSecureChannel : public CPT_Ack
{
public:
  CPT_AckOldSecureChannel(unsigned short nSequence, LicqUser* pUser);
};


//-----AckKey------------------------------------------------------------
class CPT_AckOpenSecureChannel : public CPT_Ack
{
public:
  CPT_AckOpenSecureChannel(unsigned short nSequence, bool ok, LicqUser* pUser);
};


//-----AckKey------------------------------------------------------------
class CPT_AckCloseSecureChannel : public CPT_Ack
{
public:
  CPT_AckCloseSecureChannel(unsigned short nSequence, LicqUser* pUser);
};


#if 0
//-----AckMessage------------------------------------------------------------
class CPT_AckMessage : public CPT_Ack
{
public:
  CPT_AckMessage(unsigned short _nSequence, bool _bAccept, bool _bUrgent, LicqUser* _cUser);
};


//-----AckReadAwayMessage-------------------------------------------------------
class CPT_AckReadAwayMessage : public CPT_Ack
{
public:
   CPT_AckReadAwayMessage(unsigned short _nSubCommand, unsigned short _nSequence,
      bool _bAccept, LicqUser* _cUser);
};


//-----AckUrl-------------------------------------------------------------------
class CPT_AckUrl : public CPT_Ack
{
public:
  CPT_AckUrl(unsigned short _nSequence, bool _bAccept, bool _bUrgent, LicqUser* _cUser);
};


//-----AckContactList----------------------------------------------------------
class CPT_AckContactList : public CPT_Ack
{
public:
  CPT_AckContactList(unsigned short _nSequence, bool _bAccept, bool _bUrgent,
      LicqUser* _cUser);
};
#endif

//-----AckChatRefuse------------------------------------------------------------
class CPT_AckChatRefuse : public CPT_Ack
{
public:
  CPT_AckChatRefuse(const char *_sReason, unsigned short _nSequence, LicqUser* _cUser);
   /* 50 A5 82 00 03 00 DA 07 00 00 50 A5 82 00 02 00 03 00 6E 6F 00 CF 60 AD
      95 CF 60 AD 95 1E 3C 00 00 04 01 00 00 00 01 00 00 00 00 00 00 00 00 00
      00 01 00 00 00 */
};


//-----AckChatAccept------------------------------------------------------------
class CPT_AckChatAccept : public CPT_Ack
{
public:
  CPT_AckChatAccept(unsigned short _nPort, const char *szClients,
      unsigned short _nSequence, LicqUser* _cUser, bool bICBM);
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
      LicqUser* _cUser);
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
  CPT_AckFileRefuse(const char *_sReason, unsigned short _nSequence, LicqUser* _cUser);
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
      LicqUser* _cUser);
};


//-----ChatCancel---------------------------------------------------------------
class CPT_CancelChat : public CPT_Cancel
{
public:
  CPT_CancelChat(unsigned short _nSequence, LicqUser* _cUser);
   /* 50 A5 82 00 03 00 D0 07 00 00 50 A5 82 00 02 00 01 00 00 CF 60 AD D3 CF
      60 AD D3 28 12 00 00 04 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 06
      00 00 00 */
};


//-----FileCancel---------------------------------------------------------------
class CPT_CancelFile : public CPT_Cancel
{
public:
  CPT_CancelFile(unsigned short _nSequence, LicqUser* _cUser);
   /* 50 A5 82 00 03 00 D0 07 00 00 50 A5 82 00 02 00 01 00 00 CF 60 AD D3 CF
      60 AD D3 28 12 00 00 04 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 06
      00 00 00 */
};

//----Send error reply----------------------------------------------------------
class CPT_PluginError : public CPacketTcp
{
public:
  CPT_PluginError(LicqUser* _cUser, unsigned short nSequence,
     unsigned char nChannel);
   virtual unsigned char Channel() { return m_nChannel; }

protected:
   unsigned char m_nChannel;
};

//----Request info plugin list--------------------------------------------------
class CPT_InfoPluginReq : public CPacketTcp
{
public:
  CPT_InfoPluginReq(LicqUser* _cUser, const char* GUID, unsigned long int nTime);
   virtual unsigned char Channel()   { return ICQ_CHNxINFO; }
   virtual const char *RequestGUID()        { return m_ReqGUID; }
   virtual unsigned short ExtraInfo() { return DirectInfoPluginRequest; }

protected:
   char m_ReqGUID[GUID_LENGTH];
};

//----Response to phone book request--------------------------------------------
class CPT_InfoPhoneBookResp : public CPacketTcp
{
public:
  CPT_InfoPhoneBookResp(LicqUser* _cUser, unsigned short nSequence);
   virtual unsigned char Channel() { return ICQ_CHNxINFO; }
};

//-----Response to picture request----------------------------------------------
class CPT_InfoPictureResp : public CPacketTcp
{
public:
  CPT_InfoPictureResp(LicqUser* _cUser, unsigned short nSequence);
   virtual unsigned char Channel() { return ICQ_CHNxINFO; }
};

//----Response to info plugin list request--------------------------------------
class CPT_InfoPluginListResp : public CPacketTcp
{
public:
  CPT_InfoPluginListResp(LicqUser* _cUser, unsigned short nSequence);
   virtual unsigned char Channel() { return ICQ_CHNxINFO; }
};

//----Send status plugin request------------------------------------------------
class CPT_StatusPluginReq : public CPacketTcp
{
public:
  CPT_StatusPluginReq(LicqUser* _cUser, const char* GUID, unsigned long nTime);
   virtual unsigned char  Channel()   { return ICQ_CHNxSTATUS; }
   virtual unsigned short ExtraInfo() { return DirectStatusPluginRequest;}
   virtual const char *RequestGUID() { return m_ReqGUID; }

protected:
  char m_ReqGUID[GUID_LENGTH];
};

//----Response to status plugin list request------------------------------------
class CPT_StatusPluginListResp : public CPacketTcp
{
public:
  CPT_StatusPluginListResp(LicqUser* _cUser, unsigned short nSequence);
  virtual unsigned char  Channel()   { return ICQ_CHNxSTATUS; }
};

//----Response to status request------------------------------------------------
class CPT_StatusPluginResp : public CPacketTcp
{
public:
  CPT_StatusPluginResp(LicqUser* _cUser, unsigned short nSequence,
                       unsigned long nStatus);
  virtual unsigned char  Channel()   { return ICQ_CHNxSTATUS; }
};

#endif
