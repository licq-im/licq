#ifndef AWAYMSG_H
#define AWAYMSG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qwidget.h>
#include <qmlined.h>
#include <qcheckbox.h>
#include <qpushbt.h>

#include "user.h"
#include "mledit.h"

class ShowAwayMsgDlg : public QWidget
{
  Q_OBJECT
public:
  ShowAwayMsgDlg(unsigned long _nUin, QWidget *parent = 0, const char *name = 0);

protected:
  unsigned long m_nUin;
  MLEditWrap *mleAwayMsg;
  QCheckBox *chkShowAgain;
  QPushButton *btnOk;
  void resizeEvent (QResizeEvent *);

protected slots:
  virtual void hide(); 
  void ok();
};


#endif
