#ifndef REGISTERUSER_H
#define REGISTERUSER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef USE_KDE
#include "kwizard.h"
#define QWizard KWizard
#else
#include <qwizard.h>
#endif

class QGroupBox;
class QCheckBox;
class QWidget;
class QHideEvent;
class QLabel;

class CICQDaemon;
class CInfoField;

class RegisterUserDlg : public QWizard
{
  Q_OBJECT
public:
  RegisterUserDlg (CICQDaemon *s, QWidget *parent = 0 );
  ~RegisterUserDlg();

signals:
  void signal_done();

protected:
  CICQDaemon *server;
  QGroupBox *grpInfo;
  CInfoField *nfoUin, *nfoPassword1, *nfoPassword2;
  QCheckBox *chkExistingUser;
  QWidget* page1, *page2, *page3;
  QLabel* lblInfo, *lblInfo2;

protected slots:
  void accept();
  void dataChanged();
  void nextPage();
};


#endif
