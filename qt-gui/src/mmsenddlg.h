#ifndef MMSENDDLG_H
#define MMSENDDLG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include <qstring.h>

#include "licqdialog.h"
#include "licq_user.h"

class QPushButton;
class QVGroupBox;
class QProgressBar;

class CSignalManager;
class CMMUserView;
class CMMUserViewItem;
class CICQDaemon;
class ICQEvent;

class CMMSendDlg : public LicqDialog
{
  Q_OBJECT
public:
  CMMSendDlg(CICQDaemon *, CSignalManager *, CMMUserView *,
     QWidget *p = 0);
  ~CMMSendDlg();

  int go_message(QString);
  int go_url(QString, QString);
  int go_contact(UserStringList &_users);

protected:
  QString s1, s2;
  UserStringList *users;

  unsigned long m_nEventType;
  QVGroupBox *grpSending;
  QPushButton *btnCancel;
  QProgressBar *barSend;
  unsigned long m_nUin;
  char *m_szId;
  unsigned long m_nPPID;
  CMMUserView *mmv;
  CMMUserViewItem *mmvi;
  CICQDaemon *server;
  unsigned long icqEventTag;

  void SendNext();
protected slots:
  void slot_done(ICQEvent *);
  void slot_cancel();
};

#endif
