#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qpushbutton.h>
#include <qlabel.h>
#include <qlayout.h>
#include "refusedlg.h"
#include "mledit.h"

#include "licq_user.h"

CRefuseDlg::CRefuseDlg(unsigned long _nUin, QString t, QWidget* parent)
   : LicqDialog(parent, "RefuseDialog", true)
{
  ICQUser *u = gUserManager.FetchUser(_nUin, LOCK_R);
  QLabel *lbl = new QLabel(tr("Refusal message for %1 with ").arg(t) + QString::fromLocal8Bit(u->GetAlias()) + ":", this);
  gUserManager.DropUser(u);

  mleRefuseMsg = new MLEditWrap(true, this);

  QPushButton *btnRefuse = new QPushButton(tr("Refuse"), this );
  connect( btnRefuse, SIGNAL(clicked()), SLOT(accept()) );
  QPushButton *btnCancel = new QPushButton(tr("Cancel"), this );
  connect( btnCancel, SIGNAL(clicked()), SLOT(reject()) );
  int bw = 75;
  bw = QMAX(bw, btnRefuse->sizeHint().width());
  bw = QMAX(bw, btnCancel->sizeHint().width());
  btnRefuse->setFixedWidth(bw);
  btnCancel->setFixedWidth(bw);

  QGridLayout *lay = new QGridLayout(this, 3, 5, 15, 10);
  lay->addMultiCellWidget(lbl, 0, 0, 0, 4);
  lay->addMultiCellWidget(mleRefuseMsg, 1, 1, 0, 4);
  lay->addWidget(btnRefuse, 2, 1);
  lay->addWidget(btnCancel, 2, 3);
  lay->setColStretch(0, 2);
  lay->setColStretch(4, 2);
  lay->addColSpacing(2, 10);

  setCaption(tr("Licq %1 Refusal").arg(t));
}


QString CRefuseDlg::RefuseMessage()
{
  return mleRefuseMsg->text();
}

