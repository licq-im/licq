#ifndef CHATMSG_H
#define CHATMSG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//#include <qdialog.h>
#include <qmlined.h>

class CICQDaemon;
class MLEditWrap;

class CChatAcceptDlg : public QWidget
{
   Q_OBJECT
public:
   CChatAcceptDlg(CICQDaemon *_xServer, unsigned long _nUin,
                  unsigned long _nSequence, QWidget *parent = 0,
                  const char *name = 0);
protected:
   CICQDaemon *m_xServer;
   unsigned long m_nUin;
   unsigned long m_nSequence;
   MLEditWrap *mleRefuseMsg;
protected slots:
   void accept();
   void refuse();
   void ignore();
};


#endif
