#ifndef CHATDLG_H
#define CHATDLG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qcolor.h>
#include <qfont.h>
#include <qmultilineedit.h>
#include <qsocketnotifier.h>

#include <deque.h>

#include "mledit.h"
#include "socket.h"
#include "icq-defines.h"
#include "buffer.h"
#include "icqpacket.h"

class QLabel;
class QListBox;
class QMenuBar;
class QPopupMenu;
class QGroupBox;
class QPushButton;

class CICQDaemon;

//=====CChatWindow===========================================================
class CChatWindow : public QMultiLineEdit
{
Q_OBJECT
public:
  CChatWindow(QWidget *p);
  void appendNoNewLine(QString);
  void GotoEnd();

public slots:
  virtual void insert (const QString &);
  virtual void paste();
  virtual void cut() {}
  virtual void backspace() { QMultiLineEdit::backspace(); }

protected:
  virtual void keyPressEvent (QKeyEvent *);
  void paintCell(QPainter* p, int row, int col);

signals:
  void keyPressed(QKeyEvent *);
};


//=====CChatUser=============================================================
class CChatUser
{
public:
  CChatUser()  { sn = NULL; }
  ~CChatUser() { if (sn != NULL) delete sn; }

  TCPSocket sock;
  unsigned long uin;
  QString chatname, linebuf;
  deque <unsigned char> chatQueue;
  QSocketNotifier *sn;
  unsigned short state;

  QColor colorFore, colorBack;
  QFont font;

  CChatClient client;
};

typedef list<CChatUser *> ChatUserList;
typedef ChatUserList::iterator ChatUserIter;

enum ChatMode { CHAT_PANE, CHAT_IRC };


//=====ChatDlg===============================================================

class ChatDlg : public QWidget
{
   Q_OBJECT
public:
  ChatDlg(unsigned long _nUin, CICQDaemon *daemon,
          QWidget *parent = NULL, char *name = NULL);
  virtual ~ChatDlg();

  bool StartAsClient(unsigned short nPort);
  bool StartAsServer();

  unsigned short LocalPort() { return m_cSocketChatServer.LocalPort(); }
  unsigned long Uin()  { return m_nUin; };

public slots:
  virtual void hide();

protected:
  bool StartChatServer();
  bool ConnectToChat(CChatClient &);
  CChatUser *FindChatUser(int sd);

  CChatWindow *mlePaneLocal, *mlePaneRemote, *mleIRCRemote, *mleIRCLocal;
  QGroupBox *boxPane, *boxIRC;
  QLabel *lblLocal, *lblRemote;
  QPushButton *btnClose;
  QMenuBar *mnuChat;
  QPopupMenu *mnuMode, *mnuStyle;
  CICQDaemon *licqDaemon;
  QListBox *lstUsers;

  QString chatname, linebuf;

  ChatMode m_nMode;
  ChatUserList chatUsers;
  CChatUser *chatUser;
  unsigned long m_nUin;
  unsigned short m_nSession;
  TCPSocket m_cSocketChatServer;
  QSocketNotifier *snChatServer;
  bool m_bAudio;

protected slots:
  void chatSend(QKeyEvent *);
  void chatRecv(int);
  void StateServer(int);
  void StateClient(int);
  void chatRecvConnection();
  void chatClose(CChatUser *);

  void SwitchToPaneMode();
  void SwitchToIRCMode();
};

#endif



