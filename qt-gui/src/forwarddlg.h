#ifndef FORWARDDLG_H
#define FORWARDDLG_H

#include "licqdialog.h"

class CMainWindow;
class QPushButton;
class CUserEvent;
class CInfoField;
class CICQDaemon;
class CSignalManager;

class CForwardDlg : public LicqDialog
{
Q_OBJECT
public:
  CForwardDlg(CSignalManager *sigMan, CUserEvent *e, QWidget *p = 0);
  ~CForwardDlg();
protected:
  virtual void dragEnterEvent(QDragEnterEvent * dee);
  virtual void dropEvent(QDropEvent * de);
  QString s1, s2;
  unsigned long m_nEventType;
  CSignalManager *sigman;
  CInfoField *edtUser;
  QPushButton *btnOk, *btnCancel;
  unsigned long m_nUin;
protected slots:
  void slot_ok();
};

#endif
