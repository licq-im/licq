#ifndef REGISTERUSER_H
#define REGISTERUSER_H

#include <qwizard.h>

class QGroupBox;
class QCheckBox;
class QWidget;
class QHideEvent;

class CICQDaemon;
class CInfoField;

class RegisterUserDlg : public QWizard
{
  Q_OBJECT
public:
  RegisterUserDlg (CICQDaemon *s, QWidget *parent = 0 );

signals:
  void signal_done();

protected:
  CICQDaemon *server;
  QGroupBox *grpInfo;
  CInfoField *nfoUin, *nfoPassword1, *nfoPassword2;
  QCheckBox *chkExistingUser;
  QWidget* page1, *page2, *page3;

  void hideEvent(QHideEvent*)
  {
    emit signal_done();
    close(true);
  };

protected slots:
  void accept();
  void dataChanged();
};


#endif
