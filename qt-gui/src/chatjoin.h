#ifndef CHATJOINDLG_H
#define CHATJOINDLG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qdialog.h>

#include "chatdlg.h"

class QListBox;
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
