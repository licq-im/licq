#ifndef RCDLG_H
#define RCDLG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


class CMainWindow;
class CICQDaemon;
class CSignalManager;
class QListBox;
class QPushButton;
class ICQEvent;


class CRandomChatDlg : public QWidget
{
Q_OBJECT
public:
  CRandomChatDlg(CMainWindow *_mainwin, CICQDaemon *s,
                 CSignalManager *_sigman, QWidget* p = 0);
  ~CRandomChatDlg();
protected:
  CMainWindow *mainwin;
  unsigned long tag;
  CICQDaemon *server;
  CSignalManager *sigman;
  QListBox *lstGroups;
  QPushButton *btnOk, *btnCancel;
protected slots:
  void slot_ok();
  void slot_doneUserFcn(ICQEvent *);
};


class CSetRandomChatGroupDlg : public QWidget
{
Q_OBJECT
public:
  CSetRandomChatGroupDlg(CICQDaemon *s, CSignalManager *_sigman,
                 QWidget *p = 0);
  ~CSetRandomChatGroupDlg();
protected:
  CMainWindow *mainwin;
  unsigned long tag;
  CICQDaemon *server;
  CSignalManager *sigman;
  QListBox *lstGroups;
  QPushButton *btnOk, *btnCancel;
protected slots:
  void slot_ok();
  void slot_doneUserFcn(ICQEvent *);
};


#endif
