#ifndef AUTHUSER_H
#define AUTHUSER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qdialog.h>
#include <qpushbt.h>
#include <qlabel.h>
#include <qlined.h>

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
