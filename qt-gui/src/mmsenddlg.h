#ifndef MMSENDDLG_H
#define MMSENDDLG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include <qdialog.h>
#include <qstring.h>

class QPushButton;
class QVGroupBox;
class QProgressBar;

class CSignalManager;
class CMMUserView;
class CMMUserViewItem;
class CICQDaemon;
class CICQEventTag;
class ICQEvent;

class CMMSendDlg : public QDialog
{
  Q_OBJECT
public:
  CMMSendDlg(CICQDaemon *, CSignalManager *, CMMUserView *,
     QWidget *p = NULL, const char *n = NULL);
  ~CMMSendDlg();

  int go_message(QString);
  int go_url(QString, QString);
protected:
  QString s1, s2;
  unsigned long m_nEventType;
  QVGroupBox *grpSending;
  QPushButton *btnCancel;
  QProgressBar *barSend;
  unsigned long m_nUin;
  CMMUserView *mmv;
  CMMUserViewItem *mmvi;
  CICQDaemon *server;
  CICQEventTag *icqEventTag;

  void SendNext();
protected slots:
  void slot_done(ICQEvent *);
  void slot_cancel();
};

#endif
