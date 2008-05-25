// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2006 Licq developers
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

#ifndef USEREVENTDLG_H
#define USEREVENTDLG_H

#include <qframe.h>
#include <qlabel.h>
#include <qwidget.h>
#include <functional>
#include <list>
#include <string>
#include <utility>

#include "licq_color.h"
#include "licq_filetransfer.h"
#include "licq_message.h"

#ifdef USE_KDE
#include <kdeversion.h>
#else
#define KDE_IS_VERSION(a, b, c) 0
#endif

class QTabWidget;
class QBoxLayout;
class QGroupBox;
class QVGroupBox;
class QComboBox;
class QBoxLayout;
class QCheckBox;
class QPushButton;
class QTimer;
class QSplitter;
class QLineEdit;
class QGroupBox;
class QButtonGroup;
class QListViewItem;
class QLabel;
class QTextCodec;

class MLEditWrap;
class MsgView;
class CInfoField;
class CICQDaemon;
class CSignalManager;
class CMainWindow;
class CUserEvent;
class CMMUserView;
class CEButton;
class CETabWidget;
class CMessageViewWidget;
class MLView;

/* ----------------------------------------------------------------------------- */
class UserEventTabDlg : public QWidget
{
  Q_OBJECT
public:
  UserEventTabDlg(CMainWindow *mainwin, QWidget *parent = 0, const char *name = 0);
  ~UserEventTabDlg();

  void addTab(UserEventCommon *tab, int index = -1);
  void selectTab(QWidget *tab);
  void replaceTab(QWidget *oldTab, UserEventCommon *newTab);
  bool tabIsSelected(QWidget *tab);
  bool tabExists(QWidget *tab);
  void updateConvoLabel(UserEventCommon *tab);
  void updateTabLabel(ICQUser *u);
  void gotTyping(ICQUser *u, int);
  void changeMsgWinSticky(bool _bStick);

#if defined(USE_KDE) && KDE_IS_VERSION(3, 2, 0)
  virtual void setIcon(const QPixmap &icon);
#endif
  
private:
  CETabWidget *tabw;
  CMainWindow *mainwin;
  void updateTitle(QWidget *tab);
  void clearEvents(QWidget *tab);
  void flashTaskbar(bool _bFlash);

public slots:
  void slot_currentChanged(QWidget *tab);
  void moveLeft();
  void moveRight();
  void removeTab(QWidget* tab);

protected slots:
  void setMsgWinSticky();

signals:
  void signal_done();
};
/* ----------------------------------------------------------------------------- */

class UserEventCommon : public QWidget
{
  Q_OBJECT
public:
  UserEventCommon(CICQDaemon *s, CSignalManager *theSigMan, CMainWindow *m,
                  const char *_szId, unsigned long _nPPID, QWidget *parent = 0,
                  const char *name = 0);
  virtual ~UserEventCommon();

  const char *Id()  { return m_lUsers.front().c_str(); }
  unsigned long PPID()  { return m_nPPID; }
  unsigned long ConvoId() { return m_nConvoId; }
  std::list<std::string>& ConvoUsers() { return m_lUsers; }
  void SetConvoId(unsigned long n)  { m_nConvoId = n; }
  
  bool FindUserInConvo(const char* id);
  void AddEventTag(unsigned long n)  { if (n) m_lnEventTag.push_back(n); }
  void gotTyping(unsigned short);
  void changeMsgWinSticky(bool _bStick);
   
  enum type {
  	UC_MESSAGE,
  	UC_URL,
  	UC_CHAT,
  	UC_FILE,
  	UC_CONTACT,
  	UC_SMS
  };
protected:
  QTextCodec *codec;
  bool m_bOwner;
  char *m_szId;
  std::list<std::string> m_lUsers;
  unsigned long m_nPPID;
  QBoxLayout* top_lay, *top_hlay;
  CICQDaemon *server;
  CMainWindow *mainwin;
  CSignalManager *sigman;
  std::list<unsigned long> m_lnEventTag;
  QWidget *mainWidget;
  QPushButton *btnHistory, *btnInfo, *btnEncoding, *btnSecure, 
              *btnEmoticon, *btnForeColor, *btnBackColor;
  QPopupMenu * popupEncoding;

  CInfoField *nfoStatus, *nfoTimezone;
  time_t m_nRemoteTimeOffset;
  QTimer *tmrTime, *tmrTyping;
  bool m_bDeleteUser;
  QString m_sBaseTitle, m_sProgressMsg;
  unsigned long m_nConvoId;
    
  // ID of the higest event we've processed. Helps determine
  // which events we already processed in the ctor.
  int m_highestEventId;

  virtual void UserUpdated(CICQSignal* signal, const char* id = NULL,
      unsigned long ppid = 0) = 0;
  void SetGeneralInfo(ICQUser *);
  void FlashTaskbar(bool _bFlash);

protected slots:
  void slot_connectsignal();
  void slot_userupdated(CICQSignal *);
  void slot_updatetime();
  void slot_updatetyping();
  void showHistory();
  void showUserInfo();
  void slot_usermenu() { gMainWindow->SetUserMenuUser(m_szId, m_nPPID); }
  void slot_security();
  void slot_setEncoding(int encodingMib);
  void setMsgWinSticky();

signals:
  void finished(const char *, unsigned long);
  void encodingChanged();
  void viewurl(QWidget*, QString);
};


/* ----------------------------------------------------------------------------- */

class UserViewEvent : public UserEventCommon
{
  Q_OBJECT
public:

  UserViewEvent(CICQDaemon *s, CSignalManager *theSigMan, CMainWindow *m,
                  const char *_szId, unsigned long _nPPID, QWidget *parent = 0);
  virtual ~UserViewEvent();

protected:
  // widgets

  QSplitter *splRead;
  MLView *mlvRead;
  MsgView *msgView;
  CUserEvent *m_xCurrentReadEvent;
  QCheckBox* chkAutoClose;
  QPushButton *btnRead2, *btnRead3, *btnRead4, *btnReadNext;
  CEButton *btnRead1, *btnClose;

  // The currently displayed message in decoded (Unicode) form.
  QString m_messageText;

  void generateReply();
  void sendMsg(QString txt);
  void updateNextButton();
  virtual void UserUpdated(CICQSignal* sig, const char* szId = NULL, unsigned long nPPID = 0);

protected slots:
  void slot_close();
  void slot_autoClose();
  void slot_msgtypechanged(UserSendCommon *, UserSendCommon *);
  void slot_btnRead1();
  void slot_btnRead2();
  void slot_btnRead3();
  void slot_btnRead4();
  void slot_btnReadNext();
  void slot_printMessage(QListViewItem*);
  void slot_clearEvent();
  void slot_sentevent(ICQEvent *);
  void slot_setEncoding();
};


/* ----------------------------------------------------------------------------- */

class UserSendCommon : public UserEventCommon
{
  Q_OBJECT
public:

  UserSendCommon(CICQDaemon *s, CSignalManager *theSigMan, CMainWindow *m,
                 const char *_szId, unsigned long _nPPID, QWidget *parent = 0,
                 const char *name = 0);
  virtual ~UserSendCommon();
  virtual bool eventFilter(QObject *watched, QEvent *e);

  void setText(const QString& txt);
  void convoJoin(const char *, unsigned long _nConvoId);
  void convoLeave(const char *, unsigned long _nConvoId);

  virtual void windowActivationChange(bool oldActive);
  int clearDelay;
  
signals:
  void autoCloseNotify();
  void updateUser(CICQSignal*);
  void signal_msgtypechanged(UserSendCommon *, UserSendCommon *);

protected:
  CMessageViewWidget *mleHistory;
  QSplitter * splView;
  QCheckBox *chkSendServer, *chkUrgent, *chkMass;
  QPushButton *btnSend, *btnClose;
  QGroupBox *grpMR;
  QButtonGroup *grpCmd;
  QComboBox* cmbSendType;
  CMMUserView *lstMultipleRecipients;
  MLEditWrap *mleSend;
  CICQColor icqColor;
  QString strTempMsg;
  QTimer *tmrSendTyping;

  void RetrySend(ICQEvent *e, bool bOnline, unsigned short nLevel);
  virtual void UserUpdated(CICQSignal* sig, const char* szId = NULL, unsigned long nPPID = 0);
  virtual bool sendDone(ICQEvent *) = 0;
  bool checkSecure();

  virtual void resetSettings() = 0;
  virtual bool isType(int) = 0;

protected slots:
  virtual void sendButton();
  virtual void sendDone_common(ICQEvent *);

  void slot_close();
  void slot_cancelSend();
  void massMessageToggled(bool);
  void slot_resettitle();
  void slot_Emoticon();
  void slot_insertEmoticon(const QString &);
  void slot_SetForegroundICQColor();
  void slot_SetBackgroundICQColor();
  void trySecure();
  void slot_ClearNewEvents();
  void slot_textChanged();
  void slot_textChanged_timeout();
  void slot_sendServerToggled();

public slots:
  void changeEventType(int);

private:
  int tmpWidgetWidth;
  // In QT >= 3.1 we can use QWidget::isShown(),
  // but for now we implement it ourselves:
  bool m_bGrpMRIsVisible; // Remembers visibility of grpMR Widget
};


/* ----------------------------------------------------------------------------- */

class UserSendMsgEvent : public UserSendCommon
{
  Q_OBJECT
public:

  UserSendMsgEvent(CICQDaemon *s, CSignalManager *theSigMan, CMainWindow *m,
                  const char *_szId, unsigned long _nPPID, QWidget *parent = 0);
  virtual ~UserSendMsgEvent();

protected:
  virtual bool sendDone(ICQEvent *);
  virtual void resetSettings();
  virtual bool isType(int id) { return (id == UC_MESSAGE); }

protected slots:
  virtual void sendButton();
};


/* ----------------------------------------------------------------------------- */

class UserSendUrlEvent : public UserSendCommon
{
  Q_OBJECT
public:

  UserSendUrlEvent(CICQDaemon *s, CSignalManager *theSigMan, CMainWindow *m,
                  const char *_szId, unsigned long _nPPID, QWidget *parent = 0);
  virtual ~UserSendUrlEvent();
  virtual bool eventFilter(QObject *watched, QEvent *e);

  void setUrl(const QString& url, const QString& description);

protected:
  QLabel *lblItem;
  CInfoField *edtItem;
  virtual bool sendDone(ICQEvent *);
  virtual void resetSettings();
  virtual bool isType(int id) { return (id == UC_URL); }

protected slots:
  virtual void sendButton();
};


/* ----------------------------------------------------------------------------- */

class UserSendFileEvent : public UserSendCommon
{
  Q_OBJECT
public:

  UserSendFileEvent(CICQDaemon *s, CSignalManager *theSigMan, CMainWindow *m,
                  const char *_szId, unsigned long _nPPID, QWidget *parent = 0);
  virtual ~UserSendFileEvent();

  void setFile(const QString& file, const QString& description);
  void addFile(const QString& file);
  
protected:
  QLabel *lblItem;
  CInfoField *edtItem;
  QPushButton *btnBrowse, *btnEdit;
  ConstFileList m_lFileList;
  virtual bool sendDone(ICQEvent*);
  virtual void resetSettings();
  virtual bool isType(int id) { return (id == UC_FILE); }

protected slots:
  void browseFile();
  void editFileList();
  void slot_filedel(unsigned);
  virtual void sendButton();
};


/* ----------------------------------------------------------------------------- */

class UserSendChatEvent : public UserSendCommon
{
  Q_OBJECT
public:

  UserSendChatEvent(CICQDaemon *s, CSignalManager *theSigMan, CMainWindow *m,
                  const char *_szId, unsigned long _nPPID, QWidget *parent = 0);
  virtual ~UserSendChatEvent();

protected:
  QLabel *lblItem;
  CInfoField *edtItem;
  QPushButton *btnBrowse;
  QString m_szMPChatClients;
  unsigned short m_nMPChatPort;
  virtual bool sendDone(ICQEvent *);
  virtual void resetSettings();
  virtual bool isType(int id) { return (id == UC_CHAT); }

protected slots:
  virtual void sendButton();

  void InviteUser();
};


/* ----------------------------------------------------------------------------- */

class UserSendContactEvent : public UserSendCommon
{
  Q_OBJECT
public:

  UserSendContactEvent(CICQDaemon *s, CSignalManager *theSigMan, CMainWindow *m,
                  const char *_szId, unsigned long _nPPID, QWidget *parent = 0);
  virtual ~UserSendContactEvent();

  void setContact(const char *, unsigned long, const QString& alias);

protected:
  CMMUserView *lstContacts;

  virtual bool sendDone(ICQEvent *);
  virtual void resetSettings();
  virtual bool isType(int id) { return (id == UC_CONTACT); }

protected slots:
  virtual void sendButton();
};


/* ----------------------------------------------------------------------------- */

class UserSendSmsEvent : public UserSendCommon
{
  Q_OBJECT
public:

  UserSendSmsEvent(CICQDaemon *s, CSignalManager *theSigMan, CMainWindow *m,
                  const char *_szId, unsigned long _nPPID, QWidget *parent = 0);
  virtual ~UserSendSmsEvent();

protected:
  QLabel *lblNumber;
  CInfoField *nfoNumber;
  QLabel *lblCount;
  CInfoField *nfoCount;

  virtual bool sendDone(ICQEvent *);
  virtual void resetSettings();
  virtual bool isType(int id) { return (id == UC_SMS); }

protected slots:
  virtual void sendButton();

  void slot_count();
};

/* ----------------------------------------------------------------------------- */

class EmoticonLabel : public QLabel
{
  Q_OBJECT
public:
  EmoticonLabel(const QString &file, const QString &key, QWidget *parent);

signals:
  void clicked(const QString &);

protected:
  void mouseReleaseEvent(QMouseEvent *);
  
  QString m_sKey;
};
  
/* ----------------------------------------------------------------------------- */

class SelectEmoticon : public QFrame
{
  Q_OBJECT
public:
  SelectEmoticon(QWidget *parent);
  
signals:
  void selected(const QString &);
  
protected slots:
  void emoticonClicked(const QString &);
};

#endif
