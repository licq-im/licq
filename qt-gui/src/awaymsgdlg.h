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
  AwayMsgDlg(QWidget *parent = 0, const char *name = 0);
  void SelectAutoResponse(unsigned short status);

protected:
  MLEditWrap *mleAwayMsg;
  QPushButton* btnSelect;
  QPushButton *btnOk, *btnCancel;
  unsigned short m_nStatus;
  short m_nSAR;

  virtual void hideEvent(QHideEvent*);

signals:
  void popupOptions(int);
  void done();

protected slots:
  void ok();
  void slot_selectMessage();
};


#endif
