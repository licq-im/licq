#ifndef ADDUSER_H
#define ADDUSER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qdialog.h>
#include <qpushbt.h>
#include <qlabel.h>
#include <qlined.h>
#include <qchkbox.h>

#include "icq.h"

class AddUserDlg : public QDialog
{
   Q_OBJECT
public:
   AddUserDlg (CICQDaemon *s, QWidget *parent = 0, const char *name = 0 );
protected:
   CICQDaemon *server;
   QPushButton *btnOk, *btnCancel;
   QLabel *lblUin;
   QLineEdit *edtUin;
   QCheckBox *chkAlert;
public slots:
   virtual void show();
   virtual void hide();
protected slots:
   void ok();
signals:
   void signal_updatedUsers();
};


#endif
