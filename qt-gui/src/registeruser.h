#ifndef REGISTERUSER_H
#define REGISTERUSER_H

#include <qwidget.h>

class QGroupBox;
class QCheckBox;

class CICQDaemon;
class CInfoField;

class RegisterUserDlg : public QWidget
{
  Q_OBJECT
public:
  RegisterUserDlg (CICQDaemon *s, QWidget *parent = 0, const char *name = 0 );
protected:
  CICQDaemon *server;
  QGroupBox *grpInfo;
  QPushButton *btnOk, *btnCancel;
  CInfoField *nfoUin, *nfoPassword1, *nfoPassword2;
  QCheckBox *chkExistingUser;
  virtual void resizeEvent(QResizeEvent *);
public slots:
  virtual void hide();
protected slots:
  void slot_ok();
signals:
  void signal_done();
};


#endif
