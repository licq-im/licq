#ifndef AUTHUSERDLG_H
#define AUTHUSERDLG_H

#include <qdialog.h>

class QPushButton;
class QLabel;
class QLineEdit;

class CICQDaemon;

class AuthUserDlg : public QDialog
{
   Q_OBJECT
public:
   AuthUserDlg (CICQDaemon *s, QWidget *parent = 0, const char *name = 0 );
protected:
   CICQDaemon *server;
   QPushButton *btnOk, *btnCancel;
   QLabel *lblUin;
   QLineEdit *edtUin;
public slots:
   virtual void show();
   virtual void hide();
protected slots:
   void ok();
};


#endif
