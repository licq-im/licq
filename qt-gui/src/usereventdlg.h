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
#include <list>

#include "licq_color.h"
#include "licq_filetransfer.h"

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
class CMessageViewWidget;
class MLView;

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
  QTextCodec *codec;
  bool m_bOwner;
  unsigned long m_nUin;
  QBoxLayout* top_lay, *top_hlay;
  CICQDaemon *server;
  CMainWindow *mainwin;
  CSignalManager *sigman;
  std::list<unsigned long> m_lnEventTag;
  QWidget *mainWidget;
  QPushButton *btnHistory, *btnInfo, *btnEncoding, *btnSecure, *btnForeColor, *btnBackColor;
  QPopupMenu * popupEncoding;

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
  void slot_security();
  void slot_setEncoding(int encodingMib);

signals:
  void finished(unsigned long);
  void encodingChanged();
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
  // ID of the higest event we've processed. Helps determine
  // which events we already processed in the ctor.
  int m_highestEventId;

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
  void slot_setEncoding();
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
#if QT_VERSION >= 300
  virtual void windowActivationChange(bool oldActive);
#endif

signals:
  void autoCloseNotify();
  void updateUser(CICQSignal*);
  void signal_msgtypechanged(UserSendCommon *, UserSendCommon *);

protected:
  CMessageViewWidget *mleHistory;
  QSplitter * splView;
  QCheckBox *chkSendServer, *chkUrgent, *chkMass;
  QPushButton *btnSend, *btnCancel;
  QGroupBox *grpMR;
  QButtonGroup *grpCmd;
  QComboBox* cmbSendType;
  CMMUserView *lstMultipleRecipients;
  MLEditWrap *mleSend;
  CICQColor icqColor;
  int m_nCurrentType;

  void RetrySend(ICQEvent *e, bool bOnline, unsigned short nLevel);
  virtual void UserUpdated(CICQSignal *, ICQUser *);
  virtual bool sendDone(ICQEvent *) = 0;
  bool checkSecure();

  virtual void resetSettings() = 0;

protected slots:
  virtual void sendButton();
  virtual void sendDone_common(ICQEvent *);

  void changeEventType(int);
  void cancelSend();
  void massMessageToggled(bool);
  void slot_resettitle() { setCaption(m_sBaseTitle); }
  void slot_SetForegroundICQColor();
  void slot_SetBackgroundICQColor();
  void trySecure();
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

  virtual void resetSettings();

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

  virtual void resetSettings();

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
  QPushButton *btnBrowse, *btnEdit;
  ConstFileList m_lFileList;
  virtual bool sendDone(ICQEvent*);

  virtual void resetSettings();

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
                    unsigned long _nUin, QWidget* parent = 0);
  virtual ~UserSendChatEvent();

protected:
  QLabel *lblItem;
  CInfoField *edtItem;
  QPushButton *btnBrowse;
  QString m_szMPChatClients;
  unsigned short m_nMPChatPort;
  virtual bool sendDone(ICQEvent *);

  virtual void resetSettings();

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

  virtual void resetSettings();

protected slots:
  virtual void sendButton();
};


/* ----------------------------------------------------------------------------- */

class UserSendSmsEvent : public UserSendCommon
{
  Q_OBJECT
public:

  UserSendSmsEvent(CICQDaemon *s, CSignalManager *theSigMan, CMainWindow *m,
                  unsigned long _nUin, QWidget* parent = 0);
  virtual ~UserSendSmsEvent();

protected:
  QLabel *lblNumber;
  CInfoField *nfoNumber;
  QLabel *lblCount;
  CInfoField *nfoCount;

  virtual bool sendDone(ICQEvent *);

  virtual void resetSettings();

protected slots:
  virtual void sendButton();
  
  void slot_count();
};

/* ----------------------------------------------------------------------------- */

#endif
