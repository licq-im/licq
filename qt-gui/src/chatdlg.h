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


class ChatDlg : public QWidget
{
   Q_OBJECT
public:
   ChatDlg(unsigned long _nUin, bool _bServer, unsigned short _nPort = 0, 
           QWidget *parent = NULL, char *name = NULL);
   virtual ~ChatDlg(void);

   bool startAsClient(void);
   bool startAsServer(void);
   
   unsigned short getPort(void)  { return m_nPort; };
   unsigned short getLocalPort(void)  { return m_cSocketChat.LocalPort(); };
   void setPort(unsigned short _nPort)  { m_nPort = _nPort; };
   char *getLocalName(void)  { return m_sLocalName; };
   char *getRemoteName(void)  { return m_sRemoteName; };
   unsigned long Uin(void)  { return m_nUin; };
   bool server(void)  { return m_bServer; };

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
   
   unsigned long m_nUin;
   CBuffer chatBuffer;
   deque <unsigned char> chatQueue;
   TCPSocket m_cSocketChat, 
             m_cSocketChatServer;
   QSocketNotifier *snChat, *snChatServer;
   unsigned short m_nPort, m_nState;
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



