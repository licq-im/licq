#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qpushbutton.h>
#include <qlayout.h>
#include <qlistbox.h>

#include "randomchatdlg.h"
#include "sigman.h"
#include "mainwin.h"
#include "icqd.h"
#include "icqevent.h"
#include "gui-defines.h"
#include "ewidgets.h"

CRandomChatDlg::CRandomChatDlg(CMainWindow *_mainwin, CICQDaemon *s,
                               CSignalManager *_sigman,
                               QWidget *p, const char *n)
  : QWidget(p, n)
{
  mainwin = _mainwin;
  server = s;
  sigman = _sigman;
  tag = NULL;

  setCaption(tr("Random Chat Search"));

  QGridLayout *lay = new QGridLayout(this, 2, 5, 10, 5);
  lstGroups = new QListBox(this);
  lay->addMultiCellWidget(lstGroups, 0, 0, 0, 4);

  lay->setColStretch(0, 2);
  btnOk = new QPushButton(tr("&Ok"), this);
  lay->addWidget(btnOk, 1, 1);

  lay->addColSpacing(2, 10);
  btnCancel = new QPushButton(tr("&Cancel"), this);
  lay->addWidget(btnCancel, 1, 3);
  lay->setColStretch(4, 2);

  int bw = 75;
  bw = QMAX(bw, btnOk->sizeHint().width());
  bw = QMAX(bw, btnCancel->sizeHint().width());
  btnOk->setFixedWidth(bw);
  btnCancel->setFixedWidth(bw);

  connect(btnOk, SIGNAL(clicked()), SLOT(slot_ok()));
  connect(btnCancel, SIGNAL(clicked()), SLOT(hide()));

  // Fill in the combo box
  lstGroups->insertItem(tr("General"));
  lstGroups->insertItem(tr("Romance"));
  lstGroups->insertItem(tr("Games"));
  lstGroups->insertItem(tr("Students"));
  lstGroups->insertItem(tr("20 Something"));
  lstGroups->insertItem(tr("30 Something"));
  lstGroups->insertItem(tr("40 Something"));
  lstGroups->insertItem(tr("50 Plus"));
  lstGroups->insertItem(tr("Men Seeking Women"));
  lstGroups->insertItem(tr("Women Seeking Men"));

  show();
}


CRandomChatDlg::~CRandomChatDlg()
{
  if (tag != NULL)
    server->CancelEvent(tag);
  delete tag;
}



void CRandomChatDlg::hide()
{
  QWidget::hide();
  delete this;
}


void CRandomChatDlg::slot_ok()
{
  if (lstGroups->currentItem() == -1) return;

  btnOk->setEnabled(false);
  QObject::connect(sigman, SIGNAL(signal_doneUserFcn(ICQEvent *)),
                   this, SLOT(slot_doneUserFcn(ICQEvent *)));
  unsigned long nGroup = ICQ_RANDOMxCHATxGROUP_NONE;
  switch(lstGroups->currentItem())
  {
    case 0: nGroup = ICQ_RANDOMxCHATxGROUP_GENERAL; break;
    case 1: nGroup = ICQ_RANDOMxCHATxGROUP_ROMANCE; break;
    case 2: nGroup = ICQ_RANDOMxCHATxGROUP_GAMES; break;
    case 3: nGroup = ICQ_RANDOMxCHATxGROUP_STUDENTS; break;
    case 4: nGroup = ICQ_RANDOMxCHATxGROUP_20SOME; break;
    case 5: nGroup = ICQ_RANDOMxCHATxGROUP_30SOME; break;
    case 6: nGroup = ICQ_RANDOMxCHATxGROUP_40SOME; break;
    case 7: nGroup = ICQ_RANDOMxCHATxGROUP_50PLUS; break;
    case 8: nGroup = ICQ_RANDOMxCHATxGROUP_MxSEEKxF; break;
    case 9: nGroup = ICQ_RANDOMxCHATxGROUP_FxSEEKxM; break;
  }
  tag = server->icqRandomChatSearch(nGroup);
  setCaption(tr("Searching for Random Chat Partner..."));
}


void CRandomChatDlg::slot_doneUserFcn(ICQEvent *e)
{
  if (!tag->Equals(e)) return;

  btnOk->setEnabled(true);
  if (tag != NULL)
  {
    delete tag;
    tag = NULL;
  }

  switch (e->m_eResult)
  {
  case EVENT_FAILED:
    WarnUser(this, tr("No random chat user found in that group."));
    break;
  case EVENT_TIMEDOUT:
    WarnUser(this, tr("Random chat search timed out."));
    break;
  case EVENT_ERROR:
    WarnUser(this, tr("Random chat search had an error."));
    break;
  default:
    mainwin->callFunction(mnuUserSendChat, e->m_sSearchAck->nUin);
    hide();
    return;
  }

}

#include "randomchatdlg.moc"
