#ifndef RCDLG_H
#define RCDLG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


class CMainWindow;
class CICQEventTag;
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
                 CSignalManager *_sigman,
                 QWidget *p = NULL, const char *n = NULL);
  ~CRandomChatDlg();
protected:
  CMainWindow *mainwin;
  CICQEventTag *tag;
  CICQDaemon *server;
  CSignalManager *sigman;
  QListBox *lstGroups;
  QPushButton *btnOk, *btnCancel;
protected slots:
  virtual void hide();
  void slot_ok();
  void slot_doneUserFcn(ICQEvent *);
};

#endif
