#ifndef ADDUSERDLG_H
#define ADDUSERDLG_H

#include <qdialog.h>

class QPushButton;
class QLabel;
class QLineEdit;
class QCheckBox;

class CICQDaemon;

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
};


#endif
