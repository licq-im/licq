// -*- c-basic-offset: 2 -*-
/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifndef USEREVENTDLG_H
#define USEREVENTDLG_H

#include <qwidget.h>

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

class MLEditWrap;
class MsgView;
class CInfoField;
class CICQDaemon;
class CICQEventTag;
class CSignalManager;
class CMainWindow;
class CUserEvent;
class CMMUserView;
class CEButton;

/* ----------------------------------------------------------------------------- */

class UserEventCommon : public QWidget
{
  Q_OBJECT
public:

  UserEventCommon(CICQDaemon *s, CSignalManager *theSigMan, CMainWindow *m,
                  unsigned long _nUin, QWidget* parent = 0, const char* name =0);
  virtual ~UserEventCommon();

  unsigned long Uin() { return m_nUin; }

protected:
  bool m_bOwner;
  unsigned long m_nUin;
  QBoxLayout* top_lay, *top_hlay;
  CICQDaemon *server;
  CMainWindow *mainwin;
  CSignalManager *sigman;
  CICQEventTag *icqEventTag;
  QWidget *mainWidget;
  QPushButton *btnHistory, *btnInfo;

  CInfoField *nfoStatus, *nfoTimezone;
  time_t m_nRemoteTimeOffset;
  QTimer *tmrTime;
  bool m_bDeleteUser;
  QString m_sBaseTitle, m_sProgressMsg;

  virtual void UserUpdated(CICQSignal *, ICQUser *) = 0;
  void SetGeneralInfo(ICQUser *);

protected slots:
  void slot_userupdated(CICQSignal *);
  void slot_updatetime();
  void showHistory();
  void showUserInfo();
  void slot_usermenu() { gMainWindow->SetUserMenuUin(m_nUin); }

signals:
  void finished(unsigned long);
};


/* ----------------------------------------------------------------------------- */

class UserViewEvent : public UserEventCommon
{
  Q_OBJECT
public:

  UserViewEvent(CICQDaemon *s, CSignalManager *theSigMan, CMainWindow *m,
                  unsigned long _nUin, QWidget* parent = 0);
  virtual ~UserViewEvent();

protected:
  QSplitter *splRead;
  MLEditWrap *mleRead;
  MsgView *msgView;
  CUserEvent *m_xCurrentReadEvent;
  QCheckBox* chkAutoClose;
  QPushButton *btnRead2, *btnRead3, *btnRead4, *btnReadNext;
  CEButton *btnRead1, *btnClose;

  void generateReply();
  void sendMsg(QString txt);
  void updateNextButton();
  virtual void UserUpdated(CICQSignal *, ICQUser *);

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
  void slot_sentevent(ICQEvent *);
};


/* ----------------------------------------------------------------------------- */

class UserSendCommon : public UserEventCommon
{
  Q_OBJECT
public:

  UserSendCommon(CICQDaemon *s, CSignalManager *theSigMan, CMainWindow *m,
                 unsigned long _nUin, QWidget* parent = 0, const char* name=0);
  virtual ~UserSendCommon();

  void setText(const QString& txt);

signals:
  void autoCloseNotify();
  void updateUser(CICQSignal*);
  void signal_msgtypechanged(UserSendCommon *, UserSendCommon *);

protected:
  QCheckBox *chkSendServer, *chkUrgent, *chkMass;
  QPushButton *btnSend, *btnCancel;
  QGroupBox *grpMR;
  QButtonGroup *grpCmd;
  QComboBox* cmbSendType;
  CMMUserView *lstMultipleRecipients;
  MLEditWrap *mleSend;
  QString m_msgTextTotal, m_msgTextCurrent;

  void RetrySend(ICQEvent *e, bool bOnline, unsigned short nLevel);
  QString generatePart(const QString& text);
  virtual void UserUpdated(CICQSignal *, ICQUser *);
  virtual bool sendDone(ICQEvent *) = 0;

protected slots:
  virtual void sendButton();
  virtual void sendDone_common(ICQEvent *);

  void changeEventType(int);
  void cancelSend();
  void massMessageToggled(bool);
  void slot_resettitle() { setCaption(m_sBaseTitle); }
};


/* ----------------------------------------------------------------------------- */

class UserSendMsgEvent : public UserSendCommon
{
  Q_OBJECT
public:

  UserSendMsgEvent(CICQDaemon *s, CSignalManager *theSigMan, CMainWindow *m,
                  unsigned long _nUin, QWidget* parent = 0);
  virtual ~UserSendMsgEvent();

protected:
  virtual bool sendDone(ICQEvent *);

protected slots:
  virtual void sendButton();
};


/* ----------------------------------------------------------------------------- */

class UserSendUrlEvent : public UserSendCommon
{
  Q_OBJECT
public:

  UserSendUrlEvent(CICQDaemon *s, CSignalManager *theSigMan, CMainWindow *m,
                  unsigned long _nUin, QWidget* parent = 0);
  virtual ~UserSendUrlEvent();

  void setUrl(const QString& url, const QString& description);
protected:
  QLabel *lblItem;
  CInfoField *edtItem;
  virtual bool sendDone(ICQEvent *);

protected slots:
  virtual void sendButton();
};


/* ----------------------------------------------------------------------------- */

class UserSendFileEvent : public UserSendCommon
{
  Q_OBJECT
public:

  UserSendFileEvent(CICQDaemon *s, CSignalManager *theSigMan, CMainWindow *m,
                  unsigned long _nUin, QWidget* parent = 0);
  virtual ~UserSendFileEvent();

  void setFile(const QString& file, const QString& description);

protected:
  QLabel *lblItem;
  CInfoField *edtItem;
  QPushButton *btnBrowse;
  virtual bool sendDone(ICQEvent*);

protected slots:
  void browseFile();
  virtual void sendButton();
};


/* ----------------------------------------------------------------------------- */

class UserSendChatEvent : public UserSendCommon
{
  Q_OBJECT
public:

  UserSendChatEvent(CICQDaemon *s, CSignalManager *theSigMan, CMainWindow *m,
                    unsigned long _nUin, QWidget* parent = 0);
  virtual ~UserSendChatEvent();

protected:
  QLabel *lblItem;
  CInfoField *edtItem;
  QPushButton *btnBrowse;
  QString m_szMPChatClients;
  unsigned short m_nMPChatPort;
  virtual bool sendDone(ICQEvent *);

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
                       unsigned long _nUin, QWidget* parent = 0);
  virtual ~UserSendContactEvent();

  void setContact(unsigned long uin, const QString& alias);

protected:
  CMMUserView *lstContacts;

  virtual bool sendDone(ICQEvent *);

protected slots:
  virtual void sendButton();
};


/* ----------------------------------------------------------------------------- */

#endif
