#ifndef AWAYMSG_H
#define AWAYMSG_H

#include "licqdialog.h"

class QPushButton;
class QComboBox;
class MLEditWrap;

class AwayMsgDlg : public LicqDialog
{
  Q_OBJECT
public:
  AwayMsgDlg(QWidget *parent = 0);
  ~AwayMsgDlg();
  void SelectAutoResponse(unsigned short status);

protected:
  MLEditWrap *mleAwayMsg;
  QPopupMenu* mnuSelect;
  QPushButton* btnSelect;
  QPushButton *btnOk, *btnCancel;
  unsigned short m_nStatus;
  static QPoint snPos;
  short m_nSAR;

  void closeEvent(QCloseEvent *);

signals:
  void popupOptions(int);
  void done();

protected slots:
  void ok();
  virtual void reject();
  void slot_selectMessage(int);
  void slot_hints();
};

class CustomAwayMsgDlg : public LicqDialog
{
  Q_OBJECT
public:
  CustomAwayMsgDlg(unsigned long, QWidget *parent = 0);

protected:
  MLEditWrap *mleAwayMsg;
  unsigned long m_nUin;

protected slots:
  void slot_clear();
  void slot_ok();
  void slot_hints();
};


#endif
