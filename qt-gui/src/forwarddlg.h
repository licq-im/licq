#ifndef FORWARDDLG_H
#define FORWARDDLG_H

#include <qdialog.h>

class CMainWindow;
class QPushButton;
class CUserEvent;
class CInfoField;
class CICQDaemon;
class CSignalManager;

class CForwardDlg : public QDialog
{
Q_OBJECT
public:
  CForwardDlg(CICQDaemon *s, CSignalManager *sigMan,
              CMainWindow *_mainwin, CUserEvent *e, QWidget *p = 0);
  ~CForwardDlg();
protected:
  virtual void dragEnterEvent(QDragEnterEvent * dee);
  virtual void dropEvent(QDropEvent * de);
  QString s1, s2;
  unsigned long m_nEventType;
  CICQDaemon *server;
  CMainWindow *mainwin;
  CSignalManager *sigman;
  CInfoField *edtUser;
  QPushButton *btnOk, *btnCancel;
  unsigned long m_nUin;
protected slots:
  void slot_ok();
};

#endif
