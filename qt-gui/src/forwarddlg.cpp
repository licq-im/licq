#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qpushbutton.h>
#include <qlayout.h>
#include <qdragobject.h>

#include "forwarddlg.h"
#include "mainwin.h"
#include "gui-defines.h"
#include "ewidgets.h"
#include "licq_user.h"
#include "icqfunctions.h"
#include "ewidgets.h"

CForwardDlg::CForwardDlg(CMainWindow *_mainwin, CUserEvent *e,
                         QWidget *p, const char *n)
  : QWidget(p, n)
{
  mainwin = _mainwin;
  m_nEventType = e->SubCommand();
  m_nUin = 0;

  QString t;
  switch (e->SubCommand())
  {
    case ICQ_CMDxSUB_MSG:
      t = tr("Message");
      s1 = QString::fromLocal8Bit(((CEventMsg *)e)->Message());
      break;
    case ICQ_CMDxSUB_URL:
      t = tr("URL");
      s1 = QString::fromLocal8Bit(((CEventUrl *)e)->Url());
      s2 = QString::fromLocal8Bit(((CEventUrl *)e)->Description());
      break;
    default:
      WarnUser(this, tr("Unable to forward this message type (%d).").arg(e->SubCommand()));
      return;
  }

  setCaption(tr("Forward %1 To User").arg(t));
  setAcceptDrops(true);

  QGridLayout *lay = new QGridLayout(this, 3, 5, 10, 5);
  QLabel *lbl = new QLabel(tr("Drag the user to forward to here:"), this);
  lay->addMultiCellWidget(lbl, 0, 0, 0, 4);
  edtUser = new CInfoField(this, true);
  edtUser->setAcceptDrops(false);
  lay->addMultiCellWidget(edtUser, 1, 1, 0, 4);

  lay->setColStretch(0, 2);
  btnOk = new QPushButton(tr("&Forward"), this);
  lay->addWidget(btnOk, 2, 1);

  lay->addColSpacing(2, 10);
  btnCancel = new QPushButton(tr("&Cancel"), this);
  lay->addWidget(btnCancel, 2, 3);
  lay->setColStretch(4, 2);

  int bw = 75;
  bw = QMAX(bw, btnOk->sizeHint().width());
  bw = QMAX(bw, btnCancel->sizeHint().width());
  btnOk->setFixedWidth(bw);
  btnCancel->setFixedWidth(bw);

  connect(btnOk, SIGNAL(clicked()), SLOT(slot_ok()));
  connect(btnCancel, SIGNAL(clicked()), SLOT(hide()));
}


CForwardDlg::~CForwardDlg()
{
}


void CForwardDlg::hide()
{
  QWidget::hide();
  delete this;
}

void CForwardDlg::slot_ok()
{
  if (m_nUin == 0) return;

  switch(m_nEventType)
  {
    case ICQ_CMDxSUB_MSG:
    {
      ICQFunctions *f = mainwin->callFunction(mnuUserSendMsg, m_nUin);
      if (f != NULL) f->SendMsg(s1);
      break;
    }
    case ICQ_CMDxSUB_URL:
    {
      ICQFunctions *f = mainwin->callFunction(mnuUserSendMsg, m_nUin);
      if (f != NULL) f->SendUrl(s1, s2);
      break;
    }
  }
  hide();
}



void CForwardDlg::dragEnterEvent(QDragEnterEvent * dee)
{
  dee->accept(QTextDrag::canDecode(dee));
}


void CForwardDlg::dropEvent(QDropEvent * de)
{
  QString text;

  // extract the text from the event
  if (QTextDrag::decode(de, text) == FALSE)
  {
    WarnUser(this, "Drag'n'Drop didn't work");
    return;
  }

  m_nUin = text.toULong();
  if (m_nUin == 0) return;

  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
  edtUser->setText(QString::fromLocal8Bit(u->GetAlias()) + " (" + text + ")");
  gUserManager.DropUser(u);
}


#include "forwarddlg.moc"
