#ifndef UTILITYDLG_H
#define UTILITYDLG_H

#include <stdio.h>

#include <qdialog.h>

#include <vector.h>

class QCheckBox;
class QLineEdit;
class QLabel;
class QSocketNotifier;
class QGroupBox;

class CICQDaemon;
class CUtility;
class CInfoField;

class MLEditWrap;

class CUtilityDlg : public QDialog
{
  Q_OBJECT
public:
  CUtilityDlg(CUtility *u, unsigned long _nUin, CICQDaemon *_server);
  ~CUtilityDlg();
protected:
  CUtility *m_xUtility;
  CICQDaemon *server;
  FILE *fsCommand;
  unsigned long m_nUin;
  bool m_bIntWin;

  QLabel *lblUtility;
  CInfoField *nfoUtility, *nfoWinType, *nfoDesc;
  QCheckBox *chkEditFinal;
  QGroupBox *boxFields;
  vector <QLabel *> lblFields;
  vector <QLineEdit *> edtFields;
  QPushButton *btnRun, *btnCancel;
  MLEditWrap *mleCommand;
  QSocketNotifier *snCommand;
protected slots:
  void slot_run();
  void slot_cancel();
  void slot_command();
};

#endif
