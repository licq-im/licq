#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <qpushbt.h>
#include <qlabel.h>
#include "chatacceptdlg.h"
#include "chatdlg.h"
#include "mledit.h"

#include "icqd.h"
#include "user.h"

CChatAcceptDlg::CChatAcceptDlg(CICQDaemon *_xServer, unsigned long _nUin, 
                         unsigned long _nSequence,
                         QWidget *parent, const char *name) 
   : QWidget(parent, name)
{
   m_xServer = _xServer;
   m_nUin = _nUin;
   m_nSequence = _nSequence;

   resize(300, 130);
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
   setCaption(tr("Accept chat with ") + QString::fromLocal8Bit(u->getAlias()) + " ?");
   gUserManager.DropUser(u);

   show();
}


void CChatAcceptDlg::accept()
{
   unsigned short port;
   if (m_xServer->getTcpServerPort() != 0)   // assign the chat port
   {
      unsigned short i = 0;
      while (i < 10 && m_xServer->getTcpPort(i)) i++;
      port = m_xServer->getTcpServerPort() + i + 1;
      m_xServer->setTcpPort(i, true);
   }
   else port = 0;
   ChatDlg *chatDlg = new ChatDlg(m_nUin, true, port);
   if (chatDlg->getPort() != 0)
   {
      m_xServer->icqChatRequestAccept(m_nUin, chatDlg->getPort(), m_nSequence);
      chatDlg->show();
   } 
   else 
      chatDlg->hide();

   hide();
   delete this;
}

void CChatAcceptDlg::refuse()
{
   m_xServer->icqChatRequestRefuse(m_nUin, (const char *)mleRefuseMsg->text(), m_nSequence);
   hide();
   delete this;
}


void CChatAcceptDlg::ignore()
{
   // do nothing
   hide();
   delete this;
}

#include "moc/moc_chatacceptdlg.h"
