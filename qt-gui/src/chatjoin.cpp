#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qpushbutton.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qlabel.h>

#include "chatjoin.h"

//=====CJoinChatDlg========================================================

CJoinChatDlg::CJoinChatDlg(QWidget *p, const char *n)
  : QDialog(p, n, true)
{
  setCaption(tr("Join Multiparty Chat"));

  QGridLayout *lay = new QGridLayout(this, 3, 5, 10, 5);

  QLabel *l = new QLabel(tr("Select chat to join:"), this);
  lay->addMultiCellWidget(l, 0, 0, 0, 4);

  lstChats = new QListBox(this);
  lay->addMultiCellWidget(lstChats, 1, 1, 0, 4);

  lay->setColStretch(0, 2);
  btnOk = new QPushButton(tr("&Join"), this);
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
  connect(btnCancel, SIGNAL(clicked()), SLOT(reject()));

  // Fill in the combo box
  ChatDlgList::iterator iter;
  for (iter = ChatDlg::chatDlgs.begin();
       iter != ChatDlg::chatDlgs.end(); iter++)
  {
    QString n;
    if ((*iter)->chatUser == NULL)  // check if the first user closed already
    {
      n.setNum((*iter)->m_nUin);
    }
    else
    {
      n = (*iter)->chatUser->chatname;
      if (n.isEmpty()) n.setNum((*iter)->chatUser->uin);
    }
    QString c = (*iter)->ChatClients();
    if (!c.isEmpty())
    {
      n += " (" + c + ")";
    }
    lstChats->insertItem(n);

    originalChats.push_back(*iter);
  }

}


void CJoinChatDlg::slot_ok()
{
  if (lstChats->currentItem() == -1) return;
  accept();
}


ChatDlg *CJoinChatDlg::JoinedChat()
{
  if (lstChats->currentItem() == -1) return NULL;

  unsigned short n = 0;
  ChatDlgList::iterator iter;
  for (iter = originalChats.begin();
       iter != originalChats.end() && n < lstChats->currentItem();
       iter++, n++);

  ChatDlg *cd = *iter;

  for (iter = ChatDlg::chatDlgs.begin();
       iter != ChatDlg::chatDlgs.end() && *iter != cd;
       iter++);

  // Check that this chat still exists
  if (iter == ChatDlg::chatDlgs.end()) return NULL;

  return cd;
}


#include "chatjoin.moc"
