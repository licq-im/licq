#ifndef REFUSEDLG_H
#define REFUSEDLG_H

#include "licqdialog.h" 

class MLEditWrap;

class CRefuseDlg : public LicqDialog
{
public:
  CRefuseDlg(unsigned long _nUin, QString t, QWidget *parent = 0);
#ifdef QT_PROTOCOL_PLUGIN
  CRefuseDlg(const char *, unsigned long, QString, QWidget * = 0);
#endif
  QString RefuseMessage();

protected:
  MLEditWrap *mleRefuseMsg;
};


#endif
