#ifndef REQAUTHDLG_H
#define REQAUTHDLG_H

#include <qdialog.h>

class QPushButton;
class QLabel;
class QLineEdit;
class QGroupBox;

class CICQDaemon;
class MLEditWrap;

class ReqAuthDlg : public QDialog
{
   Q_OBJECT
public:
   ReqAuthDlg (CICQDaemon *s, QWidget *parent = 0);
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
