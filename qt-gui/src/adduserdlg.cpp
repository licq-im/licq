#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <qvalidator.h>
#include "adduserdlg.h"
#include "licq-locale.h"

#include "icqd.h"

AddUserDlg::AddUserDlg(CICQDaemon *s, QWidget *parent = 0, const char *name = 0 ) : QDialog(parent, name)
{
   server = s;
   resize(240, 120);
   lblUin = new QLabel(_("New User UIN:"), this);
   lblUin->setGeometry(10, 15, 80, 20);
   edtUin = new QLineEdit(this);
   edtUin->setGeometry(100, 15, 120, 20);
   edtUin->setValidator(new QIntValidator(0, 2147483647, edtUin));
   chkAlert = new QCheckBox(_("&Alert User"), this);
   chkAlert->setGeometry(10, 50, 180, 20);
   btnOk = new QPushButton("&Ok", this);
   btnOk->setGeometry(30, 80, 80, 30);
   btnCancel = new QPushButton(_("&Cancel"), this);
   btnCancel->setGeometry(130, 80, 80, 30);
   connect (btnOk, SIGNAL(clicked()), SLOT(ok()) );
   connect (edtUin, SIGNAL(returnPressed()), SLOT(ok()) );
   connect (btnCancel, SIGNAL(clicked()), SLOT(reject()) );
}


void AddUserDlg::show()
{
   edtUin->setText("");
   edtUin->setFocus();
   chkAlert->setChecked(false);
   QDialog::show();
}

void AddUserDlg::hide()
{
   QDialog::hide();
   delete this;
}


void AddUserDlg::ok()
{
   unsigned long nUin = atol((const char *)edtUin->text());
   if (nUin != 0)
   {
     server->AddUserToList(nUin);
     if (chkAlert->isChecked()) // alert the user they were added
       server->icqAlertUser(nUin);
   }
   accept();
}

#include "moc/moc_adduserdlg.h"
