#ifndef CHATDLG_H
#define CHATDLG_H

#include <list.h>
#include <qmultilineedit.h>

#include <qmainwindow.h>
#ifdef USE_KDE
#include <kmainwindow.h>
#endif

#include <deque.h>

#include "mledit.h"

class CChatManager;
class CChatUser;

class QColor;
class QLabel;
class QListBox;
class QPopupMenu;
class QGroupBox;
class QPushButton;
class QToolButton;
class QComboBox;
class QCloseEvent;
class QMouseEvent;
class QSocketNotifier;
class QTextCodec;

class CICQDaemon;
class CMainWindow;

//=====CChatWindow===========================================================
class CChatWindow : public QMultiLineEdit
{
Q_OBJECT
public:
  CChatWindow(QWidget *p);
  virtual ~CChatWindow() {}

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
  virtual void mousePressEvent( QMouseEvent * );
  virtual void mouseMoveEvent( QMouseEvent*);
  virtual void mouseReleaseEvent( QMouseEvent *e );

signals:
  void keyPressed(QKeyEvent *);
};


class ChatDlg;
typedef list<ChatDlg *> ChatDlgList;

enum ChatMode { CHAT_PANE, CHAT_IRC };


//=====ChatDlg===============================================================

class ChatDlg : public QMainWindow
{
   Q_OBJECT
public:
  ChatDlg(unsigned long _nUin, CICQDaemon *daemon, CMainWindow *m,
          QWidget *parent = 0);
  virtual ~ChatDlg();

  bool StartAsClient(unsigned short nPort);
  bool StartAsServer();

  unsigned short LocalPort();
  unsigned long Uin()  { return m_nUin; };

  QString ChatClients();

  static ChatDlgList chatDlgs;

signals:
  void encodingChanged();

protected:
  CChatManager *chatman;

  CChatWindow *mlePaneLocal, *mlePaneRemote, *mleIRCRemote, *mleIRCLocal;
  QGroupBox *boxPane, *boxIRC;
  QLabel *lblLocal, *lblRemote;
  QPopupMenu *mnuMode, *mnuStyle, *mnuMain, *mnuFg, *mnuBg;
  CICQDaemon *licqDaemon;
  CMainWindow *mainwin;
  QListBox *lstUsers;

  QToolButton *tbtBold, *tbtItalic, *tbtUnderline;
  QToolButton *tbtLaugh, *tbtBeep, *tbtFg, *tbtBg;
  QToolButton *tbtIgnore, *tbtEncoding;

  QString linebuf, chatname;
  QComboBox *cmbFontName, *cmbFontSize;

  ChatMode m_nMode;
  CChatUser *chatUser;
  unsigned long m_nUin;
  QSocketNotifier *sn;
  bool m_bAudio;
  QTextCodec *codec;

  virtual void closeEvent(QCloseEvent*);

  friend class CJoinChatDlg;

protected slots:
  void chatSend(QKeyEvent *);
  void chatSendBeep();
  void chatClose(CChatUser *);

  void slot_chat();
  bool slot_save();
  void slot_audio();

  void fontSizeChanged(const QString&);
  void fontNameChanged(const QString&);
  void fontStyleChanged();
  void changeFrontColor();
  void changeBackColor();
  void toggleSettingsIgnore();

  void SwitchToPaneMode();
  void SwitchToIRCMode();
  void slot_setEncoding(int encoding_index);
};

#endif



