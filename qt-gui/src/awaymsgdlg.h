#ifndef AWAYMSG_H
#define AWAYMSG_H

#include <qdialog.h>

class QPushButton;
class QComboBox;
class QMultiLineEdit;

class AwayMsgDlg : public QDialog
{
  Q_OBJECT
public:
  AwayMsgDlg(QWidget *parent = 0, const char *name = 0);
  void SelectAutoResponse(unsigned short status);

protected:
  QMultiLineEdit *mleAwayMsg;
  QPushButton* btnSelect;
  QPushButton *btnOk, *btnCancel;
  unsigned short m_nStatus;
  static QPoint snPos;
  short m_nSAR;

  virtual void hideEvent(QHideEvent*);

signals:
  void popupOptions(int);
  void done();

protected slots:
  void ok();
  void slot_selectMessage();
};


class CustomAwayMsgDlg : public QDialog
{
  Q_OBJECT
public:
  CustomAwayMsgDlg(unsigned long, QWidget *parent = 0, const char *name = 0);

protected:
  QMultiLineEdit *mleAwayMsg;
  //unsigned short m_nStatus;
  unsigned long m_nUin;

  virtual void hideEvent(QHideEvent*);

//signals:
  //void popupOptions(int);

protected slots:
  void slot_clear();
  void slot_ok();
  //void slot_selectMessage();
};



#endif
