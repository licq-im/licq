#ifndef AWAYMSG_H
#define AWAYMSG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qdialog.h>
#include <qmlined.h>

#include "user.h"
#include "mledit.h"

class AwayMsgDlg : public QDialog
{
  Q_OBJECT
public:
  AwayMsgDlg(QWidget *parent = 0, const char *name = 0);
  virtual void show();
  virtual void hide();
protected:
  MLEditWrap *mleAwayMsg;

  static int s_nX, s_nY;
protected slots:
  void ok();
};


#endif
