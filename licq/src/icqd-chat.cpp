#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <ctype.h>

#include "licq_chat.h"
#include "licq_log.h"
#include "licq_constants.h"
#include "licq_icqd.h"
#include "licq_translate.h"
#include "licq_sighandler.h"
#include "support.h"

#define DEBUG_THREADS(x)


const unsigned short CHAT_STATE_DISCONNECTED = 0;
const unsigned short CHAT_STATE_HANDSHAKE = 1;
const unsigned short CHAT_STATE_WAITxFORxCOLOR = 2;
const unsigned short CHAT_STATE_WAITxFORxCOLORxFONT = 3;
const unsigned short CHAT_STATE_WAITxFORxFONT = 4;
const unsigned short CHAT_STATE_CONNECTED = 5;


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


bool CChatClient::LoadFromHandshake_v2(CBuffer &b)
{
  b.Reset();

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


bool CChatClient::LoadFromHandshake_v4(CBuffer &b)
{
  b.Reset();

  if ((unsigned char)b.UnpackChar() != ICQ_CMDxTCP_HANDSHAKE) return false;

  m_nVersion = b.UnpackUnsignedLong();
  b.UnpackUnsignedLong();
  m_nUin = b.UnpackUnsignedLong();
  m_nIp = b.UnpackUnsignedLong();  // Will probably be zero...
  m_nRealIp = b.UnpackUnsignedLong();
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
  m_nUin = hand.SourceUin();
  m_nIp = hand.LocalIp();
  m_nRealIp = hand.RealIp();
  m_nMode = hand.Mode();
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

  m_nSize = 10 + strlen(szLocalName) + 38 + strlen(szFontFamily) + 4
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

/*
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
*/


//=====ChatUser==============================================================
CChatUser::CChatUser()
{
  uin = 0;
  nToKick = 0;
  state = CHAT_STATE_DISCONNECTED;
  colorFore[0] = colorFore[1] = colorFore[2] = 0x00;
  colorBack[0] = colorBack[1] = colorBack[2] = 0xFF;
  chatname[0] = '\0';
  linebuf[0] = '\0';
  strcpy(fontFamily, "courier");
  fontSize = 12;
  fontFace = FONT_PLAIN;
  focus = true;
  sleep = false;

  pthread_mutex_init(&mutex, NULL);
}


CChatEvent::CChatEvent(unsigned char nCommand, CChatUser *u, char *szData)
{
  m_nCommand = nCommand;
  m_pUser = u;
  m_szData = (szData == NULL ? NULL : strdup(szData));
  m_bLocked = false;
}


CChatEvent::~CChatEvent()
{
  if (m_szData != NULL) free(m_szData);
  if (m_bLocked) pthread_mutex_unlock(&m_pUser->mutex);
}


//=====ChatManager===========================================================
ChatManagerList CChatManager::cmList;


CChatManager::CChatManager(CICQDaemon *d, unsigned long nUin,
  const char *fontFamily, unsigned short fontSize, bool fontBold,
  bool fontItalic, bool fontUnderline, int fr, int fg, int fb,
  int br, int bg, int bb)
{
  // Create the plugin notification pipe
  pipe(pipe_thread);
  pipe(pipe_events);

  m_nUin = nUin;
  m_nSession = rand();
  licqDaemon = d;

  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  strncpy(m_szName, o->GetAlias(), 32);
  gUserManager.DropOwner();

  m_nFontFace = FONT_PLAIN;
  if (fontBold) m_nFontFace |= FONT_BOLD;
  if (fontItalic) m_nFontFace |= FONT_ITALIC;
  if (fontUnderline) m_nFontFace |= FONT_UNDERLINE;
  strncpy(m_szFontFamily, fontFamily, 64);
  m_nFontSize = fontSize;
  m_nColorFore[0] = fr;
  m_nColorFore[1] = fg;
  m_nColorFore[2] = fb;
  m_nColorBack[0] = br;
  m_nColorBack[1] = bg;
  m_nColorBack[2] = bb;
  m_bFocus = true;
  m_bSleep = false;

  cmList.push_back(this);
}


//-----CChatManager::StartChatServer-----------------------------------------
bool CChatManager::StartChatServer()
{
  if (licqDaemon->StartTCPServer(&chatServer) == -1)
  {
    gLog.Warn("%sNo more ports available, add more or close open chat/file sessions.\n", L_WARNxSTR);
    return false;
  }

  // Add the server to the sock manager
  sockman.AddSocket(&chatServer);
  sockman.DropSocket(&chatServer);

  return true;
}



bool CChatManager::StartAsServer()
{
  if (!StartChatServer()) return false;

  // Create the socket manager thread
  if (pthread_create(&thread_chat, NULL, &ChatManager_tep, this) == -1)
    return false;

  return true;
}


//-----CChatManager::StartAsClient-------------------------------------------
bool CChatManager::StartAsClient(unsigned short nPort)
{
  if (!StartChatServer()) return false;

  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
  if (u == NULL) return false;
  CChatClient c(u);
  c.m_nPort = nPort;
  gUserManager.DropUser(u);
  if (!ConnectToChat(c)) return false;

  // Create the socket manager thread
  if (pthread_create(&thread_chat, NULL, &ChatManager_tep, this) == -1)
    return false;

  return true;
}


//-----CChatManager::ConnectToChat-------------------------------------------
bool CChatManager::ConnectToChat(CChatClient &c)
{
  CChatUser *u = new CChatUser;
  u->client = c;
  u->client.m_nSession = m_nSession;
  u->uin = c.m_nUin;

  bool bSendRealIp = false;
  ICQUser *temp_user = gUserManager.FetchUser(u->uin, LOCK_R);
  if (temp_user != NULL)
  {
    bSendRealIp = temp_user->SendRealIp();
    gUserManager.DropUser(temp_user);
  }

  gLog.Info("%sChat: Connecting to server.\n", L_TCPxSTR);
  if (!licqDaemon->OpenConnectionToUser("chat", c.m_nIp, c.m_nRealIp, &u->sock, c.m_nPort, bSendRealIp))
  {
    delete u;
    return false;
  }

  chatUsers.push_back(u);

  gLog.Info("%sChat: Shaking hands [v%d].\n", L_TCPxSTR, VersionToUse(c.m_nVersion));

  // Send handshake packet:
  if (!CICQDaemon::Handshake_Send(&u->sock, c.m_nUin, LocalPort(),
     VersionToUse(c.m_nVersion)))
    return false;

  // Send color packet
  CPChat_Color p_color(m_szName, LocalPort(),
     m_nColorFore[0], m_nColorFore[1], m_nColorFore[2],
     m_nColorBack[0], m_nColorBack[1], m_nColorBack[2]);
  u->sock.SendPacket(p_color.getBuffer());

  gLog.Info("%sChat: Waiting for color/font response.\n", L_TCPxSTR);

  u->state = CHAT_STATE_WAITxFORxCOLORxFONT;

  sockman.AddSocket(&u->sock);
  sockman.DropSocket(&u->sock);

  return true;
}


//-----CChatManager::AcceptReverseConnection---------------------------------
void CChatManager::AcceptReverseConnection(TCPSocket *s)
{
  CChatUser *u = new CChatUser;
  u->sock.TransferConnectionFrom(*s);

  u->client.m_nVersion = s->Version();
  u->client.m_nUin = s->Owner();
  u->client.m_nIp = s->RemoteIp();
  u->client.m_nRealIp = s->RemoteIp();
  u->client.m_nMode = MODE_DIRECT;
  u->client.m_nHandshake = 0x64;

  // These will still need to be set
  u->client.m_nPort = 0;
  u->client.m_nSession = 0;

  u->uin = u->client.m_nUin;
  u->state = CHAT_STATE_WAITxFORxCOLOR;
  chatUsers.push_back(u);

  // Reload the socket information
  sockman.AddSocket(&u->sock);
  sockman.DropSocket(&u->sock);
  write(pipe_thread[PIPE_WRITE], "R", 1);

  gLog.Info("%sChat: Received reverse connection.\n", L_TCPxSTR);
}


//-----CChatManager::FindChatUser--------------------------------------------
CChatUser *CChatManager::FindChatUser(int sd)
{
  // Find the right user (possible race condition, but we ignore it for now)
  ChatUserList::iterator iter;
  for (iter = chatUsers.begin(); iter != chatUsers.end(); iter++)
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
    char buf[128];
    if (u->sock.Error() == 0)
      gLog.Info("%sChat: Remote end disconnected.\n", L_TCPxSTR);
    else
      gLog.Info("%sChat: Lost remote end:\n%s%s\n", L_TCPxSTR,
                L_BLANKxSTR, u->sock.ErrorStr(buf, 128));
    return false;
  }

  if (!u->sock.RecvBufferFull()) return true;

  switch(u->state)
  {
    case CHAT_STATE_HANDSHAKE:
    {
      // get the handshake packet
      if (!CICQDaemon::Handshake_Recv(&u->sock, LocalPort()))
      {
        gLog.Warn("%sChat: Bad handshake.\n", L_ERRORxSTR);
        return false;
      }
      switch (u->sock.Version())
      {
        case 2:
        case 3:
          u->client.LoadFromHandshake_v2(u->sock.RecvBuffer());
          break;
        case 4:
          u->client.LoadFromHandshake_v4(u->sock.RecvBuffer());
          break;
        case 6:
          u->client.LoadFromHandshake_v6(u->sock.RecvBuffer());
          break;
      }
      gLog.Info("%sChat: Received handshake from %ld [v%ld].\n", L_TCPxSTR,
         u->client.m_nUin, u->sock.Version());
      u->uin = u->client.m_nUin;
      u->state = CHAT_STATE_WAITxFORxCOLOR;
      break;
    }

    case CHAT_STATE_WAITxFORxCOLOR:  // we just received the color packet
    {
      gLog.Info("%sChat: Received color packet.\n", L_TCPxSTR);

      CPChat_Color pin(u->sock.RecvBuffer());

      strncpy(u->chatname, pin.Name(), 32);
      // Fill in the remaining fields in the client structure
      u->client.m_nPort = pin.Port();
      u->client.m_nSession = m_nSession;

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
      for (iter = chatUsers.begin(); iter != chatUsers.end(); iter++)
      {
        // Skip this guys client info and anybody we haven't connected to yet
        if ((*iter)->uin == u->uin || (*iter)->client.m_nUin == 0) continue;
        l.push_back(&(*iter)->client);
      }

      CPChat_ColorFont p_colorfont(m_szName, LocalPort(), m_nSession,
         m_nColorFore[0], m_nColorFore[1], m_nColorFore[2],
         m_nColorBack[0], m_nColorBack[1], m_nColorBack[2],
         m_nFontSize, m_nFontFace & FONT_BOLD, m_nFontFace & FONT_ITALIC,
         m_nFontFace & FONT_UNDERLINE, m_szFontFamily, l);
      if (!u->sock.SendPacket(p_colorfont.getBuffer()))
      {
        char buf[128];
        gLog.Error("%sChat: Send error (color/font packet):\n%s%s\n",
                   L_ERRORxSTR, L_BLANKxSTR, u->sock.ErrorStr(buf, 128));
        return false;
      }
      u->state = CHAT_STATE_WAITxFORxFONT;
      break;
    }

    case CHAT_STATE_WAITxFORxFONT:
    {
      gLog.Info("%sChat: Received font packet.\n", L_TCPxSTR);
      CPChat_Font pin(u->sock.RecvBuffer());

      // just received the font reply
      m_nSession = pin.Session();
      u->fontSize = pin.FontSize();
      u->fontFace = pin.FontFace();
      strncpy(u->fontFamily, pin.FontFamily(), 64);

      u->state = CHAT_STATE_CONNECTED;
      PushChatEvent(new CChatEvent(CHAT_CONNECTION, u));
      break;
    }

    case CHAT_STATE_WAITxFORxCOLORxFONT:
    {
      gLog.Info("%sChat: Received color/font packet.\n", L_TCPxSTR);

      CPChat_ColorFont pin(u->sock.RecvBuffer());
      u->uin = pin.Uin();
      m_nSession = pin.Session();

      // just received the color/font packet
      strncpy(u->chatname, pin.Name(), 32);

      // set up the remote colors
      u->colorFore[0] = pin.ColorForeRed();
      u->colorFore[1] = pin.ColorForeGreen();
      u->colorFore[2] = pin.ColorForeBlue();
      u->colorBack[0] = pin.ColorBackRed();
      u->colorBack[1] = pin.ColorBackGreen();
      u->colorBack[2] = pin.ColorBackBlue();

      // set up the remote font
      m_nSession = pin.Session();
      u->fontSize = pin.FontSize();
      u->fontFace = pin.FontFace();
      strncpy(u->fontFamily, pin.FontFamily(), 64);

      // Parse the multiusers list
      if (pin.ChatClients().size() > 0)
      {
        gLog.Info("%sChat: Joined multiparty (%d people).\n", L_TCPxSTR,
           pin.ChatClients().size() + 1);
        ChatClientList::iterator iter;
        for (iter = pin.ChatClients().begin(); iter != pin.ChatClients().end(); iter++)
        {
          ChatUserList::iterator iter2;
          for (iter2 = chatUsers.begin(); iter2 != chatUsers.end(); iter2++)
          {
            if ((*iter2)->uin == iter->m_nUin) break;
          }
          if (iter2 != chatUsers.end()) continue;
          // Connect to this user
          ConnectToChat(*iter);
        }
      }

      // send the reply (font packet)
      CPChat_Font p_font(LocalPort(), m_nSession,
         m_nFontSize, m_nFontFace & FONT_BOLD, m_nFontFace & FONT_ITALIC,
         m_nFontFace & FONT_UNDERLINE, m_szFontFamily);
      if (!u->sock.SendPacket(p_font.getBuffer()))
      {
        char buf[128];
        gLog.Error("%sChat: Send error (font packet):\n%s%s\n",
                   L_ERRORxSTR, L_BLANKxSTR, u->sock.ErrorStr(buf, 128));
        return false;
      }

      // now we are done with the handshaking
      u->state = CHAT_STATE_CONNECTED;
      PushChatEvent(new CChatEvent(CHAT_CONNECTION, u));
      break;
    }

    case CHAT_STATE_CONNECTED:
    default:
      gLog.Error("%sInternal error: ChatManager::ProcessPacket(), invalid state (%d).\n",
         L_ERRORxSTR, u->state);
      break;

  } // switch

  u->sock.ClearRecvBuffer();

  return true;
}


//-----CChatManager::PopChatEvent--------------------------------------------
CChatEvent *CChatManager::PopChatEvent()
{
  if (chatEvents.size() == 0) return NULL;

  CChatEvent *e = chatEvents.front();
  chatEvents.pop_front();

  // Lock the user, will be unlocked in the event destructor
  pthread_mutex_lock(&e->m_pUser->mutex);
  e->m_bLocked = true;

  return e;
}


//-----CChatManager::PushChatEvent-------------------------------------------
void CChatManager::PushChatEvent(CChatEvent *e)
{
  chatEvents.push_back(e);
  write(pipe_events[PIPE_WRITE], "*", 1);
}


//-----CChatManager::ProcessRaw----------------------------------------------
bool CChatManager::ProcessRaw(CChatUser *u)
{
  if (!u->sock.RecvRaw())
  {
    char buf[128];
    if (u->sock.Error() == 0)
      gLog.Info("%sChat: Remote end disconnected.\n", L_TCPxSTR);
    else
      gLog.Info("%sChat: Lost remote end:\n%s%s\n", L_TCPxSTR,
                L_BLANKxSTR, u->sock.ErrorStr(buf, 128));
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
        PushChatEvent(new CChatEvent(CHAT_NEWLINE, u, u->linebuf));
        u->linebuf[0] = '\0';
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
        if (strlen(u->linebuf) > 0)
          u->linebuf[strlen(u->linebuf) - 1] = '\0';
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
         unsigned short sizeFontName, encodingFont, i;
         sizeFontName = u->chatQueue[1] | (u->chatQueue[2] << 8);
         if (u->chatQueue.size() < (unsigned long)(sizeFontName + 2 + 3)) return true;
         char nameFont[sizeFontName];
         for (i = 0; i < sizeFontName; i++)
            nameFont[i] = u->chatQueue[i + 3];
         encodingFont = u->chatQueue[sizeFontName + 3] |
                        (u->chatQueue[sizeFontName + 4] << 8);
         strncpy(u->fontFamily, nameFont, 64);

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
        for (iter = voteInfo.begin(); iter != voteInfo.end(); iter++)
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
        for (iter = voteInfo.begin(); iter != voteInfo.end(); iter++)
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

        // Deque all the characters
        for (unsigned short i = 0; i < 6; i++)
          u->chatQueue.pop_front();

        // Find the user and say bye-bye to him
        ChatUserList::iterator iter;
        for (iter = chatUsers.begin(); iter != chatUsers.end(); iter++)
        {
          if((*iter)->Uin() == nUin)
            break;
        }

        if (iter == chatUsers.end())   return true;

        CBuffer bye(4);
        SendBuffer(&bye, CHAT_DISCONNECTIONxKICKED, nUin, true);

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
          gTranslator.ServerToClient(chatChar);
          char tempStr[2] = { chatChar, '\0' };
          // Add to the users irc line buffer
          strcat(u->linebuf, tempStr);
          PushChatEvent(new CChatEvent(CHAT_CHARACTER, u, tempStr));
          if (strlen(u->linebuf) > 1000) // stop a little early
          {
             u->linebuf[1000] = '\0';
             PushChatEvent(new CChatEvent(CHAT_NEWLINE, u, u->linebuf));
             u->linebuf[0] = '\0';
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
           unsigned short sizeFontName, encodingFont, i;
           sizeFontName = u->chatQueue[0] | (u->chatQueue[1] << 8);
           char nameFont[sizeFontName];
           for (i = 0; i < sizeFontName; i++)
              nameFont[i] = u->chatQueue[i + 2];
           encodingFont = u->chatQueue[sizeFontName + 2] |
                          (u->chatQueue[sizeFontName + 3] << 8);
           strncpy(u->fontFamily, nameFont, 64);

           PushChatEvent(new CChatEvent(CHAT_FONTxFAMILY, u));
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
          for (iter = voteInfo.begin(); iter != voteInfo.end(); iter++)
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
          for (iter = voteInfo.begin(); iter != voteInfo.end(); iter++)
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

          // Find the user and say bye-bye to him
          ChatUserList::iterator iter;
          for (iter = chatUsers.begin(); iter != chatUsers.end(); iter++)
          {
            if((*iter)->Uin() == nUin)
             break;
          }

          if (iter == chatUsers.end())   return true;

          CBuffer bye(4);
          SendBuffer(&bye, CHAT_DISCONNECTIONxKICKED, nUin, true);

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
          gLog.Unknown("%sChat: Unknown chat command (%02X).\n", L_UNKNOWNxSTR,
             chatChar);
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
          PushChatEvent(new CChatEvent(CHAT_NEWLINE, u, u->linebuf));
          u->linebuf[0] = '\0';
          break;

        case CHAT_BACKSPACE:   // backspace
        {
          if (strlen(u->linebuf) > 0)
            u->linebuf[strlen(u->linebuf) - 1] = '\0';
          PushChatEvent(new CChatEvent(CHAT_BACKSPACE, u));
          break;
        }

        default:
        {
          if (!iscntrl((int)(unsigned char)chatChar))
          {
            gTranslator.ServerToClient(chatChar);
            char tempStr[2] = { chatChar, '\0' };
            // Add to the users irc line buffer
            strcat(u->linebuf, tempStr);
            PushChatEvent(new CChatEvent(CHAT_CHARACTER, u, tempStr));
	    if (strlen(u->linebuf) > 1000) // stop a little early
	    {
		u->linebuf[1000] = '\0';
                PushChatEvent(new CChatEvent(CHAT_NEWLINE, u, u->linebuf));
		u->linebuf[0] = '\0';
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
                              unsigned long _nUin = 0,
                              bool bNotIter = true)
{
  ChatUserList::iterator iter;
  ChatUserList::iterator u_iter;
  bool ok = false;

  if (_nUin != 0)
  {
    for (u_iter = chatUsers.begin(); u_iter != chatUsers.end(); u_iter++)
    {
      if ((*u_iter)->Uin() == _nUin)
        break;
    }

     if (u_iter == chatUsers.end())
       return;
  }

  while (!ok)
  {
    ok = true;

    // Send it to every user
    if (_nUin == 0)
    {
      for (iter = chatUsers.begin(); iter != chatUsers.end(); iter++)
        ok = SendBufferToClient(b, cmd, *iter);
    }
    else
    {
      // Send it to every user except _iter
      if (bNotIter)
      {
        for (iter = chatUsers.begin(); iter != u_iter; iter++)
          ok = SendBufferToClient(b, cmd, *iter);

        // Check to see if we are already at the end
        // And at the same time skip the user we don't want to send this to
        if (++iter == chatUsers.end())  return;

        for (; iter != chatUsers.end(); iter++)
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
    b_out.PackUnsignedLong(b->getDataSize());
    b_out.Pack(b->getDataStart(), b->getDataSize());
  }
  else
  {
    b_out.PackChar(cmd);
    b_out.Pack(b->getDataStart(), b->getDataSize());
  }

  if (!u->sock.SendRaw(&b_out))
  {
    char buf[128];
    gLog.Warn("%sChat: Send error:\n%s%s\n", L_WARNxSTR, L_BLANKxSTR,
               u->sock.ErrorStr(buf, 128));
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
    for (iter = chatUsers.begin(); iter != chatUsers.end(); iter++)
    {
      u = *iter;

      // If the socket was closed, ignore the key event
      if (u->state != CHAT_STATE_CONNECTED || u->sock.Descriptor() == -1) continue;

      if (!u->sock.SendRaw(b))
      {
        char buf[128];
        gLog.Warn("%sChat: Send error:\n%s%s\n", L_WARNxSTR, L_BLANKxSTR,
                   u->sock.ErrorStr(buf, 128));
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
  gTranslator.ClientToServer(c);
  buf.PackChar(c);
  SendBuffer_Raw(&buf);
}


void CChatManager::SendKick(unsigned long _nUin)
{
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
  SendBuffer(&buf, CHAT_KICK, _nUin, true);
}


void CChatManager::SendKickNoVote(unsigned long _nUin)
{
  // Tell everyone that this user has been kicked
  CBuffer buf_TellAll(6);
  buf_TellAll.PackUnsignedLong(_nUin);
  buf_TellAll.PackChar(0x02);
  buf_TellAll.PackChar(0x01);
  SendBuffer(&buf_TellAll, CHAT_KICKxPASS, _nUin, true);

  // They don't know if there was a vote or not, they just see they've been kicked
  CBuffer buf(2);
  buf.PackChar(0x02);
  buf.PackChar(0x01);
  SendBuffer(&buf, CHAT_KICKxYOU, _nUin, false);

  // And close the connection to the kicked user
  ChatUserList::iterator iter;
  for (iter = chatUsers.begin(); iter != chatUsers.end(); iter++)
  {
    if((*iter)->Uin() == _nUin)
      break;
  }

  if (iter == chatUsers.end())   return;

  CBuffer bye(4);
  SendBuffer(&bye, CHAT_DISCONNECTIONxKICKED, _nUin, false);

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


void CChatManager::ChangeFontFamily(const char *f)
{
  //CPChat_ChangeFontFamily p(f);
  //SendPacket(&p);

  CBuffer buf(strlen_safe(f) + 6);
  buf.PackString(f);
  buf.PackUnsignedShort(0x2200);
  // 0x2200 west
  // 0x22a2 turkey
  // 0x22cc cyrillic
  // 0x22a1 greek
  // 0x22ba baltic
  SendBuffer(&buf, CHAT_FONTxFAMILY);

  strncpy(m_szFontFamily, f, 64);
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


void CChatManager::ChangeFontFace(bool b, bool i, bool u)
{
  //CPChat_ChangeFontFace p(b, i, u);
  //SendPacket(&p);

  m_nFontFace = FONT_PLAIN;
  if (b) m_nFontFace |= FONT_BOLD;
  if (i) m_nFontFace |= FONT_ITALIC;
  if (u) m_nFontFace |= FONT_UNDERLINE;

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

  // Close the chat thread
  if (pipe_thread[PIPE_WRITE] != -1)
  {
    write(pipe_thread[PIPE_WRITE], "X", 1);
    pthread_join(thread_chat, NULL);

    close(pipe_thread[PIPE_READ]);
    close(pipe_thread[PIPE_WRITE]);

    pipe_thread[PIPE_READ] = pipe_thread[PIPE_WRITE] = -1;
  }
}


//----CChatManager::FinishKickVote-------------------------------------------
void CChatManager::FinishKickVote(VoteInfoList::iterator iter, bool bPassed)
{
  // Find the person we are kicking in the ChatUserList
  ChatUserList::iterator userIter;
  for (userIter = chatUsers.begin(); userIter != chatUsers.end(); userIter++)
  {
    if ((*userIter)->Uin() == (*iter)->nUin)
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
    SendBuffer(&buf, CHAT_KICKxPASS, (*iter)->nUin, true);
  else
    SendBuffer(&buf, CHAT_KICKxFAIL, (*iter)->nUin, true);

  // Send the person a notice if they were kicked
  if (bPassed)
  {
    SendBuffer(&buf, CHAT_KICKxYOU, (*iter)->nUin, false);
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
  for (iter = chatUsers.begin(); iter != chatUsers.end(); iter++)
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
char *CChatManager::ClientsStr()
{
  char *sz = new char[chatUsers.size() * 36];
  sz[0] = '\0';
  int nPos = 0;

  ChatUserList::iterator iter;
  for (iter = chatUsers.begin(); iter != chatUsers.end(); iter++)
  {
    if (sz[0] != '\0') nPos += sprintf(&sz[nPos], ", ");
    if ((*iter)->Name()[0] == '\0')
      nPos += sprintf(&sz[nPos], "%ld", (*iter)->Uin());
    else
      nPos += sprintf(&sz[nPos], "%s", (*iter)->Name());
  }
  return sz;
}



void *ChatManager_tep(void *arg)
{
  CChatManager *chatman = (CChatManager *)arg;

  fd_set f;
  int l, nSocketsAvailable, nCurrentSocket;
  char buf[2];

  while (true)
  {
    f = chatman->sockman.SocketSet();
    l = chatman->sockman.LargestSocket() + 1;

    // Add the new socket pipe descriptor
    FD_SET(chatman->pipe_thread[PIPE_READ], &f);
    if (chatman->pipe_thread[PIPE_READ] >= l)
      l = chatman->pipe_thread[PIPE_READ] + 1;

    nSocketsAvailable = select(l, &f, NULL, NULL, NULL);

    nCurrentSocket = 0;
    while (nSocketsAvailable > 0 && nCurrentSocket < l)
    {
      if (FD_ISSET(nCurrentSocket, &f))
      {
        // New socket event ----------------------------------------------------
        if (nCurrentSocket == chatman->pipe_thread[PIPE_READ])
        {
          read(chatman->pipe_thread[PIPE_READ], buf, 1);
          if (buf[0] == 'S')
          {
            DEBUG_THREADS("[ChatManager_tep] Reloading socket info.\n");
          }
          else if (buf[0] == 'X')
          {
            DEBUG_THREADS("[ChatManager_tep] Exiting.\n");
            pthread_exit(NULL);
          }
        }

        // Connection on the server port ---------------------------------------
        else if (nCurrentSocket == chatman->chatServer.Descriptor())
        {
          CChatUser *u = new CChatUser;
          chatman->chatServer.RecvConnection(u->sock);
          chatman->sockman.AddSocket(&u->sock);
          chatman->sockman.DropSocket(&u->sock);

          u->state = CHAT_STATE_HANDSHAKE;
          chatman->chatUsers.push_back(u);
          gLog.Info("%sChat: Received connection.\n", L_TCPxSTR);
        }

        // Message from connected socket----------------------------------------
        else
        {
          CChatUser *u = chatman->FindChatUser(nCurrentSocket);
          if (u == NULL)
          {
            gLog.Warn("%sChat: No user owns socket %d.\n", L_WARNxSTR, nCurrentSocket);
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


CChatManager *CChatManager::FindByPort(unsigned short p)
{
  ChatManagerList::iterator iter;
  for (iter = cmList.begin(); iter != cmList.end(); iter++)
  {
    if ( (*iter)->LocalPort() == p) return *iter;
  }
  return NULL;
}


CChatManager::~CChatManager()
{
  CloseChat();

  // Delete all the users
  CChatUser *u = NULL;
  while (chatUsersClosed.size() > 0)
  {
    u = chatUsersClosed.front();
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

  ChatManagerList::iterator iter;
  for (iter = cmList.begin(); iter != cmList.end(); iter++)
  {
    if (*iter == this) break;
  }
  if (iter != cmList.end()) cmList.erase(iter);
}

