#ifndef OUTPUTWIN_H
#define OUTPUTWIN_H

#include <qwidget.h>
#include <qpushbutton.h>
#include <qsocketnotifier.h>

#include "mledit.h"
#include "licq_log.h"
#include "licqdialog.h"

class CLogWidget;

//=====OutputWin============================================================
class CQtLogWindow : public LicqDialog, public CPluginLog
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

/* ----------------------------------------------------------------------------- */

class CLogWidget : public MLEditWrap
{
public:
  CLogWidget(QWidget* parent = 0, const char* name = 0);
  virtual ~CLogWidget() {};

protected:
  virtual void paintCell(QPainter* p, int row, int col);
};

#endif
