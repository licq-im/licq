#ifndef EDITGRP_H
#define EDITGRP_H

#include <qwidget.h>

class QListBox;
class QLineEdit;
class QGroupBox;
class QPushButton;

class CInfoField;

class EditGrpDlg : public QWidget
{
  Q_OBJECT
public:
  EditGrpDlg (QWidget *parent = 0);
protected:
  QListBox *lstGroups;
  QGroupBox *grpGroups;
  QPushButton *btnAdd, *btnRemove, *btnUp, *btnDown, *btnDone, *btnEdit,
              *btnDefault, *btnNewUser;

  QLineEdit *edtName;
  CInfoField *nfoDefault, *nfoNewUser;

  unsigned short m_nEditGrp;

  void RefreshList();

protected slots:
  void slot_add();
  void slot_remove();
  void slot_up();
  void slot_down();
  void slot_edit();
  void slot_editok();
  void slot_editcancel();
  void slot_default();
  void slot_newuser();
  void slot_done();
signals:
  void signal_updateGroups();
};


#endif
