#ifndef EDITFILEDLG_H
#define EDITFILEDLG_H 

#include <qwidget.h>
#include <qlistbox.h>
#include <qpushbutton.h>

#include "licqdialog.h"
#include "licq_filetransfer.h"

class QPushButton;
class QListView;

class CEditFileListDlg : public LicqDialog
{
   Q_OBJECT
public:
  CEditFileListDlg(ConstFileList *_lFileList, QWidget *parent = 0);
  virtual ~CEditFileListDlg();

protected:
   QPushButton *btnDone, *btnUp, *btnDown, *btnDel;
   QListBox *lstFiles;
   ConstFileList *m_lFileList;
   
  void RefreshList();

protected slots:
  void slot_selectionChanged(QListBoxItem *item);
  void slot_done();
  void slot_up();
  void slot_down();
  void slot_del();

signals:
  void file_deleted(unsigned);
};

#endif
