#ifndef PASSWORDDLG_H
#define PASSWORDDLG_H

#include <qdialog.h>

class QPushButton;
class QLineEdit;
class ICQEvent;
class CICQEventTag;

class CICQDaemon;
class CSignalManager;

class PasswordDlg : public QDialog
{
  Q_OBJECT
public:
  PasswordDlg (CICQDaemon *s, CSignalManager *,
               QWidget *parent = 0, const char *name = 0 );
  PasswordDlg::~PasswordDlg();
protected:
  CICQDaemon *server;
  CSignalManager *sigman;
  QPushButton *btnUpdate, *btnCancel;
  QLineEdit *edtFirst, *edtSecond;

  CICQEventTag *tag;

public slots:
  virtual void hide();
  void slot_doneUserFcn(ICQEvent *);
protected slots:
  void ok();
};


#endif
