#ifndef REQAUTHDLG_H
#define REQAUTHDLG_H

#include "licqdialog.h"

class QPushButton;
class QLabel;
class QLineEdit;
class QGroupBox;

class CICQDaemon;
class MLEditWrap;

class ReqAuthDlg : public LicqDialog
{
   Q_OBJECT
public:
   ReqAuthDlg (CICQDaemon *s, unsigned long nUin = 0, QWidget *parent = 0);
   ReqAuthDlg(CICQDaemon *s, const char *szId, unsigned long nPPID,
    QWidget *parent = 0);

protected:
   CICQDaemon *server;
   QPushButton *btnOk, *btnCancel;
   QLabel *lblUin;
   QLineEdit *edtUin;
   QGroupBox *grpRequest;
   MLEditWrap *mleRequest;
protected slots:
   void ok();
};


#endif
