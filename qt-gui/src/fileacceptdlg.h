#ifndef FILEACCEPTDLG_H
#define FILEACCEPTDLG_H

#include <qwidget.h>

class CICQDaemon;
class MLEditWrap;
class CEventFile;

class CFileAcceptDlg : public QWidget
{
   Q_OBJECT
public:
   CFileAcceptDlg(CICQDaemon *_xServer, unsigned long _nUin,
                  CEventFile *e, QWidget *parent = 0,
                  const char *name = 0);
   ~CFileAcceptDlg(void);
public slots:
   virtual void hide();

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
