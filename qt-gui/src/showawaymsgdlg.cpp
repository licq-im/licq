#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include "showawaymsgdlg.h"
#include "user.h"
#include "licq-locale.h"

ShowAwayMsgDlg::ShowAwayMsgDlg(unsigned long _nUin, QWidget *parent = 0, const char *name = 0)
  : QWidget(parent, name)
{
  m_nUin= _nUin;
  mleAwayMsg = new MLEditWrap(true, this);
  mleAwayMsg->setReadOnly(true);
  connect(mleAwayMsg, SIGNAL(signal_CtrlEnterPressed()), this, SLOT(ok()));
  btnOk = new QPushButton(_("&Ok"), this);
  connect(btnOk, SIGNAL(clicked()), SLOT(ok()));
  chkShowAgain = new QCheckBox(_("&Show Again"), this);

  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
  chkShowAgain->setChecked(u->ShowAwayMsg());

  char title[128];
  char szStatus[32];
  u->getStatusStr(szStatus);
  sprintf(title, _("%s Message for %s"), szStatus, u->getAlias());
  setCaption(title);
  mleAwayMsg->setText(u->getAwayMessage());

  gUserManager.DropUser(u);
  resize(300, 160);
  show();
}


void ShowAwayMsgDlg::resizeEvent (QResizeEvent *)
{
  mleAwayMsg->setGeometry(5, 5, width() - 10, height() - 80);
  chkShowAgain->setGeometry(10, height() - 70, width() - 10, 20);
  btnOk->setGeometry((width() >> 1) - 50, height() - 40, 100, 30 );
}

void ShowAwayMsgDlg::hide()
{
  QWidget::hide();
  delete this;
}


void ShowAwayMsgDlg::ok()
{
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_W);
  u->setShowAwayMsg(chkShowAgain->isChecked());
  gUserManager.DropUser(u);
  hide();
}

#include "moc/moc_showawaymsgdlg.h"
