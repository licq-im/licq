#ifndef PASSWORDDLG_H
#define PASSWORDDLG_H

#include <qdialog.h>

class QPushButton;
class QLineEdit;
class QLabel;
class ICQEvent;
class CICQEventTag;

class CICQDaemon;
class CSignalManager;

class PasswordDlg : public QDialog
{
  Q_OBJECT
public:
  PasswordDlg (CICQDaemon *s, CSignalManager *, QWidget* parent = 0);
  ~PasswordDlg();
protected:
  CICQDaemon *server;
  CSignalManager *sigman;
  QPushButton *btnUpdate, *btnCancel;
  QLineEdit *edtFirst, *edtSecond;
  QLabel *lblPassword, *lblVerify;

  CICQEventTag *tag;

public slots:
  void slot_doneUserFcn(ICQEvent *);

protected slots:
  virtual void accept();
};


#endif
