#ifndef __OWNERMANAGERDLG_H
#define __OWNERMANAGERDLG_H

#include "licqdialog.h"
#include <qlistview.h>

class QPushButton;
class QLabel;
class QLineEdit;
class QComboBox;

class CICQDaemon;

class OwnerEditDlg : public LicqDialog
{
  Q_OBJECT
public:
  OwnerEditDlg(CICQDaemon *, const char *, unsigned long, QWidget *parent = 0);
  
protected slots:
  void slot_ok();
  
protected:
  CICQDaemon *server;
  
  QPushButton *btnOk,
              *btnCancel;
  QLineEdit *edtId,
            *edtPassword;
  QComboBox *cmbProtocol;
};

class OwnerView : public QListView
{
  Q_OBJECT
public:
  OwnerView(QWidget *parent = 0);
};

class OwnerItem : public QListViewItem
{
public:
  OwnerItem(CICQDaemon *, const char *szId,
    unsigned long nPPID, QListView *parent);
  char *Id()           { return m_szId; }
  unsigned long PPID() { return m_nPPID; }
  
protected:
  char *m_szId;
  unsigned long m_nPPID;
};

class OwnerManagerDlg : public LicqDialog
{
   Q_OBJECT
public:
  OwnerManagerDlg(CICQDaemon *s);

protected slots:
  void slot_listClicked(QListViewItem *, const QPoint &, int);
  void slot_addClicked();
  void slot_modifyClicked();
  void slot_deleteClicked();
  void slot_update();
  
protected:
  void updateOwners();
  
  CICQDaemon *server;
  
  OwnerView *ownerView;
  QPushButton *btnAdd,
              *btnModify,
              *btnDelete,
              *btnDone;
};

#endif // __OWNERMANAGERDLG_H
