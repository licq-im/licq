#ifndef UTILITYDLG_H
#define UTILITYDLG_H

#include <stdio.h>

#include <qwidget.h>

#include <vector>

class QCheckBox;
class QLineEdit;
class QLabel;
class QSocketNotifier;
class QGroupBox;
class QSplitter;

class CICQDaemon;
class CUtility;
class CInfoField;
class CUtilityInternalWindow;

class MLEditWrap;

class CUtilityDlg : public QWidget
{
  Q_OBJECT
public:
  CUtilityDlg(CUtility *u, unsigned long _nUin, CICQDaemon *_server);
#ifdef QT_PROTOCOL_PLUGIN
  CUtilityDlg(CUtility *u, const char *szId, unsigned long nPPID,
    CICQDaemon *server);
#endif
  ~CUtilityDlg();
protected:
  CUtility *m_xUtility;
  CICQDaemon *server;
  unsigned long m_nUin;
#ifdef QT_PROTOCOL_PLUGIN
  char *m_szId;
  unsigned long m_nPPID;
#endif
  bool m_bIntWin, m_bStdOutClosed, m_bStdErrClosed;
  CUtilityInternalWindow *intwin;

  QLabel *lblUtility;
  CInfoField *nfoUtility, *nfoWinType, *nfoDesc;
  QCheckBox *chkEditFinal;
  QGroupBox *boxFields;
  std::vector <QLabel *> lblFields;
  std::vector <QLineEdit *> edtFields;
  QPushButton *btnRun, *btnCancel;
  MLEditWrap *mleOut, *mleErr;
  QSocketNotifier *snOut, *snErr;
  QSplitter *splOutput;

  void CloseInternalWindow();
protected slots:
  void slot_run();
  void slot_cancel();
  void slot_stdout();
  void slot_stderr();
};

#endif
