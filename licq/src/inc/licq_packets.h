#ifndef ICQPACKET_H
#define ICQPACKET_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "licq_user.h"
#include "licq_buffer.h"
#include "licq_socket.h"
#include "licq_icq.h"


unsigned short ReversePort(unsigned short p);


//=====Packet===================================================================

class CPacket
{
public:
   CBuffer *getBuffer()  { return buffer; };
   virtual CBuffer *Finalize() { return NULL; }

   virtual const unsigned long  getSequence() = 0;
   virtual const unsigned short SubSequence() = 0;
   virtual const unsigned short getCommand() = 0;
   virtual const unsigned short getSubCommand() = 0;

   static void SetMode(char c) { s_nMode = c; }
   static char Mode()  { return s_nMode; }
   static void SetLocalIp(unsigned long n)  {  s_nLocalIp = n; }
   static void SetIps(INetSocket *s);

protected:
   CBuffer *buffer;
   unsigned short m_nSize;

   static unsigned long s_nLocalIp;
   static unsigned long s_nRealIp;
   static char s_nMode;
};

//=====UDP======================================================================

//-----PacketUdp----------------------------------------------------------------
class CPacketUdp : public CPacket
{
public:
   virtual ~CPacketUdp();

   virtual CBuffer *Finalize();
   virtual const unsigned long  getSequence() { return m_nSequence; }
   virtual const unsigned short SubSequence() { return m_nSubSequence; }
   virtual const unsigned short getCommand()  { return m_nCommand; }
   virtual const unsigned short getSubCommand()  { return 0; }
protected:
   CPacketUdp(unsigned short _nCommand);
   void InitBuffer();

   static bool s_bRegistered;

#if ICQ_VERSION == 2
   unsigned short m_nVersion;
   unsigned short m_nCommand;
   unsigned short m_nSequence;
   unsigned long  m_nSourceUin;
   unsigned short m_nSubSequence;
#elif ICQ_VERSION == 4
   unsigned short m_nVersion;
   unsigned short m_nRandom;
   unsigned short m_nZero;
   unsigned short m_nCommand;
   unsigned short m_nSequence;
   unsigned short m_nSubSequence;
   unsigned long  m_nSourceUin;
   unsigned long  m_nCheckSum;
#elif ICQ_VERSION == 5
   unsigned short m_nVersion;
   unsigned long  m_nZero;
   unsigned long  m_nSourceUin;
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

#if ICQ_VERSION == 2
//-----Register----------------------------------------------------------------
// Doesn't actually descend from CPacketUdp in version 2 but we keep the
// name the same for simplicity
class CPU_Register : public CPacket
{
public:
  CPU_Register(const char *_szPasswd);
  virtual ~CPU_Register();

  virtual const unsigned long  getSequence() { return m_nSequence; }
  virtual const unsigned short SubSequence() { return 0; }
  virtual const unsigned short getCommand()  { return m_nCommand; }
  virtual const unsigned short getSubCommand() { return 0; }
protected:
  virtual unsigned long getSize();

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
};
#endif

//-----Logon--------------------------------------------------------------------
class CPU_Logon : public CPacketUdp
{
public:
  CPU_Logon(unsigned short nLocalPort, const char *_szPassword,
     unsigned short _nLogonStatus);

  virtual CBuffer *Finalize();
protected:
  /* 02 00 E8 03 08 00 8F 76 20 00 34 4A 00 00 08 00 5B 63 65 50 61 62 43 00
     72 00 04 00 7F 00 00 01 04 00 00 00 00 03 00 00 00 02 00 00 00 00 00 04
     00 72 00 */
  unsigned long  m_nLocalPort;
  unsigned long  m_nLogonStatus;
  unsigned long  m_nTcpVersion;

  char *m_szRealIpOffset;
};


//-----Ack---------------------------------------------------------------------
class CPU_Ack : public CPacketUdp
{
public:
#if ICQ_VERSION == 2
   CPU_Ack(unsigned short _nSequence);
#elif ICQ_VERSION == 4 || ICQ_VERSION == 5
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


//-----Logoff-------------------------------------------------------------------
class CPU_Logoff : public CPacketUdp
{
public:
   CPU_Logoff();
};


//-----ContactList--------------------------------------------------------------
class CPU_ContactList : public CPacketUdp
{
public:
   CPU_ContactList(UinList &uins);
   CPU_ContactList(unsigned long _nUin);
};


//-----VisibleList--------------------------------------------------------------
class CPU_VisibleList : public CPacketUdp
{
public:
  CPU_VisibleList(UinList &uins);
};


//-----InvisibleList--------------------------------------------------------------
class CPU_InvisibleList : public CPacketUdp
{
public:
   CPU_InvisibleList(UinList &uins);
};


//-----SearchByInfo--------------------------------------------------------------
class CPU_SearchByInfo : public CPacketUdp
{
public:
   CPU_SearchByInfo(const char *_sAlias, const char *_sFirstName,
                    const char *_sLastName, const char *_sEmail);
   /* 02 00 24 04 04 00 50 A5 82 00 05 00 0B 00 41 70 6F 74 68 65 6F 73 69 73
      00 07 00 47 72 61 68 61 6D 00 05 00 52 6F 66 66 00 01 00 00 */
};


//-----SearchByUin--------------------------------------------------------------
class CPU_SearchByUin : public CPacketUdp
{
public:
   CPU_SearchByUin(unsigned long nUin);
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
class CPU_Ping : public CPacketUdp
{
public:
   CPU_Ping();
};


//-----ThroughServer------------------------------------------------------------
class CPU_ThroughServer : public CPacketUdp
{
public:
   CPU_ThroughServer(unsigned long _nSourceUin, unsigned long _nDestinationUin,
                     unsigned short _nSubCommand, char *_sMessage);
   virtual const unsigned short getSubCommand()  { return m_nSubCommand; }
protected:
   unsigned long  m_nDestinationUin;
   unsigned short m_nSubCommand;
};


//-----SetStatus----------------------------------------------------------------
class CPU_SetStatus : public CPacketUdp
{
public:
   CPU_SetStatus(unsigned long _nNewStatus);
   unsigned long Status() { return m_nNewStatus; }
protected:
   unsigned long m_nNewStatus;
};


//-----GetUserBasicInfo---------------------------------------------------------
class CPU_GetUserBasicInfo : public CPacketUdp
{
public:
   CPU_GetUserBasicInfo(unsigned long _nUserUin);
protected:
   /* 02 00 60 04 B7 00 BA 95 47 00 0A 00 8F 76 20 00 */
   unsigned long  m_nUserUin;
};


//-----GetUserExtInfo---------------------------------------------------------
class CPU_GetUserExtInfo : public CPacketUdp
{
public:
   CPU_GetUserExtInfo(unsigned long _nUserUin);
protected:
   /* 02 00 60 04 B7 00 BA 95 47 00 0A 00 8F 76 20 00 */
   unsigned long  m_nUserUin;
};


//-----Authorize----------------------------------------------------------------
class CPU_Authorize : public CPacketUdp
{
public:
   CPU_Authorize(unsigned long _nAuthorizeUin);
protected:
   /* 02 00 56 04 05 00 50 A5 82 00 A7 B8 19 00 08 00 01 00 00 */
   unsigned long m_nAuthorizeUin;
};

//-----RequestSysMsg------------------------------------------------------------
class CPU_RequestSysMsg : public CPacketUdp
{
public:
   CPU_RequestSysMsg();
};


//-----SysMsgDoneAck------------------------------------------------------------
class CPU_SysMsgDoneAck : public CPacketUdp
{
public:
#if ICQ_VERSION == 2
  CPU_SysMsgDoneAck(unsigned short _nSequence);
#elif ICQ_VERSION == 4
  CPU_SysMsgDoneAck(unsigned short _nSequence, unsigned short _nSubSequence);
#elif ICQ_VERSION == 5
  CPU_SysMsgDoneAck();
#endif
};


//-----SetRandomChatGroup----------------------------------------------------
class CPU_SetRandomChatGroup : public CPacketUdp
{
public:
  CPU_SetRandomChatGroup(unsigned long nGroup);
  unsigned long Group() { return m_nGroup; }
protected:
  unsigned long m_nGroup;
};



//-----RandomChatSearch----------------------------------------------------
class CPU_RandomChatSearch : public CPacketUdp
{
public:
  CPU_RandomChatSearch(unsigned long nGroup);
protected:
  unsigned long m_nGroup;
};



//-----Meta_SetWorkInfo------------------------------------------------------
class CPU_Meta_SetGeneralInfo : public CPacketUdp
{
public:
  CPU_Meta_SetGeneralInfo(const char *szAlias,
                          const char *szFirstName,
                          const char *szLastName,
                          const char *szEmail1,
                          const char *szEmail2,
                          const char *szCity,
                          const char *szState,
                          const char *szPhoneNumber,
                          const char *szFaxNumber,
                          const char *szAddress,
                          const char *szCellularNumber,
                          unsigned long nZipCode,
                          unsigned short nCountryCode,
                          bool bHideEmail);
  virtual const unsigned short getSubCommand()  { return m_nMetaCommand; }
protected:
  unsigned short m_nMetaCommand;

  char *m_szAlias;
  char *m_szFirstName;
  char *m_szLastName;
  char *m_szEmail1;
  char *m_szEmail2;
  char *m_szCity;
  char *m_szState;
  char *m_szPhoneNumber;
  char *m_szFaxNumber;
  char *m_szAddress;
  char *m_szCellularNumber;
  unsigned long m_nZipCode;
  unsigned short m_nCountryCode;
  char m_nTimezone;
  char m_nAuthorization;
  char m_nWebAware;
  char m_nHideEmail;

friend class CICQDaemon;
};


//-----Meta_SetMoreInfo------------------------------------------------------
class CPU_Meta_SetMoreInfo : public CPacketUdp
{
public:
  CPU_Meta_SetMoreInfo(unsigned short nAge,
                       char nGender,
                       const char *szHomepage,
                       char nBirthYear,
                       char nBirthMonth,
                       char nBirthDay,
                       char nLanguage1,
                       char nLanguage2,
                       char nLanguage3);
  virtual const unsigned short getSubCommand()  { return m_nMetaCommand; }
protected:
  unsigned short m_nMetaCommand;

  unsigned short m_nAge;
  char m_nGender;
  char *m_szHomepage;
  char m_nBirthYear;
  char m_nBirthMonth;
  char m_nBirthDay;
  char m_nLanguage1;
  char m_nLanguage2;
  char m_nLanguage3;

friend class CICQDaemon;
};


//-----Meta_SetWorkInfo------------------------------------------------------
class CPU_Meta_SetWorkInfo : public CPacketUdp
{
public:
  CPU_Meta_SetWorkInfo(const char *szCity,
                       const char *szState,
                       const char *szPhoneNumber,
                       const char *szFaxNumber,
                       const char *szAddress,
                       const char *szName,
                       const char *szDepartment,
                       const char *szPosition,
                       const char *szHomepage);
  virtual const unsigned short getSubCommand()  { return m_nMetaCommand; }
protected:
  unsigned short m_nMetaCommand;

  char *m_szCity;
  char *m_szState;
  char *m_szPhoneNumber;
  char *m_szFaxNumber;
  char *m_szAddress;
  char *m_szName;
  char *m_szDepartment;
  char *m_szPosition;
  char *m_szHomepage;

friend class CICQDaemon;
};


//-----Meta_SetAbout---------------------------------------------------------
class CPU_Meta_SetAbout : public CPacketUdp
{
public:
  CPU_Meta_SetAbout(const char *szAbout);
  ~CPU_Meta_SetAbout();
  virtual const unsigned short getSubCommand()  { return m_nMetaCommand; }
protected:
  unsigned short m_nMetaCommand;

  char *m_szAbout;

friend class CICQDaemon;
};


//-----Meta_SetPassword---------------------------------------------------------
class CPU_Meta_SetPassword : public CPacketUdp
{
public:
  CPU_Meta_SetPassword(const char *szPassword);
  virtual const unsigned short getSubCommand()  { return m_nMetaCommand; }
protected:
  unsigned short m_nMetaCommand;

  char *m_szPassword;

friend class CICQDaemon;
};


//-----Meta_SetSecurityInfo--------------------------------------------------
class CPU_Meta_SetSecurityInfo : public CPacketUdp
{
public:
  CPU_Meta_SetSecurityInfo(bool bAuthorization,
                           bool bHideIp,
                           bool bWebAware);
  virtual const unsigned short getSubCommand()  { return m_nMetaCommand; }

  bool Authorization()  { return m_nAuthorization == 0; }
  bool HideIp()         { return m_nHideIp == 1; }
  bool WebAware()       { return m_nWebAware == 1; }
protected:
  unsigned short m_nMetaCommand;
  char m_nAuthorization;
  char m_nHideIp;
  char m_nWebAware;
};


//-----Meta_RequestInfo------------------------------------------------------
class CPU_Meta_RequestInfo : public CPacketUdp
{
public:
  CPU_Meta_RequestInfo(unsigned long _nUin);
  virtual const unsigned short getSubCommand()  { return m_nMetaCommand; }
  unsigned long Uin()  {  return m_nUin; }
protected:
  unsigned short m_nMetaCommand;
  unsigned long  m_nUin;
};


//=====TCP======================================================================

//-----PacketTcp_Handshake------------------------------------------------------
class CPacketTcp_Handshake : public CPacket
{
public:
   CPacketTcp_Handshake(unsigned long _nLocalPort);
   virtual ~CPacketTcp_Handshake();

   virtual const unsigned long  getSequence()   { return 0; }
   virtual const unsigned short SubSequence()   { return 0; }
   virtual const unsigned short getCommand()    { return ICQ_CMDxTCP_HANDSHAKE; }
   virtual const unsigned short getSubCommand() { return 0; }
protected:
   void InitBuffer();

   /* FF 03 00 00 00 3D 62 00 00 50 A5 82 00 CF 60 AD 95 CF 60 AD 95 04 3D 62
      00 00 */
   unsigned long  m_nLocalPort;
   unsigned long  m_nSourceUin;
   unsigned long  m_nLocalHost;
};


//-----CPacketTcp---------------------------------------------------------------
class CPacketTcp : public CPacket
{
public:
   virtual ~CPacketTcp();

   virtual CBuffer *Finalize();
   virtual const unsigned long  getSequence()   { return m_nSequence; }
   virtual const unsigned short SubSequence()   { return 0; }
   virtual const unsigned short getCommand()    { return m_nCommand; }
   virtual const unsigned short getSubCommand() { return m_nSubCommand; }

   char *LocalPortOffset()  {  return m_szLocalPortOffset; }
protected:
   CPacketTcp(unsigned long _nSourceUin, unsigned long _nCommand,
              unsigned short _nSubCommand, const char *szMessage, bool _bAccept,
              unsigned short nLevel, ICQUser *_cUser);
   void InitBuffer();
   void PostBuffer();

   unsigned long  m_nSourceUin;
   unsigned long  m_nCommand;
   unsigned short m_nSubCommand;
   char          *m_szMessage;
   unsigned long  m_nLocalPort;
   unsigned short m_nStatus;
   unsigned short m_nMsgType;
   unsigned long  m_nSequence;

   char *m_szLocalPortOffset;
};


//-----Message------------------------------------------------------------------
class CPT_Message : public CPacketTcp
{
public:
   CPT_Message(unsigned long _nSourceUin, char *_sMessage, unsigned short nLevel, ICQUser *_cUser);
};


//-----Url----------------------------------------------------------------------
class CPT_Url : public CPacketTcp
{
public:
   CPT_Url(unsigned long _nSourceUin, char *_sMessage, unsigned short nLevel, ICQUser *_cUser);
   /* BA 95 47 00 03 00 EE 07 00 00 BA 95 47 00 04 00 24 00 67 6F 6F 64 20 70 6F
      72 6E 20 73 69 74 65 FE 68 74 74 70 3A 2F 2F 63 6F 6F 6C 70 6F 72 74 6E 2E
      63 6F 6D 00 81 61 1D 9E 7F 00 00 01 3F 07 00 00 04 00 00 10 00 03 00 00 00 */
};


//-----ReadAwayMessage----------------------------------------------------------
class CPT_ReadAwayMessage : public CPacketTcp
{
public:
   CPT_ReadAwayMessage(unsigned long _nSourceUin, ICQUser *_cUser);
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
  CPT_ChatRequest(unsigned long _nSourceUin, char *_sMessage,
     const char *szChatUsers, unsigned short nPort,
     unsigned short nLevel, ICQUser *_cUser);
};


//-----FileTransfer-------------------------------------------------------------
class CPT_FileTransfer : public CPacketTcp
{
public:
   CPT_FileTransfer(unsigned long _nSourceUin, const char *_szFilename,
                    const char *_szDescription, unsigned short nLevel,
                    ICQUser *_cUser);
   bool IsValid()  { return m_bValid; };
   const char *GetFilename()  { return m_szFilename; };
   const char *GetDescription()  { return m_szMessage; };
   unsigned long GetFileSize()  { return m_nFileSize; };
protected:
   bool m_bValid;

   /* 50 A5 82 00 03 00 EE 07 00 00 50 A5 82 00 03 00 0F 00 74 68 69 73 20 69
      73 20 61 20 66 69 6C 65 00 CF 60 AD D3 CF 60 AD D3 60 12 00 00 04 00 00
      10 00 00 00 00 00 09 00 4D 61 6B 65 66 69 6C 65 00 55 0C 00 00 00 00 00
      00 04 00 00 00 */
   char           *m_szFilename;
   unsigned long  m_nFileSize;
};


//++++Ack+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CPT_Ack : public CPacketTcp
{
protected:
  CPT_Ack(unsigned short _nSubCommand, unsigned long _nSequence,
          bool _bAccept, bool _bUrgent, ICQUser *_cUser);
  /* 50 A5 82 00 03 00 DA 07 00 00 50 A5 82 00 01 00 01 00 00 CF 60 AD D3 CF
     60 AD D3 60 12 00 00 04 00 00 00 00 02 00 00 00 */
};


//-----AckMessage---------------------------------------------------------------
class CPT_AckMessage : public CPT_Ack
{
public:
  CPT_AckMessage(unsigned long _nSequence, bool _bAccept, bool _bUrgent, ICQUser *_cUser);
  /* 8F 76 20 00 03 00 DA 07 00 00 8F 76 20 00 01 00 01 00 00 CF 60 AD D3 7F
     00 00 01 5A 12 00 00 04 00 00 00 00 14 00 00 00 */
};


//-----AckReadAwayMessage-------------------------------------------------------
class CPT_AckReadAwayMessage : public CPT_Ack
{
public:
   CPT_AckReadAwayMessage(unsigned short _nSubCommand, unsigned long _nSequence,
                          bool _bAccept, ICQUser *_cUser);
};


//-----AckUrl-------------------------------------------------------------------
class CPT_AckUrl : public CPT_Ack
{
public:
  CPT_AckUrl(unsigned long _nSequence, bool _bAccept, bool _bUrgent, ICQUser *_cUser);
};


//-----AckContactList----------------------------------------------------------
class CPT_AckContactList : public CPT_Ack
{
public:
  CPT_AckContactList(unsigned long _nSequence, bool _bAccept, bool _bUrgent,
                     ICQUser *_cUser);
};


//-----AckChatRefuse------------------------------------------------------------
class CPT_AckChatRefuse : public CPT_Ack
{
public:
  CPT_AckChatRefuse(const char *_sReason, unsigned long _nSequence, ICQUser *_cUser);
   /* 50 A5 82 00 03 00 DA 07 00 00 50 A5 82 00 02 00 03 00 6E 6F 00 CF 60 AD
      95 CF 60 AD 95 1E 3C 00 00 04 01 00 00 00 01 00 00 00 00 00 00 00 00 00
      00 01 00 00 00 */
};


//-----AckChatAccept------------------------------------------------------------
class CPT_AckChatAccept : public CPT_Ack
{
public:
  CPT_AckChatAccept(unsigned short _nPort, unsigned long _nSequence,
                    ICQUser *_cUser);
   /* 50 A5 82 00 03 00 DA 07 00 00 50 A5 82 00 02 00 01 00 00 CF 60 AD 95 CF
      60 AD 95 1E 3C 00 00 04 00 00 00 00 01 00 00 40 78 00 00 78 40 00 00 02
      00 00 00 */
   unsigned long m_nPort;
};



//-----AckFileAccept------------------------------------------------------------
class CPT_AckFileAccept : public CPT_Ack
{
public:
  CPT_AckFileAccept(unsigned short _nPort, unsigned long _nSequence,
                    ICQUser *_cUser);
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
  CPT_AckFileRefuse(const char *_sReason, unsigned long _nSequence, ICQUser *_cUser);
protected:
   /* 50 A5 82 00 03 00 DA 07 00 00 50 A5 82 00 03 00 0A 00 6E 6F 20 74 68 61
      6E 6B 73 00 D1 EF 04 9F 7F 00 00 01 4A 1F 00 00 04 01 00 00 00 00 00 00
      00 01 00 00 00 00 00 00 00 00 00 00 03 00 00 00 */
};


//++++Cancel++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CPT_Cancel : public CPacketTcp
{
protected:
   CPT_Cancel(unsigned short _nSubCommand, unsigned long _nSequence,
              ICQUser *_cUser);
};


//-----ChatCancel---------------------------------------------------------------
class CPT_CancelChat : public CPT_Cancel
{
public:
   CPT_CancelChat(unsigned long _nSequence, ICQUser *_cUser);
   /* 50 A5 82 00 03 00 D0 07 00 00 50 A5 82 00 02 00 01 00 00 CF 60 AD D3 CF
      60 AD D3 28 12 00 00 04 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 06
      00 00 00 */
};


//-----FileCancel---------------------------------------------------------------
class CPT_CancelFile : public CPT_Cancel
{
public:
   CPT_CancelFile(unsigned long _nSequence, ICQUser *_cUser);
   /* 50 A5 82 00 03 00 D0 07 00 00 50 A5 82 00 02 00 01 00 00 CF 60 AD D3 CF
      60 AD D3 28 12 00 00 04 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 06
      00 00 00 */
};



#endif
