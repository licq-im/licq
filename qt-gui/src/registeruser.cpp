#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "registeruser.h"
#include "licq-locale.h"
#include "ewidgets.h"
#include "icqd.h"

RegisterUserDlg::RegisterUserDlg(CICQDaemon *s, QWidget *parent = 0, const char *name = 0 )
  : QWidget(parent, name)
{
  server = s;
  grpInfo = new QGroupBox(this);
  nfoUin = new CInfoField(10, 15, 40, 5, 100, _("Uin:"), false, grpInfo);
  nfoPassword1 = new CInfoField(10, 40, 40, 5, 100, _("Password:"), false, grpInfo);
  nfoPassword2 = new CInfoField(10, 65, 40, 5, 100, _("Verify:"), false, grpInfo);
  nfoPassword1->setEchoMode(QLineEdit::Password);
  nfoPassword2->setEchoMode(QLineEdit::Password);
  chkExistingUser = new QCheckBox(_("&Register Existing User"), grpInfo);
  btnOk = new QPushButton("&Ok", this);
  btnCancel = new QPushButton(_("&Cancel"), this);
  connect (btnOk, SIGNAL(clicked()), SLOT(slot_ok()) );
  connect (btnCancel, SIGNAL(clicked()), SLOT(hide()) );
  connect (chkExistingUser, SIGNAL(toggled(bool)), nfoUin, SLOT(setEnabled(bool)));

  chkExistingUser->setChecked(false);
  nfoUin->setEnabled(false);
  resize(300, 200);
  setCaption(_("Licq User Registration"));
  show();
  char buf[256];
  sprintf (buf, _("If you are registering a new uin, choose a password and click \"OK\".\n"
                  "If you already have a uin, then toggle \"Register Existing User\",\n"
                  "enter your uin and your password, and click \"OK\""));
  InformUser(this, QString::fromLocal8Bit(buf));
}

void RegisterUserDlg::hide()
{
  QWidget::hide();
  emit signal_done();
  delete this;
}


void RegisterUserDlg::slot_ok()
{
  const char *szPassword = nfoPassword1->text();
  const char *szPassword2 = nfoPassword2->text();
  // Validate password
  if (szPassword == NULL || strlen(szPassword) > 8)
  {
    InformUser (this, _("Invalid password, must be 8 characters or less."));
    return;
  }
  if (szPassword2 == NULL || strcmp(szPassword, szPassword2) != 0)
  {
    InformUser (this, _("Passwords do not match, try again."));
    return;
  }

  if (chkExistingUser->isChecked())
  {
    unsigned long nUin = atol(nfoUin->text());
    // Validate uin
    if (nUin <= 0)
    {
      InformUser (this, _("Invalid UIN.  Try again."));
      return;
    }
    gUserManager.SetOwnerUin(nUin);
    ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
    o->SetPassword(szPassword);
    gUserManager.DropOwner();
    InformUser (this, _("Registered succesfully.  Now log on and update your personal info."));
    hide();
  }
  else
  {
    setCaption(_("User Registration in Progress..."));
    server->icqRegister(szPassword);
    btnOk->setEnabled(false);
    btnCancel->setEnabled(false);
    nfoUin->setEnabled(false);
    nfoPassword1->setEnabled(false);
    nfoPassword2->setEnabled(false);
    chkExistingUser->setEnabled(false);
  }
}



void RegisterUserDlg::resizeEvent(QResizeEvent *)
{
  grpInfo->setGeometry(10, 15, width() - 20, height() - 70);
  nfoUin->setGeometry(10, 15, 60, 5, width() - 105);
  nfoPassword1->setGeometry(10, 40, 60, 5, width() - 105);
  nfoPassword2->setGeometry(10, 65, 60, 5, width() - 105);
  chkExistingUser->setGeometry(10, 95, width() - 40, 20);
  btnOk->setGeometry(width() / 2 - 95, height() - 40, 80, 30);
  btnCancel->setGeometry(width() / 2 + 15, height() - 40, 80, 30);
}


#include "moc/moc_registeruser.h"
