#ifndef UTILITYDLG_H
#define UTILITYDLG_H

#include <stdio.h>

#include <qwidget.h>

#include <vector.h>

class QCheckBox;
class QLineEdit;
class QLabel;
class QSocketNotifier;
class QScrollView;
class QGroupBox;

class CICQDaemon;
class CUtility;
class CInfoField;

class MLEditWrap;

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

  QLabel *lblUtility, *lblWinType, *lblDesc;
  CInfoField *nfoUtility, *nfoWinType, *nfoDesc;
  QCheckBox *chkEditFinal;
  QScrollView *scrFields;
  QGroupBox *boxFields;
  vector <QLineEdit *> edtFields;
  vector <QLabel *> lblFields;
  QPushButton *btnRun, *btnCancel;
  MLEditWrap *mleCommand;
  QSocketNotifier *snCommand;
  void resizeEvent (QResizeEvent *);
protected slots:
  void slot_run();
  void slot_cancel();
  void slot_command();
};

#endif
