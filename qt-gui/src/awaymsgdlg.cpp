#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <qpushbt.h>
#include "awaymsgdlg.h"
#include "licq-locale.h"

int AwayMsgDlg::s_nX = 100;
int AwayMsgDlg::s_nY = 100;


AwayMsgDlg::AwayMsgDlg(QWidget *parent = 0, const char *name = 0) : QDialog(parent, name)
{
  resize(330, 140);
  mleAwayMsg = new MLEditWrap(true, this);
  mleAwayMsg->setGeometry(5, 5, 320, 70);
  connect(mleAwayMsg, SIGNAL(signal_CtrlEnterPressed()), this, SLOT(ok()));
  QPushButton *btnOk, *cancel;
  btnOk = new QPushButton(_("&Ok"), this );
  btnOk->setGeometry(75, 95, 80, 30 );
  connect( btnOk, SIGNAL(clicked()), SLOT(ok()) );
  cancel = new QPushButton(_("&Cancel"), this );
  cancel->setGeometry( 185, 95, 80, 30 );
  connect( cancel, SIGNAL(clicked()), SLOT(reject()) );
}


void AwayMsgDlg::show()
{
  char title[128];
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  sprintf(title, _("Set Auto Response for %s:"), o->getAlias());
  setCaption(title);
  if (o->getAwayMessage()[0] != '\0')
    mleAwayMsg->setText(o->getAwayMessage());
  else
  {
    char s[32];
    o->getStatusStr(s);
    sprintf(title, _("I am currently %s.\nYou can leave me a message."), s);
    mleAwayMsg->setText(title);
  }
  gUserManager.DropOwner();
  setGeometry(s_nX, s_nY, width(), height());
  mleAwayMsg->setFocus();
  mleAwayMsg->selectAll();
  QDialog::show();
}

void AwayMsgDlg::hide()
{
  s_nX = x();
  s_nY = y();
  QDialog::hide();
}


void AwayMsgDlg::ok()
{
  ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
  o->setAwayMessage((const char *)mleAwayMsg->text());
  gUserManager.DropOwner();
  accept();
}

#include "moc/moc_awaymsgdlg.h"
