#ifndef REFUSEDLG_H
#define REFUSEDLG_H

#include <qdialog.h>

class MLEditWrap;

class CRefuseDlg : public QDialog
{
public:
  CRefuseDlg(unsigned long _nUin, QString t, QWidget *parent = 0);
  QString RefuseMessage();

protected:
  MLEditWrap *mleRefuseMsg;
};


#endif
