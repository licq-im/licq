#ifndef REGISTERUSER_H
#define REGISTERUSER_H

#include <qwizard.h>

class QGroupBox;
class QCheckBox;
class QWidget;

class CICQDaemon;
class CInfoField;

class RegisterUserDlg : public QWizard
{
  Q_OBJECT
public:
  RegisterUserDlg (CICQDaemon *s, QWidget *parent = 0, const char *name = 0 );
protected:
  CICQDaemon *server;
  QGroupBox *grpInfo;
  CInfoField *nfoUin, *nfoPassword1, *nfoPassword2;
  QCheckBox *chkExistingUser;
  QWidget* page1, *page2, *page3;

protected slots:
  void accept();
  void dataChanged();
signals:
  void signal_done();
};


#endif
