#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream.h>
#include <qapplication.h>
#include <ctype.h>

#include "chatdlg.h"
#include "icqpacket.h"
#include "log.h"
#include "translate.h"
#include "user.h"
#include "support.h"

#define STATE_RECVxHANDSHAKE 1
#define STATE_RECVxCOLOR 2
#define STATE_RECVxFONT 3
#define STATE_RECVxCOLORxFONT 4
#define STATE_RECVxCHAT 5


ChatDlg::ChatDlg(unsigned long _nUin,
                 bool _bServer, unsigned short _nPort,
                 QWidget *parent, char *name)
   : QWidget(parent, name)
{
   m_nUin = _nUin;
   m_nPort = _nPort;
   m_bServer = _bServer;
   m_bAudio = true;
   snChat = snChatServer = NULL;

   boxRemote = new QGroupBox(tr("Remote - Not connected"), this);
   mleRemote = new MLEditWrap(true, boxRemote);
   mleRemote->setReadOnly(true);
   m_sRemoteName = NULL;

   ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
   m_sLocalName = strdup(o->getAlias());
   gUserManager.DropOwner();
   boxLocal = new QGroupBox(tr("Local - ") + QString::fromLocal8Bit(getLocalName()), this);
   mleLocal = new MLEditWrap(true, boxLocal);
   mleLocal->setEnabled(false);

   btnClose = new QPushButton(tr("&Close Chat"), this);
   btnClose->setGeometry(200, 440, 100, 20);
   //connect(btnClose, SIGNAL(clicked()), this, SLOT(chatClose()));
   connect(btnClose, SIGNAL(clicked()), this, SLOT(hide()));

   resize(500, 475);

   // now either connect to the remote host or start up a server
   if (server())
   {  // avoid ambiguous else
      if (!startAsServer()) setPort(0);
   }
   else
   {
      if (!startAsClient()) setPort(0);
   }
}


ChatDlg::~ChatDlg(void)
{
   free(m_sLocalName);
   if (m_sRemoteName != NULL) delete[] m_sRemoteName;
   if (snChatServer != NULL) delete snChatServer;
   if (snChat != NULL) delete snChat;
}

//=====Server===================================================================

//-----startAsServer------------------------------------------------------------
bool ChatDlg::startAsServer(void)
{
   if (!(m_cSocketChatServer.StartServer(getPort())))
   {
     char buf[128];
     gLog.Error("%sFailed to start local chat server (%s)!\n", L_ERRORxSTR, m_cSocketChatServer.ErrorStr(buf, 128));
       return false;
   }

   setPort(m_cSocketChatServer.LocalPort());
   snChatServer = new QSocketNotifier(m_cSocketChatServer.Descriptor(), QSocketNotifier::Read);
   connect(snChatServer, SIGNAL(activated(int)), this, SLOT(chatRecvConnection()));

   boxRemote->setTitle(tr("Remote - Waiting for joiners..."));

   return true;
}


//-----chatRecvConnection-------------------------------------------------------
void ChatDlg::chatRecvConnection()
{
   m_cSocketChatServer.RecvConnection(m_cSocketChat);
   disconnect(snChatServer, SIGNAL(activated(int)), this, SLOT(chatRecvConnection()));
   m_nState = STATE_RECVxHANDSHAKE;
   snChat = new QSocketNotifier(m_cSocketChat.Descriptor(), QSocketNotifier::Read);
   connect(snChat, SIGNAL(activated(int)), this, SLOT(StateServer()));
   snChat->setEnabled(true);
   boxRemote->setTitle(tr("Remote - Received connection, shaking hands..."));
}


//-----StateServer--------------------------------------------------------------
void ChatDlg::StateServer()
{
  if (!m_cSocketChat.RecvPacket())
  {
    char buf[128];
    if (m_cSocketChat.Error() == 0)
      gLog.Error("%sChat receive error - remote end closed connection.\n", L_ERRORxSTR);
    else
      gLog.Error("%sChat receive error - lost remote end:\n%s%s\n", L_ERRORxSTR,
             L_BLANKxSTR, m_cSocketChat.ErrorStr(buf, 128));
   chatClose();
   return;
  }
  if (!m_cSocketChat.RecvBufferFull()) return;

  switch(m_nState)
  {
  case STATE_RECVxHANDSHAKE:
  {
    // get the handshake packet
    char cHandshake;
    m_cSocketChat.RecvBuffer() >> cHandshake;
    if ((unsigned short)cHandshake != ICQ_CMDxTCP_HANDSHAKE)
    {
       m_cSocketChat.CloseConnection();
       gLog.Error("%sReceive error - bad handshake (%04X).\n", L_ERRORxSTR,
                 cHandshake);
       chatClose();
       return;
    }
    m_nState = STATE_RECVxCOLOR;
    break;
  }

  case STATE_RECVxCOLOR:  // we just received the color packet
  {
    unsigned long testLong_1, testLong_2;
    m_cSocketChat.RecvBuffer() >> testLong_1 >> testLong_2;
    if (testLong_1 != 0x64 && testLong_1 != 0x65)
    {
      char *buf;
      gLog.Error("%sChat receive error - invalid color packet:\n%s\n",
                 L_ERRORxSTR, m_cSocketChat.RecvBuffer().print(buf));
      delete [] buf;
      chatClose();
      return;
    }

    // the only interesting thing to take out is the user chat name and colors
    unsigned long junkLong;
    unsigned short nameLen;
    m_cSocketChat.RecvBuffer() >> junkLong  // the uin
                               >> nameLen;   // length of chat name (including null)
    m_sRemoteName = new char[nameLen + 1];
    for (unsigned short i = 0; i < nameLen; i++)
       m_cSocketChat.RecvBuffer() >> m_sRemoteName[i];
    boxRemote->setTitle(tr("Remote - ") + QString::fromLocal8Bit(getRemoteName()));

    // set up the remote colors
    unsigned short junkShort;
    char colorForeRed, colorForeGreen, colorForeBlue,
         colorBackRed, colorBackGreen, colorBackBlue, junkChar;
    m_cSocketChat.RecvBuffer() >> junkShort // chat port reversed
                               >> colorForeRed
                               >> colorForeGreen
                               >> colorForeBlue
                               >> junkChar
                               >> colorBackRed
                               >> colorBackGreen
                               >> colorBackBlue;
    QColorGroup newColorGroup(QColor((unsigned char)colorForeRed, (unsigned char)colorForeGreen, (unsigned char)colorForeBlue), 
                               QColor((unsigned char)colorBackRed, (unsigned char)colorBackGreen, (unsigned char)colorBackBlue), 
                               mleRemote->palette().normal().light(),
                               mleRemote->palette().normal().dark(),
                               mleRemote->palette().normal().mid(),
                               QColor((unsigned char)colorForeRed, (unsigned char)colorForeGreen, (unsigned char)colorForeBlue),
                               QColor((unsigned char)colorBackRed, (unsigned char)colorBackGreen, (unsigned char)colorBackBlue));
    mleRemote->setPalette(QPalette(newColorGroup, mleRemote->palette().disabled(), newColorGroup));

    CPChat_ColorFont p_colorfont(getLocalName(), getLocalPort(), 0xFFFFFF,
                                 0x000000, 0x0C, 0x00, "courier");
    if (!m_cSocketChat.SendPacket(p_colorfont.getBuffer()))
    {
      char buf[128];
      gLog.Error("%sChat send error (color/font packet):\n%s%s\n",
                 L_ERRORxSTR, L_BLANKxSTR, m_cSocketChat.ErrorStr(buf, 128));
      chatClose();
      return;
    }
    m_nState = STATE_RECVxFONT;
    break;
  }

  case STATE_RECVxFONT:
  {
    unsigned long testLong_1, testLong_2;
    m_cSocketChat.RecvBuffer() >> testLong_1 >> testLong_2;    
    /* No test because this value seems to vary all over the place
    if (testLong_1 != 0x03 && testLong_1 != 0x04 && testLong_1 != 0x05)
    {
      gLog.Error("%sChat receive error - invalid font packet:\n%s\n", 
                 L_ERRORxSTR, m_cSocketChat.RecvBuffer().print());
      chatClose();
      return;
    }*/
  
    // just received the font reply
    /* 03 00 00 00 83 72 00 00 CF 60 AD 95 CF 60 AD 95 04 54 72 0C 00 00 00 00 
       00 00 00 08 00 43 6F 75 72 69 65 72 00 00 00 */
    // we don't bother with the font for now...  
    disconnect(snChat, SIGNAL(activated(int)), this, SLOT(StateServer()));
    connect(snChat, SIGNAL(activated(int)), this, SLOT(chatRecv()));
    connect(mleLocal, SIGNAL(keyPressed(QKeyEvent *)), this, SLOT(chatSend(QKeyEvent *)));
    mleLocal->setEnabled(true);
    
    m_nState = STATE_RECVxCHAT;
  }

  case STATE_RECVxCHAT:
    // should never get here...
    break;

  } // switch
  
  m_cSocketChat.ClearRecvBuffer();
}
  
  

//=====Client===================================================================

//-----startAsClient------------------------------------------------------------
bool ChatDlg::startAsClient(void)
{
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
  unsigned long nIp = u->Ip();
  gUserManager.DropUser(u);
  char buf[128];
  gLog.Info("%sConnecting to %s:%d for chat.\n", L_TCPxSTR,
            inet_ntoa_r(*(struct in_addr *)&nIp, buf), getPort());
  m_cSocketChat.SetRemoteAddr(nIp, getPort());
  if (!m_cSocketChat.OpenConnection())
  {
    gLog.Error("%sUnable to connect to chat:\n%s%s.\n", L_ERRORxSTR,
               L_BLANKxSTR, m_cSocketChat.ErrorStr(buf, 128));
    return false;
  }

  boxRemote->setTitle(tr("Remote - Connected, shaking hands..."));

  // Send handshake packet:
  CPacketTcp_Handshake p_handshake(getLocalPort());
  m_cSocketChat.SendPacket(p_handshake.getBuffer());

  // Send color packet
  CPChat_Color p_color(getLocalName(), getLocalPort(), 0xFFFFFF, 0x000000);
  m_cSocketChat.SendPacket(p_color.getBuffer());

  boxRemote->setTitle(tr("Remote - Connected, waiting for response..."));

  m_nState = STATE_RECVxCOLORxFONT;
  snChat = new QSocketNotifier(m_cSocketChat.Descriptor(), QSocketNotifier::Read);
  connect(snChat, SIGNAL(activated(int)), this, SLOT(StateClient()));

  return true;
}



//-----StateClient--------------------------------------------------------------
void ChatDlg::StateClient()
{
  if (!m_cSocketChat.RecvPacket())
  {
    char buf[128];
    if (m_cSocketChat.Error() == 0)
      gLog.Error("%sChat receive error - remote end closed connection.\n", L_ERRORxSTR);
    else
      gLog.Error("%sChat receive error - lost remote end:\n%s%s\n", L_ERRORxSTR,
                 L_BLANKxSTR, m_cSocketChat.ErrorStr(buf, 128));
    chatClose();
    return;
  }
  if (!m_cSocketChat.RecvBufferFull()) return;

  switch(m_nState)
  {
  case STATE_RECVxCOLORxFONT:
  {
    unsigned long testLong_1, testLong_2;
    m_cSocketChat.RecvBuffer() >> testLong_1 >> testLong_2;
    if ((testLong_1 != 0x64 && testLong_1 != 0x65) || testLong_2 != m_nUin)
    {
      char *buf;
      gLog.Error("%sChat receive error - invalid color/font packet:\n%s\n",
                 L_ERRORxSTR, m_cSocketChat.RecvBuffer().print(buf));
      delete [] buf;
      chatClose();
      return;
    }

    // just received the color/font packet

    // take out the interesting info from the font packet (only the name and
    // colors for now)
    unsigned short nameLen;
    m_cSocketChat.RecvBuffer() >> nameLen;   // length of chat name (including null)
    m_sRemoteName = new char[nameLen + 1];
    for (unsigned short i = 0; i < nameLen; i++)
       m_cSocketChat.RecvBuffer() >> m_sRemoteName[i];
    boxRemote->setTitle(tr("Remote - ") + QString::fromLocal8Bit(m_sRemoteName));

    // set up the remote colors
    char colorForeRed, colorForeGreen, colorForeBlue, 
         colorBackRed, colorBackGreen, colorBackBlue, junkChar;
    m_cSocketChat.RecvBuffer() >> colorForeRed
                               >> colorForeGreen
                               >> colorForeBlue
                               >> junkChar
                               >> colorBackRed
                               >> colorBackGreen
                               >> colorBackBlue
    ;
    QColorGroup newColorGroup(QColor((unsigned char)colorForeRed, (unsigned char)colorForeGreen, (unsigned char)colorForeBlue), 
                              QColor((unsigned char)colorBackRed, (unsigned char)colorBackGreen, (unsigned char)colorBackBlue), 
                              mleRemote->palette().normal().light(), 
                              mleRemote->palette().normal().dark(),
                              mleRemote->palette().normal().mid(), 
                              QColor((unsigned char)colorForeRed, (unsigned char)colorForeGreen, (unsigned char)colorForeBlue),
                              QColor((unsigned char)colorBackRed, (unsigned char)colorBackGreen, (unsigned char)colorBackBlue));
    mleRemote->setPalette(QPalette(newColorGroup, mleRemote->palette().disabled(), newColorGroup));

    // send the reply (font packet)
    CPChat_Font p_font(getLocalPort(), 0x0C, 0x00, "courier");
    if (!m_cSocketChat.SendPacket(p_font.getBuffer()))
    {
      char buf[128];
      gLog.Error("%sChat send error (font packet):\n%s%s\n", 
                 L_ERRORxSTR, L_BLANKxSTR, m_cSocketChat.ErrorStr(buf, 128));
      chatClose();
      return;
    }
  
    // now we are done with the handshaking
    disconnect(snChat, SIGNAL(activated(int)), this, SLOT(StateClient()));
    connect(snChat, SIGNAL(activated(int)), this, SLOT(chatRecv()));
    connect(mleLocal, SIGNAL(keyPressed(QKeyEvent *)), this, SLOT(chatSend(QKeyEvent *)));
    mleLocal->setEnabled(true);
    m_nState = STATE_RECVxCHAT;
    break;
  }

  case STATE_RECVxCHAT:
    // should never get here
    break;
  
  } // switch
  
  m_cSocketChat.ClearRecvBuffer();
}




//-----chatSend-----------------------------------------------------------------
void ChatDlg::chatSend(QKeyEvent *e)
{
   CBuffer buffer(1);
   if (e->key() == Key_Enter) 
      buffer.add((char)0x0D);
   else if (e->key() == Key_Return) 
      buffer.add((char)0x0D);
   else if (e->key() == Key_Backspace) 
      buffer.add((char)0x08);
   else if (e->key() == Key_unknown)
   {
      e->ignore();
      return;
   }
   else if ((unsigned char)e->ascii() >= 32 || (unsigned char) e->ascii() == 7)
   {
      char c = e->ascii();
      gTranslator.ClientToServer(c);
      buffer.add(c);
   }
   else return;

   if (!m_cSocketChat.SendRaw(&buffer))
   {
     char buf[128];
     gLog.Error("%sChat send error:\n%s%s\n", L_ERRORxSTR, L_BLANKxSTR, 
                m_cSocketChat.ErrorStr(buf, 128));
     chatClose();
   }
}


//-----chatRecv-----------------------------------------------------------------
void ChatDlg::chatRecv()
{
  if (!m_cSocketChat.RecvRaw())
  {
    char buf[128];
    if (m_cSocketChat.Error() == 0)
      gLog.Error("%sRemote end disconnected.\n", L_WARNxSTR);
    else
      gLog.Error("%sChat receive error, lost remote end:\n%s%s.\n", 
                 L_ERRORxSTR, L_BLANKxSTR, m_cSocketChat.ErrorStr(buf, 128));
    chatClose();
    return;
  }

  char chatChar;
  while (!m_cSocketChat.RecvBuffer().End())
  {
     m_cSocketChat.RecvBuffer() >> chatChar;
     chatQueue.push_back(chatChar);
  }
  m_cSocketChat.ClearRecvBuffer();

  while (chatQueue.size() > 0)
  {
     chatChar = *chatQueue.begin(); // first character in queue (not dequeued)
     switch (chatChar)
     {
     case 0x0D:   // new line
        mleRemote->appendChar('\n');
        chatQueue.pop_front();
        break;

     case 0x07:  // beep
        if (m_bAudio)
          QApplication::beep();
        else
          mleRemote->append("\n<--BEEP-->");
        chatQueue.pop_front();
        break;

     case 0x08:   // backspace
        mleRemote->backspace();
        chatQueue.pop_front();
        break;

     case 0x00: // change foreground color
     {
        if (chatQueue.size() < 5) return;
        unsigned char colorForeRed, colorForeGreen, colorForeBlue;
        colorForeRed = chatQueue[1];
        colorForeGreen = chatQueue[2];
        colorForeBlue = chatQueue[3];
        for (unsigned short i = 0; i < 5; i++) 
           chatQueue.pop_front();
        
        QColor newColor(colorForeRed, colorForeGreen, colorForeBlue);
        QColorGroup newColorGroup(newColor, mleRemote->palette().normal().background(), 
                                  mleRemote->palette().normal().light(), mleRemote->palette().normal().dark(), 
                                  mleRemote->palette().normal().mid(), newColor, 
                                  mleRemote->palette().normal().base());
        mleRemote->setPalette(QPalette(newColorGroup, mleRemote->palette().disabled(), newColorGroup));
        break;
     }
     case 0x01:  // change background color    
     {
        if (chatQueue.size() < 5) return;
        unsigned char colorBackRed, colorBackGreen, colorBackBlue;
        colorBackRed = chatQueue[1];
        colorBackGreen = chatQueue[2];
        colorBackBlue = chatQueue[3];
        for (unsigned short i = 0; i < 5; i++) 
           chatQueue.pop_front();

        QColor newColor(colorBackRed, colorBackGreen, colorBackBlue);
        QColorGroup newColorGroup(mleRemote->palette().normal().foreground(), newColor,
                                  mleRemote->palette().normal().light(), mleRemote->palette().normal().dark(),
                                  mleRemote->palette().normal().mid(), mleRemote->palette().normal().text(),
                                  newColor);
        mleRemote->setPalette(QPalette(newColorGroup, mleRemote->palette().disabled(), newColorGroup));
        break;
     }
     case 0x10: // change font type 
     {
        if (chatQueue.size() < 3) return;
        unsigned short sizeFontName, encodingFont, i;
        sizeFontName = chatQueue[1] + chatQueue[2] << 8;
        if (chatQueue.size() < (unsigned long)(sizeFontName + 2)) return;
        char nameFont[sizeFontName];
        for (i = 0; i < sizeFontName; i++)
           nameFont[i] = chatQueue[i + 3];
        encodingFont = chatQueue[sizeFontName + 3] + 
                       chatQueue[sizeFontName + 4] << 8;
        
        // Dequeue all characters
        for (unsigned short i = 0; i < 6 + sizeFontName; i++) 
           chatQueue.pop_front();
        break;
     }
     case 0x11: // change font style
     {
        if (chatQueue.size() < 5) return;
        unsigned long styleFont;
        styleFont = chatQueue[1] + chatQueue[2] << 8 + chatQueue[3] << 16 +
                    chatQueue[4] << 24;
        // Dequeue all characters
        for (unsigned short i = 0; i < 5; i++) 
           chatQueue.pop_front();
        break;
     }
     case 0x12: // change font size
     {
        if (chatQueue.size() < 5) return;
        unsigned long sizeFont;
        sizeFont = chatQueue[1] + chatQueue[2] << 8 + chatQueue[3] << 16 +
                    chatQueue[4] << 24;
        // Dequeue all characters
        for (unsigned short i = 0; i < 5; i++) 
           chatQueue.pop_front();
        break;
     }
     default:
        if (!iscntrl(chatChar))
        {
           gTranslator.ServerToClient(chatChar);
           mleRemote->appendChar(chatChar);
        }
        chatQueue.pop_front();
        break;
     } // switch
  } // while
}




void ChatDlg::chatClose()
{
  m_cSocketChat.CloseConnection();
  mleLocal->setReadOnly(true);
}


void ChatDlg::hide()
{
  chatClose();
  QWidget::hide();
  delete this;
}


//-----ChatDlg::resizeEvent------------------------------------------------------------------------
void ChatDlg::resizeEvent (QResizeEvent *)
{
   boxRemote->setGeometry(10, 10, width() - 20, (height() - 80) / 2);
   mleRemote->setGeometry(10, 15, boxRemote->width() - 25, boxRemote->height() - 30);
   boxLocal->setGeometry(10, boxRemote->height() + 20, width() - 20, (height() - 80) / 2);
   mleLocal->setGeometry(10, 15, boxLocal->width() - 25, boxLocal->height() - 30);
   btnClose->setGeometry((width() / 2) - 50, height() - 40, 100, 30);
   mleRemote->repaint();
}

#include "moc/moc_chatdlg.h"
