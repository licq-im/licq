#ifndef AWAYMSG_H
#define AWAYMSG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qdialog.h>

#include "user.h"
#include "mledit.h"

class QPushButton;
class QStringList;

class AwayMsgDlg : public QDialog
{
  Q_OBJECT
public:
  AwayMsgDlg(QStringList& _respHeader, QStringList& _respText, QWidget *parent = 0, const char *name = 0);

  void selectAutoResponse(unsigned short status);
  
  virtual void show();
  virtual void hide();
protected:
  MLEditWrap *mleAwayMsg;

  static int s_nX, s_nY;
protected slots:
  void ok();
  void selectMessage();
  
private:
  QPushButton* btnSelect;
  QStringList& responseHeader;
  QStringList& responseText;
  unsigned short status;
};


#endif
