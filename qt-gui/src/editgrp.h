#ifndef EDITGRP_H
#define EDITGRP_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qpushbt.h>
#include <qlabel.h>
#include <qlined.h>
#include <qgroupbox.h>
#include <qlistbox.h>

#include "ewidgets.h"

class EditGrpDlg : public QWidget
{
  Q_OBJECT
public:
  EditGrpDlg (QWidget *parent = 0, const char *name = 0 );
protected:
  QListBox *lstGroups;
  QGroupBox *grpGroups;
  QPushButton *btnAdd, *btnRemove, *btnUp, *btnDown, *btnDone, *btnEdit, *btnDefault;

  QLineEdit *edtName;
  CInfoField *nfoDefault;

  unsigned short m_nEditGrp;

  void RefreshList();
  void resizeEvent (QResizeEvent *);
public slots:
  //virtual void show();
  virtual void hide();
protected slots:
  void slot_add();
  void slot_remove();
  void slot_up();
  void slot_down();
  void slot_edit();
  void slot_editok();
  void slot_editcancel();
  void slot_default();
signals:
  void signal_updateGroups();
};


#endif
