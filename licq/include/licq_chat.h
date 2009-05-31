#ifndef CHAT_H
#define CHAT_H

#include <cstdlib>
#include <deque>
#include <list>

#include "licq_packets.h"
#include "licq_socket.h"
#include "licq_types.h"
class CICQDaemon;

// Define for marking functions as deprecated
#ifndef LICQ_DEPRECATED
# if defined(__GNUC__) && !defined(__INTEL_COMPILER) && (__GNUC__ - 0 > 3 || (__GNUC__ - 0 == 3 && __GNUC_MINOR__ - 0 >= 2))
#  define LICQ_DEPRECATED __attribute__ ((__deprecated__))
# elif defined(_MSC_VER) && (_MSC_VER >= 1300)
#  define LICQ_DEPRECATED __declspec(deprecated)
# else
#  define LICQ_DEPRECATED
# endif
#endif


/*----------------------
 * Licq Chat Module
 *
 * ICQ Chat is fairly easy to implement using the Licq ChatManager.
 * There are three parts, initiating a chat, accepting a chat, and
 * managing a chat in session.
 * 1. Initiating
 *
 *  <to be filled in later>
 *
 *
 * 2. Accepting
 *
 *  < to be filled in later>
 *
 *
 * 3. Managing
 *
 *  Once the plugin has created whatever chat dialog it needs, the following
 *  steps should be performed to start the actual chat session:
 *  a) Create a new instance of CChatManager.  This class encapsulates
 *     all the necessary handshaking and packet processing to both connect
 *     to and receive chat clients.
 *     The class takes as arguments a pointer to the licq daemon, the Uin
 *     we are first connecting to or first receiving a connection from,
 *     and the initial GUI settings (colors, font).
 *  b) Attach to the chat manager pipe.  The ChatManager contains a function
 *     Pipe() which will return a socket to listen on for messages.  When
 *     something interesting happens during the chat (a new user connects,
 *     a user disconnects, a user changes colors, a user types a
 *     character...) a single byte is written to this socket and the relevant
 *     CChatEvent is pushed onto the Chat Event queue.  Hence if there is
 *     data to be read on the Pipe(), then pop it off (it can be ignored) and
 *     then call CChatManager::PopChatEvent() which will return the
 *     CChatEvent.  This is just like the plugin pipe used to send messages
 *     from the daemon to the plugin.
 *  c) Call either CChatManager::StartAsServer() or
 *     CChatManager::StartAsClient(<port>) to tell the chat manager to start
 *     doing stuff.
 *
 *  Now it is just a matter of waiting on the Pipe() until an event occurs,
 *  which will be an event of type CHAT_CONNECTION indicating that a new
 *  user has joined the chat.  Writing to the chat session is done by
 *  calling any of the CChatManager::Send<...> functions (such as
 *  SendCharacter(char)).
 *
 *  When a user types a newline, the CHAT_NEWLINE
 *  event is sent, and the CChatEvent::Data() element will contain the entire
 *  past line of text typed.  This can be used to implement IRC mode
 *  efficiently (note that since each character is also passed to the plugin
 *  as it is typed, the line sent in a CHAT_NEWLINE event is composed
 *  of duplicate characters).  For a CHAT_CHARACTER event, the
 *  CChatEvent::Data() field contains the character typed.  For all other
 *  events the Data() field will be NULL.
 *
 *  When a user requests to kick another user, the CHAT_KICK event is sent,
 *  and then the CChatUser::ToKick() contains the uin of the person to kick.
 *  The plugin shall then call CChatManager::SendVoteNo() or
 *  CChatManager::SendVoteYes().  Both functions take the UIN of the user we
 *  are voting on.
 *
 *  If you want to kick someone, just simply call CChatManager::SendKick().
 *  The single parameter is the UIN of the user to collect a vote on.  After
 *  a majority is collected, or a tie, the daemon will send out everything that
 *  is necessary.
 *
 *  When a user is kicked out of a room, the CHAT_DISCONNECTIONxKICKED
 *  event is sent.
 *
 *  Call CChatManager::CloseChat() when finished with the chat.  Note that
 *  this will generate a number of CHAT_DISCONNECTION events as each client
 *  is disconnected from the chat.
 *
 *--------------------------------------------------------------------------*/

// Chat event types
const unsigned char CHAT_COLORxFG             = 0x00;
const unsigned char CHAT_COLORxBG             = 0x01;
const unsigned char CHAT_KICK                 = 0x02;
const unsigned char CHAT_FOCUSxIN             = 0x03;
const unsigned char CHAT_FOCUSxOUT            = 0x04;
const unsigned char CHAT_KICKxYES             = 0x05;
const unsigned char CHAT_KICKxNO              = 0x06;
const unsigned char CHAT_BEEP                 = 0x07;
const unsigned char CHAT_BACKSPACE            = 0x08;
const unsigned char CHAT_KICKxPASS            = 0x09;
const unsigned char CHAT_DISCONNECTION        = 0x0B;
const unsigned char CHAT_DISCONNECTIONxKICKED = 0x0C;
const unsigned char CHAT_NEWLINE              = 0x0D;
const unsigned char CHAT_KICKxYOU             = 0x0E;
const unsigned char CHAT_KICKxFAIL            = 0x0F;
const unsigned char CHAT_FONTxFAMILY          = 0x10;
const unsigned char CHAT_FONTxFACE            = 0x11;
const unsigned char CHAT_FONTxSIZE            = 0x12;
const unsigned char CHAT_SLEEPxOFF            = 0x16;
const unsigned char CHAT_SLEEPxON             = 0x17;
const unsigned char CHAT_LAUGH                = 0x1A;

const unsigned char CHAT_CHARACTER            = 0x7E;
const unsigned char CHAT_CONNECTION           = 0x7F;

const unsigned char CHAT_ERRORxCONNECT        = 0xFF;
const unsigned char CHAT_ERRORxBIND           = 0xFE;
const unsigned char CHAT_ERRORxRESOURCES      = 0xFD;

// Font contants (should not need to be used by the plugin)
const unsigned long FONT_PLAIN     = 0x00000000;
const unsigned long FONT_BOLD      = 0x00000001;
const unsigned long FONT_ITALIC    = 0x00000002;
const unsigned long FONT_UNDERLINE = 0x00000004;
const unsigned long FONT_STRIKEOUT = 0x00000008;

//Font encodings (taken from windows api docs)
const unsigned char ENCODING_ANSI        =   0;
const unsigned char ENCODING_DEFAULT     =   1;
const unsigned char ENCODING_SYMBOL      =   2;
const unsigned char ENCODING_MAC         =  77;
const unsigned char ENCODING_SHIFTJIS    = 128;
const unsigned char ENCODING_HANGEUL     = 129;
const unsigned char ENCODING_JOHAB       = 130;
const unsigned char ENCODING_GB2312      = 134;
const unsigned char ENCODING_CHINESEBIG5 = 136;
const unsigned char ENCODING_GREEK       = 161;
const unsigned char ENCODING_TURKISH     = 162;
const unsigned char ENCODING_VIETNAMESE  = 163;
const unsigned char ENCODING_HEBREW      = 177;
const unsigned char ENCODING_ARABIC      = 178;
const unsigned char ENCODING_BALTIC      = 186;
const unsigned char ENCODING_RUSSIAN     = 204;
const unsigned char ENCODING_THAI        = 222;
const unsigned char ENCODING_EASTEUROPE  = 238;
const unsigned char ENCODING_OEM         = 255;

//Font style (taken from windows api docs)
const unsigned char STYLE_DEFAULTxPITCH  = 0x00;
const unsigned char STYLE_FIXEDxPITCH    = 0x01;
const unsigned char STYLE_VARIABLExPITCH = 0x02;

const unsigned char STYLE_DONTCARE       = 0x00;
const unsigned char STYLE_ROMAN          = 0x10;
const unsigned char STYLE_SWISS          = 0x20;
const unsigned char STYLE_MODERN         = 0x30;
const unsigned char STYLE_SCRIPT         = 0x40;
const unsigned char STYLE_DECORATIVE     = 0x50;

struct SVoteInfo
{
  unsigned long nUin;
  unsigned short nNumUsers;
  unsigned short nYes;
  unsigned short nNo;
};
typedef std::list<SVoteInfo *> VoteInfoList;

class CChatUser;
class CChatManager;

struct SChatReverseConnectInfo
{
  int nId;
  bool bTryDirect;
  CChatUser *u;
  CChatManager *m;
};

//=====Chat=====================================================================
class CPacketChat : public CPacket
{
public:
  virtual unsigned short Sequence()   { return 0; };
  virtual unsigned short SubSequence()   { return 0; };
  virtual unsigned short Command()    { return 0; };
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
  CPChat_Color(const char *szLocalName, unsigned short nLocalPort,
               int nColorForeRed, int nColorForeGreen, int nColorForeBlue,
               int nColorBackRed, int nColorBackBlue, int nColorBackGreen);
  CPChat_Color(CBuffer &);

  // Accessors
  const char *Name() { return m_szName; }
  const UserId& userId() const { return myUserId; }
  unsigned short Port() { return m_nPort; }
  int ColorForeRed() { return m_nColorForeRed; }
  int ColorForeGreen() { return m_nColorForeGreen; }
  int ColorForeBlue() { return m_nColorForeBlue; }
  int ColorBackRed() { return m_nColorBackRed; }
  int ColorBackGreen() { return m_nColorBackGreen; }
  int ColorBackBlue() { return m_nColorBackBlue; }

  virtual ~CPChat_Color();

protected:
  UserId myUserId;
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
  CChatClient(const LicqUser* u);
  CChatClient(const CChatClient &);
  CChatClient& operator=(const CChatClient &);
  ~CChatClient();

  // Initialize from the handshake buffer (does not set the session
  // or port fields however)
  bool LoadFromHandshake_v2(CBuffer &);
  bool LoadFromHandshake_v4(CBuffer &);
  bool LoadFromHandshake_v6(CBuffer &);
  bool LoadFromHandshake_v7(CBuffer &);

  unsigned long m_nVersion;
  unsigned short m_nPort;
  UserId myUserId;
  unsigned long m_nIp;
  unsigned long m_nIntIp;
  char m_nMode;
  unsigned short m_nSession;
  unsigned long m_nHandshake;
  unsigned short m_nId;
  
protected:
  CChatClient(CBuffer &);
  bool LoadFromBuffer(CBuffer &);

friend class CPChat_ColorFont;
};

typedef std::list<CChatClient> ChatClientList;
typedef std::list<CChatClient *> ChatClientPList;


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
     bool bFontBold, bool bFontItalic, bool bFontUnderline, bool bFontStrikeOut,
     const char *szFontFamily, unsigned char nFontEncoding,
     unsigned char nFontStyle, ChatClientPList &clientList);

  CPChat_ColorFont(CBuffer &);

  virtual ~CPChat_ColorFont();

  // Accessors
  const char *Name() { return m_szName; }
  const UserId& userId() const { return myUserId; }
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
  const char *FontFamily() { return m_szFontFamily; }
  unsigned char FontEncoding() { return m_nFontEncoding; }
  unsigned char FontStyle() { return m_nFontStyle; }
  ChatClientList &ChatClients()  { return chatClients; }

protected:
  UserId myUserId;
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
               bool bFontStrikeOut, const char *szFontFamily,
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
  const char *FontFamily() { return m_szFontFamily; }
  unsigned char FontEncoding() { return m_nFontEncoding; }
  unsigned char FontStyle() { return m_nFontStyle; }

protected:
  unsigned short m_nPort;
  unsigned short m_nSession;
  unsigned long m_nFontSize;
  unsigned long m_nFontFace;
  char *m_szFontFamily;
  unsigned char m_nFontEncoding, m_nFontStyle;
};






/*
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


//=====CChatUser=============================================================
extern "C"
{
  void *ChatManager_tep(void *);
  void *ChatWaitForSignal_tep(void *);
  void ChatWaitForSignal_cleanup(void *);
}


class CChatUser
{
public:
  const UserId& userId() const { return myUserId; }
  unsigned long ToKick()       { return nToKick; }
  const char *Name()           { return chatname; }
  int *ColorFg()               { return colorFore; }
  int *ColorBg()               { return colorBack; }
  char *FontFamily()           { return fontFamily; }
  unsigned char FontEncoding() { return fontEncoding; }
  unsigned char FontStyle()    { return fontStyle; }
  unsigned short FontSize()    { return fontSize; }
  bool FontBold()              { return fontFace & FONT_BOLD; }
  bool FontItalic()            { return fontFace & FONT_ITALIC; }
  bool FontUnderline()         { return fontFace & FONT_UNDERLINE; }
  bool FontStrikeOut()         { return fontFace & FONT_STRIKEOUT; }
  bool Focus()                 { return focus; }
  bool Sleep()                 { return sleep; }

  ~CChatUser();

protected:
  CChatUser();

  UserId myUserId;
  unsigned long nToKick;
  char chatname[32];
  int colorFore[3], colorBack[3];
  char fontFamily[64];
  unsigned char fontEncoding, fontStyle;
  unsigned short fontSize;
  unsigned long fontFace;
  bool focus, sleep;

  CChatClient *m_pClient;
  TCPSocket sock;
  std::deque <unsigned char> chatQueue;
  unsigned short state;
  char linebuf[1024];

  pthread_mutex_t mutex;

friend class CChatManager;
friend class CChatEvent;
friend void *ChatManager_tep(void *);
friend void *ChatWaitForSignal_tep(void *);
friend void ChatWaitForSignal_cleanup(void *);
};

typedef std::list<CChatUser *> ChatUserList;


//=====ChatEvent=============================================================

class CChatEvent
{
public:
  CChatEvent(unsigned char, CChatUser *, char * = NULL);
  ~CChatEvent();

  unsigned char Command() { return m_nCommand; }
  CChatUser *Client() { return m_pUser; }
  char *Data() { return m_szData; }

protected:
  unsigned char m_nCommand;
  CChatUser *m_pUser;
  char *m_szData;
  bool m_bLocked;

friend class CChatManager;
};

typedef std::list <CChatEvent *> ChatEventList;


//=====ChatManager===========================================================
typedef std::list<class CChatManager *> ChatManagerList;
typedef std::list<pthread_t> ThreadList;

class CChatManager
{
public:
  CChatManager(CICQDaemon *d, unsigned long nUin,
     const char *fontFamily = "courier",
     unsigned char fontEncoding = ENCODING_DEFAULT,
     unsigned char fontStyle = STYLE_DONTCARE | STYLE_DEFAULTxPITCH,
     unsigned short fontSize = 12, bool fontBold = false,
     bool fontItalic = false, bool fontUnderline = false,
     bool fontStrikeOut = false, int fr = 0xFF, int fg = 0xFF,
     int fb = 0xFF, int br = 0x00, int bg = 0x00, int bb = 0x00);
  ~CChatManager();

  bool StartAsServer();
  void StartAsClient(unsigned short nPort);

  void CloseChat();
  char *ClientsStr();
  unsigned short ConnectedUsers()  { return chatUsers.size(); }

  uint16_t LocalPort() const            { return chatServer.getLocalPort(); }
  const char *Name()  { return m_szName; }
  const char *FontFamily()  { return m_szFontFamily; }
  unsigned char FontEncoding() { return m_nFontEncoding; }
  unsigned char FontStyle() { return m_nFontStyle; }
  unsigned long FontFace()  { return m_nFontFace; }
  unsigned short FontSize()  { return m_nFontSize; }
  int *ColorFg()  { return m_nColorFore; }
  int *ColorBg()  { return m_nColorBack; }

  bool Sleep()  { return m_bSleep; }
  bool Focus()  { return m_bFocus; }

  void ChangeFontFamily(const char *, unsigned char, unsigned char);
  void ChangeFontSize(unsigned short);
  void ChangeFontFace(bool, bool, bool, bool);
  void ChangeColorFg(int, int, int);
  void ChangeColorBg(int, int, int);
  void SendBeep();
  void SendLaugh();
  void SendNewline();
  void SendBackspace();
  void SendCharacter(char);
  void SendKick(const char* id);
  void SendKickNoVote(const char* id);
  void SendVoteYes(unsigned long);
  void SendVoteNo(unsigned long);

  void FocusOut();
  void FocusIn();
  void Sleep(bool);

  int Pipe() { return pipe_events[PIPE_READ]; }
  CChatEvent *PopChatEvent();

  void AcceptReverseConnection(TCPSocket *);
  static CChatManager *FindByPort(unsigned short);

protected:
  static ChatManagerList cmList;
  static pthread_mutex_t cmList_mutex;
  static pthread_mutex_t waiting_thread_cancel_mutex;

  CICQDaemon *licqDaemon;
  int pipe_events[2], pipe_thread[2];
  UserId myUserId;
  unsigned short m_nSession;
  ChatUserList chatUsers;
  ChatUserList chatUsersClosed;
  ChatEventList chatEvents;
  VoteInfoList voteInfo;
  ThreadList waitingThreads;
  pthread_mutex_t thread_list_mutex;
  pthread_t thread_chat;
  CChatClient *m_pChatClient;

  int m_nColorFore[3], m_nColorBack[3];
  char m_szFontFamily[64], m_szName[64];
  unsigned char m_nFontEncoding, m_nFontStyle;
  unsigned short m_nFontSize;
  unsigned long m_nFontFace;
  bool m_bSleep, m_bFocus;

  TCPSocket chatServer;

  CSocketManager sockman;
  bool m_bThreadCreated;

  bool StartChatServer();
  bool ConnectToChat(CChatClient *);
  bool SendChatHandshake(CChatUser *);
  CChatUser *FindChatUser(int);
  void CloseClient(CChatUser *);
  bool ProcessPacket(CChatUser *);
  bool ProcessRaw(CChatUser *);
  bool ProcessRaw_v2(CChatUser *);
  bool ProcessRaw_v6(CChatUser *);
  void PushChatEvent(CChatEvent *);
  void FinishKickVote(VoteInfoList::iterator, bool);

  void SendBuffer(CBuffer *, unsigned char,
      const char* id = NULL, bool bNotIter = true);
  bool SendBufferToClient(CBuffer *, unsigned char, CChatUser *);
  void SendBuffer_Raw(CBuffer *);
  //void SendPacket(CPacket *);

friend void *ChatManager_tep(void *);
friend void *ChatWaitForSignal_tep(void *);
friend void ChatWaitForSignal_cleanup(void *);
};



#endif


