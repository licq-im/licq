#ifndef UTIL_H
#define UTIL_H

#include <qscrollview.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qmultilinedit.h>
#include <qsocketnotifier.h>

#include <vector.h>

#include "ewidgets.h"
#include "utility.h"

class CICQDaemon;

class CUtilityDlg : public QWidget
{
  Q_OBJECT
public:
  CUtilityDlg(CUtility *u, unsigned long _nUin, CICQDaemon *_server);
  ~CUtilityDlg(void);
  virtual void hide();
protected:
  CUtility *m_xUtility;
  CICQDaemon *server;
  FILE *fsCommand;
  unsigned long m_nUin;
  bool m_bIntWin;

  CInfoField *nfoUtility, *nfoWinType, *nfoDesc;
  QScrollView *scrFields;
  QGroupBox *boxFields;
  vector <QLineEdit *> edtFields;
  vector <QLabel *> lblFields;
  QPushButton *btnRun, *btnCancel;
  QMultiLineEdit *mleCommand;
  QSocketNotifier *snCommand;
  void resizeEvent (QResizeEvent *);
protected slots:
  void slot_run();
  void slot_cancel();
  void slot_command();
};

#endif
