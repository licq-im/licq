#ifndef SECURITYDLG_H
#define SECURITYDLG_H

#include <qwidget.h>
#include <qdialog.h>
#include <qlineedit.h>
#include <qlabel.h>

class QPushButton;
class QCheckBox;
class ICQEvent;

class CICQDaemon;
class CSignalManager;

class SecurityDlg : public QDialog
{
   Q_OBJECT
public:
  SecurityDlg (CICQDaemon *s, CSignalManager *, QWidget* parent = 0);
  ~SecurityDlg();

protected:
  CICQDaemon *server;
  CSignalManager *sigman;
  QPushButton *btnUpdate, *btnCancel;
  QCheckBox *chkWebAware, *chkAuthorization, *chkHideIp, *chkOnlyLocal;
  QLineEdit *edtUin, *edtFirst, *edtSecond;
  QLabel *lblUin, *lblPassword, *lblVerify;

  // Some returned events
  unsigned long eSecurityInfo, ePasswordChange;

  // For saving initial values
  bool initAuthorization, initWebAware, initHideIp;
  QString initEdtUin, initEdtFirst, initEdtSecond;

public slots:
  void slot_doneUserFcn(ICQEvent *);

protected slots:
  void ok();
  void slot_chkOnlyLocalToggled(bool b);
};


#endif
