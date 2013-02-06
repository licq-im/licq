/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2013 Licq developers <licq-dev@googlegroups.com>
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

#ifndef LICQICQ_PACKET_TCP_H
#define LICQICQ_PACKET_TCP_H

#include <cstring>
#include <list>
#include <string>

#include <licq/buffer.h>
#include <licq/userid.h>
#include <licq/packet.h>

#include "chat.h"
#include "defines.h"

namespace Licq
{
class Color;
class INetSocket;
}

namespace LicqIcq
{

typedef Licq::Buffer CBuffer;
typedef Licq::Packet CPacket;

// values of extra info to identify plugin request
const unsigned short DirectInfoPluginRequest     = 1;
const unsigned short DirectStatusPluginRequest   = 2;


//=====TCP======================================================================
bool Decrypt_Client(Licq::Buffer* pkt, unsigned long version);
void Encrypt_Client(Licq::Buffer* pkt, unsigned long version);

//-----PacketTcp_Handshake------------------------------------------------------
class CPacketTcp_Handshake : public CPacket
{
public:
  ~CPacketTcp_Handshake();

  virtual Licq::Buffer* getBuffer() { return buffer; }

  virtual unsigned short Sequence()   { return 0; }
  virtual unsigned short SubSequence()   { return 0; }
  virtual unsigned short SubCommand() { return 0; }

protected:
  CPacketTcp_Handshake();

  Licq::Buffer* buffer;
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

  virtual Licq::Buffer* getBuffer() { return buffer; }

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
      User* user);
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

  Licq::Buffer* buffer;
};


//-----Message------------------------------------------------------------------
class CPT_Message : public CPacketTcp
{
public:
   CPT_Message(const std::string& message, unsigned short nLevel, bool bMR,
      const Licq::Color* pColor, User* pUser, bool isUtf8);
};


//-----Url----------------------------------------------------------------------
/* BA 95 47 00 03 00 EE 07 00 00 BA 95 47 00 04 00 24 00 67 6F 6F 64 20 70 6F
   72 6E 20 73 69 74 65 FE 68 74 74 70 3A 2F 2F 63 6F 6F 6C 70 6F 72 74 6E 2E
   63 6F 6D 00 81 61 1D 9E 7F 00 00 01 3F 07 00 00 04 00 00 10 00 03 00 00 00 */
class CPT_Url : public CPacketTcp
{
public:
  CPT_Url(const std::string& message, unsigned short nLevel, bool bMR,
      const Licq::Color* pColor, User *pUser);
};


class CPT_ContactList : public CPacketTcp
{
public:
  CPT_ContactList(const std::string& message, unsigned short nLevel, bool bMR,
      const Licq::Color* pColor, User* pUser);
};




//-----ReadAwayMessage----------------------------------------------------------
class CPT_ReadAwayMessage : public CPacketTcp
{
public:
  CPT_ReadAwayMessage(User* _cUser);
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
      unsigned short nLevel, User* pUser, bool bICBM);
};


//-----FileTransfer-------------------------------------------------------------
class CPT_FileTransfer : public CPacketTcp
{
public:
  CPT_FileTransfer(const std::list<std::string>& lFileList, const std::string& filename,
      const std::string& description, unsigned short nLevel, User* pUser);
  const std::string& description() { return myMessage; }

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

   /* 50 A5 82 00 03 00 EE 07 00 00 50 A5 82 00 03 00 0F 00 74 68 69 73 20 69
      73 20 61 20 66 69 6C 65 00 CF 60 AD D3 CF 60 AD D3 60 12 00 00 04 00 00
      10 00 00 00 00 00 09 00 4D 61 6B 65 66 69 6C 65 00 55 0C 00 00 00 00 00
      00 04 00 00 00 */
};


//-----OpenSecureChannel------------------------------------------------------------
class CPT_OpenSecureChannel : public CPacketTcp
{
public:
  CPT_OpenSecureChannel(User* pUser);
};


class CPT_CloseSecureChannel : public CPacketTcp
{
public:
  CPT_CloseSecureChannel(User* pUser);
};




//++++Ack++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/* 50 A5 82 00 03 00 DA 07 00 00 50 A5 82 00 01 00 01 00 00 CF 60 AD D3 CF
   60 AD D3 60 12 00 00 04 00 00 00 00 02 00 00 00 */
class CPT_Ack : public CPacketTcp
{
protected:
  CPT_Ack(unsigned short _nSubCommand, unsigned short _nSequence,
      bool _bAccept, bool _bUrgent, User* _cUser);
  virtual ~CPT_Ack();
};


//-----AckGeneral------------------------------------------------------------
class CPT_AckGeneral : public CPT_Ack
{
public:
  CPT_AckGeneral(unsigned short nSubCommand, unsigned short nSequence,
      bool bAccept, bool bUrgent, User* pUser);
};


//-----AckKey------------------------------------------------------------
class CPT_AckOldSecureChannel : public CPT_Ack
{
public:
  CPT_AckOldSecureChannel(unsigned short nSequence, User* pUser);
};


//-----AckKey------------------------------------------------------------
class CPT_AckOpenSecureChannel : public CPT_Ack
{
public:
  CPT_AckOpenSecureChannel(unsigned short nSequence, bool ok, User* pUser);
};


//-----AckKey------------------------------------------------------------
class CPT_AckCloseSecureChannel : public CPT_Ack
{
public:
  CPT_AckCloseSecureChannel(unsigned short nSequence, User* pUser);
};


#if 0
//-----AckMessage------------------------------------------------------------
class CPT_AckMessage : public CPT_Ack
{
public:
  CPT_AckMessage(unsigned short _nSequence, bool _bAccept, bool _bUrgent, User* _cUser);
};


//-----AckReadAwayMessage-------------------------------------------------------
class CPT_AckReadAwayMessage : public CPT_Ack
{
public:
   CPT_AckReadAwayMessage(unsigned short _nSubCommand, unsigned short _nSequence,
      bool _bAccept, User* _cUser);
};


//-----AckUrl-------------------------------------------------------------------
class CPT_AckUrl : public CPT_Ack
{
public:
  CPT_AckUrl(unsigned short _nSequence, bool _bAccept, bool _bUrgent, User* _cUser);
};


//-----AckContactList----------------------------------------------------------
class CPT_AckContactList : public CPT_Ack
{
public:
  CPT_AckContactList(unsigned short _nSequence, bool _bAccept, bool _bUrgent,
      User* _cUser);
};
#endif

//-----AckChatRefuse------------------------------------------------------------
class CPT_AckChatRefuse : public CPT_Ack
{
public:
  CPT_AckChatRefuse(const std::string& reason, unsigned short _nSequence, User* _cUser);
   /* 50 A5 82 00 03 00 DA 07 00 00 50 A5 82 00 02 00 03 00 6E 6F 00 CF 60 AD
      95 CF 60 AD 95 1E 3C 00 00 04 01 00 00 00 01 00 00 00 00 00 00 00 00 00
      00 01 00 00 00 */
};


//-----AckChatAccept------------------------------------------------------------
class CPT_AckChatAccept : public CPT_Ack
{
public:
  CPT_AckChatAccept(unsigned short _nPort, const std::string& clients,
      unsigned short _nSequence, User* _cUser, bool bICBM);
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
      User* _cUser);
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
  CPT_AckFileRefuse(const std::string& reason, unsigned short _nSequence, User* _cUser);
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
      User* _cUser);
};


//-----ChatCancel---------------------------------------------------------------
class CPT_CancelChat : public CPT_Cancel
{
public:
  CPT_CancelChat(unsigned short _nSequence, User* _cUser);
   /* 50 A5 82 00 03 00 D0 07 00 00 50 A5 82 00 02 00 01 00 00 CF 60 AD D3 CF
      60 AD D3 28 12 00 00 04 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 06
      00 00 00 */
};


//-----FileCancel---------------------------------------------------------------
class CPT_CancelFile : public CPT_Cancel
{
public:
  CPT_CancelFile(unsigned short _nSequence, User* _cUser);
   /* 50 A5 82 00 03 00 D0 07 00 00 50 A5 82 00 02 00 01 00 00 CF 60 AD D3 CF
      60 AD D3 28 12 00 00 04 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 06
      00 00 00 */
};

//----Send error reply----------------------------------------------------------
class CPT_PluginError : public CPacketTcp
{
public:
  CPT_PluginError(User* _cUser, unsigned short nSequence,
      int channel);
};

//----Request info plugin list--------------------------------------------------
class CPT_InfoPluginReq : public CPacketTcp
{
public:
  CPT_InfoPluginReq(User* _cUser, const uint8_t* GUID, unsigned long int nTime);
   virtual const char *RequestGUID()        { return m_ReqGUID; }
   virtual unsigned short ExtraInfo() { return DirectInfoPluginRequest; }

protected:
   char m_ReqGUID[GUID_LENGTH];
};

//----Response to phone book request--------------------------------------------
class CPT_InfoPhoneBookResp : public CPacketTcp
{
public:
  CPT_InfoPhoneBookResp(User* _cUser, unsigned short nSequence);
};

//-----Response to picture request----------------------------------------------
class CPT_InfoPictureResp : public CPacketTcp
{
public:
  CPT_InfoPictureResp(User* _cUser, unsigned short nSequence);
};

//----Response to info plugin list request--------------------------------------
class CPT_InfoPluginListResp : public CPacketTcp
{
public:
  CPT_InfoPluginListResp(User* _cUser, unsigned short nSequence);
};

//----Send status plugin request------------------------------------------------
class CPT_StatusPluginReq : public CPacketTcp
{
public:
  CPT_StatusPluginReq(User* _cUser, const uint8_t* GUID, unsigned long nTime);
   virtual unsigned short ExtraInfo() { return DirectStatusPluginRequest;}
   virtual const char *RequestGUID() { return m_ReqGUID; }

protected:
  char m_ReqGUID[GUID_LENGTH];
};

//----Response to status plugin list request------------------------------------
class CPT_StatusPluginListResp : public CPacketTcp
{
public:
  CPT_StatusPluginListResp(User* _cUser, unsigned short nSequence);
};

//----Response to status request------------------------------------------------
class CPT_StatusPluginResp : public CPacketTcp
{
public:
  CPT_StatusPluginResp(User* _cUser, unsigned short nSequence,
                       unsigned long nStatus);
};


//=====File=====================================================================
class CPacketFile : public Licq::Packet
{
public:
  CPacketFile();
  virtual ~CPacketFile();

  virtual Licq::Buffer* getBuffer() { return buffer; }

  virtual unsigned short Sequence()    { return 0; };
  virtual unsigned short SubSequence() { return 0; };
  virtual unsigned short SubCommand()  { return 0; };
protected:
   void InitBuffer()   { buffer = new CBuffer(m_nSize); };

  Licq::Buffer* buffer;
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
  ~CPacketChat();

  virtual Licq::Buffer* getBuffer() { return buffer; }

  virtual unsigned short Sequence()   { return 0; };
  virtual unsigned short SubSequence()   { return 0; };
  virtual unsigned short SubCommand() { return 0; };
protected:
  CPacketChat();
   void InitBuffer();

  Licq::Buffer* buffer;
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
  unsigned long uin() const { return myUin; }
  unsigned short Port() { return m_nPort; }
  int ColorForeRed() { return m_nColorForeRed; }
  int ColorForeGreen() { return m_nColorForeGreen; }
  int ColorForeBlue() { return m_nColorForeBlue; }
  int ColorBackRed() { return m_nColorBackRed; }
  int ColorBackGreen() { return m_nColorBackGreen; }
  int ColorBackBlue() { return m_nColorBackBlue; }

  virtual ~CPChat_Color();

protected:
  unsigned long myUin;
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
  unsigned long uin() const { return myUin; }
  unsigned short Session() { return m_nSession; }
  int ColorForeRed() { return m_nColorForeRed; }
  int ColorForeGreen() { return m_nColorForeGreen; }
  int ColorForeBlue() { return m_nColorForeBlue; }
  int ColorBackRed() { return m_nColorBackRed; }
  int ColorBackGreen() { return m_nColorBackGreen; }
  int ColorBackBlue() { return m_nColorBackBlue; }
  unsigned short Port() { return m_nPort; }
  unsigned long FontSize() { return m_nFontSize; }
  bool FontBold() { return m_nFontFace & Licq::FONT_BOLD; }
  bool FontItalic() { return m_nFontFace & Licq::FONT_ITALIC; }
  bool FontUnderline() { return m_nFontFace & Licq::FONT_UNDERLINE; }
  bool FontStrikeOut() { return m_nFontFace & Licq::FONT_STRIKEOUT; }
  unsigned long FontFace() { return m_nFontFace; }
  const std::string& fontFamily() const { return myFontFamily; }
  unsigned char FontEncoding() { return m_nFontEncoding; }
  unsigned char FontStyle() { return m_nFontStyle; }
  ChatClientList &ChatClients()  { return chatClients; }

protected:
  unsigned long myUin;
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
  bool FontBold() { return m_nFontFace & Licq::FONT_BOLD; }
  bool FontItalic() { return m_nFontFace & Licq::FONT_ITALIC; }
  bool FontUnderline() { return m_nFontFace & Licq::FONT_UNDERLINE; }
  bool FontStrikeOut() { return m_nFontFace & Licq::FONT_STRIKEOUT; }
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

  bool FontBold() { return m_nFontFace & Licq::FONT_BOLD; }
  bool FontItalic() { return m_nFontFace & Licq::FONT_ITALIC; }
  bool FontUnderline() { return m_nFontFace & Licq::FONT_UNDERLINE; }
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

} // namespace LicqIcq

#endif
