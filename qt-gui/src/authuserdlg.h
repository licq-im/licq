#ifndef AUTHUSERDLG_H
#define AUTHUSERDLG_H

#include <qdialog.h>

class QPushButton;
class QLabel;
class QGroupBox;
class QLineEdit;

class CICQDaemon;
class MLEditWrap;

class AuthUserDlg : public QDialog
{
   Q_OBJECT
public:
   AuthUserDlg (CICQDaemon *s, unsigned long UIN, QWidget *parent = 0, const char *name = 0 );
protected:
   CICQDaemon *server;
   QPushButton *btnOk, *btnCancel;
   QLabel *lblUin;
   QGroupBox *grpResponse;
   QLineEdit *edtUin;
   MLEditWrap* mleResponse;
protected slots:
   void ok();
};


#endif
