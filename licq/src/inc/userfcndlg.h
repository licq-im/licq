#ifndef CUSERDLG_H
#define CUSERDLG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

class CUserFunctionDlg
{
public:
  unsigned long Uin(void)  { return m_nUin; };
protected:
  unsigned long m_nUin;
};


#endif
