#ifndef USERSELECTDLG_H
#define USERSELECTDLG_H

#include <qwidget.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qstring.h>

#include "licqdialog.h"

class QPushButton;
class QCheckBox;
class QComboBox;

class CICQDaemon;

class UserSelectDlg : public LicqDialog
{
  Q_OBJECT
public:
  UserSelectDlg(CICQDaemon *s, QWidget *parent = 0);
  ~UserSelectDlg();

protected:
  CICQDaemon *server;
  QPushButton *btnOk, *btnCancel;
  QCheckBox *chkSavePassword;
  QComboBox *cmbUser;
  QLineEdit *edtPassword;
  QLabel *lblUser, *lblPassword;
  
protected slots:
  void slot_ok();
  void slot_cmbSelectUser(const QString &);
};

#endif
