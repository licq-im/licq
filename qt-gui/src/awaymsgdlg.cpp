#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <qarray.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlayout.h>

#include "awaymsgdlg.h"
#include "sar.h"
#include "log.h"

int AwayMsgDlg::s_nX = 100;
int AwayMsgDlg::s_nY = 100;

AwayMsgDlg::AwayMsgDlg(QWidget *parent, const char *name)
    : QDialog(parent, name)
{
  QBoxLayout* top_lay = new QVBoxLayout(this, 10);

  mleAwayMsg = new MLEditWrap(true, this);
  // ICQ99b allows 37 chars per line, so we do the same
  mleAwayMsg->setWordWrap(QMultiLineEditNew::FixedColumnWrap);
  mleAwayMsg->setWrapColumnOrWidth(37);
  connect(mleAwayMsg, SIGNAL(signal_CtrlEnterPressed()), this, SLOT(ok()));
  top_lay->addWidget(mleAwayMsg);

  QBoxLayout* l = new QHBoxLayout(top_lay, 10);

  /*btnSelect = new QPushButton(tr("&Select"), this);
  // this doesn't work yet (Qt bug)
  //btnSelect->setIsMenuButton(true);
  connect(btnSelect, SIGNAL(clicked()), SLOT(selectMessage()));
  l->addWidget(btnSelect);*/
  cmbSAR = new QComboBox(this);
  connect(cmbSAR, SIGNAL(activated(int)), this, SLOT(slot_SARSelected(int)));
  l->addWidget(cmbSAR);

  l->addStretch(1);
  l->addSpacing(30);

  QPushButton *btnOk, *cancel;
  btnOk = new QPushButton(tr("&Ok"), this );
  btnOk->setDefault(true);
  connect( btnOk, SIGNAL(clicked()), SLOT(ok()) );
  l->addWidget(btnOk);

  cancel = new QPushButton(tr("&Cancel"), this );
  connect( cancel, SIGNAL(clicked()), SLOT(reject()) );
  l->addWidget(cancel);
}

void AwayMsgDlg::SelectAutoResponse(unsigned short _status)
{
  char s[32];
  m_nStatus = _status;
  ICQUser::StatusStr(m_nStatus, false, s);

  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  setCaption(QString(tr("Set %1 Response for %2"))
             .arg(s).arg(QString::fromLocal8Bit(o->getAlias())));
  if (*o->AutoResponse())
    mleAwayMsg->setText(QString::fromLocal8Bit(o->AutoResponse()));
  else
    mleAwayMsg->setText(tr("I am currently %1.\nYou can leave me a message.")
                        .arg(s));
  gUserManager.DropOwner();

  // Fill in the combo box
  cmbSAR->clear();
  switch (m_nStatus)
  {
  case ICQ_STATUS_AWAY: m_nSAR = SAR_AWAY; break;
  case ICQ_STATUS_NA: m_nSAR = SAR_NA; break;
  case ICQ_STATUS_OCCUPIED: m_nSAR = SAR_OCCUPIED; break;
  case ICQ_STATUS_DND: m_nSAR = SAR_DND; break;
  case ICQ_STATUS_FREEFORCHAT: m_nSAR = SAR_FFC; break;
  default: m_nSAR = -1;
  }
  if (m_nSAR >= 0)
  {
    SARList &sar = gSARManager.Fetch(m_nSAR);
    for (SARListIter i = sar.begin(); i != sar.end(); i++)
      cmbSAR->insertItem((*i)->Name());
    gSARManager.Drop();
  }

  move(s_nX, s_nY);
  mleAwayMsg->setFocus();
  mleAwayMsg->selectAll();

  QDialog::show();
}


void AwayMsgDlg::slot_SARSelected(int n)
{
  if (m_nSAR < 0) return;
  SARList &sar = gSARManager.Fetch(m_nSAR);
  mleAwayMsg->setText(sar[n]->AutoResponse());
  gSARManager.Drop();
}


void AwayMsgDlg::show()
{
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  unsigned short m_nStatus = o->getStatus();
  gUserManager.DropOwner();

  SelectAutoResponse(m_nStatus);
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
  o->SetAutoResponse(mleAwayMsg->text().local8Bit());
  gUserManager.DropOwner();
  accept();
}

/*void AwayMsgDlg::selectMessage()
{
  QPopupMenu* menu = new QPopupMenu(this);

  int result = 0;
  int offset = 0;

  // Select auto response group
  switch(status) {
    case ICQ_STATUS_FREEFORCHAT: offset = 32; break;
    case ICQ_STATUS_OCCUPIED:    offset = 16; break;
    case ICQ_STATUS_DND:         offset = 24; break;
    case ICQ_STATUS_NA:          offset =  8; break;
    case ICQ_STATUS_AWAY:
    default:                     offset =  0; break;
  }

  for(int i = 0; i<8; i++)
      menu->insertItem(responseHeader[i+offset], i);

  menu->insertSeparator();
  // as this is not yet implemented, give user feedback
  menu->setItemEnabled(menu->insertItem(tr("&Edit Items"), -2), false);

  result = menu->exec(btnSelect->mapToGlobal(QPoint(0,btnSelect->height())));

  if(result == -2) {
    // todo: open options menu
  }
  else {
    // (unsigned) -1 > 8 !
    if ((unsigned) result < 8)
      mleAwayMsg->setText(responseText[result+offset]);
  }

  delete menu;
}
*/

#include "moc/moc_awaymsgdlg.h"
