#ifndef CHATJOINDLG_H
#define CHATJOINDLG_H

#include <qdialog.h>
#include <qlistbox.h>

#include "chatdlg.h"

class QPushButton;

class CJoinChatDlg : public QDialog
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
