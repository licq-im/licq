#ifndef REFUSEDLG_H
#define REFUSEDLG_H

#include "licqdialog.h" 

class MLEditWrap;

class CRefuseDlg : public LicqDialog
{
  Q_OBJECT

public:
  CRefuseDlg(unsigned long _nUin, QString t, QWidget *parent = 0);
  CRefuseDlg(const char *, unsigned long, QString, QWidget * = 0);
  QString RefuseMessage();

protected:
  MLEditWrap *mleRefuseMsg;
};


#endif
