#ifndef FORWARDDLG_H
#define FORWARDDLG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include <qwidget.h>

class CMainWindow;
class QPushButton;
class CUserEvent;
class CInfoField;


class CForwardDlg : public QWidget
{
Q_OBJECT
public:
  CForwardDlg(CMainWindow *_mainwin, CUserEvent *e,
                 QWidget *p = NULL, const char *n = NULL);
  ~CForwardDlg();
protected:
  virtual void dragEnterEvent(QDragEnterEvent * dee);
  virtual void dropEvent(QDropEvent * de);
  QString s1, s2;
  unsigned long m_nEventType;
  CMainWindow *mainwin;
  CInfoField *edtUser;
  QPushButton *btnOk, *btnCancel;
  unsigned long m_nUin;
protected slots:
  void slot_ok();
  virtual void hide();
};

#endif
