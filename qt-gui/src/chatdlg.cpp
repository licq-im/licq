#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream.h>
#include <qapplication.h>
#include <ctype.h>
#include <qlayout.h>
#include <stdlib.h>
#include <qlistbox.h>
#include <qlabel.h>
#include <qsocketnotifier.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qmenubar.h>
#include <qpopupmenu.h>


#include "chatdlg.h"
#include "ewidgets.h"
#include "licq_packets.h"
#include "licq_log.h"
#include "licq_translate.h"
#include "licq_user.h"
#include "licq_icqd.h"

#define STATE_RECVxHANDSHAKE 1
#define STATE_RECVxCOLOR 2
#define STATE_RECVxFONT 3
#define STATE_RECVxCOLORxFONT 4
#define STATE_RECVxCHAT 5
#define STATE_CLOSED 6


ChatDlgList ChatDlg::chatDlgs;


ChatDlg::ChatDlg(unsigned long _nUin, CICQDaemon *daemon,
                 QWidget *parent, char *name)
   : QWidget(parent, name)
{
  m_nUin = _nUin;
  m_bAudio = true;
  licqDaemon = daemon;
  snChatServer = NULL;
  chatUser = NULL;
  m_nSession = rand();

  m_nMode = CHAT_PANE;

  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  chatname = QString::fromLocal8Bit(o->GetAlias());
  gUserManager.DropOwner();

  // Panel mode setup
  boxPane = new QGroupBox(this);
  QGridLayout *play = new QGridLayout(boxPane, 5, 1, 10, 5);

  lblRemote = new QLabel(tr("Remote - Not connected"), boxPane);
  mlePaneRemote = new CChatWindow(boxPane);
  mlePaneRemote->setMinimumHeight(100);
  mlePaneRemote->setMinimumWidth(150);
  mlePaneRemote->setReadOnly(true);
  play->addWidget(lblRemote, 0, 0);
  play->addWidget(mlePaneRemote, 1, 0);
  play->setRowStretch(1, 1);
  play->addRowSpacing(2, 15);

  lblLocal = new QLabel(tr("Local - %1").arg(chatname), boxPane);
  mlePaneLocal = new CChatWindow(boxPane);
  mlePaneLocal->setMinimumHeight(100);
  mlePaneLocal->setMinimumWidth(150);
  mlePaneLocal->setEnabled(false);
  play->addWidget(lblLocal, 3, 0);
  play->addWidget(mlePaneLocal, 4, 0);
  play->setRowStretch(4, 1);

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
  btnClose->setFixedSize(btnClose->sizeHint());
  connect(btnClose, SIGNAL(clicked()), this, SLOT(hide()));

  QGridLayout *g = new QGridLayout(this, 2, 1, 10, 5);
  g->setMenuBar(mnuChat);
  g->addWidget(boxPane, 0, 0);
  g->addWidget(boxIRC, 0, 0);
  g->addWidget(btnClose, 1, 0);

  SwitchToPaneMode();

  // Add ourselves to the list
  chatDlgs.push_back(this);

  resize(500, 475);
  show();
}


ChatDlg::~ChatDlg()
{
  for (ChatUserList::iterator iter = chatUsers.begin();
       iter != chatUsers.end(); iter++)
  {
    delete *iter;
  }

  if (snChatServer != NULL) delete snChatServer;

  list<ChatDlg *>::iterator iter;
  for (iter = chatDlgs.begin(); iter != chatDlgs.end(); iter++)
  {
    if (this == *iter)
    {
      chatDlgs.erase(iter);
      break;
    }
  }
}


//=====Server===================================================================
bool ChatDlg::StartAsServer()
{
  if (!StartChatServer()) return false;

  lblRemote->setText(tr("Remote - Waiting for joiners..."));
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

  if (chatUsers.size() == 0)
  {
    chatUser = u;
  }
  else
  {
    SwitchToIRCMode();
    mnuMode->setItemEnabled(mnuMode->idAt(0), false);
  }

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
    if (n.isEmpty()) n.setNum(u->uin);
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
      CChatClient c;
      if (!c.LoadFromHandshake(u->sock.RecvBuffer()))
      {
        u->sock.CloseConnection();
        gLog.Warn("%sChat: Bad handshake.\n", L_ERRORxSTR);
        chatClose(u);
        return;
      }
      gLog.Info("%sChat: Received handshake from %ld.\n", L_TCPxSTR, c.m_nUin);
      u->client = c;
      u->uin = c.m_nUin;
      u->state = STATE_RECVxCOLOR;
      break;
    }

    case STATE_RECVxCOLOR:  // we just received the color packet
    {
      gLog.Info("%sChat: Received color packet.\n", L_TCPxSTR);

      CPChat_Color pin(u->sock.RecvBuffer());

      u->chatname = QString::fromLocal8Bit(pin.Name());
      // Fill in the remaining fields in the client structure
      u->client.m_nPort = pin.Port();
      u->client.m_nSession = m_nSession;
      lstUsers->insertItem(u->chatname);
      if (u == chatUser)
        lblRemote->setText(tr("Remote - %1").arg(u->chatname));

      // set up the remote colors
      u->colorFore = QColor (pin.ColorForeRed(), pin.ColorForeGreen(), pin.ColorForeBlue());
      u->colorBack = QColor (pin.ColorBackRed(), pin.ColorBackGreen(), pin.ColorBackBlue());
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

      // Send the response
      ChatClientPList l;
      ChatUserList::iterator iter;
      for (iter = chatUsers.begin(); iter != chatUsers.end(); iter++)
      {
        // Skip this guys client info and anybody we haven't connected to yet
        if ((*iter)->uin == u->uin || (*iter)->client.m_nUin == 0) continue;
        l.push_back(&(*iter)->client);
      }

      CPChat_ColorFont p_colorfont(chatname, LocalPort(), m_nSession,
         0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF,
         mlePaneLocal->font().pointSize(), false, false, false,
         mlePaneLocal->font().family(), l);
      if (!u->sock.SendPacket(p_colorfont.getBuffer()))
      {
        char buf[128];
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
      gLog.Info("%sChat: Received font packet.\n", L_TCPxSTR);
      CPChat_Font pin(u->sock.RecvBuffer());

      // just received the font reply
      m_nSession = pin.Session();
      u->font.setPointSize(pin.FontSize());
      u->font.setBold(pin.FontBold());
      u->font.setItalic(pin.FontItalic());
      u->font.setUnderline(pin.FontUnderline());
      u->font.setFamily(pin.FontFamily());

      if (u == chatUser) mlePaneRemote->setFont(u->font);

      disconnect(u->sn, SIGNAL(activated(int)), this, SLOT(StateServer(int)));
      connect(u->sn, SIGNAL(activated(int)), this, SLOT(chatRecv(int)));
      if (u == chatUser)
      {
        connect(mlePaneLocal, SIGNAL(keyPressed(QKeyEvent *)), this, SLOT(chatSend(QKeyEvent *)));
        connect(mleIRCLocal, SIGNAL(keyPressed(QKeyEvent *)), this, SLOT(chatSend(QKeyEvent *)));
        mlePaneLocal->setEnabled(true);
        mleIRCLocal->setEnabled(true);
      }

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
  CChatClient c(u);
  c.m_nPort = nPort;
  gUserManager.DropUser(u);

  return ConnectToChat(c);
}


bool ChatDlg::ConnectToChat(CChatClient &c)
{
  CChatUser *u = new CChatUser;
  u->font = mlePaneRemote->font();
  u->client = c;
  u->client.m_nSession = m_nSession;
  u->uin = c.m_nUin;

  gLog.Info("%sChat: Connecting to server.\n", L_TCPxSTR);
  if (!licqDaemon->OpenConnectionToUser("chat", c.m_nIp, c.m_nRealIp, &u->sock, c.m_nPort))
  {
    WarnUser(this, tr("Unable to connect to remote chat.\n"
                      "See the network log for details."));
    delete u;
    return false;
  }

  if (chatUsers.size() == 0)
  {
    chatUser = u;
  }
  else
  {
    SwitchToIRCMode();
    mnuMode->setItemEnabled(mnuMode->idAt(0), false);
  }
  chatUsers.push_back(u);

  gLog.Info("%sChat: Shaking hands.\n", L_TCPxSTR);

  // Send handshake packet:
  CPacketTcp_Handshake p_handshake(u->sock.LocalPort());
  u->sock.SendPacket(p_handshake.getBuffer());

  // Send color packet
  CPChat_Color p_color(chatname, LocalPort(), 0x00, 0x00, 0x00,
     0xFF, 0xFF, 0xFF);
  u->sock.SendPacket(p_color.getBuffer());

  gLog.Info("%sChat: Waiting for color/font response.\n", L_TCPxSTR);

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
    if (n.isEmpty()) n.setNum(u->uin);
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
      gLog.Info("%sChat: Received color/font packet.\n", L_TCPxSTR);

      CPChat_ColorFont pin(u->sock.RecvBuffer());
      u->uin = pin.Uin();
      m_nSession = pin.Session();

      // just received the color/font packet
      u->chatname = QString::fromLocal8Bit(pin.Name());
      lstUsers->insertItem(u->chatname);
      if (u == chatUser)
        lblRemote->setText(tr("Remote - %1").arg(u->chatname));

      // set up the remote colors
      u->colorFore = QColor (pin.ColorForeRed(), pin.ColorForeGreen(), pin.ColorForeBlue());
      u->colorBack = QColor (pin.ColorBackRed(), pin.ColorBackGreen(), pin.ColorBackBlue());
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

      u->font.setPointSize(pin.FontSize());
      u->font.setBold(pin.FontBold());
      u->font.setItalic(pin.FontItalic());
      u->font.setUnderline(pin.FontUnderline());
      u->font.setFamily(pin.FontFamily());
      if (u == chatUser) mlePaneRemote->setFont(u->font);

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
         mlePaneRemote->font().pointSize(),
         false, false, false, mlePaneRemote->font().family());
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
      if (u == chatUser)
      {
        connect(mlePaneLocal, SIGNAL(keyPressed(QKeyEvent *)), SLOT(chatSend(QKeyEvent *)));
        connect(mleIRCLocal, SIGNAL(keyPressed(QKeyEvent *)), SLOT(chatSend(QKeyEvent *)));
        mlePaneLocal->setEnabled(true);
        mleIRCLocal->setEnabled(true);
      }
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
      QCString charbuf = e->text().local8Bit();
      gTranslator.ClientToServer(charbuf[0]);
      buffer.PackChar(charbuf[0]);
      linebuf += e->text();
      if (m_nMode == CHAT_IRC) mlePaneLocal->appendNoNewLine(e->text());
      break;
    }
  }

  ChatUserList::iterator iter;
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


QString ChatDlg::ChatClients()
{
  QString n;
  ChatUserList::iterator iter;
  for (iter = chatUsers.begin(); iter != chatUsers.end(); iter++)
  {
    if (*iter != chatUser)
    {
      QString m = (*iter)->chatname;
      if (m.isEmpty()) m.setNum((*iter)->uin);
      if (!n.isEmpty()) n += ", ";
      n += m;
    }
  }
  return n;
}


CChatUser *ChatDlg::FindChatUser(int sd)
{
  // Find the right user
  ChatUserList::iterator iter;
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
    if (n.isEmpty()) n.setNum(u->uin);
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
          u->linebuf += QString::fromLocal8Bit(tempStr);
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
  ChatUserList::iterator iter;
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
