#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream.h>
#include <qapplication.h>
#include <ctype.h>
#include <qlayout.h>

#include "chatdlg.h"
#include "icqpacket.h"
#include "log.h"
#include "translate.h"
#include "user.h"
#include "icqd.h"
#include "ewidgets.h"

#define STATE_RECVxHANDSHAKE 1
#define STATE_RECVxCOLOR 2
#define STATE_RECVxFONT 3
#define STATE_RECVxCOLORxFONT 4
#define STATE_RECVxCHAT 5
#define STATE_CLOSED 6


ChatDlg::ChatDlg(unsigned long _nUin, CICQDaemon *daemon,
                 QWidget *parent, char *name)
   : QWidget(parent, name)
{
  m_nUin = _nUin;
  m_bAudio = true;
  licqDaemon = daemon;
  snChatServer = NULL;
  chatUser = NULL;

  m_nMode = CHAT_PANE;

  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  chatname = QString::fromLocal8Bit(o->GetAlias());
  gUserManager.DropOwner();

  // Panel mode setup
  boxPane = new QGroupBox(2, Vertical, this);

  boxRemote = new QGroupBox(1, Horizontal, tr("Remote - Not connected"), boxPane);
  mlePaneRemote = new CChatWindow(boxRemote);
  mlePaneRemote->setMinimumHeight(100);
  mlePaneRemote->setMinimumWidth(150);
  mlePaneRemote->setReadOnly(true);

  boxLocal = new QGroupBox(1, Horizontal, tr("Local - %1").arg(chatname), boxPane);
  mlePaneLocal = new CChatWindow(boxLocal);
  mlePaneLocal->setMinimumHeight(100);
  mlePaneLocal->setMinimumWidth(150);
  mlePaneLocal->setEnabled(false);

  // IRC mode setup
  boxIRC = new QGroupBox(this);
  QGridLayout *lay = new QGridLayout(boxIRC, 2, 2, 10, 5);
  mleIRCRemote = new CChatWindow(boxIRC);
  mleIRCRemote->setReadOnly(true);
  mleIRCRemote->setMinimumHeight(100);
  mleIRCRemote->setMinimumWidth(150);
  lay->addWidget(mleIRCRemote, 0, 0);
  lstUsers = new QListBox(boxIRC);
  lstUsers->insertItem(chatname);
  lay->addMultiCellWidget(lstUsers, 0, 1, 1, 1);
  mleIRCLocal = new CChatWindow(boxIRC);
  mleIRCLocal->setEnabled(false);
  mleIRCLocal->setFixedHeight(mleIRCLocal->fontMetrics().lineSpacing() * 4);
  lay->addWidget(mleIRCLocal, 1, 0);
  lay->setRowStretch(0, 1);
  lay->setColStretch(0, 1);

  // Generic setup
  mnuChat = new QMenuBar(this);
  mnuMode = new QPopupMenu(mnuChat);
  mnuMode->insertItem(tr("Pane Mode"), this, SLOT(SwitchToPaneMode()));
  mnuMode->insertItem(tr("IRC Mode"), this, SLOT(SwitchToIRCMode()));
  mnuChat->insertItem(tr("Mode"), mnuMode);
  mnuChat->insertItem(tr("Style"));

  btnClose = new QPushButton(tr("&Close Chat"), this);
  connect(btnClose, SIGNAL(clicked()), this, SLOT(hide()));

  SwitchToPaneMode();

  resize(500, 475);
  show();
}


ChatDlg::~ChatDlg()
{
  ChatUserIter iter;
  for (iter = chatUsers.begin(); iter != chatUsers.end(); iter++)
  {
    delete *iter;
  }

  if (snChatServer != NULL) delete snChatServer;
}

//=====Server===================================================================
bool ChatDlg::StartAsServer()
{
  if (!StartChatServer()) return false;

  boxRemote->setTitle(tr("Remote - Waiting for joiners..."));
  gLog.Info("%sChat: Waiting for joiners.\n", L_TCPxSTR);

  return true;
}


//-----startAsServer------------------------------------------------------------
bool ChatDlg::StartChatServer()
{
  if (licqDaemon->StartTCPServer(&m_cSocketChatServer) == -1)
  {
    WarnUser(this, tr("No more ports available, add more\nor close open chat/file sessions."));
    return false;
  }

  snChatServer = new QSocketNotifier(m_cSocketChatServer.Descriptor(), QSocketNotifier::Read);
  connect(snChatServer, SIGNAL(activated(int)), this, SLOT(chatRecvConnection()));

  return true;
}


//-----chatRecvConnection-------------------------------------------------------
void ChatDlg::chatRecvConnection()
{
  CChatUser *u = new CChatUser;
  u->font = mlePaneRemote->font();

  if (chatUsers.size() == 0) chatUser = u;

  m_cSocketChatServer.RecvConnection(u->sock);

  u->state = STATE_RECVxHANDSHAKE;
  u->sn = new QSocketNotifier(u->sock.Descriptor(), QSocketNotifier::Read);
  connect(u->sn, SIGNAL(activated(int)), this, SLOT(StateServer(int)));
  u->sn->setEnabled(true);

  chatUsers.push_back(u);

  gLog.Info("%sChat: Received connection.\n", L_TCPxSTR);
}


//-----StateServer--------------------------------------------------------------
void ChatDlg::StateServer(int sd)
{
  // Find the right user
  CChatUser *u = FindChatUser(sd);
  if (u == NULL) return;

  if (!u->sock.RecvPacket())
  {
    char buf[128];
    QString n = u->chatname;
    if (u->sock.Error() == 0)
      gLog.Info("%sChat: Remote end disconnected.\n", L_TCPxSTR);
    else
      gLog.Info("%sChat: Lost remote end:\n%s%s\n", L_TCPxSTR,
                L_BLANKxSTR, u->sock.ErrorStr(buf, 128));
    chatClose(u);
    InformUser(this, tr("%1 closed connection.").arg(n));
    return;
  }
  if (!u->sock.RecvBufferFull()) return;

  switch(u->state)
  {
    case STATE_RECVxHANDSHAKE:
    {
      // get the handshake packet
      unsigned char cHandshake;
      u->sock.RecvBuffer() >> cHandshake;
      if (cHandshake != ICQ_CMDxTCP_HANDSHAKE)
      {
        u->sock.CloseConnection();
        gLog.Error("%sChat: Receive error - bad handshake (%04X).\n", L_ERRORxSTR,
                   cHandshake);
        chatClose(u);
        return;
      }
      gLog.Info("%sChat: Received handshake (%d).\n", L_TCPxSTR, u->sock.Descriptor());
      u->state = STATE_RECVxCOLOR;
      break;
    }

    case STATE_RECVxCOLOR:  // we just received the color packet
    {
      unsigned long testLong_1, testLong_2;
      u->sock.RecvBuffer() >> testLong_1 >> testLong_2;
      if (testLong_1 != 0x64 && testLong_1 != 0x65)
      {
        char *buf;
        gLog.Error("%sChat: Invalid color packet:\n%s\n",
                   L_ERRORxSTR, u->sock.RecvBuffer().print(buf));
        delete [] buf;
        chatClose(u);
        return;
      }

      gLog.Info("%sChat: Received color packet.\n", L_TCPxSTR);

      // the only interesting thing to take out is the user chat name and colors
      char buf[128];
      u->uin = u->sock.RecvBuffer().UnpackUnsignedLong();
      u->chatname = QString::fromLocal8Bit(u->sock.RecvBuffer().UnpackString(buf));
      lstUsers->insertItem(u->chatname);
      if (u == chatUser)
        boxRemote->setTitle(tr("Remote - %1").arg(u->chatname));

      // set up the remote colors
      unsigned short junkShort;
      char colorForeRed, colorForeGreen, colorForeBlue,
           colorBackRed, colorBackGreen, colorBackBlue, junkChar;
      u->sock.RecvBuffer() >> junkShort // chat port reversed
                           >> colorForeRed >> colorForeGreen >> colorForeBlue
                           >> junkChar
                           >> colorBackRed >> colorBackGreen >> colorBackBlue;
      u->colorFore = QColor ((unsigned char)colorForeRed, (unsigned char)colorForeGreen, (unsigned char)colorForeBlue);
      u->colorBack = QColor ((unsigned char)colorBackRed, (unsigned char)colorBackGreen, (unsigned char)colorBackBlue);
      if (u == chatUser)
      {
        QPalette pal = mlePaneRemote->palette();
#if QT_VERSION >= 210
        pal.setColor(QPalette::Active, QColorGroup::Text, u->colorFore);
        pal.setColor(QPalette::Inactive, QColorGroup::Text, u->colorFore);
        pal.setColor(QPalette::Active, QColorGroup::Base, u->colorBack);
        pal.setColor(QPalette::Inactive, QColorGroup::Base, u->colorBack);
#else
        pal.setColor(QPalette::Active, QColorGroup::Text, u->colorFore);
        pal.setColor(QPalette::Normal, QColorGroup::Text, u->colorFore);
        pal.setColor(QPalette::Active, QColorGroup::Base, u->colorBack);
        pal.setColor(QPalette::Normal, QColorGroup::Base, u->colorBack);
#endif
        mlePaneRemote->setPalette(pal);
      }

      CPChat_ColorFont p_colorfont(chatname, LocalPort(), 0x000000,
                                   0xFFFFFF, mlePaneLocal->font().pointSize(),
                                   0x00, mlePaneLocal->font().family());
      if (!u->sock.SendPacket(p_colorfont.getBuffer()))
      {
        gLog.Error("%sChat: Send error (color/font packet):\n%s%s\n",
                   L_ERRORxSTR, L_BLANKxSTR, u->sock.ErrorStr(buf, 128));
        chatClose(u);
        return;
      }
      u->state = STATE_RECVxFONT;
      break;
    }

    case STATE_RECVxFONT:
    {
      unsigned long l;
      unsigned short s;
      char c, buf[128];
      u->sock.RecvBuffer() >> l >> l >> l >> l >> c >> s;

      gLog.Info("%sChat: Received font packet.\n", L_TCPxSTR);

      // just received the font reply
      u->font.setPointSize(u->sock.RecvBuffer().UnpackUnsignedLong());
      u->sock.RecvBuffer().UnpackUnsignedLong();
      u->font.setFamily(u->sock.RecvBuffer().UnpackString(buf));

      if (u == chatUser) mlePaneRemote->setFont(u->font);

      disconnect(u->sn, SIGNAL(activated(int)), this, SLOT(StateServer(int)));
      connect(u->sn, SIGNAL(activated(int)), this, SLOT(chatRecv(int)));
      connect(mlePaneLocal, SIGNAL(keyPressed(QKeyEvent *)), this, SLOT(chatSend(QKeyEvent *)));
      connect(mleIRCLocal, SIGNAL(keyPressed(QKeyEvent *)), this, SLOT(chatSend(QKeyEvent *)));
      mlePaneLocal->setEnabled(true);
      mleIRCLocal->setEnabled(true);

      u->state = STATE_RECVxCHAT;
    }

    case STATE_RECVxCHAT:
      // should never get here...
      break;

  } // switch

  u->sock.ClearRecvBuffer();
}



//=====Client===================================================================

//-----StartAsClient------------------------------------------------------------
bool ChatDlg::StartAsClient(unsigned short nPort)
{
  if (!StartChatServer()) return false;

  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
  if (u == NULL) return false;
  unsigned long ip = u->Ip();
  unsigned long realip = u->RealIp();
  gUserManager.DropUser(u);

  return ConnectToChat(ip, realip, nPort);
}


bool ChatDlg::ConnectToChat(unsigned long nIp, unsigned long nRealIp, unsigned short nPort)
{
  CChatUser *u = new CChatUser;
  u->font = mlePaneRemote->font();
  u->ip = nIp;
  u->realip = nRealIp;
  u->port = nPort;

  if (!licqDaemon->OpenConnectionToUser("chat", nIp, nRealIp, &u->sock, nPort))
  {
    WarnUser(this, tr("Unable to connect to remote chat.\n"
                      "See the network log for details."));
    delete u;
    return false;
  }

  if (chatUsers.size() == 0) chatUser = u;
  chatUsers.push_back(u);

  gLog.Info("%sChat: Connected, shaking hands.\n", L_TCPxSTR);

  // Send handshake packet:
  CPacketTcp_Handshake p_handshake(u->sock.LocalPort());
  u->sock.SendPacket(p_handshake.getBuffer());

  // Send color packet
  CPChat_Color p_color(chatname, LocalPort(), 0x000000, 0xFFFFFF);
  u->sock.SendPacket(p_color.getBuffer());

  gLog.Info("%sChat: Connected, waiting for response.\n", L_TCPxSTR);

  u->state = STATE_RECVxCOLORxFONT;
  u->sn = new QSocketNotifier(u->sock.Descriptor(), QSocketNotifier::Read);
  connect(u->sn, SIGNAL(activated(int)), this, SLOT(StateClient(int)));

  return true;
}


//-----StateClient--------------------------------------------------------------
void ChatDlg::StateClient(int sd)
{
  // Find the right user
  CChatUser *u = FindChatUser(sd);
  if (u == NULL) return;

  if (!u->sock.RecvPacket())
  {
    char buf[128];
    QString n = u->chatname;
    if (u->sock.Error() == 0)
      gLog.Info("%sChat: Remote end disconnected.\n", L_TCPxSTR);
    else
      gLog.Info("%sChat: Lost remote end:\n%s%s\n", L_TCPxSTR,
                L_BLANKxSTR, u->sock.ErrorStr(buf, 128));
    chatClose(u);
    InformUser(this, tr("%1 closed connection.").arg(n));
    return;
  }
  if (!u->sock.RecvBufferFull()) return;

  switch(u->state)
  {
    case STATE_RECVxCOLORxFONT:
    {
      unsigned long testLong_1, testLong_2;
      u->sock.RecvBuffer() >> testLong_1 >> testLong_2;
      if (testLong_1 != 0x64 && testLong_1 != 0x65)
      {
        char *buf;
        gLog.Error("%sChat: Invalid color/font packet:\n%s\n",
                   L_ERRORxSTR, u->sock.RecvBuffer().print(buf));
        delete [] buf;
        chatClose(u);
        return;
      }
      u->uin = testLong_2;

      gLog.Info("%sChat: Received color/font packet.\n", L_TCPxSTR);

      // just received the color/font packet
      char buf[128];
      u->chatname = QString::fromLocal8Bit(u->sock.RecvBuffer().UnpackString(buf));
      lstUsers->insertItem(u->chatname);
      if (u == chatUser)
        boxRemote->setTitle(tr("Remote - %1").arg(u->chatname));

      // set up the remote colors
      char colorForeRed, colorForeGreen, colorForeBlue,
           colorBackRed, colorBackGreen, colorBackBlue, junkChar;
      u->sock.RecvBuffer() >> colorForeRed >> colorForeGreen >> colorForeBlue
                           >> junkChar
                           >> colorBackRed >> colorBackGreen >> colorBackBlue
                           >> junkChar;
      u->colorFore = QColor ((unsigned char)colorForeRed, (unsigned char)colorForeGreen, (unsigned char)colorForeBlue);
      u->colorBack = QColor ((unsigned char)colorBackRed, (unsigned char)colorBackGreen, (unsigned char)colorBackBlue);
      if (u == chatUser)
      {
        QPalette pal = mlePaneRemote->palette();
#if QT_VERSION >= 210
        pal.setColor(QPalette::Active, QColorGroup::Text, u->colorFore);
        pal.setColor(QPalette::Inactive, QColorGroup::Text, u->colorFore);
        pal.setColor(QPalette::Active, QColorGroup::Base, u->colorBack);
        pal.setColor(QPalette::Inactive, QColorGroup::Base, u->colorBack);
#else
        pal.setColor(QPalette::Active, QColorGroup::Text, u->colorFore);
        pal.setColor(QPalette::Normal, QColorGroup::Text, u->colorFore);
        pal.setColor(QPalette::Active, QColorGroup::Base, u->colorBack);
        pal.setColor(QPalette::Normal, QColorGroup::Base, u->colorBack);
#endif
        mlePaneRemote->setPalette(pal);
      }

      unsigned long l;
      unsigned short s;
      u->sock.RecvBuffer() >> l >> l >> l >> l >> buf[0] >> s;
      u->font.setPointSize(u->sock.RecvBuffer().UnpackUnsignedLong());
      u->sock.RecvBuffer().UnpackUnsignedLong();
      u->font.setFamily(u->sock.RecvBuffer().UnpackString(buf));

      if (u == chatUser) mlePaneRemote->setFont(u->font);

      // send the reply (font packet)
      CPChat_Font p_font(LocalPort(), mlePaneRemote->font().pointSize(),
                         0x00, mlePaneRemote->font().family());
      if (!u->sock.SendPacket(p_font.getBuffer()))
      {
        char buf[128];
        gLog.Error("%sChat: Send error (font packet):\n%s%s\n",
                   L_ERRORxSTR, L_BLANKxSTR, u->sock.ErrorStr(buf, 128));
        chatClose(u);
        return;
      }

      // now we are done with the handshaking
      disconnect(u->sn, SIGNAL(activated(int)), this, SLOT(StateClient(int)));
      connect(u->sn, SIGNAL(activated(int)), this, SLOT(chatRecv(int)));
      connect(mlePaneLocal, SIGNAL(keyPressed(QKeyEvent *)), SLOT(chatSend(QKeyEvent *)));
      connect(mleIRCLocal, SIGNAL(keyPressed(QKeyEvent *)), SLOT(chatSend(QKeyEvent *)));
      mlePaneLocal->setEnabled(true);
      mleIRCLocal->setEnabled(true);
      u->state = STATE_RECVxCHAT;
      break;
    }

    case STATE_RECVxCHAT:
      // should never get here
      break;

  } // switch

  u->sock.ClearRecvBuffer();
}




//-----chatSend-----------------------------------------------------------------
void ChatDlg::chatSend(QKeyEvent *e)
{
  CBuffer buffer(1);
  switch (e->key())
  {
    case Key_Enter:
    case Key_Return:
    {
      buffer.PackChar(0x0D);
      mleIRCRemote->append(chatname + "> " + linebuf);
      linebuf = "";
      mleIRCLocal->clear();
      if (m_nMode == CHAT_IRC) mlePaneLocal->insertLine("");
      break;
    }
    case Key_Backspace:
    {
      buffer.PackChar(0x08);
      if (m_nMode == CHAT_IRC) mlePaneLocal->backspace();
      if (linebuf.length() > 0)
        linebuf.remove(linebuf.length() - 1, 1);
      break;
    }
    case Key_Tab:
    case Key_Backtab:
      break;

    default:
    {
      char c = e->ascii();
      gTranslator.ClientToServer(c);
      buffer.PackChar(c);
      linebuf += e->text();
      if (m_nMode == CHAT_IRC) mlePaneLocal->appendNoNewLine(e->text());
      break;
    }
  }

  ChatUserIter iter;
  CChatUser *u = NULL;
  for (iter = chatUsers.begin(); iter != chatUsers.end(); iter++)
  {
    u = *iter;

    // If the socket was closed, ignore the key event
    if (u->state != STATE_RECVxCHAT || u->sock.Descriptor() == -1) continue;

    if (!u->sock.SendRaw(&buffer))
    {
      char buf[128];
      gLog.Warn("%sChat: Send error:\n%s%s\n", L_WARNxSTR, L_BLANKxSTR,
                 u->sock.ErrorStr(buf, 128));
      chatClose(u);
      // this is bad...will probably crash after this FIXME
    }
  }
}


CChatUser *ChatDlg::FindChatUser(int sd)
{
  // Find the right user
  ChatUserIter iter;
  for (iter = chatUsers.begin(); iter != chatUsers.end(); iter++)
    if ( (*iter)->sock.Descriptor() == sd) break;

  if (iter == chatUsers.end())
  {
    gLog.Error("%sChat: No such socket (%d).\n", L_ERRORxSTR, sd);
    return NULL;
  }
  return *iter;
}


//-----chatRecv-----------------------------------------------------------------
void ChatDlg::chatRecv(int sd)
{
  CChatUser *u = FindChatUser(sd);
  if (u == NULL) return;

  if (!u->sock.RecvRaw())
  {
    char buf[128];
    if (u->sock.Error() == 0)
      gLog.Info("%sChat: Remote end disconnected.\n", L_TCPxSTR);
    else
      gLog.Info("%sChat: Lost remote end:\n%s%s\n", L_TCPxSTR,
                L_BLANKxSTR, u->sock.ErrorStr(buf, 128));
    QString n = u->chatname;
    chatClose(u);
    InformUser(this, tr("%1 closed connection.").arg(n));
    return;
  }

  char chatChar;
  while (!u->sock.RecvBuffer().End())
  {
     u->sock.RecvBuffer() >> chatChar;
     u->chatQueue.push_back(chatChar);
  }
  u->sock.ClearRecvBuffer();

  while (u->chatQueue.size() > 0)
  {
    chatChar = *u->chatQueue.begin(); // first character in queue (not dequeued)
    switch (chatChar)
    {
      case 0x0D:   // new line
        // add to irc window
        mleIRCRemote->append(u->chatname + "> " + u->linebuf);
        u->linebuf = "";
        if (u == chatUser) mlePaneRemote->insertLine("");
        u->chatQueue.pop_front();
        break;

      case 0x07:  // beep
      {
        if (m_bAudio)
          QApplication::beep();
        else
        {
          if (u == chatUser) mlePaneRemote->append("\n<--BEEP-->");
          mleIRCRemote->append(u->chatname + "> <--BEEP-->");
        }
        u->chatQueue.pop_front();
        break;
      }

      case 0x08:   // backspace
      {
        if (u->linebuf.length() > 0)
          u->linebuf.remove(u->linebuf.length() - 1, 1);
        if (u == chatUser) mlePaneRemote->backspace();
        u->chatQueue.pop_front();
        break;
      }

      case 0x00: // change foreground color
      {
        if (u->chatQueue.size() < 5) return;
        unsigned char colorForeRed, colorForeGreen, colorForeBlue;
        colorForeRed = u->chatQueue[1];
        colorForeGreen = u->chatQueue[2];
        colorForeBlue = u->chatQueue[3];
        for (unsigned short i = 0; i < 5; i++)
          u->chatQueue.pop_front();

        u->colorFore = QColor (colorForeRed, colorForeGreen, colorForeBlue);
        if (u == chatUser)
        {
          QPalette pal = mlePaneRemote->palette();
#if QT_VERSION >= 210
          pal.setColor(QPalette::Active, QColorGroup::Text, u->colorFore);
          pal.setColor(QPalette::Inactive, QColorGroup::Text, u->colorFore);
#else
          pal.setColor(QPalette::Active, QColorGroup::Text, u->colorFore);
          pal.setColor(QPalette::Normal, QColorGroup::Text, u->colorFore);
#endif
          mlePaneRemote->setPalette(pal);
        }
        break;
      }

      case 0x01:  // change background color
      {
        if (u->chatQueue.size() < 5) return;
        unsigned char colorBackRed, colorBackGreen, colorBackBlue;
        colorBackRed = u->chatQueue[1];
        colorBackGreen = u->chatQueue[2];
        colorBackBlue = u->chatQueue[3];
        for (unsigned short i = 0; i < 5; i++)
          u->chatQueue.pop_front();

        u->colorBack = QColor (colorBackRed, colorBackGreen, colorBackBlue);
        if (u == chatUser)
        {
          QPalette pal = mlePaneRemote->palette();
#if QT_VERSION >= 210
          pal.setColor(QPalette::Active, QColorGroup::Base, u->colorBack);
          pal.setColor(QPalette::Inactive, QColorGroup::Base, u->colorBack);
#else
          pal.setColor(QPalette::Active, QColorGroup::Base, u->colorBack);
          pal.setColor(QPalette::Normal, QColorGroup::Base, u->colorBack);
#endif
          mlePaneRemote->setPalette(pal);
        }
        break;
      }
      case 0x10: // change font type
      {
         if (u->chatQueue.size() < 3) return;
         unsigned short sizeFontName, encodingFont, i;
         sizeFontName = u->chatQueue[1] | (u->chatQueue[2] << 8);
         if (u->chatQueue.size() < (unsigned long)(sizeFontName + 2 + 3)) return;
         char nameFont[sizeFontName];
         for (i = 0; i < sizeFontName; i++)
            nameFont[i] = u->chatQueue[i + 3];
         encodingFont = u->chatQueue[sizeFontName + 3] |
                        (u->chatQueue[sizeFontName + 4] << 8);
         u->font.setFamily(nameFont);

         if (u == chatUser)
         {
           mlePaneRemote->setFont(u->font);
         }

         // Dequeue all characters
         for (unsigned short i = 0; i < 3 + sizeFontName + 2; i++)
           u->chatQueue.pop_front();
         break;
      }

      case 0x11: // change font style
      {
        if (u->chatQueue.size() < 5) return;
        unsigned long styleFont;
        styleFont = u->chatQueue[1] | (u->chatQueue[2] << 8) |
                    (u->chatQueue[3] << 16) | (u->chatQueue[4] << 24);
        //FIXME add font style support

        // Dequeue all characters
        for (unsigned short i = 0; i < 5; i++)
          u->chatQueue.pop_front();
        break;
      }

      case 0x12: // change font size
      {
        if (u->chatQueue.size() < 5) return;
        unsigned long sizeFont;
        sizeFont = u->chatQueue[1] | (u->chatQueue[2] << 8) |
                   (u->chatQueue[3] << 16) | (u->chatQueue[4] << 24);
        u->font.setPointSize(sizeFont);

        if (u == chatUser)
          mlePaneRemote->setFont(u->font);

        // Dequeue all characters
        for (unsigned short i = 0; i < 5; i++)
          u->chatQueue.pop_front();
        break;
      }

      default:
      {
        if (!iscntrl((int)(unsigned char)chatChar))
        {
          gTranslator.ServerToClient(chatChar);
          char tempStr[2];
          tempStr[0] = chatChar;
          tempStr[1] = 0;
          // Add to the users irc line buffer
          u->linebuf += chatChar;
          if (u == chatUser)
            mlePaneRemote->appendNoNewLine(QString::fromLocal8Bit(tempStr));
        }
        u->chatQueue.pop_front();
        break;
      }
    } // switch
  } // while
}


void ChatDlg::SwitchToIRCMode()
{
  mnuMode->setItemChecked(mnuMode->idAt(0), false);
  mnuMode->setItemChecked(mnuMode->idAt(1), true);
  boxPane->hide();
  mleIRCLocal->setText(linebuf);
  mleIRCLocal->GotoEnd();
  mleIRCLocal->setFocus();
  boxIRC->show();
}


void ChatDlg::SwitchToPaneMode()
{
  mnuMode->setItemChecked(mnuMode->idAt(0), true);
  mnuMode->setItemChecked(mnuMode->idAt(1), false);
  boxIRC->hide();
  mlePaneLocal->setFocus();
  mlePaneLocal->GotoEnd();
  boxPane->show();
}


void ChatDlg::chatClose(CChatUser *u)
{
  ChatUserIter iter;
  if (u == NULL)
  {
    for (iter = chatUsers.begin(); iter != chatUsers.end(); iter++)
    {
      (*iter)->sock.CloseConnection();
      delete (*iter);
    }
    chatUsers.clear();
    chatUser = NULL;
    lstUsers->clear();
  }
  else
  {
    // Remove the user from the list box
    for (unsigned short i = 0; i < lstUsers->count(); i++)
    {
      if (lstUsers->text(i) == u->chatname)
      {
        lstUsers->removeItem(i);
        break;
      }
    }
    // Remove the user from the user list
    for (iter = chatUsers.begin(); iter != chatUsers.end(); iter++)
    {
      if (u == *iter)
      {
        u->sock.CloseConnection();
        chatUsers.erase(iter);
        delete u;
        break;
      }
    }
    if (chatUser == u) chatUser = NULL;
  }

  // Modify the dialogs
  if (chatUser == NULL)
  {
    mlePaneLocal->setReadOnly(true);
    disconnect(mlePaneLocal, SIGNAL(keyPressed(QKeyEvent *)), this, SLOT(chatSend(QKeyEvent *)));
  }
  if (chatUsers.size() == 0)
  {
    mleIRCLocal->setReadOnly(true);
    disconnect(mleIRCLocal, SIGNAL(keyPressed(QKeyEvent *)), this, SLOT(chatSend(QKeyEvent *)));
  }
}


void ChatDlg::hide()
{
  chatClose(NULL);
  QWidget::hide();
  delete this;
}


//-----ChatDlg::resizeEvent------------------------------------------------------------------------
void ChatDlg::resizeEvent (QResizeEvent *)
{
  boxPane->setGeometry(10, mnuChat->height() + 10, width() - 20, height() - 90);
  boxIRC->setGeometry(10, mnuChat->height() + 10, width() - 20, height() - 90);

  btnClose->setGeometry((width() / 2) - (btnClose->sizeHint().width()/2), height() - 40,
                        btnClose->sizeHint().width(), btnClose->sizeHint().height());
  mlePaneRemote->repaint();
}


//=====CChatWindow===========================================================

CChatWindow::CChatWindow (QWidget *parent)
  : QMultiLineEdit(parent, 0)
{
#if QT_VERSION >= 210
  setWordWrap(WidgetWidth);
  setWrapPolicy(AtWhiteSpace);
#endif
}


void CChatWindow::appendNoNewLine(QString s)
{
  if (!atEnd()) GotoEnd();
  QMultiLineEdit::insert(s);
}

void CChatWindow::GotoEnd()
{
  setCursorPosition(numLines() - 1, lineLength(numLines() - 1) - 1);
}

void CChatWindow::insert(const QString &s)
{
  if (!atEnd()) GotoEnd();
  QMultiLineEdit::insert(s);
}


void CChatWindow::keyPressEvent (QKeyEvent *e)
{
  if ( (e->key() < Key_Space ||
        e->key() > Key_AsciiTilde ||
        e->state() & ControlButton ||
        e->state() & AltButton)
      &&
       (e->key() != Key_Tab &&
        e->key() != Key_Backtab &&
        e->key() != Key_Backspace &&
        e->key() != Key_Return &&
        e->key() != Key_Enter) )
    return;

  if (!atEnd()) GotoEnd();
  QMultiLineEdit::keyPressEvent(e);
  emit keyPressed(e);
}


void CChatWindow::paste()
{
}


void CChatWindow::paintCell(QPainter* p, int row, int col)
{
  QMultiLineEdit::paintCell(p, row, col);
}



#include "chatdlg.moc"
