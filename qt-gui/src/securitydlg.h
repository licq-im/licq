#ifndef SECURITYDLG_H
#define SECURITYDLG_H

#include <qdialog.h>

class QPushButton;
class QCheckBox;
class ICQEvent;
class CICQEventTag;

class CICQDaemon;
class CSignalManager;

class SecurityDlg : public QDialog
{
   Q_OBJECT
public:
  SecurityDlg (CICQDaemon *s, CSignalManager *, QWidget* parent = 0);
  SecurityDlg::~SecurityDlg();
protected:
  CICQDaemon *server;
  CSignalManager *sigman;
  QPushButton *btnUpdate, *btnCancel;
  QCheckBox *chkWebAware, *chkAuthorization, *chkHideIp;

  CICQEventTag *tag;

public slots:
  void slot_doneUserFcn(ICQEvent *);
protected slots:
  void ok();
};


#endif
