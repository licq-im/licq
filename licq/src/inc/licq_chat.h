#ifndef CHAT_H
#define CHAT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "licq_packets.h"

const unsigned long FONT_PLAIN     = 0x00000000;
const unsigned long FONT_BOLD      = 0x00000001;
const unsigned long FONT_ITALIC    = 0x00000002;
const unsigned long FONT_UNDERLINE = 0x00000004;
const unsigned char CHAT_COLORxFG    = 0x00;
const unsigned char CHAT_COLORxBG    = 0x01;
const unsigned char CHAT_BEEP        = 0x07;
const unsigned char CHAT_BACKSPACE   = 0x08;
const unsigned char CHAT_NEWLINE     = 0x0D;
const unsigned char CHAT_FONTxFAMILY = 0x10;
const unsigned char CHAT_FONTxFACE   = 0x11;
const unsigned char CHAT_FONTxSIZE   = 0x12;


//=====Chat=====================================================================
class CPacketChat : public CPacket
{
public:
  virtual const unsigned long  getSequence()   { return 0; };
  virtual const unsigned short SubSequence()   { return 0; };
  virtual const unsigned short getCommand()    { return 0; };
  virtual const unsigned short getSubCommand() { return 0; };
protected:
   void InitBuffer();
};


//-----ChatColor----------------------------------------------------------------
  /* 64 00 00 00 FD FF FF FF 50 A5 82 00 08 00 38 35 36 32 30 30 30 00 62 3D
     FF FF FF 00 00 00 00 00 00 */
class CPChat_Color : public CPacketChat  // First info packet after handshake
{
public:
  CPChat_Color(const char *szLocalName, unsigned short nLocalPort,
               int nColorForeRed, int nColorForeGreen, int nColorForeBlue,
               int nColorBackRed, int nColorBackBlue, int nColorBackGreen);
  CPChat_Color(CBuffer &);

  // Accessors
  const char *Name() { return m_szName; }
  unsigned long Uin() { return m_nUin; }
  unsigned short Port() { return m_nPort; }
  int ColorForeRed() { return m_nColorForeRed; }
  int ColorForeGreen() { return m_nColorForeGreen; }
  int ColorForeBlue() { return m_nColorForeBlue; }
  int ColorBackRed() { return m_nColorBackRed; }
  int ColorBackGreen() { return m_nColorBackGreen; }
  int ColorBackBlue() { return m_nColorBackBlue; }

  virtual ~CPChat_Color() { if (m_szName != NULL) free(m_szName); }
protected:
  unsigned long m_nUin;
  char *m_szName;
  unsigned short m_nPort;
  int m_nColorForeRed;
  int m_nColorForeGreen;
  int m_nColorForeBlue;
  int m_nColorBackRed;
  int m_nColorBackGreen;
  int m_nColorBackBlue;
};




//-----ChatColorFont------------------------------------------------------------
class CChatClient
{
public:
  CChatClient();
  CChatClient(ICQUser *);

  // Initialize from the handshake buffer (does not set the session
  // or port fields however
  bool LoadFromHandshake(CBuffer &);

  unsigned long m_nVersion;
  unsigned short m_nPort;
  unsigned long m_nUin;
  unsigned long m_nIp;
  unsigned long m_nRealIp;
  char m_nMode;
  unsigned short m_nSession;
  unsigned long m_nHandshake;

protected:
  CChatClient(CBuffer &);
  bool LoadFromBuffer(CBuffer &);

friend class CPChat_ColorFont;
};

typedef list<CChatClient> ChatClientList;
typedef list<CChatClient *> ChatClientPList;


/* 64 00 00 00 50 A5 82 00 08 00 38 35 36 32 30 30 30 00 FF FF FF 00 00 00
   00 00 03 00 00 00 DB 64 00 00 CF 60 AD 95 CF 60 AD 95 04 75 5A 0C 00 00
   00 00 00 00 00 08 00 43 6F 75 72 69 65 72 00 00 00 00 */
class CPChat_ColorFont : public CPacketChat  // Second info packet after handshake
{
public:
  CPChat_ColorFont(const char *szLocalName, unsigned short nLocalPort,
     unsigned short nSession,
     int nColorForeRed, int nColorForeGreen, int nColorForeBlue,
     int nColorBackRed, int nColorBackBlue, int nColorBackGreen,
     unsigned long nFontSize,
     bool bFontBold, bool bFontItalic, bool bFontUnderline,
     const char *szFontFamily,
     ChatClientPList &clientList);

  CPChat_ColorFont(CBuffer &);

  virtual ~CPChat_ColorFont()  {  if (m_szName != NULL) free (m_szName); if (m_szFontFamily != NULL) free(m_szFontFamily); }

  // Accessors
  const char *Name() { return m_szName; }
  unsigned long Uin() { return m_nUin; }
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
  const char *FontFamily() { return m_szFontFamily; }
  ChatClientList &ChatClients()  { return chatClients; }

protected:
  unsigned long m_nUin;
  unsigned short m_nSession;
  char *m_szName;
  int m_nColorForeRed;
  int m_nColorForeGreen;
  int m_nColorForeBlue;
  int m_nColorBackRed;
  int m_nColorBackGreen;
  int m_nColorBackBlue;
  unsigned short m_nPort;
  unsigned long m_nFontSize;
  unsigned long m_nFontFace;
  char *m_szFontFamily;
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
               const char *szFontFamily);
   CPChat_Font(CBuffer &);
   virtual ~CPChat_Font()  { if (m_szFontFamily != NULL) free (m_szFontFamily); }

  unsigned short Port() { return m_nPort; }
  unsigned short Session() { return m_nSession; }
  unsigned long FontSize() { return m_nFontSize; }
  bool FontBold() { return m_nFontFace & FONT_BOLD; }
  bool FontItalic() { return m_nFontFace & FONT_ITALIC; }
  bool FontUnderline() { return m_nFontFace & FONT_UNDERLINE; }
  const char *FontFamily() { return m_szFontFamily; }

protected:
  unsigned short m_nPort;
  unsigned short m_nSession;
  unsigned long m_nFontSize;
  unsigned long m_nFontFace;
  char *m_szFontFamily;
};







class CPChat_ChangeFontFamily : public CPacketChat
{
public:
  CPChat_ChangeFontFamily(const char *szFamily);
  CPChat_ChangeFontFamily(CBuffer &);
  virtual ~CPChat_ChangeFontFamily() { if (m_szFontFamily != NULL) free(m_szFontFamily); }

  const char *FontFamily()  { return m_szFontFamily; }

protected:
  char *m_szFontFamily;
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



#endif


