#ifndef CHATDLG_H
#define CHATDLG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qmenubar.h>
#include <qsocketnotifier.h>

#include <deque.h>

#include "mledit.h"
#include "socket.h"
#include "icq-defines.h"
#include "buffer.h"

class CICQDaemon;

class ChatDlg : public QWidget
{
   Q_OBJECT
public:
   ChatDlg(unsigned long _nUin, bool _bServer, CICQDaemon *daemon,
           unsigned short _nPort = 0,
           QWidget *parent = NULL, char *name = NULL);
   virtual ~ChatDlg();

   bool startAsClient();
   bool startAsServer();

   unsigned short getPort()  { return m_nPort; };
   unsigned short getLocalPort()  { return m_cSocketChat.LocalPort(); };
   void setPort(unsigned short _nPort)  { m_nPort = _nPort; };
   char *getLocalName()  { return m_sLocalName; };
   char *getRemoteName()  { return m_sRemoteName; };
   unsigned long Uin()  { return m_nUin; };
   bool server()  { return m_bServer; };

public slots:
   virtual void hide();

protected:
   void resizeEvent (QResizeEvent *);

   MLEditWrap *mleLocal,
              *mleRemote;
   QGroupBox *boxLocal,
             *boxRemote;
   QPushButton *btnClose;
   QMenuBar *mnuChat;
   CICQDaemon *licqDaemon;

   unsigned long m_nUin;
   CBuffer chatBuffer;
   deque <unsigned char> chatQueue;
   TCPSocket m_cSocketChat,
             m_cSocketChatServer;
   QSocketNotifier *snChat, *snChatServer;
   unsigned short m_nPort, m_nState, m_nGivenPort;
   bool m_bServer, m_bAudio;
   char *m_sRemoteName,
        *m_sLocalName;

protected slots:
   void chatSend(QKeyEvent *);
   void chatRecv();
   void StateServer();
   void StateClient();
   void chatRecvConnection();
   void chatClose();
};

#endif



