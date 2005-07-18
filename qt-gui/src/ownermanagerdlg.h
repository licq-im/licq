#ifndef __OWNERMANAGERDLG_H
#define __OWNERMANAGERDLG_H

#include "licqdialog.h"
#include <qlistview.h>

#include "registeruser.h"

class QPushButton;
class QLabel;
class QLineEdit;
class QComboBox;

class CICQDaemon;
class CSignalManager;
class ICQEvent;
class CMainWindow;

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
  OwnerManagerDlg(CMainWindow *m, CICQDaemon *s, CSignalManager *sm);
  virtual ~OwnerManagerDlg();
  void slot_doneRegisterUser(ICQEvent *);

protected slots:
  void slot_listClicked(QListViewItem *);
  void slot_listClicked(QListViewItem *, const QPoint &, int);
  void slot_addClicked();
  void slot_registerClicked();
  void slot_doneregister(bool, char *, unsigned long);
  void slot_modifyClicked();
  void slot_deleteClicked();
  void slot_update();
  
protected:
  void updateOwners();
  
  CMainWindow *mainwin;
  CICQDaemon *server;
  CSignalManager *sigman;
  RegisterUserDlg *registerUserDlg;
  
  OwnerView *ownerView;
  QPushButton *btnAdd,
              *btnRegister,
              *btnModify,
              *btnDelete,
              *btnDone;

signals:
  void signal_done();
};

#endif // __OWNERMANAGERDLG_H
