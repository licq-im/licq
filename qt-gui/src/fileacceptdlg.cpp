#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <qpushbutton.h>

#include "fileacceptdlg.h"
#include "filedlg.h"
#include "mledit.h"

#include "icqd.h"
#include "user.h"
#include "message.h"

CFileAcceptDlg::CFileAcceptDlg(CICQDaemon *_xServer, unsigned long _nUin,
                               CEventFile *e,
                               QWidget *parent, const char *name)
   : QWidget(parent, name)
{
   m_xServer = _xServer;
   m_nUin = _nUin;
   m_xEventFile = e->Copy();

   setGeometry(100, 100, 300, 130);
   QLabel *lblRefuse = new QLabel(tr("Refusal Reason (if applicable):"), this);
   lblRefuse->setGeometry(5, 5, 200, 20);
   mleRefuseMsg = new MLEditWrap(true, this);
   mleRefuseMsg->setGeometry(5, 30, 290, 50);

   QPushButton *btnAccept, *btnRefuse, *btnIgnore;
   btnAccept = new QPushButton(tr("Accept"), this );
   btnAccept->setGeometry(10, 85, 80, 30 );
   connect( btnAccept, SIGNAL(clicked()), SLOT(accept()) );
   btnRefuse = new QPushButton(tr("Refuse"), this );
   btnRefuse->setGeometry( 100, 85, 80, 30 );
   connect( btnRefuse, SIGNAL(clicked()), SLOT(refuse()) );
   btnIgnore = new QPushButton(tr("Ignore"), this );
   btnIgnore->setGeometry(190, 85, 80, 30 );
   connect( btnIgnore, SIGNAL(clicked()), SLOT(ignore()) );

   ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
   setCaption(tr("Accept file transfer from ") + QString::fromLocal8Bit(u->GetAlias())+ " ?");
   gUserManager.DropUser(u);

   show();
}


CFileAcceptDlg::~CFileAcceptDlg(void)
{
   delete m_xEventFile;
}


void CFileAcceptDlg::accept()
{
   unsigned short port;
   if (m_xServer->getTcpServerPort() != 0)   // assign the file port
   {
      unsigned short i = 0;
      while (i < 10 && m_xServer->getTcpPort(i)) i++;
      port = m_xServer->getTcpServerPort() + i + 1;
      m_xServer->setTcpPort(i, true);
   }
   else port = 0;
   CFileDlg *fileDlg = new CFileDlg(m_nUin, m_xEventFile->Filename(), 
                                    m_xEventFile->FileSize(), 
                                    true, port);
   if (fileDlg->getPort() != 0)
   {
      m_xServer->icqFileTransferAccept(m_nUin, fileDlg->getPort(), m_xEventFile->Sequence());
      fileDlg->show();
   }
   else
      fileDlg->hide();

   hide();
   delete this;
}

void CFileAcceptDlg::refuse()
{
   m_xServer->icqFileTransferRefuse(m_nUin, (const char *)mleRefuseMsg->text(), 
                         m_xEventFile->Sequence());
   hide();
   delete this;
}


void CFileAcceptDlg::ignore()
{
   // do nothing
   hide();
   delete this;
}

#include "moc/moc_fileacceptdlg.h"
