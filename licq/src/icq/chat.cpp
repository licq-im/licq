// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998-2011 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

#include "config.h"

#include <licq/icqchat.h>

#include <cctype>
#include <cstdio>
#include <cstring>
#include <unistd.h>

#include <licq/logging/log.h>
#include <licq/contactlist/owner.h>
#include <licq/contactlist/usermanager.h>
#include <licq/daemon.h>
#include <licq/translator.h>

#include "icq/icq.h"
#include "icq/packet.h"
#include "support.h"

// Localization
#include "gettext.h"

using namespace std;
using Licq::gDaemon;
using Licq::gLog;

#define MAX_CONNECTS  256
#define DEBUG_THREADS(x)


const unsigned short CHAT_STATE_DISCONNECTED = 0;
const unsigned short CHAT_STATE_HANDSHAKE = 1;
const unsigned short CHAT_STATE_WAITxFORxCOLOR = 2;
const unsigned short CHAT_STATE_WAITxFORxCOLORxFONT = 3;
const unsigned short CHAT_STATE_WAITxFORxFONT = 4;
const unsigned short CHAT_STATE_CONNECTED = 5;


//---Chat-----------------------------------------------------------------------
void CPacketChat::InitBuffer()
{
  buffer = new CBuffer(m_nSize);
}

//-----ChatColor----------------------------------------------------------------
CPChat_Color::CPChat_Color(const string& localName, unsigned short _nLocalPort,
   int nColorForeRed, int nColorForeGreen, int nColorForeBlue, int nColorBackRed,
   int nColorBackBlue, int nColorBackGreen)
{
  m_nPort = _nLocalPort;
  myUserId = Licq::gUserManager.ownerUserId(LICQ_PPID);
  unsigned long m_nUin = atol(myUserId.accountId().c_str());
  m_nColorForeRed = nColorForeRed;
  m_nColorForeGreen = nColorForeGreen;
  m_nColorForeBlue = nColorForeBlue;
  m_nColorBackRed = nColorBackRed;
  m_nColorBackGreen = nColorBackGreen;
  m_nColorBackBlue = nColorBackBlue;

  m_nSize = 10 + localName.size() + 16;
  InitBuffer();

  buffer->PackUnsignedLong(0x65);
  buffer->PackUnsignedLong(-ICQ_VERSION_TCP);
  buffer->PackUnsignedLong(m_nUin);
  buffer->PackString(localName.c_str());
  buffer->PackUnsignedShort(ReversePort(_nLocalPort));
  buffer->PackChar(nColorForeRed);
  buffer->PackChar(nColorForeGreen);
  buffer->PackChar(nColorForeBlue);
  buffer->PackChar(0);
  buffer->PackChar(nColorBackRed);
  buffer->PackChar(nColorBackGreen);
  buffer->PackChar(nColorBackBlue);
  buffer->PackChar(0);
  buffer->PackChar(0);
}


CPChat_Color::CPChat_Color(CBuffer &b)
{
  b.UnpackUnsignedLong();
  b.UnpackUnsignedLong();
  unsigned long m_nUin = b.UnpackUnsignedLong();
  char szUin[24];
  sprintf(szUin, "%lu", m_nUin);
  myUserId = Licq::UserId(szUin, LICQ_PPID);
  myName = b.unpackString();
  m_nPort = b.UnpackUnsignedShort();
  m_nPort = (m_nPort >> 8) + (m_nPort << 8);
  m_nColorForeRed = (unsigned char)b.UnpackChar();
  m_nColorForeGreen = (unsigned char)b.UnpackChar();
  m_nColorForeBlue = (unsigned char)b.UnpackChar();
  b.UnpackChar();
  m_nColorBackRed = (unsigned char)b.UnpackChar();
  m_nColorBackGreen = (unsigned char)b.UnpackChar();
  m_nColorBackBlue = (unsigned char)b.UnpackChar();
  b.UnpackChar();
}

CPChat_Color::~CPChat_Color()
{
  // Empty
}


//-----ChatColorFont----------------------------------------------------------------
CChatClient::CChatClient()
{
  m_nVersion = m_nIp = m_nIntIp = m_nPort = m_nMode
     = m_nSession = m_nHandshake = 0;
}


CChatClient::CChatClient(const Licq::User* u)
{
  m_nVersion = u->Version();
  myUserId = u->id();
  m_nIp = u->Ip();
  m_nIntIp = u->IntIp();
  m_nMode = u->Mode();
  m_nSession = 0;
  m_nHandshake = 0x65;

  // These will still need to be set
  m_nPort = 0;
  m_nSession = 0;
}


CChatClient::CChatClient(CBuffer &b)
{
  LoadFromBuffer(b);
}

CChatClient::CChatClient(const CChatClient &p)
{
  *this = p;
}

CChatClient::~CChatClient()
{
}

CChatClient& CChatClient::operator=(const CChatClient &p)
{
  if (this != &p)
  {
    m_nVersion = p.m_nVersion;
    m_nPort = p.m_nPort;
    myUserId = p.myUserId;
    m_nIp = p.m_nIp;
    m_nIntIp = p.m_nIntIp;
    m_nMode = p.m_nMode;
    m_nSession = p.m_nSession;
    m_nHandshake = p.m_nHandshake;
  }
  return *this;
}

bool CChatClient::LoadFromBuffer(CBuffer &b)
{
  m_nVersion = b.UnpackUnsignedLong();
  m_nPort = b.UnpackUnsignedShort();
  b.UnpackUnsignedShort();
  unsigned long m_nUin = b.UnpackUnsignedLong();
  char szUin[24];
  sprintf(szUin, "%lu", m_nUin);
  myUserId = Licq::UserId(szUin, LICQ_PPID);
  m_nIp = b.UnpackUnsignedLong();
  m_nIntIp = b.UnpackUnsignedLong();
  m_nMode = b.UnpackChar();
  b.UnpackUnsignedShort();
  m_nSession = b.UnpackUnsignedShort();
  m_nHandshake = b.UnpackUnsignedLong();

  return true;
}


bool CChatClient::LoadFromHandshake_v2(CBuffer &b)
{
  b.Reset();

  if ((unsigned char)b.UnpackChar() != ICQ_CMDxTCP_HANDSHAKE) return false;

  m_nVersion = b.UnpackUnsignedLong();
  b.UnpackUnsignedLong();
  unsigned long m_nUin = b.UnpackUnsignedLong();
  char szUin[24];
  sprintf(szUin, "%lu", m_nUin);
  myUserId = Licq::UserId(szUin, LICQ_PPID);
  m_nIp = b.UnpackUnsignedLong();
  m_nIntIp = b.UnpackUnsignedLong();
  m_nMode = b.UnpackChar();
  m_nHandshake = 0x64;

  // These will still need to be set
  m_nPort = 0;
  m_nSession = 0;

  return true;
}


bool CChatClient::LoadFromHandshake_v4(CBuffer &b)
{
  b.Reset();

  if ((unsigned char)b.UnpackChar() != ICQ_CMDxTCP_HANDSHAKE) return false;

  m_nVersion = b.UnpackUnsignedLong();
  b.UnpackUnsignedLong();
  unsigned long m_nUin = b.UnpackUnsignedLong();
  char szUin[24];
  sprintf(szUin, "%lu", m_nUin);
  myUserId = Licq::UserId(szUin, LICQ_PPID);
  m_nIp = b.UnpackUnsignedLong();  // Will probably be zero...
  m_nIntIp = b.UnpackUnsignedLong();
  m_nMode = b.UnpackChar();
  m_nHandshake = 0x64;

  // These will still need to be set
  m_nPort = 0;
  m_nSession = 0;

  return true;
}


bool CChatClient::LoadFromHandshake_v6(CBuffer &b)
{
  CPacketTcp_Handshake_v6 hand(&b);

  m_nVersion = hand.VersionMajor();
  unsigned long m_nUin = hand.SourceUin();
  char szUin[24];
  sprintf(szUin, "%lu", m_nUin);
  myUserId = Licq::UserId(szUin, LICQ_PPID);
  m_nIntIp = hand.LocalIp();
  m_nIp = hand.RealIp();
  m_nMode = hand.Mode();
  m_nHandshake = 0x64;

  // These will still need to be set
  m_nPort = 0;
  m_nSession = 0;

  return true;
}


bool CChatClient::LoadFromHandshake_v7(CBuffer &b)
{
  CPacketTcp_Handshake_v7 hand(&b);

  m_nVersion = hand.VersionMajor();
  unsigned long m_nUin = hand.SourceUin();
  char szUin[24];
  sprintf(szUin, "%lu", m_nUin);
  myUserId = Licq::UserId(szUin, LICQ_PPID);
  m_nIntIp = hand.LocalIp();
  m_nIp = hand.RealIp();
  m_nMode = hand.Mode();
  m_nHandshake = 0x65;

  // These will still need to be set
  m_nPort = 0;
  m_nSession = 0;

  return true;
}


CPChat_ColorFont::CPChat_ColorFont(const string& localName, unsigned short nLocalPort,
   unsigned short nSession,
   int nColorForeRed, int nColorForeGreen, int nColorForeBlue, int nColorBackRed,
   int nColorBackBlue, int nColorBackGreen,
   unsigned long nFontSize,
   bool bFontBold, bool bFontItalic, bool bFontUnderline, bool bFontStrikeOut,
    const string& fontFamily, unsigned char nFontEncoding,
   unsigned char nFontStyle, ChatClientPList &clientList)
{
  m_nPort = nLocalPort;
  myUserId = Licq::gUserManager.ownerUserId(LICQ_PPID);
  unsigned long m_nUin = atol(myUserId.accountId().c_str());
  m_nColorForeRed = nColorForeRed;
  m_nColorForeGreen = nColorForeGreen;
  m_nColorForeBlue = nColorForeBlue;
  m_nColorBackRed = nColorBackRed;
  m_nColorBackGreen = nColorBackGreen;
  m_nColorBackBlue = nColorBackBlue;
  m_nSession = nSession;
  m_nFontSize = nFontSize;
  m_nFontFace = FONT_PLAIN;
  if (bFontBold) m_nFontFace |= FONT_BOLD;
  if (bFontItalic) m_nFontFace |= FONT_ITALIC;
  if (bFontUnderline) m_nFontFace |= FONT_UNDERLINE;
  if (bFontStrikeOut) m_nFontFace |= FONT_STRIKEOUT;
  m_nFontEncoding = nFontEncoding;
  m_nFontStyle = nFontStyle;

  m_nSize = 10 + localName.size() + 38 + fontFamily.size() + 4
            + clientList.size() * (sizeof(CChatClient) + 2);
  InitBuffer();

  buffer->PackUnsignedLong(0x65);
  buffer->PackUnsignedLong(m_nUin);
  buffer->PackString(localName.c_str());
  buffer->PackChar(nColorForeRed);
  buffer->PackChar(nColorForeGreen);
  buffer->PackChar(nColorForeBlue);
  buffer->PackChar(0);
  buffer->PackChar(nColorBackRed);
  buffer->PackChar(nColorBackGreen);
  buffer->PackChar(nColorBackBlue);
  buffer->PackChar(0);
  buffer->PackUnsignedLong(ICQ_VERSION_TCP);
  buffer->PackUnsignedLong(m_nPort);
  buffer->PackUnsignedLong(s_nLocalIp);
  buffer->PackUnsignedLong(s_nRealIp);
  buffer->PackChar(s_nMode);
  buffer->PackUnsignedShort(m_nSession);
  buffer->PackUnsignedLong(m_nFontSize);
  buffer->PackUnsignedLong(m_nFontFace);
  buffer->PackString(fontFamily.c_str());
  buffer->PackChar(nFontEncoding);
  buffer->PackChar(nFontStyle);
  buffer->PackChar(clientList.size());

  ChatClientPList::iterator iter;
  for (iter = clientList.begin(); iter != clientList.end(); ++iter)
  {
    buffer->PackUnsignedLong((*iter)->m_nVersion);
    buffer->PackUnsignedLong((*iter)->m_nPort);
    buffer->PackUnsignedLong(strtoul((*iter)->myUserId.accountId().c_str(), NULL, 10));
    buffer->PackUnsignedLong((*iter)->m_nIp);
    buffer->PackUnsignedLong((*iter)->m_nIntIp);
    buffer->PackChar((*iter)->m_nMode);
    buffer->PackUnsignedShort((*iter)->m_nPort);
    buffer->PackUnsignedShort((*iter)->m_nSession);
    buffer->PackUnsignedLong((*iter)->m_nHandshake);
  }
}


CPChat_ColorFont::CPChat_ColorFont(CBuffer &b)
{
  b.UnpackUnsignedLong();
  unsigned long m_nUin = b.UnpackUnsignedLong();
  char szUin[24];
  sprintf(szUin, "%lu", m_nUin);
  myUserId = Licq::UserId(szUin, LICQ_PPID);
  myName = b.unpackString();
  m_nColorForeRed = (unsigned char)b.UnpackChar();
  m_nColorForeGreen = (unsigned char)b.UnpackChar();
  m_nColorForeBlue = (unsigned char)b.UnpackChar();
  b.UnpackChar();
  m_nColorBackRed = (unsigned char)b.UnpackChar();
  m_nColorBackGreen = (unsigned char)b.UnpackChar();
  m_nColorBackBlue = (unsigned char)b.UnpackChar();
  b.UnpackChar();

  b.UnpackUnsignedLong();
  m_nPort = b.UnpackUnsignedLong();
  b.UnpackUnsignedLong();
  b.UnpackUnsignedLong();
  b.UnpackChar();
  m_nSession = b.UnpackUnsignedShort();
  m_nFontSize = b.UnpackUnsignedLong();
  m_nFontFace = b.UnpackUnsignedLong();
  myFontFamily = b.unpackString();
  m_nFontEncoding = b.UnpackChar();
  m_nFontStyle = b.UnpackChar();

  // Read out client packets
  unsigned short nc = b.UnpackChar();
  for (unsigned short i = 0; i < nc; i++)
  {
    chatClients.push_back(CChatClient(b));
  }

}

CPChat_ColorFont::~CPChat_ColorFont()
{
  // Empty
}

//-----ChatFont---------------------------------------------------------------------
CPChat_Font::CPChat_Font(unsigned short nLocalPort, unsigned short nSession,
                         unsigned long nFontSize,
                         bool bFontBold, bool bFontItalic, bool bFontUnderline,
    bool bFontStrikeOut, const string& fontFamily,
                         unsigned char nFontEncoding, unsigned char nFontStyle)
{
  m_nPort = nLocalPort;
  m_nSession = nSession;
  m_nFontSize = nFontSize;
  m_nFontFace = FONT_PLAIN;
  if (bFontBold) m_nFontFace |= FONT_BOLD;
  if (bFontItalic) m_nFontFace |= FONT_ITALIC;
  if (bFontUnderline) m_nFontFace |= FONT_UNDERLINE;
  if (bFontStrikeOut) m_nFontFace |= FONT_STRIKEOUT;
  m_nFontEncoding = nFontEncoding;
  m_nFontStyle = nFontStyle;

  m_nSize = 29 + fontFamily.size() + 3;
  InitBuffer();

  buffer->PackUnsignedLong(ICQ_VERSION_TCP);
  buffer->PackUnsignedLong(m_nPort);
  buffer->PackUnsignedLong(s_nLocalIp);
  buffer->PackUnsignedLong(s_nRealIp);
  buffer->PackChar(s_nMode);
  buffer->PackUnsignedShort(nSession);//0x5A89);
  buffer->PackUnsignedLong(m_nFontSize);
  buffer->PackUnsignedLong(m_nFontFace);
  buffer->PackString(fontFamily.c_str());
  buffer->PackChar(nFontEncoding);
  buffer->PackChar(nFontStyle);
}

CPChat_Font::CPChat_Font(CBuffer &b)
{
  b.UnpackUnsignedLong();
  m_nPort = b.UnpackUnsignedLong();
  b.UnpackUnsignedLong();
  b.UnpackUnsignedLong();
  b.UnpackChar();
  m_nSession = b.UnpackUnsignedShort();
  m_nFontSize = b.UnpackUnsignedLong();
  m_nFontFace = b.UnpackUnsignedLong();
  myFontFamily = b.unpackString();
  m_nFontEncoding = b.UnpackChar();
  m_nFontStyle = b.UnpackChar();
}

CPChat_Font::~CPChat_Font()
{
  // Empty
}

/*
//-----CPChat_ChangeFontFamily-----------------------------------------------
CPChat_ChangeFontFamily::CPChat_ChangeFontFamily(const string& fontFamily)
{
  m_nSize = fontFamily.size() + 6;
  InitBuffer();

  buffer->PackChar(CHAT_FONTxFAMILY);
  buffer->PackString(fontFamily.c_str());
  buffer->PackUnsignedShort(0x2200);
  // 0x2200 west
  // 0x22a2 turkey
  // 0x22cc cyrillic
  // 0x22a1 greek
  // 0x22ba baltic
}


CPChat_ChangeFontFamily::CPChat_ChangeFontFamily(CBuffer &b)
{
  //b.UnpackChar(); // CHAT_CHANGExFONT
  myFontFamily = b.unpackString();
  b.UnpackUnsignedShort();  // Charset?
}


//-----CPChat_ChangeFontFamily-----------------------------------------------
CPChat_ChangeFontSize::CPChat_ChangeFontSize(unsigned short nSize)
{
  m_nFontSize = nSize;

  m_nSize = 3;
  InitBuffer();

  buffer->PackChar(CHAT_FONTxSIZE);
  buffer->PackUnsignedLong(nSize);
}


CPChat_ChangeFontSize::CPChat_ChangeFontSize(CBuffer &b)
{
  //b.UnpackChar();
  m_nFontSize = b.UnpackUnsignedLong();
}



//-----CPChat_ChangeFontFace-----------------------------------------------
CPChat_ChangeFontFace::CPChat_ChangeFontFace(bool bBold, bool bItalic,
   bool bUnderline)
{
  m_nFontFace = FONT_PLAIN;
  if (bBold) m_nFontFace |= FONT_BOLD;
  if (bItalic) m_nFontFace |= FONT_ITALIC;
  if (bUnderline) m_nFontFace |= FONT_UNDERLINE;

  m_nSize = 5;
  InitBuffer();

  buffer->PackChar(CHAT_FONTxFACE);
  buffer->PackUnsignedLong(m_nFontFace);
}


CPChat_ChangeFontFace::CPChat_ChangeFontFace(CBuffer &b)
{
  //b.UnpackChar();
  m_nFontFace = b.UnpackUnsignedLong();
}


CPChat_ChangeColorBg::CPChat_ChangeColorBg(int nRed, int nGreen, int nBlue)
{
  m_nColorBackRed = nRed;
  m_nColorBackGreen = nGreen;
  m_nColorBackBlue = nBlue;

  m_nSize = 5;
  InitBuffer();

  buffer->PackChar(CHAT_COLORxBG);
  buffer->PackChar(nRed);
  buffer->PackChar(nGreen);
  buffer->PackChar(nBlue);
  buffer->PackChar(0);
}


CPChat_ChangeColorBg::CPChat_ChangeColorBg(CBuffer &b)
{
  //b.UnpackChar();
  m_nColorBackRed = (unsigned char)b.UnpackChar();
  m_nColorBackGreen = (unsigned char)b.UnpackChar();
  m_nColorBackBlue = (unsigned char)b.UnpackChar();
  b.UnpackChar();
}


CPChat_ChangeColorFg::CPChat_ChangeColorFg(int nRed, int nGreen, int nBlue)
{
  m_nColorForeRed = nRed;
  m_nColorForeGreen = nGreen;
  m_nColorForeBlue = nBlue;

  m_nSize = 5;
  InitBuffer();

  buffer->PackChar(CHAT_COLORxFG);
  buffer->PackChar(nRed);
  buffer->PackChar(nGreen);
  buffer->PackChar(nBlue);
  buffer->PackChar(0);
}


CPChat_ChangeColorFg::CPChat_ChangeColorFg(CBuffer &b)
{
  //b.UnpackChar();
  m_nColorForeRed = (unsigned char)b.UnpackChar();
  m_nColorForeGreen = (unsigned char)b.UnpackChar();
  m_nColorForeBlue = (unsigned char)b.UnpackChar();
  b.UnpackChar();
}


CPChat_Beep::CPChat_Beep()
{
  m_nSize = 1;
  InitBuffer();

  buffer->PackChar(CHAT_BEEP);
}
*/


//=====ChatUser==============================================================
CChatUser::CChatUser()
{
  nToKick = 0;
  state = CHAT_STATE_DISCONNECTED;
  colorFore[0] = colorFore[1] = colorFore[2] = 0x00;
  colorBack[0] = colorBack[1] = colorBack[2] = 0xFF;
  myFontFamily = "courier";
  fontEncoding = ENCODING_DEFAULT;
  fontStyle = STYLE_MODERN | STYLE_FIXEDxPITCH; // style of courier
  fontSize = 12;
  fontFace = FONT_PLAIN;
  focus = true;
  sleep = false;
  m_pClient = NULL;

  pthread_mutex_init(&mutex, NULL);
}

CChatUser::~CChatUser()
{
}

CChatEvent::CChatEvent(unsigned char nCommand, CChatUser *u, const string& data)
  : myData(data)
{
  m_nCommand = nCommand;
  m_pUser = u;
  m_bLocked = false;
}


CChatEvent::~CChatEvent()
{
  if (m_bLocked)
    pthread_mutex_unlock(&m_pUser->mutex);
}


//=====ChatManager===========================================================
ChatManagerList CChatManager::cmList;
pthread_mutex_t CChatManager::cmList_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t CChatManager::waiting_thread_cancel_mutex
                                                  = PTHREAD_MUTEX_INITIALIZER;


CChatManager::CChatManager(unsigned long nUin,
  const string& fontFamily, unsigned char fontEncoding, unsigned char fontStyle,
  unsigned short fontSize, bool fontBold, bool fontItalic, bool fontUnderline,
  bool fontStrikeOut, int fr, int fg, int fb, int br, int bg, int bb)
{
  char szUin[24];
  sprintf(szUin, "%lu", nUin);
  myUserId = Licq::UserId(szUin, LICQ_PPID);

//  m_nSession = rand();

  {
    Licq::OwnerReadGuard o(LICQ_PPID);
    myName = o->getAlias();
    m_nSession = o->Port();
  }

  m_nFontFace = FONT_PLAIN;
  if (fontBold) m_nFontFace |= FONT_BOLD;
  if (fontItalic) m_nFontFace |= FONT_ITALIC;
  if (fontUnderline) m_nFontFace |= FONT_UNDERLINE;
  if (fontStrikeOut) m_nFontFace |= FONT_STRIKEOUT;
  myFontFamily = fontFamily;
  m_nFontEncoding = fontEncoding;
  m_nFontStyle = fontStyle;
  m_nFontSize = fontSize;
  m_nColorFore[0] = fr;
  m_nColorFore[1] = fg;
  m_nColorFore[2] = fb;
  m_nColorBack[0] = br;
  m_nColorBack[1] = bg;
  m_nColorBack[2] = bb;
  m_bFocus = true;
  m_bSleep = false;
  m_pChatClient = NULL;
  m_bThreadCreated = false;

  pthread_mutex_init(&thread_list_mutex, NULL);

  pthread_mutex_lock(&cmList_mutex);
  cmList.push_back(this);
  pthread_mutex_unlock(&cmList_mutex);
}


//-----CChatManager::StartChatServer-----------------------------------------
bool CChatManager::StartChatServer()
{
  if (gDaemon.StartTCPServer(&chatServer) == -1)
  {
    gLog.warning(tr("%sNo more ports available, add more or close open chat/file sessions.\n"), L_WARNxSTR);
    return false;
  }

  // Add the server to the sock manager
  sockman.AddSocket(&chatServer);
  sockman.DropSocket(&chatServer);

  return true;
}



bool CChatManager::StartAsServer()
{
  if (!StartChatServer())
  {
    PushChatEvent(new CChatEvent(CHAT_ERRORxBIND, NULL));
    return false;
  }

  // Create the socket manager thread
  if (pthread_create(&thread_chat, NULL, &ChatManager_tep, this) == -1)
  {
    PushChatEvent(new CChatEvent(CHAT_ERRORxRESOURCES, NULL));
    return false;
  }

  m_bThreadCreated = true;

  return true;
}


//-----CChatManager::StartAsClient-------------------------------------------
void CChatManager::StartAsClient(unsigned short nPort)
{
  if (!StartChatServer()) return;

  {
    Licq::UserReadGuard u(myUserId);
    if (!u.isLocked())
      return;
    m_pChatClient = new CChatClient(*u);
    m_pChatClient->m_nPort = nPort;
  }

  // Create the socket manager thread
  if (pthread_create(&thread_chat, NULL, &ChatManager_tep, this) == -1)
  {
    PushChatEvent(new CChatEvent(CHAT_ERRORxRESOURCES, NULL));
    return;
  }
}


//-----CChatManager::ConnectToChat-------------------------------------------
bool CChatManager::ConnectToChat(CChatClient *c)
{
  CChatUser *u = new CChatUser;
  u->m_pClient = c;
  u->m_pClient->m_nSession = m_nSession;
  u->myUserId = c->myUserId;

  bool bSendIntIp = false;
  bool bTryDirect = true;
  bool bResult = false;

  {
    Licq::UserReadGuard temp_user(u->myUserId);
    if (temp_user.isLocked())
    {
      bSendIntIp = temp_user->SendIntIp();
      bTryDirect = temp_user->Version() <= 6 || temp_user->Mode() == MODE_DIRECT;
    }
  }

  bool bSuccess = false;
  if (bTryDirect)
  {
    gLog.info("%sChat: Connecting to server.\n", L_TCPxSTR);
    bSuccess = gIcqProtocol.OpenConnectionToUser("chat", c->m_nIp, c->m_nIntIp,
                                            &u->sock, c->m_nPort, bSendIntIp);
  }

  if (!bSuccess)
  {
    unsigned long nIp;
    {
      Licq::OwnerReadGuard o(LICQ_PPID);
      nIp = bSendIntIp ? o->IntIp() : o->Ip();
    }

    // try reverse connect
    int nId = gIcqProtocol.requestReverseConnection(c->myUserId, c->m_nSession,
                                                 nIp, LocalPort(), c->m_nPort);
    if (nId != -1)
    {
      pthread_t t;
      struct SChatReverseConnectInfo *r = new struct SChatReverseConnectInfo;
      r->nId = nId;
      r->u = u;
      r->m = this;
      r->bTryDirect = !bTryDirect;
      pthread_mutex_lock(&thread_list_mutex);
      pthread_create(&t, NULL, ChatWaitForSignal_tep, r);
      waitingThreads.push_back(t);
      pthread_mutex_unlock(&thread_list_mutex);
      bResult = true;
    }
    else
    {
      delete u->m_pClient;
      delete u;
    }
  }
  else
  {
    chatUsers.push_back(u);
    bResult = SendChatHandshake(u);
  }
 
  return bResult;
}

bool CChatManager::SendChatHandshake(CChatUser *u)
{
  CChatClient *c = u->m_pClient;
  
  gLog.info(tr("%sChat: Shaking hands [v%d].\n"), L_TCPxSTR, VersionToUse(c->m_nVersion));

  // Send handshake packet:
  if (!IcqProtocol::handshake_Send(&u->sock, c->myUserId, LocalPort(),
     VersionToUse(c->m_nVersion), false))
    return false;

  // Send color packet
  CPChat_Color p_color(myName, LocalPort(),
     m_nColorFore[0], m_nColorFore[1], m_nColorFore[2],
     m_nColorBack[0], m_nColorBack[1], m_nColorBack[2]);
  u->sock.SendPacket(p_color.getBuffer());

  gLog.info(tr("%sChat: Waiting for color/font response.\n"), L_TCPxSTR);

  u->state = CHAT_STATE_WAITxFORxCOLORxFONT;

  sockman.AddSocket(&u->sock);
  sockman.DropSocket(&u->sock);

  return true;
}


//-----CChatManager::AcceptReverseConnection---------------------------------
void CChatManager::AcceptReverseConnection(Licq::TCPSocket* s)
{
  CChatUser *u = new CChatUser;
  u->sock.TransferConnectionFrom(*s);

  u->m_pClient = new CChatClient();
  u->m_pClient->m_nVersion = s->Version();
  u->m_pClient->myUserId = s->userId();
  u->m_pClient->m_nIp = s->getRemoteIpInt();
  u->m_pClient->m_nIntIp = s->getRemoteIpInt();
  u->m_pClient->m_nMode = MODE_DIRECT;
  u->m_pClient->m_nHandshake = 0x65;

  // These will still need to be set
  u->m_pClient->m_nPort = 0;
  u->m_pClient->m_nSession = 0;

  u->myUserId = u->m_pClient->myUserId;
  u->state = CHAT_STATE_WAITxFORxCOLOR;
  chatUsers.push_back(u);

  // Reload the socket information
  sockman.AddSocket(&u->sock);
  sockman.DropSocket(&u->sock);
  myThreadPipe.putChar('R');

  gLog.info(tr("%sChat: Received reverse connection.\n"), L_TCPxSTR);
}


//-----CChatManager::FindChatUser--------------------------------------------
CChatUser *CChatManager::FindChatUser(int sd)
{
  // Find the right user (possible race condition, but we ignore it for now)
  ChatUserList::iterator iter;
  for (iter = chatUsers.begin(); iter != chatUsers.end(); ++iter)
    if ( (*iter)->sock.Descriptor() == sd) break;

  if (iter == chatUsers.end())
    return NULL;

  return *iter;
}


//-----CChatManager::ProcessPacket-------------------------------------------
bool CChatManager::ProcessPacket(CChatUser *u)
{
  if (!u->sock.RecvPacket())
  {
    if (u->sock.Error() == 0)
      gLog.info(tr("%sChat: Remote end disconnected.\n"), L_TCPxSTR);
    else
      gLog.info(tr("%sChat: Lost remote end:\n%s%s\n"), L_TCPxSTR,
          L_BLANKxSTR, u->sock.errorStr().c_str());
    return false;
  }

  if (!u->sock.RecvBufferFull()) return true;

  switch(u->state)
  {
    case CHAT_STATE_HANDSHAKE:
    {
      CBuffer handshake = u->sock.RecvBuffer();
      // get the handshake packet
      if (!IcqProtocol::Handshake_Recv(&u->sock, LocalPort(), false, true))
      {
        gLog.warning("%sChat: Bad handshake.\n", L_ERRORxSTR);
        return false;
      }
      switch (u->sock.Version())
      {
        case 1:
        case 2:
        case 3:
          u->m_pClient->LoadFromHandshake_v2(handshake);
          break;
        case 4:
        case 5:
          u->m_pClient->LoadFromHandshake_v4(handshake);
          break;
        case 6:
          u->m_pClient->LoadFromHandshake_v6(handshake);
          break;
        case 7:
        case 8:
          u->m_pClient->LoadFromHandshake_v7(handshake);
          break;
      }
      gLog.info(tr("%sChat: Received handshake from %s [v%ld].\n"), L_TCPxSTR,
          u->m_pClient->myUserId.toString().c_str(), u->sock.Version());
      u->myUserId = u->m_pClient->myUserId;

      bool bFound = false;
      {
        pthread_mutex_lock(&gIcqProtocol.mutex_reverseconnect);
        std::list<CReverseConnectToUserData *>::iterator iter;
        for (iter = gIcqProtocol.m_lReverseConnect.begin();
            iter != gIcqProtocol.m_lReverseConnect.end();  ++iter)
        {
          if ((*iter)->myIdString == u->userId().accountId())
          {
            bFound = true;
            (*iter)->bSuccess = true;
            (*iter)->bFinished = true;
            u->m_pClient->m_nSession = (*iter)->nData;
            u->m_pClient->m_nPort = (*iter)->nPort;
            pthread_cond_broadcast(&gIcqProtocol.cond_reverseconnect_done);
            break;
          }
        }
        pthread_mutex_unlock(&gIcqProtocol.mutex_reverseconnect);
      }

      if (bFound)
      {
        // Send color packet
        CPChat_Color p_color(myName, LocalPort(),
        m_nColorFore[0], m_nColorFore[1], m_nColorFore[2],
        m_nColorBack[0], m_nColorBack[1], m_nColorBack[2]);
        u->sock.SendPacket(p_color.getBuffer());

        gLog.info("%sChat: Waiting for color/font response.\n", L_TCPxSTR);

        u->state = CHAT_STATE_WAITxFORxCOLORxFONT;
      }
      else
      {
        u->state = CHAT_STATE_WAITxFORxCOLOR;
      }
      break;
    }

    case CHAT_STATE_WAITxFORxCOLOR:  // we just received the color packet
    {
      gLog.info(tr("%sChat: Received color packet.\n"), L_TCPxSTR);

      CPChat_Color pin(u->sock.RecvBuffer());

      u->myName = pin.name();
      // Fill in the remaining fields in the client structure
      u->m_pClient->m_nPort = pin.Port();
      u->m_pClient->m_nSession = m_nSession;

      // set up the remote colors
      u->colorFore[0] = pin.ColorForeRed();
      u->colorFore[1] = pin.ColorForeGreen();
      u->colorFore[2] = pin.ColorForeBlue();
      u->colorBack[0] = pin.ColorBackRed();
      u->colorBack[1] = pin.ColorBackGreen();
      u->colorBack[2] = pin.ColorBackBlue();

      // Send the response
      ChatClientPList l;
      ChatUserList::iterator iter;
      for (iter = chatUsers.begin(); iter != chatUsers.end(); ++iter)
      {
        // Skip this guys client info and anybody we haven't connected to yet
        if ((*iter)->myUserId == u->myUserId || !(*iter)->m_pClient->myUserId.isValid())
          continue;
        l.push_back((*iter)->m_pClient);
      }

      CPChat_ColorFont p_colorfont(myName, LocalPort(), m_nSession,
         m_nColorFore[0], m_nColorFore[1], m_nColorFore[2],
         m_nColorBack[0], m_nColorBack[1], m_nColorBack[2],
         m_nFontSize, m_nFontFace & FONT_BOLD, m_nFontFace & FONT_ITALIC,
         m_nFontFace & FONT_UNDERLINE, m_nFontFace & FONT_STRIKEOUT,
          myFontFamily, m_nFontEncoding, m_nFontStyle, l);
      if (!u->sock.SendPacket(p_colorfont.getBuffer()))
      {
        gLog.error("%sChat: Send error (color/font packet):\n%s%s\n",
            L_ERRORxSTR, L_BLANKxSTR, u->sock.errorStr().c_str());
        return false;
      }
      u->state = CHAT_STATE_WAITxFORxFONT;
      break;
    }

    case CHAT_STATE_WAITxFORxFONT:
    {
      gLog.info(tr("%sChat: Received font packet.\n"), L_TCPxSTR);
      CPChat_Font pin(u->sock.RecvBuffer());

      // just received the font reply
      m_nSession = pin.Session();
      u->fontSize = pin.FontSize();
      u->fontFace = pin.FontFace();
      u->myFontFamily = pin.fontFamily();
      u->fontEncoding = pin.FontEncoding();
      u->fontStyle = pin.FontStyle();

      u->state = CHAT_STATE_CONNECTED;
      PushChatEvent(new CChatEvent(CHAT_CONNECTION, u));
      break;
    }

    case CHAT_STATE_WAITxFORxCOLORxFONT:
    {
      gLog.info(tr("%sChat: Received color/font packet.\n"), L_TCPxSTR);

      CPChat_ColorFont pin(u->sock.RecvBuffer());
      u->myUserId = pin.userId();
//      m_nSession = pin.Session();

      // just received the color/font packet
      u->myName = pin.name();

      // set up the remote colors
      u->colorFore[0] = pin.ColorForeRed();
      u->colorFore[1] = pin.ColorForeGreen();
      u->colorFore[2] = pin.ColorForeBlue();
      u->colorBack[0] = pin.ColorBackRed();
      u->colorBack[1] = pin.ColorBackGreen();
      u->colorBack[2] = pin.ColorBackBlue();

      // set up the remote font
//      m_nSession = pin.Session();
      u->fontSize = pin.FontSize();
      u->fontFace = pin.FontFace();
      u->myFontFamily = pin.fontFamily();
      u->fontEncoding = pin.FontEncoding();
      u->fontStyle = pin.FontStyle();

      // Parse the multiusers list
      if (pin.ChatClients().size() > 0)
      {
        gLog.info(tr("%sChat: Joined multiparty (%d people).\n"), L_TCPxSTR,
           int(pin.ChatClients().size() + 1));
        ChatClientList::iterator iter;
        for (iter = pin.ChatClients().begin(); iter != pin.ChatClients().end(); ++iter)
        {
          ChatUserList::iterator iter2;
          for (iter2 = chatUsers.begin(); iter2 != chatUsers.end(); iter2++)
          {
            if ((*iter2)->myUserId == iter->myUserId)
              break;
          }
          if (iter2 != chatUsers.end()) continue;
          // Connect to this user
          CChatClient *p = new CChatClient(*iter);
          ConnectToChat(p);
        }
      }

      // send the reply (font packet)
			CPChat_Font p_font(LocalPort(), m_nSession,
         m_nFontSize, m_nFontFace & FONT_BOLD, m_nFontFace & FONT_ITALIC,
         m_nFontFace & FONT_UNDERLINE, m_nFontFace & FONT_STRIKEOUT,
          myFontFamily, m_nFontEncoding, m_nFontStyle);
      if (!u->sock.SendPacket(p_font.getBuffer()))
      {
        gLog.error("%sChat: Send error (font packet):\n%s%s\n",
            L_ERRORxSTR, L_BLANKxSTR, u->sock.errorStr().c_str());
        return false;
      }

      // now we are done with the handshaking
      u->state = CHAT_STATE_CONNECTED;
      PushChatEvent(new CChatEvent(CHAT_CONNECTION, u));
      break;
    }

    case CHAT_STATE_CONNECTED:
    default:
      gLog.error("%sInternal error: ChatManager::ProcessPacket(), invalid state (%d).\n",
         L_ERRORxSTR, u->state);
      break;

  } // switch

  u->sock.ClearRecvBuffer();

  return true;
}


//-----CChatManager::PopChatEvent--------------------------------------------
CChatEvent *CChatManager::PopChatEvent()
{
  if (chatEvents.empty()) return NULL;

  CChatEvent *e = chatEvents.front();
  chatEvents.pop_front();

  // Lock the user, will be unlocked in the event destructor
  if (e->m_pUser)
  {
    pthread_mutex_lock(&e->m_pUser->mutex);
    e->m_bLocked = true;
  }

  return e;
}


//-----CChatManager::PushChatEvent-------------------------------------------
void CChatManager::PushChatEvent(CChatEvent *e)
{
  chatEvents.push_back(e);
  myEventsPipe.putChar('*');
}


//-----CChatManager::ProcessRaw----------------------------------------------
bool CChatManager::ProcessRaw(CChatUser *u)
{
  if (!u->sock.RecvRaw())
  {
    if (u->sock.Error() == 0)
      gLog.info(tr("%sChat: Remote end disconnected.\n"), L_TCPxSTR);
    else
      gLog.info(tr("%sChat: Lost remote end:\n%s%s\n"), L_TCPxSTR,
          L_BLANKxSTR, u->sock.errorStr().c_str());
    return false;
  }

  while (!u->sock.RecvBuffer().End())
    u->chatQueue.push_back(u->sock.RecvBuffer().UnpackChar());
  u->sock.ClearRecvBuffer();

  if (u->sock.Version() >= 6)
    return ProcessRaw_v6(u);
  else
    return ProcessRaw_v2(u);
}


bool CChatManager::ProcessRaw_v2(CChatUser *u)
{
  char chatChar;
  while (u->chatQueue.size() > 0)
  {
    chatChar = *u->chatQueue.begin(); // first character in queue (not dequeued)
    switch (chatChar)
    {
      case CHAT_NEWLINE:   // new line
        // add to irc window
        PushChatEvent(new CChatEvent(CHAT_NEWLINE, u, u->myLinebuf));
        u->myLinebuf.clear();
        u->chatQueue.pop_front();
        break;

      case CHAT_BEEP:  // beep
      {
        PushChatEvent(new CChatEvent(CHAT_BEEP, u));
        u->chatQueue.pop_front();
        break;
      }

      case CHAT_LAUGH:  // laugh
      {
        PushChatEvent(new CChatEvent(CHAT_LAUGH, u));
        u->chatQueue.pop_front();
        break;
      }

      case CHAT_BACKSPACE:   // backspace
      {
        if (u->myLinebuf.size() > 0)
          u->myLinebuf.erase(u->myLinebuf.size()-1);
        PushChatEvent(new CChatEvent(CHAT_BACKSPACE, u));
        u->chatQueue.pop_front();
        break;
      }

      case CHAT_COLORxFG: // change foreground color
      {
        if (u->chatQueue.size() < 5) return true;
        u->colorFore[0] = u->chatQueue[1];
        u->colorFore[1] = u->chatQueue[2];
        u->colorFore[2] = u->chatQueue[3];
        for (unsigned short i = 0; i < 5; i++)
          u->chatQueue.pop_front();

        PushChatEvent(new CChatEvent(CHAT_COLORxFG, u));
        break;
      }

      case CHAT_COLORxBG:  // change background color
      {
        if (u->chatQueue.size() < 5) return true;
        u->colorBack[0] = u->chatQueue[1];
        u->colorBack[1] = u->chatQueue[2];
        u->colorBack[2] = u->chatQueue[3];
        for (unsigned short i = 0; i < 5; i++)
          u->chatQueue.pop_front();

        PushChatEvent(new CChatEvent(CHAT_COLORxBG, u));
        break;
      }
      case CHAT_FONTxFAMILY: // change font type
      {
         if (u->chatQueue.size() < 3) return true;
         unsigned short sizeFontName, i;
         sizeFontName = u->chatQueue[1] | (u->chatQueue[2] << 8);
         if (u->chatQueue.size() < (unsigned long)(sizeFontName + 2 + 3)) return true;
         char* nameFont = new char[sizeFontName + 1];
         for (i = 0; i < sizeFontName; i++)
            nameFont[i] = u->chatQueue[i + 3];
         nameFont[sizeFontName] = '\0';
         u->myFontFamily = nameFont;
         delete [] nameFont;
         u->fontEncoding = u->chatQueue[sizeFontName + 3];
         u->fontStyle = u->chatQueue[sizeFontName + 4];

         // Dequeue all characters
         for (unsigned short i = 0; i < 3 + sizeFontName + 2; i++)
           u->chatQueue.pop_front();

         PushChatEvent(new CChatEvent(CHAT_FONTxFAMILY, u));
         break;
      }

      case CHAT_FONTxFACE: // change font style
      {
        if (u->chatQueue.size() < 5) return true;
        unsigned long styleFont;
        styleFont = u->chatQueue[1] | (u->chatQueue[2] << 8) |
                    (u->chatQueue[3] << 16) | (u->chatQueue[4] << 24);

        u->fontFace = styleFont;

        // Dequeue all characters
        for (unsigned short i = 0; i < 5; i++)
          u->chatQueue.pop_front();

        PushChatEvent(new CChatEvent(CHAT_FONTxFACE, u));
        break;
      }

      case CHAT_FONTxSIZE: // change font size
      {
        if (u->chatQueue.size() < 5) return true;
        unsigned long sizeFont;
        sizeFont = u->chatQueue[1] | (u->chatQueue[2] << 8) |
                   (u->chatQueue[3] << 16) | (u->chatQueue[4] << 24);
        u->fontSize = sizeFont;

        // Dequeue all characters
        for (unsigned short i = 0; i < 5; i++)
          u->chatQueue.pop_front();

        PushChatEvent(new CChatEvent(CHAT_FONTxSIZE, u));
        break;
      }

      case CHAT_FOCUSxIN:
      {
        u->focus = true;
        PushChatEvent(new CChatEvent(CHAT_FOCUSxIN, u));
        u->chatQueue.pop_front();
        break;
      }

      case CHAT_FOCUSxOUT:
      {
        u->focus = false;
        PushChatEvent(new CChatEvent(CHAT_FOCUSxOUT, u));
        u->chatQueue.pop_front();
        break;
      }

      case CHAT_SLEEPxOFF:
      {
        u->sleep = false;
        PushChatEvent(new CChatEvent(CHAT_SLEEPxOFF, u));
        u->chatQueue.pop_front();
        break;
      }

      case CHAT_SLEEPxON:
      {
        u->sleep = true;
        PushChatEvent(new CChatEvent(CHAT_SLEEPxON, u));
        u->chatQueue.pop_front();
        break;
      }

      case CHAT_KICK:
      {
        if (u->chatQueue.size() < 4)  return true;
        u->nToKick = u->chatQueue[0] | (u->chatQueue[1] << 8) |
                     (u->chatQueue[2] << 16) | (u->chatQueue[3] << 24);

        // Dequeue all the characters
        for (unsigned short i = 0; i < 4; i++)
          u->chatQueue.pop_front();

        PushChatEvent(new CChatEvent(CHAT_KICK, u));
        break;
      }

      case CHAT_KICKxYES:
      {
        if (u->chatQueue.size() < 4)  return true;
        unsigned long nUin = u->chatQueue[0] | (u->chatQueue[1] << 8) |
               (u->chatQueue[2] << 16) | (u->chatQueue[3] << 24);

        // Deque all the characters
        for (unsigned short i = 0; i < 4; i++)
          u->chatQueue.pop_front();

        // Find the person that we receive the yes vote
        VoteInfoList::iterator iter;
        for (iter = voteInfo.begin(); iter != voteInfo.end(); ++iter)
        {
          if ((*iter)->nUin == nUin)
            break;
        }

        if (iter == voteInfo.end())  return true;

        (*iter)->nYes++;

        // Is there a majority?
        unsigned short nMajority = (*iter)->nNumUsers / 2;
        nMajority++;
        if ((*iter)->nYes == nMajority)
          FinishKickVote(iter, true);
        else if ((*iter)->nYes + (*iter)->nNo == (*iter)->nNumUsers)
          FinishKickVote(iter, false);


        PushChatEvent(new CChatEvent(CHAT_KICKxYES, u));
        break;
      }

      case CHAT_KICKxNO:
      {
        if (u->chatQueue.size() < 4)  return true;
        unsigned long nUin = u->chatQueue[0] | (u->chatQueue[1] << 8) |
               (u->chatQueue[2] << 16) | (u->chatQueue[3] << 24);

        // Deque all the characters
        for (unsigned short i = 0; i < 4; i++)
          u->chatQueue.pop_front();

        // Find the person that we receive the yes vote
        VoteInfoList::iterator iter;
        for (iter = voteInfo.begin(); iter != voteInfo.end(); ++iter)
        {
          if ((*iter)->nUin == nUin)
            break;
        }

        if (iter == voteInfo.end())  return true;

        (*iter)->nNo++;

        // Is there a majority?
        unsigned short nMajority = (*iter)->nNumUsers / 2;
        nMajority++;
        if ((*iter)->nNo == nMajority)
          FinishKickVote(iter, false);
        else if ((*iter)->nYes + (*iter)->nNo == (*iter)->nNumUsers)
          FinishKickVote(iter, false);

        PushChatEvent(new CChatEvent(CHAT_KICKxNO, u));
        break;
      }

      case CHAT_KICKxPASS:
      {
        // The user here was kicked, close our connection to the user
        if (u->chatQueue.size() < 6)  return true;
        unsigned long nUin = u->chatQueue[0] | (u->chatQueue[1] << 8) |
          (u->chatQueue[2] << 16) | (u->chatQueue[3] << 24);
        char id[16];
        snprintf(id, 16, "%lu", nUin);
        Licq::UserId userId(id, LICQ_PPID);

        // Deque all the characters
        for (unsigned short i = 0; i < 6; i++)
          u->chatQueue.pop_front();

        // Find the user and say bye-bye to him
        ChatUserList::iterator iter;
        for (iter = chatUsers.begin(); iter != chatUsers.end(); ++iter)
        {
          if ((*iter)->userId() == userId)
            break;
        }

        if (iter == chatUsers.end())   return true;

        CBuffer bye(4);
        SendBuffer(&bye, CHAT_DISCONNECTIONxKICKED, id, true);

        CloseClient(*iter);
        break;
      }

      case CHAT_KICKxFAIL:
      {
        if (u->chatQueue.size() < 6)  return true;

        for (unsigned short i = 0; i < 6; i++)
          u->chatQueue.pop_front();

        PushChatEvent(new CChatEvent(CHAT_KICKxFAIL, u));
        break;
      }

      case CHAT_KICKxYOU:
      {
        if (u->chatQueue.size() < 2)  return true;

        for (unsigned short i = 0; i < 2; i++)
          u->chatQueue.pop_front();

        PushChatEvent(new CChatEvent(CHAT_KICKxYOU, u));
        break;
      }

      case CHAT_DISCONNECTIONxKICKED:
      {
        PushChatEvent(new CChatEvent(CHAT_DISCONNECTIONxKICKED, u));
        u->chatQueue.pop_front();
        break;
      }

      case CHAT_DISCONNECTION: // they will disconnect anyway
      {
        u->chatQueue.pop_front();
        break;
      }

      default:
      {
        if (!iscntrl((int)(unsigned char)chatChar))
        {
          Licq::gTranslator.ServerToClient(chatChar);
          char tempStr[2] = { chatChar, '\0' };
          // Add to the users irc line buffer
          u->myLinebuf += tempStr;
          PushChatEvent(new CChatEvent(CHAT_CHARACTER, u, tempStr));
          if (u->myLinebuf.size() > 1000) // stop a little early
          {
            u->myLinebuf.erase(1000);
            PushChatEvent(new CChatEvent(CHAT_NEWLINE, u, u->myLinebuf));
            u->myLinebuf.clear();
          }
        }
        u->chatQueue.pop_front();
        break;
      }
    } // switch
  } // while

  return true;
}


bool CChatManager::ProcessRaw_v6(CChatUser *u)
{
  char chatChar;
  unsigned long chatSize = 0;
  while (u->chatQueue.size() > 0)
  {
    chatChar = *u->chatQueue.begin(); // first character in queue (not dequeued)
    if (chatChar == 0)
    {
      // We need at least 6 chars
      if (u->chatQueue.size() < 6) return true;
      chatChar = u->chatQueue[1];
      chatSize = (u->chatQueue[2]) |
                 (u->chatQueue[3] << 8) |
                 (u->chatQueue[4] << 16) |
                 (u->chatQueue[5] << 24);

      if (u->chatQueue.size() < 6 + chatSize) return true;
      for (unsigned short i = 0; i < 6; i++)
        u->chatQueue.pop_front();

      switch (chatChar)
      {
        case CHAT_BEEP:  // beep
        {
          PushChatEvent(new CChatEvent(CHAT_BEEP, u));
          break;
        }

        case CHAT_LAUGH:  // laugh
        {
          PushChatEvent(new CChatEvent(CHAT_LAUGH, u));
          break;
        }

        case CHAT_COLORxFG: // change foreground color
        {
          u->colorFore[0] = u->chatQueue[0];
          u->colorFore[1] = u->chatQueue[1];
          u->colorFore[2] = u->chatQueue[2];

          PushChatEvent(new CChatEvent(CHAT_COLORxFG, u));
          break;
        }

        case CHAT_COLORxBG:  // change background color
        {
          u->colorBack[0] = u->chatQueue[0];
          u->colorBack[1] = u->chatQueue[1];
          u->colorBack[2] = u->chatQueue[2];

          PushChatEvent(new CChatEvent(CHAT_COLORxBG, u));
          break;
        }

        case CHAT_FONTxFAMILY: // change font type
        {
           unsigned short sizeFontName, i;
           sizeFontName = u->chatQueue[0] | (u->chatQueue[1] << 8);
           char* nameFont = new char [sizeFontName + 1];
           for (i = 0; i < sizeFontName; i++)
              nameFont[i] = u->chatQueue[i + 2];
           nameFont[sizeFontName] = '\0';
           u->myFontFamily = nameFont;
           delete [] nameFont;
           u->fontEncoding = u->chatQueue[sizeFontName + 2];
           u->fontStyle = u->chatQueue[sizeFontName + 3];

           PushChatEvent(new CChatEvent(CHAT_FONTxFAMILY, u));
           //the size includes the following character, so don't dequeue it
           chatSize--;
           break;
        }

        case CHAT_FONTxFACE: // change font style
        {
          unsigned long styleFont;
          styleFont = u->chatQueue[0] | (u->chatQueue[1] << 8) |
                      (u->chatQueue[2] << 16) | (u->chatQueue[3] << 24);

          u->fontFace = styleFont;

          PushChatEvent(new CChatEvent(CHAT_FONTxFACE, u));
          break;
        }

        case CHAT_FONTxSIZE: // change font size
        {
          if (u->chatQueue.size() < 4) return true;
          unsigned long sizeFont;
          sizeFont = u->chatQueue[0] | (u->chatQueue[1] << 8) |
                     (u->chatQueue[2] << 16) | (u->chatQueue[3] << 24);
          u->fontSize = sizeFont;

          PushChatEvent(new CChatEvent(CHAT_FONTxSIZE, u));
          break;
        }

        case CHAT_FOCUSxIN:
        {
          u->focus = true;
          PushChatEvent(new CChatEvent(CHAT_FOCUSxIN, u));
          break;
        }

        case CHAT_FOCUSxOUT:
        {
          u->focus = false;
          PushChatEvent(new CChatEvent(CHAT_FOCUSxOUT, u));
          break;
        }

        case CHAT_SLEEPxOFF:
        {
          u->sleep = false;
          PushChatEvent(new CChatEvent(CHAT_SLEEPxOFF, u));
          break;
        }

        case CHAT_SLEEPxON:
        {
          u->sleep = true;
          PushChatEvent(new CChatEvent(CHAT_SLEEPxON, u));
          break;
        }

        case CHAT_KICK:
        {
          if (u->chatQueue.size() < 4)  return true;
          unsigned long nUinToKick;
          nUinToKick = u->chatQueue[0] | (u->chatQueue[1] << 8) |
                       (u->chatQueue[2] << 16) | (u->chatQueue[3] << 24);
          u->nToKick = nUinToKick;

          PushChatEvent(new CChatEvent(CHAT_KICK, u));
          break;
        }

        case CHAT_KICKxYES:
        {
          if (u->chatQueue.size() < 4)  return true;
          unsigned long nUin = u->chatQueue[0] | (u->chatQueue[1] << 8) |
            (u->chatQueue[2] << 16) | (u->chatQueue[3] << 24);

          // Find the person that we received the yes vote for
          VoteInfoList::iterator iter;
          for (iter = voteInfo.begin(); iter != voteInfo.end(); ++iter)
          {
            if ((*iter)->nUin == nUin)
              break;
          }

          if (iter == voteInfo.end())  return true;

          (*iter)->nYes++;

          // Is there a majority?
          unsigned short nMajority = (*iter)->nNumUsers / 2;
          nMajority++;
          if ((*iter)->nYes == nMajority)
            FinishKickVote(iter, true);
          else if (((*iter)->nYes + (*iter)->nNo) == (*iter)->nNumUsers)
            FinishKickVote(iter, false);

          PushChatEvent(new CChatEvent(CHAT_KICKxYES, u));
          break;
        }

        case CHAT_KICKxNO:
        {
          if (u->chatQueue.size() < 4)  return true;
          unsigned long nUin = u->chatQueue[0] | (u->chatQueue[1] << 8) |
            (u->chatQueue[2] << 16) | (u->chatQueue[3] << 24);

          // Find the person that we received the no vote for
          VoteInfoList::iterator iter;
          for (iter = voteInfo.begin(); iter != voteInfo.end(); ++iter)
          {
            if ((*iter)->nUin == nUin)
              break;
          }

          if (iter == voteInfo.end())  return true;

          (*iter)->nNo++;

          // Is there a majority?
          unsigned short nMajority = (*iter)->nNumUsers / 2;
          nMajority++;
          if ((*iter)->nNo == nMajority)
            FinishKickVote(iter, false);
          else if (((*iter)->nYes + (*iter)->nNo) == (*iter)->nNumUsers)
            FinishKickVote(iter, false);

          PushChatEvent(new CChatEvent(CHAT_KICKxNO, u));
          break;
        }

        case CHAT_KICKxPASS:
        {
          // The user here was kicked, close our connection to the user
          if (u->chatQueue.size() < 6)  return true;
          unsigned long nUin = u->chatQueue[0] | (u->chatQueue[1] << 8) |
            (u->chatQueue[2] << 16) | (u->chatQueue[3] << 24);
          char id[16];
          snprintf(id, 16, "%lu", nUin);
          Licq::UserId userId(id, LICQ_PPID);

          // Find the user and say bye-bye to him
          ChatUserList::iterator iter;
          for (iter = chatUsers.begin(); iter != chatUsers.end(); ++iter)
          {
            if ((*iter)->userId() == userId)
              break;
          }

          if (iter == chatUsers.end())   return true;

          CBuffer bye(4);
          SendBuffer(&bye, CHAT_DISCONNECTIONxKICKED, id, true);

          CloseClient(*iter);
          break;
        }

        case CHAT_KICKxFAIL:
        {
          // The user was not kicked, a majority wasn't received
          PushChatEvent(new CChatEvent(CHAT_KICKxFAIL, u));
          break;
        }

        case CHAT_KICKxYOU:   // we were kicked
        {
          PushChatEvent(new CChatEvent(CHAT_KICKxYOU, u));
          break;
        }

        case CHAT_DISCONNECTIONxKICKED:  // they disconnected cuz we were kicked
        {
          PushChatEvent(new CChatEvent(CHAT_DISCONNECTIONxKICKED, u));
          break;
        }

        case CHAT_DISCONNECTION:        // they will disconnect anyway
        {
          break;
        }

        default:
        {
          gLog.unknown("Chat: Unknown chat command (%02X)", chatChar);
          break;
        }

      } // switch

      // dequeue all characters
      for (unsigned short i = 0; i < chatSize; i++)
        u->chatQueue.pop_front();

    } // if

    else // not a command
    {
      switch (chatChar)
      {
        case CHAT_NEWLINE:   // new line
          // add to irc window
          PushChatEvent(new CChatEvent(CHAT_NEWLINE, u, u->myLinebuf));
          u->myLinebuf.clear();
          break;

        case CHAT_BACKSPACE:   // backspace
        {
          if (u->myLinebuf.size() > 0)
            u->myLinebuf.erase(u->myLinebuf.size() - 1);
          PushChatEvent(new CChatEvent(CHAT_BACKSPACE, u));
          break;
        }

        default:
        {
          if (!iscntrl((int)(unsigned char)chatChar))
          {
            Licq::gTranslator.ServerToClient(chatChar);
            char tempStr[2] = { chatChar, '\0' };
            // Add to the users irc line buffer
            u->myLinebuf += tempStr;
            PushChatEvent(new CChatEvent(CHAT_CHARACTER, u, tempStr));
            if (u->myLinebuf.size() > 1000) // stop a little early
            {
              u->myLinebuf.erase(1000);
              PushChatEvent(new CChatEvent(CHAT_NEWLINE, u, u->myLinebuf));
              u->myLinebuf.clear();
            }
          }
          break;
        }
      }
      // Remove the character
      u->chatQueue.pop_front();
    }

  }


  return true;
}


//-----CChatManager::SendPacket----------------------------------------------
/*
void CChatManager::SendPacket(CPacket *p)
{
  SendBuffer(p->getBuffer());
}
*/

//-----CChatManager::SendBuffer----------------------------------------------
void CChatManager::SendBuffer(CBuffer *b, unsigned char cmd,
    const char* id, bool bNotIter)
{
  ChatUserList::iterator iter;
  ChatUserList::iterator u_iter;
  bool ok = false;

  if (id != NULL)
  {
    Licq::UserId userId(id, LICQ_PPID);
    for (u_iter = chatUsers.begin(); u_iter != chatUsers.end(); ++u_iter)
    {
      if ((*u_iter)->userId() == userId)
        break;
    }

     if (u_iter == chatUsers.end())
       return;
  }

  while (!ok)
  {
    ok = true;

    // Send it to every user
    if (id == NULL)
    {
      for (iter = chatUsers.begin(); iter != chatUsers.end(); ++iter)
        ok = SendBufferToClient(b, cmd, *iter);
    }
    else
    {
      // Send it to every user except _iter
      if (bNotIter)
      {
        for (iter = chatUsers.begin(); iter != u_iter; ++iter)
          ok = SendBufferToClient(b, cmd, *iter);

        // Check to see if we are already at the end
        // And at the same time skip the user we don't want to send this to
        if (++iter == chatUsers.end())  return;

        for (; iter != chatUsers.end(); ++iter)
          ok = SendBufferToClient(b, cmd, *iter);
      }
      // Send it only to _iter
      else
        ok = SendBufferToClient(b, cmd, *u_iter);
    }
  }
}


bool CChatManager::SendBufferToClient(CBuffer *b, unsigned char cmd, CChatUser *u)
{
  CBuffer b_out(128);

  // If the socket was closed, ignore the key event
  if (u->state != CHAT_STATE_CONNECTED || u->sock.Descriptor() == -1)
    return true;

  if (u->sock.Version() >= 6)
  {
    b_out.PackChar(0);
    b_out.PackChar(cmd);
    //the change font command size includes the following character
    b_out.PackUnsignedLong(b->getDataSize() + ((cmd == CHAT_FONTxFAMILY)?1:0));
    b_out.Pack(b->getDataStart(), b->getDataSize());
  }
  else
  {
    b_out.PackChar(cmd);
    b_out.Pack(b->getDataStart(), b->getDataSize());
  }

  if (!u->sock.SendRaw(&b_out))
  {
    gLog.warning(tr("%sChat: Send error:\n%s%s\n"), L_WARNxSTR, L_BLANKxSTR,
        u->sock.errorStr().c_str());
    CloseClient(u);
    return false;
  }

  b_out.setDataPosWrite(b_out.getDataStart());
  b_out.setDataPosRead(b_out.getDataStart());

  return true;
}


void CChatManager::SendBuffer_Raw(CBuffer *b)
{
  ChatUserList::iterator iter;
  CChatUser *u = NULL;
  bool ok = false;
  while (!ok)
  {
    ok = true;
    for (iter = chatUsers.begin(); iter != chatUsers.end(); ++iter)
    {
      u = *iter;

      // If the socket was closed, ignore the key event
      if (u->state != CHAT_STATE_CONNECTED || u->sock.Descriptor() == -1) continue;

      if (!u->sock.SendRaw(b))
      {
        gLog.warning(tr("%sChat: Send error:\n%s%s\n"), L_WARNxSTR, L_BLANKxSTR,
            u->sock.errorStr().c_str());
        CloseClient(u);
        ok = false;
        break;
      }
    }
  }
}


void CChatManager::SendNewline()
{
  CBuffer buf(1);
  buf.PackChar(CHAT_NEWLINE);
  SendBuffer_Raw(&buf);
}


void CChatManager::SendBackspace()
{
  CBuffer buf(1);
  buf.PackChar(CHAT_BACKSPACE);
  SendBuffer_Raw(&buf);
}


void CChatManager::SendBeep()
{
  CBuffer buf;
  SendBuffer(&buf, CHAT_BEEP);
}


void CChatManager::SendLaugh()
{
  CBuffer buf;
  SendBuffer(&buf, CHAT_LAUGH);
}


void CChatManager::SendCharacter(char c)
{
  CBuffer buf(1);
  Licq::gTranslator.ClientToServer(c);
  buf.PackChar(c);
  SendBuffer_Raw(&buf);
}


void CChatManager::SendKick(const char* id)
{
  unsigned long _nUin = strtoul(id, NULL, 10);

  // Take care of the vote stuff now
  // The user we are kicking automatically is a no vote
  // And we are an automatic yes vote
  SVoteInfo *vote = new SVoteInfo;
  vote->nUin = _nUin;
  vote->nNumUsers = ConnectedUsers();
  vote->nYes = 1;
  vote->nNo = 1;
  voteInfo.push_back(vote);

  // Send the packet to all connected clients except the one we are
  // requesting to kick
  CBuffer buf(4);
  buf.PackUnsignedLong(_nUin);
  SendBuffer(&buf, CHAT_KICK, id, true);
}


void CChatManager::SendKickNoVote(const char *id)
{
  Licq::UserId userId(id, LICQ_PPID);
  unsigned long _nUin = strtoul(id, NULL, 10);

  // Tell everyone that this user has been kicked
  CBuffer buf_TellAll(6);
  buf_TellAll.PackUnsignedLong(_nUin);
  buf_TellAll.PackChar(0x02);
  buf_TellAll.PackChar(0x01);
  SendBuffer(&buf_TellAll, CHAT_KICKxPASS, id, true);

  // They don't know if there was a vote or not, they just see they've been kicked
  CBuffer buf(2);
  buf.PackChar(0x02);
  buf.PackChar(0x01);
  SendBuffer(&buf, CHAT_KICKxYOU, id, false);

  // And close the connection to the kicked user
  ChatUserList::iterator iter;
  for (iter = chatUsers.begin(); iter != chatUsers.end(); ++iter)
  {
    if((*iter)->userId() == userId)
      break;
  }

  if (iter == chatUsers.end())   return;

  CBuffer bye(4);
  SendBuffer(&bye, CHAT_DISCONNECTIONxKICKED, id, false);

  CloseClient(*iter);
}


// Only the person requesting the kick should receive it..
// but what if more than one person has been asked to be kicked by
// different people?  It shouldn't send a reply to everyone..
// Mirabilis ICQ just ignores it if it's not expecting it, i believe
void CChatManager::SendVoteYes(unsigned long _nUin)
{
  CBuffer buf(4);
  buf.PackUnsignedLong(_nUin);
  SendBuffer(&buf, CHAT_KICKxYES);
}


void CChatManager::SendVoteNo(unsigned long _nUin)
{
  CBuffer buf(4);
  buf.PackUnsignedLong(_nUin);
  SendBuffer(&buf, CHAT_KICKxNO);
}


void CChatManager::FocusIn()
{
  CBuffer buf;
  SendBuffer(&buf, CHAT_FOCUSxIN);

  m_bFocus = true;
}


void CChatManager::FocusOut()
{
  CBuffer buf;
  SendBuffer(&buf, CHAT_FOCUSxOUT);

  m_bFocus = false;
}


void CChatManager::Sleep(bool s)
{
  CBuffer buf;
  SendBuffer(&buf, s ? CHAT_SLEEPxON : CHAT_SLEEPxOFF);

  m_bSleep = s;
}


void CChatManager::changeFontFamily(const string& fontFamily, unsigned char enc,
                                    unsigned char style)
{
  //CPChat_ChangeFontFamily p(fontFamily);
  //SendPacket(&p);

  CBuffer buf(fontFamily.size() + 5);
  buf.PackString(fontFamily.c_str());
  buf.PackChar(enc);
  buf.PackChar(style);
  SendBuffer(&buf, CHAT_FONTxFAMILY);

  myFontFamily = fontFamily;
  m_nFontEncoding = enc;
  m_nFontStyle = style;
}


void CChatManager::ChangeFontSize(unsigned short s)
{
  //CPChat_ChangeFontSize p(s);
  //SendPacket(&p);

  CBuffer buf(4);
  buf.PackUnsignedLong(s);
  SendBuffer(&buf, CHAT_FONTxSIZE);

  m_nFontSize = s;
}


void CChatManager::ChangeFontFace(bool b, bool i, bool u, bool s)
{
  //CPChat_ChangeFontFace p(b, i, u);
  //SendPacket(&p);

  m_nFontFace = FONT_PLAIN;
  if (b) m_nFontFace |= FONT_BOLD;
  if (i) m_nFontFace |= FONT_ITALIC;
  if (u) m_nFontFace |= FONT_UNDERLINE;
  if (s) m_nFontFace |= FONT_STRIKEOUT;

  CBuffer buf(4);
  buf.PackUnsignedLong(m_nFontFace);
  SendBuffer(&buf, CHAT_FONTxFACE);
}


void CChatManager::ChangeColorFg(int r, int g, int b)
{
  //CPChat_ChangeColorFg p(r, g, b);
  //SendPacket(&p);

  CBuffer buf(4);
  buf.PackChar(r);
  buf.PackChar(g);
  buf.PackChar(b);
  buf.PackChar(0);
  SendBuffer(&buf, CHAT_COLORxFG);

  m_nColorFore[0] = r;
  m_nColorFore[1] = g;
  m_nColorFore[2] = b;
}


void CChatManager::ChangeColorBg(int r, int g, int b)
{
  //CPChat_ChangeColorBg p(r, g, b);
  //SendPacket(&p);

  CBuffer buf(4);
  buf.PackChar(r);
  buf.PackChar(g);
  buf.PackChar(b);
  buf.PackChar(0);
  SendBuffer(&buf, CHAT_COLORxBG);

  m_nColorBack[0] = r;
  m_nColorBack[1] = g;
  m_nColorBack[2] = b;
}


//----CChatManager::CloseChat------------------------------------------------
void CChatManager::CloseChat()
{
  // Close the chat thread
  // We must do it before trying to close the socket to avoid
  // the chat thread trying to close the socket itself once
  // it notices it cannot read from it
  myThreadPipe.putChar('X');
  if (m_bThreadCreated)
    pthread_join(thread_chat, NULL);
  m_bThreadCreated = false;

  CChatUser *u = NULL;
  CBuffer buf;
  SendBuffer(&buf, CHAT_DISCONNECTION);
  while (chatUsers.size() > 0)
  {
    u = chatUsers.front();
    sockman.CloseSocket(u->sock.Descriptor(), false, false);
    u->state = CHAT_STATE_DISCONNECTED;
    chatUsersClosed.push_back(u);
    chatUsers.pop_front();
    // Alert the plugin
    PushChatEvent(new CChatEvent(CHAT_DISCONNECTION, u));
  }

  sockman.CloseSocket(chatServer.Descriptor(), false, false);
}


//----CChatManager::FinishKickVote-------------------------------------------
void CChatManager::FinishKickVote(VoteInfoList::iterator iter, bool bPassed)
{
  char voteId[16];
  snprintf(voteId, 16, "%lu", (*iter)->nUin);
  Licq::UserId userId(voteId, LICQ_PPID);

  // Find the person we are kicking in the ChatUserList
  ChatUserList::iterator userIter;
  for (userIter = chatUsers.begin(); userIter != chatUsers.end(); ++userIter)
  {
    if ((*userIter)->userId() == userId)
      break;
  }

  // User no longer in the chat
  if (userIter == chatUsers.end())
  {
    delete *iter;
    voteInfo.erase(iter);
    return;
  }

  // Send a CHAT_KICKxPASS or CHAT_KICKxFAIL to everyone but the person
  // that is attempting to be kicked.
  CBuffer buf(6);
  buf.PackUnsignedLong((*iter)->nUin);
  buf.PackChar((*iter)->nYes);
  buf.PackChar((*iter)->nNo);

  if (bPassed)
    SendBuffer(&buf, CHAT_KICKxPASS, voteId, true);
  else
    SendBuffer(&buf, CHAT_KICKxFAIL, voteId, true);

  // Send the person a notice if they were kicked
  if (bPassed)
  {
    SendBuffer(&buf, CHAT_KICKxYOU, voteId, false);
    CloseClient(*userIter);
  }

  delete *iter;
  voteInfo.erase(iter);
}


//----CChatManager::CloseClient----------------------------------------------
void CChatManager::CloseClient(CChatUser *u)
{
  // Remove the user from the user list
  ChatUserList::iterator iter;
  for (iter = chatUsers.begin(); iter != chatUsers.end(); ++iter)
  {
    if (u == *iter)
    {
      sockman.CloseSocket(u->sock.Descriptor(), false, false);
      chatUsers.erase(iter);
      u->state = CHAT_STATE_DISCONNECTED;
      chatUsersClosed.push_back(u);
      break;
    }
  }

  // Alert the plugin
  PushChatEvent(new CChatEvent(CHAT_DISCONNECTION, u));
}


//-----CChatManager::ClientsStr----------------------------------------------
string CChatManager::clientsString() const
{
  string sz;

  ChatUserList::const_iterator iter;
  for (iter = chatUsers.begin(); iter != chatUsers.end(); ++iter)
  {
    if (!sz.empty())
      sz += ", ";
    if ((*iter)->name().empty())
      sz += (*iter)->userId().accountId();
    else
      sz += (*iter)->name();
  }
  return sz;
}



void *ChatManager_tep(void *arg)
{
  CChatManager *chatman = (CChatManager *)arg;

  fd_set f;
  int l, nSocketsAvailable, nCurrentSocket;

  if (chatman->m_pChatClient)
  {
    if (!chatman->ConnectToChat(chatman->m_pChatClient))
    {
      chatman->PushChatEvent(new CChatEvent(CHAT_ERRORxCONNECT, NULL));
      return NULL;
    }
    chatman->m_pChatClient = 0;
  }

  while (true)
  {
    f = chatman->sockman.socketSet();
    l = chatman->sockman.LargestSocket() + 1;

    // Add the new socket pipe descriptor
    FD_SET(chatman->myThreadPipe.getReadFd(), &f);
    if (chatman->myThreadPipe.getReadFd() >= l)
      l = chatman->myThreadPipe.getReadFd() + 1;

    nSocketsAvailable = select(l, &f, NULL, NULL, NULL);

    nCurrentSocket = 0;
    while (nSocketsAvailable > 0 && nCurrentSocket < l)
    {
      if (FD_ISSET(nCurrentSocket, &f))
      {
        // New socket event ----------------------------------------------------
        if (nCurrentSocket == chatman->myThreadPipe.getReadFd())
        {
          char buf = chatman->myThreadPipe.getChar();
          if (buf == 'R')
          {
            DEBUG_THREADS("[ChatManager_tep] Reloading socket info.\n");
          }
          else if (buf == 'X')
          {
            DEBUG_THREADS("[ChatManager_tep] Exiting.\n");
            pthread_exit(NULL);
          }
        }

        // Connection on the server port ---------------------------------------
        else if (nCurrentSocket == chatman->chatServer.Descriptor())
        {
          if (chatman->sockman.Num() >= MAX_CONNECTS)
          {
            // Too many sockets, drop this one
            gLog.warning(tr("%sToo many connected clients, rejecting new connection.\n"), L_WARNxSTR);
          }
          else
          {
            CChatUser *u = new CChatUser;
            u->m_pClient = new CChatClient;

            if (chatman->chatServer.RecvConnection(u->sock))
            {
              chatman->sockman.AddSocket(&u->sock);
              chatman->sockman.DropSocket(&u->sock);

              u->state = CHAT_STATE_HANDSHAKE;
              chatman->chatUsers.push_back(u);
              gLog.info(tr("%sChat: Received connection.\n"), L_TCPxSTR);
            }
            else
            {
              delete u;
              gLog.error(tr("%sChat: Unable to receive new connection.\n"), L_ERRORxSTR);
            }
          }
        }

        // Message from connected socket----------------------------------------
        else
        {
          CChatUser *u = chatman->FindChatUser(nCurrentSocket);
          if (u == NULL)
          {
            gLog.warning(tr("%sChat: No user owns socket %d.\n"), L_WARNxSTR, nCurrentSocket);
          }
          else
          {
            pthread_mutex_lock(&u->mutex);
            u->sock.Lock();
            bool ok = true;

            if (u->state != CHAT_STATE_CONNECTED)
            {
              ok = chatman->ProcessPacket(u);
            }

            else  // Raw character being received
            {
              ok = chatman->ProcessRaw(u);
            }

            u->sock.Unlock();
            if (!ok) chatman->CloseClient(u);
            pthread_mutex_unlock(&u->mutex);
          }
        }

        nSocketsAvailable--;
      }
      nCurrentSocket++;
    }
  }
  return NULL;
}

void *ChatWaitForSignal_tep(void *arg)
{
  pthread_detach(pthread_self());

  struct SChatReverseConnectInfo *rc = (struct SChatReverseConnectInfo *)arg;
  pthread_mutex_t *cancel_mutex = &CChatManager::waiting_thread_cancel_mutex;

  pthread_mutex_lock(cancel_mutex);
  pthread_cleanup_push(ChatWaitForSignal_cleanup, arg);
    pthread_testcancel();
  pthread_cleanup_pop(0);
  pthread_mutex_unlock(cancel_mutex);

  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  gLog.info("%sChat: Waiting for reverse connection.\n", L_TCPxSTR);
  bool bConnected = gIcqProtocol.waitForReverseConnection(rc->nId, rc->u->userId());
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

  pthread_mutex_lock(cancel_mutex);
  pthread_cleanup_push(ChatWaitForSignal_cleanup, arg);
    pthread_testcancel();
  pthread_cleanup_pop(0);

  if (bConnected || !rc->bTryDirect)
  {
    if (!bConnected && rc->m->chatUsers.empty())
      rc->m->PushChatEvent(new CChatEvent(CHAT_ERRORxCONNECT, NULL));


    pthread_mutex_lock(&rc->m->thread_list_mutex);
    ThreadList::iterator iter;
    for (iter = rc->m->waitingThreads.begin();
                                 iter != rc->m->waitingThreads.end(); ++iter)
    {
      if (pthread_equal(*iter, pthread_self()))
      {
        rc->m->waitingThreads.erase(iter);
        break;
      }
    }
    pthread_mutex_unlock(&rc->m->thread_list_mutex);
    pthread_mutex_unlock(cancel_mutex);

    delete rc->u->m_pClient;
    delete rc->u;
    delete rc;
    pthread_exit(NULL);
  }

  pthread_mutex_unlock(cancel_mutex);

  bool bSendIntIp = false;
  {
    Licq::UserReadGuard temp_user(rc->u->userId());
    if (temp_user.isLocked())
      bSendIntIp = temp_user->SendIntIp();
  }

    pthread_mutex_lock(cancel_mutex);
    pthread_cleanup_push(ChatWaitForSignal_cleanup, arg);
      pthread_testcancel();
    pthread_cleanup_pop(0);

    unsigned long nIp = rc->u->m_pClient->m_nIp;
    unsigned long nIntIp = rc->u->m_pClient->m_nIntIp;
    unsigned short nPort = rc->u->m_pClient->m_nPort;
    pthread_mutex_unlock(cancel_mutex);

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    gLog.info("%sChat: Reverse connection failed, trying direct.\n", L_TCPxSTR);
    bool bSuccess = gIcqProtocol.OpenConnectionToUser("chat", nIp, nIntIp, &rc->u->sock,
                                            nPort, bSendIntIp);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    if (bSuccess)
    {
      pthread_mutex_lock(cancel_mutex);
      pthread_cleanup_push(ChatWaitForSignal_cleanup, arg);
        pthread_testcancel();
      pthread_cleanup_pop(0);

      if (rc->m->SendChatHandshake(rc->u))
      {
        rc->m->chatUsers.push_back(rc->u);

        pthread_mutex_lock(&rc->m->thread_list_mutex);
        ThreadList::iterator iter;
        for (iter = rc->m->waitingThreads.begin();
                                   iter != rc->m->waitingThreads.end(); ++iter)
        {
          if (pthread_equal(*iter, pthread_self()))
          {
            rc->m->waitingThreads.erase(iter);
            break;
          }
        }
        pthread_mutex_unlock(&rc->m->thread_list_mutex);
        pthread_mutex_unlock(cancel_mutex);

        delete rc;
        pthread_exit(NULL);
      }

      pthread_mutex_unlock(cancel_mutex);
  }

  pthread_mutex_lock(cancel_mutex);
  pthread_cleanup_push(ChatWaitForSignal_cleanup, arg);
    pthread_testcancel();
  pthread_cleanup_pop(0);

  if (rc->m->chatUsers.empty())
    rc->m->PushChatEvent(new CChatEvent(CHAT_ERRORxCONNECT, NULL));

  pthread_mutex_lock(&rc->m->thread_list_mutex);
  ThreadList::iterator iter2;
  for (iter2 = rc->m->waitingThreads.begin();
                                  iter2 != rc->m->waitingThreads.end(); ++iter2)
  {
    if (pthread_equal(*iter2, pthread_self()))
    {
      rc->m->waitingThreads.erase(iter2);
      break;
    }
  }
  pthread_mutex_unlock(&rc->m->thread_list_mutex);
  pthread_mutex_unlock(cancel_mutex);


  delete rc->u->m_pClient;
  delete rc->u;
  delete rc;
  
  pthread_exit(NULL);
}

void ChatWaitForSignal_cleanup(void *arg)
{
  struct SChatReverseConnectInfo *rc = (struct SChatReverseConnectInfo *)arg;

  delete rc->u->m_pClient;
  delete rc->u;
  delete rc;
  pthread_mutex_unlock(&CChatManager::waiting_thread_cancel_mutex);
}

CChatManager *CChatManager::FindByPort(unsigned short p)
{
  pthread_mutex_lock(&cmList_mutex);
  ChatManagerList::iterator iter;
  CChatManager *cm = NULL;
  for (iter = cmList.begin(); iter != cmList.end(); ++iter)
  {
    if ( (*iter)->LocalPort() == p)
    {
      cm = *iter;
      break;
    }    
  }
  pthread_mutex_unlock(&cmList_mutex);
  return cm;
}


CChatManager::~CChatManager()
{
  // cancel all waiting threads first
  pthread_mutex_lock(&waiting_thread_cancel_mutex);
  pthread_mutex_lock(&thread_list_mutex);
  ThreadList::iterator t_iter;
  for (t_iter = waitingThreads.begin(); t_iter != waitingThreads.end();)
  {
    pthread_cancel(*t_iter);
    t_iter = waitingThreads.erase(t_iter);
  }
  pthread_mutex_unlock(&thread_list_mutex);
  pthread_mutex_unlock(&waiting_thread_cancel_mutex);

  CloseChat();

  // Delete all the users
  CChatUser *u = NULL;
  while (chatUsersClosed.size() > 0)
  {
    u = chatUsersClosed.front();
    if (u->m_pClient)
      delete u->m_pClient;
    delete u;
    chatUsersClosed.pop_front();
  }

  // Delete any pending events
  CChatEvent *e = NULL;
  while (chatEvents.size() > 0)
  {
    e = chatEvents.front();
    delete e;
    chatEvents.pop_front();
  }

  pthread_mutex_lock(&cmList_mutex);
  ChatManagerList::iterator iter;
  for (iter = cmList.begin(); iter != cmList.end(); ++iter)
  {
    if (*iter == this) break;
  }
  if (iter != cmList.end()) cmList.erase(iter);
  pthread_mutex_unlock(&cmList_mutex);
}

