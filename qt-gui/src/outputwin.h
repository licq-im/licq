#ifndef OUTPUTWIN_H
#define OUTPUTWIN_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qdialog.h>
#include <qpushbutton.h>
#include <qsocketnotifier.h>
#include <qmessagebox.h>

#include "mledit.h"
#include "ewidgets.h"
#include "log.h"

//=====OutputWin===================================================================================
class CQtLogWindow : public QDialog, public CPluginLog
{
  Q_OBJECT
public:
  CQtLogWindow (QWidget *parent = 0, const char *name = 0);
protected:
  //MLEditWrap *outputBox;
  CLogWidget *outputBox;
  QPushButton *btnHide, *btnClear, *btnSave;
  QSocketNotifier *sn;
protected slots:
  void slot_log(int);
  void slot_save();

  virtual void showEvent(QShowEvent*);
};

class CLicqMessageBox : public QMessageBox
{
public:
  CLicqMessageBox(QString _szText, Icon icon, QWidget *q = NULL)
   : QMessageBox ("Licq", _szText, icon, Ok | Default, 0, 0, q, NULL, q == NULL ? false : true)
  { show(); }

protected:
  virtual void hide(void)
  {
    QMessageBox::hide();
    delete this;
  }
};


#endif
