#ifndef EDITFILE_H
#define EDITFILE_H

#include <qwidget.h>
#include <qstring.h>

class QMultiLineEdit;
class QPushButton;

class EditFileDlg: public QWidget
{
  Q_OBJECT
public:
  EditFileDlg (QString fname, QWidget *parent = 0, const char *name = 0);
  virtual void hide();
protected:
  QString sFile;
  QMultiLineEdit *mleFile;
  QPushButton *btnSave, *btnClose;
protected slots:
  void slot_save();
};


#endif
