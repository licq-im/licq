#ifndef EDITFILE_H
#define EDITFILE_H

#include <qwidget.h>
#include <qstring.h>

class MLEditWrap;
class QPushButton;

class EditFileDlg: public QWidget
{
  Q_OBJECT
public:
  EditFileDlg (QString fname, QWidget *parent = 0, const char *name = 0);
  virtual void hide();
protected:
  QString sFile;
  MLEditWrap *mleFile;
  QPushButton *btnSave, *btnClose;
protected slots:
  void slot_save();
};


#endif
