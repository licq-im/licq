#ifndef OUTPUTWIN_H
#define OUTPUTWIN_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qdialog.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qsocketnotifier.h>
#include <qmessagebox.h>

#include "licq-locale.h"
#include "log.h"

//=====OutputWin===================================================================================
class CQtLogWindow : public QDialog, public CPluginLog
{
  Q_OBJECT
public:
  CQtLogWindow (QWidget *parent = 0, const char *name = 0);
protected:
  QMultiLineEdit *outputBox;
  QPushButton *btnHide, *btnClear;
  QSocketNotifier *sn;
protected slots:
  void slot_log(int);

  virtual void showEvent(QShowEvent*);
};

class CLicqMessageBox : public QMessageBox
{
public:
  CLicqMessageBox(const char *_szText, Icon icon, QWidget *q = NULL)
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
