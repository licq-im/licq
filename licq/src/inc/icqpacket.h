#ifndef ICQPACKET_H
#define ICQPACKET_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "user.h"
#include "buffer.h"
#include "socket.h"


//=====Packet===================================================================

class CPacket
{
public:
   CBuffer *getBuffer(void)  { return buffer; };
   virtual void Create(void) = 0;

   virtual const unsigned long  getSequence(void) = 0;
   virtual const unsigned short SubSequence(void) = 0;
   virtual const unsigned short getCommand(void) = 0;
   virtual const unsigned short getSubCommand(void) = 0;

protected:
   CBuffer *buffer;
};

//=====UDP======================================================================

//-----Register----------------------------------------------------------------
class CPacketRegister : public CPacket
{
public:
  CPacketRegister(const char *_szPasswd);
  virtual ~CPacketRegister(void);
  virtual void Create(void) {}

  virtual const unsigned long  getSequence(void) { return m_nSequence; }
  virtual const unsigned short SubSequence(void) { return 0; }
  virtual const unsigned short getCommand(void)  { return m_nCommand; }
  virtual const unsigned short getSubCommand(void) { return 0; }
protected:
  virtual unsigned long getSize(void);

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


//-----PacketUdp----------------------------------------------------------------
class CPacketUdp : public CPacket
{
public:
   virtual ~CPacketUdp(void);

   virtual const unsigned long  getSequence(void) { return m_nSequence; }
   virtual const unsigned short SubSequence(void) { return 0; }
   virtual const unsigned short getCommand(void)  { return m_nCommand; }
   virtual const unsigned short getSubCommand(void)  { return 0; }
   virtual void Create(void) {};
protected:
   CPacketUdp(unsigned short _nCommand);
   virtual unsigned long getSize(void)     { return 10; };
   void initBuffer(void);

   unsigned short m_nVersion;
   unsigned short m_nCommand;
   unsigned short m_nSequence;
   unsigned long  m_nSourceUin;

   static unsigned short m_nSpecialSequence;
};


//-----Logon--------------------------------------------------------------------
class CPU_Logon : public CPacketUdp
{
public:
  CPU_Logon(INetSocket *_s, const char *_szPassword, unsigned short _nLogonStatus);
  virtual ~CPU_Logon(void);
protected:
  virtual unsigned long getSize(void);

  /* 02 00 E8 03 08 00 8F 76 20 00 34 4A 00 00 08 00 5B 63 65 50 61 62 43 00
     72 00 04 00 7F 00 00 01 04 00 00 00 00 03 00 00 00 02 00 00 00 00 00 04
     00 72 00 */
  // ... PacketUdp header
  unsigned long  m_nLocalPort;
  unsigned short m_nPasswordLength;
  char           *m_sPassword;
  char           m_aUnknown_1[4];
  unsigned long  m_nLocalIP;
  char           m_aUnknown_2;
  unsigned long  m_nLogonStatus;
  unsigned short m_nTcpVersion;
  char           m_aUnknown_3[12];
};


//-----Ack---------------------------------------------------------------------
class CPU_Ack : public CPacketUdp
{
public:
   CPU_Ack(unsigned long _nSequence);
protected:
   virtual unsigned long getSize(void);

   // ... PacketUdp header
};


//-----AddUser------------------------------------------------------------------
class CPU_AddUser : public CPacketUdp
{
public:
   CPU_AddUser(unsigned long _nAddedUin);
protected:
   virtual unsigned long getSize(void);

   /* 02 00 3C 05 06 00 50 A5 82 00 8F 76 20 00 */
   // ... PacketUdp header
   unsigned long  m_nAddedUin;
};


//-----Logoff-------------------------------------------------------------------
class CPU_Logoff : public CPacketUdp
{
public:
   CPU_Logoff(void);
protected:
   virtual unsigned long getSize(void);

   // ... PacketUdp header
   char m_aUnknown_1[2];
   char m_aDisconnect[20];
   char m_aUnknown_2[2];
};


//-----ContactList--------------------------------------------------------------
class CPU_ContactList : public CPacketUdp
{
public:
   CPU_ContactList(CUserGroup *_cUsers, unsigned short first,
                   unsigned short num);
   CPU_ContactList(unsigned long _nUin);
protected:
   virtual unsigned long getSize(void);

   // ... PacketUdp header
   char                   m_nNumUsers;
   vector <unsigned long> m_vnUins;
};


//-----VisibleList--------------------------------------------------------------
class CPU_VisibleList : public CPacketUdp
{
public:
   CPU_VisibleList(CUserGroup *_cUsers);
   bool empty(void)  { return (m_nNumUsers == 0); };
protected:
   virtual unsigned long getSize(void);

   // ... PacketUdp header
   char                   m_nNumUsers;
   vector <unsigned long> m_vnUins;
};


//-----InvisibleList--------------------------------------------------------------
class CPU_InvisibleList : public CPacketUdp
{
public:
   CPU_InvisibleList(CUserGroup *_cUsers);
   bool empty(void)  { return (m_nNumUsers == 0); };
protected:
   virtual unsigned long getSize(void);

   // ... PacketUdp header
   char                   m_nNumUsers;
   vector <unsigned long> m_vnUins;
};


//-----StartSearch--------------------------------------------------------------
class CPU_StartSearch : public CPacketUdp
{
public:
   CPU_StartSearch(const char *_sAlias, const char *_sFirstName,
                   const char *_sLastName, const char *_sEmail);
   virtual ~CPU_StartSearch(void);
   virtual const unsigned short SubSequence(void)  { return m_nSearchSequence; }
protected:
   virtual unsigned long getSize(void);

   static unsigned short s_nSearchSequence;

   /* 02 00 24 04 04 00 50 A5 82 00 05 00 0B 00 41 70 6F 74 68 65 6F 73 69 73
      00 07 00 47 72 61 68 61 6D 00 05 00 52 6F 66 66 00 01 00 00 */
   // ... PacketUdp header
   unsigned short m_nSearchSequence;
   unsigned short m_nAliasLength;
   char           *m_sAlias;
   unsigned short m_nFirstNameLength;
   char           *m_sFirstName;
   unsigned short m_nLastNameLength;
   char           *m_sLastName;
   unsigned short m_nEmailLength;
   char           *m_sEmail;
};


//-----UpdatePersonalBasicInfo--------------------------------------------------
class CPU_UpdatePersonalBasicInfo : public CPacketUdp
{
public:
   CPU_UpdatePersonalBasicInfo(const char *_sAlias, const char *_sFirstName,
                               const char *_sLastName, const char *_sEmail,
                               bool _bAuthorization);
   virtual ~CPU_UpdatePersonalBasicInfo(void);
   virtual const unsigned short SubSequence(void)  { return m_nUpdateSequence; };

   const char *Alias(void)  { return m_sAlias; }
   const char *FirstName(void)  { return m_sFirstName; }
   const char *LastName(void)  { return m_sLastName; }
   const char *Email(void)  { return m_sEmail; }
   bool Authorization(void)  { return m_nAuthorization == 0; }
protected:
   virtual unsigned long getSize(void);

   // ... PacketUdp header
   unsigned short m_nUpdateSequence;
   unsigned short m_nAliasLength;
   char           *m_sAlias;
   unsigned short m_nFirstNameLength;
   char           *m_sFirstName;
   unsigned short m_nLastNameLength;
   char           *m_sLastName;
   unsigned short m_nEmailLength;
   char           *m_sEmail;
   char           m_nAuthorization;
};


//-----UpdatePersonalExtInfo-------------------------------------------------------
class CPU_UpdatePersonalExtInfo : public CPacketUdp
{
public:
   CPU_UpdatePersonalExtInfo(const char *_sCity, unsigned short _nCountry,
                             const char *_sState, unsigned short _nAge,
                             char _cSex, const char *_sPhone,
                             const char *_sHomepage, const char *_sAbout);
   virtual ~CPU_UpdatePersonalExtInfo(void);
   virtual const unsigned short SubSequence(void)  { return m_nUpdateSequence; }

   const char *City(void)  { return m_sCity; }
   unsigned short Country(void)  { return m_nCountry; }
   const char *State(void)  { return m_sState; }
   unsigned short Age(void)  { return m_nAge; }
   char Sex(void)  { return m_cSex; }
   const char *PhoneNumber(void)  { return m_sPhone; }
   const char *Homepage(void)  { return m_sHomepage; }
   const char *About(void)  { return m_sAbout; }
protected:
   virtual unsigned long getSize(void);

   // ... PacketUdp header
   unsigned short m_nUpdateSequence;
   unsigned short m_nCityLength;
   char           *m_sCity;
   unsigned short m_nCountry;
   char           m_cCountryStatus;
   unsigned short m_nStateLength;
   char           *m_sState;
   unsigned short m_nAge;
   char           m_cSex;
   unsigned short m_nPhoneLength;
   char           *m_sPhone;
   unsigned short m_nHomepageLength;
   char           *m_sHomepage;
   unsigned short m_nAboutLength;
   char           *m_sAbout;
};


//-----Ping---------------------------------------------------------------------
class CPU_Ping : public CPacketUdp
{
public:
   CPU_Ping(void);
protected:
   virtual unsigned long getSize(void);

   // ... PacketUdp header
};


//-----ThroughServer------------------------------------------------------------
class CPU_ThroughServer : public CPacketUdp
{
public:
   CPU_ThroughServer(unsigned long _nSourceUin, unsigned long _nDestinationUin, 
                     unsigned short _nSubCommand, char *_sMessage);
   virtual ~CPU_ThroughServer(void);
protected:
   virtual unsigned long getSize(void);

   // ... PacketUdp header
   unsigned long  m_nDestinationUin;
   unsigned short m_nSubCommand;
   unsigned short m_nMsgLength;
   char           *m_sMessage;
};


//-----SetStatus----------------------------------------------------------------
class CPU_SetStatus : public CPacketUdp
{
public:
   CPU_SetStatus(unsigned long _nNewStatus);
protected:
   virtual unsigned long getSize(void);

   // ... PacketUdp header
   unsigned long m_nNewStatus;
};


//-----GetUserBasicInfo---------------------------------------------------------
class CPU_GetUserBasicInfo : public CPacketUdp
{
public:
   CPU_GetUserBasicInfo(unsigned long _nUserUin);
   virtual const unsigned short SubSequence(void)  { return m_nInfoSequence; }
protected:
   virtual unsigned long getSize(void);

   static unsigned short s_nInfoSequence;

   /* 02 00 60 04 B7 00 BA 95 47 00 0A 00 8F 76 20 00 */
   // ... PacketUdp header
   unsigned short m_nInfoSequence;
   unsigned long  m_nUserUin;
};


//-----GetUserExtInfo---------------------------------------------------------
class CPU_GetUserExtInfo : public CPacketUdp
{
public:
   CPU_GetUserExtInfo(unsigned long _nUserUin);
   virtual const unsigned short SubSequence(void)  { return m_nInfoSequence; }
protected:
   virtual unsigned long getSize(void);

   static unsigned short s_nInfoSequence;

   /* 02 00 60 04 B7 00 BA 95 47 00 0A 00 8F 76 20 00 */
   // ... PacketUdp header
   unsigned short m_nInfoSequence;
   unsigned long  m_nUserUin;
};


//-----Authorize----------------------------------------------------------------
class CPU_Authorize : public CPacketUdp
{
public:
   CPU_Authorize(unsigned long _nAuthorizeUin);
protected:
   virtual unsigned long getSize(void);

   /* 02 00 56 04 05 00 50 A5 82 00 A7 B8 19 00 08 00 01 00 00 */
   // ... PacketUdp header
   unsigned long m_nAuthorizeUin;
   char          m_aUnknown_1[5];
};

//-----RequestSysMsg------------------------------------------------------------
class CPU_RequestSysMsg : public CPacketUdp
{
public:
   CPU_RequestSysMsg(void);
protected:
   unsigned long getSize(void);
   /* 02 00 4C 04 02 00 50 A5 82 00 */
   // ... PacketUdp header
};


//-----SysMsgDoneAck------------------------------------------------------------
class CPU_SysMsgDoneAck : public CPacketUdp
{
public:
   CPU_SysMsgDoneAck(unsigned long _nSequence);
protected:
   unsigned long getSize(void);

   /* 02 00 42 04 04 00 50 A5 82 00 */
   // ... PacketUdp header
};


//=====TCP======================================================================

//-----PacketTcp_Handshake------------------------------------------------------
class CPacketTcp_Handshake : public CPacket
{
public:
   CPacketTcp_Handshake(unsigned long _nLocalPort);
   virtual ~CPacketTcp_Handshake(void);

   virtual const unsigned long  getSequence(void)   { return 0; }
   virtual const unsigned short SubSequence(void)   { return 0; }
   virtual const unsigned short getCommand(void)    { return m_cHandshakeCommand; }
   virtual const unsigned short getSubCommand(void) { return 0; }
   void Create(void) {};
protected:
   virtual unsigned long getSize(void) { return (26); };
   void initBuffer(void);

   /* FF 03 00 00 00 3D 62 00 00 50 A5 82 00 CF 60 AD 95 CF 60 AD 95 04 3D 62
      00 00 */
   char  m_cHandshakeCommand;
   unsigned long  m_nTcpVersion;
   unsigned long  m_nLocalPort;
   unsigned long  m_nSourceUin;
   unsigned long  m_nLocalHost;  // local ip
   //             m_nLocalHost;  // local real ip
   char           m_aUnknown_2;
   //             m_nLocalPort;
};


//-----CPacketTcp---------------------------------------------------------------
class CPacketTcp : public CPacket
{
public:
   virtual ~CPacketTcp(void);

   virtual const unsigned long  getSequence(void)   { return m_nSequence; }
   virtual const unsigned short SubSequence(void)   { return 0; }
   virtual const unsigned short getCommand(void)    { return m_nCommand; }
   virtual const unsigned short getSubCommand(void) { return m_nSubCommand; }
   virtual void Create(void);
protected:
   CPacketTcp(unsigned long _nSourceUin, unsigned long _nCommand,
              unsigned short _nSubCommand, const char *_sMessage, bool _bAccept,
              bool _bUrgent, ICQUser *_cUser);
   virtual unsigned long getSize(void);
   void initBuffer(void);
   void postBuffer(void);
   ICQUser *m_cUser;

   unsigned long  m_nSourceUin;
   unsigned short m_nTcpVersion;
   unsigned long  m_nCommand;
 //unsigned long  m_nSourceUin;
   unsigned short m_nSubCommand;
   unsigned short m_nMsgLength;
   char           *m_sMessage;
   unsigned long  m_nLocalIP;
   unsigned long  m_nLocalHost;
   unsigned long  m_nLocalPort;
   char           m_aUnknown_1;
   unsigned short m_nStatus;
   unsigned short m_nMsgType;
   // ...
   unsigned long  m_nSequence;
   char           m_sLicqTag;
   unsigned short m_nLicqVersion;
};


//-----Message------------------------------------------------------------------
class CPT_Message : public CPacketTcp
{
public:
   CPT_Message(unsigned long _nSourceUin, char *_sMessage, bool _bUrgent, ICQUser *_cUser);
   virtual void Create(void);
protected:
   virtual unsigned long getSize(void);
};


//-----Url----------------------------------------------------------------------
class CPT_Url : public CPacketTcp
{
public:
   CPT_Url(unsigned long _nSourceUin, char *_sMessage, bool _bUrgent, ICQUser *_cUser);
   virtual void Create(void);
protected:
   virtual unsigned long getSize(void);
   /* BA 95 47 00 03 00 EE 07 00 00 BA 95 47 00 04 00 24 00 67 6F 6F 64 20 70 6F
      72 6E 20 73 69 74 65 FE 68 74 74 70 3A 2F 2F 63 6F 6F 6C 70 6F 72 74 6E 2E
      63 6F 6D 00 81 61 1D 9E 7F 00 00 01 3F 07 00 00 04 00 00 10 00 03 00 00 00 */
};


//-----ReadAwayMessage----------------------------------------------------------
class CPT_ReadAwayMessage : public CPacketTcp
{
public:
   CPT_ReadAwayMessage(unsigned long _nSourceUin, ICQUser *_cUser);
   virtual void Create(void);
protected:
   virtual unsigned long getSize(void);
   /* 76 1E 3F 00 03 00 EE 07 00 00 76 1E 3F 00 E8 03 01 00 00 81 61 1D 9D 81 61
      1D 9D C9 05 00 00 04 00 00 10 00 FE FF FF FF */
};


//-----ChatRequest--------------------------------------------------------------
class CPT_ChatRequest : public CPacketTcp
{
public:
   CPT_ChatRequest(unsigned long _nSourceUin, char *_sMessage, bool _bUrgent, ICQUser *_cUser);
   virtual void Create(void);
protected:
   virtual unsigned long getSize(void);
   /* 50 A5 82 00 03 00 EE 07 00 00 50 A5 82 00 02 00 0D 00 63 68 61 74 20 72
      65 71 75 65 73 74 00 CF 60 AD D3 CF 60 AD D3 28 12 00 00 04 00 00 10 00
      01 00 00 00 00 00 00 00 00 00 00 06 00 00 00 */
   char m_aUnknown_1[11];
};


//-----FileTransfer-------------------------------------------------------------
class CPT_FileTransfer : public CPacketTcp
{
public:
   CPT_FileTransfer(unsigned long _nSourceUin, const char *_szFilename,
                    const char *_szDescription, bool _bUrgent, ICQUser *_cUser);
   virtual void Create(void);
   bool IsValid(void)  { return m_bValid; };
   const char *GetFilename(void)  { return m_szFilename; };
   const char *GetDescription(void)  { return m_sMessage; };
   unsigned long GetFileSize(void)  { return m_nFileSize; };
protected:
   virtual unsigned long getSize(void);
   bool m_bValid;

   /* 50 A5 82 00 03 00 EE 07 00 00 50 A5 82 00 03 00 0F 00 74 68 69 73 20 69
      73 20 61 20 66 69 6C 65 00 CF 60 AD D3 CF 60 AD D3 60 12 00 00 04 00 00
      10 00 00 00 00 00 09 00 4D 61 6B 65 66 69 6C 65 00 55 0C 00 00 00 00 00
      00 04 00 00 00 */
   // TCP header stuff
   unsigned long  m_nUnknown_1;
   unsigned short m_nFilenameLength;
   char           *m_szFilename;
   unsigned long  m_nFileSize;
   unsigned long  m_nUnknown_2;
};


//++++Ack+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CPT_Ack : public CPacketTcp
{
protected:
  CPT_Ack(unsigned short _nSubCommand, unsigned long _nSequence,
          bool _bAccept, ICQUser *_cUser);
  virtual unsigned long getSize(void);

  /* 50 A5 82 00 03 00 DA 07 00 00 50 A5 82 00 01 00 01 00 00 CF 60 AD D3 CF
     60 AD D3 60 12 00 00 04 00 00 00 00 02 00 00 00 */
};


//-----AckMessage---------------------------------------------------------------
class CPT_AckMessage : public CPT_Ack
{
public:
  CPT_AckMessage(unsigned long _nSequence, bool _bAccept, ICQUser *_cUser);
protected:
  virtual unsigned long getSize(void);
  /* 8F 76 20 00 03 00 DA 07 00 00 8F 76 20 00 01 00 01 00 00 CF 60 AD D3 7F
     00 00 01 5A 12 00 00 04 00 00 00 00 14 00 00 00 */
};


//-----AckReadAwayMessage-------------------------------------------------------
class CPT_AckReadAwayMessage : public CPT_Ack
{
public:
   CPT_AckReadAwayMessage(unsigned short _nSubCommand, unsigned long _nSequence,
                          bool _bAccept, ICQUser *_cUser);
protected:
   virtual unsigned long getSize(void);
};


//-----AckUrl-------------------------------------------------------------------
class CPT_AckUrl : public CPT_Ack
{
public:
  CPT_AckUrl(unsigned long _nSequence, bool _bAccept, ICQUser *_cUser);
protected:
   virtual unsigned long getSize(void);
};


//-----AckContactList----------------------------------------------------------
class CPT_AckContactList : public CPT_Ack
{
public:
  CPT_AckContactList(unsigned long _nSequence, bool _bAccept, ICQUser *_cUser);
protected:
  virtual unsigned long getSize(void);
};


//-----AckChatRefuse------------------------------------------------------------
class CPT_AckChatRefuse : public CPT_Ack
{
public:
  CPT_AckChatRefuse(const char *_sReason, unsigned long _nSequence, ICQUser *_cUser);
protected:
   virtual unsigned long getSize(void);

   /* 50 A5 82 00 03 00 DA 07 00 00 50 A5 82 00 02 00 03 00 6E 6F 00 CF 60 AD
      95 CF 60 AD 95 1E 3C 00 00 04 01 00 00 00 01 00 00 00 00 00 00 00 00 00
      00 01 00 00 00 */
   char m_aUnknown_1[11];
};


//-----AckChatAccept------------------------------------------------------------
class CPT_AckChatAccept : public CPT_Ack
{
public:
  CPT_AckChatAccept(unsigned short _nPort, unsigned long _nSequence,
                    ICQUser *_cUser);
protected:
   virtual unsigned long getSize(void);

   /* 50 A5 82 00 03 00 DA 07 00 00 50 A5 82 00 02 00 01 00 00 CF 60 AD 95 CF
      60 AD 95 1E 3C 00 00 04 00 00 00 00 01 00 00 40 78 00 00 78 40 00 00 02
      00 00 00 */
   char          m_aUnknown_1[3];  // actually a 0 length string
   unsigned long m_nPortReversed;
   unsigned long m_nPort;
};



//-----AckFileAccept------------------------------------------------------------
class CPT_AckFileAccept : public CPT_Ack
{
public:
  CPT_AckFileAccept(unsigned short _nPort, unsigned long _nSequence,
                    ICQUser *_cUser);
protected:
   virtual unsigned long getSize(void);

   /* 50 A5 82 00 03 00 DA 07 00 00 50 A5 82 00 03 00 01 00 00 D1 EF 04 9F 7F
      00 00 01 4A 1F 00 00 04 00 00 00 00 20 3A 00 00 01 00 00 00 00 00 00 3A
      20 00 00 05 00 00 00 */

   unsigned long  m_nPortReversed;
   unsigned short m_nStrLength;
   char           m_cEmptyStr;
   unsigned long  m_nFileSize;  // not used in the ack
   unsigned long  m_nPort;

};


//-----AckFileRefuse------------------------------------------------------------
class CPT_AckFileRefuse : public CPT_Ack
{
public:
  CPT_AckFileRefuse(const char *_sReason, unsigned long _nSequence, ICQUser *_cUser);
protected:
   virtual unsigned long getSize(void);

   /* 50 A5 82 00 03 00 DA 07 00 00 50 A5 82 00 03 00 0A 00 6E 6F 20 74 68 61
      6E 6B 73 00 D1 EF 04 9F 7F 00 00 01 4A 1F 00 00 04 01 00 00 00 00 00 00
      00 01 00 00 00 00 00 00 00 00 00 00 03 00 00 00 */

   unsigned long  m_nUnknown_1;
   unsigned short m_nStrLen;
   char           m_cEmptyStr;
   unsigned long  m_nUnknown_2;
   unsigned long  m_nUnknown_3;
};


//++++Cancel++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CPT_Cancel : public CPacketTcp
{
protected:
   CPT_Cancel(unsigned short _nSubCommand, unsigned long _nSequence,
              ICQUser *_cUser);
   virtual unsigned long getSize(void);
};


//-----ChatCancel---------------------------------------------------------------
class CPT_CancelChat : public CPT_Cancel
{
public:
   CPT_CancelChat(unsigned long _nSequence, ICQUser *_cUser);
protected:
   virtual unsigned long getSize(void);

   /* 50 A5 82 00 03 00 D0 07 00 00 50 A5 82 00 02 00 01 00 00 CF 60 AD D3 CF
      60 AD D3 28 12 00 00 04 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 06
      00 00 00 */
   char m_aUnknown_1[11];
};


//-----FileCancel---------------------------------------------------------------
class CPT_CancelFile : public CPT_Cancel
{
public:
   CPT_CancelFile(unsigned long _nSequence, ICQUser *_cUser);
protected:
   virtual unsigned long getSize(void);

   /* 50 A5 82 00 03 00 D0 07 00 00 50 A5 82 00 02 00 01 00 00 CF 60 AD D3 CF
      60 AD D3 28 12 00 00 04 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 06
      00 00 00 */
   unsigned long  m_nUnknown_1;
   unsigned short m_nStrLen;
   char           m_cEmptyStr;
   unsigned long  m_nUnknown_2;
   unsigned long  m_nUnknown_3;
};


//=====Chat=====================================================================
class CPacketChat : public CPacket
{
public:
  virtual const unsigned long  getSequence(void)   { return 0; };
  virtual const unsigned short SubSequence(void)   { return 0; };
  virtual const unsigned short getCommand(void)    { return 0; };
  virtual const unsigned short getSubCommand(void) { return 0; };
  virtual void Create(void) {};
protected:
   virtual unsigned long getSize(void)        { return 0; };
   void initBuffer(void);
};


//-----ChatColor----------------------------------------------------------------
class CPChat_Color : public CPacketChat  // First info packet after handshake
{
public:
   CPChat_Color(char *_sLocalName, unsigned short _nLocalPort,
                unsigned long _nColorForeground, unsigned long _nColorBackground);
protected:
   virtual unsigned long getSize(void)
      { return (CPacketChat::getSize() + 10 + m_nLocalNameLength + 15); };

   /* 64 00 00 00 FD FF FF FF 50 A5 82 00 08 00 38 35 36 32 30 30 30 00 62 3D
      FF FF FF 00 00 00 00 00 00 */
   unsigned long  m_nCommand;
   char           m_aUnknown_1[4];
   unsigned long  m_nSourceUin;
   unsigned short m_nLocalNameLength;
   char           *m_sLocalName;
   char           m_aLocalPortReversed[2];
   unsigned long  m_nColorForeground;  // rr gg bb 00
   unsigned long  m_nColorBackground;  // rr gg bb 00
   char           m_aUnknown_2;
};


//-----ChatColorFont------------------------------------------------------------
class CPChat_ColorFont : public CPacketChat  // Second info packet after handshake
{
public:
   CPChat_ColorFont(char *_sLocalName, unsigned short _nLocalPort,
                    unsigned long _nColorForeground,
                    unsigned long _nColorBackground,
                    unsigned long _nFontSize, unsigned long _nFontFace,
                    char *_sFontName);
protected:
   virtual unsigned long getSize(void)
      { return (CPacketChat::getSize() + 10 + m_nLocalNameLength + 34 +
                m_nFontNameLength + 3); };

   /* 64 00 00 00 50 A5 82 00 08 00 38 35 36 32 30 30 30 00 FF FF FF 00 00 00
      00 00 03 00 00 00 DB 64 00 00 CF 60 AD 95 CF 60 AD 95 04 75 5A 0C 00 00
      00 00 00 00 00 08 00 43 6F 75 72 69 65 72 00 00 00 00 */
   unsigned long  m_nCommand;
   unsigned long  m_nSourceUin;
   unsigned short m_nLocalNameLength;
   char           *m_sLocalName;
   unsigned long  m_nColorForeground;  // rr gg bb 00
   unsigned long  m_nColorBackground;  // rr gg bb 00
   unsigned long  m_nUnknown_1;
   unsigned long  m_nLocalPort;
   unsigned long  m_nLocalHost;
   //             m_nLocalHost;
   char           m_aUnknown_2;
   unsigned short m_nUnknown_Port;
   unsigned long  m_nFontSize;
   unsigned long  m_nFontFace;
   unsigned short m_nFontNameLength;
   char           *m_sFontName;
   char           m_aUnknown_3[3];
};


//-----ChatFont-----------------------------------------------------------------
class CPChat_Font : public CPacketChat  // First info packet after handshake
{
public:
   CPChat_Font(unsigned short _nLocalPort, unsigned long _nFontSize,
               unsigned long _nFontFace, char *_sFontName);
protected:
   virtual unsigned long getSize(void)
      { return (CPacketChat::getSize() + 29 + m_nFontNameLength + 3); };

   /* 03 00 00 00 83 72 00 00 CF 60 AD 95 CF 60 AD 95 04 54 72 0C 00 00 00 00
      00 00 00 08 00 43 6F 75 72 69 65 72 00 00 00 */
   unsigned long  m_nCommand;
   unsigned long  m_nLocalPort;
   unsigned long  m_nLocalHost;
   //             m_nLocalHost;
   char           m_aUnknown_2;
   unsigned short m_nUnknown_Port;
   unsigned long  m_nFontSize;
   unsigned long  m_nFontFace;
   unsigned short m_nFontNameLength;
   char           *m_sFontName;
   char           m_aUnknown_3[3];
};

//=====File=====================================================================
class CPacketFile : public CPacket
{
public:
  virtual const unsigned long  getSequence(void)   { return 0; };
  virtual const unsigned short SubSequence(void)   { return 0; };
  virtual const unsigned short getCommand(void)    { return 0; };
  virtual const unsigned short getSubCommand(void) { return 0; };
  virtual void Create(void) {};
protected:
   virtual unsigned long getSize(void)        { return 0; };
   void initBuffer(void)   { buffer = new CBuffer(getSize()); };
};

//-----File_InitClient----------------------------------------------------------
class CPFile_InitClient : public CPacketFile
{
public:
   CPFile_InitClient(char *_szLocalName, unsigned long _nNumFiles,
                     unsigned long _nTotalSize);
   virtual ~CPFile_InitClient(void)
      { free (m_szLocalName); };
protected:
   virtual unsigned long getSize(void)
      { return (CPacketFile::getSize() + 19 + m_nLocalNameLength); };

   /* 00 00 00 00 00 01 00 00 00 45 78 00 00 64 00 00 00 08 00 38 35 36 32 30
      30 30 00 */
   char           m_cUnknown1;
   unsigned long  m_nUnknown2;
   unsigned long  m_nNumFiles;
   unsigned long  m_nTotalSize;
   unsigned long  m_nUnknown3;
   unsigned short m_nLocalNameLength;
   char           *m_szLocalName;
};


//-----File_InitServer----------------------------------------------------------
class CPFile_InitServer : public CPacketFile
{
public:
   CPFile_InitServer(char *_szLocalName);
   virtual ~CPFile_InitServer(void)
      { free (m_szLocalName); };
protected:
   virtual unsigned long getSize(void)
      { return (CPacketFile::getSize() + 7 + m_nLocalNameLength); };

   /* 01 64 00 00 00 08 00 38 35 36 32 30 30 30 00 */
   char           m_cUnknown1;
   unsigned long  m_nUnknown2;
   unsigned short m_nLocalNameLength;
   char           *m_szLocalName;
};


//-----File_Info---------------------------------------------------------------
class CPFile_Info : public CPacketFile
{
public:
   CPFile_Info(const char *_szFileName);
   virtual ~CPFile_Info(void);
   bool IsValid(void)  { return m_bValid; };
   unsigned long GetFileSize(void)
     { return m_nFileSize; };
   const char *GetFileName(void)
     { return m_szFileName; }
   const char *ErrorStr(void)
     { return strerror(m_nError); }
protected:
   virtual unsigned long getSize(void);

   bool m_bValid;
   int m_nError;
   /* 02 00 0D 00 63 75 72 72 65 6E 74 2E 64 69 66 66 00 01 00 00 45 78 00 00
      00 00 00 00 64 00 00 00 */
   unsigned short m_nPacketId;
   char *m_szFileName;
   unsigned short m_nUnknown1;
   char m_cUnknown2;
   unsigned long m_nFileSize;
   unsigned long m_nUnknown3;
   unsigned long m_nUnknown4;
};


//-----File_Start---------------------------------------------------------------
class CPFile_Start : public CPacketFile
{
public:
   CPFile_Start(unsigned long _nFilePos);
protected:
   virtual unsigned long getSize(void)
      { return 13; };

   /* 03 00 00 00 00 00 00 00 00 64 00 00 00 */
   char m_cUnknown1;
   unsigned long m_nFilePos;
   unsigned long m_nUnknown2;
   unsigned long m_nUnknown3;
};


#endif
