#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qlayout.h>

#include <stdio.h>
#include "showawaymsgdlg.h"
#include "user.h"
#include "licq-locale.h"

ShowAwayMsgDlg::ShowAwayMsgDlg(unsigned long _nUin, QWidget *parent = 0, const char *name = 0)
  : QDialog(parent, name)
{
  m_nUin= _nUin;

  QBoxLayout* top_lay = new QVBoxLayout(this, 10);
  
#if QT_VERSION >= 210
  qleAwayMsg = new QMultiLineEdit(this);
  // ICQ99b allows 37 chars per line, so we do the same
  qleAwayMsg->setWordWrap(QMultiLineEdit::FixedColumnWrap);
  qleAwayMsg->setWrapColumnOrWidth(37);
  qleAwayMsg->setMinimumSize(280, 90);
#else  
  qleAwayMsg = new MLEditWrap(true, this);
#endif  
  qleAwayMsg->setReadOnly(true);
  top_lay->addWidget(qleAwayMsg);

  QBoxLayout* lay = new QHBoxLayout(top_lay, 10);
  
  chkShowAgain = new QCheckBox(_("&Show Again"), this);
  lay->addWidget(chkShowAgain);

  lay->addStretch(1);
  lay->addSpacing(30);

  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
  chkShowAgain->setChecked(u->ShowAwayMsg());

  char szStatus[32];
  u->getStatusStr(szStatus);
  setCaption(QString(_("%1 Response for %2")).arg(szStatus).arg(u->getAlias()));
  qleAwayMsg->setText(u->getAwayMessage());

  gUserManager.DropUser(u);

  btnOk = new QPushButton(_("&Ok"), this);
  btnOk->setDefault(true);
  connect(btnOk, SIGNAL(clicked()), SLOT(accept()));
  lay->addWidget(btnOk);

  show();
}

void ShowAwayMsgDlg::accept()
{
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_W);
  
  u->setShowAwayMsg(chkShowAgain->isChecked());
  gUserManager.DropUser(u);

  QDialog::accept();
}

#include "moc/moc_showawaymsgdlg.h"
