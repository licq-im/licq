#ifndef CHATDLG_H
#define CHATDLG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qcolor.h>
#include <qfont.h>
#include <qmainwindow.h>
#include <qmultilineedit.h>
#include <qsocketnotifier.h>

#include <deque.h>

#include "mledit.h"
#include "licq_socket.h"
#include "licq_icq.h"
#include "licq_buffer.h"
#include "licq_packets.h"

class QLabel;
class QListBox;
class QMenuBar;
class QPopupMenu;
class QGroupBox;
class QPushButton;
class QToolButton;
class QComboBox;

class CICQDaemon;

//=====CChatWindow===========================================================
class CChatWindow : public QMultiLineEdit
{
Q_OBJECT
public:
  CChatWindow(QWidget *p);
  void appendNoNewLine(QString);
  void GotoEnd();

  void setBackground(const QColor&);
  void setForeground(const QColor&);

public slots:
  virtual void insert (const QString &);
  virtual void paste();
  virtual void cut() {}
  virtual void backspace() { QMultiLineEdit::backspace(); }

protected:
  virtual void keyPressEvent (QKeyEvent *);

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
class ChatDlg;
typedef list<ChatDlg *> ChatDlgList;

enum ChatMode { CHAT_PANE, CHAT_IRC };


//=====ChatDlg===============================================================

class ChatDlg : public QMainWindow
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

  QString ChatClients();

  static ChatDlgList chatDlgs;

protected:
  bool StartChatServer();
  bool ConnectToChat(CChatClient &);
  CChatUser *FindChatUser(int sd);

  CChatWindow *mlePaneLocal, *mlePaneRemote, *mleIRCRemote, *mleIRCLocal;
  QGroupBox *boxPane, *boxIRC;
  QLabel *lblLocal, *lblRemote;
  QPopupMenu *mnuMode, *mnuStyle, *mnuMain, *mnuFg, *mnuBg;
  CICQDaemon *licqDaemon;
  QListBox *lstUsers;

  QToolButton* tbtBold, *tbtItalic, *tbtUnderline;
  QToolButton* tbtLaugh, *tbtBeep, *tbtFg, *tbtBg;

  QString chatname, linebuf;
  QComboBox *cmbFontName, *cmbFontSize;

  ChatMode m_nMode;
  ChatUserList chatUsers;
  CChatUser *chatUser;
  unsigned long m_nUin;
  unsigned short m_nSession;
  TCPSocket m_cSocketChatServer;
  QSocketNotifier *snChatServer;
  bool m_bAudio;

  virtual void hideEvent(QHideEvent*);

  friend class CJoinChatDlg;

protected slots:
  void chatSend(QKeyEvent *);
  void chatSendBuffer(const CBuffer*);
  void chatSendBeep();
  void chatRecv(int);
  void StateServer(int);
  void StateClient(int);
  void chatRecvConnection();
  void chatClose(CChatUser *);

  void fontSizeChanged(const QString&);
  void fontNameChanged(const QString&);
  void fontStyleChanged();
  void changeFrontColor();
  void changeBackColor();

  void SwitchToPaneMode();
  void SwitchToIRCMode();
};

#endif



