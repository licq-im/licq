#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <qvalidator.h>
#include "authuserdlg.h"

#include "icqd.h"

AuthUserDlg::AuthUserDlg(CICQDaemon *s, QWidget *parent, const char *name)
  : QDialog(parent, name)
{
   server = s;
   setCaption(tr("Licq"));
   resize(340, 100);
   lblUin = new QLabel(tr("Authorize which user (UIN):"), this);
   lblUin->setGeometry(10, 15, 160, 20);
   edtUin = new QLineEdit(this);
   edtUin->setGeometry(180, 15, 120, 20);
   edtUin->setValidator(new QIntValidator(0, 2147483647, edtUin));
   btnOk = new QPushButton(tr("&Ok"), this);
   btnOk->setGeometry(80, 60, 80, 30);
   btnCancel = new QPushButton(tr("&Cancel"), this);
   btnCancel->setGeometry(180, 60, 80, 30);
   connect (btnOk, SIGNAL(clicked()), SLOT(ok()) );
   connect (edtUin, SIGNAL(returnPressed()), SLOT(ok()) );
   connect (btnCancel, SIGNAL(clicked()), SLOT(reject()) );
}


void AuthUserDlg::show()
{
  edtUin->setText("");
  edtUin->setFocus();
  QDialog::show();
}


void AuthUserDlg::hide()
{
   QDialog::hide();
   delete this;
}


void AuthUserDlg::ok()
{
  unsigned long nUin = atol((const char *)edtUin->text());
  if (nUin != 0) server->icqAuthorize(nUin);
  accept();
}

#include "moc/moc_authuserdlg.h"
