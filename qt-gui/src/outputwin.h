#ifndef OUTPUTWIN_H
#define OUTPUTWIN_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qdialog.h>
#include <qpushbutton.h>
#include <qsocketnotifier.h>

#include "mledit.h"
#include "ewidgets.h"
#include "licq_log.h"

//=====OutputWin============================================================
class CQtLogWindow : public QDialog, public CPluginLog
{
  Q_OBJECT
public:
  CQtLogWindow (QWidget *parent = 0);
protected:
  CLogWidget *outputBox;
  QPushButton *btnHide, *btnClear, *btnSave;
  QSocketNotifier *sn;
protected slots:
  void slot_log(int);
  void slot_save();

  virtual void showEvent(QShowEvent*);
};

#endif
