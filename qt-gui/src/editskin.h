#ifndef EDITSKIN_H
#define EDITSKIN_H

#include <qwidget.h>
#include <qstring.h>

class QMultiLineEdit;
class QPushButton;

class EditSkinDlg: public QWidget
{
  Q_OBJECT
public:
  EditSkinDlg (QString skin, QWidget *parent = 0, const char *name = 0);
  virtual void hide();
protected:
  QString sSkin, sSkinConf;
  QMultiLineEdit *mleSkin;
  QPushButton *btnSave, *btnClose;
protected slots:
  void slot_save();
};


#endif
