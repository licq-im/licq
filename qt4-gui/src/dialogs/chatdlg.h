/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2006 Licq developers
 *
 * Licq is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Licq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Licq; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef CHATDLG_H
#define CHATDLG_H

#include <deque>
#include <list>

#include <QDialog>
#include <QTextEdit>

class QColor;
class QComboBox;
class QGridLayout;
class QGroupBox;
class QLabel;
class QListWidget;
class QMenu;
class QPixmap;
class QSocketNotifier;
class QTextCodec;
class QToolButton;

class CChatManager;
class CChatUser;

namespace LicqQtGui
{
class ChatWindow : public QTextEdit
{
Q_OBJECT
public:
  ChatWindow(QWidget* p);
  virtual ~ChatWindow() {}

  QString lastLine() const;
  void appendNoNewLine(QString);
  void GotoEnd();

  void setBackground(const QColor&);
  void setForeground(const QColor&);

public slots:
  virtual void insert (const QString&);
  virtual void paste();
  virtual void cut() {}
  void backspace();

private:
  virtual void keyPressEvent(QKeyEvent*);
  virtual void mousePressEvent(QMouseEvent*);
  virtual void mouseMoveEvent(QMouseEvent*);
  virtual void mouseReleaseEvent(QMouseEvent* e);

signals:
  void keyPressed(QKeyEvent*);
};


class ChatDlg;
typedef std::list<ChatDlg*> ChatDlgList;
typedef struct {
  CChatUser* u;
  ChatWindow* w;
  QLabel* l;
} UserWindowPair;
typedef std::list<UserWindowPair> ChatUserWindowsList;
typedef std::list<QPixmap*> QPixmapList;

enum ChatMode { CHAT_PANE, CHAT_IRC };


//=====ChatDlg===============================================================

class ChatDlg : public QDialog
{
   Q_OBJECT
public:
  ChatDlg(QString id, unsigned long ppid, QWidget* parent = 0);
  virtual ~ChatDlg();

  bool StartAsClient(unsigned short nPort);
  bool StartAsServer();

  unsigned short LocalPort();
  QString id()  { return myId; }
  unsigned long Ppid()  { return myPpid; }

  QString ChatClients();
  QString ChatName();

  static ChatDlgList chatDlgs;

signals:
  void encodingChanged();

private:
  CChatManager* chatman;

  ChatWindow* mlePaneLocal;
  ChatWindow* mleIRCRemote;
  ChatWindow* mleIRCLocal;
  QGridLayout* paneLayout;
  QGridLayout* remoteLayout;
  QGroupBox* boxPane;
  QGroupBox* boxIRC;
  QLabel* lblLocal;
  QLabel* lblRemote;
  QMenu* mnuMode;
  QMenu* mnuStyle;
  QMenu* mnuMain;
  QMenu* mnuFg;
  QMenu* mnuBg;
  QListWidget* lstUsers;

  QAction* tbtBold;
  QAction* tbtItalic;
  QAction* tbtUnderline;
  QAction* tbtStrikeOut;
  QAction* tbtLaugh;
  QAction* tbtBeep;
  QAction* tbtFg;
  QAction* tbtBg;
  QAction* tbtIgnore;
  QAction* tbtEncoding;

  QString linebuf, chatname;
  QComboBox* cmbFontName;
  QComboBox* cmbFontSize;

  ChatMode m_nMode;
  ChatUserWindowsList chatUserWindows;

  QString myId;
  unsigned long myPpid;
  QSocketNotifier* sn;
  bool myAudio;
  QTextCodec* codec;

  virtual void closeEvent(QCloseEvent*);
  ChatWindow* GetWindow(CChatUser* u);
  void UpdateRemotePane();

  friend class JoinChatDlg;

private slots:
  void chatSend(QKeyEvent*);
  void chatSendBeep();
  void chatClose(CChatUser*);

  void slot_chat();
  bool slot_save();
  void slot_audio(bool audio);

  void fontSizeChanged(const QString&);
  void fontNameChanged(const QString&);
  void sendFontInfo();
  void fontStyleChanged();
  void changeFrontColor(QAction* action);
  void changeBackColor(QAction* action);
  void updateRemoteStyle();

  void SwitchToPaneMode();
  void SwitchToIRCMode();
  void setEncoding(QAction* action);
};

} // namespace LicqQtGui

#endif
