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
  CJoinChatDlg(QWidget *p = NULL, const char *n = NULL);
  ~CJoinChatDlg() {}
  ChatDlg *JoinedChat();
protected:
  QListBox *lstChats;
  QPushButton *btnOk, *btnCancel;
  ChatDlgList originalChats;
protected slots:
  void slot_ok();
};



#endif
