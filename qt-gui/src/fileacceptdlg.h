#ifndef FILEMSG_H
#define FILEMSG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qmlined.h>

#include "icq.h"
#include "user.h"
#include "mledit.h"

class CFileAcceptDlg : public QWidget
{
   Q_OBJECT
public:
   CFileAcceptDlg(CICQDaemon *_xServer, unsigned long _nUin, 
                  CEventFile *e, QWidget *parent = 0, 
                  const char *name = 0);
   ~CFileAcceptDlg(void);
protected:
   CICQDaemon *m_xServer;
   unsigned long m_nUin;
   CEventFile *m_xEventFile;
   MLEditWrap *mleRefuseMsg;
protected slots:
   void accept();
   void refuse();
   void ignore();
};


#endif
