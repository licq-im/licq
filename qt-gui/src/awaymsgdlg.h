#ifndef AWAYMSG_H
#define AWAYMSG_H

#include <qdialog.h>

class QPushButton;
class QComboBox;
class MLEditWrap;

class AwayMsgDlg : public QDialog
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

signals:
  void popupOptions(int);
  void done();

protected slots:
  void ok();
  void slot_selectMessage(int);
};


class CustomAwayMsgDlg : public QDialog
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
};


#endif
