#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "licq_chat.h"
#include "support.h"


//=====Chat=====================================================================
void CPacketChat::InitBuffer()
{
  buffer = new CBuffer(m_nSize);
}

//-----ChatColor----------------------------------------------------------------
CPChat_Color::CPChat_Color(const char *_sLocalName, unsigned short _nLocalPort,
   int nColorForeRed, int nColorForeGreen, int nColorForeBlue, int nColorBackRed,
   int nColorBackBlue, int nColorBackGreen)
{
  m_szName = NULL;
  m_nPort = _nLocalPort;
  m_nUin = gUserManager.OwnerUin();
  m_nColorForeRed = nColorForeRed;
  m_nColorForeGreen = nColorForeGreen;
  m_nColorForeBlue = nColorForeBlue;
  m_nColorBackRed = nColorBackRed;
  m_nColorBackGreen = nColorBackGreen;
  m_nColorBackBlue = nColorBackBlue;

  m_nSize = 10 + strlen(_sLocalName) + 16;
  InitBuffer();

  buffer->PackUnsignedLong(0x64);
  buffer->PackUnsignedLong(-ICQ_VERSION_TCP);
  buffer->PackUnsignedLong(gUserManager.OwnerUin());
  buffer->PackString(_sLocalName);
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
  char buf[128];

  b.UnpackUnsignedLong();
  b.UnpackUnsignedLong();
  m_nUin = b.UnpackUnsignedLong();
  m_szName = strdup(b.UnpackString(buf));
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


//-----ChatColorFont----------------------------------------------------------------
CChatClient::CChatClient()
{
  m_nVersion = m_nUin = m_nIp = m_nRealIp = m_nPort = m_nMode
     = m_nSession = m_nHandshake = 0;
}


CChatClient::CChatClient(ICQUser *u)
{
  m_nVersion = u->Version();
  m_nUin = u->Uin();
  m_nIp = u->Ip();
  m_nRealIp = u->RealIp();
  m_nMode = u->Mode();
  m_nSession = 0;
  m_nHandshake = 0x64;

  // These will still need to be set
  m_nPort = 0;
  m_nSession = 0;
}


CChatClient::CChatClient(CBuffer &b)
{
  LoadFromBuffer(b);
}


bool CChatClient::LoadFromBuffer(CBuffer &b)
{
  m_nVersion = b.UnpackUnsignedLong();
  m_nPort = b.UnpackUnsignedShort();
  b.UnpackUnsignedShort();
  m_nUin = b.UnpackUnsignedLong();
  m_nIp = b.UnpackUnsignedLong();
  m_nRealIp = b.UnpackUnsignedLong();
  b.UnpackUnsignedShort();
  m_nMode = b.UnpackChar();
  m_nSession = b.UnpackUnsignedShort();
  m_nHandshake = b.UnpackUnsignedLong();

  return true;
}


bool CChatClient::LoadFromHandshake(CBuffer &b)
{
  //b.Reset

  if ((unsigned char)b.UnpackChar() != ICQ_CMDxTCP_HANDSHAKE) return false;

  m_nVersion = b.UnpackUnsignedLong();
  b.UnpackUnsignedLong();
  m_nUin = b.UnpackUnsignedLong();
  m_nIp = b.UnpackUnsignedLong();
  m_nRealIp = b.UnpackUnsignedLong();
  m_nMode = b.UnpackChar();
  m_nHandshake = 0x64;

  // These will still need to be set
  m_nPort = 0;
  m_nSession = 0;

  return true;
}


CPChat_ColorFont::CPChat_ColorFont(const char *szLocalName, unsigned short nLocalPort,
   unsigned short nSession,
   int nColorForeRed, int nColorForeGreen, int nColorForeBlue, int nColorBackRed,
   int nColorBackBlue, int nColorBackGreen,
   unsigned long nFontSize,
   bool bFontBold, bool bFontItalic, bool bFontUnderline,
   const char *szFontFamily,
   ChatClientPList &clientList)
{
  m_szName = NULL;
  m_nPort = nLocalPort;
  m_nUin = gUserManager.OwnerUin();
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
  m_szFontFamily = NULL;

  m_nSize = 10 + strlen(szLocalName) + 35 + strlen(szFontFamily) + 4
            + clientList.size() * (sizeof(CChatClient) + 2);
  InitBuffer();

  buffer->PackUnsignedLong(0x64);
  buffer->PackUnsignedLong(gUserManager.OwnerUin());
  buffer->PackString(szLocalName);
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
  buffer->PackString(szFontFamily);
  buffer->PackUnsignedShort(0x0002);
  buffer->PackChar(clientList.size());

  ChatClientPList::iterator iter;
  for (iter = clientList.begin(); iter != clientList.end(); iter++)
  {
    buffer->PackUnsignedLong((*iter)->m_nVersion);
    buffer->PackUnsignedLong((*iter)->m_nPort);
    buffer->PackUnsignedLong((*iter)->m_nUin);
    buffer->PackUnsignedLong((*iter)->m_nIp);
    buffer->PackUnsignedLong((*iter)->m_nRealIp);
    buffer->PackUnsignedShort(ReversePort((*iter)->m_nPort));
    buffer->PackChar((*iter)->m_nMode);
    buffer->PackUnsignedShort((*iter)->m_nSession);
    buffer->PackUnsignedLong((*iter)->m_nHandshake);
  }
}


CPChat_ColorFont::CPChat_ColorFont(CBuffer &b)
{
  char buf[128];

  b.UnpackUnsignedLong();
  m_nUin = b.UnpackUnsignedLong();
  m_szName = strdup(b.UnpackString(buf));
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
  m_szFontFamily = strdup(b.UnpackString(buf));
  b.UnpackUnsignedShort();

  // Read out client packets
  unsigned short nc = b.UnpackChar();
  for (unsigned short i = 0; i < nc; i++)
  {
    chatClients.push_back(CChatClient(b));
  }

}



//-----ChatFont---------------------------------------------------------------------
CPChat_Font::CPChat_Font(unsigned short nLocalPort, unsigned short nSession,
                         unsigned long nFontSize,
                         bool bFontBold, bool bFontItalic, bool bFontUnderline,
                         const char *szFontFamily)
{
  m_nPort = nLocalPort;
  m_nSession = nSession;
  m_nFontSize = nFontSize;
  m_nFontFace = FONT_PLAIN;
  if (bFontBold) m_nFontFace |= FONT_BOLD;
  if (bFontItalic) m_nFontFace |= FONT_ITALIC;
  if (bFontUnderline) m_nFontFace |= FONT_UNDERLINE;
  m_szFontFamily = NULL;

  m_nSize = 29 + strlen(szFontFamily) + 4;
  InitBuffer();

  buffer->PackUnsignedLong(ICQ_VERSION_TCP);
  buffer->PackUnsignedLong(m_nPort);
  buffer->PackUnsignedLong(s_nLocalIp);
  buffer->PackUnsignedLong(s_nRealIp);
  buffer->PackChar(s_nMode);
  buffer->PackUnsignedShort(nSession);//0x5A89);
  buffer->PackUnsignedLong(m_nFontSize);
  buffer->PackUnsignedLong(m_nFontFace);
  buffer->PackString(szFontFamily);
  buffer->PackUnsignedShort(0x0002);
  buffer->PackChar(0);
}

CPChat_Font::CPChat_Font(CBuffer &b)
{
  char buf[128];

  b.UnpackUnsignedLong();
  m_nPort = b.UnpackUnsignedLong();
  b.UnpackUnsignedLong();
  b.UnpackUnsignedLong();
  b.UnpackChar();
  m_nSession = b.UnpackUnsignedShort();
  m_nFontSize = b.UnpackUnsignedLong();
  m_nFontFace = b.UnpackUnsignedLong();
  m_szFontFamily = strdup(b.UnpackString(buf));
}


//-----CPChat_ChangeFontFamily-----------------------------------------------
CPChat_ChangeFontFamily::CPChat_ChangeFontFamily(const char *szFamily)
{
  m_szFontFamily = NULL;

  m_nSize = strlen_safe(szFamily) + 6;
  InitBuffer();

  buffer->PackChar(CHAT_FONTxFAMILY);
  buffer->PackString(szFamily);
  buffer->PackUnsignedShort(0x2200);
  // 0x2200 west
  // 0x22a2 turkey
  // 0x22cc cyrillic
  // 0x22a1 greek
  // 0x22ba baltic
}


CPChat_ChangeFontFamily::CPChat_ChangeFontFamily(CBuffer &b)
{
  char buf[128];

  //b.UnpackChar(); // CHAT_CHANGExFONT
  b.UnpackString(buf);
  b.UnpackUnsignedShort();  // Charset?

  m_szFontFamily = strdup(buf);
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
