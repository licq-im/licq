#ifndef CHGPASSDLG_H
#define CHGPASSDLG_H

#include <qdialog.h>

class QPushButton;
class QLineEdit;
class QLabel;
class ICQEvent;

class ChangePassDlg : public QDialog
{
  Q_OBJECT
public:
  ChangePassDlg (QWidget* parent = 0);
protected:
  QPushButton *btnUpdate, *btnCancel;
  QLineEdit *edtUin, *edtFirst, *edtSecond;
  QLabel *lblUin, *lblPassword, *lblVerify;

protected slots:
  void accept();
};


#endif
