#ifndef ADDUSERDLG_H
#define ADDUSERDLG_H

#include "licqdialog.h"

class QPushButton;
class QLabel;
class QLineEdit;
class QCheckBox;
class QComboBox;

class CICQDaemon;

class AddUserDlg : public LicqDialog
{
   Q_OBJECT
public:
   AddUserDlg (CICQDaemon *s, const char* szId = 0, unsigned long PPID = 0,
               QWidget *parent = 0);
protected:
   CICQDaemon *server;
   QPushButton *btnOk, *btnCancel;
   QLabel *lblUin, *lblProtocol;
   QLineEdit *edtUin;
   QCheckBox *chkAlert;
   QComboBox *cmbProtocol;
public slots:
   virtual void show();
protected slots:
   void ok();
};


#endif
