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
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  setCaption(_("Set Auto Response for %1:")
             .arg(QString::fromLocal8Bit(o->getAlias())));
  if (o->getAwayMessage()[0] != '\0')
    mleAwayMsg->setText(QString::fromLocal8Bit(o->getAwayMessage()));
  else
  {
    char s[32];
    o->getStatusStr(s);
    mleAwayMsg->setText(_("I am currently %1.\nYou can leave me a message.")
                        .arg(s));
  }
  gUserManager.DropOwner();
  move(s_nX, s_nY);
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
  o->setAwayMessage(mleAwayMsg->text().local8Bit());
  gUserManager.DropOwner();
  accept();
}

#include "moc/moc_awaymsgdlg.h"
