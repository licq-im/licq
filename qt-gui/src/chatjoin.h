#ifndef CHATJOINDLG_H
#define CHATJOINDLG_H

#include <qlistbox.h>

#include "licqdialog.h"
#include "chatdlg.h"

class QPushButton;

class CJoinChatDlg : public LicqDialog
{
  Q_OBJECT

public:
  CJoinChatDlg(bool bRequesting, QWidget *p = 0);
  ~CJoinChatDlg() {}
  ChatDlg *JoinedChat();
  QString ChatClients() { return lstChats->text(lstChats->currentItem()); }
protected:
  QListBox *lstChats;
  QPushButton *btnOk, *btnCancel;
  ChatDlgList originalChats;
protected slots:
  void slot_ok();
};



#endif
